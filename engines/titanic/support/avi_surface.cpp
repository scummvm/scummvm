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
#include "titanic/support/video_surface.h"
#include "video/avi_decoder.h"

namespace Titanic {

AVISurface::AVISurface(const CResourceKey &key) {

	// TODO
/*
Video::AVIDecoder *decoder = new Video::AVIDecoder();
decoder->ignoreSecondaryVideoTracks();
_video = decoder;
_field14 = 1;

if (!_video->loadFile(name.getString()))
error("Could not open video - %s", name.getString().c_str());
*/
}

AVISurface::~AVISurface() {
	if (_videoSurface)
		_videoSurface->_blitStyleFlag = false;
	delete _frameInfo;
}

bool AVISurface::play(uint flags, CGameObject *obj) {
	if (flags & MOVIE_REVERSE)
		return play(_decoder->getFrameCount() - 1, 0, flags, obj);
	else
		return play(0, _decoder->getFrameCount() - 1, flags, obj);
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
		changeFrame(initialFrame);
	} else {
		return true;
	}	
}

void AVISurface::stop() {
	_isPlaying = false;
	_decoder->stop();
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
	_decoder->seekToFrame(frameNumber);
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

	warning("TODO: Get video track list from video decoder");
}

uint AVISurface::getWidth() const {
	return _decoder->getWidth();
}

uint AVISurface::getHeight() const {
	return _decoder->getHeight();
}

void AVISurface::setFrame(int frameNumber) {
	// If playback was in process, stop it
	if (_isPlaying)
		stop();

	// Ensure the frame number is valid
	if (frameNumber >= _decoder->getFrameCount())
		frameNumber = _decoder->getFrameCount() - 1;

	seekToFrame(frameNumber);
	renderFrame();
}

int AVISurface::getFrame() const {
	return _decoder->getCurFrame();
}

bool AVISurface::renderFrame() {
	// Check there's a frame ready for 
	assert(_videoSurface);
	if (!_decoder->needsUpdate())
		return false;

	// Get the frame to render, and draw it on the surface
	const Graphics::Surface *frame = _decoder->decodeNextFrame();
	_videoSurface->blitFrom(Point(0, 0), frame);
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

void *AVISurface::duplicateFrameInfo() const {
	// TODO
	return nullptr;
}

} // End of namespace Titanic
