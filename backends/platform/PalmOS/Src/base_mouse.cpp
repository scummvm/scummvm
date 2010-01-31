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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "be_base.h"

void OSystem_PalmBase::warpMouse(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		x = x >= _screenWidth  ? _screenWidth  - 1 : x;
		y = y >= _screenHeight ? _screenHeight - 1 : y;

		_mouseCurState.x = x;
		_mouseCurState.y = y;
	}
}

bool OSystem_PalmBase::showMouse(bool visible) {
	bool last = _mouseVisible;
	_mouseVisible = visible;

	return last;
}

void OSystem_PalmBase::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, int cursorTargetScale, const Graphics::PixelFormat *format) {
	if (w == 0 || h == 0)
		return;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColor = keycolor;

	if (_mouseCurState.w != w || _mouseCurState.h != h) {
		_mouseCurState.w = w;
		_mouseCurState.h = h;

		if (_mouseDataP)
			free(_mouseDataP);

		if (_mouseBackupP)
			free(_mouseBackupP);

		_mouseDataP = (byte *)malloc(w * h);
		_mouseBackupP = (byte *)malloc(w * h * 2); // if 16bit = *2
	}

	if (!_mouseBackupP) {
		free(_mouseDataP);
		_mouseDataP = NULL;
	}

	if (_mouseDataP)
		memcpy(_mouseDataP, buf, w * h);
}

void OSystem_PalmBase::simulate_mouse(Common::Event &event, Int8 iHoriz, Int8 iVert, Coord *xr, Coord *yr) {
	Int16 x = _mouseCurState.x;
	Int16 y = _mouseCurState.y;
	Int16 slow, fact;

	_lastKeyRepeat++;
	fact = _screenWidth / 320;
	fact = (fact) ? fact : 1;

	if (_lastKeyRepeat > 32 * fact)
		_lastKeyRepeat = 32 * fact;

	slow = (iHoriz && iVert) ? 2 : 1;

	x += iHoriz * (_lastKeyRepeat >> 2) / slow * fact;
	y += iVert * (_lastKeyRepeat >> 2) / slow * fact;

	x = (x < 0				) ? 0					: x;
	x = (x >= _screenWidth	) ? _screenWidth - 1	: x;
	y = (y < 0				) ? 0					: y;
	y = (y >= _screenHeight	) ? _screenHeight - 1	: y;

	*xr = x;
	*yr = y;
}
