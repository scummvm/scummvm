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

#include "be_os5.h"
#include "common/endian.h"

void OSystem_PalmOS5::showOverlay() {
	// hide fight indicator
	draw_osd(kDrawFight, _screenDest.w - 34, _screenDest.h + 2, false);

	undraw_mouse();
	_overlayVisible = true;
	clearOverlay();
}

void OSystem_PalmOS5::hideOverlay() {
	undraw_mouse();
	_overlayVisible = false;
	_redawOSD = true;
}

void OSystem_PalmOS5::clearOverlay() {
	if (!_overlayVisible)
		return;

	byte *src = _offScreenP;
	int16 *dst =  _overlayP;
	int cnt = _screenWidth * _screenHeight;
	do {
		*dst++ = _nativePal[*src++];
	} while (--cnt);
}

void OSystem_PalmOS5::grabOverlay(OverlayColor *buf, int pitch) {
	OverlayColor *src = _overlayP;
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth * 2);
		src += _screenWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_PalmOS5::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	if (w == 0 || h == 0)
		return;

	OverlayColor *dst = _overlayP + x + y * _screenWidth;
	do {
		memcpy(dst, buf, w * 2);
		dst += _screenWidth;
		buf += pitch;
	} while (--h);
}
