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

#include "be_zodiac.h"

void OSystem_PalmZodiac::draw_mouse() {
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
		int16 *dst;

		TwGfxLockSurface(_overlayP, (void **)&dst);
		dst += y * _screenWidth + x;

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

		TwGfxUnlockSurface(_overlayP, true);

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

void OSystem_PalmZodiac::undraw_mouse() {
	if (!_mouseDrawn)
		return;

	int h = _mouseOldState.h;

	// no need to do clipping here, since draw_mouse() did that already
	if (_overlayVisible) {
		uint16 *dst;
		uint16 *bak = (uint16 *)_mouseBackupP;

		TwGfxLockSurface(_overlayP, (void **)&dst);
		dst += _mouseOldState.y * _screenWidth + _mouseOldState.x;

		do {
			MemMove(dst, bak, _mouseOldState.w * 2);
			dst += _screenWidth;
			bak += _mouseOldState.w;
		} while (--h);

		TwGfxUnlockSurface(_overlayP, true);

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
