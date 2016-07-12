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

#include "titanic/support/avi_surface.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/video_surface.h"
#include "common/system.h"
#include "graphics/pixelformat.h"
#include "video/avi_decoder.h"

namespace Titanic {

Video::AVIDecoder::AVIVideoTrack &AVIDecoder::getVideoTrack() {
	for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return *static_cast<AVIVideoTrack *>(*it);
			
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
	_currentPos = 0;
	_priorFrame = 0;
	_streamCount = 0;
	_movieFrameSurface[0] = _movieFrameSurface[1] = nullptr;
	_isPlaying = false;

	// Create a decoder for the audio (if any) and primary video track
	_decoders[0] = new AVIDecoder(Audio::Mixer::kPlainSoundType, primaryTrackSelect);
	if (!_decoders[0]->loadFile(key.getString()))
		error("Could not open video - %s", key.getString().c_str());

	// Create a decoder for any secondary video track
	AVIDecoder *decoder2 = new AVIDecoder(Audio::Mixer::kPlainSoundType, secondaryTrackSelect);
	if (decoder2->loadFile(key.getString())) {
		_decoders[1] = decoder2;
	} else {
		delete decoder2;
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
	info->_isFlag1 = flags & MOVIE_1;

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
		return changeFrame(initialFrame);
	} else {
		return true;
	}	
}

void AVISurface::stop() {
	_isPlaying = false;
	_decoders[0]->stop();
	if (_decoders[1])
		_decoders[1]->stop();

	_movieRangeInfo.destroyContents();
}

bool AVISurface::changeFrame(int frameNumber) {
	if (_isPlaying)
		return false;

	if (frameNumber == -1)
		// Default to starting frame of first movie range
		frameNumber = _movieRangeInfo.front()->_startFrame;

	seekToFrame(frameNumber);
	renderFrame();

	_isPlaying = true;
	return true;
}

void AVISurface::seekToFrame(uint frameNumber) {
	_decoders[0]->seekToFrame(frameNumber);
	if (_decoders[1])
		_decoders[1]->seekToFrame(frameNumber);

	_priorFrame = frameNumber;
}

bool AVISurface::handleEvents(CMovieEventList &events) {
	if (!_isPlaying)
		return true;

	CMovieRangeInfo *info = _movieRangeInfo.front();
	_currentPos += info->_isReversed ? -1 : 1;
	if ((info->_isReversed && _currentPos < info->_endFrame) ||
		(!info->_isReversed && _currentPos > info->_endFrame)) {
		if (info->_isFlag1) {
			info->getMovieEnd(events);
			_movieRangeInfo.remove(info);
			delete info;

			if (_movieRangeInfo.empty()) {
				// NO more ranges, so stop playback
				stop();
			} else {
				// Not empty, so move onto new first one
				info = _movieRangeInfo.front();
				_currentPos = info->_startFrame;
			}
		} else {
			_currentPos = info->_startFrame;
		}
	}

	if (_isPlaying) {
		// SInce movie ranges can change the position in the movie,
		// ensure the decoder is kept in sync
		seekToFrame(_currentPos);
				
		info->getMovieFrame(events, _currentPos);
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
		if (idx == 2)
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
	if (_isPlaying)
		stop();

	// Ensure the frame number is valid
	if (frameNumber >= (int)_decoders[0]->getFrameCount())
		frameNumber = _decoders[0]->getFrameCount() - 1;

	seekToFrame(frameNumber);
	renderFrame();
}

int AVISurface::getFrame() const {
	return _decoders[0]->getCurFrame();
}

bool AVISurface::renderFrame() {
	// Check there's a frame ready for display
	assert(_videoSurface);
	if (!_decoders[0]->needsUpdate() || (_decoders[1] && !_decoders[1]->needsUpdate()))
		return false;

	// Decode each decoder's video stream into the appropriate surface
	for (int idx = 0; idx < 2; ++idx) {
		if (_decoders[idx]) {
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

		return _movieRangeInfo.size() == 1 && frameNumber == _priorFrame;
	}

	return false;
}

void AVISurface::setFrameRate(double rate) {
	if (rate >= 0.0 && rate <= 100.0) {
		_frameRate = rate;
		warning("TODO: Frame rate set to %d yet to be implemented", (int)rate);
	}
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

} // End of namespace Titanic
