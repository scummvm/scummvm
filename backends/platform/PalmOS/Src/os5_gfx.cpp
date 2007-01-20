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
#include "graphics/surface.h"
#include "common/config-manager.h"
#include <PenInputMgr.h>
#include <palmOneResources.h>

#ifdef PALMOS_ARM
#include <System/WIP.h>
#include <Libraries/AIA/palmOneStatusBarMgrARM.h>
#endif

#include "oscalls.h"

void OSystem_PalmOS5::int_initSize(uint w, uint h) {
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

	// get command line config
//	_fullscreen = ConfMan.getBool("fullscreen");	// TODO : (NORMAL mode)
	_ratio.adjustAspect = ConfMan.getBool("aspect_ratio") ? kRatioHeight : kRatioNone;

	// precalc ratio (WIDE mode)
	_ratio.width = (gVars->screenFullHeight * _screenWidth / _screenHeight);
	_ratio.height = (gVars->screenFullWidth * _screenHeight / _screenWidth);

	_mouseBackupP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H * 2); // *2 if 16bit
	_mouseDataP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H);
	_offScreenP	= (byte *)malloc(_screenWidth * _screenHeight);

	MemSet(_offScreenP, _screenWidth * _screenHeight, 0);
	MemSet(_nativePal, sizeof(_nativePal), 0);
	MemSet(_currentPalette, sizeof(_currentPalette), 0);

	UInt32 depth = 16;
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	clearScreen();

	gVars->indicator.on = RGBToColor(0,255,0);
	gVars->indicator.off = RGBToColor(0,0,0);

	_overlayH =  alloc_screen(_screenWidth, _screenHeight);
	_overlayP = (OverlayColor *)(BmpGetBits(WinGetBitmap(_overlayH)));

	_screenH = WinGetDisplayWindow();
	_screenP = (byte *)(BmpGetBits(WinGetBitmap(_screenH)));

	_isSwitchable = OPTIONS_TST(kOptModeLandscape) && OPTIONS_TST(kOptCollapsible);
	if (!_isSwitchable)
		_mode = GFX_NORMAL;

	hotswap_gfx_mode(_mode);
}

void OSystem_PalmOS5::hotswap_gfx_mode(int mode) {
	Err e;

	if (_mode != GFX_NORMAL && !_isSwitchable)
		return;

#ifdef PALMOS_ARM
	UInt32 device;
	Boolean isT3 = false;
	if (!FtrGet(sysFileCSystem, sysFtrNumOEMDeviceID, &device))
		isT3 = (device == kPalmOneDeviceIDTungstenT3);
#endif

	if (_workScreenH)
		WinDeleteWindow(_workScreenH, false);
	_workScreenH = NULL;

	_screenDest.w = _screenWidth;
	_screenDest.h = _screenHeight;

	// prevent bad DIA redraw (Stat part)
	if (mode == GFX_NORMAL) {
		_redawOSD = true;
		_stretched = (_screenWidth > gVars->screenWidth);

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

		if (_stretched) {
			calc_rect(false);
		} else {
			// offsets
			_screenOffset.x = (gVars->screenWidth - _screenWidth) / 2;
			_screenOffset.y = (gVars->screenHeight - _screenHeight) / 2;
		}

	} else {
		_redawOSD = false;
		_stretched = true;

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

		calc_rect(true);
	}

	if (_stretched) {
		calc_scale();
		OPTIONS_SET(kOptDisableOnScrDisp);
		_render = &OSystem_PalmOS5::render_landscapeAny;
	} else {
		OPTIONS_RST(kOptDisableOnScrDisp);
		_render = &OSystem_PalmOS5::render_1x;
	}

	_workScreenH = alloc_screen(_screenDest.w, _screenDest.h);
	_workScreenP = (int16 *)(BmpGetBits(WinGetBitmap(_workScreenH)));
	MemSet(_workScreenP, _screenDest.w * _screenDest.h * 2, 0);

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

bool OSystem_PalmOS5::grabRawScreen(Graphics::Surface *surf) {
	assert(surf);

	surf->create(_screenWidth, _screenHeight, 1);
	MemMove(surf->pixels, _offScreenP, _screenWidth * _screenHeight);
	
	return true;
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
	_nativePal[index] = gfxMakeDisplayRGB(r, g, b);
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
