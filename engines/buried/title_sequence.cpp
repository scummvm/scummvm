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
#include "buried/title_sequence.h"
#include "buried/resources.h"
#include "buried/video_window.h"

#include "graphics/surface.h"

namespace Buried {

TitleSequenceWindow::TitleSequenceWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	// Initialize member variables
	_currentBackground = nullptr;
	_currentMovie = nullptr;
	_exitNow = false;
	_currentAnimation = 0;

	// Create a rect to use to place the window inside the parent
	Common::Rect parentRect = parent->getRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = _rect.left + 640;
	_rect.bottom = _rect.top + 480;

	_timerID = setTimer(1000);
}

TitleSequenceWindow::~TitleSequenceWindow() {
	if (_currentBackground) {
		_currentBackground->free();
		delete _currentBackground;
	}

	delete _currentMovie;

	killTimer(_timerID);
}

bool TitleSequenceWindow::playTitleSequence() {
	switch (_currentAnimation) {
	case 0: // Display Sanctuary Woods Logo
		// Clean the window
		invalidateWindow();

		_currentMovie = new VideoWindow(_vm, this);

		if (!_currentMovie->openVideo(_vm->getFilePath(IDS_TITLE_SW_LOGO_FILENAME))) {
			delete _currentMovie;
			_currentMovie = nullptr;
			((FrameWindow *)_parent)->returnToMainMenu();
			return false;
		}

		_currentMovie->setWindowPos(nullptr, 195, 115, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
		_currentMovie->enableWindow(false);
		_currentMovie->showWindow(kWindowShow);
		_currentMovie->playVideo();
		_currentAnimation = 1;
		return true;
	case 1: // Display Presto Studios Logo
		// Clean the window
		invalidateWindow();

		_currentMovie = new VideoWindow(_vm, this);

		if (!_currentMovie->openVideo(_vm->getFilePath(IDS_TITLE_PRESTO_LOGO_FILENAME))) {
			delete _currentMovie;
			_currentMovie = nullptr;
			((FrameWindow *)_parent)->returnToMainMenu();
			return false;
		}

		_currentMovie->setWindowPos(nullptr, 200, 60, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
		_currentMovie->enableWindow(false);
		_currentMovie->showWindow(kWindowShow);
		_currentMovie->playVideo();
		_currentAnimation = 2;
		return true;
	case 2: // Display the intro credits movie
		_currentBackground = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_TITLE_STARFIELD_FILENAME));

		_currentMovie = new VideoWindow(_vm, this);

		if (!_currentMovie->openVideo(_vm->getFilePath(IDS_TITLE_MOVIE_FILENAME))) {
			delete _currentMovie;
			_currentMovie = nullptr;
			((FrameWindow *)_parent)->returnToMainMenu();
			return false;
		}

		_currentMovie->setWindowPos(nullptr, 60, 138, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
		_currentMovie->enableWindow(false);
		_currentMovie->showWindow(kWindowShow);
		invalidateWindow();
		_currentMovie->playVideo();
		_currentAnimation = 3;
		return true;
	case 3: // Exit the title sequence
		((FrameWindow *)_parent)->showMainMenu();
		return true;
	}

	return true;
}

void TitleSequenceWindow::onPaint() {
	if (_currentBackground)
		_vm->_gfx->blit(_currentBackground, 0, 0);
}

bool TitleSequenceWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void TitleSequenceWindow::onLButtonUp(const Common::Point &point, uint flags) {
	_exitNow = true;
}

void TitleSequenceWindow::onMButtonUp(const Common::Point &point, uint flags) {
	_exitNow = true;
}

void TitleSequenceWindow::onRButtonUp(const Common::Point &point, uint flags) {
	_exitNow = true;
}

void TitleSequenceWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	_exitNow = true;
}

void TitleSequenceWindow::onTimer(uint timer) {
	if (_exitNow || (_currentMovie && _currentMovie->getMode() == VideoWindow::kModeStopped)) {
		// Destroy all resources
		if (_currentBackground) {
			_currentBackground->free();
			delete _currentBackground;
			_currentBackground = nullptr;
		}

		delete _currentMovie;
		_currentMovie = nullptr;

		// Clean out the input queue
		_exitNow = false;

		playTitleSequence();
	}
}

} // End of namespace Buried
