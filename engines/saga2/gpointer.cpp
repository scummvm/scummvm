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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "graphics/cursorman.h"

#include "saga2/saga2.h"
#include "saga2/gpointer.h"

namespace Saga2 {

gMousePointer::gMousePointer(gDisplayPort &port) {
	_hideCount = 0;                          // pointer not hidden

	//  initialize coords
	_offsetPosition.x = _offsetPosition.y = 0;
	_currentPosition.x = _currentPosition.y = 0;

	//  a backsave extent of 0 means not saved
	_saveExtent.width = _saveExtent.height = 0;
	_shown = 0;

	//  set up the backsave port
	_savePort.setMap(&_saveMap);
	_savePort.setMode(kDrawModeReplace);

	_videoPort = &port;

	//  no imagery at this time.
	_pointerImage = nullptr;
}

gMousePointer::~gMousePointer() {
	if (_saveMap._data)
		free(_saveMap._data);
}

//  Init & status check
bool gMousePointer::init(Point16 pointerLimits) {
	return true;
}

//  Private routine to draw the mouse pointer image
void gMousePointer::draw() {
	if (_hideCount < 1) {
		CursorMan.showMouse(true);
		_shown = 1;
	} else
		_shown = 0;
}

//  Private routine to restore the mouse pointer image
void gMousePointer::restore() {
	if (_shown) {
		//  blit from the saved map to the current position.

		CursorMan.showMouse(false);

		//  A height of zero means backsave is invalid

		_shown = 0;
	}
}

//  Makes the mouse pointer visible
void gMousePointer::show() {
	assert(_hideCount > 0);

	if (--_hideCount == 0) {
		draw();
	}
}

//  Makes the mouse pointer invisible
void gMousePointer::hide() {
	if (_hideCount++ == 0) {
		restore();
	}
}

//  Makes the mouse pointer visible
void gMousePointer::show(gPort &port, Rect16 r) {
	Point16         org = port.getOrigin();

	r.x += org.x;
	r.y += org.y;

	if (_saveExtent.overlap(r)) {
		if (--_hideCount == 0) {
			draw();
		}

	}
}

//  Makes the mouse pointer visible
int gMousePointer::manditoryShow() {
	int rv = 0;
	while (_hideCount > 0) {
		show();
		rv++;
	}
	while (_hideCount < 0) {
		hide();
		rv--;
	}
	if (!_shown)
		draw();
	return rv;
}


//  Makes the mouse pointer invisible
void gMousePointer::hide(gPort &port, Rect16 r) {
	Point16         org = port.getOrigin();

	r.x += org.x;
	r.y += org.y;

	if (_saveExtent.overlap(r)) {
		if (_hideCount++ == 0) {
			restore();
			CursorMan.showMouse(false);
		}
	}
}

//  Moves the mouse pointer to a new position
void gMousePointer::move(Point16 pos) {
	Point16         offsetPos = pos + _offsetPosition;

	if (offsetPos != _currentPosition) {
		restore();
		_currentPosition = offsetPos;
		draw();
	}
}

//  Sets the mouse pointer imagery
void gMousePointer::setImage(
    gPixelMap       &img,
    int             x,
    int             y) {
	Point16         pos = _currentPosition - _offsetPosition;

	if (_pointerImage != &img
	        ||  x != _offsetPosition.x
	        ||  y != _offsetPosition.y
	        ||  img._size != _saveMap._size) {
		_offsetPosition.x = x;
		_offsetPosition.y = y;

		hide();
		if (_saveMap._data)
			free(_saveMap._data);
		_saveMap._size = img._size;
		_saveMap._data = (uint8 *)malloc(img.bytes());
		_pointerImage = &img;
		_currentPosition = pos + _offsetPosition;

		CursorMan.replaceCursor(img._data, img._size.x, img._size.y, -x, -y, 0);
		CursorMan.showMouse(true);
		show();
	}
}

} // end of namespace Saga2
