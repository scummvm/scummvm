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

//
// Partially based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#include "video/qt_decoder.h"

#include "audio/audiostream.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

// Video codecs
#include "video/codecs/codec.h"
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
	if (_videoTrackIndex < 0)
		return 0;

	return (Common::Rational(_tracks[_videoTrackIndex]->width) / getScaleFactorX()).toInt();
}

uint16 QuickTimeDecoder::getHeight() const {
	if (_videoTrackIndex < 0)
		return 0;

	return (Common::Rational(_tracks[_videoTrackIndex]->height) / getScaleFactorY()).toInt();
}

uint32 QuickTimeDecoder::getFrameCount() const {
	if (_videoTrackIndex < 0)
		return 0;

	return _tracks[_videoTrackIndex]->frameCount;
}

Common::Rational QuickTimeDecoder::getScaleFactorX() const {
	if (_videoTrackIndex < 0)
		return 1;

	return (_scaleFactorX * _tracks[_videoTrackIndex]->scaleFactorX);
}

Common::Rational QuickTimeDecoder::getScaleFactorY() const {
	if (_videoTrackIndex < 0)
		return 1;

	return (_scaleFactorY * _tracks[_videoTrackIndex]->scaleFactorY);
}

uint32 QuickTimeDecoder::getFrameDuration() {
	if (_videoTrackIndex < 0)
		return 0;

	uint32 curFrameIndex = 0;
	for (int32 i = 0; i < _tracks[_videoTrackIndex]->timeToSampleCount; i++) {
		curFrameIndex += _tracks[_videoTrackIndex]->timeToSample[i].count;
		if ((uint32)_curFrame < curFrameIndex) {
			// Ok, now we have what duration this frame has.
			return _tracks[_videoTrackIndex]->timeToSample[i].duration;
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
	for (int i = _tracks[_videoTrackIndex]->keyframeCount - 1; i >= 0; i--)
		if (_tracks[_videoTrackIndex]->keyframes[i] <= frame)
				return _tracks[_videoTrackIndex]->keyframes[i];

	// If none found, we'll assume the requested frame is a key frame
	return frame;
}

void QuickTimeDecoder::seekToFrame(uint32 frame) {
	assert(_videoTrackIndex >= 0);
	assert(frame < _tracks[_videoTrackIndex]->frameCount);

	// Stop all audio (for now)
	stopAudio();

	// Track down the keyframe
	_curFrame = findKeyFrame(frame) - 1;
	while (_curFrame < (int32)frame - 1)
		decodeNextFrame();

	// Map out the starting point
	_nextFrameStartTime = 0;
	uint32 curFrame = 0;

	for (int32 i = 0; i < _tracks[_videoTrackIndex]->timeToSampleCount && curFrame < frame; i++) {
		for (int32 j = 0; j < _tracks[_videoTrackIndex]->timeToSample[i].count && curFrame < frame; j++) {
			curFrame++;
			_nextFrameStartTime += _tracks[_videoTrackIndex]->timeToSample[i].duration;
		}
	}

	// Adjust the video starting point
	const Audio::Timestamp curVideoTime(0, _nextFrameStartTime, _tracks[_videoTrackIndex]->timeScale);
	_startTime = g_system->getMillis() - curVideoTime.msecs();
	resetPauseStartTime();

	// Adjust the audio starting point
	if (_audioTrackIndex >= 0) {
		_audioStartOffset = curVideoTime;

		// Seek to the new audio location
		setAudioStreamPos(_audioStartOffset);

		// Restart the audio
		startAudio();
	}
}

void QuickTimeDecoder::seekToTime(Audio::Timestamp time) {
	// Use makeQuickTimeStream() instead
	if (_videoTrackIndex < 0)
		error("Audio-only seeking not supported");

	// Try to find the last frame that should have been decoded
	uint32 frame = 0;
	Audio::Timestamp totalDuration(0, _tracks[_videoTrackIndex]->timeScale);
	bool done = false;

	for (int32 i = 0; i < _tracks[_videoTrackIndex]->timeToSampleCount && !done; i++) {
		for (int32 j = 0; j < _tracks[_videoTrackIndex]->timeToSample[i].count; j++) {
			totalDuration = totalDuration.addFrames(_tracks[_videoTrackIndex]->timeToSample[i].duration);
			if (totalDuration > time) {
				done = true;
				break;
			}
			frame++;
		}
	}

	seekToFrame(frame);
}

void QuickTimeDecoder::startAudio() {
	if (_audStream) {
		updateAudioBuffer();
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_audHandle, _audStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	} // else no audio or the audio compression is not supported
}

void QuickTimeDecoder::stopAudio() {
	if (_audStream)
		g_system->getMixer()->stopHandle(_audHandle);
}

void QuickTimeDecoder::pauseVideoIntern(bool pause) {
	if (_audStream)
		g_system->getMixer()->pauseHandle(_audHandle, pause);
}

Codec *QuickTimeDecoder::findDefaultVideoCodec() const {
	if (_videoTrackIndex < 0 || _tracks[_videoTrackIndex]->sampleDescs.empty())
		return 0;

	return ((VideoSampleDesc *)_tracks[_videoTrackIndex]->sampleDescs[0])->_videoCodec;
}

const Graphics::Surface *QuickTimeDecoder::decodeNextFrame() {
	if (_videoTrackIndex < 0 || _curFrame >= (int32)getFrameCount() - 1)
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

	if (!frameData || !descId || descId > _tracks[_videoTrackIndex]->sampleDescs.size())
		return 0;

	// Find which video description entry we want
	VideoSampleDesc *entry = (VideoSampleDesc *)_tracks[_videoTrackIndex]->sampleDescs[descId - 1];

	if (!entry->_videoCodec)
		return 0;

	const Graphics::Surface *frame = entry->_videoCodec->decodeImage(frameData);
	delete frameData;

	// Update the palette
	if (entry->_videoCodec->containsPalette()) {
		// The codec itself contains a palette
		if (entry->_videoCodec->hasDirtyPalette()) {
			_palette = entry->_videoCodec->getPalette();
			_dirtyPalette = true;
		}
	} else {
		// Check if the video description has been updated
		byte *palette = entry->_palette;

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
			memcpy(_scaledSurface->getBasePtr(k, j), frame->getBasePtr((k * getScaleFactorX()).toInt() , (j * getScaleFactorY()).toInt()), frame->format.bytesPerPixel);

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
	uint32 nextFrameStartTime = _nextFrameStartTime * 1000 / _tracks[_videoTrackIndex]->timeScale;
	uint32 elapsedTime = getElapsedTime();

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

bool QuickTimeDecoder::loadFile(const Common::String &filename) {
	if (!Common::QuickTimeParser::parseFile(filename))
		return false;

	init();
	return true;
}

bool QuickTimeDecoder::loadStream(Common::SeekableReadStream *stream) {
	if (!Common::QuickTimeParser::parseStream(stream))
		return false;

	init();
	return true;
}

void QuickTimeDecoder::init() {
	Audio::QuickTimeAudioDecoder::init();

	_videoTrackIndex = -1;
	_startTime = 0;

	// Find video streams
	for (uint32 i = 0; i < _tracks.size(); i++)
		if (_tracks[i]->codecType == CODEC_TYPE_VIDEO && _videoTrackIndex < 0)
			_videoTrackIndex = i;

	// Start the audio codec if we've got one that we can handle
	if (_audStream) {
		startAudio();
		_audioStartOffset = Audio::Timestamp(0);
	}

	// Initialize video, if present
	if (_videoTrackIndex >= 0) {
		for (uint32 i = 0; i < _tracks[_videoTrackIndex]->sampleDescs.size(); i++)
			((VideoSampleDesc *)_tracks[_videoTrackIndex]->sampleDescs[i])->initCodec();

		if (getScaleFactorX() != 1 || getScaleFactorY() != 1) {
			// We have to initialize the scaled surface
			_scaledSurface = new Graphics::Surface();
			_scaledSurface->create(getWidth(), getHeight(), getPixelFormat());
		}
	}
}

Common::QuickTimeParser::SampleDesc *QuickTimeDecoder::readSampleDesc(Track *track, uint32 format) {
	if (track->codecType == CODEC_TYPE_VIDEO) {
		debug(0, "Video Codec FourCC: \'%s\'", tag2str(format));

		VideoSampleDesc *entry = new VideoSampleDesc(track, format);

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
			track->width = width;

		if (height)
			track->height = height;

		_fd->readUint32BE(); // horiz resolution
		_fd->readUint32BE(); // vert resolution
		_fd->readUint32BE(); // data size, always 0
		_fd->readUint16BE(); // frames per samples

		byte codecName[32];
		_fd->read(codecName, 32); // codec name, pascal string (FIXME: true for mp4?)
		if (codecName[0] <= 31) {
			memcpy(entry->_codecName, &codecName[1], codecName[0]);
			entry->_codecName[codecName[0]] = 0;
		}

		entry->_bitsPerSample = _fd->readUint16BE(); // depth
		entry->_colorTableId = _fd->readUint16BE(); // colortable id

		// figure out the palette situation
		byte colorDepth = entry->_bitsPerSample & 0x1F;
		bool colorGreyscale = (entry->_bitsPerSample & 0x20) != 0;

		debug(0, "color depth: %d", colorDepth);

		// if the depth is 2, 4, or 8 bpp, file is palettized
		if (colorDepth == 2 || colorDepth == 4 || colorDepth == 8) {
			// Initialize the palette
			entry->_palette = new byte[256 * 3];
			memset(entry->_palette, 0, 256 * 3);

			if (colorGreyscale) {
				debug(0, "Greyscale palette");

				// compute the greyscale palette
				uint16 colorCount = 1 << colorDepth;
				int16 colorIndex = 255;
				byte colorDec = 256 / (colorCount - 1);
				for (byte j = 0; j < colorCount; j++) {
					entry->_palette[j * 3] = entry->_palette[j * 3 + 1] = entry->_palette[j * 3 + 2] = colorIndex;
					colorIndex -= colorDec;
					if (colorIndex < 0)
						colorIndex = 0;
				}
			} else if (entry->_colorTableId & 0x08) {
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
					entry->_palette[j * 3] = _fd->readByte();
					_fd->readByte();
					entry->_palette[j * 3 + 1] = _fd->readByte();
					_fd->readByte();
					entry->_palette[j * 3 + 2] = _fd->readByte();
					_fd->readByte();
				}
			}
		}

		return entry;
	}

	// Pass it on up
	return Audio::QuickTimeAudioDecoder::readSampleDesc(track, format);
}

void QuickTimeDecoder::close() {
	stopAudio();

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = 0;
	}

	Common::QuickTimeParser::close();
	SeekableVideoDecoder::reset();
}

Common::SeekableReadStream *QuickTimeDecoder::getNextFramePacket(uint32 &descId) {
	if (_videoTrackIndex < 0)
		return NULL;

	// First, we have to track down which chunk holds the sample and which sample in the chunk contains the frame we are looking for.
	int32 totalSampleCount = 0;
	int32 sampleInChunk = 0;
	int32 actualChunk = -1;

	for (uint32 i = 0; i < _tracks[_videoTrackIndex]->chunkCount; i++) {
		int32 sampleToChunkIndex = -1;

		for (uint32 j = 0; j < _tracks[_videoTrackIndex]->sampleToChunkCount; j++)
			if (i >= _tracks[_videoTrackIndex]->sampleToChunk[j].first)
				sampleToChunkIndex = j;

		if (sampleToChunkIndex < 0)
			error("This chunk (%d) is imaginary", sampleToChunkIndex);

		totalSampleCount += _tracks[_videoTrackIndex]->sampleToChunk[sampleToChunkIndex].count;

		if (totalSampleCount > getCurFrame()) {
			actualChunk = i;
			descId = _tracks[_videoTrackIndex]->sampleToChunk[sampleToChunkIndex].id;
			sampleInChunk = _tracks[_videoTrackIndex]->sampleToChunk[sampleToChunkIndex].count - totalSampleCount + getCurFrame();
			break;
		}
	}

	if (actualChunk < 0) {
		warning ("Could not find data for frame %d", getCurFrame());
		return NULL;
	}

	// Next seek to that frame
	_fd->seek(_tracks[_videoTrackIndex]->chunkOffsets[actualChunk]);

	// Then, if the chunk holds more than one frame, seek to where the frame we want is located
	for (int32 i = getCurFrame() - sampleInChunk; i < getCurFrame(); i++) {
		if (_tracks[_videoTrackIndex]->sampleSize != 0)
			_fd->skip(_tracks[_videoTrackIndex]->sampleSize);
		else
			_fd->skip(_tracks[_videoTrackIndex]->sampleSizes[i]);
	}

	// Finally, read in the raw data for the frame
	//printf ("Frame Data[%d]: Offset = %d, Size = %d\n", getCurFrame(), _fd->pos(), _tracks[_videoTrackIndex]->sampleSizes[getCurFrame()]);

	if (_tracks[_videoTrackIndex]->sampleSize != 0)
		return _fd->readStream(_tracks[_videoTrackIndex]->sampleSize);

	return _fd->readStream(_tracks[_videoTrackIndex]->sampleSizes[getCurFrame()]);
}

void QuickTimeDecoder::updateAudioBuffer() {
	if (!_audStream)
		return;

	uint32 numberOfChunksNeeded = 0;

	if (_videoTrackIndex < 0 || _curFrame == (int32)_tracks[_videoTrackIndex]->frameCount - 1) {
		// If we have no video, there's nothing to base our buffer against
		// However, one must ask why a QuickTimeDecoder is being used instead of the nice makeQuickTimeStream() function

		// If we're on the last frame, make sure all audio remaining is buffered
		numberOfChunksNeeded = _tracks[_audioTrackIndex]->chunkCount;
	} else {
		Audio::QuickTimeAudioDecoder::AudioSampleDesc *entry = (Audio::QuickTimeAudioDecoder::AudioSampleDesc *)_tracks[_audioTrackIndex]->sampleDescs[0];

		// Calculate the amount of chunks we need in memory until the next frame
		uint32 timeToNextFrame = getTimeToNextFrame();
		uint32 timeFilled = 0;
		uint32 curAudioChunk = _curAudioChunk - _audStream->numQueuedStreams();

		for (; timeFilled < timeToNextFrame && curAudioChunk < _tracks[_audioTrackIndex]->chunkCount; numberOfChunksNeeded++, curAudioChunk++) {
			uint32 sampleCount = entry->getAudioChunkSampleCount(curAudioChunk);
			assert(sampleCount);

			timeFilled += sampleCount * 1000 / entry->_sampleRate;
		}

		// Add a couple extra to ensure we don't underrun
		numberOfChunksNeeded += 3;
	}

	// Keep three streams in buffer so that if/when the first two end, it goes right into the next
	while (_audStream->numQueuedStreams() < numberOfChunksNeeded && _curAudioChunk < _tracks[_audioTrackIndex]->chunkCount)
		queueNextAudioChunk();
}

QuickTimeDecoder::VideoSampleDesc::VideoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) : Common::QuickTimeParser::SampleDesc(parentTrack, codecTag) {
	memset(_codecName, 0, 32);
	_colorTableId = 0;
	_palette = 0;
	_videoCodec = 0;
	_bitsPerSample = 0;
}

QuickTimeDecoder::VideoSampleDesc::~VideoSampleDesc() {
	delete[] _palette;
	delete _videoCodec;
}

void QuickTimeDecoder::VideoSampleDesc::initCodec() {
	switch (_codecTag) {
	case MKTAG('c','v','i','d'):
		// Cinepak: As used by most Myst and all Riven videos as well as some Myst ME videos. "The Chief" videos also use this.
		_videoCodec = new CinepakDecoder(_bitsPerSample & 0x1f);
		break;
	case MKTAG('r','p','z','a'):
		// Apple Video ("Road Pizza"): Used by some Myst videos.
		_videoCodec = new RPZADecoder(_parentTrack->width, _parentTrack->height);
		break;
	case MKTAG('r','l','e',' '):
		// QuickTime RLE: Used by some Myst ME videos.
		_videoCodec = new QTRLEDecoder(_parentTrack->width, _parentTrack->height, _bitsPerSample & 0x1f);
		break;
	case MKTAG('s','m','c',' '):
		// Apple SMC: Used by some Myst videos.
		_videoCodec = new SMCDecoder(_parentTrack->width, _parentTrack->height);
		break;
	case MKTAG('S','V','Q','1'):
		// Sorenson Video 1: Used by some Myst ME videos.
		warning("Sorenson Video 1 not yet supported");
		break;
	case MKTAG('S','V','Q','3'):
		// Sorenson Video 3: Used by some Myst ME videos.
		warning("Sorenson Video 3 not yet supported");
		break;
	case MKTAG('j','p','e','g'):
		// Motion JPEG: Used by some Myst ME 10th Anniversary videos.
		_videoCodec = new JPEGDecoder();
		break;
	case MKTAG('Q','k','B','k'):
		// CDToons: Used by most of the Broderbund games.
		_videoCodec = new CDToonsDecoder(_parentTrack->width, _parentTrack->height);
		break;
	default:
		warning("Unsupported codec \'%s\'", tag2str(_codecTag));
	}
}

} // End of namespace Video
