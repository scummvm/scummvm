/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "stdafx.h"

#include "backends/intern.h"
#include "scumm.h"
#include "common/scaler.h"
#include "common/config-manager.h"

#include "shared.h"
#include "ArmNative.h"

#include <BmpGlue.h>
#include "start.h"
#include "palm.h"
#include "vibrate.h"
#include <PalmNavigator.h>

#include "cd_msa.h"

#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif

#define EXITDELAY			(500) // delay to exit : calc button : double tap 1/500 sec
#define ftrOverlayPtr		(1000)
#define ftrBackupPtr		(1001)
//#define	SND_BLOCK			(8192)
#define	SND_BLOCK			(3072)

OSystem *OSystem_PALMOS_create(int gfx_mode) {
	return OSystem_PALMOS::create(gfx_mode);
}

OSystem *OSystem_PALMOS::create(UInt16 gfx_mode) {
	OSystem_PALMOS *syst = new OSystem_PALMOS();
	syst->init_intern(gfx_mode);
	return syst;
}

void OSystem_PALMOS::init_intern(UInt16 gfx_mode) {
	_mode = gfx_mode;
	_vibrate = gVars->vibrator;
	_fullscreen = (ConfMan.getBool("fullscreen") && OPTIONS_TST(kOptModeWide));
	_adjustAspectRatio = ConfMan.getBool("aspect_ratio");
}

void OSystem_PALMOS::set_palette(const byte *colors, uint start, uint num) {
	if (_quitCount)
		return;

	const byte *b = colors;
	uint i;
	RGBColorType *base = _currentPalette + start;
	for(i=0; i < num; i++) {
		base[i].r = b[0];
		base[i].g = b[1];
		base[i].b = b[2];
		b += 4;
	}

	if (start < _paletteDirtyStart)
		_paletteDirtyStart = start;

	if (start + num > _paletteDirtyEnd)
		_paletteDirtyEnd = start + num;
}

static void ClearScreen() {
	RGBColorType rgb	= {0, 0,0,0};
	RectangleType r		= {0, 0, gVars->screenFullWidth, gVars->screenFullHeight};
	WinSetForeColor(WinRGBToIndex(&rgb));
	WinDrawRectangle(&r,0);
}

static void SwitchTo16bit() {
	if (OPTIONS_TST(kOptMode16Bit)) {
		UInt32 width = hrWidth;
		UInt32 height= hrHeight;
		UInt32 depth = 16;
		Boolean color = true;
		
		WinScreenMode(winScreenModeSet, &width, &height, &depth, &color);
		ClearScreen();
		OPTIONS_SET(kOptDisableOnScrDisp);
	}
}

void OSystem_PALMOS::load_gfx_mode() {
	Err e;
	const byte startupPalette[] = {
		0	,0	,0	,0,
		0	,0	,171,0,
		0	,171, 0	,0,
		0	,171,171,0,
		171	,0	,0	,0, 
		171	,0	,171,0,
		171	,87	,0	,0,
		171	,171,171,0,
		87	,87	,87	,0,
		87	,87	,255,0,
		87	,255,87	,0,
		87	,255,255,0,
		255	,87	,87	,0,
		255	,87	,255,0,
		255	,255,87	,0,
		255	,255,255,0
	};

	// init screens
	switch(_mode) {
		case GFX_FLIPPING:
			gVars->screenLocked = true;
			_offScreenP	= WinScreenLock(winLockErase) + _screenOffset.addr;
			_screenP	= _offScreenP;
			_offScreenH	= WinGetDisplayWindow();
			_screenH	= _offScreenH;	
			_renderer_proc = &OSystem_PALMOS::update_screen__flipping;
			break;
		case GFX_WIDE:
		case GFX_BUFFERED:
			_screenH	= WinGetDisplayWindow();
			_offScreenH	= WinCreateOffscreenWindow(_screenWidth, _screenHeight, screenFormat, &e);
			_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH)));

			if (_mode == GFX_WIDE) {
#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif			
				{
					if (OPTIONS_TST(kOptDeviceARM))
						_arm.pnoPtr = PceNativeCallInit(ARMCODE_1, &_arm.pnoDesc);

					gVars->screenLocked = true;
					_screenP = WinScreenLock(winLockErase) + _screenOffset.addr;
					_renderer_proc = (OPTIONS_TST(kOptModeLandscape)) ?
						&OSystem_PALMOS::update_screen__wide_landscape :
						&OSystem_PALMOS::update_screen__wide_portrait;
				}

			} else {
				_screenP = (byte *)(BmpGetBits(WinGetBitmap(_screenH))) + _screenOffset.addr;
				_renderer_proc =  &OSystem_PALMOS::update_screen__buffered;
			}
			_offScreenPitch = _screenWidth;
			break;

		case GFX_NORMAL:
		default:
			_offScreenH	= WinGetDisplayWindow();
			_screenH = _offScreenH;
			_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH))) + _screenOffset.addr;
			_screenP	= _offScreenP;
			_renderer_proc =  &OSystem_PALMOS::update_screen__direct;
			break;
	}

	if (OPTIONS_TST(kOptModeHiDensity))
		WinSetCoordinateSystem(kCoordinatesNative);

	// palette for preload dialog
	set_palette(startupPalette,0,16);

	// try to allocate on storage heap
	FtrPtrNew(appFileCreator, ftrOverlayPtr, _screenWidth * _screenHeight, (void **)&_tmpScreenP);
	FtrPtrNew(appFileCreator, ftrBackupPtr, _screenWidth * _screenHeight, (void **)&_tmpBackupP);
	// failed ? dynamic heap
	if (!_tmpScreenP) _tmpScreenP = (byte *)malloc(_screenWidth * _screenHeight);
	if (!_tmpBackupP) _tmpBackupP = (byte *)malloc(_screenWidth * _screenHeight);
}

void OSystem_PALMOS::unload_gfx_mode() {

	WinSetDrawWindow(WinGetDisplayWindow());

	if (OPTIONS_TST(kOptModeHiDensity))
		WinSetCoordinateSystem(kCoordinatesStandard);

	switch (_mode) {
		case GFX_FLIPPING:
			WinScreenUnlock();
			break;

		case GFX_WIDE:
#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif
			{
				WinScreenUnlock();
				if (OPTIONS_TST(kOptDeviceARM) && _arm.pnoPtr)
					PceNativeCallRelease(&_arm.pnoDesc, _arm.pnoPtr);
			}
				// continue to GFX_BUFFERED

		case GFX_BUFFERED:
			WinDeleteWindow(_offScreenH,false);
			break;
	}

	if (_tmpScreenP)
		if (MemPtrDataStorage(_tmpScreenP))
			FtrPtrFree(appFileCreator, ftrOverlayPtr);
		else
			free(_tmpScreenP);

	if (_tmpBackupP)
		if (MemPtrDataStorage(_tmpBackupP))
			FtrPtrFree(appFileCreator, ftrBackupPtr);
		else
			free(_tmpBackupP);

}

void OSystem_PALMOS::init_size(uint w, uint h) {
	_screenWidth = w;
	_screenHeight = h;
	_offScreenPitch = gVars->screenPitch;	// direct screen / flipping use this, reset later if buffered
	_screenPitch = gVars->screenPitch;

	_overlayVisible = false;
	_quitCount = 0;

	// 640x480 only on Zodiac and in GFX_WIDE mode
	if (h == 480)
		if (!(_mode == GFX_WIDE && OPTIONS_TST(kOptDeviceZodiac)))
			error("640x480 game can only be run on Zodiac in wide mode.");

	if (OPTIONS_TST(kOptCollapsible))
		SysSetOrientationTriggerState(sysOrientationTriggerDisabled);

	// check HiRes+
	if (_mode == GFX_WIDE) {
		if (OPTIONS_TST(kOptModeWide)) {
			Boolean std = true;

#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif
			if (std) {
				// only for 320x200 games
				if (!(w == 320 && h == 200)) {
/*					if (OPTIONS_TST(kOptModeLandscape)) {
						// not for landscape display at this time, only for portrait
						warning("Wide display not avalaible on this mode, switching to GFX_NORMAL mode.");
						_mode = GFX_NORMAL;
					}
				} else {*/
					warning("Wide display not avalaible for this game, switching to GFX_NORMAL mode.");
					_mode = GFX_NORMAL;
				}
			}

		} else {
			warning("HiRes+ not avalaible on this device, switching to GFX_NORMAL mode.");
			_mode = GFX_NORMAL;
		}
	}

	if (_fullscreen || _mode == GFX_WIDE) {
		// Sony wide
		if (gVars->slkRefNum != sysInvalidRefNum) {
			if (gVars->slkVersion == vskVersionNum1) {
				SilkLibEnableResize (gVars->slkRefNum);
				SilkLibResizeDispWin(gVars->slkRefNum, silkResizeMax);
				SilkLibDisableResize(gVars->slkRefNum);
			} else {
				VskSetState(gVars->slkRefNum, vskStateEnable, (gVars->slkVersion != vskVersionNum3 ? vskResizeVertically : vskResizeHorizontally));
				VskSetState(gVars->slkRefNum, vskStateResize, vskResizeNone);
				VskSetState(gVars->slkRefNum, vskStateEnable, vskResizeDisable);
			}

		// Tapwave Zodiac and other DIA compatible devices
		} else if (OPTIONS_TST(kOptModeWide)) {
			PINSetInputAreaState(pinInputAreaClosed);
			StatHide();
		}
	}

	if (_mode == GFX_WIDE) {
		OPTIONS_SET(kOptDisableOnScrDisp);

		if (OPTIONS_TST(kOptDeviceZodiac)) {
			// landscape
			_screenOffset.x = 0;
			_screenOffset.y = (_adjustAspectRatio) ? 10 : 0;
			_screenOffset.addr = _screenOffset.y * _screenPitch;

		// others only for 320x200
		} else {
			_screenOffset.x = 0;
			_screenOffset.y = 10;
			_screenOffset.addr = (OPTIONS_TST(kOptModeLandscape) ? _screenOffset.y : _screenOffset.x) * _screenPitch;
		}
	} else {
		_screenOffset.x = ((_fullscreen ? gVars->screenFullWidth : gVars->screenWidth) - w) >> 1;
		_screenOffset.y = ((_fullscreen ? gVars->screenFullHeight : gVars->screenHeight) - h) >> 1;
		_screenOffset.addr = _screenOffset.x + _screenOffset.y * _screenPitch;
	}

	set_mouse_pos(200,150);

	_currentPalette = (RGBColorType*)calloc(sizeof(RGBColorType), 256);
	_mouseBackupP = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H);
	
	load_gfx_mode();
}

void OSystem_PALMOS::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	/* Clip the coordinates */
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

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn) {
/*		RectangleType mouse = {_mouseCurState.x, _mouseCurState.y, _mouseCurState.w, _mouseCurState.h};
		RectangleType copy	= {x, y, w, h};
		RectangleType result;
		
		RctGetIntersection(&mouse, &copy, &result);
		
		if (result.extent.x !=0 && result.extent.y !=0)*/
			undraw_mouse();
	}

	byte *dst = _offScreenP + y * _offScreenPitch + x;
/*
	if (OPTIONS_TST(kOptDeviceARM)) {
		DataOSysCopyRectType userData = { kOSysCopyRect, dst, buf, pitch, _offScreenPitch, w, h };
		PceNativeRsrcCall(ARMCODE_1, &userData);
		
#ifdef DEBUG_ARM
	} else if (OPTIONS_TST(kOptDeviceProcX86)) {
		DataOSysCopyRectType userData = { kOSysCopyRect, dst, buf, pitch, _offScreenPitch, w, h };
		UInt32 result = PceNativeCall((NativeFuncType*)"ARMlet.dll\0ARMlet_Main", &userData);		
#endif

	} else {
*/
		if (_offScreenPitch == pitch && pitch == w) {
			MemMove (dst, buf, h * w);
		} else {
			do {
				MemMove(dst, buf, w);
				dst += _offScreenPitch;
				buf += pitch;
			} while (--h);
		}
//	}
}

#ifndef DISABLE_TAPWAVE
// Tapwave code will come here
#endif

void OSystem_PALMOS::update_screen__wide_portrait() {
	UInt8 *dst = _screenP + _screenOffset.y;
	UInt8 *src1 = _offScreenP + WIDE_PITCH - 1;

	if (OPTIONS_TST(kOptDeviceARM)) {
		DataOSysWideType userData = { kOSysWidePortrait, dst, src1 };
		PceNativeRsrcCall(&_arm.pnoDesc, &userData);
		
#ifdef DEBUG_ARM
	} else if (OPTIONS_TST(kOptDeviceProcX86)) {
		DataOSysWideType userData = { kOSysWidePortrait, dst, src1 };
		UInt32 result = PceNativeCall((NativeFuncType*)"ARMlet.dll\0ARMlet_Main", &userData);		
#endif

	} else {
		Coord x, y;
		UInt8 *src2 = src1;

		for (x = 0; x < WIDE_HALF_WIDTH; x++) {
			for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
				*dst++ = *src1;
				src1 += WIDE_PITCH;
				*dst++ = *src1;
				*dst++ = *src1;
				src1 += WIDE_PITCH;
			}
			src1 = --src2;
			dst += 20; // we draw 200pix scaled to 1.5 = 300, screen width=320, so next is 20

			for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
				*dst++ = *src1;
				src1 += WIDE_PITCH;
				*dst++ = *src1;
				*dst++ = *src1;
				src1 += WIDE_PITCH;
			}
			src1 = --src2;
			dst += 20;

			MemMove(dst, dst - WIDE_PITCH, 300);	// 300 = 200 x 1.5
			dst += WIDE_PITCH;
		}
	}

	WinScreenUnlock();
	_screenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
}

void OSystem_PALMOS::update_screen__wide_landscape() {
	UInt8 *dst = _screenP;
	UInt8 *src = _offScreenP;


	if (OPTIONS_TST(kOptDeviceARM)) {
		DataOSysWideType userData = { kOSysWideLandscape, dst, src };
		PceNativeRsrcCall(&_arm.pnoDesc, &userData);

#ifdef DEBUG_ARM
	} else if (OPTIONS_TST(kOptDeviceProcX86)) {
		DataOSysWideType userData = { kOSysWideLandscape, dst, src };
		UInt32 result = PceNativeCall((NativeFuncType*)"ARMlet.dll\0ARMlet_Main", &userData);		
#endif
	} else {
		Coord x, y;

		for (y = 0; y < WIDE_HALF_HEIGHT; y++) {
			for (x = 0; x < WIDE_HALF_WIDTH; x++) {
				*dst++ = *src++;
				*dst++ = *src;
				*dst++ = *src++;
			}
			for (x = 0; x < WIDE_HALF_WIDTH; x++) {
				*dst++ = *src++;
				*dst++ = *src;
				*dst++ = *src++;
			}

			MemMove(dst, dst - 480, 480);
			dst += 480;
		}
	}

	WinScreenUnlock();
	_screenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
}

void OSystem_PALMOS::update_screen__flipping() {
	RectangleType r, dummy;
	Boolean shaked = false;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		RctSetRectangle(&r, _screenOffset.x, _screenOffset.y - _new_shake_pos, _screenWidth, _screenHeight + (_new_shake_pos << 2));

		if (_useHRmode)
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, _new_shake_pos, &dummy);
		else
			WinScrollRectangle(&r, winDown, _new_shake_pos, &dummy);

		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}

		_current_shake_pos = _new_shake_pos;
		shaked = true;
	}

	// update screen
	WinScreenUnlock();
	_offScreenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
	_screenP = _offScreenP;

	if (shaked) {
		if (_useHRmode)
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, _new_shake_pos, &dummy);
		else
			WinScrollRectangle(&r, winUp, _new_shake_pos, &dummy);
	}

}

void OSystem_PALMOS::update_screen__buffered() {
	UInt32 move = 0;
	UInt32 size = _screenWidth * _screenHeight;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		move = (_new_shake_pos * _screenPitch);
		// copy clear bottom of the screen to top to cover shaking image

		if (_screenPitch == _screenWidth) {
			MemMove(_screenP, _screenP + size , move);
			MemMove(_screenP + move, _offScreenP, size - move);
		} else if (_new_shake_pos != 0) {
			UInt16 h = _new_shake_pos;
			byte *src = _screenP + _screenPitch * _screenHeight;
			byte *dst = _screenP;
			do {
				memcpy(dst, src, _screenWidth);
				dst += _screenPitch;
				src += _screenPitch;
			} while (--h);
		}

		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}

		_current_shake_pos = _new_shake_pos;
	}
	// update screen
	if (_screenPitch == _screenWidth) {
		MemMove(_screenP + move, _offScreenP, size - move);
	} else {
		byte *src = _offScreenP;
		byte *dst = _screenP + move;
		UInt16 h = _screenHeight - _new_shake_pos;
		do {
			memcpy(dst, src, _screenWidth);
			dst += _screenPitch;
			src += _offScreenPitch;
		} while (--h);
	}

}

void OSystem_PALMOS::update_screen__direct() {
	if (_current_shake_pos != _new_shake_pos) {
		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}	
		_current_shake_pos = _new_shake_pos;
	}
}

enum {
	MD_NONE = 0,
	MD_CMD,
	MD_ALT,
	MD_CTRL
};

#define kDrawKeyState	3000
#define kDrawNumPad		3010

void OSystem_PALMOS::draw1BitGfx(UInt16 id, UInt32 x, UInt32 y, Boolean show) {
	if (OPTIONS_TST(kOptDisableOnScrDisp))
		return;

	MemHandle hTemp = DmGetResource(bitmapRsc, id);
	
	if (hTemp) {
		BitmapType *bmTemp;
		UInt32 *bmData;
		UInt8 ih, iw, ib;
		Coord w, h;
		Int16 blocks, next;

		UInt8 *scr = _screenP + x + _screenPitch * y;

		bmTemp	= (BitmapType *)MemHandleLock(hTemp);
		bmData	= (UInt32 *)BmpGetBits(bmTemp);
		BmpGlueGetDimensions(bmTemp, &w, &h, 0);

		blocks = w >> 5;
		next = w - (blocks << 5);

		if (next)
			blocks++;
		
		for (ih = 0; ih < h; ih++) {			// line
			for (ib = 0; ib < blocks; ib++) {	// 32pix block
				next = w - (ib << 5);
				next = MIN(next, (Coord)32);
		
				for (iw = 0; iw < next; iw++) {	// row

					*scr++ = ((*bmData & (1 << (31 - iw))) && show) ?
						gVars->indicator.on :
						gVars->indicator.off;
				}

				bmData++;
			}
			scr += _screenPitch - w;
		}

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
	}
}

void OSystem_PALMOS::update_screen() {
	if(_quitCount)
		return;

	// Make sure the mouse is drawn, if it should be drawn.
	draw_mouse();

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_paletteDirtyEnd != 0) {
		UInt8 oldCol;

		if (gVars->stdPalette) {
			WinSetDrawWindow(WinGetDisplayWindow());	// hack by Doug
			WinPalette(winPaletteSet, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart,_currentPalette + _paletteDirtyStart);
		} else {
			HwrDisplayPalette(winPaletteSet, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart,_currentPalette + _paletteDirtyStart);
		}
		_paletteDirtyEnd = 0;
		oldCol = gVars->indicator.on;
		gVars->indicator.on = RGBToColor(0,255,0);

		if (oldCol != gVars->indicator.on) {	
			// redraw if needed
			if (_lastKeyModifier)
				draw1BitGfx((kDrawKeyState + _lastKeyModifier - 1), 2, get_height() + 2, true);
			
			if(_useNumPad)
				draw1BitGfx(kDrawNumPad, (get_width() >> 1) - 32, get_height() + 2, true);
		}
	}

	if (_overlayVisible) {
		byte *src = _tmpScreenP;
		byte *dst = _offScreenP;
		UInt16 h = _screenHeight;
		
		do {
			memcpy(dst, src, _screenWidth);
			dst += _offScreenPitch;
			src += _screenWidth;
		} while (--h);
	}

	// redraw the screen
	((this)->*(_renderer_proc))();


}

bool OSystem_PALMOS::show_mouse(bool visible) {
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

void OSystem_PALMOS::warp_mouse(int x, int y) {
	set_mouse_pos(x, y);
}

void OSystem_PALMOS::set_mouse_pos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		undraw_mouse();
	}
}

void OSystem_PALMOS::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseDataP = (byte*)buf;

	undraw_mouse();
}

void OSystem_PALMOS::set_shake_pos(int shake_pos) {
	_new_shake_pos = shake_pos;
	
	if (shake_pos == 0 && _vibrate) {
		Boolean active = false;
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
	}
}

uint32 OSystem_PALMOS::get_msecs() {
	uint32 ticks = TimGetTicks();
	ticks *= (1000/SysTicksPerSecond());
	return ticks;

}

void OSystem_PALMOS::delay_msecs(uint msecs) {
	UInt32 current = get_msecs();
	UInt32 last = current + msecs;

	do {
		timer_handler(current);
		current = get_msecs();
	} while (current < last);
}

void OSystem_PALMOS::set_timer(TimerProc callback, int timer) {
	if (callback != NULL) {
		_timer.duration = timer;
		_timer.nextExpiry = get_msecs() + timer;
		_timer.callback = callback;
		_timer.active = true;
	} else {
		_timer.active = false;
	}
}

/* Mutex handling */
OSystem::MutexRef OSystem_PALMOS::create_mutex() {return NULL;}
void OSystem_PALMOS::lock_mutex(MutexRef mutex) {}
void OSystem_PALMOS::unlock_mutex(MutexRef mutex) {}
void OSystem_PALMOS::delete_mutex(MutexRef mutex) {}

void OSystem_PALMOS::SimulateArrowKeys(Event *event, Int8 iHoriz, Int8 iVert, Boolean repeat) {
	Int16 x = _mouseCurState.x;
	Int16 y = _mouseCurState.y;

	if (_lastKeyPressed != kLastKeyNone || repeat) {
		_lastKeyRepeat += 2;

		if (_lastKeyRepeat > 32)
			_lastKeyRepeat = 32;
	}
	else
		_lastKeyRepeat = 0;

	x = x + iHoriz * (_lastKeyRepeat >> 1);
	y = y + iVert * (_lastKeyRepeat >> 1);

	x = (x < 0				) ? 0					: x;
	x = (x >= _screenWidth	) ? _screenWidth - 1	: x;
	y = (y < 0				) ? 0					: y;
	y = (y >= _screenHeight	) ? _screenHeight - 1	: y;

	event->event_code = EVENT_MOUSEMOVE;
	event->mouse.x = x;
	event->mouse.y = y;
	set_mouse_pos(x, y);
}

void OSystem_PALMOS::getCoordinates(EventPtr event, Coord *x, Coord *y) {
	if (OPTIONS_TST(kOptModeHiDensity)) {
		Boolean dummy;
		EvtGetPenNative(WinGetDisplayWindow(), &event->screenX, &event->screenY, &dummy);
	} else {
		event->screenX <<= 1;
		event->screenY <<= 1;
	}

	if (_mode == GFX_WIDE) {
		// wide landscape
		if (OPTIONS_TST(kOptModeLandscape)) {

			// zodiac have mutliple ratio 
			if (OPTIONS_TST(kOptDeviceZodiac)) {
				Int32 w,h;

				h = (_adjustAspectRatio ? ((_screenHeight == 200) ? 300 : 360) : 320);
				w = gVars->screenFullWidth;

				*x = ((event->screenX - _screenOffset.x) << 8) / ((w << 8) / _screenWidth);
				*y = ((event->screenY - _screenOffset.y) << 8) / ((h << 8) / _screenHeight);

			// default (NR, NZ, NX, UX50, TT3, ...), wide only for 320x200
			} else {
				*x = ((event->screenX - _screenOffset.x) << 1) / 3;
				*y = ((event->screenY - _screenOffset.y) << 1) / 3;
			}

		// wide portrait, only for 320x200
		} else {
			*y =       ((event->screenX - _screenOffset.y) << 1) / 3;
			*x = 320 - ((event->screenY - _screenOffset.x) << 1) / 3 - 1;	
		}

	// normal coord
	} else {
		*x = (event->screenX - _screenOffset.x);
		*y = (event->screenY - _screenOffset.y);
	}
}

bool OSystem_PALMOS::poll_event(Event *event) {
	EventType ev;
	Boolean handled;
	//UInt32 current_msecs;
	UInt32 keyCurrentState;
	Coord x, y;

	if(_quitCount) {
		if (_quitCount >= 10)
			SysReset();
		else
			_quitCount++;
	
		event->event_code = EVENT_QUIT;
		exit(0);	// resend an exit event
		return false;
	}

	//current_msecs = get_msecs();

	// sound handler
	sound_handler();

	// timer handler
	timer_handler(get_msecs());

	if (_selfQuit)
		quit();

	for(;;) {
		EvtGetEvent(&ev, evtNoWait);

		// check for hardkey repeat for mouse emulation (no 5way only)
		keyCurrentState = KeyCurrentState();
		if (_lastKeyPressed != kLastKeyNone && _lastKeyPressed != kLastKeyCalc &&
			!(	(keyCurrentState & keyBitHard2) ||		// left
				(keyCurrentState & keyBitPageUp) ||		// up
				(keyCurrentState & keyBitPageDown) ||	// down
				(keyCurrentState & keyBitHard3)			// right
				)
			) {
					_lastKeyPressed = kLastKeyNone;
		}

		if (ev.eType == nilEvent)
			return false;

		if (ev.eType == keyDownEvent) {
			switch (ev.data.keyDown.chr) {
				// ESC key
				case vchrLaunch:
					_lastKeyPressed = kLastKeyNone;
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 27;
					event->kbd.ascii = 27;
					event->kbd.flags = 0;
					return true;
				
				// F5 = menu
				case vchrJogPushRepeat:
				case vchrMenu:
					_lastKeyPressed = kLastKeyNone;
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 319;
					event->kbd.ascii = 319;
					event->kbd.flags = 0;
					return true;

				case vchrFind:
					// do nothing
					return true;

				case vchrBrightness:
				case vchrContrast:
					WinPalette(winPaletteSet, 0, 256, _currentPalette);
					break;

				case vchrCalc:
					if (_lastKeyPressed & kLastKeyCalc)
						if ((get_msecs() - _exit_delay) <= (EXITDELAY)) {
							event->event_code = EVENT_QUIT;
							_selfQuit = true;
						}

					_exit_delay = get_msecs();
					_lastKeyPressed = kLastKeyCalc;
					return true;

				// wheel
				case vchrJogUp:
					event->event_code = EVENT_WHEELUP;
					return true;

				case vchrJogDown:
					event->event_code = EVENT_WHEELDOWN;
					return true;

				// if hotsync pressed
				case vchrHardCradle:
				case vchrHardCradle2:
					_selfQuit = true;
					break;

				// mouse button
				case vchrJogBack:
				case vchrHard4: // right button
					event->event_code = EVENT_RBUTTONDOWN;
					event->mouse.x = _mouseCurState.x;
					event->mouse.y = _mouseCurState.y;
					_lastKeyPressed = kLastKeyNone;
					return true;

				case vchrJogPush:
				case vchrHard1: // left button
					event->event_code = EVENT_LBUTTONDOWN;
					event->mouse.x = _mouseCurState.x;
					event->mouse.y = _mouseCurState.y;
					_lastKeyPressed = kLastKeyNone;
					return true;
			}
			
			if (OPTIONS_TST(kOpt5WayNavigator)) {
				// mouse emulation for device with 5-Way navigator
				switch (ev.data.keyDown.chr) {
					// ESC key
					case vchrHard2:
						_lastKeyPressed = kLastKeyNone;
						event->event_code = EVENT_KEYDOWN;
						event->kbd.keycode = 27;
						event->kbd.ascii = 27;
						event->kbd.flags = 0;
						return true;
					
					// F5 = menu
					case vchrHard3:
						_lastKeyPressed = kLastKeyNone;
						event->event_code = EVENT_KEYDOWN;
						event->kbd.keycode = 319;
						event->kbd.ascii = 319;
						event->kbd.flags = 0;
						return true;
					
					case vchrPageUp:
					case vchrPageDown:
					case vchrNavChange:
						// left mouse button
						if (ev.data.keyDown.keyCode & (navBitSelect|navChangeSelect)) {
							event->event_code = EVENT_LBUTTONDOWN;
							event->mouse.x = _mouseCurState.x;
							event->mouse.y = _mouseCurState.y;
							_lastKeyPressed = kLastKeyNone;
							return true;						

						// mouse move
						} else {
							Boolean kUp		= (ev.data.keyDown.keyCode & navBitUp);
							Boolean kDown	= (ev.data.keyDown.keyCode & navBitDown);
							Boolean kLeft	= (ev.data.keyDown.keyCode & navBitLeft);
							Boolean kRight	= (ev.data.keyDown.keyCode & navBitRight);
							Boolean kRepeat	= (ev.data.keyDown.modifiers & autoRepeatKeyMask);
							Boolean process	= (kUp || kDown || kLeft || kRight);
							
							if (process) {
								Int8 stepX = 0;
								Int8 stepY = 0;

								if (kLeft)
									stepX = -1;
								else if (kRight)
									stepX = +1;
									
								if (kUp)
									stepY = -1;
								else if (kDown)
									stepY = +1;
								
								SimulateArrowKeys(event, stepX, stepY, kRepeat);
								_lastKeyPressed = kLastKeyNone;	// use only repeat flag
								return true;
							}
						}
						break;
				}

			} else {
				// mouse emulation for device without 5-Way navigator
				switch (ev.data.keyDown.chr) {
					case vchrHard2:	// move left
						SimulateArrowKeys(event, -1, 0);
						_lastKeyPressed = kLastKeyMouseLeft;
						return true;
						
					case vchrPageUp: // move up
						SimulateArrowKeys(event, 0, -1);
						_lastKeyPressed = kLastKeyMouseUp;
						return true;

					case vchrPageDown: // move down
						SimulateArrowKeys(event, 0, 1);
						_lastKeyPressed = kLastKeyMouseDown;
						return true;

					case vchrHard3: // move right
						SimulateArrowKeys(event, 1, 0);
						_lastKeyPressed = kLastKeyMouseRight;
						return true;
				}
			}
		}

		// prevent crash when alarm is raised
		handled = ((ev.eType == keyDownEvent) && 
						(ev.data.keyDown.modifiers & commandKeyMask) && 
						((ev.data.keyDown.chr == vchrAttnStateChanged) || 
						(ev.data.keyDown.chr == vchrAttnUnsnooze))); 

		// graffiti strokes, auto-off, etc...
		if (!handled)
			if (SysHandleEvent(&ev))
				continue;

		// others events
		switch(ev.eType) {

		case keyDownEvent: {
				_lastEvent = keyDownEvent;
				_lastKeyPressed = kLastKeyNone;

				if (ev.data.keyDown.chr == vchrCommand && (ev.data.keyDown.modifiers & commandKeyMask)) {
					_lastKeyModifier++;
					_lastKeyModifier %= 4;
					
					if (_lastKeyModifier)
						draw1BitGfx((kDrawKeyState + _lastKeyModifier - 1), 2, get_height() + 2, true);
					else
						draw1BitGfx(kDrawKeyState, 2, get_height() + 2, false);

				} else {
					byte b = 0;
					if (_lastKeyModifier == MD_CMD) b = KBD_CTRL|KBD_ALT;
					if (_lastKeyModifier == MD_ALT) b = KBD_ALT;
					if (_lastKeyModifier == MD_CTRL) b = KBD_CTRL;
					
					if  ((ev.data.keyDown.chr == 'z' && b == KBD_CTRL) || (b == KBD_ALT && ev.data.keyDown.chr == 'x')) {
						event->event_code = EVENT_QUIT;
						_selfQuit = true;

					} else if (ev.data.keyDown.chr == 'n' && b == KBD_CTRL) {
						UInt8 *scr = _screenP + _screenWidth * (_screenHeight + 2);
						_useNumPad = !_useNumPad;
						draw1BitGfx(kDrawNumPad, (get_width() >> 1) - 32, get_height() + 2, _useNumPad);

					} else if (ev.data.keyDown.chr == 'a' && b == (KBD_CTRL|KBD_ALT)) {
						property(PROP_TOGGLE_ASPECT_RATIO, NULL);
						break;
					}

					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = ev.data.keyDown.chr;
					event->kbd.ascii = (ev.data.keyDown.chr>='a' && ev.data.keyDown.chr<='z' && (event->kbd.flags & KBD_SHIFT) ? ev.data.keyDown.chr &~ 0x20 : ev.data.keyDown.chr);
					event->kbd.flags = b;
					
					if (_lastKeyModifier) {
						_lastKeyModifier = MD_NONE;
						draw1BitGfx(kDrawKeyState, 2, get_height() + 2, false);
					}
				}
				return true;
			}

		case penMoveEvent:
			getCoordinates(&ev, &x, &y);

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return true;

			if (_lastEvent != penMoveEvent && (abs(y - event->mouse.y) <= 2 || abs(x - event->mouse.x) <= 2)) // move only if
				return true;

			_lastEvent = penMoveEvent;
			event->event_code = EVENT_MOUSEMOVE;
			event->mouse.x = x;
			event->mouse.y = y;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case penDownEvent:
			getCoordinates(&ev, &x, &y);

			if (_useNumPad) {
				Coord x2 = (_screenWidth >> 1) - 20; // - 64 / 2 + 12
				Coord y2 = _screenHeight + 2;

				if (y >= y2 && y < (y2 + 34) && x >= x2 && x < (x2 + 40)) {	// numpad location
					UInt8 key = '1';
					key += 9 - ( (3 - ((x - x2) / 13)) + (3 * ((y - y2) / 11)) );
					
					_lastEvent = keyDownEvent;
					_lastKeyPressed = kLastKeyNone;
					
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = key;
					event->kbd.ascii = key;
					event->kbd.flags = 0;
					return true;
				}
			}

			_lastEvent = penDownEvent;
			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return true;

			event->event_code = EVENT_LBUTTONDOWN;
			event->mouse.x = x;
			event->mouse.y = y;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case penUpEvent:
			getCoordinates(&ev, &x, &y);
			event->event_code = EVENT_LBUTTONUP;

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return true;

			event->mouse.x = x;
			event->mouse.y = y;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		default:
			return false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
uint32 OSystem_PALMOS::property(int param, Property *value) {
	switch(param) {

	case PROP_SET_WINDOW_CAPTION:
		
		Char *caption = "Loading, please wait\0";
		Coord h = FntLineHeight() + 2;
		Coord w, y;

		// quick erase the screen
		WinScreenLock(winLockErase);
		WinScreenUnlock();
		
		WinSetTextColor(255);
		WinSetForeColor(255);

		if (_useHRmode) {
			y = 160 - (h >> 1) - 10;
			HRFntSetFont(gVars->HRrefNum,hrTinyBoldFont);
			w = FntCharsWidth(caption,StrLen(caption));
			w = (320 - w) >> 1;
			HRWinDrawChars(gVars->HRrefNum, caption, StrLen(caption), w, y + h);

			HRFntSetFont(gVars->HRrefNum,hrTinyFont);
			w = FntCharsWidth(value->caption, StrLen(value->caption));
			w = (320 - w) >> 1;
			HRWinDrawChars(gVars->HRrefNum, value->caption, StrLen(value->caption), w, y);
		} else {
			Err e;
			BitmapTypeV3 *bmp2P;
			BitmapType *bmp1P = BmpCreate(320, (h << 1), 8, NULL, &e);
			WinHandle tmpH = WinCreateBitmapWindow(bmp1P, &e);

			WinSetDrawWindow(tmpH);

			FntSetFont(boldFont);
			y = 80 - (h >> 2) - 5;
			w = FntCharsWidth(caption, StrLen(caption));
			w = (320 - w) >> 1;
			WinDrawChars(caption, StrLen(caption), w, 0 + h);

			FntSetFont(stdFont);
			w = FntCharsWidth(value->caption, StrLen(value->caption));
			w = (320 - w) >> 1;
			WinDrawChars(value->caption, StrLen(value->caption), w, 0);

			WinSetDrawWindow(WinGetDisplayWindow());
			bmp2P = BmpCreateBitmapV3(bmp1P, kDensityDouble, BmpGetBits(bmp1P), NULL);
			WinDrawBitmap((BitmapPtr)bmp2P, 0, y);

			BmpDelete((BitmapPtr)bmp2P);
			WinDeleteWindow(tmpH,0);
			BmpDelete(bmp1P);
		}
		return 1;

	case PROP_OPEN_CD:
		break;

	case PROP_TOGGLE_ASPECT_RATIO:
		if (_mode == GFX_WIDE && OPTIONS_TST(kOptDeviceZodiac)) {
			_adjustAspectRatio ^= true;
			ClearScreen();
		}
		break;

	case PROP_GET_SAMPLE_RATE:
		return 8000;
	
	case PROP_HAS_SCALER:
		return 0;
	}

	return 0;
}

void OSystem_PALMOS::quit() {
	// There is no exit(.) function under PalmOS, to exit an app
	// we need to send an 'exit' event to the event handler
	// and then the system return to the launcher. But this event
	// is not handled by the main loop and so we need to force exit.
	// In other systems like Windows ScummVM exit immediatly and so this doesn't appear.

	if (_quitCount)
		return;

	if (_selfQuit && Scumm::g_scumm)
		Scumm::g_scumm->_quit = true;

	free(_currentPalette);
	free(_mouseBackupP);

	if (_cdPlayer) {
		_cdPlayer->release();
		_cdPlayer = NULL;
	}

	unload_gfx_mode();
	_quitCount++;
	exit(1);
}

void OSystem_PALMOS::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible || _quitCount)
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
				if (color != 0xFF)	// 0xFF = transparent, don't draw
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
		if (MemPtrDataStorage(_tmpScreenP)) {
			int offset = y * _screenWidth + x;
			dst = _tmpScreenP;
			
			while (h > 0) {
				width = w;
				while (width > 0) {
					*bak++ = *(dst + offset);
					color = *src++;
					if (color != 0xFF)
						DmWrite(dst, offset, &color, 1);
					offset++;
					width--;
				}
				src += _mouseCurState.w - w;
				bak += MAX_MOUSE_W - w;
				offset += _screenWidth - w;
				h--;
			}
		} else {
			dst = _tmpScreenP + y * _screenWidth + x;

			while (h > 0) {
				width = w;
				while (width > 0) {
					*bak++ = *dst;
					color = *src++;
					if (color != 0xFF)
						*dst = color;
					dst++;
					width--;
				}
				src += _mouseCurState.w - w;
				bak += MAX_MOUSE_W - w;
				dst += _screenWidth - w;
				h--;
			}
		}
	}

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseDrawn = true;
}

void OSystem_PALMOS::undraw_mouse() {
	if (!_mouseDrawn || _quitCount)
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
		if (MemPtrDataStorage(_tmpScreenP)) {
			int offset = old_mouse_y * _screenWidth + old_mouse_x;
			do {
				DmWrite(_tmpScreenP, offset, bak, old_mouse_w);
				bak += MAX_MOUSE_W;
				offset += _screenWidth;
			} while (--old_mouse_h);

		} else {
			dst = _tmpScreenP + old_mouse_y * _screenWidth + old_mouse_x;
			do {
				memcpy(dst, bak, old_mouse_w);
				bak += MAX_MOUSE_W;
				dst += _screenWidth;
			} while (--old_mouse_h);
		}
	}
}

void OSystem_PALMOS::stop_cdrom() {
	if (!_cdPlayer)
		return;

	_cdPlayer->stop();
}

void OSystem_PALMOS::play_cdrom(int track, int num_loops, int start_frame, int duration) {
	if (!_cdPlayer)
		return;

	_cdPlayer->play(track, num_loops, start_frame, duration);
}

bool OSystem_PALMOS::poll_cdrom() {
	if (!_cdPlayer)
		return false;
	
	return _cdPlayer->poll();
}

void OSystem_PALMOS::update_cdrom() {
	if (!_cdPlayer)
		return;

	_cdPlayer->update();
}

OSystem_PALMOS::OSystem_PALMOS() {
	_quitCount = 0;
	_selfQuit = false; // prevent illegal access to g_scumm
	_current_shake_pos = 0;
	_new_shake_pos = 0;

	_paletteDirtyStart = 0;
	_paletteDirtyEnd = 0;
	
	memset(&_sound, 0, sizeof(SoundDataType));
	
	_currentPalette = NULL;

	_lastKeyPressed = kLastKeyNone;
	_lastKeyRepeat = 100;
	_lastKeyModifier = MD_NONE;
	
	_useNumPad = false;

	// mouse
	memset(&_mouseOldState,0,sizeof(MousePos));
	memset(&_mouseCurState,0,sizeof(MousePos));
	_mouseDrawn = false;
	_mouseBackupP = NULL;
	_mouseVisible = false;
	
	// overlay
	_tmpScreenP = NULL;
	_tmpBackupP = NULL;
	
	// HiRes
	_useHRmode	= (gVars->HRrefNum != sysInvalidRefNum);
	
	// ARM
	_arm.pnoPtr = NULL;
	
	// enable cdrom ?
	// TODO : defaultTrackLength player
	_cdPlayer = NULL;
	if (gVars->music.MP3) {
		_cdPlayer = new MsaCDPlayer(this);
//		_cdPlayer = new ZodiacCDPlayer(this);
		_cdPlayer->init();
		
		// TODO : use setDefaultTrackLength
//		_isCDRomAvalaible = _cdPlayer->init();
//		if (!_isCDRomAvalaible)
//			_cdPlayer->release();
	}
	
	// sound
//	_isSndPlaying = false;
	memset(&_sound,0,sizeof(SoundDataType));
}

void OSystem_PALMOS::move_screen(int dx, int dy, int height) {
	// Short circuit check - do we have to do anything anyway?
	if ((dx == 0 && dy == 0) || height <= 0)
		return;

	// Hide the mouse
	if (_mouseDrawn)
		undraw_mouse();

	RectangleType r, dummy;
	WinSetDrawWindow(_offScreenH);
	RctSetRectangle(&r, ((_offScreenH != _screenH) ? 0 : _screenOffset.x), ((_offScreenH != _screenH) ? 0 : _screenOffset.y), _screenWidth, _screenHeight);

	// vertical movement
	if (dy > 0) {
		// move down - copy from bottom to top
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, dy, &dummy);
		} else {
			WinScrollRectangle(&r, winDown, dy, &dummy);
		}
	} else if (dy < 0) {
		// move up - copy from top to bottom
		dy = -dy;
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, dy, &dummy);
		} else {
			WinScrollRectangle(&r, winUp, dy, &dummy);
		}
	}

	// horizontal movement
	if (dx > 0) {
		// move right - copy from right to left
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winRight, dx, &dummy);
		} else {
			WinScrollRectangle(&r, winRight, dx, &dummy);
		}
	} else if (dx < 0)  {
		// move left - copy from left to right
		dx = -dx;
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winLeft, dx, &dummy);
		} else {
			WinScrollRectangle(&r, winLeft, dx, &dummy);
		}
	}


	WinSetDrawWindow(_screenH);
	// Prevent crash on Clie device using successive [HR]WinScrollRectangle !
	SysTaskDelay(1);
}

static Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 *bufferSizeP) {
	SoundDataType *snd = (SoundDataType *)UserDataP;
//	snd->proc(snd->param, (UInt8 *)bufferP, *bufferSizeP);
	MemSet(bufferP, *bufferSizeP, 0);


/*	if (snd->lock || !snd->started) {
		MemSet(bufferP, *bufferSizeP, 0);
		return errNone;
	}
	
	if ((snd->lastSize + *bufferSizeP) <= MemPtrSize(snd->dataP)) {
		snd->lock = true;
		MemMove(bufferP, (UInt8 *)snd->dataP + snd->lastSize , *bufferSizeP);
		snd->lastSize += *bufferSizeP;
		snd->lock = false;
	} else {
		MemSet(bufferP, *bufferSizeP, 0);
	}*/
//	if (!snd->timerMode) {
//		snd->proc(snd->param, (UInt8 *)bufferP, *bufferSizeP);
/*	} else {
		if (!snd->active) {
			snd->dataP = MemPtrNew(*bufferSizeP);
			MemSet(snd->dataP, *bufferSizeP, 0);
			snd->active = true;
		}
		
		if (snd->filled)
			MemMove(bufferP, snd->dataP, *bufferSizeP);
		else
			MemSet(bufferP, *bufferSizeP, 0);

		snd->filled = false;
	}*/

	return errNone;
}

bool OSystem_PALMOS::set_sound_proc(SoundProc proc, void *param, SoundFormat format) {
	Boolean success = false;

	if (!_sound.active) {
		_sound.proc = proc;
		_sound.param = param;
		_sound.format = format;
		_sound.active = true;	// always true when we call this function
		
		// try to create sound stream
		if (OPTIONS_TST(kOptPalmSoundAPI)) {
			Err e = SndStreamCreateExtended(&_sound.sndRefNum, sndOutput,sndFormatPCM, 8000, sndInt16Big, sndStereo, &sndCallback, &_sound, 1024, false);
			e = (e) ? e : SndStreamStart(_sound.sndRefNum);
			//e = e ? e :	SndStreamSetVolume(_sound.sndRefNum, 32767 * gVars->soundVolume / 256);

			_sound.dataP = NULL;		// sound buffer is set by the system
			_sound.useHandler = true;	// don't execute sound_handler
			success = (e == errNone);
		}
		// failed or not supported 
		if (!success) {
			_sound.useHandler = true;
			_sound.dataP = calloc(1,SND_BLOCK);	// required by sound_handler
			success = true;					// don't generate samples
		}
	}
	return success;
}

void OSystem_PALMOS::clear_sound_proc() {
	if (_sound.active) {
		if (!_sound.useHandler) {
			SndStreamStop(_sound.sndRefNum);
			SndStreamDelete(_sound.sndRefNum);
		}

		free(_sound.dataP);
	}

	_sound.active = false;
	_sound.useHandler = false;
	_sound.dataP = NULL;
}

void OSystem_PALMOS::sound_handler() {
	if (_sound.useHandler) {
		_sound.proc(_sound.param, (byte *)_sound.dataP, SND_BLOCK);
	}
}

void OSystem_PALMOS::timer_handler(UInt32 current_msecs) {
	if (_timer.active && (current_msecs >= _timer.nextExpiry)) {
		_timer.duration = _timer.callback(_timer.duration);
		_timer.nextExpiry = current_msecs + _timer.duration;
	}
}

void OSystem_PALMOS::show_overlay() {
	// hide the mouse
	undraw_mouse();
	// save background
	byte *src = _offScreenP;
	byte *dst = _tmpBackupP;
	int h = _screenHeight;

	if (MemPtrDataStorage(_tmpScreenP)) {
		UInt32 offset = 0;
		do {
			DmWrite(dst, offset, src, _screenWidth);
			offset += _screenWidth;
			src += _offScreenPitch;
		} while (--h);	
	} else {
		do {
			memcpy(dst, src, _screenWidth);
			dst += _screenWidth;
			src += _offScreenPitch;
		} while (--h);	
	}

	_overlayVisible = true;
	clear_overlay();
}

void OSystem_PALMOS::hide_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlayVisible = false;
	copy_rect(_tmpBackupP, _screenWidth, 0, 0, _screenWidth, _screenHeight);
}

void OSystem_PALMOS::clear_overlay() {
	if (!_overlayVisible)
		return;

	// hide the mouse
	undraw_mouse();
	// restore background
	if (MemPtrDataStorage(_tmpScreenP))
		DmWrite(_tmpScreenP, 0, _tmpBackupP, _screenWidth * _screenHeight);
	else
		MemMove(_tmpScreenP, _tmpBackupP, _screenWidth * _screenHeight);
}

void OSystem_PALMOS::grab_overlay(byte *buf, int pitch) {
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

void OSystem_PALMOS::copy_rect_overlay(const byte *buf, int pitch, int x, int y, int w, int h) {
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

	if (MemPtrDataStorage(_tmpScreenP)) {
		byte *dst = _tmpScreenP;
		int offset = y * _screenWidth + x;
		do {
			DmWrite(dst, offset, buf, w);
			offset += _screenWidth;
			buf += pitch;
		} while (--h);
	} else {
		byte *dst = _tmpScreenP + y * _screenWidth + x;
		do {
			memcpy(dst, buf, w);
			dst += _screenWidth;
			buf += pitch;
		} while (--h);
	}
}


int16 OSystem_PALMOS::get_height() {
	return _screenHeight;
}

int16 OSystem_PALMOS::get_width() {
	return _screenWidth;
}

byte OSystem_PALMOS::RGBToColor(uint8 r, uint8 g, uint8 b) {
	byte color;

	if (gVars->stdPalette) {
		RGBColorType rgb = {0, r, g, b};
		color = WinRGBToIndex(&rgb);

	} else {
		byte nearest = 255;
		byte check;
		byte r2, g2, b2;

		color = 255;

		for (int i = 0; i < 256; i++)
		{
			r2 = _currentPalette[i].r;
			g2 = _currentPalette[i].g;
			b2 = _currentPalette[i].b;

			check = (ABS(r2 - r) + ABS(g2 - g) + ABS(b2 - b)) / 3;

			if (check == 0)				// perfect match
				return i;
			else if (check < nearest) { // else save and continue
				color = i;
				nearest = check;
			}
		}
	}
	
	return color;
}

void OSystem_PALMOS::ColorToRGB(byte color, uint8 &r, uint8 &g, uint8 &b) {
	r = _currentPalette[color].r;
	g = _currentPalette[color].g;
	b = _currentPalette[color].b;
}
