/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "video/qt_data.h"

#include "audio/audiostream.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

// Video codecs
#include "image/codecs/codec.h"

namespace Video {

////////////////////////////////////////////
// QuickTimeDecoder
////////////////////////////////////////////

QuickTimeDecoder::QuickTimeDecoder() {
	_scaledSurface = 0;
	_width = _height = 0;
	_enableEditListBoundsCheckQuirk = false;
}

QuickTimeDecoder::~QuickTimeDecoder() {
	close();
}

bool QuickTimeDecoder::loadFile(const Common::Path &filename) {
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

void QuickTimeDecoder::close() {
	VideoDecoder::close();
	Common::QuickTimeParser::close();

	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
		_scaledSurface = 0;
	}
}

const Graphics::Surface *QuickTimeDecoder::decodeNextFrame() {
	const Graphics::Surface *frame = VideoDecoder::decodeNextFrame();

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

Common::QuickTimeParser::SampleDesc *QuickTimeDecoder::readSampleDesc(Common::QuickTimeParser::Track *track, uint32 format, uint32 descSize) {
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
			entry->_palette.clear();

			if (colorGreyscale) {
				debug(0, "Greyscale palette");

				// compute the greyscale palette
				uint16 colorCount = 1 << colorDepth;
				int16 colorIndex = 255;
				byte colorDec = 256 / (colorCount - 1);
				for (uint16 j = 0; j < colorCount; j++) {
					entry->_palette.data[j * 3] = entry->_palette.data[j * 3 + 1] = entry->_palette.data[j * 3 + 2] = colorIndex;
					colorIndex -= colorDec;
					if (colorIndex < 0)
						colorIndex = 0;
				}
			} else if (entry->_colorTableId & 0x08) {
				// if flag bit 3 is set, use the default palette
				//uint16 colorCount = 1 << colorDepth;

				debug(0, "Predefined palette! %dbpp", colorDepth);
				if (colorDepth == 2)
					entry->_palette.set(quickTimeDefaultPalette4, 0, 4);
				else if (colorDepth == 4)
					entry->_palette.set(quickTimeDefaultPalette16, 0, 16);
				else if (colorDepth == 8)
					entry->_palette.set(quickTimeDefaultPalette256, 0, 256);
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
					entry->_palette.data[j * 3] = _fd->readByte();
					_fd->readByte();
					entry->_palette.data[j * 3 + 1] = _fd->readByte();
					_fd->readByte();
					entry->_palette.data[j * 3 + 2] = _fd->readByte();
					_fd->readByte();
				}
			}

			entry->_bitsPerSample &= 0x1f; // clear grayscale bit
		}

		return entry;
	}

	// Pass it on up
	return Audio::QuickTimeAudioDecoder::readSampleDesc(track, format, descSize);
}

void QuickTimeDecoder::init() {
	Audio::QuickTimeAudioDecoder::init();

	// Initialize all the audio tracks
	for (uint32 i = 0; i < _audioTracks.size(); i++)
		addTrack(new AudioTrackHandler(this, _audioTracks[i]));

	// Initialize all the video tracks
	const Common::Array<Common::QuickTimeParser::Track *> &tracks = Common::QuickTimeParser::_tracks;
	for (uint32 i = 0; i < tracks.size(); i++) {
		if (tracks[i]->codecType == CODEC_TYPE_VIDEO) {
			for (uint32 j = 0; j < tracks[i]->sampleDescs.size(); j++)
				((VideoSampleDesc *)tracks[i]->sampleDescs[j])->initCodec();

			addTrack(new VideoTrackHandler(this, tracks[i]));
		}
	}

	// Prepare the first video track
	VideoTrackHandler *nextVideoTrack = (VideoTrackHandler *)findNextVideoTrack();

	if (nextVideoTrack) {
		if (_scaleFactorX != 1 || _scaleFactorY != 1) {
			// We have to take the scale into consideration when setting width/height
			_width = (nextVideoTrack->getScaledWidth() / _scaleFactorX).toInt();
			_height = (nextVideoTrack->getScaledHeight() / _scaleFactorY).toInt();
		} else {
			_width = nextVideoTrack->getWidth();
			_height = nextVideoTrack->getHeight();
		}
	}
}

void QuickTimeDecoder::updateAudioBuffer() {
	// Updates the audio buffers for all audio tracks
	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
		if ((*it)->getTrackType() == VideoDecoder::Track::kTrackTypeAudio)
			((AudioTrackHandler *)*it)->updateBuffer();
}

void QuickTimeDecoder::scaleSurface(const Graphics::Surface *src, Graphics::Surface *dst, const Common::Rational &scaleFactorX, const Common::Rational &scaleFactorY) {
	assert(src && dst);

	for (int32 j = 0; j < dst->h; j++)
		for (int32 k = 0; k < dst->w; k++)
			memcpy(dst->getBasePtr(k, j), src->getBasePtr((k * scaleFactorX).toInt() , (j * scaleFactorY).toInt()), src->format.bytesPerPixel);
}

QuickTimeDecoder::VideoSampleDesc::VideoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag) : Common::QuickTimeParser::SampleDesc(parentTrack, codecTag), _palette(256) {
	memset(_codecName, 0, 32);
	_colorTableId = 0;
	_videoCodec = 0;
	_bitsPerSample = 0;
}

QuickTimeDecoder::VideoSampleDesc::~VideoSampleDesc() {
	delete _videoCodec;
}

void QuickTimeDecoder::VideoSampleDesc::initCodec() {
	_videoCodec = Image::createQuickTimeCodec(_codecTag, _parentTrack->width, _parentTrack->height, _bitsPerSample);
}

QuickTimeDecoder::AudioTrackHandler::AudioTrackHandler(QuickTimeDecoder *decoder, QuickTimeAudioTrack *audioTrack) :
		SeekableAudioTrack(decoder->getSoundType()),
		_decoder(decoder),
		_audioTrack(audioTrack) {
}

void QuickTimeDecoder::AudioTrackHandler::updateBuffer() {
	if (_decoder->endOfVideoTracks()) // If we have no video left (or no video), there's nothing to base our buffer against
		_audioTrack->queueRemainingAudio();
	else // Otherwise, queue enough to get us to the next frame plus another half second spare
		_audioTrack->queueAudio(Audio::Timestamp(_decoder->getTimeToNextFrame() + 500, 1000));
}

Audio::SeekableAudioStream *QuickTimeDecoder::AudioTrackHandler::getSeekableAudioStream() const {
	return _audioTrack;
}

QuickTimeDecoder::VideoTrackHandler::VideoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent) : _decoder(decoder), _parent(parent) {
	if (decoder->_enableEditListBoundsCheckQuirk) {
		checkEditListBounds();
	}

	_curEdit = 0;
	_curFrame = -1;
	_delayedFrameToBufferTo = -1;
	enterNewEditListEntry(true, true); // might set _curFrame

	_durationOverride = -1;
	_scaledSurface = 0;
	_curPalette = 0;
	_dirtyPalette = false;
	_reversed = false;
	_forcedDitherPalette = 0;
	_ditherTable = 0;
	_ditherFrame = 0;
}

// FIXME: This check breaks valid QuickTime movies, such as the KQ6 Mac opening.
// It doesn't take media rate into account and mixes up units that are in movie
// time scale and media time scale, which is easy to do since they're often the
// same value. Other decoder bugs have been fixed since this was written, so it
// would be good to re-evaluate what the problem was with the Riven Spanish video.
// It's now disabled for everything except Riven.
void QuickTimeDecoder::VideoTrackHandler::checkEditListBounds() {
	// Check all the edit list entries are within the bounds of the media
	// In the Spanish version of Riven, the last edit of the video ogk.mov
	// ends one frame after the end of the media.

	uint32 offset = 0;
	uint32 mediaDuration = _parent->mediaDuration * _decoder->_timeScale / _parent->timeScale;

	for (uint i = 0; i < _parent->editList.size(); i++) {
		EditListEntry &edit = _parent->editList[i];

		if (edit.mediaTime < 0) {
			offset += edit.trackDuration;
			continue; // Ignore empty edits
		}

		if ((uint32) edit.mediaTime > mediaDuration) {
			// Check if the edit starts after the end of the media
			// If so, mark it as empty so it is ignored
			edit.mediaTime = -1;
		} else if (edit.mediaTime + edit.trackDuration > mediaDuration) {
			// Check if the edit ends after the end of the media
			// If so, clip it so it fits in the media
			edit.trackDuration = mediaDuration - edit.mediaTime;
		}

		edit.timeOffset = offset;
		offset += edit.trackDuration;
	}
}

QuickTimeDecoder::VideoTrackHandler::~VideoTrackHandler() {
	if (_scaledSurface) {
		_scaledSurface->free();
		delete _scaledSurface;
	}

	delete _forcedDitherPalette;
	delete[] _ditherTable;

	if (_ditherFrame) {
		_ditherFrame->free();
		delete _ditherFrame;
	}
}

bool QuickTimeDecoder::VideoTrackHandler::endOfTrack() const {
	// A track is over when we've finished going through all edits
	return _reversed ? (_curEdit == 0 && _curFrame < 0) : atLastEdit();
}

bool QuickTimeDecoder::VideoTrackHandler::seek(const Audio::Timestamp &requestedTime) {
	_delayedFrameToBufferTo = -1; // abort any delayed buffering

	uint32 convertedFrames = requestedTime.convertToFramerate(_decoder->_timeScale).totalNumberOfFrames();
	for (_curEdit = 0; !atLastEdit(); _curEdit++)
		if (convertedFrames >= _parent->editList[_curEdit].timeOffset && convertedFrames < _parent->editList[_curEdit].timeOffset + _parent->editList[_curEdit].trackDuration)
			break;

	// If we did reach the end of the track, break out
	if (atLastEdit()) {
		// Call setReverse to set the position to the last frame of the last edit
		if (_reversed)
			setReverse(true);
		return true;
	}

	// If this track is in an empty edit, position us at the next non-empty
	// edit. There's nothing else to do after this.
	if (_parent->editList[_curEdit].mediaTime == -1) {
		while (!atLastEdit() && _parent->editList[_curEdit].mediaTime == -1)
			_curEdit++;

		if (!atLastEdit())
			enterNewEditListEntry(true);

		return true;
	}

	enterNewEditListEntry(false);

	// One extra check for the end of a track
	if (atLastEdit()) {
		// Call setReverse to set the position to the last frame of the last edit
		if (_reversed)
			setReverse(true);
		return true;
	}

	// Now we're in the edit and need to figure out what frame we need
	Audio::Timestamp time = requestedTime.convertToFramerate(_parent->timeScale);
	while (getRateAdjustedFrameTime() < (uint32)time.totalNumberOfFrames()) {
		_curFrame++;
		if (_durationOverride >= 0) {
			_nextFrameStartTime += _durationOverride;
			_durationOverride = -1;
		} else {
			_nextFrameStartTime += getCurFrameDuration();
		}
	}

	// Check if we went past, then adjust the frame times
	if (getRateAdjustedFrameTime() != (uint32)time.totalNumberOfFrames()) {
		_curFrame--;
		_durationOverride = getRateAdjustedFrameTime() - time.totalNumberOfFrames();
		_nextFrameStartTime = time.totalNumberOfFrames();
	}

	if (_reversed) {
		// Call setReverse again to update
		setReverse(true);
	} else {
		// Handle the keyframe here
		int32 destinationFrame = _curFrame + 1;

		assert(destinationFrame < (int32)_parent->frameCount);
		_curFrame = findKeyFrame(destinationFrame) - 1;
		while (_curFrame < destinationFrame - 1)
			bufferNextFrame();
	}

	return true;
}

Audio::Timestamp QuickTimeDecoder::VideoTrackHandler::getDuration() const {
	return Audio::Timestamp(0, _parent->duration, _decoder->_timeScale);
}

uint16 QuickTimeDecoder::VideoTrackHandler::getWidth() const {
	return getScaledWidth().toInt();
}

uint16 QuickTimeDecoder::VideoTrackHandler::getHeight() const {
	return getScaledHeight().toInt();
}

Graphics::PixelFormat QuickTimeDecoder::VideoTrackHandler::getPixelFormat() const {
	if (_forcedDitherPalette)
		return Graphics::PixelFormat::createFormatCLUT8();

	return ((VideoSampleDesc *)_parent->sampleDescs[0])->_videoCodec->getPixelFormat();
}

bool QuickTimeDecoder::VideoTrackHandler::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (_forcedDitherPalette)
		return false;

	return ((VideoSampleDesc *)_parent->sampleDescs[0])->_videoCodec->setOutputPixelFormat(format);
}

int QuickTimeDecoder::VideoTrackHandler::getFrameCount() const {
	return _parent->frameCount;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getNextFrameStartTime() const {
	if (endOfTrack())
		return 0;

	Audio::Timestamp frameTime(0, getRateAdjustedFrameTime(), _parent->timeScale);

	// Check if the frame goes beyond the end of the edit. In that case, the next frame
	// should really be when we cross the edit boundary.
	if (_reversed) {
		Audio::Timestamp editStartTime(0, _parent->editList[_curEdit].timeOffset, _decoder->_timeScale);
		if (frameTime < editStartTime)
			return editStartTime.msecs();
	} else {
		Audio::Timestamp nextEditStartTime(0, _parent->editList[_curEdit].timeOffset + _parent->editList[_curEdit].trackDuration, _decoder->_timeScale);
		if (frameTime > nextEditStartTime)
			return nextEditStartTime.msecs();
	}

	// Not past an edit boundary, so the frame time is what should be used
	return frameTime.msecs();
}

const Graphics::Surface *QuickTimeDecoder::VideoTrackHandler::decodeNextFrame() {
	if (endOfTrack())
		return 0;

	if (_reversed) {
		// Subtract one to place us on the frame before the current displayed frame.
		_curFrame--;

		// We have one "dummy" frame at the end to so the last frame is displayed
		// for the right amount of time.
		if (_curFrame < 0)
			return 0;

		// Decode from the last key frame to the frame before the one we need.
		// TODO: Probably would be wise to do some caching
		int targetFrame = _curFrame;
		_curFrame = findKeyFrame(targetFrame) - 1;
		while (_curFrame != targetFrame - 1)
			bufferNextFrame();
	}

	// Update the edit list, if applicable
	// FIXME: Add support for playing backwards videos with more than one edit
	// For now, stay on the first edit for reversed playback
	if (endOfCurEdit() && !_reversed) {
		_curEdit++;

		if (atLastEdit())
			return 0;

		enterNewEditListEntry(true);

		if (isEmptyEdit())
			return 0;
	}

	const Graphics::Surface *frame = bufferNextFrame();

	if (_reversed) {
		if (_durationOverride >= 0) {
			// Use our own duration overridden from a media seek
			_nextFrameStartTime -= _durationOverride;
			_durationOverride = -1;
		} else {
			// Just need to subtract the time
			_nextFrameStartTime -= getCurFrameDuration();
		}
	} else {
		if (_durationOverride >= 0) {
			// Use our own duration overridden from a media seek
 			_nextFrameStartTime += _durationOverride;
			_durationOverride = -1;
		} else {
			_nextFrameStartTime += getCurFrameDuration();
		}
	}

	// Handle forced dithering
	if (frame && _forcedDitherPalette)
		frame = forceDither(*frame);

	if (frame && (_parent->scaleFactorX != 1 || _parent->scaleFactorY != 1)) {
		if (!_scaledSurface) {
			_scaledSurface = new Graphics::Surface();
			_scaledSurface->create(getScaledWidth().toInt(), getScaledHeight().toInt(), getPixelFormat());
		}

		_decoder->scaleSurface(frame, _scaledSurface, _parent->scaleFactorX, _parent->scaleFactorY);
		return _scaledSurface;
	}

	return frame;
}

Common::String QuickTimeDecoder::getAliasPath() {
	const Common::Array<Common::QuickTimeParser::Track *> &tracks = Common::QuickTimeParser::_tracks;
	for (uint32 i = 0; i < tracks.size(); i++) {
		if (!tracks[i]->path.empty())
			return tracks[i]->path;
	}
	return Common::String();
}

Audio::Timestamp QuickTimeDecoder::VideoTrackHandler::getFrameTime(uint frame) const {
	// TODO: This probably doesn't work right with edit lists
	int cumulativeDuration = 0;
	for (int ttsIndex = 0; ttsIndex < _parent->timeToSampleCount; ttsIndex++) {
		const TimeToSampleEntry &tts = _parent->timeToSample[ttsIndex];
		if ((int)frame < tts.count)
			return Audio::Timestamp(0, _parent->timeScale).addFrames(cumulativeDuration + frame * tts.duration);
		else {
			frame -= tts.count;
			cumulativeDuration += tts.duration * tts.count;
		}
	}

	return Audio::Timestamp().addFrames(-1);
}

const byte *QuickTimeDecoder::VideoTrackHandler::getPalette() const {
	_dirtyPalette = false;
	return _forcedDitherPalette ? _forcedDitherPalette->data : _curPalette;
}

bool QuickTimeDecoder::VideoTrackHandler::setReverse(bool reverse) {
	_delayedFrameToBufferTo = -1; // abort any delayed buffering

	_reversed = reverse;

	if (_reversed) {
		if (_parent->editList.size() != 1) {
			// TODO: Myst's holo.mov needs this :(
			warning("Can only set reverse without edits");
			return false;
		}

		if (atLastEdit()) {
			// If we're at the end of the video, go to the penultimate edit.
			// The current frame is set to one beyond the last frame here;
			// one "past" the currently displayed frame.
			_curEdit = _parent->editList.size() - 1;
			_curFrame = _parent->frameCount;
			_nextFrameStartTime = _parent->editList[_curEdit].trackDuration + _parent->editList[_curEdit].timeOffset;
		} else if (_durationOverride >= 0) {
			// We just had a media seek, so "pivot" around the frame that should
			// be displayed.
			_curFrame += 2;
			_nextFrameStartTime += _durationOverride;
		} else {
			// We need to put _curFrame to be the one after the one that should be displayed.
			// Since we're on the frame that should be displaying right now, add one.
			_curFrame++;
		}
	} else {
		// Update the edit list, if applicable
		if (!atLastEdit() && endOfCurEdit()) {
			_curEdit++;

			if (atLastEdit())
				return true;
		}

		if (_durationOverride >= 0) {
			// We just had a media seek, so "pivot" around the frame that should
			// be displayed.
			_curFrame--;
			_nextFrameStartTime -= _durationOverride;
 		}

		// We need to put _curFrame to be the one before the one that should be displayed.
		// Since we're on the frame that should be displaying right now, subtract one.
		// (As long as the current frame isn't -1, of course)
		if (_curFrame > 0) {
			// We then need to handle the keyframe situation
			int targetFrame = _curFrame - 1;
			_curFrame = findKeyFrame(targetFrame) - 1;
			while (_curFrame < targetFrame)
				bufferNextFrame();
		} else if (_curFrame == 0) {
			// Make us start at the first frame (no keyframe needed)
			_curFrame--;
		}
	}

	return true;
}

Common::Rational QuickTimeDecoder::VideoTrackHandler::getScaledWidth() const {
	return Common::Rational(_parent->width) / _parent->scaleFactorX;
}

Common::Rational QuickTimeDecoder::VideoTrackHandler::getScaledHeight() const {
	return Common::Rational(_parent->height) / _parent->scaleFactorY;
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

	if (actualChunk < 0)
		error("Could not find data for frame %d", _curFrame);

	// Next seek to that frame
	Common::SeekableReadStream *stream = _decoder->_fd;
	stream->seek(_parent->chunkOffsets[actualChunk]);

	// Then, if the chunk holds more than one frame, seek to where the frame we want is located
	for (int32 i = _curFrame - sampleInChunk; i < _curFrame; i++) {
		if (_parent->sampleSize != 0)
			stream->skip(_parent->sampleSize);
		else
			stream->skip(_parent->sampleSizes[i]);
	}

	// Finally, read in the raw data for the frame
	//debug("Frame Data[%d]: Offset = %d, Size = %d", _curFrame, stream->pos(), _parent->sampleSizes[_curFrame]);

	if (_parent->sampleSize != 0)
		return stream->readStream(_parent->sampleSize);

	return stream->readStream(_parent->sampleSizes[_curFrame]);
}

uint32 QuickTimeDecoder::VideoTrackHandler::getCurFrameDuration() {
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

uint32 QuickTimeDecoder::VideoTrackHandler::findKeyFrame(uint32 frame) const {
	for (int i = _parent->keyframeCount - 1; i >= 0; i--)
		if (_parent->keyframes[i] <= frame)
			return _parent->keyframes[i];

	// If none found, we'll assume the requested frame is a key frame
	return frame;
}

bool QuickTimeDecoder::VideoTrackHandler::isEmptyEdit() const {
	return (_parent->editList[_curEdit].mediaTime == -1);
}

void QuickTimeDecoder::VideoTrackHandler::enterNewEditListEntry(bool bufferFrames, bool initializingTrack) {
	if (atLastEdit())
		return;

	// if this is an empty edit then the only thing to do is set the
	// time for the next frame, which is the duration of this edit.
	if (isEmptyEdit()) {
		_curFrame = -1;
		_nextFrameStartTime = getCurEditTimeOffset() + getCurEditTrackDuration();
		return;
	}

	uint32 mediaTime = _parent->editList[_curEdit].mediaTime;
	uint32 frameNum = 0;
	uint32 totalDuration = 0;
	_durationOverride = -1;

	// Track down where the mediaTime is in the media
	// This is basically time -> frame mapping
	// Note that this code uses first frame = 0
	for (int32 i = 0; i < _parent->timeToSampleCount; i++) {
		uint32 duration = _parent->timeToSample[i].count * _parent->timeToSample[i].duration;

		if (totalDuration + duration >= mediaTime) {
			uint32 frameInc = (mediaTime - totalDuration) / _parent->timeToSample[i].duration;
			frameNum += frameInc;
			totalDuration += frameInc * _parent->timeToSample[i].duration;

			// If we didn't get to the exact media time, mark an override for
			// the time.
			if (totalDuration != mediaTime)
				_durationOverride = totalDuration + _parent->timeToSample[i].duration - mediaTime;

			break;
		}

		frameNum += _parent->timeToSample[i].count;
		totalDuration += duration;
	}

	if (bufferFrames) {
		// Track down the keyframe
		// Then decode until the frame before target
		_curFrame = findKeyFrame(frameNum) - 1;
		if (initializingTrack) {
			// We can't decode frames during track initialization,
			// so delay buffering until the first decode.
			_delayedFrameToBufferTo = (int32)frameNum - 1;
		} else {
			while (_curFrame < (int32)frameNum - 1) {
				bufferNextFrame();
			}
		}
	} else {
		// Since frameNum is the frame that needs to be displayed
		// we'll set _curFrame to be the "last frame displayed"
		_curFrame = frameNum - 1;
	}

	_nextFrameStartTime = getCurEditTimeOffset();
}

const Graphics::Surface *QuickTimeDecoder::VideoTrackHandler::bufferNextFrame() {
	// Buffer any frames that were identified during track initialization
	// and delayed until decoding.
	if (_delayedFrameToBufferTo != -1) {
		int32 frameNum = _delayedFrameToBufferTo;
		_delayedFrameToBufferTo = -1;
		while (_curFrame < frameNum) {
			bufferNextFrame();
		}
	}

	_curFrame++;

	// Get the next packet
	uint32 descId;
	Common::SeekableReadStream *frameData = getNextFramePacket(descId);

	if (!frameData || !descId || descId > _parent->sampleDescs.size()) {
		delete frameData;
		return 0;
	}

	// Find which video description entry we want
	VideoSampleDesc *entry = (VideoSampleDesc *)_parent->sampleDescs[descId - 1];

	if (!entry->_videoCodec) {
		delete frameData;
		return 0;
	}

	const Graphics::Surface *frame = entry->_videoCodec->decodeFrame(*frameData);
	delete frameData;

	// Update the palette
	if (entry->_videoCodec->containsPalette()) {
		// The codec itself contains a palette
		if (entry->_videoCodec->hasDirtyPalette()) {
			_curPalette = entry->_videoCodec->getPalette();
			_dirtyPalette = true;
		}
	} else {
		// Check if the video description has been updated
		byte *palette = entry->_palette.data;

		if (palette != _curPalette) {
			_curPalette = palette;
			_dirtyPalette = true;
		}
	}

	return frame;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getRateAdjustedFrameTime() const {
	// Figure out what time the next frame is at taking the edit list rate into account,
	// unless this is an empty edit, in which case the rate isn't applicable.
	Common::Rational offsetFromEdit = Common::Rational(_nextFrameStartTime - getCurEditTimeOffset());
	if (!isEmptyEdit()) {
		offsetFromEdit /= _parent->editList[_curEdit].mediaRate;
	}
	uint32 convertedTime = offsetFromEdit.toInt();

	if ((offsetFromEdit.getNumerator() % offsetFromEdit.getDenominator()) > (offsetFromEdit.getDenominator() / 2))
		convertedTime++;

	return convertedTime + getCurEditTimeOffset();
}

uint32 QuickTimeDecoder::VideoTrackHandler::getCurEditTimeOffset() const {
	// Need to convert to the track scale

	// We have to round the time off to the nearest in the scale, otherwise
	// bad things happen. QuickTime docs are pretty silent on all this stuff,
	// so this was found from samples. It doesn't help that this is really
	// the only open source implementation of QuickTime edits.

	uint32 mult = _parent->editList[_curEdit].timeOffset * _parent->timeScale;
	uint32 result = mult / _decoder->_timeScale;

	if ((mult % _decoder->_timeScale) > (_decoder->_timeScale / 2))
		result++;

	return result;
}

uint32 QuickTimeDecoder::VideoTrackHandler::getCurEditTrackDuration() const {
	// convert from movie time scale to the track's media time scale
	return _parent->editList[_curEdit].trackDuration * _parent->timeScale / _decoder->_timeScale;
}

bool QuickTimeDecoder::VideoTrackHandler::atLastEdit() const {
	return _curEdit == _parent->editList.size();
}

bool QuickTimeDecoder::VideoTrackHandler::endOfCurEdit() const {
	// We're at the end of the edit once the next frame's time would
	// bring us past the end of the edit.
	return getRateAdjustedFrameTime() >= getCurEditTimeOffset() + getCurEditTrackDuration();
}

bool QuickTimeDecoder::VideoTrackHandler::canDither() const {
	for (uint i = 0; i < _parent->sampleDescs.size(); i++) {
		VideoSampleDesc *desc = (VideoSampleDesc *)_parent->sampleDescs[i];

		if (!desc || !desc->_videoCodec)
			return false;
	}

	return true;
}

void QuickTimeDecoder::VideoTrackHandler::setDither(const byte *palette) {
	assert(canDither());

	for (uint i = 0; i < _parent->sampleDescs.size(); i++) {
		VideoSampleDesc *desc = (VideoSampleDesc *)_parent->sampleDescs[i];

		if (desc->_videoCodec->canDither(Image::Codec::kDitherTypeQT)) {
			// Codec dither
			desc->_videoCodec->setDither(Image::Codec::kDitherTypeQT, palette);
		} else {
			// Forced dither
			_forcedDitherPalette = new Graphics::Palette(256);
			_forcedDitherPalette->set(palette, 0, 256);
			_ditherTable = Image::Codec::createQuickTimeDitherTable(_forcedDitherPalette->data, 256);
			_dirtyPalette = true;
		}
	}
}

namespace {

// Return a pixel in RGB554
uint16 makeDitherColor(byte r, byte g, byte b) {
	return ((r & 0xF8) << 6) | ((g & 0xF8) << 1) | (b >> 4);
}

// Default template to convert a dither color
template<typename PixelInt>
inline uint16 readDitherColor(PixelInt srcColor, const Graphics::PixelFormat& format, const byte *palette) {
	byte r, g, b;
	format.colorToRGB(srcColor, r, g, b);
	return makeDitherColor(r, g, b);
}

// Specialized version for 8bpp
template<>
inline uint16 readDitherColor(byte srcColor, const Graphics::PixelFormat& format, const byte *palette) {
	return makeDitherColor(palette[srcColor * 3], palette[srcColor * 3 + 1], palette[srcColor * 3 + 2]);
}

template<typename PixelInt>
void ditherFrame(const Graphics::Surface &src, Graphics::Surface &dst, const byte *ditherTable, const byte *palette = 0) {
	static const uint16 colorTableOffsets[] = { 0x0000, 0xC000, 0x4000, 0x8000 };

	for (int y = 0; y < dst.h; y++) {
		const PixelInt *srcPtr = (const PixelInt *)src.getBasePtr(0, y);
		byte *dstPtr = (byte *)dst.getBasePtr(0, y);
		uint16 colorTableOffset = colorTableOffsets[y & 3];

		for (int x = 0; x < dst.w; x++) {
			uint16 color = readDitherColor(*srcPtr++, src.format, palette);
			*dstPtr++ = ditherTable[colorTableOffset + color];
			colorTableOffset += 0x4000;
		}
	}
}

} // End of anonymous namespace

const Graphics::Surface *QuickTimeDecoder::VideoTrackHandler::forceDither(const Graphics::Surface &frame) {
	if (frame.format.bytesPerPixel == 1) {
		// This should always be true, but this is for sanity
		if (!_curPalette)
			return &frame;

		// If the palettes match, bail out
		if (_forcedDitherPalette && memcmp(_forcedDitherPalette->data, _curPalette, 256 * 3) == 0)
			return &frame;
	}

	// Need to create a new one
	if (!_ditherFrame) {
		_ditherFrame = new Graphics::Surface();
		_ditherFrame->create(frame.w, frame.h, Graphics::PixelFormat::createFormatCLUT8());
	}

	if (frame.format.bytesPerPixel == 1)
		ditherFrame<byte>(frame, *_ditherFrame, _ditherTable, _curPalette);
	else if (frame.format.bytesPerPixel == 2)
		ditherFrame<uint16>(frame, *_ditherFrame, _ditherTable);
	else if (frame.format.bytesPerPixel == 4)
		ditherFrame<uint32>(frame, *_ditherFrame, _ditherTable);

	return _ditherFrame;
}

} // End of namespace Video
