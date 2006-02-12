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
 * $Header: /cvsroot/scummvm/scummvm/backends/PalmOS/Src/zodiac_gfx.cpp,v 1.4 2005/11/05 11:18:56 chrilith Exp $
 *
 */

#include "be_zodiac.h"
#include "common/config-manager.h"
#include "graphics/surface.h"
#include "rumble.h"

#ifdef PALMOS_68K
#define _TwGfxOpen(x, y)	TwGfxOpen((TwGfxHandle*)x, y);
#else
static asm Err _TwGfxOpen(void **aResult, void *aInfoResult) {
	stmfd sp!, {r4-r11,lr}
	ldr r9, [r9]
	ldr r9, [r9]
	sub sp, sp, #0x24
	mov r6, r0
	mov r7, r1
	ldr pc, =0x200995F0
}
#endif

int OSystem_PalmZodiac::getDefaultGraphicsMode() const {
	return GFX_WIDE;
}

void OSystem_PalmZodiac::load_gfx_mode() {
	Err e;

	if (_gfxLoaded)
		return;
	_gfxLoaded = true;

	// get command line config
	_fullscreen = ConfMan.getBool("fullscreen");	// TODO : (NORMAL mode)
	_ratio.adjustAspect = ConfMan.getBool("aspect_ratio") ? kRatioHeight : kRatioNone;

	// precalc ratio (WIDE mode)
	_ratio.width = ((float)_screenWidth / _screenHeight * gVars->screenFullHeight);
	_ratio.height = ((float)_screenHeight / _screenWidth * gVars->screenFullWidth);

	_sysOldOrientation = SysGetOrientation();
	SysSetOrientation(sysOrientationLandscape);

	_mouseBackupP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H * 2); // *2 if 16bit
	_mouseDataP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H);
	_offScreenP = (byte *)MemPtrNew(_screenWidth * _screenHeight);

	MemSet(_offScreenP, _screenWidth * _screenHeight, 0);
	MemSet(_nativePal, sizeof(_nativePal), 0);
	MemSet(_currentPalette, sizeof(_currentPalette), 0);

	UInt32 depth = 16;
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);

	gVars->indicator.on = RGBToColor(0,255,0);
	gVars->indicator.off = RGBToColor(0,0,0);

	_screenH = WinGetDisplayWindow();
	_screenP = (byte *)BmpGetBits(WinGetBitmap(_screenH));

	e = _TwGfxOpen((void **)&_gfxH, 0);
	e = TwGfxGetPalmDisplaySurface(_gfxH, &_palmScreenP);

	// overlay buffer
	TwGfxSurfaceInfoType nfo = {
		sizeof(TwGfxSurfaceInfoType),
		_screenWidth, _screenHeight, _screenWidth * 2,
		twGfxLocationAcceleratorMemory,
		twGfxPixelFormatRGB565_LE
	};
	e = TwGfxAllocSurface(_gfxH, &_overlayP, &nfo);

	// prepare main bitmap
	_srcBmp.size = sizeof(TwGfxBitmapType);
	_srcBmp.width = _screenWidth;
	_srcBmp.height = _screenHeight;
	_srcBmp.rowBytes = _screenWidth;
	_srcBmp.pixelFormat = twGfxPixelFormat8bpp;
	_srcBmp.data = _offScreenP;
	_srcBmp.palette = _nativePal;

	_srcRect.x = 0;
	_srcRect.y = 0;
	_srcRect.w = _screenWidth;
	_srcRect.h = _screenHeight;

	hotswap_gfx_mode(_mode);
}

void OSystem_PalmZodiac::hotswap_gfx_mode(int mode) {
	Err e;
	TwGfxSetClip(_palmScreenP, 0);
	WinSetDrawWindow(_screenH);

	_screenDest.w = _screenWidth;
	_screenDest.h = _screenHeight;

	// prevent bad DIA redraw (Stat part)
	if (mode  == GFX_NORMAL) {
		_redawOSD = true;
		_stretched = (_screenWidth > gVars->screenWidth);
		StatShow();
		PINSetInputAreaState(pinInputAreaOpen);

		if (_stretched) {
			calc_rect(false);
		} else {
			// offsets
			_screenOffset.x = (gVars->screenWidth - _screenWidth) / 2;
			_screenOffset.y = (gVars->screenHeight - _screenHeight) / 2;

			// clip Tapwave API
			TwGfxRectType rt = { _screenOffset.x, _screenOffset.y, _screenWidth, _screenHeight };
			TwGfxSetClip(_palmScreenP, &rt);
		}

	} else {
		_redawOSD = false;
		_stretched = true;
		PINSetInputAreaState(pinInputAreaClosed);
		StatHide();

		calc_rect(true);
	}

	if (_stretched) {
		OPTIONS_SET(kOptDisableOnScrDisp);
		TwGfxSetClip(_palmScreenP, &_dstRect);

		if (!_tmpScreenP) {
			// wide buffer
			TwGfxSurfaceInfoType nfo = {
				sizeof(TwGfxSurfaceInfoType),
				_screenWidth, _screenHeight, _screenWidth * 2,
				twGfxLocationAcceleratorMemory,
				twGfxPixelFormatRGB565_LE
			};
			e = TwGfxAllocSurface(_gfxH, &_tmpScreenP, &nfo);
		}
	} else {
		OPTIONS_RST(kOptDisableOnScrDisp);
		if (_tmpScreenP) {
			e = TwGfxFreeSurface(_tmpScreenP);
			_tmpScreenP = NULL;
		}
	}

	_mode = mode;
	_srcPos.x = _screenOffset.x;
	_srcPos.y = _screenOffset.y;
	clearScreen();
//	updateScreen();
}

void OSystem_PalmZodiac::unload_gfx_mode() {
	Err e;

	if (!_gfxLoaded)
		return;	
	_gfxLoaded = false;

	if (_tmpScreenP) {
		e = TwGfxFreeSurface(_tmpScreenP);
		_tmpScreenP = NULL;
	}

	e = TwGfxFreeSurface(_overlayP);
	e = TwGfxClose(_gfxH);

	UInt32 depth = 8;		
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	clearScreen();

	MemPtrFree(_mouseBackupP);
	MemPtrFree(_mouseDataP);
	MemPtrFree(_offScreenP);

	SysSetOrientation(_sysOldOrientation);
	StatShow();
	PINSetInputAreaState(pinInputAreaOpen);
}

static void rumblePack(Boolean active) {
	if (!gVars->vibrator)
		return;

	RumbleRun(active);
}

void OSystem_PalmZodiac::int_setShakePos(int shakeOffset) {
	if (shakeOffset == 0)
		rumblePack(false);
}

void OSystem_PalmZodiac::updateScreen() {
	Err e;

	// draw the mouse pointer
	draw_mouse();		
	// update the screen
	if (_overlayVisible) {
		if (_stretched) {
			TwGfxRectType dst = {_dstRect.x, _dstRect.y, _dstRect.w, _dstRect.h};
			e = TwGfxStretchBlt2(_palmScreenP, &dst, _overlayP, &_srcRect, twGfxStretchFast| (gVars->filter ? twGfxStretchSmooth : 0));
		} else {
			e = TwGfxBitBlt(_palmScreenP, &_srcPos, _overlayP, &_srcRect);
		}

	} else {
		if (_stretched) {
			TwGfxPointType pos = {0, 0};
			TwGfxRectType dst = {_dstRect.x, _dstRect.y, _dstRect.w, _dstRect.h};

			if (_new_shake_pos != _current_shake_pos) {
				TwGfxRectType r = { _screenOffset.x, _screenOffset.y, _dstRect.w, _new_shake_pos };

				if (_new_shake_pos != 0) {
					TwGfxFillRect(_palmScreenP, &r, 0);
					rumblePack(_new_shake_pos >= 3);
				}
				r.y += dst.h;
				r.h = _current_shake_pos;
				TwGfxFillRect(_palmScreenP, &r, 0);

				_current_shake_pos = _new_shake_pos;
				dst.y += _new_shake_pos;
			}
			e = TwGfxDrawBitmap(_tmpScreenP, &pos, &_srcBmp);
			e = TwGfxStretchBlt2(_palmScreenP, &dst, _tmpScreenP, &_srcRect, twGfxStretchFast| (gVars->filter ? twGfxStretchSmooth : 0)); 
			
		} else {
			TwGfxPointType pos = {_srcPos.x, _srcPos.y};
		
			if (_new_shake_pos != _current_shake_pos) {
				if (_new_shake_pos != 0) {
					TwGfxRectType r = { _screenOffset.x, _screenOffset.y, _screenWidth, _new_shake_pos };
					TwGfxFillRect(_palmScreenP, &r, 0);
					rumblePack(_new_shake_pos >= 3);
				}
				_current_shake_pos = _new_shake_pos;
				pos.y += _new_shake_pos;
			}
			e = TwGfxDrawBitmap(_palmScreenP, &pos, &_srcBmp);
		}
	}
	undraw_mouse();
}

bool OSystem_PalmZodiac::grabRawScreen(Graphics::Surface *surf) {
	assert(surf);
	
	surf->create(_screenWidth, _screenHeight, 1);
	MemMove(surf->pixels, _offScreenP, _screenWidth * _screenHeight);
	
	return true;
}

void OSystem_PalmZodiac::extras_palette(uint8 index, uint8 r, uint8 g, uint8 b) {
	_nativePal[index] = TwGfxMakeDisplayRGB( r, g, b);
}

void OSystem_PalmZodiac::draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color) {
	if (_mode != GFX_NORMAL)
		return;
	MemHandle hTemp = DmGetResource(bitmapRsc, id + 100);

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
		WinGetBitmapDimensions(bmTemp, &w, &h);	// return the size of the low density bmp

		PointType dst = { _screenOffset.x + x, _screenOffset.y + y };
		RectangleType c, r = { dst.x, dst.y, w * 2, h * 2 };

		UInt16 old = WinSetCoordinateSystem(kCoordinatesNative);
		WinSetDrawWindow(_screenH);
		if (show) {
			WinSetForeColorRGB(&pal[3], &oldRGB);
			WinSetBackColorRGB(&pal[color], &oldRGB);
			WinPaintBitmap(bmTemp, dst.x, dst.y);
		} else {
			WinSetBackColorRGB(&pal[3], &oldRGB);
			WinFillRectangle(&r, 0);
		}
		WinSetCoordinateSystem(old);

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
	}
}
