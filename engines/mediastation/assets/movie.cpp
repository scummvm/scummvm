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

#include "audio/decoders/raw.h"
#include "audio/decoders/adpcm.h"

#include "mediastation/mediastation.h"
#include "mediastation/assets/movie.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

MovieFrameHeader::MovieFrameHeader(Chunk &chunk) : BitmapHeader(chunk) {
	_index = Datum(chunk).u.i;
	debugC(5, kDebugLoading, "MovieFrameHeader::MovieFrameHeader(): _index = 0x%x (@0x%llx)", _index, static_cast<long long int>(chunk.pos()));
	_keyframeEndInMilliseconds = Datum(chunk).u.i;
}

MovieFrameFooter::MovieFrameFooter(Chunk &chunk) {
	_unk1 = Datum(chunk).u.i;
	_unk2 = Datum(chunk).u.i;
	if (g_engine->isFirstGenerationEngine()) { // TODO: Add the version number check.
		_startInMilliseconds = Datum(chunk).u.i;
		_endInMilliseconds = Datum(chunk).u.i;
		_left = Datum(chunk).u.i;
		_top = Datum(chunk).u.i;
		_unk3 = Datum(chunk).u.i;
		_unk4 = Datum(chunk).u.i;
		_index = Datum(chunk).u.i;
	} else {
		_unk4 = Datum(chunk).u.i;
		_startInMilliseconds = Datum(chunk).u.i;
		_endInMilliseconds = Datum(chunk).u.i;
		_left = Datum(chunk).u.i;
		_top = Datum(chunk).u.i;
		_zIndex = Datum(chunk).u.i;
		// This represents the difference between the left coordinate of the
		// keyframe (if applicable) and the left coordinate of this frame. Zero
		// if there is no keyframe.
		_diffBetweenKeyframeAndFrameX = Datum(chunk).u.i;
		// This represents the difference between the top coordinate of the
		// keyframe (if applicable) and the top coordinate of this frame. Zero
		// if there is no keyframe.
		_diffBetweenKeyframeAndFrameY = Datum(chunk).u.i;
		_index = Datum(chunk).u.i;
		_keyframeIndex = Datum(chunk).u.i;
		_unk9 = Datum(chunk).u.i;
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

Movie::Movie(AssetHeader *header) : Asset(header) {
	if (header->_startup == kAssetStartupActive) {
		setActive();
		_showByDefault = true;
	}
}

Movie::~Movie() {
	g_engine->_mixer->stopHandle(_soundHandle);

	for (MovieFrame *frame : _frames) {
		delete frame;
	}
	_frames.clear();

	for (MovieFrame *still : _stills) {
		delete still;
	}
	_stills.clear();

	for (Audio::SeekableAudioStream *stream : _audioStreams) {
		delete stream;
	}
	_audioStreams.clear();

	for (MovieFrameFooter *footer : _footers) {
		delete footer;
	}
	_footers.clear();
}

Operand Movie::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kTimePlayMethod: {
		assert(args.empty());
		timePlay();
		return Operand();
	}

	case kSpatialShowMethod: {
		assert(args.empty());
		spatialShow();
		return Operand();
	}

	case kTimeStopMethod: {
		assert(args.empty());
		timeStop();
		return Operand();
	}

	case kSpatialHideMethod: {
		assert(args.empty());
		spatialHide();
		return Operand();
	}

	case kIsPlayingMethod: {
		assert(args.empty());
		Operand returnValue(kOperandTypeLiteral1);
		returnValue.putInteger(_isPlaying);
		return returnValue;
	}

	default: {
		error("Movie::callMethod(): Got unimplemented method ID %d", methodId);
	}
	}
}

void Movie::spatialShow() {
	if (_isPlaying) {
		warning("Movie::spatialShow(): (%d) Attempted to spatialShow movie that is already showing", _header->_id);
		return;
	} else if (_isShowing) {
		warning("Movie::spatialShow(): (%d) Attempted to spatialShow movie that is already showing", _header->_id);
		return;
	} else if (_stills.empty()) {
		warning("Movie::spatialShow(): (%d) No still frame to show", _header->_id);
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

	setActive();
	_isShowing = true;
	_isPlaying = false;
}

void Movie::spatialHide() {
	if (_isPlaying) {
		warning("Movie::spatialShow(): (%d) Attempted to spatialHide movie that is playing", _header->_id);
		return;
	} else if (!_isShowing) {
		warning("Movie::spatialHide(): (%d) Attempted to spatialHide movie that is not showing", _header->_id);
		return;
	}

	for (MovieFrame *frame : _framesOnScreen) {
		g_engine->_dirtyRects.push_back(getFrameBoundingBox(frame));
	}
	_framesOnScreen.clear();
	_framesNotYetShown.clear();

	_isShowing = false;
	_isPlaying = false;
	setInactive();
}

void Movie::timePlay() {
	// TODO: Play movies one chunk at a time, which more directly approximates
	// the original's reading from the CD one chunk at a time.
	if (_isPlaying) {
		warning("Movie::timePlay(): (%d) Attempted to play a movie that is already playing", _header->_id);
		return;
	}

	// START PLAYING SOUND.
	// TODO: This won't work when we have some chunks that don't have audio.
	if (!_audioStreams.empty()) {
		Audio::QueuingAudioStream *audio = Audio::makeQueuingAudioStream(22050, false);
		for (Audio::SeekableAudioStream *stream : _audioStreams) {
			stream->rewind();
			audio->queueAudioStream(stream, DisposeAfterUse::NO);
		}
		// Then play the audio!
		g_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audio, -1, Audio::Mixer::kMaxChannelVolume);
		audio->finish();
	}

	_framesNotYetShown = _frames;
	_isShowing = true;
	_isPlaying = true;
	setActive();
	runEventHandlerIfExists(kMovieBeginEvent);
}

void Movie::timeStop() {
	if (!_isShowing) {
		warning("Movie::timeStop(): (%d) Attempted to stop a movie that isn't showing", _header->_id);
		return;
	} else if (!_isPlaying) {
		warning("Movie::timePlay(): (%d) Attempted to stop a movie that isn't playing", _header->_id);
		return;
	}

	for (MovieFrame *frame : _framesOnScreen) {
		g_engine->_dirtyRects.push_back(getFrameBoundingBox(frame));
	}
	_framesOnScreen.clear();
	_framesNotYetShown.clear();

	g_engine->_mixer->stopHandle(_soundHandle);
	_soundHandle = Audio::SoundHandle();

	// Show the persistent frames.
	_isPlaying = false;
	if (!_stills.empty()) {
		for (MovieFrame *still : _stills) {
			_framesOnScreen.push_back(still);
			g_engine->_dirtyRects.push_back(getFrameBoundingBox(still));
		}
	} else {
		setInactive();
	}

	runEventHandlerIfExists(kMovieStoppedEvent);
}

void Movie::process() {
	processTimeEventHandlers();
	updateFrameState();
}

void Movie::updateFrameState() {
	if (_showByDefault) {
		spatialShow();
		_showByDefault = false;
	}

	if (!_isPlaying) {
		debugC(6, kDebugGraphics, "Movie::updateFrameState (%d): Not playing", _header->_id);
		for (MovieFrame *frame : _framesOnScreen) {
			debugC(6, kDebugGraphics, "   PERSIST: Frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, zIndex = %d", 
				frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
		}
		return;
	}

	uint currentTime = g_system->getMillis();
	uint movieTime = currentTime - _startTime;
	debugC(5, kDebugGraphics, "Movie::updateFrameState (%d): Starting update (movie time: %d)", _header->_id, movieTime);
	if (_framesNotYetShown.empty()) {
		_isPlaying = false;
		setInactive();
		_framesOnScreen.clear();
		runEventHandlerIfExists(kMovieEndEvent);
		return;
	}
	
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

	// Show the frames that are currently active, for debugging purposes.
	for (MovieFrame *frame : _framesOnScreen) {
		debugC(5, kDebugGraphics, "   (time: %d ms) Frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, zIndex = %d", movieTime, frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
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
			areaToRedraw.translate(-frame->left() - _header->_boundingBox->left, -frame->top() - _header->_boundingBox->top);
			g_engine->_screen->simpleBlitFrom(frame->_surface, areaToRedraw, originOnScreen);
		}
	}
}

Common::Rect Movie::getFrameBoundingBox(MovieFrame *frame) {
	Common::Rect bbox = frame->boundingBox();
	bbox.translate(_header->_boundingBox->left, _header->_boundingBox->top);
	return bbox;
}

void Movie::readChunk(Chunk &chunk) {
	// Individual chunks are "stills" and are stored in the first subfile.
	uint sectionType = Datum(chunk).u.i;
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

	default: {
		error("Unknown movie still section type");
	}
	}
}

void Movie::readSubfile(Subfile &subfile, Chunk &chunk) {
	// READ THE METADATA FOR THE WHOLE MOVIE.
	uint expectedRootSectionType = Datum(chunk).u.i;
	debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(expectedRootSectionType), static_cast<long long int>(chunk.pos()));
	if (kMovieRootSection != (MovieSectionType)expectedRootSectionType) {
		error("Expected ROOT section type, got 0x%x", expectedRootSectionType);
	}
	uint chunkCount = Datum(chunk).u.i;
	debugC(5, kDebugLoading, "Movie::readSubfile(): chunkCount = 0x%x (@0x%llx)", chunkCount, static_cast<long long int>(chunk.pos()));

	uint dataStartOffset = Datum(chunk).u.i;
	debugC(5, kDebugLoading, "Movie::readSubfile(): dataStartOffset = 0x%x (@0x%llx)", dataStartOffset, static_cast<long long int>(chunk.pos()));

	Common::Array<uint> chunkLengths;
	for (uint i = 0; i < chunkCount; i++) {
		uint chunkLength = Datum(chunk).u.i;
		debugC(5, kDebugLoading, "Movie::readSubfile(): chunkLength = 0x%x (@0x%llx)", chunkLength, static_cast<long long int>(chunk.pos()));
		chunkLengths.push_back(chunkLength);
	}

	// RAD THE INTERLEAVED AUDIO AND ANIMATION DATA.
	for (uint i = 0; i < chunkCount; i++) {
		debugC(5, kDebugLoading, "\nMovie::readSubfile(): Reading frameset %d of %d in subfile (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		chunk = subfile.nextChunk();

		// READ ALL THE FRAMES IN THIS CHUNK.
		debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading animation chunks... (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		bool isAnimationChunk = (chunk._id == _header->_animationChunkReference);
		if (!isAnimationChunk) {
			warning("Movie::readSubfile(): (Frameset %d of %d) No animation chunks found (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		}
		while (isAnimationChunk) {
			uint sectionType = Datum(chunk).u.i;
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

			default: {
				error("Movie::readSubfile(): Unknown movie animation section type 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
			}
			}

			// READ THE NEXT CHUNK.
			chunk = subfile.nextChunk();
			isAnimationChunk = (chunk._id == _header->_animationChunkReference);
		}

		// READ THE AUDIO.
		debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading audio chunk... (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		bool isAudioChunk = (chunk._id == _header->_audioChunkReference);
		if (isAudioChunk) {
			byte *buffer = (byte *)malloc(chunk._length);
			chunk.read((void *)buffer, chunk._length);
			Audio::SeekableAudioStream *stream = nullptr;
			switch (_header->_soundEncoding) {
			case SoundEncoding::PCM_S16LE_MONO_22050:
				stream = Audio::makeRawStream(buffer, chunk._length, 22050, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
				break;

			case SoundEncoding::IMA_ADPCM_S16LE_MONO_22050:
				error("Movie::readSubfile(): ADPCM decoding not supported for movies");
				break;

			default:
				error("Movie::readSubfile(): Unknown audio encoding 0x%x", static_cast<uint>(_header->_soundEncoding));
			}
			_audioStreams.push_back(stream);
			chunk = subfile.nextChunk();
		} else {
			debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) No audio chunk to read. (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		}

		// READ THE FOOTER FOR THIS SUBFILE.
		debugC(5, kDebugLoading, "Movie::readSubfile(): (Frameset %d of %d) Reading header chunk... (@0x%llx)", i, chunkCount, static_cast<long long int>(chunk.pos()));
		bool isHeaderChunk = (chunk._id == _header->_chunkReference);
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
