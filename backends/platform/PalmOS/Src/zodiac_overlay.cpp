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
#include "common/endian.h"

void OSystem_PalmZodiac::clearOverlay() {
	if (!_overlayVisible)
		return;

	Err e;
	TwGfxPointType pos = {0, 0};
	TwGfxBitmapType bmp = {
		sizeof(TwGfxBitmapType),
		_screenWidth, _screenHeight, _screenWidth, twGfxPixelFormat8bpp,
		(void *)_offScreenP, (UInt16 *)_nativePal
	};
	e = TwGfxDrawBitmap(_overlayP, &pos, &bmp);
}

void OSystem_PalmZodiac::grabOverlay(OverlayColor *buf, int pitch) {
	Err e;
	OverlayColor *src;

	e = TwGfxLockSurface(_overlayP, (void **)&src);
	int h = _screenHeight;
	do {
		memcpy(buf, src, _screenWidth * 2);
		src += _screenWidth;
		buf += pitch;
	} while (--h);

	e = TwGfxUnlockSurface(_overlayP, 0);
}

void OSystem_PalmZodiac::copyRectToOverlay(const OverlayColor *buf, int pitch, int x, int y, int w, int h) {
	Err e;
	TwGfxPointType pos = {x, y};
	TwGfxBitmapType bmp = {
		sizeof(TwGfxBitmapType),
		w, h, pitch * 2 , twGfxPixelFormatRGB565_LE,
		(void *)buf, 0
	};
	e = TwGfxDrawBitmap(_overlayP, &pos, &bmp);
}
