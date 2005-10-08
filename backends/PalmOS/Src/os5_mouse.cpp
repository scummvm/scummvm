/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 * Copyright (C) 2002-2005 Chris Apers - PalmOS Backend
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "be_os5.h"

void OSystem_PalmOS5::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	if (w == 0 || h == 0)
		return;

	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColor = keycolor;

#ifdef PALMOS_68K
	// free old cursor if any
	if (_mouseDataH)
		WinDeleteWindow(_mouseDataH, false);

	// allocate new cursor
	Err e;
	_mouseDataH = WinCreateOffscreenWindow(w, h, nativeFormat, &e);
	_mouseDataP = (byte *)BmpGetBits(WinGetBitmap(_mouseDataH));

	// set bitmap transparency
	BmpSetTransparentValue(WinGetBitmap(_mouseDataH), _mouseKeyColor);

	// copy new cursor
	BitmapTypeV1 nfo = {
		w, h, w,
		{0,0,0,0,0,0,0,0,0},
		8, BitmapVersionOne, 0
	};

	BitmapTypeV3 *v3 = BmpCreateBitmapV3((BitmapType*)&nfo, kDensityDouble, (void *)buf, 0);

	WinSetDrawWindow(_mouseDataH);
	WinDrawBitmap((BitmapPtr)v3, 0, 0);
	BmpDelete((BitmapPtr)v3);
#endif
}

void OSystem_PalmOS5::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible)
		return;
	_mouseDrawn = true;

	_mouseCurState.y = _mouseCurState.y >= _screenHeight ? _screenHeight - 1 : _mouseCurState.y;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;

	int draw_x = x;
	int draw_y = y;

	// clip the mouse rect
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (w > _screenWidth - x)
		w = _screenWidth - x;
	if (h > _screenHeight - y)
		h = _screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

#ifdef PALMOS_68K
	// Backup the covered area draw the mouse cursor
	if (_overlayVisible) {
	} else {
		// backup...
		RectangleType r = {x, y, w, h};
		WinCopyRectangle(_offScreenH, _mouseBackupH, &r, 0, 0, winPaint);
		// ...and draw
		WinSetDrawWindow(_offScreenH);
		WinDrawBitmap(WinGetBitmap(_mouseDataH), draw_x, draw_y);
	}
#endif
}

void OSystem_PalmOS5::undraw_mouse() {
	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

#ifdef PALMOS_68K
	// No need to do clipping here, since draw_mouse() did that already
	if (_overlayVisible) {
	} else {
		RectangleType r = {0, 0, _mouseOldState.w, _mouseOldState.h};
		WinCopyRectangle(_mouseBackupH, _offScreenH, &r, _mouseOldState.x, _mouseOldState.y, winPaint);
	}
#endif
}
