/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/system.h"
#include "graphics/surface.h"
#include "video/qt_decoder.h"
#include "video/video_decoder.h"

#include "pegasus/movie.h"

namespace Pegasus {

Movie::Movie(const tDisplayElementID id) : Animation(id) {
	_video = 0;
	_directDraw = false;
	setScale(600);
}

Movie::~Movie() {
	releaseMovie();
}

//	*** Make sure this will stop displaying the movie.

void Movie::releaseMovie() {
	if (_video) {
		delete _video;
		_video = 0;
		disposeAllCallBacks();
		deallocateSurface();
		// TODO: Decrease global direct draw counter
	}
}

void Movie::initFromMovieFile(const Common::String &fileName, bool transparent) {
	_transparent = transparent;

	releaseMovie();
	_video = new Video::QuickTimeDecoder();
	if (!_video->loadFile(fileName))	
		error("Could not load video '%s'", fileName.c_str());

	_video->pauseVideo(true);

	Common::Rect bounds(0, 0, _video->getWidth(), _video->getHeight());
	allocateSurface(bounds);
	setBounds(bounds);
	_movieBox = bounds;

	setStart(0, getScale());
	setStop(_video->getDuration() * getScale() / 1000, getScale());
}

void Movie::setDirectDraw(const bool flag) {
	_directDraw = flag;
	// TODO: Increase/decrease the global direct draw counter
}

void Movie::redrawMovieWorld() {
	if (_video) {
		const Graphics::Surface *frame = _video->decodeNextFrame();

		if (!frame)
			return;

		if (_directDraw) {
			// Copy to the screen
			Common::Rect bounds;
			getBounds(bounds);
			g_system->copyRectToScreen((byte *)frame->pixels, frame->pitch, bounds.left, bounds.top, frame->w, frame->h);
		} else {
			// Copy to the surface using _movieBox
			uint16 width = MIN<int>(MIN<int>(frame->w, _movieBox.width()), _surface->w - _movieBox.left);
			uint16 height = MIN<int>(MIN<int>(frame->h, _movieBox.height()), _surface->h - _movieBox.top);

			for (uint16 y = 0; y < height; y++)
				memcpy((byte *)_surface->getBasePtr(_movieBox.left, _movieBox.top + y), (const byte *)frame->getBasePtr(0, y), width * frame->format.bytesPerPixel);
		}

		triggerRedraw();
	}
}

void Movie::draw(const Common::Rect &r) {
	Common::Rect surfaceBounds;
	getSurfaceBounds(surfaceBounds);
	Common::Rect r1 = r;

	Common::Rect bounds;
	getBounds(bounds);
	surfaceBounds.moveTo(bounds.left, bounds.top);
	r1 = r1.findIntersectingRect(surfaceBounds);
	getSurfaceBounds(surfaceBounds);

	Common::Rect r2 = r1;
	r2.translate(surfaceBounds.left - bounds.left, surfaceBounds.top - bounds.top);
	drawImage(r2, r1);
}

void Movie::moveMovieBoxTo(const tCoordType h, const tCoordType v) {
	_movieBox.moveTo(h, v);
}

void Movie::setVolume(uint16 volume) {
	// TODO
}

void Movie::setTime(const TimeValue time, const TimeScale scale) {
	if (_video) {
		// Don't go past the ends of the movie
		Common::Rational timeFrac = Common::Rational(time, ((scale == 0) ? getScale() : scale));

		if (timeFrac < Common::Rational(_startTime, _startScale))
			timeFrac = Common::Rational(_startTime, _startScale);
		else if (timeFrac >= Common::Rational(_stopTime, _stopScale))
			return;

		_video->seekToTime(Audio::Timestamp(0, timeFrac.getNumerator(), timeFrac.getDenominator()));
		_time = timeFrac;
	}
}

void Movie::setRate(const Common::Rational rate) {
	if (rate != 1 && rate != 0)
		error("Cannot set movie rate");

	TimeBase::setRate(rate);
}

void Movie::start() {
	if (_video && _video->isPaused())
		_video->pauseVideo(false);

	TimeBase::start();
}

void Movie::stop() {
	if (_video && !_video->isPaused())
		_video->pauseVideo(true);

	TimeBase::stop();
}

void Movie::resume() {
	if (_video && _video->isPaused())
		_video->pauseVideo(false);

	TimeBase::resume();
}

void Movie::checkCallBacks() {
	TimeBase::checkCallBacks();

	// The reason why we overrode TimeBase's checkCallBacks():
	// Again, avoiding timers and handling it here
	if (!_video->isPaused()) {
		if (_video->needsUpdate())
			redrawMovieWorld();

		uint32 startTime = _startTime * getScale() / _startScale;
		uint32 stopTime = _stopTime * getScale() / _stopScale;
		uint32 actualTime = CLIP<int>(_video->getElapsedTime() * getScale() / 1000, startTime, stopTime);
		_time = Common::Rational(actualTime, getScale());
	}
}

} // End of namespace Pegasus
