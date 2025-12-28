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

#include "mediastation/actors/movie.h"
#include "mediastation/debugchannels.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

MovieFrameHeader::MovieFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
	_index = chunk.readTypedUint32();
	debugC(5, kDebugLoading, "MovieFrameHeader::MovieFrameHeader(): _index = 0x%x (@0x%llx)", _index, static_cast<long long int>(chunk.pos()));
	_keyframeEndInMilliseconds = chunk.readTypedUint32();
}

MovieFrame::MovieFrame(Chunk &chunk) {
	if (g_engine->isFirstGenerationEngine()) {
		blitType = static_cast<MovieBlitType>(chunk.readTypedUint16());
		startInMilliseconds = chunk.readTypedUint32();
		endInMilliseconds = chunk.readTypedUint32();
		// These are unsigned in the data files but ScummVM expects signed.
		leftTop.x = static_cast<int16>(chunk.readTypedUint16());
		leftTop.y = static_cast<int16>(chunk.readTypedUint16());
		index = chunk.readTypedUint32();
		keyframeIndex = chunk.readTypedUint32();
		keepAfterEnd = chunk.readTypedByte();
	} else {
		layerId = chunk.readTypedUint32();
		blitType = static_cast<MovieBlitType>(chunk.readTypedUint16());
		startInMilliseconds = chunk.readTypedUint32();
		endInMilliseconds = chunk.readTypedUint32();
		// These are unsigned in the data files but ScummVM expects signed.
		leftTop.x = static_cast<int16>(chunk.readTypedUint16());
		leftTop.y = static_cast<int16>(chunk.readTypedUint16());
		zIndex = chunk.readTypedSint16();
		// This represents the difference between the left-top coordinate of the
		// keyframe (if applicable) and the left coordinate of this frame. Zero
		// if there is no keyframe.
		diffBetweenKeyframeAndFrame.x = chunk.readTypedSint16();
		diffBetweenKeyframeAndFrame.y = chunk.readTypedSint16();
		index = chunk.readTypedUint32();
		keyframeIndex = chunk.readTypedUint32();
		keepAfterEnd = chunk.readTypedByte();
		debugC(5, kDebugLoading, "MovieFrame::MovieFrame(): _blitType = %d, _startInMilliseconds = %d, \
			_endInMilliseconds = %d, _left = %d, _top = %d, _zIndex = %d, _diffBetweenKeyframeAndFrameX = %d, \
			_diffBetweenKeyframeAndFrameY = %d, _index = %d, _keyframeIndex = %d, _keepAfterEnd = %d (@0x%llx)",
			blitType, startInMilliseconds, endInMilliseconds, leftTop.x, leftTop.y, zIndex, diffBetweenKeyframeAndFrame.x, \
			diffBetweenKeyframeAndFrame.y, index, keyframeIndex, keepAfterEnd, static_cast<long long int>(chunk.pos()));
	}
}

MovieFrameImage::MovieFrameImage(Chunk &chunk, MovieFrameHeader *header) : Bitmap(chunk, header) {
	_bitmapHeader = header;
}

MovieFrameImage::~MovieFrameImage() {
	// The base class destructor takes care of deleting the bitmap header, so
	// we don't need to delete that here.
}

StreamMovieActor::~StreamMovieActor() {
	unregisterWithStreamManager();
	if (_streamFeed != nullptr) {
		g_engine->getStreamFeedManager()->closeStreamFeed(_streamFeed);
		_streamFeed = nullptr;
	}

	delete _streamFrames;
	_streamFrames = nullptr;

	delete _streamSound;
	_streamSound = nullptr;
}

void StreamMovieActor::readParameter(Chunk &chunk, ActorHeaderSectionType paramType) {
	switch (paramType) {
	case kActorHeaderActorId: {
		// We already have this actor's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateActorId = chunk.readTypedUint16();
		if (duplicateActorId != _id) {
			warning("%s: Duplicate actor ID %d does not match original ID %d", __func__, duplicateActorId, _id);
		}
		break;
	}

	case kActorHeaderMovieLoadType:
		_loadType = chunk.readTypedByte();
		break;

	case kActorHeaderChannelIdent:
		_channelIdent = chunk.readTypedChannelIdent();
		registerWithStreamManager();
		break;

	case kActorHeaderHasOwnSubfile: {
		bool hasOwnSubfile = static_cast<bool>(chunk.readTypedByte());
		if (!hasOwnSubfile) {
			error("%s: StreamMovieActor doesn't have a subfile", __func__);
		}
		break;
	}

	case kActorHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kActorHeaderMovieAudioChannelIdent: {
		ChannelIdent soundChannelIdent = chunk.readTypedChannelIdent();
		_streamSound->setChannelIdent(soundChannelIdent);
		_streamSound->registerWithStreamManager();
		break;
	}

	case kActorHeaderMovieAnimationChannelIdent: {
		ChannelIdent framesChannelIdent = chunk.readTypedChannelIdent();
		_streamFrames->setChannelIdent(framesChannelIdent);
		_streamFrames->registerWithStreamManager();
		break;
	}

	case kActorHeaderSoundInfo:
		_streamSound->_audioSequence.readParameters(chunk);
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue StreamMovieActor::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.empty());
		timePlay();
		return returnValue;
	}

	case kSpatialShowMethod: {
		assert(args.empty());
		setVisibility(true);
		updateFrameState();
		return returnValue;
	}

	case kTimeStopMethod: {
		assert(args.empty());
		timeStop();
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		setVisibility(false);
		return returnValue;
	}

	case kIsPlayingMethod: {
		assert(args.empty());
		returnValue.setToBool(_isPlaying);
		return returnValue;
	}

	case kGetLeftXMethod: {
		assert(args.empty());
		double left = static_cast<double>(_boundingBox.left);
		returnValue.setToFloat(left);
		return returnValue;
	}

	case kGetTopYMethod: {
		assert(args.empty());
		double top = static_cast<double>(_boundingBox.top);
		returnValue.setToFloat(top);
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

void StreamMovieActor::timePlay() {
	if (_streamFeed == nullptr) {
		_streamFeed = g_engine->getStreamFeedManager()->openStreamFeed(_id);
		_streamFeed->readData();
	}

	if (_isPlaying) {
		return;
	}

	_streamSound->_audioSequence.play();
	_framesNotYetShown = _streamFrames->_frames;
	_framesOnScreen.clear();
	_isPlaying = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	runEventHandlerIfExists(kMovieBeginEvent);
	process();
}

void StreamMovieActor::timeStop() {
	if (!_isPlaying) {
		return;
	}

	for (MovieFrame *frame : _framesOnScreen) {
		invalidateRect(getFrameBoundingBox(frame));
	}
	_streamSound->_audioSequence.stop();
	_framesNotYetShown.empty();
	if (_hasStill) {
		_framesNotYetShown = _streamFrames->_frames;
	}
	_framesOnScreen.clear();
	_startTime = 0;
	_lastProcessedTime = 0;
	_isPlaying = false;
	runEventHandlerIfExists(kMovieStoppedEvent);
}

void StreamMovieActor::process() {
	if (_isVisible) {
		if (_isPlaying) {
			processTimeEventHandlers();
		}
		updateFrameState();
	}
}

void StreamMovieActor::setVisibility(bool visibility) {
	if (visibility != _isVisible) {
		_isVisible = visibility;
		invalidateLocalBounds();
	}
}

void StreamMovieActor::updateFrameState() {
	uint movieTime = 0;
	if (_isPlaying) {
		uint currentTime = g_system->getMillis();
		movieTime = currentTime - _startTime;
	}
	debugC(5, kDebugGraphics, "StreamMovieActor::updateFrameState (%d): Starting update (movie time: %d)", _id, movieTime);

	// This complexity is necessary becuase movies can have more than one frame
	// showing at the same time - for instance, a movie background and an
	// animation on that background are a part of the saem movie and are on
	// screen at the same time, it's just the starting and ending times of one
	// can be different from the starting and ending times of another.
	//
	// We can rely on the frames being ordered in order of their start. First,
	// see if there are any new frames to show.
	for (auto it = _framesNotYetShown.begin(); it != _framesNotYetShown.end();) {
		MovieFrame *frame = *it;
		bool isAfterStart = movieTime >= frame->startInMilliseconds;
		if (isAfterStart) {
			_framesOnScreen.insert(frame);
			invalidateRect(getFrameBoundingBox(frame));

			// We don't need ++it because we will either have another frame
			// that needs to be drawn, or we have reached the end of the new
			// frames.
			it = _framesNotYetShown.erase(it);
		} else {
			// We've hit a frame that shouldn't yet be shown.
			// Rely on the ordering to not bother with any further frames.
			break;
		}
	}

	// Now see if there are any old frames that no longer need to be shown.
	for (auto it = _framesOnScreen.begin(); it != _framesOnScreen.end();) {
		MovieFrame *frame = *it;
		bool isAfterEnd = movieTime >= frame->endInMilliseconds;
		if (isAfterEnd) {
			invalidateRect(getFrameBoundingBox(frame));
			it = _framesOnScreen.erase(it);

			if (_framesOnScreen.empty() && movieTime >= _fullTime) {
				_isPlaying = false;
				if (_hasStill) {
					_framesNotYetShown = _streamFrames->_frames;
					updateFrameState();
				}
				runEventHandlerIfExists(kMovieEndEvent);
				break;
			}
		} else {
			++it;
		}
	}

	// Show the frames that are currently active, for debugging purposes.
	for (MovieFrame *frame : _framesOnScreen) {
		debugC(5, kDebugGraphics, "   (time: %d ms) Frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, zIndex = %d", \
			movieTime, frame->index, frame->image->width(), frame->image->height(), frame->leftTop.x, frame->leftTop.y, frame->startInMilliseconds, frame->endInMilliseconds, frame->zIndex);
	}
}

void StreamMovieActor::draw(DisplayContext &displayContext) {
	for (MovieFrame *frame : _framesOnScreen) {
		Common::Rect bbox = getFrameBoundingBox(frame);

		switch (frame->blitType) {
		case kUncompressedMovieBlit:
			g_engine->getDisplayManager()->imageBlit(bbox.origin(), frame->image, _dissolveFactor, &displayContext);
			break;

		case kUncompressedDeltaMovieBlit:
			g_engine->getDisplayManager()->imageDeltaBlit(
				bbox.origin(), frame->diffBetweenKeyframeAndFrame,
				frame->image, frame->keyframeImage, _dissolveFactor, &displayContext);
			break;

		case kCompressedDeltaMovieBlit:
			if (frame->keyframeImage->isCompressed()) {
				decompressIntoAuxImage(frame);
			}
			g_engine->getDisplayManager()->imageDeltaBlit(
				bbox.origin(), frame->diffBetweenKeyframeAndFrame,
				frame->image, frame->keyframeImage, _dissolveFactor, &displayContext);
			break;

		default:
			error("%s: Got unknown movie frame blit type: %d", __func__, frame->blitType);
		}
	}
}

Common::Rect StreamMovieActor::getFrameBoundingBox(MovieFrame *frame) {
	// Use _boundingBox directly (which may be temporarily offset by camera rendering)
	// The camera offset is already applied to _boundingBox by pushBoundingBoxOffset()
	Common::Point origin = _boundingBox.origin() + frame->leftTop;
	Common::Rect bbox = Common::Rect(origin, frame->image->width(), frame->image->height());
	return bbox;
}

StreamMovieActorFrames::~StreamMovieActorFrames() {
	unregisterWithStreamManager();

	for (MovieFrame *frame : _frames) {
		delete frame;
	}
	_frames.clear();

	for (MovieFrameImage *image : _images) {
		delete image;
	}
	_images.clear();
}

void StreamMovieActorFrames::readChunk(Chunk &chunk) {
	uint sectionType = chunk.readTypedUint16();
	switch ((MovieSectionType)sectionType) {
	case kMovieImageDataSection:
		readImageData(chunk);
		break;

	case kMovieFrameDataSection:
		readFrameData(chunk);
		break;

	default:
		error("%s: Unknown movie still section type", __func__);
	}

	for (MovieFrame *frame : _frames) {
		if (frame->endInMilliseconds > _parent->_fullTime) {
			_parent->_fullTime = frame->endInMilliseconds;
		}
		if (frame->keepAfterEnd) {
			_parent->_hasStill = true;
		}
	}

	if (_parent->_hasStill) {
		_parent->_framesNotYetShown = _frames;
	}
}

StreamMovieActorSound::~StreamMovieActorSound() {
	unregisterWithStreamManager();
}

void StreamMovieActorSound::readChunk(Chunk &chunk) {
	_audioSequence.readChunk(chunk);
}

StreamMovieActor::StreamMovieActor() : _framesOnScreen(StreamMovieActor::compareFramesByZIndex), SpatialEntity(kActorTypeMovie) {
	_streamFrames = new StreamMovieActorFrames(this);
	_streamSound = new StreamMovieActorSound();
}

void StreamMovieActor::readChunk(Chunk &chunk) {
	MovieSectionType sectionType = static_cast<MovieSectionType>(chunk.readTypedUint16());
	if (sectionType == kMovieRootSection) {
		parseMovieHeader(chunk);
	} else if (sectionType == kMovieChunkMarkerSection) {
		parseMovieChunkMarker(chunk);
	} else {
		error("%s: Got unused movie chunk header section", __func__);
	}
}

void StreamMovieActor::parseMovieHeader(Chunk &chunk) {
	_chunkCount = chunk.readTypedUint16();
	_frameRate = chunk.readTypedDouble();
	debugC(5, kDebugLoading, "%s: chunkCount = 0x%x, frameRate = %f (@0x%llx)", __func__, _chunkCount, _frameRate, static_cast<long long int>(chunk.pos()));

	Common::Array<uint> chunkLengths;
	for (uint i = 0; i < _chunkCount; i++) {
		uint chunkLength = chunk.readTypedUint32();
		debugC(5, kDebugLoading, "StreamMovieActor::readSubfile(): chunkLength = 0x%x (@0x%llx)", chunkLength, static_cast<long long int>(chunk.pos()));
		chunkLengths.push_back(chunkLength);
	}
}

void StreamMovieActor::parseMovieChunkMarker(Chunk &chunk) {
	// TODO: There is no warning here because that would spam with thousands of warnings.
	// This takes care of scheduling stream load and such - it doesn't actually read from the
	// chunk that is passed in. Since we don't need that scheduling since we are currently reading
	// the whole movie at once rather than streaming it from the CD-ROM, we don't currently need
	// to do much here anyway.
}

void StreamMovieActor::invalidateRect(const Common::Rect &rect) {
	invalidateLocalBounds();
}

void StreamMovieActor::decompressIntoAuxImage(MovieFrame *frame) {
	const Common::Point origin(0, 0);
	frame->keyframeImage->_image.create(frame->keyframeImage->width(), frame->keyframeImage->height(), Graphics::PixelFormat::createFormatCLUT8());
	frame->keyframeImage->_image.setTransparentColor(0);
	g_engine->getDisplayManager()->imageBlit(origin, frame->keyframeImage, 1.0, nullptr, &frame->keyframeImage->_image);
}

void StreamMovieActorFrames::readImageData(Chunk &chunk) {
	MovieFrameHeader *header = new MovieFrameHeader(chunk);
	MovieFrameImage *frame = new MovieFrameImage(chunk, header);
	_images.push_back(frame);
}

void StreamMovieActorFrames::readFrameData(Chunk &chunk) {
	uint frameDataToRead = chunk.readTypedUint16();
	for (uint i = 0; i < frameDataToRead; i++) {
		MovieFrame *frame = new MovieFrame(chunk);

		// We cannot use a hashmap here because multiple frames can have the
		// same index, and frames are not necessarily in index order. So we'll
		// do a linear search, which is how the original does it.
		for (MovieFrameImage *image : _images) {
			if (image->index() == frame->index) {
				frame->image = image;
				break;
			}
		}

		if (frame->keyframeIndex != 0) {
			for (MovieFrameImage *image : _images) {
				if (image->index() == frame->keyframeIndex) {
					frame->keyframeImage = image;
					break;
				}
			}
		}

		_frames.push_back(frame);
	}
}

int StreamMovieActor::compareFramesByZIndex(const MovieFrame *a, const MovieFrame *b) {
	if (b->zIndex > a->zIndex) {
		return 1;
	} else if (a->zIndex > b->zIndex) {
		return -1;
	} else {
		return 0;
	}
}

} // End of namespace MediaStation
