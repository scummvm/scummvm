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
	_footer(nullptr),
	_showing(false) {
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
	if (_footer != nullptr) {
		return _footer->_index;
	} else {
		error("MovieFrame::index(): Cannot get the index of a keyframe");
	}
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
	delete _footer;
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

	default: {
		error("Got unimplemented method ID %d", methodId);
	}
	}
}

void Movie::timePlay() {
	debugC(5, kDebugScript, "Called Movie::timePlay()");
	// TODO: Play movies one chunk at a time, which more directly approximates
	// the original's reading from the CD one chunk at a time.
	if (_isActive) {
		error("Movie::play(): Attempted to play a movie that is already playing");
		return;
	}

	// SET ANIMATION VARIABLES.
	_isActive = true;
	_startTime = g_system->getMillis();
	_lastProcessedTime = 0;
	g_engine->addPlayingAsset(this);

	// GET THE DURATION OF THE MOVIE.
	_duration = 0;
	for (MovieFrame *frame : _frames) {
		if (frame->endInMilliseconds() > _duration) {
			_duration = frame->endInMilliseconds();
		}
	}

	// START PLAYING SOUND.
	// TODO: This won't work when we have some chunks that don't have audio.
	if (!_audioStreams.empty()) {
		Audio::QueuingAudioStream *audio = Audio::makeQueuingAudioStream(22050, false);
		for (Audio::SeekableAudioStream *stream : _audioStreams) {
			audio->queueAudioStream(stream, DisposeAfterUse::NO);
		}
		// Then play the audio!
		Audio::SoundHandle handle;
		g_engine->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, audio, -1, Audio::Mixer::kMaxChannelVolume);
	}

	runEventHandlerIfExists(kMovieBeginEvent);
}

void Movie::timeStop() {
	// RESET ANIMATION VARIABLES.
	_isActive = false;
	_startTime = 0;
	_lastProcessedTime = 0;

	runEventHandlerIfExists(kMovieStoppedEvent);
}

void Movie::process() {
	processTimeEventHandlers();
	drawNextFrame();
}

void Movie::processTimeEventHandlers() {
	if (!_isActive) {
		warning("Movie::processTimeEventHandlers(): Attempted to process time event handlers while movie is not playing");
		return;
	}

	uint currentTime = g_system->getMillis();
	for (EventHandler *timeEvent : _header->_timeHandlers) {
		double timeEventInFractionalSeconds = timeEvent->_argumentValue.u.f;
		uint timeEventInMilliseconds = timeEventInFractionalSeconds * 1000;
		bool timeEventAlreadyProcessed = timeEventInMilliseconds < _lastProcessedTime;
		bool timeEventNeedsToBeProcessed = timeEventInMilliseconds <= currentTime - _startTime;
		if (!timeEventAlreadyProcessed && timeEventNeedsToBeProcessed) {
			debugC(5, kDebugScript, "Movie::processTimeEventHandlers(): Running On Time handler for movie time %d ms (real movie time: %d ms)", timeEventInMilliseconds, currentTime - _startTime);
			timeEvent->execute(_header->_id);
		}
	}
	_lastProcessedTime = currentTime - _startTime;
}

bool Movie::drawNextFrame() {
	// TODO: We'll need to support persistent frames in movies too. Do movies
	// have the same distinction between spatialShow and timePlay that sprites
	// do?
	
	uint currentTime = g_system->getMillis();
	uint movieTime = currentTime - _startTime;
	debugC(5, kDebugGraphics, "GRAPHICS (Movie %d): Starting blitting (movie time: %d)", _header->_id, movieTime);
	bool donePlaying = movieTime > _duration;
	if (donePlaying) {
		_isActive = false;
		_startTime = 0;
		_lastProcessedTime = 0;

		runEventHandlerIfExists(kMovieEndEvent);
		return false;
	}

	Common::Array<MovieFrame *> framesToDraw;
	for (MovieFrame *frame : _frames) {
		bool isAfterStart = _startTime + frame->startInMilliseconds() <= currentTime;
		bool isBeforeEnd = _startTime + frame->endInMilliseconds() >= currentTime;
		if (!isAfterStart || (isAfterStart && !isBeforeEnd)) {
			continue;
		}
		debugC(5, kDebugGraphics, "    (time: %d ms) Must re-draw frame %d (%d x %d) @ (%d, %d); start: %d ms, end: %d ms, keyframeEnd: %d ms, zIndex = %d", movieTime, frame->index(), frame->width(), frame->height(), frame->left(), frame->top(), frame->startInMilliseconds(), frame->endInMilliseconds(), frame->keyframeEndInMilliseconds(), frame->zCoordinate());
		framesToDraw.push_back(frame);
	}

	Common::sort(framesToDraw.begin(), framesToDraw.end(), [](MovieFrame * a, MovieFrame * b) {
		return a->zCoordinate() > b->zCoordinate();
	});
	for (MovieFrame *frame : framesToDraw) {
		g_engine->_screen->simpleBlitFrom(frame->_surface, Common::Point(frame->left(), frame->top()));
	}

	uint blitEnd = g_system->getMillis() - _startTime;
	uint elapsedTime = blitEnd - movieTime;
	debugC(5, kDebugGraphics, "GRAPHICS (Movie %d): Finished blitting in %d ms (movie time: %d ms)", _header->_id, elapsedTime, blitEnd);
	return true;
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
		MovieFrameHeader *header = nullptr;
		MovieFrame *frame = nullptr;
		while (isAnimationChunk) {
			uint sectionType = Datum(chunk).u.i;
			debugC(5, kDebugLoading, "Movie::readSubfile(): sectionType = 0x%x (@0x%llx)", static_cast<uint>(sectionType), static_cast<long long int>(chunk.pos()));
			switch (MovieSectionType(sectionType)) {
			case kMovieFrameSection: {
				header = new MovieFrameHeader(chunk);
				frame = new MovieFrame(chunk, header);
				_frames.push_back(frame);
				break;
			}

			case kMovieFooterSection: {
				MovieFrameFooter *footer = new MovieFrameFooter(chunk);
				// _footers.push_back(footer);
				// TODO: This does NOT handle the case where there are
				// keyframes. We need to match the footer to an arbitrary
				// frame, since some keyframes don't have footers, sigh.
				if (header == nullptr) {
					error("Movie::readSubfile(): No frame to match footer to");
				}
				if (header->_index == footer->_index) {
					frame->setFooter(footer);
				} else {
					error("Movie::readSubfile(): Footer index does not match frame index: %d != %d", header->_index, footer->_index);
				}
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
		bool isAudioChunk = (chunk._id = _header->_audioChunkReference);
		if (isAudioChunk) {
			byte *buffer = (byte *)malloc(chunk._length);
			chunk.read((void *)buffer, chunk._length);
			Audio::SeekableAudioStream *stream = nullptr;
			switch (_header->_soundEncoding) {
			case SoundEncoding::PCM_S16LE_MONO_22050:
				stream = Audio::makeRawStream(buffer, chunk._length, 22050, Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN, DisposeAfterUse::NO);
				break;

			case SoundEncoding::IMA_ADPCM_S16LE_MONO_22050:
				// TODO: The interface here is different. We can't pass in the
				// buffers directly. We have to make a stream first.
				// stream = Audio::makeADPCMStream(buffer, chunk.length,
				// DisposeAfterUse::NO, Audio::ADPCMType::kADPCMMSIma, 22050, 1,
				// 4);
				error("Movie::readSubfile(): ADPCM decoding not implemented yet");
				break;

			default:
				error("Sound::readChunk(): Unknown audio encoding 0x%x", (uint)_header->_soundEncoding);
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
	// TODO: We donʻt do anything with this yet!
}

} // End of namespace MediaStation
