/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * $Header$
 *
 */

#include "be_base.h"

void OSystem_PalmBase::warpMouse(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		undraw_mouse();
	}
}

bool OSystem_PalmBase::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;
	
	bool last = _mouseVisible;
	_mouseVisible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}

void OSystem_PalmBase::simulate_mouse(Event &event, Int8 iHoriz, Int8 iVert, Coord *xr, Coord *yr) {
	Int16 x = _mouseCurState.x;
	Int16 y = _mouseCurState.y;
	Int16 slow;

	if (_lastKey != kKeyNone) {
		_lastKeyRepeat++;

		if (_lastKeyRepeat > 16)
			_lastKeyRepeat = 16;
	}
	else
		_lastKeyRepeat = 0;

	slow = (iHoriz && iVert) ? 2 : 1;

	x += iHoriz * (_lastKeyRepeat >> 2) / slow;
	y += iVert * (_lastKeyRepeat >> 2) / slow;

	x = (x < 0				) ? 0					: x;
	x = (x >= _screenWidth	) ? _screenWidth - 1	: x;
	y = (y < 0				) ? 0					: y;
	y = (y >= _screenHeight	) ? _screenHeight - 1	: y;
	
	*xr = x;
	*yr = y;
}
