/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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

#include "backends/intern.h"
#include "scumm.h"
#include "common/scaler.h"

#include "start.h"
#include "palm.h"
#include "vibrate.h"

#include "cd_msa.h"
//#undef DISABLE_TAPWAVE

#ifndef DISABLE_TAPWAVE
#include "tapwave.h"
//#include "cd_zodiac.h"
#endif

#define EXITDELAY			(500) // delay to exit : calc button : double tap 1/500 sec
#define ftrOverlayPtr		(1000)
#define ftrBackupPtr		(1001)
//#define	SND_BLOCK			(8192)
#define	SND_BLOCK			(3072)

OSystem *OSystem_PALMOS::create(UInt16 gfx_mode, bool full_screen) {
	OSystem_PALMOS *syst = new OSystem_PALMOS();
	syst->_mode = gfx_mode;
	syst->_vibrate = gVars->vibrator;
	syst->_fullscreen = (full_screen && (gVars->options & optHasWideMode));
	return syst;
}

OSystem *OSystem_PALMOS_create(int gfx_mode, bool full_screen) {
	return OSystem_PALMOS::create(gfx_mode, full_screen);
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
			_renderer_proc = &update_screen__flipping;
			break;
		case GFX_WIDE:
		case GFX_DOUBLEBUFFER:
			_screenH	= WinGetDisplayWindow();
			_offScreenH	= WinCreateOffscreenWindow(_screenWidth, _screenHeight,screenFormat, &e);
			_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH)));

			if (_mode == GFX_WIDE) {
				gVars->screenLocked = true;
				_screenP = WinScreenLock(winLockErase) + _screenOffset.addr;
				_renderer_proc = &update_screen__wide;
			} else {
				_screenP = (byte *)(BmpGetBits(WinGetBitmap(_screenH))) + _screenOffset.addr;
				_renderer_proc = &update_screen__dbuffer;
				_offScreenPitch = _screenWidth;
			}
			break;

		case GFX_NORMAL:
		default:
			_offScreenH	= WinGetDisplayWindow();
			_screenH = _offScreenH;
			_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH))) + _screenOffset.addr;
			_screenP	= _offScreenP;
			_renderer_proc = &update_screen__direct;
			break;
	}

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
	switch (_mode)
	{
		case GFX_FLIPPING:
			WinScreenUnlock();
			break;

		case GFX_WIDE:
			WinScreenUnlock();
			// continue to GFX_DOUBLEBUFFER

		case GFX_DOUBLEBUFFER:
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
	_offScreenPitch = gVars->screenPitch;	// direct screen / flipping use this, set later if double buffer
	_screenPitch = gVars->screenPitch;

	_overlayVisible = false;
	_quitCount = 0;

	if (OPTIONS(optIsCollapsible))
		SysSetOrientationTriggerState(sysOrientationTriggerDisabled);

	// check HiRes+
	if (_mode == GFX_WIDE) {
		if (gVars->slkRefNum != sysInvalidRefNum) {
			if (!(w == 320 && h == 200 && gVars->slkVersion != vskVersionNum3)) {	// only for 320x200 games and not for UX50 at this time
				warning("Wide display not avalaible for this game, switching to GFX_NORMAL mode.");
				_mode = GFX_NORMAL;
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
		} else if (OPTIONS(optHasWideMode)) {
/*			UInt32 width = hrWidth;
			UInt32 height= hrHeight;
			UInt32 depth = 16;
			Boolean color = true;
			Err e;
			e = WinScreenMode (winScreenModeSet, &width, &height, &depth, &color );
*/			//SysSetOrientation(sysOrientationLandscape);
			PINSetInputAreaState(pinInputAreaClosed);
			StatHide();

		}
	}

	if (_mode == GFX_WIDE) {
		_screenOffset.x = 0;
		_screenOffset.y = 10;
		_screenOffset.addr = _screenOffset.y;
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

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn)
		undraw_mouse();

	byte *dst = _offScreenP + y * _offScreenPitch + x;

	if (_offScreenPitch == pitch && pitch == w) {
		memcpy (dst, buf, h * w);
	} else {
		do {
			memcpy(dst, buf, w);
			dst += _offScreenPitch;
			buf += pitch;
		} while (--h);
	}
}

// only avalaible for 320x200 games, so use values instead of vars
#define WIDE_PITCH			320
#define WIDE_HALF_HEIGHT	100	// 200 / 2

void OSystem_PALMOS::update_screen__wide() {
	Coord x, y;
	UInt32 next = _screenOffset.y << 1;
	UInt8 *dst = _screenP;
	UInt8 *src1 = _offScreenP + WIDE_PITCH - 1;
	UInt8 *src2 = src1;

	for (x = 0; x < _screenWidth >> 1; x++) 
	{
		for (y = 0; y < WIDE_HALF_HEIGHT; y++) 
		{
			*dst++ = *src1;
			src1 += WIDE_PITCH;
			*dst++ = *src1;
			*dst++ = *src1;
			src1 += WIDE_PITCH;
		}
		src1 = --src2;
		dst += next;

		for (y = 0; y < WIDE_HALF_HEIGHT; y++) 
		{
			*dst++ = *src1;
			src1 += WIDE_PITCH;
			*dst++ = *src1;
			*dst++ = *src1;
			src1 += WIDE_PITCH;
		}
		src1 = --src2;
		dst += next;

		MemMove(dst, dst - WIDE_PITCH, 300);	// 300 = 200 x 1.5
		dst += WIDE_PITCH;
	}	

	WinScreenUnlock();
	_screenP = WinScreenLock(winLockCopy) + _screenOffset.addr;
}

void OSystem_PALMOS::update_screen__flipping() {
	RectangleType r, dummy;
	UInt8 *screen;
	UInt32 size = _screenWidth * _screenHeight + 6400; // 10 pix top and bottom border, see TODO
	Boolean shaked = false;
	UInt32 move = 0;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		if (_useHRmode) {
			RctSetRectangle(&r, _screenOffset.x, _screenOffset.y - _new_shake_pos, _screenWidth, _screenHeight + (_new_shake_pos << 2));
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, _new_shake_pos, &dummy);
		} else {
			// TODO : need to change this to line copy if palm give us an HiRes+ device and change 3200/6400 value to the good one depending on the screen pitch
			move = (_new_shake_pos * _screenWidth);
			screen = _offScreenP - 3200;
			MemMove(screen + move, screen, size);
		}

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
		if (_useHRmode) {
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, _new_shake_pos, &dummy);
		} else {
			// TODO : need to change this to line copy if palm give us an HiRes+ device
			screen = _offScreenP - 3200;
			MemMove(screen, screen + move, size);
		}
	}

}

void OSystem_PALMOS::update_screen__dbuffer() {
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
#ifndef DISABLE_TAPWAVE
	Err e;
	TwGfxSurfaceType *src;
	TwGfxSurfaceType *dst;
	TwGfxRectType d = {0,0,480,300}, s = {0,0,320,200};
	TwGfxSurfaceInfoType t;
	TwGfxPointType dd = {0,0};

	t.size = sizeof(TwGfxSurfaceInfoType);
	t.width = 320;
	t.height = 200;
	t.rowBytes = 640;
	t.location = twGfxLocationAcceleratorMemory;
	t.pixelFormat = twGfxPixelFormatRGB565_LE;
/*	
	ColorTableType table;
	RGBColorType *rgb = ColorTableEntries (table);
	table.numEntries = 256;
	*rgb = _currentPalette;
	*/
	//BitmapType *newBmp = 
	

	
	TwGfxType *gfx;
	e = TwGfxOpen(&gfx, NULL);
	e = TwGfxAllocSurface(gfx, &src, &t);
	e = TwGfxDrawPalmBitmap(src, &dd, WinGetBitmap(_offScreenH));
	
	e = TwGfxGetPalmDisplaySurface(gfx, &dst);
	e = TwGfxStretchBlt(dst, &d, src, &s); 
	e = TwGfxFreeSurface(src);
	e = TwGfxClose(gfx);
#endif
//#if 0
		byte *src = _offScreenP;
		byte *dst = _screenP + move;
		UInt16 h = _screenHeight - _new_shake_pos;
		do {
			memcpy(dst, src, _screenWidth);
			dst += _screenPitch;
			src += _offScreenPitch;
		} while (--h);
//#endif
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

#define MD_NONE 0
#define MD_CTRL 1
#define MD_ALT	2

static void drawKeyState(OSystem_PALMOS *sys, UInt8 state) {
	UInt8 i,j;
	UInt16 bmpID = 3000 + state - 1;
	
	MemHandle hTemp;
	BitmapType *bmTemp;
	UInt32 *bmData;
	
	UInt32 pitch = sys->_screenPitch;
	UInt8 *scr = sys->_screenP + sys->_screenPitch * (sys->get_height() + 2) + 2;

	hTemp	= DmGetResource(bitmapRsc,bmpID);
	
	if (hTemp) { // draw
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);
		bmData	= (UInt32 *)BmpGetBits(bmTemp);

		for (i = 0; i < 7; i++) {
			for (j = 0; j < 32; j++) {
				if (*bmData & (1 << (31-j)))
					*scr++ = gVars->indicator.on;
				else
					*scr++ = gVars->indicator.off;
			}
			scr += pitch - 32;
			bmData++;
		}

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
	} else {	// undraw
		for (i = 0; i < 7; i++) {
			for (j = 0; j < 32; j++) {
				*scr++ = gVars->indicator.off;
			}
			scr += pitch - 32;
		}
	}
}

static void drawNumPad(OSystem_PALMOS *sys, UInt8 color) {
	UInt8 i,j,k;
	UInt16 bmpID = 3010;	// numPadBitmap 64x34

	MemHandle hTemp;
	BitmapType *bmTemp;
	UInt32 *bmData;

	UInt32 pitch = sys->_screenPitch;
	UInt8 *scr = sys->_screenP + sys->_screenPitch * (sys->get_height() + 2);

	scr += (sys->get_width() >> 1) - 32;
	hTemp	= DmGetResource(bitmapRsc,bmpID);
	
	if (hTemp) {
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);
		bmData	= (UInt32 *)BmpGetBits(bmTemp);

		for (i = 0; i < 34; i++) {
			for (k = 0; k < 2; k++) {
				for (j = 0; j < 32; j++) {
					if (*bmData & (1 << (31-j)))
						*scr++ = color;
					else
						*scr++ = 0;
				}
				bmData++;
			}
			scr += pitch - 64;
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
				drawKeyState(this, _lastKeyModifier);
			
			if(_useNumPad)
				drawNumPad(this, gVars->indicator.on);
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

void OSystem_PALMOS::warp_mouse(int x, int y) {}

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
	
	if (shake_pos == 0 && _vibrate)
	{
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
	UInt32 delay = (SysTicksPerSecond() * msecs) / 1000;

	if (delay)
		SysTaskDelay(delay);
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

void OSystem_PALMOS::SimulateArrowKeys(Event *event, Int8 iHoriz, Int8 iVert) {
	Int16 x = _mouseCurState.x;
	Int16 y = _mouseCurState.y;

	if (_lastKeyPressed != -1) {
		_lastKeyRepeat += 100;

		if (_lastKeyRepeat > 3200)
			_lastKeyRepeat = 3200;
	}
	else
		_lastKeyRepeat = 100;

	x = x + iHoriz * (_lastKeyRepeat / 100);
	y = y + iVert * (_lastKeyRepeat / 100);

	x = (x < 0				) ? 0					: x;
	x = (x >= _screenWidth	) ? _screenWidth - 1	: x;
	y = (y < 0				) ? 0					: y;
	y = (y >= _screenHeight	) ? _screenHeight - 1	: y;


	event->event_code = EVENT_MOUSEMOVE;
	event->mouse.x = x;
	event->mouse.y = y;
	set_mouse_pos(x, y);
}

static void getCoordinates(EventPtr event, Boolean wide, Coord *x, Coord *y) {
	if (wide) {
		*y = (event->screenX << 2) / 3;
		*x = 320 - (event->screenY << 2) / 3 - 1;	// wide only for 320x200, so ...
	} else {
		*x = event->screenX << 1;
		*y = event->screenY << 1;
	}
}

bool OSystem_PALMOS::poll_event(Event *event) {
	EventType ev;
	Boolean handled;
	UInt32 current_msecs;
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

	current_msecs = get_msecs();

	// sound handler
	if(_sound.active)
		check_sound();

	// timer handler
//	if (_timer.active)
//		 _timer.callback(_timer.duration);
	if (_timer.active && (current_msecs >= _timer.nextExpiry)) {
		_timer.duration = _timer.callback(_timer.duration);
		_timer.nextExpiry = current_msecs + _timer.duration;
	}

/*
	if (_timer.active) {
		if (current_msecs - _timer.next_expiry >= 10)
			_timer.sleep = false;

		if (!_timer.sleep) {
			_timer.sleep = true;
			while (_timer.next_expiry < current_msecs) {
				_timer.next_expiry += 10;
				_timer.duration = _timer.callback(_timer.duration);
			}
		}
	}
*/
	if (_selfQuit)
		quit();

	for(;;) {
		EvtGetEvent(&ev, evtNoWait);

		// check for hardkey repeat
		keyCurrentState = KeyCurrentState();
		if (_lastKeyPressed != -1 && _lastKeyPressed != vchrCalc &&
			!(	(keyCurrentState & keyBitHard2) ||
				(keyCurrentState & keyBitPageUp) ||
				(keyCurrentState & keyBitPageDown) ||
				(keyCurrentState & keyBitHard3)
				)
			) {
					_lastKeyPressed = -1;
		}

		if (ev.eType == nilEvent)
			return false;

		if (ev.eType == keyDownEvent) {
			switch (ev.data.keyDown.chr) {
				case vchrLaunch:
					_lastKeyPressed = -1;
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 27;
					event->kbd.ascii = 27;
					event->kbd.flags = 0;
					return true;

				case vchrJogPushRepeat:
				case vchrMenu:
					_lastKeyPressed = -1;
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 319;
					event->kbd.ascii = 319;
					event->kbd.flags = 0;
					return true;

				case vchrBrightness:
				case vchrContrast:
				case vchrFind:
					WinPalette(winPaletteSet, 0, 256, _currentPalette);
					break;

				case vchrCalc:
					if (_lastKeyPressed == vchrCalc)
						if ((get_msecs() - _exit_delay) <= (EXITDELAY)) {
							event->event_code = EVENT_QUIT;
							_selfQuit = true;
						}

					_exit_delay = get_msecs();
					_lastKeyPressed = vchrCalc;
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
			}
			
//			if (gVars->options & opt5WayNavigator)*/ {
//			} else {
				// mouse emulation for device without 5-Way navigator
				switch (ev.data.keyDown.chr) {
					case vchrJogBack:
					case vchrHard4: // right button
						event->event_code = EVENT_RBUTTONDOWN;
						event->mouse.x = _mouseCurState.x;
						event->mouse.y = _mouseCurState.y;
						_lastKeyPressed = -1;
						return true;

					case vchrJogPush:
					case vchrHard1: // left button
						event->event_code = EVENT_LBUTTONDOWN;
						event->mouse.x = _mouseCurState.x;
						event->mouse.y = _mouseCurState.y;
						_lastKeyPressed = -1;
						return true;

					case vchrHard2:	// move left
						SimulateArrowKeys(event, -1, 0);
						_lastKeyPressed = vchrHard2;
						return true;
						
					case vchrPageUp: // move up
						SimulateArrowKeys(event, 0, -1);
						_lastKeyPressed = vchrPageUp;
						return true;

					case vchrPageDown: // move down
						SimulateArrowKeys(event, 0, 1);
						_lastKeyPressed = vchrPageDown;
						return true;

					case vchrHard3: // move right
						SimulateArrowKeys(event, 1, 0);
						_lastKeyPressed = vchrHard3;
						return true;
//				}
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
				_lastKeyPressed = -1;
				//if (ev.data.keyDown.modifiers & shiftKeyMask) b |= KBD_SHIFT;

				if (ev.data.keyDown.chr == vchrCommand && (ev.data.keyDown.modifiers & commandKeyMask)) {
					_lastKeyModifier++;
					_lastKeyModifier %= 3;
					drawKeyState(this, _lastKeyModifier);			

				} else {
					byte b = 0;
					if (_lastKeyModifier == MD_CTRL) b = KBD_CTRL;
					if (_lastKeyModifier == MD_ALT) b = KBD_ALT;
					
					if  ((ev.data.keyDown.chr == 'z' && b == KBD_CTRL) || (b == KBD_ALT && ev.data.keyDown.chr == 'x')) {
						event->event_code = EVENT_QUIT;
						_selfQuit = true;

					} else if (ev.data.keyDown.chr == 'n' && b == KBD_CTRL) {
						UInt8 *scr = _screenP + _screenWidth * (_screenHeight + 2);
						_useNumPad = !_useNumPad;
						drawNumPad(this, _useNumPad ? gVars->indicator.on : 0);
					}

					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = ev.data.keyDown.chr;
					event->kbd.ascii = (ev.data.keyDown.chr>='a' && ev.data.keyDown.chr<='z' && (event->kbd.flags & KBD_SHIFT) ? ev.data.keyDown.chr &~ 0x20 : ev.data.keyDown.chr);
					event->kbd.flags = b;
					_lastKeyModifier = MD_NONE;
					drawKeyState(this, _lastKeyModifier);			
				}
				return true;
			}

		case penMoveEvent:
			getCoordinates(&ev, (_mode == GFX_WIDE), &x, &y);

			if ((y - _screenOffset.y) > _screenHeight || (y - _screenOffset.y) < 0 || (x - _screenOffset.x) > _screenWidth || (x - _screenOffset.x) < 0)
				return true;

			if (_lastEvent != penMoveEvent && (abs(y - event->mouse.y) <= 2 || abs(x - event->mouse.x) <= 2)) // move only if
				return true;

			_lastEvent = penMoveEvent;
			event->event_code = EVENT_MOUSEMOVE;
			event->mouse.x = x - _screenOffset.x;
			event->mouse.y = y - _screenOffset.y;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case penDownEvent:
			getCoordinates(&ev, (_mode == GFX_WIDE), &x, &y);

			if (_useNumPad) {
				Coord x2 = _screenOffset.x + (_screenWidth >> 1) - 20; // - 64 / 2 + 12
				Coord y2 = _screenOffset.y + _screenHeight + 2;

				if (y >= y2 && y < (y2 + 34) && x >= x2 && x < (x2 + 64)) {	// numpad location
					UInt8 key = '1';
					key += 9 - ( (3 - ((x - x2) / 13)) + (3 * ((y - y2) / 11)) );

					_lastEvent = keyDownEvent;
					_lastKeyPressed = -1;
					
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = key;
					event->kbd.ascii = key;
					event->kbd.flags = 0;
					return true;
				}
			}

			_lastEvent = penDownEvent;
			if ((y - _screenOffset.y) > _screenHeight || (y - _screenOffset.y) < 0 || (x - _screenOffset.x) > _screenWidth || (x - _screenOffset.x) < 0)
				return true;

			event->event_code = EVENT_LBUTTONDOWN;
			event->mouse.x = x - _screenOffset.x;
			event->mouse.y = y - _screenOffset.y;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case penUpEvent:
			getCoordinates(&ev, (_mode == GFX_WIDE), &x, &y);
			event->event_code = EVENT_LBUTTONUP;

			if ((y - _screenOffset.y) > _screenHeight || (y - _screenOffset.y) < 0 || (x - _screenOffset.x) > _screenWidth || (x - _screenOffset.x) < 0)
				return true;

			event->mouse.x = x - _screenOffset.x;
			event->mouse.y = y - _screenOffset.y;
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

	case PROP_GET_SAMPLE_RATE:
		return 8000;
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
//	free(_sndDataP);
	free(_sndTempP);

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

	_lastKeyPressed = -1;
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
	_isSndPlaying = false;
	_sndTempP = (UInt8 *)calloc(SND_BLOCK,1);
	_sndDataP = NULL;
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
	// FIXME : use this only if the Hi-Density feature is present ?
	if (!_useHRmode) WinSetCoordinateSystem(kCoordinatesNative);

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

	// FIXME : use this only if the Hi-Density feature is present ?
	if (!_useHRmode) WinSetCoordinateSystem(kCoordinatesStandard);
	WinSetDrawWindow(_screenH);
	// Prevent crash on Clie device using successive [HR]WinScrollRectangle !
	SysTaskDelay(1);
}
/*
typedef struct {
	bool played;
	void *data;
	UInt8 count;
} CallbackDataType;

CallbackDataType mycallback = {true, NULL, 0};

static Err sndCallback(void* UserDataP, SndStreamRef stream, void* bufferP, UInt32 frameCount) {
//	SoundDataType *snd = (SoundDataType *)UserDataP;
//	snd->proc(snd->param, (UInt8 *)bufferP, frameCount);

	CallbackDataType *ptr = (CallbackDataType *)UserDataP;
	MemSet(bufferP,frameCount,0);

	if (!ptr->played) {
		MemMove(bufferP, ptr->data, SND_BLOCK * 3);
		ptr->played = true;
	} else {
		MemSet(bufferP,SND_BLOCK,0);
	}
	return errNone;
}
*/
//FILE *mf = NULL;

bool OSystem_PALMOS::set_sound_proc(SoundProc proc, void *param, SoundFormat format) {
	_sound.active = true;
	_sound.proc = proc;
	_sound.param = param;
	_sound.format = format;
/*
	mycallback.data = _sndTempP;

	Err e;
	//_sound.active = false;
	e = SndStreamCreate(&_sound.sndRefNum, sndOutput, 8000, sndInt16Big, sndStereo, sndCallback, &mycallback, SND_BLOCK *3, false);
	e = SndStreamStart(_sound.sndRefNum);

	//mf = fopen("/PALM/Programs/ScummVM/dump.wav","wb");
	*/
	return _sound.active;
}

void OSystem_PALMOS::clear_sound_proc() {
/*	SndStreamStop(_sound.sndRefNum);
	SndStreamDelete(_sound.sndRefNum);
	*/
	//fclose(mf);
	
	_sound.active = false;
}

void OSystem_PALMOS::check_sound() {
	// currently not supported
	// but i need to use this function to prevent out of memory
	// on games because the sound buffer growns and it's never
	// freed.
//	if (mycallback.played) {
		_sound.proc(_sound.param, _sndTempP, SND_BLOCK);
	/*	
		if (mycallback.count == 3) {
			mycallback.played = false;
			mycallback.count = 0;
		}
		//if (mf)
		//	fwrite(_sndTempP, SND_BLOCK, 1, mf);
	}*/
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

	if (w > _screenWidth - x) {
		w = _screenWidth - x;
	}

	if (h > _screenHeight - y) {
		h = _screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	undraw_mouse();

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
