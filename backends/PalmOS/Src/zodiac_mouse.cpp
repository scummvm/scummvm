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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#include "be_zodiac.h"

void OSystem_PalmZodiac::setMouseCursor(const byte *buf, uint w, uint h, int hotspotX, int hotspotY, byte keycolor, int cursorTargetScale) {
	if (w == 0 || h == 0)
		return;

	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspotX;
	_mouseHotspotY = hotspotY;

	_mouseKeyColor = keycolor;
	
#ifdef PALMOS_68K
	// set bitmap transparency
	WinSetDrawWindow(_mouseDataH);
	WinSetBackColor(_mouseKeyColor);
	WinEraseWindow();
	BmpSetTransparentValue(WinGetBitmap(_mouseDataH), _mouseKeyColor);

	if (w & 1) {
		// copy new cursor
		byte *dst = _mouseDataP;
		while (h--) {
			memcpy(dst, buf, w);
			dst += MAX_MOUSE_W;
			buf += w;
		}

	} else {
		// copy new cursor
		BitmapTypeV1 nfo = {
			w, h, w,
			{0,0,0,0,0,0,0,0,0},
			8, BitmapVersionOne, 0
		};

		BitmapTypeV3 *v3 = BmpCreateBitmapV3((BitmapType*)&nfo, kDensityDouble, (void *)buf, 0);
		WinDrawBitmap((BitmapPtr)v3, 0, 0);
		BmpDelete((BitmapPtr)v3);
	}
#else

	// copy new cursor
	byte *dst = _mouseDataP;
	while (h--) {
		memcpy(dst, buf, w);
		dst += MAX_MOUSE_W;
		buf += w;
	}
#endif
}

void OSystem_PalmZodiac::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible)
		return;

#ifdef PALMOS_ARM
	byte *src = _mouseDataP;		// Image representing the mouse
	byte *bak = _mouseBackupP;		// Surface used to backup the area obscured by the mouse
	byte *dst;						// Surface we are drawing into
	byte color;
	int width;
#endif

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
#ifdef PALMOS_ARM
		src -= x;
#endif
		x = 0;
	}
	if (y < 0) {
		h += y;
#ifdef PALMOS_ARM
		src -= y * MAX_MOUSE_W;
#endif
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

	// Backup the covered area draw the mouse cursor
#ifdef PALMOS_68K
	if (_overlayVisible) {
		WinSetDrawWindow(_screenH);
		WinDrawBitmap(WinGetBitmap(_mouseDataH), draw_x + _screenOffset.x, draw_y + _screenOffset.y);
		// force mouse redraw at each frame so that an extra buffer is not needed
		_mouseDrawn = false;

	} else {
		// backup...
		RectangleType r = {x, y, w, h};
		WinCopyRectangle(_offScreenH, _mouseBackupH, &r, 0, 0, winPaint);
		// ...and draw
		WinSetDrawWindow(_offScreenH);
		WinDrawBitmap(WinGetBitmap(_mouseDataH), draw_x, draw_y);
		_mouseDrawn = true;
	}
#else	
	if (_overlayVisible) {
		// where is the mouse ...
	} else {
		dst = _offScreenP + y * _screenWidth + x;

		do {
			width = w;
			do {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					*dst = color;
				dst++;
			} while (--width);

			src += MAX_MOUSE_W - w;
			bak += MAX_MOUSE_W - w;
			dst += _screenWidth - w;
		} while (--h);
		_mouseDrawn = true;
	}
#endif
}

void OSystem_PalmZodiac::undraw_mouse() {
	if (!_mouseDrawn)
		return;
	_mouseDrawn = false;

	// No need to do clipping here, since draw_mouse() did that already
#ifdef PALMOS_68K
	if (_overlayVisible) {
	} else {
		RectangleType r = {0, 0, _mouseOldState.w, _mouseOldState.h};
		WinCopyRectangle(_mouseBackupH, _offScreenH, &r, _mouseOldState.x, _mouseOldState.y, winPaint);
	}
#else
	if (_overlayVisible) {
	} else {
		int h = _mouseOldState.h;
		byte *dst, *bak = _mouseBackupP;
		dst = _offScreenP + _mouseOldState.y * _screenWidth + _mouseOldState.x;
		
		do {
			memcpy(dst, bak, _mouseOldState.w);
			bak += MAX_MOUSE_W;
			dst += _screenWidth;
		} while (--h);
	}
#endif
}
