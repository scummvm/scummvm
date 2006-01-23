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
 * $Header$
 *
 */

#include "be_os5.h"

#ifndef __TWKEYS_H__
#include <PalmNavigator.h>
#include <HsKeyCommon.h>
#endif

OSystem_PalmOS5::OSystem_PalmOS5() : OSystem_PalmBase() {
	_sound.active = false;
	_setPalette = false;
	
	_workScreenH = NULL;
	_overlayH = NULL;
	_isSwitchable = false;
	_wasRotated = false;

	MemSet(&_soundEx, sizeof(SoundExType), 0);
	_soundEx.sound = &_sound;
}

void OSystem_PalmOS5::int_initBackend() {
	if (OPTIONS_TST(kOpt5WayNavigatorV1)) {
		_keyMouse.bitUp		= keyBitPageUp;
		_keyMouse.bitDown	= keyBitPageDown;
		_keyMouse.bitLeft	= keyBitNavLeft;
		_keyMouse.bitRight	= keyBitNavRight;
		_keyMouse.bitButLeft= keyBitNavSelect;
		_keyMouse.hasMore	= true;

	} else if (OPTIONS_TST(kOpt5WayNavigatorV2)) {
		_keyMouse.bitUp		= keyBitRockerUp|keyBitPageUp;
		_keyMouse.bitDown	= keyBitRockerDown|keyBitPageDown;
		_keyMouse.bitLeft	= keyBitRockerLeft;
		_keyMouse.bitRight	= keyBitRockerRight;
		_keyMouse.bitButLeft= keyBitRockerCenter;
		_keyMouse.hasMore	= true;
	}
}

void OSystem_PalmOS5::setWindowCaption(const char *caption) {
	Err e;
	Char buf[64];
	Coord w, y, h = FntLineHeight() + 2;
	const Char *loading = "Loading, please wait\0";

	// allocate bitmap
	BitmapTypeV3 *bmp2P;
	BitmapType *bmp1P = BmpCreate(320, (h * 3), 8, NULL, &e);
	WinHandle tmpH = WinCreateBitmapWindow(bmp1P, &e);

	WinSetDrawWindow(tmpH);
	WinSetBackColor(0);
	WinSetTextColor(255);
	WinEraseWindow();

	// loading message
	FntSetFont(boldFont);
	w = FntCharsWidth(loading, StrLen(loading));
	w = (320 - w) / 2;
	WinDrawChars(loading, StrLen(loading), w, 0 + h);

	// caption
	FntSetFont(stdFont);
	w = FntCharsWidth(caption, StrLen(caption));
	w = (320 - w) / 2;
	WinDrawChars(caption, StrLen(caption), w, 0);

	// memory size
	StrPrintF(buf, "memory : %ld KB", gVars->startupMemory);
	w = FntCharsWidth(buf, StrLen(buf));
	w = (320 - w) / 2;
	WinDrawChars(buf, StrLen(buf), w, h * 2);

	// set the bitmap as v3
	bmp2P = BmpCreateBitmapV3(bmp1P, kDensityDouble, BmpGetBits(bmp1P), NULL);
	y = (80 - (h / 4) - 5);

	// draw it
	WinSetDrawWindow(WinGetDisplayWindow());
	WinEraseWindow();
	WinDrawBitmap((BitmapPtr)bmp2P, 0, y);

	// free
	WinDeleteWindow(tmpH, 0);
	BmpDelete((BitmapPtr)bmp2P);
	BmpDelete(bmp1P);
}
