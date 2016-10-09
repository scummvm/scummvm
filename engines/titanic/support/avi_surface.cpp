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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "graphics/pixelformat.h"
#include "video/avi_decoder.h"
#include "titanic/support/avi_surface.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "titanic/titanic.h"

namespace Titanic {

Video::AVIDecoder::AVIVideoTrack &AVIDecoder::getVideoTrack() {
	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return *dynamic_cast<AVIVideoTrack *>(*it);

	error("Could not find video track");
}

/**
 * Track filter for AVIDecoder that filters out any secondary video track
 */
static bool primaryTrackSelect(bool isVideo, int trackCounter) {
	return !isVideo || trackCounter == 0;
}

/**
 * Track filter for AVIDecoder that only accepts the secondary video track
 * for a video, if present
 */
static bool secondaryTrackSelect(bool isVideo, int trackCounter) {
	return isVideo && trackCounter > 0;
}

AVISurface::AVISurface(const CResourceKey &key) {
	_videoSurface = nullptr;
	_streamCount = 0;
	_movieFrameSurface[0] = _movieFrameSurface[1] = nullptr;

	// Reset current frame. We need to keep track of frames separately from the decoders,
	// since it needs to be able to go beyond the frame count or to negative to allow
	// correct detection of when range playbacks have finished
	_currentFrame = -1;
	_isReversed = false;

	// Create a decoder for the audio (if any) and primary video track
	_decoders[0] = new AVIDecoder(Audio::Mixer::kPlainSoundType, primaryTrackSelect);
	if (!_decoders[0]->loadFile(key.getString()))
		error("Could not open video - %s", key.getString().c_str());

	_streamCount = 1;

	// Create a decoder for any secondary video track
	AVIDecoder *decoder2 = new AVIDecoder(Audio::Mixer::kPlainSoundType, secondaryTrackSelect);
	if (decoder2->loadFile(key.getString())) {
		_decoders[1] = decoder2;
		++_streamCount;
	} else {
		delete decoder2;
		_decoders[1] = nullptr;
	}
}

AVISurface::~AVISurface() {
	if (_videoSurface)
		_videoSurface->_transBlitFlag = false;
	delete _movieFrameSurface[0];
	delete _movieFrameSurface[1];
	delete _decoders[0];
	delete _decoders[1];
}

bool AVISurface::play(uint flags, CGameObject *obj) {
	if (flags & MOVIE_REVERSE)
		return play(_decoders[0]->getFrameCount() - 1, 0, flags, obj);
	else
		return play(0, _decoders[0]->getFrameCount() - 1, flags, obj);
}

bool AVISurface::play(int startFrame, int endFrame, uint flags, CGameObject *obj) {
	if (flags & MOVIE_STOP_PREVIOUS)
		stop();

	return play(startFrame, endFrame, -1, flags, obj);
}

bool AVISurface::play(int startFrame, int endFrame, int initialFrame, uint flags, CGameObject *obj) {
	CMovieRangeInfo *info = new CMovieRangeInfo();
	info->_startFrame = startFrame;
	info->_endFrame = endFrame;
	info->_isReversed = endFrame < startFrame;
	info->_initialFrame = 0;
	info->_isRepeat = flags & MOVIE_REPEAT;

	if (obj) {
		CMovieEvent *me = new CMovieEvent();
		me->_type = MET_MOVIE_END;
		me->_startFrame = startFrame;
		me->_endFrame = endFrame;
		me->_initialFrame = 0;
		me->_gameObject = obj;

		info->addEvent(me);
	}

	_movieRangeInfo.push_back(info);

	if (_movieRangeInfo.size() == 1) {
		// First play call, so start the movie playing
		setReversed(info->_isReversed);
		return startAtFrame(initialFrame);
	} else {
		return true;
	}
}

void AVISurface::stop() {
	_decoders[0]->stop();
	if (_decoders[1])
		_decoders[1]->stop();

	_movieRangeInfo.destroyContents();
}

bool AVISurface::startAtFrame(int frameNumber) {
	if (isPlaying())
		// If it's already playing, then don't allow it
		return false;

	if (frameNumber == -1)
		// Default to starting frame of first movie range
		frameNumber = _movieRangeInfo.front()->_startFrame;

	// Get the initial frame
	seekToFrame(frameNumber);
	renderFrame();

	// Start the playback
	_decoders[0]->start();
	if (_decoders[1])
		_decoders[1]->start();

	return true;
}

void AVISurface::seekToFrame(uint frameNumber) {
	if ((int)frameNumber != getFrame()) {
		_decoders[0]->seekToFrame(frameNumber);
		if (_decoders[1])
			_decoders[1]->seekToFrame(frameNumber);

		_currentFrame = (int)frameNumber;
	}

	renderFrame();
}

void AVISurface::setReversed(bool isReversed) {
	_decoders[0]->setReverse(isReversed);
	if (_decoders[1])
		_decoders[1]->setReverse(isReversed);

	_isReversed = isReversed;
}

bool AVISurface::handleEvents(CMovieEventList &events) {
	if (!isPlaying())
		return true;

	CMovieRangeInfo *info = _movieRangeInfo.front();
	_currentFrame += _isReversed ? -1 : 1;

	int newFrame = _currentFrame;
	if ((info->_isReversed && newFrame <= info->_endFrame) ||
		(!info->_isReversed && newFrame >= info->_endFrame)) {
		if (info->_isRepeat) {
			newFrame = info->_startFrame;
		} else {
			info->getMovieEnd(events);
			_movieRangeInfo.remove(info);
			delete info;

			if (_movieRangeInfo.empty()) {
				// NO more ranges, so stop playback
				stop();
			} else {
				// Not empty, so move onto new first one
				info = _movieRangeInfo.front();
				newFrame = info->_startFrame;
			}
		}
	}

	if (isPlaying()) {
		if (newFrame != getFrame()) {
			// The frame has been changed, so move to new position
			setReversed(info->_isReversed);
			seekToFrame(newFrame);
		}

		// Get any events for the given position
		info->getMovieFrame(events, newFrame);
		return renderFrame();
	} else {
		return false;
	}
}

void AVISurface::setVideoSurface(CVideoSurface *surface) {
	_videoSurface = surface;

	// Handling for secondary video stream
	if (_decoders[1]) {
		const Common::String &streamName = _decoders[1]->getVideoTrack().getName();

		if (streamName == "mask0") {
			_videoSurface->_transparencyMode = TRANS_MASK0;
		} else if (streamName == "mask255") {
			_videoSurface->_transparencyMode = TRANS_MASK255;
		} else if (streamName == "alpha0") {
			_videoSurface->_transparencyMode = TRANS_ALPHA0;
		} else if (streamName == "alpha255") {
			_videoSurface->_transparencyMode = TRANS_ALPHA255;
		}
	}

	setupDecompressor();
}

void AVISurface::setupDecompressor() {
	for (int idx = 0; idx < 2; ++idx) {
		if (!_decoders[idx])
			continue;
		AVIDecoder &decoder = *_decoders[idx];

		// Setup frame surface
		_movieFrameSurface[idx] = new Graphics::ManagedSurface(decoder.getWidth(), decoder.getHeight(),
			g_system->getScreenFormat());

		// TODO: See whether this simplified form of original works
		if (idx == 1)
			_videoSurface->_transBlitFlag = true;
	}
}

uint AVISurface::getWidth() const {
	return _decoders[0]->getWidth();
}

uint AVISurface::getHeight() const {
	return _decoders[0]->getHeight();
}

void AVISurface::setFrame(int frameNumber) {
	// If playback was in process, stop it
	if (isPlaying())
		stop();

	// Ensure the frame number is valid
	if (frameNumber >= (int)_decoders[0]->getFrameCount())
		frameNumber = _decoders[0]->getFrameCount() - 1;

	seekToFrame(frameNumber);
	renderFrame();
}

bool AVISurface::isNextFrame() const {
	return _decoders[0]->getTimeToNextFrame() == 0;
}

bool AVISurface::renderFrame() {
	// Check there's a frame ready for display
	if (!_decoders[0]->needsUpdate())
		return false;

	// Decode each decoder's video stream into the appropriate surface
	for (int idx = 0; idx < _streamCount; ++idx) {
		const Graphics::Surface *frame = _decoders[idx]->decodeNextFrame();

		if (_movieFrameSurface[idx]->format == frame->format) {
			_movieFrameSurface[idx]->blitFrom(*frame);
		} else {
			// Format mis-match, so we need to convert the frame
			Graphics::Surface *s = frame->convertTo(_movieFrameSurface[idx]->format,
				_decoders[idx]->getPalette());
			_movieFrameSurface[idx]->blitFrom(*s);
			s->free();
			delete s;
		}
	}

	// Blit the primary video frame onto the main overall surface
	_videoSurface->lock();
	_videoSurface->getRawSurface()->blitFrom(*_movieFrameSurface[0]);
	_videoSurface->unlock();

	return false;
}

bool AVISurface::addEvent(int frameNumber, CGameObject *obj) {
	if (!_movieRangeInfo.empty()) {
		CMovieRangeInfo *tail = _movieRangeInfo.back();
		if (frameNumber == -1)
			frameNumber = tail->_startFrame;

		CMovieEvent *me = new CMovieEvent();
		me->_type = MET_FRAME;
		me->_startFrame = 0;
		me->_endFrame = 0;
		me->_initialFrame = frameNumber;
		me->_gameObject = obj;
		tail->addEvent(me);

		return _movieRangeInfo.size() == 1 && frameNumber == getFrame();
	}

	return false;
}

void AVISurface::setFrameRate(double rate) {
	_decoders[0]->setRate(Common::Rational((int)rate));
	if (_decoders[1])
		_decoders[1]->setRate(Common::Rational((int)rate));
}

Graphics::ManagedSurface *AVISurface::getSecondarySurface() {
	return _streamCount <= 1 ? nullptr : _movieFrameSurface[1];
}

Graphics::ManagedSurface *AVISurface::duplicateSecondaryFrame() const {
	if (_streamCount <= 1) {
		return nullptr;
	} else {
		Graphics::ManagedSurface *dest = new Graphics::ManagedSurface(_movieFrameSurface[1]->w,
			_movieFrameSurface[1]->h, _movieFrameSurface[1]->format);
		dest->blitFrom(*_movieFrameSurface[1]);
		return dest;
	}
}

void AVISurface::playCutscene(const Rect &r, uint startFrame, uint endFrame) {
	bool isDifferent = _movieFrameSurface[0]->w != r.width() ||
		_movieFrameSurface[0]->h != r.height();

	startAtFrame(startFrame);
	while (_currentFrame < (int)endFrame && !g_vm->shouldQuit()) {
		if (isNextFrame()) {
			renderFrame();
			_currentFrame = _decoders[0]->getCurFrame();

			if (isDifferent) {
				// Clear the destination area, and use the transBlitFrom method,
				// which supports arbitrary scaling, to reduce to the desired size
				g_vm->_screen->fillRect(r, 0);
				g_vm->_screen->transBlitFrom(*_movieFrameSurface[0],
					Common::Rect(0, 0, _movieFrameSurface[0]->w, _movieFrameSurface[0]->h), r);
			} else {
				g_vm->_screen->blitFrom(*_movieFrameSurface[0], Common::Point(r.left, r.top));
			}

			g_vm->_screen->update();
			g_vm->_events->pollEvents();
		}

		// Brief wait, and check at the same time for clicks to abort the clip
		if (g_vm->_events->waitForPress(10))
			break;
	}

	stop();
}

} // End of namespace Titanic
