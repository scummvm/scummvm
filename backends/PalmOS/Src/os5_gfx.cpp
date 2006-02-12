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
#include <PenInputMgr.h>
#include <palmOneResources.h>

#ifdef PALMOS_ARM
#include <System/WIP.h>
#include <Libraries/AIA/palmOneStatusBarMgrARM.h>
#endif

#include "oscalls.h"

void OSystem_PalmOS5::int_initSize(uint w, uint h, int overlayScale) {
}

WinHandle OSystem_PalmOS5::alloc_screen(Coord w, Coord h) {
	Err e;
	WinHandle winH;
	UInt16 old = WinSetCoordinateSystem(kCoordinatesNative);
	winH = WinCreateOffscreenWindow(w, h, nativeFormat, &e);
	WinSetCoordinateSystem(old);
	
	return winH;
}

void OSystem_PalmOS5::load_gfx_mode() {
	Err e;

	if (_gfxLoaded)
		return;
	_gfxLoaded = true;


	_mouseBackupP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H * 2); // *2 if 16bit
	_mouseDataP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H);
	_offScreenP	= (byte *)malloc(_screenWidth * _screenHeight);

	UInt32 depth = 16;
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	clearScreen();

	gVars->indicator.on = RGBToColor(0,255,0);
	gVars->indicator.off = RGBToColor(0,0,0);

	_overlayH =  alloc_screen(_screenWidth, _screenHeight);
	_screenH = WinGetDisplayWindow();

	_overlayP = (OverlayColor *)(BmpGetBits(WinGetBitmap(_overlayH)));
	_screenP = (byte *)(BmpGetBits(WinGetBitmap(_screenH)));

	MemSet(_offScreenP, _screenWidth * _screenHeight, 0);
	MemSet(_nativePal, sizeof(_nativePal), 0);
	MemSet(_currentPalette, sizeof(_currentPalette), 0);

	_isSwitchable = (_screenWidth == 320 && _screenHeight == 200 && OPTIONS_TST(kOptCollapsible));
	if (_screenWidth > 320 || _screenHeight > 200 || !_isSwitchable)
		_mode = GFX_NORMAL;

	hotswap_gfx_mode(_mode);
}

void OSystem_PalmOS5::hotswap_gfx_mode(int mode) {
	Err e;
	UInt32 device;

	if (_mode != GFX_NORMAL && !_isSwitchable)
		return;

	if (_workScreenH)
		WinDeleteWindow(_workScreenH, false);
	_workScreenH = NULL;

#ifdef PALMOS_ARM
	Boolean isT3 = false;
	if (!FtrGet(sysFileCSystem, sysFtrNumOEMDeviceID, &device))
		isT3 = (device == kPalmOneDeviceIDTungstenT3);
#endif

	// prevent bad DIA redraw (Stat part)
	if (mode  == GFX_NORMAL) {		
		// only if this API is available
		if (_stretched && OPTIONS_TST(kOptCollapsible)) {
#ifdef PALMOS_ARM
			if (isT3) {
				//AiaSetInputAreaState(aiaInputAreaShow);
				StatShow_68k();
				PINSetInputAreaState_68k(pinInputAreaOpen);
			} else
#endif
			{
				StatShow();
				PINSetInputAreaState(pinInputAreaOpen);
			}
		}

		_redawOSD = true;
		_stretched = false;
		OPTIONS_RST(kOptDisableOnScrDisp);
		_screenDest.w = _screenWidth;
		_screenDest.h = _screenHeight;

		if (_wasRotated) {
			// restore controls rotation
			SWAP(_keyMouse.bitLeft, _keyMouse.bitRight);
			SWAP(_keyMouse.bitRight, _keyMouse.bitDown);
			SWAP(_keyMouse.bitLeft, _keyMouse.bitUp);
			_wasRotated = false;
		}

		_workScreenH = alloc_screen(_screenWidth, _screenHeight);
		_workScreenP = (int16 *)(BmpGetBits(WinGetBitmap(_workScreenH)));
		MemSet(_workScreenP, _screenWidth * _screenHeight * 2, 0);

		_screenOffset.x = (gVars->screenWidth - _screenWidth) / 2;
		_screenOffset.y = (gVars->screenHeight - _screenHeight)  / 2;
		
		_render = &OSystem_PalmOS5::render_1x;

	} else {
#ifdef PALMOS_ARM
		// T3 DIA library is 68k base, there is no possible native call
		if (isT3) {
			//AiaSetInputAreaState(aiaInputAreaFullScreen);
			PINSetInputAreaState_68k(pinInputAreaClosed);
			StatHide_68k();
		} else
#endif
		{
			PINSetInputAreaState(pinInputAreaClosed);
			StatHide();
		}

		_redawOSD = false;
		_stretched = true;
		OPTIONS_SET(kOptDisableOnScrDisp);

		if (OPTIONS_TST(kOptModeLandscape)) {
			_screenDest.w = 480;
			_screenDest.h = 300;
			_workScreenH = alloc_screen(480, 300);
			_render = &OSystem_PalmOS5::render_landscape;

		} else {
			_screenDest.w = 300;
			_screenDest.h = 480;
			_workScreenH = alloc_screen(300, 480);
			_render = &OSystem_PalmOS5::render_portrait;
			// This mode need a controls rotation
			SWAP(_keyMouse.bitLeft, _keyMouse.bitUp);
			SWAP(_keyMouse.bitRight, _keyMouse.bitDown);
			SWAP(_keyMouse.bitLeft, _keyMouse.bitRight);
			_wasRotated = true;
		}

		_workScreenP = (int16 *)(BmpGetBits(WinGetBitmap(_workScreenH)));
		MemSet(_workScreenP, 480 * 300 * 2, 0);

		_screenOffset.x = 0;
		_screenOffset.y = 10;
	}

	_mode = mode;
	clearScreen();
}

void OSystem_PalmOS5::unload_gfx_mode() {
	if (!_gfxLoaded)
		return;	
	_gfxLoaded = false;
	
	MemPtrFree(_mouseBackupP);
	MemPtrFree(_mouseDataP);
	free(_offScreenP);

	if (_workScreenH)
		WinDeleteWindow(_workScreenH, false);
	if (_overlayH)
		WinDeleteWindow(_overlayH, false);
		
	_workScreenH = NULL;
	_overlayH = NULL;

	UInt32 depth = 8;
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	clearScreen();

	WinSetCoordinateSystem(_sysOldCoord);
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

	byte *dst = _offScreenP + y * _screenWidth + x;

	if (w == pitch && w == _screenWidth) {
		MemMove(dst, buf, w * h);
	} else {
		do {
			MemMove(dst, buf, w);
			dst += _screenWidth;
			buf += pitch;
		} while (--h);
	}
}

void OSystem_PalmOS5::int_updateScreen() {
	RectangleType r;
	PointType p;

	draw_mouse();
	((this)->*(_render))(r, p);

	_sysOldCoord = WinSetCoordinateSystem(kCoordinatesNative);
	WinCopyRectangle(_workScreenH, _screenH, &r, p.x, p.y, winPaint);
	WinSetCoordinateSystem(_sysOldCoord);
	undraw_mouse();
}

void OSystem_PalmOS5::clearScreen() {
	RGBColorType rgb = { 0,0,0,0 };
	WinSetDrawWindow(WinGetDisplayWindow());
	WinSetBackColorRGB(&rgb, 0);
	WinEraseWindow();
}

void OSystem_PalmOS5::extras_palette(uint8 index, uint8 r, uint8 g, uint8 b) {
	_nativePal[index] = gfxMakeDisplayRGB( r, g, b);
}

void OSystem_PalmOS5::draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color) {
	if (_mode != GFX_NORMAL)
		return;
//	MemHandle hTemp = DmGetResource(bitmapRsc, id);
	MemHandle hTemp = DmGetResource('abmp', id + 100);

	if (hTemp) {
		RGBColorType oldRGB;
		static const RGBColorType pal[4] = {
			{0,0,255,0},
			{0,255,255,0},
			{0,255,0,0},
			{0,0,0,0}
		};

		BitmapType *bmTemp;
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);

		Coord w, h;
		BmpGetDimensions(bmTemp, &w, &h, 0);

		PointType dst = { _screenOffset.x + x, _screenOffset.y + y };
		RectangleType c, r = { dst.x, dst.y, w, h };

		UInt16 old = WinSetCoordinateSystem(kCoordinatesNative);
		WinSetDrawWindow(_screenH);
		WinGetClip(&c);
		WinResetClip();

		if (show) {
			WinSetForeColorRGB(&pal[3], &oldRGB);
			WinSetBackColorRGB(&pal[color], &oldRGB);
			WinDrawBitmap(bmTemp, dst.x, dst.y);
		} else {
			WinSetBackColorRGB(&pal[3], &oldRGB);
			WinFillRectangle(&r, 0);
		}

		WinSetClip(&c);
		WinSetCoordinateSystem(old);

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
	}
}
