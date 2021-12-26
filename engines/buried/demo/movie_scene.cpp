/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/video_window.h"
#include "buried/demo/movie_scene.h"

#include "graphics/surface.h"

namespace Buried {

MovieDisplayWindow::MovieDisplayWindow(BuriedEngine *vm, Window *parent, const Common::String &background, const Common::String &movie, int movieLeft, int movieTop)
		: Window(vm, parent) {
	_background = _vm->_gfx->getBitmap(background);

	// Create a rect to use to place the window inside the parent
	Common::Rect parentRect = parent->getRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = _rect.left + 640;
	_rect.bottom = _rect.top + 480;

	// Create and position the movie
	_movie = new VideoWindow(_vm, this);

	if (!_movie->openVideo(movie))
		error("Failed to open movie '%s'", movie.c_str());

	_movie->setWindowPos(nullptr, movieLeft, movieTop, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
	_movie->enableWindow(false);

	_timer = 0;
}

MovieDisplayWindow::~MovieDisplayWindow() {
	if (_timer != 0)
		killTimer(_timer);

	delete _movie;

	_background->free();
	delete _background;
}

bool MovieDisplayWindow::showMovieInWindow() {
	showWindow(kWindowShow);

	_movie->enableWindow(false);
	_movie->showWindow(kWindowShow);
	_movie->playVideo();

	_timer = setTimer(5000);
	return true;
}

void MovieDisplayWindow::onPaint() {
	_vm->_gfx->blit(_background, 0, 0);
}

bool MovieDisplayWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void MovieDisplayWindow::onLButtonUp(const Common::Point &point, uint flags) {
	((FrameWindow *)_parent)->returnToMainMenu();
}

void MovieDisplayWindow::onTimer(uint timer) {
	if (_movie->getMode() == VideoWindow::kModeStopped)
		((FrameWindow *)_parent)->returnToMainMenu();
}

} // End of namespace Buried
