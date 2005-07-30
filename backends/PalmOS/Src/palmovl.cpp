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

void OSystem_PALMOS::showOverlay() {
	// hide the mouse
	undraw_mouse();
	// save background
	byte *src = _offScreenP;
	byte *dst = _tmpBackupP;
	int h = _screenHeight;

	UInt32 offset = 0;
	do {
		DmWrite(dst, offset, src, _screenWidth);
		offset += _screenWidth;
		src += _offScreenPitch;
	} while (--h);

	_overlayVisible = true;
	clearOverlay();
}

void OSystem_PALMOS::hideOverlay() {
	// hide the mouse
	undraw_mouse();

	_overlayVisible = false;
	copyRectToScreen(_tmpBackupP, _screenWidth, 0, 0, _screenWidth, _screenHeight);
}

void OSystem_PALMOS::clearOverlay() {
	if (!_overlayVisible)
		return;

	// hide the mouse
	undraw_mouse();
	// restore background
	DmWrite(_tmpScreenP, 0, _tmpBackupP, _screenWidth * _screenHeight);
}

void OSystem_PALMOS::grabOverlay(byte *buf, int pitch) {
	if (!_overlayVisible)
		return;

	// hide the mouse
	undraw_mouse();

	byte *src = _tmpScreenP;
	int h = _screenHeight;

	do {
		memcpy(buf, src, _screenWidth);
		src += _screenWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_PALMOS::copyRectToOverlay(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (!_overlayVisible)
		return;

	if (!_tmpScreenP)
		return;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y; buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (h > _screenHeight - y)
		h = _screenHeight - y;

	if (w <= 0 || h <= 0)
		return;

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn) {
/*		RectangleType mouse = {_mouseCurState.x, _mouseCurState.y, _mouseCurState.w, _mouseCurState.h};
		RectangleType copy	= {x, y, w, h};
		RectangleType result;

		RctGetIntersection(&mouse, &copy, &result);

		if (result.extent.x !=0 && result.extent.y !=0)*/
			undraw_mouse();
	}

	byte *dst = _tmpScreenP;
	int offset = y * _screenWidth + x;
	do {
		DmWrite(dst, offset, buf, w);
		offset += _screenWidth;
		buf += pitch;
	} while (--h);
}
