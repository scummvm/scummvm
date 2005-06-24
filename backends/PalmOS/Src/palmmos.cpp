/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "palm.h"

bool OSystem_PALMOS::showMouse(bool visible) {
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

void OSystem_PALMOS::warpMouse(int x, int y) {
	set_mouse_pos(x, y);
}

void OSystem_PALMOS::set_mouse_pos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		undraw_mouse();
	}
}

void OSystem_PALMOS::setMouseCursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y, byte keycolor) {
	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseKeyColor = keycolor;

	if (_mouseDataP)
		free(_mouseDataP);

	_mouseDataP = (byte *)malloc(w * h);
	memcpy(_mouseDataP, buf, w * h);

	undraw_mouse();
}

void OSystem_PALMOS::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible)
		return;

	_mouseCurState.y = _mouseCurState.y >= _screenHeight ? _screenHeight - 1 : _mouseCurState.y;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	byte *src = _mouseDataP;		// Image representing the mouse
	byte *bak = _mouseBackupP;		// Surface used to backup the area obscured by the mouse
	byte *dst;						// Surface we are drawing into
	int width;

	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
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

	// Draw the mouse cursor; backup the covered area in "bak"
	if (!_overlayVisible) {
		dst = _offScreenP + y * _offScreenPitch + x;

		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			dst += _offScreenPitch - w;
			h--;
		}
	} else {
		int offset = y * _screenWidth + x;
		dst = _tmpScreenP;
		
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *(dst + offset);
				color = *src++;
				if (color != _mouseKeyColor)	// transparent, don't draw
					DmWrite(dst, offset, &color, 1);
				offset++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += MAX_MOUSE_W - w;
			offset += _screenWidth - w;
			h--;
		}
	}

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_PALMOS::undraw_mouse() {
	if (!_mouseDrawn)
		return;

	_mouseDrawn = false;

	byte *dst, *bak = _mouseBackupP;
	const int old_mouse_x = _mouseOldState.x;
	const int old_mouse_y = _mouseOldState.y;
	const int old_mouse_w = _mouseOldState.w;
	int old_mouse_h = _mouseOldState.h;

	// No need to do clipping here, since draw_mouse() did that already
	if (!_overlayVisible) {
		dst = _offScreenP + old_mouse_y * _offScreenPitch + old_mouse_x;
		do {
			memcpy(dst, bak, old_mouse_w);
			bak += MAX_MOUSE_W;
			dst += _offScreenPitch;
		} while (--old_mouse_h);

	} else {
		int offset = old_mouse_y * _screenWidth + old_mouse_x;
		do {
			DmWrite(_tmpScreenP, offset, bak, old_mouse_w);
			bak += MAX_MOUSE_W;
			offset += _screenWidth;
		} while (--old_mouse_h);
	}
}

