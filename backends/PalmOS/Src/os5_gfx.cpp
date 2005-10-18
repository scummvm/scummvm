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

#include "be_os5.h"

#ifdef PALMOS_ARM
#include "pace.h"
#include "oscalls.h"
#endif

void OSystem_PalmOS5::int_initSize(uint w, uint h, int overlayScale) {
}

void OSystem_PalmOS5::load_gfx_mode() {
	Err e;
	
	if (_gfxLoaded)
		return;
	_gfxLoaded = true;

	_sysOldCoord = WinSetCoordinateSystem(kCoordinatesNative);

#ifdef PALMOS_68K
	// init mouse (must be here, after WinSetCoordinateSystem)
	_mouseBackupH = WinCreateOffscreenWindow(MAX_MOUSE_W, MAX_MOUSE_H, nativeFormat, &e);
	_mouseBackupP = (byte *)(BmpGetBits(WinGetBitmap(_mouseBackupH)));

	_offScreenH	= WinCreateOffscreenWindow(_screenWidth, _screenHeight, nativeFormat, &e);
	_screenH = WinGetDisplayWindow();
	_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH)));
	_screenP = (byte *)(BmpGetBits(WinGetBitmap(_screenH)));
#endif

	_screenOffset.x = (gVars->screenWidth - _screenWidth) / 2;
	_screenOffset.y = (gVars->screenHeight - _screenHeight)  / 2;
}

void OSystem_PalmOS5::unload_gfx_mode() {
	if (!_gfxLoaded)
		return;	
	_gfxLoaded = false;
	
	if (_mouseBackupH)
		WinDeleteWindow(_mouseBackupH, false);
}

void OSystem_PalmOS5::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	// Clip the coordinates
	if (x < 0) {
		w += x;
		buf -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		buf -= y * pitch;
		y = 0;
	}

	if (w > _screenWidth - x)
		w = _screenWidth - x;

	if (h > _screenHeight - y)
		h = _screenHeight - y;

	if (w <= 0 || h <= 0)
		return;

#ifdef PALMOS_68K
	BitmapTypeV1 nfo = {
		w, h, pitch,
		{0,0,0,0,0,0,0,0,0},
		8, BitmapVersionOne, 0
	};
	
	BitmapTypeV3 *v3 = BmpCreateBitmapV3((BitmapType*)&nfo, kDensityDouble, (void *)buf, 0);

	WinSetDrawWindow(_offScreenH);
	WinDrawBitmap((BitmapPtr)v3, x, y);
	BmpDelete((BitmapPtr)v3);
#else
	byte *dst = _offScreenP + y * _screenWidth + x;

	if (w == pitch && w == _screenWidth) {
		MemMove(dst, buf, w*h);
	} else {
		do {
			MemMove(dst, buf, w);
			dst += _screenWidth;
			buf += pitch;
		} while (--h);
	}
#endif
}

void OSystem_PalmOS5::int_updateScreen() {
#ifdef PALMOS_68K
	RectangleType r;
	RctSetRectangle(&r, 0, 0, _screenWidth, _screenHeight - _current_shake_pos);
	WinCopyRectangle(_offScreenH, _screenH, &r, _screenOffset.x, _screenOffset.y + _current_shake_pos, winPaint);
#endif
}

void OSystem_PalmOS5::clearScreen() {
	RGBColorType rgb = { 0,0,0,0 };
	WinSetDrawWindow(WinGetDisplayWindow());
	WinSetBackColorRGB(&rgb, 0);
	WinEraseWindow();
}
