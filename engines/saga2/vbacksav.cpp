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

#include "saga2/saga2.h"
#include "saga2/vbacksav.h"

namespace Saga2 {

gBackSave::gBackSave(const Rect16 &extent) {
	Rect16  displayRect(0, 0, 640, 480);

	//  initialize the rectangle

	_savedRegion = intersect(extent, displayRect);    // intersect with display size

	//  Set up the image structure for the video page

	_savedPixels._size.x = _savedRegion.width;
	_savedPixels._size.y = _savedRegion.height;
//	_savedPixels._data = (uint8 *)malloc(_savedPixels.bytes());
	_savedPixels._data = (uint8 *)malloc(_savedPixels.bytes());

	//  Initialize the graphics port

	setMap(&_savedPixels);
	setMode(kDrawModeReplace);

	_saved = false;
}

gBackSave::~gBackSave() {
	free(_savedPixels._data);
}

void gBackSave::save(gDisplayPort &port) {
	if (!_saved && _savedPixels._data) {
		port._protoPage.readPixels(_savedRegion, _savedPixels._data, _savedPixels._size.x);
		_saved = true;
	}
}

void gBackSave::restore(gDisplayPort &port) {
	if (_saved && _savedPixels._data) {
		port._protoPage.writePixels(_savedRegion, _savedPixels._data, _savedPixels._size.x);
		_saved = false;
	}
}

} // end of namespace Saga2
