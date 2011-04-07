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

//
// Partially based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "video/qt_decoder.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/util.h"

// Audio codecs
#include "audio/decoders/adpcm.h"
#include "audio/decoders/raw.h"
#include "video/codecs/qdm2.h"

// Video codecs
#include "video/codecs/cinepak.h"
#include "video/codecs/mjpeg.h"
#include "video/codecs/qtrle.h"
#include "video/codecs/rpza.h"
#include "video/codecs/smc.h"
#include "video/codecs/cdtoons.h"

namespace Video {

////////////////////////////////////////////
// QuickTimeDecoder
////////////////////////////////////////////

QuickTimeDecoder::QuickTimeDecoder() {
	_audStream = NULL;
	_curFrame = -1;
	_startTime = _nextFrameStartTime = 0;
	_audHandle = Audio::SoundHandle();
	_scaledSurface = 0;
	_dirtyPalette = false;
	_palette = 0;
}

QuickTimeDecoder::~QuickTimeDecoder() {
	close();
}

uint16 QuickTimeDecoder::getWidth() const {
	if (_videoStreamIndex < 0)
		return 0;

	return (Common::Rational(_streams[_videoStreamIndex]->width) / getScaleFactorX()).toInt();
}

uint16 QuickTimeDecoder::getHeight() const {
	if (_videoStreamIndex < 0)
		return 0;

	return (Common::Rational(_streams[_videoStreamIndex]->height) / getScaleFactorY()).toInt();
}

uint32 QuickTimeDecoder::getFrameCount() const {
	if (_videoStreamIndex < 0)
		return 0;

	return _streams[_videoStreamIndex]->nb_frames;
}

Common::Rational QuickTimeDecoder::getScaleFactorX() const {
	if (_videoStreamIndex < 0)
		return 1;

	return (_scaleFactorX * _streams[_videoStreamIndex]->scaleFactorX);
}

Common::Rational QuickTimeDecoder::getScaleFactorY() const {
	if (_videoStreamIndex < 0)
		return 1;

	return (_scaleFactorY * _streams[_videoStreamIndex]->scaleFactorY);
}

uint32 QuickTimeDecoder::getFrameDuration() {
	if (_videoStreamIndex < 0)
		return 0;

	uint32 curFrameIndex = 0;
	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count; i++) {
		curFrameIndex += _streams[_videoStreamIndex]->stts_data[i].count;
		if ((uint32)_curFrame < curFrameIndex) {
			// Ok, now we have what duration this frame has.
			return _streams[_videoStreamIndex]->stts_data[i].duration;
		}
	}

	// This should never occur
	error ("Cannot find duration for frame %d", _curFrame);
	return 0;
}

Graphics::PixelFormat QuickTimeDecoder::getPixelFormat() const {
	Codec *codec = findDefaultVideoCodec();

	if (!codec)
		return Graphics::PixelFormat::createFormatCLUT8();

	return codec->getPixelFormat();
}

uint32 QuickTimeDecoder::findKeyFrame(uint32 frame) const {
	for (int i = _streams[_videoStreamIndex]->keyframe_count - 1; i >= 0; i--)
		if (_streams[_videoStreamIndex]->keyframes[i] <= frame)
				return _streams[_videoStreamIndex]->keyframes[i];
	
	// If none found, we'll assume the requested frame is a key frame
	return frame;
}

void QuickTimeDecoder::seekToFrame(uint32 frame) {
	assert(_videoStreamIndex >= 0);
	assert(frame < _streams[_videoStreamIndex]->nb_frames);

	// Stop all audio (for now)
	stopAudio();

	// Track down the keyframe
	_curFrame = findKeyFrame(frame) - 1;
	while (_curFrame < (int32)frame - 1)
		decodeNextFrame();

	// Map out the starting point
	_nextFrameStartTime = 0;
	uint32 curFrame = 0;

	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count && curFrame < frame; i++) {
		for (int32 j = 0; j < _streams[_videoStreamIndex]->stts_data[i].count && curFrame < frame; j++) {
			curFrame++;
			_nextFrameStartTime += _streams[_videoStreamIndex]->stts_data[i].duration;
		}
	}

	// Adjust the video starting point
	const Audio::Timestamp curVideoTime(0, _nextFrameStartTime, _streams[_videoStreamIndex]->time_scale);
	_startTime = g_system->getMillis() - curVideoTime.msecs();
	resetPauseStartTime();

	// Adjust the audio starting point
	if (_audioStreamIndex >= 0) {
		_audioStartOffset = curVideoTime;

		// Re-create the audio stream
		AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];
		_audStream = Audio::makeQueuingAudioStream(entry->sampleRate, entry->channels == 2);

		// First, we need to track down what audio sample we need
		Audio::Timestamp curAudioTime(0, _streams[_audioStreamIndex]->time_scale);
		uint sample = 0;
		bool done = false;
		for (int32 i = 0; i < _streams[_audioStreamIndex]->stts_count && !done; i++) {
			for (int32 j = 0; j < _streams[_audioStreamIndex]->stts_data[i].count; j++) {
				curAudioTime = curAudioTime.addFrames(_streams[_audioStreamIndex]->stts_data[i].duration);

				if (curAudioTime > curVideoTime) {
					done = true;
					break;
				}

				sample++;
			}
		}

		// Now to track down what chunk it's in
		_curAudioChunk = 0;
		uint32 totalSamples = 0;
		for (uint32 i = 0; i < _streams[_audioStreamIndex]->chunk_count; i++, _curAudioChunk++) {
			int sampleToChunkIndex = -1;

			for (uint32 j = 0; j < _streams[_audioStreamIndex]->sample_to_chunk_sz; j++)
				if (i >= _streams[_audioStreamIndex]->sample_to_chunk[j].first)
					sampleToChunkIndex = j;

			assert(sampleToChunkIndex >= 0);

			totalSamples += _streams[_audioStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;

			if (sample < totalSamples) {
				totalSamples -= _streams[_audioStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;
				break;
			}
		}
		
		// Reposition the audio stream
		readNextAudioChunk();
		if (sample != totalSamples) {
			// HACK: Skip a certain amount of samples from the stream
			// (There's got to be a better way to do this!)
			int16 *tempBuffer = new int16[sample - totalSamples];
			_audStream->readBuffer(tempBuffer, sample - totalSamples);
			delete[] tempBuffer;
			debug(3, "Skipping %d audio samples", sample - totalSamples);
		}
		
		// Restart the audio
		startAudio();
	}
}

void QuickTimeDecoder::seekToTime(Audio::Timestamp time) {
	// TODO: Audio-only seeking (or really, have QuickTime sounds)
	if (_videoStreamIndex < 0)
		error("Audio-only seeking not supported");

	// Try to find the last frame that should have been decoded
	uint32 frame = 0;
	Audio::Timestamp totalDuration(0, _streams[_videoStreamIndex]->time_scale);
	bool done = false;

	for (int32 i = 0; i < _streams[_videoStreamIndex]->stts_count && !done; i++) {
		for (int32 j = 0; j < _streams[_videoStreamIndex]->stts_data[i].count; j++) {
			totalDuration = totalDuration.addFrames(_streams[_videoStreamIndex]->stts_data[i].duration);
			if (totalDuration > time) {
				done = true;
				break;
			}
			frame++;
		}
	}

	seekToFrame(frame);
}

Codec *QuickTimeDecoder::createCodec(uint32 codecTag, byte bitsPerPixel) {
	if (codecTag == MKID_BE('cvid')) {
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this.
		return new CinepakDecoder(bitsPerPixel);
	} else if (codecTag == MKID_BE('rpza')) {
		// Apple Video ("Road Pizza"): Used by some Myst videos.
		return new RPZADecoder(getWidth(), getHeight());
	} else if (codecTag == MKID_BE('rle ')) {
		// QuickTime RLE: Used by some Myst ME videos.
		return new QTRLEDecoder(getWidth(), getHeight(), bitsPerPixel);
	} else if (codecTag == MKID_BE('smc ')) {
		// Apple SMC: Used by some Myst videos.
		return new SMCDecoder(getWidth(), getHeight());
	} else if (codecTag == MKID_BE('SVQ1')) {
		// Sorenson Video 1: Used by some Myst ME videos.
		warning("Sorenson Video 1 not yet supported");
	} else if (codecTag == MKID_BE('SVQ3')) {
		// Sorenson Video 3: Used by some Myst ME videos.
		warning("Sorenson Video 3 not yet supported");
	} else if (codecTag == MKID_BE('jpeg')) {
		// Motion JPEG: Used by some Myst ME 10th Anniversary videos.
		return new JPEGDecoder();
	} else if (codecTag == MKID_BE('QkBk')) {
		// CDToons: Used by most of the Broderbund games.
		return new CDToonsDecoder(getWidth(), getHeight());
	} else {
		warning("Unsupported codec \'%s\'", tag2str(codecTag));
	}

	return NULL;
}

void QuickTimeDecoder::startAudio() {
	if (_audStream) { // No audio/audio not supported
		updateAudioBuffer();
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_audHandle, _audStream);
	}
}

void QuickTimeDecoder::stopAudio() {
	if (_audStream) {
		g_system->getMixer()->stopHandle(_audHandle);
		_audStream = NULL; // the mixer automatically frees the stream
	}
}

void QuickTimeDecoder::pauseVideoIntern(bool pause) {
	if (_audStream)
		g_system->getMixer()->pauseHandle(_audHandle, pause);
}

Codec *QuickTimeDecoder::findDefaultVideoCodec() const {
	if (_videoStreamIndex < 0 || _streams[_videoStreamIndex]->sampleDescs.empty())
		return 0;

	return ((VideoSampleDesc *)_streams[_videoStreamIndex]->sampleDescs[0])->videoCodec;
}

const Graphics::Surface *QuickTimeDecoder::decodeNextFrame() {
	if (_videoStreamIndex < 0 || _curFrame >= (int32)getFrameCount() - 1)
		return 0;

	if (_startTime == 0)
		_startTime = g_system->getMillis();

	_curFrame++;
	_nextFrameStartTime += getFrameDuration();

	// Update the audio while we're at it
	updateAudioBuffer();

	// Get the next packet
	uint32 descId;
	Common::SeekableReadStream *frameData = getNextFramePacket(descId);

	if (!frameData || !descId || descId > _streams[_videoStreamIndex]->sampleDescs.size())
		return 0;

	// Find which video description entry we want
	VideoSampleDesc *entry = (VideoSampleDesc *)_streams[_videoStreamIndex]->sampleDescs[descId - 1];

	if (!entry->videoCodec)
		return 0;

	const Graphics::Surface *frame = entry->videoCodec->decodeImage(frameData);
	delete frameData;

	// Update the palette
	if (entry->videoCodec->containsPalette()) {
		// The codec itself contains a palette
		if (entry->videoCodec->hasDirtyPalette()) {
			_palette = entry->videoCodec->getPalette();
			_dirtyPalette = true;
		}
	} else {
		// Check if the video description has been updated
		byte *palette = entry->palette;

		if (palette != _palette) {
			_palette = palette;
			_dirtyPalette = true;
		}
	}

	return scaleSurface(frame);
}

const Graphics::Surface *QuickTimeDecoder::scaleSurface(const Graphics::Surface *frame) {
	if (getScaleFactorX() == 1 && getScaleFactorY() == 1)
		return frame;

	assert(_scaledSurface);

	for (int32 j = 0; j < _scaledSurface->h; j++)
		for (int32 k = 0; k < _scaledSurface->w; k++)
			memcpy(_scaledSurface->getBasePtr(k, j), frame->getBasePtr((k * getScaleFactorX()).toInt() , (j * getScaleFactorY()).toInt()), frame->bytesPerPixel);

	return _scaledSurface;
}

bool QuickTimeDecoder::endOfVideo() const {
	return (!_audStream || _audStream->endOfData()) && (!findDefaultVideoCodec() || SeekableVideoDecoder::endOfVideo());
}

uint32 QuickTimeDecoder::getElapsedTime() const {
	if (_audStream)
		return g_system->getMixer()->getSoundElapsedTime(_audHandle) + _audioStartOffset.msecs();

	return SeekableVideoDecoder::getElapsedTime();
}

uint32 QuickTimeDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _curFrame < 0)
		return 0;

	// Convert from the QuickTime rate base to 1000
	uint32 nextFrameStartTime = _nextFrameStartTime * 1000 / _streams[_videoStreamIndex]->time_scale;
	uint32 elapsedTime = getElapsedTime();

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

bool QuickTimeDecoder::loadFile(const Common::String &filename) {
	if (!Common::QuickTimeParser::loadFile(filename))
		return false;

	_videoStreamIndex = _audioStreamIndex = -1;
	_startTime = 0;
	init();

	return true;
}

bool QuickTimeDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (!Common::QuickTimeParser::loadStream(stream))
		return false;

	_videoStreamIndex = _audioStreamIndex = -1;
	_startTime = 0;
	init();

	return true;
}

void QuickTimeDecoder::init() {
	Common::QuickTimeParser::init();

	// Find audio/video streams
	for (uint32 i = 0; i < _numStreams; i++) {
		if (_streams[i]->codec_type == CODEC_TYPE_VIDEO && _videoStreamIndex < 0)
			_videoStreamIndex = i;
		else if (_streams[i]->codec_type == CODEC_TYPE_AUDIO && _audioStreamIndex < 0)
			_audioStreamIndex = i;
	}

	// Initialize audio, if present
	if (_audioStreamIndex >= 0) {
		AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];

		if (checkAudioCodecSupport(entry->codecTag)) {
			_audStream = Audio::makeQueuingAudioStream(entry->sampleRate, entry->channels == 2);
			_curAudioChunk = 0;

			// Make sure the bits per sample transfers to the sample size
			if (entry->codecTag == MKID_BE('raw ') || entry->codecTag == MKID_BE('twos'))
				_streams[_audioStreamIndex]->sample_size = (entry->bitsPerSample / 8) * entry->channels;

			startAudio();
		}

		_audioStartOffset = Audio::Timestamp(0);
	}

	// Initialize video, if present
	if (_videoStreamIndex >= 0) {
		for (uint32 i = 0; i < _streams[_videoStreamIndex]->sampleDescs.size(); i++) {
			VideoSampleDesc *entry = (VideoSampleDesc *)_streams[_videoStreamIndex]->sampleDescs[i];
			entry->videoCodec = createCodec(entry->codecTag, entry->bitsPerSample & 0x1F);
		}

		if (getScaleFactorX() != 1 || getScaleFactorY() != 1) {
			// We have to initialize the scaled surface
			_scaledSurface = new Graphics::Surface();
			_scaledSurface->create(getWidth(), getHeight(), getPixelFormat().bytesPerPixel);
		}
	}
}

Common::QuickTimeParser::SampleDesc *QuickTimeDecoder::readSampleDesc(MOVStreamContext *st, uint32 format) {
	if (st->codec_type == CODEC_TYPE_VIDEO) {
		debug(0, "Video Codec FourCC: \'%s\'", tag2str(format));

		VideoSampleDesc *entry = new VideoSampleDesc();
		entry->codecTag = format;

		_fd->readUint16BE(); // version
		_fd->readUint16BE(); // revision level
		_fd->readUint32BE(); // vendor
		_fd->readUint32BE(); // temporal quality
		_fd->readUint32BE(); // spacial quality

		uint16 width = _fd->readUint16BE(); // width
		uint16 height = _fd->readUint16BE(); // height

		// The width is most likely invalid for entries after the first one
		// so only set the overall width if it is not zero here.
		if (width)
			st->width = width;

		if (height)
			st->height = height;

		_fd->readUint32BE(); // horiz resolution
		_fd->readUint32BE(); // vert resolution
		_fd->readUint32BE(); // data size, always 0
		_fd->readUint16BE(); // frames per samples

		byte codec_name[32];
		_fd->read(codec_name, 32); // codec name, pascal string (FIXME: true for mp4?)
		if (codec_name[0] <= 31) {
			memcpy(entry->codecName, &codec_name[1], codec_name[0]);
			entry->codecName[codec_name[0]] = 0;
		}

		entry->bitsPerSample = _fd->readUint16BE(); // depth
		entry->colorTableId = _fd->readUint16BE(); // colortable id

		// figure out the palette situation
		byte colorDepth = entry->bitsPerSample & 0x1F;
		bool colorGreyscale = (entry->bitsPerSample & 0x20) != 0;

		debug(0, "color depth: %d", colorDepth);

		// if the depth is 2, 4, or 8 bpp, file is palettized
		if (colorDepth == 2 || colorDepth == 4 || colorDepth == 8) {
			// Initialize the palette
			entry->palette = new byte[256 * 3];
			memset(entry->palette, 0, 256 * 3);

			if (colorGreyscale) {
				debug(0, "Greyscale palette");

				// compute the greyscale palette
				uint16 colorCount = 1 << colorDepth;
				int16 colorIndex = 255;
				byte colorDec = 256 / (colorCount - 1);
				for (byte j = 0; j < colorCount; j++) {
					entry->palette[j * 3] = entry->palette[j * 3 + 1] = entry->palette[j * 3 + 2] = colorIndex;
					colorIndex -= colorDec;
					if (colorIndex < 0)
						colorIndex = 0;
				}
			} else if (entry->colorTableId & 0x08) {
				// if flag bit 3 is set, use the default palette
				//uint16 colorCount = 1 << colorDepth;

				warning("Predefined palette! %dbpp", colorDepth);
			} else {
				debug(0, "Palette from file");

				// load the palette from the file
				uint32 colorStart = _fd->readUint32BE();
				/* uint16 colorCount = */ _fd->readUint16BE();
				uint16 colorEnd = _fd->readUint16BE();
				for (uint32 j = colorStart; j <= colorEnd; j++) {
					// each R, G, or B component is 16 bits;
					// only use the top 8 bits; skip alpha bytes
					// up front
					_fd->readByte();
					_fd->readByte();
					entry->palette[j * 3] = _fd->readByte();
					_fd->readByte();
					entry->palette[j * 3 + 1] = _fd->readByte();
					_fd->readByte();
					entry->palette[j * 3 + 2] = _fd->readByte();
					_fd->readByte();
				}
			}
		}

		return entry;
	} else if (st->codec_type == CODEC_TYPE_AUDIO) {
		debug(0, "Audio Codec FourCC: \'%s\'", tag2str(format));

		AudioSampleDesc *entry = new AudioSampleDesc();
		entry->codecTag = format;

		uint16 stsdVersion = _fd->readUint16BE();
		_fd->readUint16BE(); // revision level
		_fd->readUint32BE(); // vendor

		entry->channels = _fd->readUint16BE();			 // channel count
		entry->bitsPerSample = _fd->readUint16BE();	  // sample size

		_fd->readUint16BE(); // compression id = 0
		_fd->readUint16BE(); // packet size = 0

		entry->sampleRate = (_fd->readUint32BE() >> 16);

		debug(0, "stsd version =%d", stsdVersion);
		if (stsdVersion == 0) {
			// Not used, except in special cases. See below.
			entry->samplesPerFrame = entry->bytesPerFrame = 0;
		} else if (stsdVersion == 1) {
			// Read QT version 1 fields. In version 0 these dont exist.
			entry->samplesPerFrame = _fd->readUint32BE();
			debug(0, "stsd samples_per_frame =%d",entry->samplesPerFrame);
			_fd->readUint32BE(); // bytes per packet
			entry->bytesPerFrame = _fd->readUint32BE();
			debug(0, "stsd bytes_per_frame =%d", entry->bytesPerFrame);
			_fd->readUint32BE(); // bytes per sample
		} else {
			warning("Unsupported QuickTime STSD audio version %d", stsdVersion);
			delete entry;
			return 0;
		}

		// Version 0 videos (such as the Riven ones) don't have this set,
		// but we need it later on. Add it in here.
		if (format == MKID_BE('ima4')) {
			entry->samplesPerFrame = 64;
			entry->bytesPerFrame = 34 * entry->channels;
		}

		if (entry->sampleRate == 0 && st->time_scale > 1)
			entry->sampleRate = st->time_scale;

		return entry;
	}

	return 0;
}

void QuickTimeDecoder::close() {
	stopAudio();

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = 0;
	}

	// The audio stream is deleted automatically
	_audStream = NULL;

	Common::QuickTimeParser::close();
	SeekableVideoDecoder::reset();
}

Common::SeekableReadStream *QuickTimeDecoder::getNextFramePacket(uint32 &descId) {
	if (_videoStreamIndex < 0)
		return NULL;

	// First, we have to track down which chunk holds the sample and which sample in the chunk contains the frame we are looking for.
	int32 totalSampleCount = 0;
	int32 sampleInChunk = 0;
	int32 actualChunk = -1;

	for (uint32 i = 0; i < _streams[_videoStreamIndex]->chunk_count; i++) {
		int32 sampleToChunkIndex = -1;

		for (uint32 j = 0; j < _streams[_videoStreamIndex]->sample_to_chunk_sz; j++)
			if (i >= _streams[_videoStreamIndex]->sample_to_chunk[j].first)
				sampleToChunkIndex = j;

		if (sampleToChunkIndex < 0)
			error("This chunk (%d) is imaginary", sampleToChunkIndex);

		totalSampleCount += _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].count;

		if (totalSampleCount > getCurFrame()) {
			actualChunk = i;
			descId = _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].id;
			sampleInChunk = _streams[_videoStreamIndex]->sample_to_chunk[sampleToChunkIndex].count - totalSampleCount + getCurFrame();
			break;
		}
	}

	if (actualChunk < 0) {
		warning ("Could not find data for frame %d", getCurFrame());
		return NULL;
	}

	// Next seek to that frame
	_fd->seek(_streams[_videoStreamIndex]->chunk_offsets[actualChunk]);

	// Then, if the chunk holds more than one frame, seek to where the frame we want is located
	for (int32 i = getCurFrame() - sampleInChunk; i < getCurFrame(); i++) {
		if (_streams[_videoStreamIndex]->sample_size != 0)
			_fd->skip(_streams[_videoStreamIndex]->sample_size);
		else
			_fd->skip(_streams[_videoStreamIndex]->sample_sizes[i]);
	}

	// Finally, read in the raw data for the frame
	//printf ("Frame Data[%d]: Offset = %d, Size = %d\n", getCurFrame(), _fd->pos(), _streams[_videoStreamIndex]->sample_sizes[getCurFrame()]);

	if (_streams[_videoStreamIndex]->sample_size != 0)
		return _fd->readStream(_streams[_videoStreamIndex]->sample_size);

	return _fd->readStream(_streams[_videoStreamIndex]->sample_sizes[getCurFrame()]);
}

bool QuickTimeDecoder::checkAudioCodecSupport(uint32 tag) {
	// Check if the codec is a supported codec
	if (tag == MKID_BE('twos') || tag == MKID_BE('raw ') || tag == MKID_BE('ima4'))
		return true;

#ifdef VIDEO_CODECS_QDM2_H
	if (tag == MKID_BE('QDM2'))
		return true;
#endif

	if (tag == MKID_BE('mp4a'))
		warning("No MPEG-4 audio (AAC) support");
	else
		warning("Audio Codec Not Supported: \'%s\'", tag2str(tag));

	return false;
}

Audio::AudioStream *QuickTimeDecoder::createAudioStream(Common::SeekableReadStream *stream) {
	if (!stream || _audioStreamIndex < 0)
		return NULL;

	AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];

	if (entry->codecTag == MKID_BE('twos') || entry->codecTag == MKID_BE('raw ')) {
		// Fortunately, most of the audio used in Myst videos is raw...
		uint16 flags = 0;
		if (entry->codecTag == MKID_BE('raw '))
			flags |= Audio::FLAG_UNSIGNED;
		if (entry->channels == 2)
			flags |= Audio::FLAG_STEREO;
		if (entry->bitsPerSample == 16)
			flags |= Audio::FLAG_16BITS;
		uint32 dataSize = stream->size();
		byte *data = (byte *)malloc(dataSize);
		stream->read(data, dataSize);
		delete stream;
		return Audio::makeRawStream(data, dataSize, entry->sampleRate, flags);
	} else if (entry->codecTag == MKID_BE('ima4')) {
		// Riven uses this codec (as do some Myst ME videos)
		return Audio::makeADPCMStream(stream, DisposeAfterUse::YES, stream->size(), Audio::kADPCMApple, entry->sampleRate, entry->channels, 34);
#ifdef VIDEO_CODECS_QDM2_H
	} else if (entry->codecTag == MKID_BE('QDM2')) {
		// Several Myst ME videos use this codec
		return makeQDM2Stream(stream, _streams[_audioStreamIndex]->extradata);
#endif
	}

	error("Unsupported audio codec");

	return NULL;
}

uint32 QuickTimeDecoder::getAudioChunkSampleCount(uint chunk) {
	if (_audioStreamIndex < 0)
		return 0;

	uint32 sampleCount = 0;

	for (uint32 j = 0; j < _streams[_audioStreamIndex]->sample_to_chunk_sz; j++)
		if (chunk >= _streams[_audioStreamIndex]->sample_to_chunk[j].first)
			sampleCount = _streams[_audioStreamIndex]->sample_to_chunk[j].count;

	return sampleCount;
}

void QuickTimeDecoder::readNextAudioChunk() {
	AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];
	Common::MemoryWriteStreamDynamic *wStream = new Common::MemoryWriteStreamDynamic();

	_fd->seek(_streams[_audioStreamIndex]->chunk_offsets[_curAudioChunk]);

	// First, we have to get the sample count
	uint32 sampleCount = getAudioChunkSampleCount(_curAudioChunk);
	assert(sampleCount);

	// Then calculate the right sizes
	while (sampleCount > 0) {
		uint32 samples = 0, size = 0;

		if (entry->samplesPerFrame >= 160) {
			samples = entry->samplesPerFrame;
			size = entry->bytesPerFrame;
		} else if (entry->samplesPerFrame > 1) {
			samples = MIN<uint32>((1024 / entry->samplesPerFrame) * entry->samplesPerFrame, sampleCount);
			size = (samples / entry->samplesPerFrame) * entry->bytesPerFrame;
		} else {
			samples = MIN<uint32>(1024, sampleCount);
			size = samples * _streams[_audioStreamIndex]->sample_size;
		}

		// Now, we read in the data for this data and output it
		byte *data = (byte *)malloc(size);
		_fd->read(data, size);
		wStream->write(data, size);
		free(data);
		sampleCount -= samples;
	}

	// Now queue the buffer
	_audStream->queueAudioStream(createAudioStream(new Common::MemoryReadStream(wStream->getData(), wStream->size(), DisposeAfterUse::YES)));
	delete wStream;

	_curAudioChunk++;
}

void QuickTimeDecoder::updateAudioBuffer() {
	if (!_audStream)
		return;

	uint32 numberOfChunksNeeded = 0;

	if (_curFrame == (int32)_streams[_videoStreamIndex]->nb_frames - 1) {
		// If we're on the last frame, make sure all audio remaining is buffered
		numberOfChunksNeeded = _streams[_audioStreamIndex]->chunk_count;
	} else {
		AudioSampleDesc *entry = (AudioSampleDesc *)_streams[_audioStreamIndex]->sampleDescs[0];

		// Calculate the amount of chunks we need in memory until the next frame
		uint32 timeToNextFrame = getTimeToNextFrame();
		uint32 timeFilled = 0;
		uint32 curAudioChunk = _curAudioChunk - _audStream->numQueuedStreams();

		for (; timeFilled < timeToNextFrame && curAudioChunk < _streams[_audioStreamIndex]->chunk_count; numberOfChunksNeeded++, curAudioChunk++) {
			uint32 sampleCount = getAudioChunkSampleCount(curAudioChunk);
			assert(sampleCount);

			timeFilled += sampleCount * 1000 / entry->sampleRate;
		}

		// Add a couple extra to ensure we don't underrun
		numberOfChunksNeeded += 3;
	}

	// Keep three streams in buffer so that if/when the first two end, it goes right into the next
	while (_audStream->numQueuedStreams() < numberOfChunksNeeded && _curAudioChunk < _streams[_audioStreamIndex]->chunk_count)
		readNextAudioChunk();
}

QuickTimeDecoder::VideoSampleDesc::VideoSampleDesc() : Common::QuickTimeParser::SampleDesc() {
	memset(codecName, 0, 32);
	colorTableId = 0;
	palette = 0;
	videoCodec = 0;
}

QuickTimeDecoder::VideoSampleDesc::~VideoSampleDesc() {
	delete[] palette;
	delete videoCodec;
}

QuickTimeDecoder::AudioSampleDesc::AudioSampleDesc() : Common::QuickTimeParser::SampleDesc() {
	channels = 0;
	sampleRate = 0;
	samplesPerFrame = 0;
	bytesPerFrame = 0;
}

} // End of namespace Video
