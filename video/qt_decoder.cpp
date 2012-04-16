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
#include "video/codecs/svq1.h"

namespace Video {

////////////////////////////////////////////
// QuickTimeDecoder
////////////////////////////////////////////

QuickTimeDecoder::QuickTimeDecoder() {
	_setStartTime = false;
	_scaledSurface = 0;
	_dirtyPalette = false;
	_palette = 0;
	_width = _height = 0;
	_needUpdate = false;
}

QuickTimeDecoder::~QuickTimeDecoder() {
	close();
}

int32 QuickTimeDecoder::getCurFrame() const {
	// TODO: This is rather simplistic and doesn't take edits that
	// repeat sections of the media into account. Doing that
	// over-complicates things and shouldn't be necessary, but
	// it would be nice to have in the future.

	int32 frame = -1;

	for (uint32 i = 0; i < _handlers.size(); i++)
		if (_handlers[i]->getTrackType() == TrackHandler::kTrackTypeVideo)
			frame += ((VideoTrackHandler *)_handlers[i])->getCurFrame() + 1;

	return frame;
}

uint32 QuickTimeDecoder::getFrameCount() const {
	uint32 count = 0;

	for (uint32 i = 0; i < _handlers.size(); i++)
		if (_handlers[i]->getTrackType() == TrackHandler::kTrackTypeVideo)
			count += ((VideoTrackHandler *)_handlers[i])->getFrameCount();

	return count;
}

void QuickTimeDecoder::startAudio() {
	updateAudioBuffer();

	for (uint32 i = 0; i < _audioTracks.size(); i++) {
		g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_audioHandles[i], _audioTracks[i], -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

		// Pause the audio again if we're still paused
		if (isPaused())
			g_system->getMixer()->pauseHandle(_audioHandles[i], true);
	}
}

void QuickTimeDecoder::stopAudio() {
	for (uint32 i = 0; i < _audioHandles.size(); i++)
		g_system->getMixer()->stopHandle(_audioHandles[i]);
}

void QuickTimeDecoder::pauseVideoIntern(bool pause) {
	for (uint32 i = 0; i < _audioHandles.size(); i++)
		g_system->getMixer()->pauseHandle(_audioHandles[i], pause);
}

QuickTimeDecoder::VideoTrackHandler *QuickTimeDecoder::findNextVideoTrack() const {
	VideoTrackHandler *bestTrack = 0;
	uint32 bestTime = 0xffffffff;

	for (uint32 i = 0; i < _handlers.size(); i++) {
		if (_handlers[i]->getTrackType() == TrackHandler::kTrackTypeVideo && !_handlers[i]->endOfTrack()) {
			VideoTrackHandler *track = (VideoTrackHandler *)_handlers[i];
			uint32 time = track->getNextFrameStartTime();

			if (time < bestTime) {
				bestTime = time;
				bestTrack = track;
			}
		}
	}

	return bestTrack;
}

const Graphics::Surface *QuickTimeDecoder::decodeNextFrame() {
	if (!_nextVideoTrack)
		return 0;

	const Graphics::Surface *frame = _nextVideoTrack->decodeNextFrame();

	if (!_setStartTime) {
		_startTime = g_system->getMillis();
		_setStartTime = true;
	}

	_nextVideoTrack = findNextVideoTrack();
	_needUpdate = false;

	// Update audio buffers too
	// (needs to be done after we find the next track)
	updateAudioBuffer();

	// We have to initialize the scaled surface
	if (frame && (_scaleFactorX != 1 || _scaleFactorY != 1)) {
		if (!_scaledSurface) {
			_scaledSurface = new Graphics::Surface();
			_scaledSurface->create(_width, _height, getPixelFormat());
		}

		scaleSurface(frame, _scaledSurface, _scaleFactorX, _scaleFactorY);
		return _scaledSurface;
	}

	return frame;
}

void QuickTimeDecoder::scaleSurface(const Graphics::Surface *src, Graphics::Surface *dst, Common::Rational scaleFactorX, Common::Rational scaleFactorY) {
	assert(src && dst);

	for (int32 j = 0; j < dst->h; j++)
		for (int32 k = 0; k < dst->w; k++)
			memcpy(dst->getBasePtr(k, j), src->getBasePtr((k * scaleFactorX).toInt() , (j * scaleFactorY).toInt()), src->format.bytesPerPixel);
}

bool QuickTimeDecoder::endOfVideo() const {
	if (!isVideoLoaded())
		return true;

	for (uint32 i = 0; i < _handlers.size(); i++)
		if (!_handlers[i]->endOfTrack())
			return false;

	return true;
}

uint32 QuickTimeDecoder::getElapsedTime() const {
	// Try to base sync off an active audio track
	for (uint32 i = 0; i < _audioHandles.size(); i++) {
		if (g_system->getMixer()->isSoundHandleActive(_audioHandles[i])) {
			uint32 time = g_system->getMixer()->getSoundElapsedTime(_audioHandles[i]) + _audioStartOffset.msecs();
			if (Audio::Timestamp(time, 1000) < _audioTracks[i]->getLength())
				return time;
		}
	}

	// Just use time elapsed since the beginning
	return SeekableVideoDecoder::getElapsedTime();
}

uint32 QuickTimeDecoder::getTimeToNextFrame() const {
	if (_needUpdate)
		return 0;

	if (_nextVideoTrack) {
		uint32 nextFrameStartTime = _nextVideoTrack->getNextFrameStartTime();

		if (nextFrameStartTime == 0)
			return 0;

		// TODO: Add support for rate modification

		uint32 elapsedTime = getElapsedTime();

		if (elapsedTime < nextFrameStartTime)
			return nextFrameStartTime - elapsedTime;
	}

	return 0;
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

	_startTime = 0;
	_setStartTime = false;

	// Initialize all the audio tracks
	if (!_audioTracks.empty()) {
		_audioHandles.resize(_audioTracks.size());

		for (uint32 i = 0; i < _audioTracks.size(); i++)
			_handlers.push_back(new AudioTrackHandler(this, _audioTracks[i]));
	}

	// Initialize all the video tracks
	for (uint32 i = 0; i < _tracks.size(); i++) {
		if (_tracks[i]->codecType == CODEC_TYPE_VIDEO) {
			for (uint32 j = 0; j < _tracks[i]->sampleDescs.size(); j++)
				((VideoSampleDesc *)_tracks[i]->sampleDescs[j])->initCodec();

			_handlers.push_back(new VideoTrackHandler(this, _tracks[i]));
		}
	}

	// Prepare the first video track
	_nextVideoTrack = findNextVideoTrack();

	if (_nextVideoTrack) {
		if (_scaleFactorX != 1 || _scaleFactorY != 1) {
			// We have to take the scale into consideration when setting width/height
			_width = (_nextVideoTrack->getWidth() / _scaleFactorX).toInt();
			_height = (_nextVideoTrack->getHeight() / _scaleFactorY).toInt();
		} else {
			_width = _nextVideoTrack->getWidth().toInt();
			_height = _nextVideoTrack->getHeight().toInt();
		}

		_needUpdate = true;
	} else {
		_needUpdate = false;
	}

	// Now start any audio
	if (!_audioTracks.empty()) {
		startAudio();
		_audioStartOffset = Audio::Timestamp(0);
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
	freeAllTrackHandlers();

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = 0;
	}

	_width = _height = 0;

	Common::QuickTimeParser::close();
	SeekableVideoDecoder::reset();
}

void QuickTimeDecoder::freeAllTrackHandlers() {
	for (uint32 i = 0; i < _handlers.size(); i++)
		delete _handlers[i];

	_handlers.clear();
}

void QuickTimeDecoder::seekToTime(Audio::Timestamp time) {
	stopAudio();
	_audioStartOffset = time;

	// Sets all tracks to this time
	for (uint32 i = 0; i < _handlers.size(); i++)
		_handlers[i]->seekToTime(time);

	startAudio();

	// Reset our start time
	_startTime = g_system->getMillis() - time.msecs();
	_setStartTime = true;
	resetPauseStartTime();

	// Reset the next video track too
	_nextVideoTrack = findNextVideoTrack();
	_needUpdate = _nextVideoTrack != 0;
}

void QuickTimeDecoder::updateAudioBuffer() {
	// Updates the audio buffers for all audio tracks
	for (uint32 i = 0; i < _handlers.size(); i++)
		if (_handlers[i]->getTrackType() == TrackHandler::kTrackTypeAudio)
			((AudioTrackHandler *)_handlers[i])->updateBuffer();
}

Graphics::PixelFormat QuickTimeDecoder::getPixelFormat() const {
	if (_nextVideoTrack)
		return _nextVideoTrack->getPixelFormat();

	return Graphics::PixelFormat();
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
		_videoCodec = new SVQ1Decoder(_parentTrack->width, _parentTrack->height);
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

bool QuickTimeDecoder::endOfVideoTracks() const {
	for (uint32 i = 0; i < _handlers.size(); i++)
		if (_handlers[i]->getTrackType() == TrackHandler::kTrackTypeVideo && !_handlers[i]->endOfTrack())
			return false;

	return true;
}

QuickTimeDecoder::TrackHandler::TrackHandler(QuickTimeDecoder *decoder, Track *parent) : _decoder(decoder), _parent(parent), _fd(_decoder->_fd) {
	_curEdit = 0;
}

bool QuickTimeDecoder::TrackHandler::endOfTrack() {
	// A track is over when we've finished going through all edits
	return _curEdit == _parent->editCount;
}

QuickTimeDecoder::AudioTrackHandler::AudioTrackHandler(QuickTimeDecoder *decoder, QuickTimeAudioTrack *audioTrack)
		: TrackHandler(decoder, audioTrack->getParent()), _audioTrack(audioTrack) {
}

void QuickTimeDecoder::AudioTrackHandler::updateBuffer() {
	if (_decoder->endOfVideoTracks()) // If we have no video left (or no video), there's nothing to base our buffer against
		_audioTrack->queueRemainingAudio();
	else // Otherwise, queue enough to get us to the next frame plus another half second spare
		_audioTrack->queueAudio(Audio::Timestamp(_decoder->getTimeToNextFrame() + 500, 1000));
}

bool QuickTimeDecoder::AudioTrackHandler::endOfTrack() {
	return _audioTrack->endOfData();
}

void QuickTimeDecoder::AudioTrackHandler::seekToTime(Audio::Timestamp time) {
	_audioTrack->seek(time);
}

QuickTimeDecoder::VideoTrackHandler::VideoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent) : TrackHandler(decoder, parent) {
	enterNewEditList(false);

	_holdNextFrameStartTime = false;
	_curFrame = -1;
	_durationOverride = -1;
	_scaledSurface = 0;
}

QuickTimeDecoder::VideoTrackHandler::~VideoTrackHandler() {
	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
	}
}

const Graphics::Surface *QuickTimeDecoder::VideoTrackHandler::decodeNextFrame() {
	if (endOfTrack())
		return 0;

	const Graphics::Surface *frame = bufferNextFrame();

	if (_holdNextFrameStartTime) {
		// Don't set the next frame start time here; we just did a seek
		_holdNextFrameStartTime = false;
	} else if (_durationOverride >= 0) {
		// Use our own duration from the edit list calculation
		_nextFrameStartTime += _durationOverride;
		_durationOverride = -1;
	} else {
		_nextFrameStartTime += getFrameDuration();
	}

	// Update the edit list, if applicable
	// HACK: We're also accepting the time minus one because edit lists
	// aren't as accurate as one would hope.
	if (!endOfTrack() && getRateAdjustedFrameTime() >= getCurEditTimeOffset() + getCurEditTrackDuration() - 1) {
		_curEdit++;

		if (!endOfTrack())
			enterNewEditList(true);
	}

	if (frame && (_parent->scaleFactorX != 1 || _parent->scaleFactorY != 1)) {
		if (!_scaledSurface) {
			_scaledSurface = new Graphics::Surface();
			_scaledSurface->create(getWidth().toInt(), getHeight().toInt(), getPixelFormat());
		}

		_decoder->scaleSurface(frame, _scaledSurface, _parent->scaleFactorX, _parent->scaleFactorY);
		return _scaledSurface;
	}

	return frame;
}

void QuickTimeDecoder::VideoTrackHandler::enterNewEditList(bool bufferFrames) {
	// Bypass all empty edit lists first
	while (!endOfTrack() && _parent->editList[_curEdit].mediaTime == -1)
		_curEdit++;

	if (endOfTrack())
		return;

	uint32 frameNum = 0;
	bool done = false;
	uint32 totalDuration = 0;
	uint32 prevDuration = 0;

	// Track down where the mediaTime is in the media
	for (int32 i = 0; i < _parent->timeToSampleCount && !done; i++) {
		for (int32 j = 0; j < _parent->timeToSample[i].count; j++) {
			if (totalDuration == (uint32)_parent->editList[_curEdit].mediaTime) {
				done = true;
				prevDuration = totalDuration;
				break;
			} else if (totalDuration > (uint32)_parent->editList[_curEdit].mediaTime) {
				done = true;
				frameNum--;
				break;
			}

			prevDuration = totalDuration;
			totalDuration += _parent->timeToSample[i].duration;
			frameNum++;
		}
	}

	if (bufferFrames) {
		// Track down the keyframe
		_curFrame = findKeyFrame(frameNum) - 1;
		while (_curFrame < (int32)frameNum - 1)
			bufferNextFrame();
	} else {
		_curFrame = frameNum - 1;
	}

	_nextFrameStartTime = getCurEditTimeOffset();

	// Set an override for the duration since we came up in-between two frames
	if (prevDuration != totalDuration)
		_durationOverride = totalDuration - prevDuration;
}

const Graphics::Surface *QuickTimeDecoder::VideoTrackHandler::bufferNextFrame() {
	_curFrame++;

	// Get the next packet
	uint32 descId;
	Common::SeekableReadStream *frameData = getNextFramePacket(descId);

	if (!frameData || !descId || descId > _parent->sampleDescs.size())
		return 0;

	// Find which video description entry we want
	VideoSampleDesc *entry = (VideoSampleDesc *)_parent->sampleDescs[descId - 1];

	if (!entry->_videoCodec)
		return 0;

	const Graphics::Surface *frame = entry->_videoCodec->decodeImage(frameData);
	delete frameData;

	// Update the palette
	if (entry->_videoCodec->containsPalette()) {
		// The codec itself contains a palette
		if (entry->_videoCodec->hasDirtyPalette()) {
			_decoder->_palette = entry->_videoCodec->getPalette();
			_decoder->_dirtyPalette = true;
		}
	} else {
		// Check if the video description has been updated
		byte *palette = entry->_palette;

		if (palette !=_decoder-> _palette) {
			_decoder->_palette = palette;
			_decoder->_dirtyPalette = true;
		}
	}

	return frame;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getNextFrameStartTime() {
	if (endOfTrack())
		return 0;

	// Convert to milliseconds so the tracks can be compared
	return getRateAdjustedFrameTime() * 1000 / _parent->timeScale;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getFrameCount() {
	return _parent->frameCount;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getFrameDuration() {
	uint32 curFrameIndex = 0;
	for (int32 i = 0; i < _parent->timeToSampleCount; i++) {
		curFrameIndex += _parent->timeToSample[i].count;
		if ((uint32)_curFrame < curFrameIndex) {
			// Ok, now we have what duration this frame has.
			return _parent->timeToSample[i].duration;
		}
	}

	// This should never occur
	error("Cannot find duration for frame %d", _curFrame);
	return 0;
}

Common::SeekableReadStream *QuickTimeDecoder::VideoTrackHandler::getNextFramePacket(uint32 &descId) {
	// First, we have to track down which chunk holds the sample and which sample in the chunk contains the frame we are looking for.
	int32 totalSampleCount = 0;
	int32 sampleInChunk = 0;
	int32 actualChunk = -1;
	uint32 sampleToChunkIndex = 0;

	for (uint32 i = 0; i < _parent->chunkCount; i++) {
		if (sampleToChunkIndex < _parent->sampleToChunkCount && i >= _parent->sampleToChunk[sampleToChunkIndex].first)
			sampleToChunkIndex++;

		totalSampleCount += _parent->sampleToChunk[sampleToChunkIndex - 1].count;

		if (totalSampleCount > _curFrame) {
			actualChunk = i;
			descId = _parent->sampleToChunk[sampleToChunkIndex - 1].id;
			sampleInChunk = _parent->sampleToChunk[sampleToChunkIndex - 1].count - totalSampleCount + _curFrame;
			break;
		}
	}

	if (actualChunk < 0) {
		warning("Could not find data for frame %d", _curFrame);
		return 0;
	}

	// Next seek to that frame
	_fd->seek(_parent->chunkOffsets[actualChunk]);

	// Then, if the chunk holds more than one frame, seek to where the frame we want is located
	for (int32 i = _curFrame - sampleInChunk; i < _curFrame; i++) {
		if (_parent->sampleSize != 0)
			_fd->skip(_parent->sampleSize);
		else
			_fd->skip(_parent->sampleSizes[i]);
	}

	// Finally, read in the raw data for the frame
	//debug("Frame Data[%d]: Offset = %d, Size = %d", _curFrame, _fd->pos(), _parent->sampleSizes[_curFrame]);

	if (_parent->sampleSize != 0)
		return _fd->readStream(_parent->sampleSize);

	return _fd->readStream(_parent->sampleSizes[_curFrame]);
}

uint32 QuickTimeDecoder::VideoTrackHandler::findKeyFrame(uint32 frame) const {
	for (int i = _parent->keyframeCount - 1; i >= 0; i--)
		if (_parent->keyframes[i] <= frame)
			return _parent->keyframes[i];

	// If none found, we'll assume the requested frame is a key frame
	return frame;
}

void QuickTimeDecoder::VideoTrackHandler::seekToTime(Audio::Timestamp time) {
	// First, figure out what edit we're in
	time = time.convertToFramerate(_parent->timeScale);

	// Continue until we get to where we need to be
	for (_curEdit = 0; !endOfTrack(); _curEdit++)
		if ((uint32)time.totalNumberOfFrames() >= getCurEditTimeOffset() && (uint32)time.totalNumberOfFrames() < getCurEditTimeOffset() + getCurEditTrackDuration())
			break;

	// This track is done
	if (endOfTrack())
		return;

	enterNewEditList(false);

	// One extra check for the end of a track
	if (endOfTrack())
		return;

	// Now we're in the edit and need to figure out what frame we need
	while (getRateAdjustedFrameTime() < (uint32)time.totalNumberOfFrames()) {
		_curFrame++;
		if (_durationOverride >= 0) {
			_nextFrameStartTime += _durationOverride;
			_durationOverride = -1;
		} else {
			_nextFrameStartTime += getFrameDuration();
		}
	}

	// All that's left is to figure out what our starting time is going to be
	// Compare the starting point for the frame to where we need to be
	_holdNextFrameStartTime = getRateAdjustedFrameTime() != (uint32)time.totalNumberOfFrames();

	// If we went past the time, go back a frame
	if (_holdNextFrameStartTime)
		_curFrame--;

	// Handle the keyframe here
	int32 destinationFrame = _curFrame + 1;

	assert(destinationFrame < (int32)_parent->frameCount);
	_curFrame = findKeyFrame(destinationFrame) - 1;
	while (_curFrame < destinationFrame - 1)
		bufferNextFrame();
}

Common::Rational QuickTimeDecoder::VideoTrackHandler::getWidth() const {
	return Common::Rational(_parent->width) / _parent->scaleFactorX;
}

Common::Rational QuickTimeDecoder::VideoTrackHandler::getHeight() const {
	return Common::Rational(_parent->height) / _parent->scaleFactorY;
}

Graphics::PixelFormat QuickTimeDecoder::VideoTrackHandler::getPixelFormat() const {
	return ((VideoSampleDesc *)_parent->sampleDescs[0])->_videoCodec->getPixelFormat();
}

uint32 QuickTimeDecoder::VideoTrackHandler::getRateAdjustedFrameTime() const {
	// Figure out what time the next frame is at taking the edit list rate into account
	uint32 convertedTime =  (Common::Rational(_nextFrameStartTime - getCurEditTimeOffset()) / _parent->editList[_curEdit].mediaRate).toInt();
	return convertedTime + getCurEditTimeOffset();
}

uint32 QuickTimeDecoder::VideoTrackHandler::getCurEditTimeOffset() const {
	// Need to convert to the track scale
	return _parent->editList[_curEdit].timeOffset * _parent->timeScale / _decoder->_timeScale;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getCurEditTrackDuration() const {
	// Need to convert to the track scale
	return _parent->editList[_curEdit].trackDuration * _parent->timeScale / _decoder->_timeScale;
}

} // End of namespace Video
