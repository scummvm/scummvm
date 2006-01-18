/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "be_zodiac.h"
#include "common/config-manager.h"
#include "rumble.h"

#ifdef PALMOS_68K
#include "globals.h"
#include "arm/native.h"
#include "arm/macros.h"

static Err _TwGfxOpen(void** aResult, void* aInfoResult) {
#if !defined(SIMULATOR_MODE)
		ARM_START(TwHackGfxType)
			ARM_INIT(COMMON_ZODIACGFX)
			ARM_ADDM(aResult)
			ARM_ADDM(aInfoResult)
			ARM_CALL_RET(ARM_COMMON, PNO_DATA())
		ARM_END_RET(Err)
#endif
		return TwGfxOpen((TwGfxType**)aResult, (TwGfxInfoType *)aInfoResult);
}

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

void OSystem_PalmZodiac::load_gfx_mode() {
	Err e;
	
	if (_gfxLoaded)
		return;
	_gfxLoaded = true;

	OPTIONS_RST(kOptDisableOnScrDisp); // TODO

	// get command line config
	_fullscreen = ConfMan.getBool("fullscreen");	// (NORMAL mode)
	_ratio.adjustAspect = ConfMan.getBool("aspect_ratio") ? kRatioHeight : kRatioNone;

	// precalc ratio (WIDE mode)
	_ratio.width = ((float)_screenWidth / _screenHeight * gVars->screenFullHeight);
	_ratio.height = ((float)_screenHeight / _screenWidth * gVars->screenFullWidth);

	_sysOldCoord = WinSetCoordinateSystem(kCoordinatesNative);
	_sysOldOrientation = SysGetOrientation();
	SysSetOrientation(sysOrientationLandscape);

#ifdef PALMOS_68K
	// init mouse (must be here, after WinSetCoordinateSystem)
	_mouseBackupH = WinCreateOffscreenWindow(MAX_MOUSE_W, MAX_MOUSE_H, nativeFormat, &e);
	_mouseBackupP = (byte *)(BmpGetBits(WinGetBitmap(_mouseBackupH)));

	_mouseDataH = WinCreateOffscreenWindow(MAX_MOUSE_W, MAX_MOUSE_H, nativeFormat, &e);
	_mouseDataP = (byte *)BmpGetBits(WinGetBitmap(_mouseDataH));

	_offScreenH	= WinCreateOffscreenWindow(_screenWidth, _screenHeight, nativeFormat, &e);
	_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH)));
#else
	_mouseBackupP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H);
	_mouseDataP = (byte *)MemPtrNew(MAX_MOUSE_W * MAX_MOUSE_H);
	_offScreenP = (byte *)MemPtrNew(_screenWidth * _screenHeight);
#endif

	MemSet(_offScreenP, _screenWidth * _screenHeight, 0);
	MemSet(_nativePal, sizeof(_nativePal), 0);
	MemSet(_currentPalette, sizeof(_currentPalette), 0);

	UInt32 depth = 16;		
	WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);
	_screenH = WinGetDisplayWindow();
	_screenP = (byte *)WinScreenLock(winLockDontCare);
	WinScreenUnlock();

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
	WinSetCoordinateSystem(kCoordinatesStandard);
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
	WinSetCoordinateSystem(kCoordinatesNative);

	if (_stretched) {		
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
		if (_tmpScreenP) {
			e = TwGfxFreeSurface(_tmpScreenP);
			_tmpScreenP = NULL;
		}
	}

	_mode = mode;
	_srcPos.x = _screenOffset.x;
	_srcPos.y = _screenOffset.y;
	clearScreen();
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

#ifdef PALMOS_68K
	WinDeleteWindow(_mouseBackupH, 0);
	WinDeleteWindow(_mouseDataH, 0);
	WinDeleteWindow(_offScreenH, 0);
#else
	MemPtrFree(_mouseBackupP);
	MemPtrFree(_mouseDataP);
	MemPtrFree(_offScreenP);
#endif

	SysSetOrientation(_sysOldOrientation);
	WinSetCoordinateSystem(_sysOldCoord);
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

	if (_redawOSD) {
		_redawOSD = false;
		if (!_overlayVisible)
			draw_osd(kDrawFight, _screenDest.w - 34, _screenDest.h + 2, _useNumPad, 1);
		// + battery
	}

	if (_overlayVisible) {
		// update the palette only with overlay, since it is in 16bit mode, this
		// is needed so that the cursor has the correct colors. Hopefully Zodiac
		// set the palette even in 16bit mode, where other OS5 devices don't :(
		// FIXME : seems to corrupt the Stat grafiti panel
		if (_paletteDirtyEnd != 0) {
			WinSetDrawWindow(_screenH);
// no cursor for now
//			WinPalette(winPaletteSet, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart, _currentPalette + _paletteDirtyStart);
			_paletteDirtyEnd = 0;
		}

		if (_stretched) {
			TwGfxRectType dst = {_dstRect.x, _dstRect.y, _dstRect.w, _dstRect.h};
			// TODO : where is the mouse :)
			e = TwGfxStretchBlt2(_palmScreenP, &dst, _overlayP, &_srcRect, twGfxStretchFast| (gVars->filter ? twGfxStretchSmooth : 0)); 

		} else {
			// not really time sensitive, can use WinScreenLock/Unlock, this prevent use
			// of a additional backpup buffer to draw the mouse
			WinScreenLock(winLockCopy);
			e = TwGfxBitBlt(_palmScreenP, &_srcPos, _overlayP, &_srcRect);
			draw_mouse();
			WinScreenUnlock();
		}

	} else {
		draw_mouse();		
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
		undraw_mouse();
	}
}

void OSystem_PalmZodiac::extras_palette(uint8 index, uint8 r, uint8 g, uint8 b) {
	_nativePal[index] = TwGfxMakeDisplayRGB( r, g, b);
}

void OSystem_PalmZodiac::draw_osd(UInt16 id, Int32 x, Int32 y, Boolean show, UInt8 color) {
	if (_mode != GFX_NORMAL)
		return;

	MemHandle hTemp = DmGetResource(bitmapRsc, id + 100);

	if (hTemp) {
		static const UInt32 pal[2] = {
			(TwGfxComponentsToPackedRGB(0,255,0)),
			(TwGfxComponentsToPackedRGB(255,0,0))
		};
	
		BitmapType *bmTemp;
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);

		Coord w, h;
		BmpGetDimensions(bmTemp, &w, &h, 0);
		TwGfxPointType dst = { _screenOffset.x + x, _screenOffset.y + y };
		TwGfxRectType r = { dst.x, dst.y, w, h };

		TwGfxRectType c;
		TwGfxGetClip(_palmScreenP, &c);
		TwGfxSetClip(_palmScreenP, 0);
		if (show) {
			WinSetDrawWindow(_screenH);
			TwGfxFillRect(_palmScreenP, &r, pal[color]);
			WinSetDrawMode(winOverlay);
			WinPaintBitmap(bmTemp,dst.x, dst.y);
			WinSetDrawMode(winPaint);

		} else {
			TwGfxFillRect(_palmScreenP, &r, 0);
		}
		TwGfxSetClip(_palmScreenP, &c);

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
		
	}
}
