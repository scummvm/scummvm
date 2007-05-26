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
 * $URL$
 * $Id$
 *
 */

#include "be_os5.h"

void OSystem_PalmOS5::setCursorPalette(const byte *colors, uint start, uint num) {
	for(uint i = 0; i < num; i++) {
		_mousePal[i + start] = gfxMakeDisplayRGB(colors[0], colors[1], colors[2]);
		colors += 4;
	}
	
	_cursorPaletteDisabled = false;
}

void OSystem_PalmOS5::disableCursorPalette(bool disable) {
	_cursorPaletteDisabled = disable;
}

// TODO: this code is almost the same as Zodiac version.
void OSystem_PalmOS5::draw_mouse() {
	if (!_mouseDataP || _mouseDrawn || !_mouseVisible)
		return;
	
	byte *src = _mouseDataP;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;

	// clip the mouse rect
	if (y < 0) {
		src -= y * w;
		h += y;
		y = 0;
	}
	if (x < 0) {
		src -= x;
		w += x;
		x = 0;
	}

	if (h > _screenHeight - y)
		h = _screenHeight - y;
	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (w <= 0 || h <= 0)
		return;

	// store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content
	_mouseOldState.x = x;
	_mouseOldState.y = y;
	_mouseOldState.w = w;
	_mouseOldState.h = h;

	byte color;
	int ww;

	if (_overlayVisible) {
		int16 *bak = (int16 *)_mouseBackupP;
		int16 *pal = _cursorPaletteDisabled ? _nativePal : _mousePal;
		int16 *dst = _overlayP + y * _screenWidth + x;

		do {
			ww = w;
			do {
				*bak++ = *dst;
				color = *src++;

				// transparent, don't draw
				if (color != _mouseKeyColor)
					*dst = pal[color];
				dst++;
			} while (--ww);

			src += _mouseCurState.w - w;
			dst += _screenWidth - w;
		} while (--h);

	} else {
		byte *bak = _mouseBackupP;
		byte *dst =_offScreenP + y * _screenWidth + x;

		do {
			ww = w;
			do {
				*bak++ = *dst;
				color = *src++;

				// transparent, don't draw
				if (color != _mouseKeyColor)
					*dst = color;
				dst++;
			} while (--ww);

			src += _mouseCurState.w - w;
			dst += _screenWidth - w;
		} while (--h);
	}

	_mouseDrawn = true;
}

void OSystem_PalmOS5::undraw_mouse() {
	if (!_mouseDrawn)
		return;

	int h = _mouseOldState.h;

	// no need to do clipping here, since draw_mouse() did that already
	if (_overlayVisible) {
		int16 *dst = _overlayP + _mouseOldState.y * _screenWidth + _mouseOldState.x;
		int16 *bak = (int16 *)_mouseBackupP;

		do {
			MemMove(dst, bak, _mouseOldState.w * 2);
			dst += _screenWidth;
			bak += _mouseOldState.w;
		} while (--h);

	} else {
		byte *dst = _offScreenP + _mouseOldState.y * _screenWidth + _mouseOldState.x;
		byte *bak = _mouseBackupP;
		
		do {
			MemMove(dst, bak, _mouseOldState.w);
			dst += _screenWidth;
			bak +=  _mouseOldState.w;
		} while (--h);
	}

	_mouseDrawn = false;
}