/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * Source is based on the player example from libvorbis package,
 * available at: http://svn.xiph.org/trunk/theora/examples/player_example.c
 *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.
 *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/
 *
 */

#include "sword25/fmv/theora_decoder.h"

#ifdef USE_THEORADEC
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/yuv_to_rgb.h"
#include "audio/decoders/raw.h"
#include "sword25/kernel/common.h"

namespace Sword25 {

#define AUDIOFD_FRAGSIZE 10240

static double rint(double v) {
	return floor(v + 0.5);
}

TheoraDecoder::TheoraDecoder(Audio::Mixer::SoundType soundType) {
	_fileStream = 0;

	_theoraPacket = 0;
	_vorbisPacket = 0;
	_theoraDecode = 0;
	_theoraSetup = 0;
	_nextFrameStartTime = 0.0;

	_soundType = soundType;
	_audStream = 0;
	_audHandle = new Audio::SoundHandle();

	ogg_sync_init(&_oggSync);

	_curFrame = -1;
	_audiobuf = (ogg_int16_t *)malloc(AUDIOFD_FRAGSIZE * sizeof(ogg_int16_t));

	reset();
}

TheoraDecoder::~TheoraDecoder() {
	close();
	delete _fileStream;
	delete _audHandle;
	free(_audiobuf);
}

void TheoraDecoder::queuePage(ogg_page *page) {
	if (_theoraPacket)
		ogg_stream_pagein(&_theoraOut, page);

	if (_vorbisPacket)
		ogg_stream_pagein(&_vorbisOut, page);
}

int TheoraDecoder::bufferData() {
	char *buffer = ogg_sync_buffer(&_oggSync, 4096);
	int bytes = _fileStream->read(buffer, 4096);

	ogg_sync_wrote(&_oggSync, bytes);

	return bytes;
}

bool TheoraDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_endOfAudio = false;
	_endOfVideo = false;
	_fileStream = stream;

	// start up Ogg stream synchronization layer
	ogg_sync_init(&_oggSync);

	// init supporting Vorbis structures needed in header parsing
	vorbis_info_init(&_vorbisInfo);
	vorbis_comment_init(&_vorbisComment);

	// init supporting Theora structures needed in header parsing
	th_comment_init(&_theoraComment);
	th_info_init(&_theoraInfo);

	// Ogg file open; parse the headers
	// Only interested in Vorbis/Theora streams
	bool foundHeader = false;
	while (!foundHeader) {
		int ret = bufferData();

		if (ret == 0)
			break;

		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0) {
			ogg_stream_state test;

			// is this a mandated initial header? If not, stop parsing
			if (!ogg_page_bos(&_oggPage)) {
				// don't leak the page; get it into the appropriate stream
				queuePage(&_oggPage);
				foundHeader = true;
				break;
			}

			ogg_stream_init(&test, ogg_page_serialno(&_oggPage));
			ogg_stream_pagein(&test, &_oggPage);
			ogg_stream_packetout(&test, &_oggPacket);

			// identify the codec: try theora
			if (!_theoraPacket && th_decode_headerin(&_theoraInfo, &_theoraComment, &_theoraSetup, &_oggPacket) >= 0) {
				// it is theora
				memcpy(&_theoraOut, &test, sizeof(test));
				_theoraPacket = 1;
			} else if (!_vorbisPacket && vorbis_synthesis_headerin(&_vorbisInfo, &_vorbisComment, &_oggPacket) >= 0) {
				// it is vorbis
				memcpy(&_vorbisOut, &test, sizeof(test));
				_vorbisPacket = 1;
			} else {
				// whatever it is, we don't care about it
				ogg_stream_clear(&test);
			}
		}
		// fall through to non-bos page parsing
	}

	// we're expecting more header packets.
	while ((_theoraPacket && _theoraPacket < 3) || (_vorbisPacket && _vorbisPacket < 3)) {
		int ret;

		// look for further theora headers
		while (_theoraPacket && (_theoraPacket < 3) && (ret = ogg_stream_packetout(&_theoraOut, &_oggPacket))) {
			if (ret < 0)
				error("Error parsing Theora stream headers; corrupt stream?");

			if (!th_decode_headerin(&_theoraInfo, &_theoraComment, &_theoraSetup, &_oggPacket))
				error("Error parsing Theora stream headers; corrupt stream?");

			_theoraPacket++;
		}

		// look for more vorbis header packets
		while (_vorbisPacket && (_vorbisPacket < 3) && (ret = ogg_stream_packetout(&_vorbisOut, &_oggPacket))) {
			if (ret < 0)
				error("Error parsing Vorbis stream headers; corrupt stream?");

			if (vorbis_synthesis_headerin(&_vorbisInfo, &_vorbisComment, &_oggPacket))
				error("Error parsing Vorbis stream headers; corrupt stream?");

			_vorbisPacket++;

			if (_vorbisPacket == 3)
				break;
		}

		// The header pages/packets will arrive before anything else we
		// care about, or the stream is not obeying spec

		if (ogg_sync_pageout(&_oggSync, &_oggPage) > 0) {
			queuePage(&_oggPage); // demux into the appropriate stream
		} else {
			ret = bufferData(); // someone needs more data

			if (ret == 0)
				error("End of file while searching for codec headers.");
		}
	}

	// and now we have it all.  initialize decoders
	if (_theoraPacket) {
		_theoraDecode = th_decode_alloc(&_theoraInfo, _theoraSetup);
		debugN(1, "Ogg logical stream %lx is Theora %dx%d %.02f fps",
		       _theoraOut.serialno, _theoraInfo.pic_width, _theoraInfo.pic_height,
		       (double)_theoraInfo.fps_numerator / _theoraInfo.fps_denominator);

		switch (_theoraInfo.pixel_fmt) {
		case TH_PF_420:
			debug(1, " 4:2:0 video");
			break;
		case TH_PF_422:
			debug(1, " 4:2:2 video");
			break;
		case TH_PF_444:
			debug(1, " 4:4:4 video");
			break;
		case TH_PF_RSVD:
		default:
			debug(1, " video\n  (UNKNOWN Chroma sampling!)");
			break;
		}

		if (_theoraInfo.pic_width != _theoraInfo.frame_width || _theoraInfo.pic_height != _theoraInfo.frame_height)
			debug(1, "  Frame content is %dx%d with offset (%d,%d).",
			      _theoraInfo.frame_width, _theoraInfo.frame_height, _theoraInfo.pic_x, _theoraInfo.pic_y);

		switch (_theoraInfo.colorspace){
		case TH_CS_UNSPECIFIED:
			/* nothing to report */
			break;
		case TH_CS_ITU_REC_470M:
			debug(1, "  encoder specified ITU Rec 470M (NTSC) color.");
			break;
		case TH_CS_ITU_REC_470BG:
			debug(1, "  encoder specified ITU Rec 470BG (PAL) color.");
			break;
		default:
			debug(1, "warning: encoder specified unknown colorspace (%d).", _theoraInfo.colorspace);
			break;
		}

		debug(1, "Encoded by %s", _theoraComment.vendor);
		if (_theoraComment.comments) {
			debug(1, "theora comment header:");
			for (int i = 0; i < _theoraComment.comments; i++) {
				if (_theoraComment.user_comments[i]) {
					int len = _theoraComment.comment_lengths[i];
					char *value = (char *)malloc(len + 1);
					if (value) {
						memcpy(value, _theoraComment.user_comments[i], len);
						value[len] = '\0';
						debug(1, "\t%s", value);
						free(value);
					}
				}
			}
		}

		th_decode_ctl(_theoraDecode, TH_DECCTL_GET_PPLEVEL_MAX, &_ppLevelMax, sizeof(_ppLevelMax));
		_ppLevel = _ppLevelMax;
		th_decode_ctl(_theoraDecode, TH_DECCTL_SET_PPLEVEL, &_ppLevel, sizeof(_ppLevel));
		_ppInc = 0;
	} else {
		// tear down the partial theora setup
		th_info_clear(&_theoraInfo);
		th_comment_clear(&_theoraComment);
	}

	th_setup_free(_theoraSetup);
	_theoraSetup = 0;

	if (_vorbisPacket) {
		vorbis_synthesis_init(&_vorbisDSP, &_vorbisInfo);
		vorbis_block_init(&_vorbisDSP, &_vorbisBlock);
		debug(3, "Ogg logical stream %lx is Vorbis %d channel %ld Hz audio.",
		      _vorbisOut.serialno, _vorbisInfo.channels, _vorbisInfo.rate);

		_audStream = Audio::makeQueuingAudioStream(_vorbisInfo.rate, _vorbisInfo.channels);

		// Get enough audio data to start us off
		while (_audStream->numQueuedStreams() == 0) {
			// Queue more data
			bufferData();
			while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
				queuePage(&_oggPage);

			queueAudio();
		}

		if (_audStream)
			g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, _audHandle, _audStream);
	} else {
		// tear down the partial vorbis setup
		vorbis_info_clear(&_vorbisInfo);
		vorbis_comment_clear(&_vorbisComment);
		_endOfAudio = true;
	}

	_surface.create(_theoraInfo.frame_width, _theoraInfo.frame_height, g_system->getScreenFormat());

	// Set up a display surface
	_displaySurface.pixels = _surface.getBasePtr(_theoraInfo.pic_x, _theoraInfo.pic_y);
	_displaySurface.w = _theoraInfo.pic_width;
	_displaySurface.h = _theoraInfo.pic_height;
	_displaySurface.format = _surface.format;
	_displaySurface.pitch = _surface.pitch;

	// Set the frame rate
	_frameRate = Common::Rational(_theoraInfo.fps_numerator, _theoraInfo.fps_denominator);

	return true;
}

void TheoraDecoder::close() {
	if (_vorbisPacket) {
		ogg_stream_clear(&_vorbisOut);
		vorbis_block_clear(&_vorbisBlock);
		vorbis_dsp_clear(&_vorbisDSP);
		vorbis_comment_clear(&_vorbisComment);
		vorbis_info_clear(&_vorbisInfo);

		g_system->getMixer()->stopHandle(*_audHandle);

		_audStream = 0;
		_vorbisPacket = false;
	}
	if (_theoraPacket) {
		ogg_stream_clear(&_theoraOut);
		th_decode_free(_theoraDecode);
		th_comment_clear(&_theoraComment);
		th_info_clear(&_theoraInfo);
		_theoraDecode = 0;
		_theoraPacket = false;
	}

	if (!_fileStream)
		return;

	ogg_sync_clear(&_oggSync);

	delete _fileStream;
	_fileStream = 0;

	_surface.free();
	_displaySurface.pixels = 0;
	_displaySurface.free();

	reset();
}

const Graphics::Surface *TheoraDecoder::decodeNextFrame() {
	// First, let's get our frame
	while (_theoraPacket) {
		// theora is one in, one out...
		if (ogg_stream_packetout(&_theoraOut, &_oggPacket) > 0) {

			if (_ppInc) {
				_ppLevel += _ppInc;
				th_decode_ctl(_theoraDecode, TH_DECCTL_SET_PPLEVEL, &_ppLevel, sizeof(_ppLevel));
				_ppInc = 0;
			}

			if (th_decode_packetin(_theoraDecode, &_oggPacket, NULL) == 0) {
				_curFrame++;

				// Convert YUV data to RGB data
				th_ycbcr_buffer yuv;
				th_decode_ycbcr_out(_theoraDecode, yuv);
				translateYUVtoRGBA(yuv);

				if (_curFrame == 0)
					_startTime = g_system->getMillis();

				double time = th_granule_time(_theoraDecode, _oggPacket.granulepos);

				// We need to calculate when the next frame should be shown
				// This is all in floating point because that's what the Ogg code gives us
				// Ogg is a lossy container format, so it doesn't always list the time to the
				// next frame. In such cases, we need to calculate it ourselves.
				if (time == -1.0)
					_nextFrameStartTime += _frameRate.getInverse().toDouble();
				else
					_nextFrameStartTime = time;

				// break out
				break;
			}
		} else {
			// If we can't get any more frames, we're done.
			if (_theoraOut.e_o_s || _fileStream->eos()) {
				_endOfVideo = true;
				break;
			}

			// Queue more data
			bufferData();
			while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
				queuePage(&_oggPage);
		}

		// Update audio if we can
		queueAudio();
	}

	// Force at least some audio to be buffered
	// TODO: 5 is very arbitrary. We probably should do something like QuickTime does.
	while (!_endOfAudio && _audStream->numQueuedStreams() < 5) {
		bufferData();
		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0)
			queuePage(&_oggPage);

		bool queuedAudio = queueAudio();
		if ((_vorbisOut.e_o_s  || _fileStream->eos()) && !queuedAudio) {
			_endOfAudio = true;
			break;
		}
	}

	return &_displaySurface;
}

bool TheoraDecoder::queueAudio() {
	if (!_audStream)
		return false;

	// An audio buffer should have been allocated (either in the constructor or after queuing the current buffer)
	if (!_audiobuf) {
		warning("[TheoraDecoder::queueAudio] Invalid audio buffer");
		return false;
	}

	bool queuedAudio = false;

	for (;;) {
		float **pcm;

		// if there's pending, decoded audio, grab it
		int ret = vorbis_synthesis_pcmout(&_vorbisDSP, &pcm);
		if (ret > 0) {
			int count = _audiobufFill / 2;
			int maxsamples = ((AUDIOFD_FRAGSIZE - _audiobufFill) / _vorbisInfo.channels) >> 1;
			int i;
			for (i = 0; i < ret && i < maxsamples; i++)
				for (int j = 0; j < _vorbisInfo.channels; j++) {
					int val = CLIP((int)rint(pcm[j][i] * 32767.f), -32768, 32767);
					_audiobuf[count++] = val;
				}

			vorbis_synthesis_read(&_vorbisDSP, i);
			_audiobufFill += (i * _vorbisInfo.channels) << 1;

			if (_audiobufFill == AUDIOFD_FRAGSIZE) {
				byte flags = Audio::FLAG_16BITS | Audio::FLAG_STEREO;
#ifdef SCUMM_LITTLE_ENDIAN
				flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
				_audStream->queueBuffer((byte *)_audiobuf, AUDIOFD_FRAGSIZE, DisposeAfterUse::NO, flags);

				// The audio mixer is now responsible for the old audio buffer.
				// We need to create a new one.
				_audiobuf = (ogg_int16_t *)malloc(AUDIOFD_FRAGSIZE * sizeof(ogg_int16_t));
				if (!_audiobuf) {
					warning("[TheoraDecoder::queueAudio] Cannot allocate memory for audio buffer");
					return false;
				}

				_audiobufFill = 0;
				queuedAudio = true;
			}
		} else {
			// no pending audio; is there a pending packet to decode?
			if (ogg_stream_packetout(&_vorbisOut, &_oggPacket) > 0) {
				if (vorbis_synthesis(&_vorbisBlock, &_oggPacket) == 0) // test for success!
					vorbis_synthesis_blockin(&_vorbisDSP, &_vorbisBlock);
			} else   // we've buffered all we have, break out for now
				return queuedAudio;
		}
	}

	// Unreachable
	return false;
}

void TheoraDecoder::reset() {
	VideoDecoder::reset();

	// FIXME: This does a rewind() instead of a reset()!

	if (_fileStream)
		_fileStream->seek(0);

	_audiobufFill = 0;
	_audiobufReady = false;

	_curFrame = -1;

	_theoraPacket = 0;
	_vorbisPacket = 0;
}

bool TheoraDecoder::endOfVideo() const {
	return !isVideoLoaded() || (_endOfVideo && (!_audStream || (_audStream->endOfData() && _endOfAudio)));
}

uint32 TheoraDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _curFrame < 0)
		return 0;

	uint32 elapsedTime = getElapsedTime();
	uint32 nextFrameStartTime = (uint32)(_nextFrameStartTime * 1000);

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

uint32 TheoraDecoder::getElapsedTime() const {
	if (_audStream)
		return g_system->getMixer()->getSoundElapsedTime(*_audHandle);

	return VideoDecoder::getElapsedTime();
}

void TheoraDecoder::pauseVideoIntern(bool pause) {
	if (_audStream)
		g_system->getMixer()->pauseHandle(*_audHandle, pause);
}

enum TheoraYUVBuffers {
	kBufferY = 0,
	kBufferU = 1,
	kBufferV = 2
};

void TheoraDecoder::translateYUVtoRGBA(th_ycbcr_buffer &YUVBuffer) {
	// Width and height of all buffers have to be divisible by 2.
	assert((YUVBuffer[kBufferY].width & 1)   == 0);
	assert((YUVBuffer[kBufferY].height & 1)  == 0);
	assert((YUVBuffer[kBufferU].width & 1)   == 0);
	assert((YUVBuffer[kBufferV].width & 1)   == 0);

	// UV images have to have a quarter of the Y image resolution
	assert(YUVBuffer[kBufferU].width  == YUVBuffer[kBufferY].width  >> 1);
	assert(YUVBuffer[kBufferV].width  == YUVBuffer[kBufferY].width  >> 1);
	assert(YUVBuffer[kBufferU].height == YUVBuffer[kBufferY].height >> 1);
	assert(YUVBuffer[kBufferV].height == YUVBuffer[kBufferY].height >> 1);

	Graphics::convertYUV420ToRGB(&_surface, YUVBuffer[kBufferY].data, YUVBuffer[kBufferU].data, YUVBuffer[kBufferV].data, YUVBuffer[kBufferY].width, YUVBuffer[kBufferY].height, YUVBuffer[kBufferY].stride, YUVBuffer[kBufferU].stride);
}

} // End of namespace Sword25

#endif
