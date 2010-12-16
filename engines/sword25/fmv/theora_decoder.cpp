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
 * $URL$
 * $Id$
 *
 */

/*
 * Source is based on the player example from libvorbis package
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
#include "sword25/fmv/yuvtorgba.h"
#include "common/system.h"
#include "sound/decoders/raw.h"

namespace Sword25 {

#define AUDIOFD_FRAGSIZE 10240

static double rint(double v) {
	return floor(v + 0.5);
}

TheoraDecoder::TheoraDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType) : _mixer(mixer) {
	_fileStream = 0;
	_surface = 0;

	_theoraPacket = 0;
	_vorbisPacket = 0;
	_theoraDecode = 0;
	_theoraSetup = 0;
	_stateFlag = false;

	_soundType = soundType;
	_audStream = 0;
	_audHandle = new Audio::SoundHandle();

	ogg_sync_init(&_oggSync);

	_curFrame = 0;
	_audiobuf = (ogg_int16_t *)calloc(AUDIOFD_FRAGSIZE, sizeof(ogg_int16_t));

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

bool TheoraDecoder::load(Common::SeekableReadStream *stream) {
	close();

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
	while (!_stateFlag) {
		int ret = bufferData();

		if (ret == 0)
			break;

		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0) {
			ogg_stream_state test;

			// is this a mandated initial header? If not, stop parsing
			if (!ogg_page_bos(&_oggPage)) {
				// don't leak the page; get it into the appropriate stream
				queuePage(&_oggPage);
				_stateFlag = true;
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
			break;;
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
	} else {
		// tear down the partial vorbis setup
		vorbis_info_clear(&_vorbisInfo);
		vorbis_comment_clear(&_vorbisComment);
	}

	// open audio
	if (_vorbisPacket) {
		_audStream = createAudioStream();
		if (_audStream && _mixer)
			_mixer->playStream(_soundType, _audHandle, _audStream);
	}

	_surface = new Graphics::Surface();

	_surface->create(_theoraInfo.frame_width, _theoraInfo.frame_height, 4);

	return true;
}

void TheoraDecoder::close() {
	if (_vorbisPacket) {
		ogg_stream_clear(&_vorbisOut);
		vorbis_block_clear(&_vorbisBlock);
		vorbis_dsp_clear(&_vorbisDSP);
		vorbis_comment_clear(&_vorbisComment);
		vorbis_info_clear(&_vorbisInfo);

		if (_mixer)
			_mixer->stopHandle(*_audHandle);
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

	_surface->free();
	delete _surface;
	_surface = 0;

	reset();
}

const Graphics::Surface *TheoraDecoder::decodeNextFrame() {
	int i, j;

//	_stateFlag = false; // playback has not begun

	// we want a video and audio frame ready to go at all times.  If
	// we have to buffer incoming, buffer the compressed data (ie, let
	// ogg do the buffering)
	while (_vorbisPacket && !_audiobufReady) {
		int ret;
		float **pcm;

		// if there's pending, decoded audio, grab it
		if ((ret = vorbis_synthesis_pcmout(&_vorbisDSP, &pcm)) > 0) {
			int count = _audiobufFill / 2;
			int maxsamples = (AUDIOFD_FRAGSIZE - _audiobufFill) / 2 / _vorbisInfo.channels;
			for (i = 0; i < ret && i < maxsamples; i++)
				for (j = 0; j < _vorbisInfo.channels; j++) {
					int val = CLIP((int)rint(pcm[j][i] * 32767.f), -32768, 32767);
					_audiobuf[count++] = val;
				}

			vorbis_synthesis_read(&_vorbisDSP, i);
			_audiobufFill += i * _vorbisInfo.channels * 2;

			if (_audiobufFill == AUDIOFD_FRAGSIZE)
				_audiobufReady = true;

			if (_vorbisDSP.granulepos >= 0)
				_audiobufGranulePos = _vorbisDSP.granulepos - ret + i;
			else
				_audiobufGranulePos += i;
		} else {

			// no pending audio; is there a pending packet to decode?
			if (ogg_stream_packetout(&_vorbisOut, &_oggPacket) > 0) {
				if (vorbis_synthesis(&_vorbisBlock, &_oggPacket) == 0) // test for success!
					vorbis_synthesis_blockin(&_vorbisDSP, &_vorbisBlock);
			} else   // we need more data; break out to suck in another page
				break;
		}
	}

	while (_theoraPacket && !_videobufReady) {
		// theora is one in, one out...
		if (ogg_stream_packetout(&_theoraOut, &_oggPacket) > 0) {

			if (_ppInc) {
				_ppLevel += _ppInc;
				th_decode_ctl(_theoraDecode, TH_DECCTL_SET_PPLEVEL, &_ppLevel, sizeof(_ppLevel));
				_ppInc = 0;
			}
			// HACK: This should be set after a seek or a gap, but we might not have
			// a granulepos for the first packet (we only have them for the last
			// packet on a page), so we just set it as often as we get it.
			// To do this right, we should back-track from the last packet on the
			// page and compute the correct granulepos for the first packet after
			// a seek or a gap.
			if (_oggPacket.granulepos >= 0) {
				th_decode_ctl(_theoraDecode, TH_DECCTL_SET_GRANPOS, &_oggPacket.granulepos, sizeof(_oggPacket.granulepos));
			}
			if (th_decode_packetin(_theoraDecode, &_oggPacket, &_videobufGranulePos) == 0) {
				_videobufTime = th_granule_time(_theoraDecode, _videobufGranulePos);
				_curFrame++;

				_videobufReady = true;
			}
		} else
			break;
	}

	if (!_videobufReady && !_audiobufReady && _fileStream->eos()) {
		return NULL;
	}

	if (!_videobufReady || !_audiobufReady) {
		// no data yet for somebody.  Grab another page
		bufferData();
		while (ogg_sync_pageout(&_oggSync, &_oggPage) > 0) {
			queuePage(&_oggPage);
		}
	}

	// If playback has begun, top audio buffer off immediately.
	if (_stateFlag && _audiobufReady) {
		_audStream->queueBuffer((byte *)_audiobuf, AUDIOFD_FRAGSIZE, DisposeAfterUse::NO, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN | Audio::FLAG_STEREO);

		// The audio mixer is now responsible for the old audio buffer.
		// We need to create a new one.
		_audiobuf = (ogg_int16_t *)calloc(AUDIOFD_FRAGSIZE, sizeof(ogg_int16_t));
		_audiobufFill = 0;
		_audiobufReady = false;
	}

	// are we at or past time for this video frame?
	if (_stateFlag && _videobufReady) {
		th_ycbcr_buffer yuv;

		th_decode_ycbcr_out(_theoraDecode, yuv);

		// Convert YUV data to RGB data
		YUVtoBGRA::translate(yuv, _theoraInfo, (byte *)_surface->getBasePtr(0, 0), _surface->pitch * _surface->h);
		
		switch (_theoraInfo.pixel_fmt) {
		case TH_PF_420:
			break;
		case TH_PF_422:
			break;
		case TH_PF_444:
			break;
		default:
			break;
		}

		_videobufReady = false;
	}

	// if our buffers either don't exist or are ready to go,
	// we can begin playback
	if ((!_theoraPacket || _videobufReady) &&
	        (!_vorbisPacket || _audiobufReady))
		_stateFlag = true;

	// same if we've run out of input
	if (_fileStream->eos())
		_stateFlag = true;

	return _surface;
}

void TheoraDecoder::reset() {
	VideoDecoder::reset();

	if (_fileStream)
		_fileStream->seek(0);

	_videobufReady = false;
	_videobufGranulePos = -1;
	_videobufTime = 0;

	_audiobufFill = 0;
	_audiobufReady = false;
	_audiobufGranulePos = 0;

	_curFrame = 0;

	_theoraPacket = 0;
	_vorbisPacket = 0;
	_stateFlag = false;
}

bool TheoraDecoder::endOfVideo() const {
	return !isVideoLoaded();
}


uint32 TheoraDecoder::getElapsedTime() const {
	if (_audStream && _mixer)
		return _mixer->getSoundElapsedTime(*_audHandle);

	return VideoDecoder::getElapsedTime();
}

Audio::QueuingAudioStream *TheoraDecoder::createAudioStream() {
	return Audio::makeQueuingAudioStream(_vorbisInfo.rate, _vorbisInfo.channels);
}

} // End of namespace Sword25

#endif
