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

#include "mediastation/mediastation.h"
#include "mediastation/assets/movie.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

MovieFrameHeader::MovieFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
	_index = chunk.readTypedUint32();
	debugC(5, kDebugLoading, "MovieFrameHeader::MovieFrameHeader(): _index = 0x%x (@0x%llx)", _index, static_cast<long long int>(chunk.pos()));
	_keyframeEndInMilliseconds = chunk.readTypedUint32();
}

MovieFrameFooter::MovieFrameFooter(Chunk &chunk) {
	_unk1 = chunk.readTypedUint16();
	_unk2 = chunk.readTypedUint32();
	if (g_engine->isFirstGenerationEngine()) {
		_startInMilliseconds = chunk.readTypedUint32();
		_endInMilliseconds = chunk.readTypedUint32();
		_left = chunk.readTypedUint16();
		_top = chunk.readTypedUint16();
		_unk3 = chunk.readTypedUint16();
		_unk4 = chunk.readTypedUint16();
		_index = chunk.readTypedUint16();
	} else {
		_unk4 = chunk.readTypedUint16();
		_startInMilliseconds = chunk.readTypedUint32();
		_endInMilliseconds = chunk.readTypedUint32();
		_left = chunk.readTypedUint16();
		_top = chunk.readTypedUint16();
		_zIndex = chunk.readTypedSint16();
		// This represents the difference between the left coordinate of the
		// keyframe (if applicable) and the left coordinate of this frame. Zero
		// if there is no keyframe.
		_diffBetweenKeyframeAndFrameX = chunk.readTypedSint16();
		// This represents the difference between the top coordinate of the
		// keyframe (if applicable) and the top coordinate of this frame. Zero
		// if there is no keyframe.
		_diffBetweenKeyframeAndFrameY = chunk.readTypedSint16();
		_index = chunk.readTypedUint32();
		_keyframeIndex = chunk.readTypedUint32();
		_unk9 = chunk.readTypedByte();
		debugC(5, kDebugLoading, "MovieFrameFooter::MovieFrameFooter(): _startInMilliseconds = %d, _endInMilliseconds = %d, _left = %d, _top = %d, _index = %d, _keyframeIndex = %d (@0x%llx)",
			_startInMilliseconds, _endInMilliseconds, _left, _top, _index, _keyframeIndex, static_cast<long long int>(chunk.pos()));
		debugC(5, kDebugLoading, "MovieFrameFooter::MovieFrameFooter(): _zIndex = %d, _diffBetweenKeyframeAndFrameX = %d, _diffBetweenKeyframeAndFrameY = %d, _unk4 = %d, _unk9 = %d",
			_zIndex, _diffBetweenKeyframeAndFrameX, _diffBetweenKeyframeAndFrameY, _unk4,_unk9);
	}
}

MovieFrame::MovieFrame(Chunk &chunk, MovieFrameHeader *header) :
	Bitmap(chunk, header),
	_footer(nullptr) {
	_bitmapHeader = header;
}

void MovieFrame::setFooter(MovieFrameFooter *footer) {
	if (footer != nullptr) {
		assert(footer->_index == _bitmapHeader->_index);
	}
	_footer = footer;
}

uint32 MovieFrame::left() {
	if (_footer != nullptr) {
		return _footer->_left;
	} else {
		error("MovieFrame::left(): Cannot get the left coordinate of a keyframe");
	}
}

uint32 MovieFrame::top() {
	if (_footer != nullptr) {
		return _footer->_top;
	} else {
		error("MovieFrame::left(): Cannot get the top coordinate of a keyframe");
	}
}

Common::Point MovieFrame::topLeft() {
	if (_footer != nullptr) {
		return Common::Point(_footer->_left, _footer->_top);
	} else {
		error("MovieFrame::topLeft(): Cannot get the top-left coordinate of a keyframe");
	}
}

Common::Rect MovieFrame::boundingBox() {
	if (_footer != nullptr) {
		return Common::Rect(Common::Point(_footer->_left, _footer->_top), width(), height());
	} else {
		error("MovieFrame::boundingBox(): Cannot get the bounding box of a keyframe");
	}
}

uint32 MovieFrame::index() {
	return _bitmapHeader->_index;
}

uint32 MovieFrame::startInMilliseconds() {
	if (_footer != nullptr) {
		return _footer->_startInMilliseconds;
	} else {
		error("MovieFrame::startInMilliseconds(): Cannot get the start time of a keyframe");
	}
}

uint32 MovieFrame::endInMilliseconds() {
	if (_footer != nullptr) {
		return _footer->_endInMilliseconds;
	} else {
		error("MovieFrame::endInMilliseconds(): Cannot get the end time of a keyframe");
	}
}

uint32 MovieFrame::zCoordinate() {
	if (_footer != nullptr) {
		return _footer->_zIndex;
	} else {
		error("MovieFrame::zCoordinate(): Cannot get the z-coordinate of a keyframe");
	}
}

uint32 MovieFrame::keyframeEndInMilliseconds() {
	return _bitmapHeader->_keyframeEndInMilliseconds;
}

MovieFrame::~MovieFrame() {
	// The base class destructor takes care of deleting the bitmap header, so
	// we don't need to delete that here.
	// The movie will delete the footer.
	_footer = nullptr;
}

Movie::~Movie() {
	for (MovieFrame *frame : _frames) {
		delete frame;
	}
	_frames.clear();

	for (MovieFrame *still : _stills) {
		delete still;
	}
	_stills.clear();

	for (MovieFrameFooter *footer : _footers) {
		delete footer;
	}
	_footers.clear();
}

void Movie::readParameter(Chunk &chunk, AssetHeaderSectionType paramType) {
	switch (paramType) {
	case kAssetHeaderAssetId: {
		// We already have this asset's ID, so we will just verify it is the same
		// as the ID we have already read.
		uint32 duplicateAssetId = chunk.readTypedUint16();
		if (duplicateAssetId != _id) {
			warning("Duplicate asset ID %d does not match original ID %d", duplicateAssetId, _id);
		}
		break;
	}

	case kAssetHeaderMovieLoadType:
		_loadType = chunk.readTypedByte();
		break;

	case kAssetHeaderChunkReference:
		_chunkReference = chunk.readTypedChunkReference();
		break;

	case kAssetHeaderHasOwnSubfile: {
		bool hasOwnSubfile = static_cast<bool>(chunk.readTypedByte());
		if (!hasOwnSubfile) {
			error("Movie doesn't have a subfile");
		}
		break;
	}

	case kAssetHeaderStartup:
		_isVisible = static_cast<bool>(chunk.readTypedByte());
		break;

	case kAssetHeaderDissolveFactor:
		_dissolveFactor = chunk.readTypedDouble();
		break;

	case kAssetHeaderMovieAudioChunkReference:
		_audioChunkReference = chunk.readTypedChunkReference();
		break;

	case kAssetHeaderMovieAnimationChunkReference:
		_animationChunkReference = chunk.readTypedChunkReference();
		break;

	case kAssetHeaderSoundInfo:
		_audioChunkCount = chunk.readTypedUint16();
		_audioSequence.readParameters(chunk);
		break;

	default:
		SpatialEntity::readParameter(chunk, paramType);
	}
}

ScriptValue Movie::callMethod(BuiltInMethod methodId, Common::Array<ScriptValue> &args) {
	ScriptValue returnValue;

	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.empty());
		timePlay();
		return returnValue;
	}

	case kSpatialShowMethod: {
		assert(args.empty());
		spatialShow();
		return returnValue;
	}

	case kTimeStopMethod: {
		assert(args.empty());
		timeStop();
		return returnValue;
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		spatialHide();
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

	case kSetDissolveFactorMethod: {
		warning("STUB: setDissolveFactor");
		assert(args.size() == 1);
		_dissolveFactor = args[0].asFloat();
		return returnValue;
	}

	default:
		return SpatialEntity::callMethod(methodId, args);
	}
}

void Movie::spatialShow() {
	if (_isPlaying) {
		warning("Movie::spatialShow(): (%d) Attempted to spatialShow movie that is already playing", _id);
		return;
	} else if (_stills.empty()) {
		warning("Movie::spatialShow(): (%d) No still frame to show", _id);
		return;
	}

	// TODO: For movies with keyframes, there is more than one still and they
	// must be composited. All other movies should have just one still.)
	_framesNotYetShown.clear();
	_framesOnScreen.clear();
	for (MovieFrame *still : _stills) {
		_framesOnScreen.push_back(still);
		g_engine->_dirtyRects.push_back(getFrameBoundingBox(still));
	}

	_isVisible = true;
	_isPlaying = false;
}

void Movie::spatialHide() {
	if (_isPlaying) {
		warning("Movie::spatialShow(): (%d) Attempted to spatialHide movie that is playing", _id);
		return;
	} else if (!_isVisible) {
		warning("Movie::spatialHide(): (%d) Attempted to spatialHide movie that is not showing", _id);
		return;
	}

	for (MovieFrame *frame : _framesOnScreen) {
		g_engine->_dirtyRects.push_back(getFrameBoundingBox(frame));
	}
	_framesOnScreen.clear();
	_framesNotYetShown.clear();

	_isVisible = false;
	_isPlaying = false;
}

void Movie::timePlay() {
	// TODO: Play movies one chunk at a time, which more directly approximates
	// the original's reading from the CD one chunk at a time.
	if (_isPlaying) {
		return;
	}

	// TODO: This won't work when we have some chunks that don't have audio.
	_audioSequence.play();
	_framesNotYetShown = _frames;
	_isVisible = true;
	_isPlaying = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	runEventHandlerIfExists(kMovieBeginEvent);
	process();
}

void Movie::timeStop() {
	if (!_isVisible) {
		warning("Movie::timeStop(): (%d) Attempted to stop a movie that isn't showing", _id);
		return;
	} else if (!_isPlaying) {
		warning("Movie::timePlay(): (%d) Attempted to stop a movie that isn't playing", _id);
		return;
	}

	for (MovieFrame *frame : _framesOnScreen) {
		g_engine->_dirtyRects.push_back(getFrameBoundingBox(frame));
	}
	_framesOnScreen.clear();
	_framesNotYetShown.clear();
	_audioSequence.stop();

	// Show the persistent frames.
	_isPlaying = false;
	if (!_stills.empty()) {
		for (MovieFrame *still : _stills) {
			_framesOnScreen.push_back(still);
			g_engine->_dirtyRects.push_back(getFrameBoundingBox(still));
		}
	}

	runEventHandlerIfExists(kMovieStoppedEvent);
}

void Movie::process() {
	if (_isVisible && _atFirstFrame) {
		spatialShow();
		_atFirstFrame = false;
	}

	if (_isPlaying) {
		processTimeEventHandlers();
		updateFrameState();
	}
}

void Movie::updateFrameState() {
	if (!_isPlaying) {
		debugC(6, kDebugGraphics, "Movie::updateFrameState (%d): Not playing", _id);
		for (MovieFrame *frame : _framesOnScreen) {
			debugC(6, kDebugGraphics, "   PERSIST: Frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, zIndex = %d",
				frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
		}
		return;
	}

	uint currentTime = g_system->getMillis();
	uint movieTime = currentTime - _startTime;
	debugC(5, kDebugGraphics, "Movie::updateFrameState (%d): Starting update (movie time: %d)", _id, movieTime);

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
		bool isAfterStart = movieTime >= frame->startInMilliseconds();
		if (isAfterStart) {
			_framesOnScreen.push_back(frame);
			g_engine->_dirtyRects.push_back(getFrameBoundingBox(frame));

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
		bool isAfterEnd = movieTime >= frame->endInMilliseconds();
		if (isAfterEnd) {
			g_engine->_dirtyRects.push_back(getFrameBoundingBox(frame));
			it = _framesOnScreen.erase(it);
		} else {
			++it;
		}
	}

	// Now see if we're at the end of the movie.
	if (_framesOnScreen.empty() && _framesNotYetShown.empty()) {
		_isPlaying = false;
		_framesOnScreen.clear();
		if (!_stills.empty()) {
			showPersistentFrame();
		}

		runEventHandlerIfExists(kMovieEndEvent);
		return;
	}

	// Show the frames that are currently active, for debugging purposes.
	for (MovieFrame *frame : _framesOnScreen) {
		debugC(5, kDebugGraphics, "   (time: %d ms) Frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, zIndex = %d", movieTime, frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
	}
}

void Movie::showPersistentFrame() {
	for (MovieFrame *still : _stills) {
		_framesOnScreen.push_back(still);
		g_engine->_dirtyRects.push_back(getFrameBoundingBox(still));
	}
}

void Movie::redraw(Common::Rect &rect) {
	// Make sure the frames are ordered properly before we attempt to draw them.
	Common::sort(_framesOnScreen.begin(), _framesOnScreen.end(), [](MovieFrame * a, MovieFrame * b) {
		return a->zCoordinate() > b->zCoordinate();
	});

	for (MovieFrame *frame : _framesOnScreen) {
		Common::Rect bbox = getFrameBoundingBox(frame);
		Common::Rect areaToRedraw = bbox.findIntersectingRect(rect);
		if (!areaToRedraw.isEmpty()) {
			Common::Point originOnScreen(areaToRedraw.left, areaToRedraw.top);
			areaToRedraw.translate(-frame->left() - _boundingBox.left, -frame->top() - _boundingBox.top);
			areaToRedraw.clip(Common::Rect(0, 0, frame->width(), frame->height()));
			g_engine->_screen->simpleBlitFrom(frame->_surface, areaToRedraw, originOnScreen);
		}
	}
}

Common::Rect Movie::getFrameBoundingBox(MovieFrame *frame) {
	Common::Rect bbox = frame->boundingBox();
	bbox.translate(_boundingBox.left, _boundingBox.top);
	return bbox;
}

void Movie::readChunk(Chunk &chunk) {
	// Individual chunks are "stills" and are stored in the first subfile.
	uint sectionType = chunk.readTypedUint16();
	switch ((MovieSectionType)sectionType) {
	case kMovieFrameSection: {
		debugC(5, kDebugLoading, "Movie::readStill(): Reading frame");
		MovieFrameHeader *header = new MovieFrameHeader(chunk);
		MovieFrame *frame = new MovieFrame(chunk, header);
		_stills.push_back(frame);
		break;
	}

	case kMovieFooterSection: {
		debugC(5, kDebugLoading, "Movie::readStill(): Reading footer");
		MovieFrameFooter *footer = new MovieFrameFooter(chunk);
		_footers.push_back(footer);
		break;
	}

	default:
		error("Unknown movie still section type");
	}
}

void Movie::readSubfile(Subfile &subfile, Chunk &chunk) {
	// READ THE METADATA FOR THE WHOLE MOVIE.
	uint expectedRootSectionType = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(expectedRootSectionType), static_cast<long long int>(chunk.pos()));
	if (kMovieRootSection != (MovieSectionType)expectedRootSectionType) {
		error("Expected ROOT section type, got 0x%x", expectedRootSectionType);
	}
	uint chunkCount = chunk.readTypedUint16();
	double unk1 = chunk.readTypedDouble();
	debugC(5, kDebugLoading, "Movie::readSubfile(): chunkCount = 0x%x, unk1 = %f (@0x%llx)", chunkCount, unk1, static_cast<long long int>(chunk.pos()));

	Common::Array<uint> chunkLengths;
	for (uint i = 0; i < chunkCount; i++) {
		uint chunkLength = chunk.readTypedUint32();
		debugC(5, kDebugLoading, "Movie::readSubfile(): chunkLength = 0x%x (@0x%llx)", chunkLength, static_cast<long long int>(chunk.pos()));
		chunkLengths.push_back(chunkLength);
	}

	// RAD THE INTERLEAVED AUDIO AND ANIMATION DATA.
	for (uint i = 0; i < chunkCount; i++) {
		debugC(5, kDebugLoading, "\nMovie::readSubfile(): Reading frameset %d of %d in subfile (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		chunk = subfile.nextChunk();

		// READ ALL THE FRAMES IN THIS CHUNK.
		debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading animation chunks... (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		bool isAnimationChunk = (chunk._id == _animationChunkReference);
		if (!isAnimationChunk) {
			warning("Movie::readSubfile(): (Frameset %d of %d) No animation chunks found (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		}
		while (isAnimationChunk) {
			uint sectionType = chunk.readTypedUint16();
			debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
			switch (MovieSectionType(sectionType)) {
			case kMovieFrameSection: {
				MovieFrameHeader *header = new MovieFrameHeader(chunk);
				MovieFrame *frame = new MovieFrame(chunk, header);
				_frames.push_back(frame);
				break;
			}

			case kMovieFooterSection: {
				MovieFrameFooter *footer = new MovieFrameFooter(chunk);
				_footers.push_back(footer);
				break;
			}

			default:
				error("Movie::readSubfile(): Unknown movie animation section type 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
			}

			// READ THE NEXT CHUNK.
			chunk = subfile.nextChunk();
			isAnimationChunk = (chunk._id == _animationChunkReference);
		}

		// READ THE AUDIO.
		debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading audio chunk... (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		bool isAudioChunk = (chunk._id == _audioChunkReference);
		if (isAudioChunk) {
			_audioSequence.readChunk(chunk);
			chunk = subfile.nextChunk();
		} else {
			debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) No audio chunk to read. (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		}

		// READ THE FOOTER FOR THIS SUBFILE.
		debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading header chunk... (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		bool isHeaderChunk = (chunk._id == _chunkReference);
		if (isHeaderChunk) {
			if (chunk._length != 0x04) {
				error("Movie::readSubfile(): Expected movie header chunk of size 0x04, got 0x%x (@0x%llx)", chunk._length, static_cast<long long int>(chunk.pos()));
			}
			chunk.skip(chunk._length);
		} else {
			error("Movie::readSubfile(): Expected header chunk, got %s (@0x%llx)", tag2str(chunk._id), static_cast<long long int>(chunk.pos()));
		}
	}

	// SET THE MOVIE FRAME FOOTERS.
	for (MovieFrame *frame : _stills) {
		for (MovieFrameFooter *footer : _footers) {
			if (frame->index() == footer->_index) {
				frame->setFooter(footer);
			}
		}
	}

	for (MovieFrame *frame : _frames) {
		for (MovieFrameFooter *footer : _footers) {
			if (frame->index() == footer->_index) {
				frame->setFooter(footer);
			}
		}
	}
}

} // End of namespace MediaStation
