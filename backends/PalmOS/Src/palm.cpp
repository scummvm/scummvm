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

#include "palm.h"
#include "vibrate.h"

#define EXITDELAY			(500) // delay to exit : calc button : double tap 1/500 sec
#define ftrOverlayPtr		(1000)
#define ftrBackupPtr		(1001)
#define	SND_BLOCK			(3072)

OSystem *OSystem_PALMOS::create(UInt16 gfx_mode, bool full_screen) {
	OSystem_PALMOS *syst = new OSystem_PALMOS();
	syst->_mode = gfx_mode;
	syst->_vibrate = gVars->vibrator;
	syst->_fullscreen = (full_screen && gVars->slkRefNum != sysInvalidRefNum);
	return syst;
}

OSystem *OSystem_PALMOS_create(int gfx_mode, bool full_screen) {
	return OSystem_PALMOS::create(gfx_mode, full_screen);
}

void OSystem_PALMOS::set_palette(const byte *colors, uint start, uint num) {
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

	switch(_mode)
	{
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
			_offScreenH	= WinCreateOffscreenWindow(_screenWidth, _screenHeight, screenFormat, &e);
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
	_offScreenPitch = gVars->screenFullWidth;
	_screenPitch = gVars->screenFullWidth;

	_overlayVisible = false;
	_quit = false;


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
		if (gVars->slkVersion == vskVersionNum1) {
			SilkLibEnableResize (gVars->slkRefNum);
			SilkLibResizeDispWin(gVars->slkRefNum, silkResizeMax);
			SilkLibDisableResize(gVars->slkRefNum);
		} else {
			VskSetState(gVars->slkRefNum, vskStateEnable, (gVars->slkVersion != vskVersionNum3 ? vskResizeVertically : vskResizeHorizontally));
			VskSetState(gVars->slkRefNum, vskStateResize, vskResizeNone);
			VskSetState(gVars->slkRefNum, vskStateEnable, vskResizeDisable);
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
		byte *src = _offScreenP;
		byte *dst = _screenP + move;
		UInt16 h = _screenHeight - _new_shake_pos;
		do {
			memcpy(dst, src, _screenWidth);
			dst += _screenPitch;
			src += _screenWidth;
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

	scr += sys->get_width() - 66;

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
	if(_quit)
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
	SysTaskDelay((SysTicksPerSecond()*msecs)/1000);
}

void OSystem_PALMOS::create_thread(ThreadProc *proc, void *param) {
	if (_threadCounter == MAX_THREAD) {
		warning("Cannot create thread.");
		return;
	}

	_thread[_threadCounter].active = true;
	_thread[_threadCounter].proc = proc;
	_thread[_threadCounter].param = param;
	_thread[_threadCounter].old_time = get_msecs();
	_thread[_threadCounter].sleep = true;

	_threadCounter++;
}

void OSystem_PALMOS::set_timer(int timer, int (*callback)(int)) {
	if (callback != NULL) {
		_timer.duration = timer;
		_timer.next_expiry = get_msecs() + timer;
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

	if(_quit) {
		exit(0);	// resend an exit event
		return false;
	}

	current_msecs = get_msecs();
	//thread handler
	for(_threadID = 0; _threadID < _threadCounter; _threadID++) {
		if (_thread[_threadID].active)
			_thread[_threadID].proc(_thread[_threadID].param);
	}

	// sound handler
	if(_sound.active)
		check_sound();
	
	// timer handler
	if (_timer.active && (current_msecs >= _timer.next_expiry)) {
		_timer.duration = _timer.callback(_timer.duration);
		_timer.next_expiry = current_msecs + _timer.duration;
	}

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
						if ((get_msecs() - _exit_delay) <= (EXITDELAY))
							_selfQuit = true;

					_exit_delay = get_msecs();
					_lastKeyPressed = vchrCalc;
					return true;

				// mouse emulation
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

				case vchrJogBack:
				case vchrHard4: // right button
					event->event_code = EVENT_RBUTTONDOWN;
					event->mouse.x = _mouseCurState.x;
					event->mouse.y = _mouseCurState.y;
					_lastKeyPressed = -1;
					return true;

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
				Coord x2 = _screenOffset.x + _screenWidth - 64 - 2;
				Coord y2 = _screenOffset.y + _screenHeight + 2;
				if (y >= y2 && y < (y2 + 34) && x >= x2 && x < (x2 + 64)) {	// numpad location
					UInt8 key = '1';
					key += 9 - ( (3 - ((x - x2) / 21)) + (3 * ((y - y2) / 11)) );

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
		if (StrCaselessCompare(value->caption,"ScummVM") == 0)
			return 1;
		
		Char *caption = "Loading...\0";
		UInt16 h = FntLineHeight() + 2;
		UInt16 w, y;
		
		WinSetTextColor(255);
		WinSetForeColor(255);

		if (_useHRmode) {
			HRFntSetFont(gVars->HRrefNum,hrTinyBoldFont);
			y = 160 - (h >> 1);
			w = FntCharsWidth(caption,StrLen(caption));
			w = (320 - w) >> 1;
			HRWinDrawChars(gVars->HRrefNum, caption, StrLen(caption), w, y - h);

			HRFntSetFont(gVars->HRrefNum,hrTinyFont);
			w = FntCharsWidth(value->caption, StrLen(value->caption));
			w = (320 - w) >> 1;
			HRWinDrawChars(gVars->HRrefNum, value->caption, StrLen(value->caption), w, y);
		} else {
			FntSetFont(boldFont);
			y = 80 - (h >> 1);
			w = FntCharsWidth(caption, StrLen(caption));
			w = (160 - w) >> 1;
			WinDrawChars(caption, StrLen(caption), w, y);
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
	if (_quit)
		return;

	if (_selfQuit && g_scumm)
		g_scumm->_quit = true;

	free(_currentPalette);
	free(_mouseBackupP);
//	free(_sndDataP);
	free(_sndTempP);

	if (_msaRefNum != sysInvalidRefNum) {
		MsaStop(_msaRefNum, true);	// stop the current track if any (needed if we use enforce open to prevent the track to play after exit)
		MsaLibClose(_msaRefNum, msaLibOpenModeAlbum);
	}

	unload_gfx_mode();
	_quit = true;
	exit(1);
}

void OSystem_PALMOS::draw_mouse() {
	if (_mouseDrawn || !_mouseVisible || _quit)
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
	if (!_mouseDrawn || _quit)
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


// TODO : unify lib check functions
static UInt16 checkMSA() {
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	Err error = errNone;
	UInt16 refNum = sysInvalidRefNum;

	if (!(error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP))) {
		// not found with audio adapter ?!
		//if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrMsa) {		
			if ((error = SysLibFind(sonySysLibNameMsa, &refNum)))
				if (error == sysErrLibNotFound)
					error = SysLibLoad(sonySysFileTMsaLib, sonySysFileCMsaLib, &refNum);

			if (!error) {
//				Char buf[100];
//				StrPrintF(buf,"MSA refNum %ld, Try to open lib ...", refNum);
//				FrmCustomAlert(1000,buf,0,0);
				MsaLibClose(refNum, msaLibOpenModeAlbum);	// close the lib if we previously let it open (?) Need to add Notify for sonySysNotifyMsaEnforceOpenEvent just in case ...
				error = MsaLibOpen(refNum, msaLibOpenModeAlbum);			
/*				switch (error) {
				case msaErrAlreadyOpen:
					FrmCustomAlert(1000,"msaErrAlreadyOpen",0,0);
					break;
				case msaErrMemory:
					FrmCustomAlert(1000,"msaErrMemory",0,0);
					break;
				case msaErrDifferentMode:
					FrmCustomAlert(1000,"msaErrDifferentMode",0,0);
					break;
				case expErrCardNotPresent:
					FrmCustomAlert(1000,"expErrCardNotPresent",0,0);
					break;
				}
*/
				if (error == msaErrAlreadyOpen)
					error = MsaLibEnforceOpen(refNum, msaLibOpenModeAlbum, appFileCreator);
			
				error = (error != msaErrStillOpen) ? error : errNone;
			}
		//}
	}

	if (error)
		refNum = sysInvalidRefNum;
	
	return refNum;
}

void OSystem_PALMOS::stop_cdrom() {	/* Stop CD Audio in 1/10th of a second */
	_msaStopTime = get_msecs() + 100;
	_msaLoops = 0;
	return;
}

// frames are 1/75 sec
#define CD_FPS 75
#define TO_MSECS(frame)	((UInt32)((frame) * 1000 / CD_FPS))
// consider frame at 1/1000 sec
#define TO_SEC(msecs)	((UInt16)((msecs) / 1000))
#define TO_MIN(msecs)	((UInt16)(TO_SEC((msecs)) / 60))
#define FROM_MIN(mins)	((UInt32)((mins) * 60 * 1000))
#define FROM_SEC(secs)	((UInt32)((secs) * 1000))

#define FRAMES_TO_MSF(f, M,S,F) {                                       \
        int value = f;                                                  \
        *(F) = value%CD_FPS;                                            \
        value /= CD_FPS;                                                \
        *(S) = value%60;                                                \
        value /= 60;                                                    \
        *(M) = value;                                                   \
}

static void doErr(Err e, const Char *msg) {
	Char err[100];
	StrPrintF(err, "%ld : " , e);
	StrCat(err,msg);
	FrmCustomAlert(1000,err,0,0);
}

void OSystem_PALMOS::play_cdrom(int track, int num_loops, int start_frame, int duration) {
	if (!_isCDRomAvalaible)
		return;

	if (!num_loops && !start_frame)
		return;
	
	Err e;
	
	// open MSA lib if needed
	if (_msaRefNum == sysInvalidRefNum) {
		_msaRefNum = checkMSA();

		// if not found disable CD-Rom if needed
		if (_msaRefNum == sysInvalidRefNum) {
//			FrmCustomAlert(1000,"MSA Lib not found",0,0);
			_isCDRomAvalaible = (gVars->music.setDefaultTrackLength);
			return;
		} else {
			MsaOutSetVolume(_msaRefNum, 20, 20);
			Err e = MsaOutStartBeep(_msaRefNum, 1000, msaOutBeepPatternOK);
//			debug(0, ">> MSALib successfully loaded. Play beep result %d", e);
		}

		UInt32 dummy, albumIterater = albumIteratorStart;
		Char nameP[256];
		MemSet(&_msaAlbum, sizeof(_msaAlbum), 0);
		_msaAlbum.maskflag = msa_INF_ALBUM;
		_msaAlbum.code = msa_LANG_CODE_ASCII;
		_msaAlbum.nameP = nameP;
		_msaAlbum.fileNameLength = 256;

		e = MsaAlbumEnumerate(_msaRefNum, &albumIterater, &_msaAlbum);
//		if (e) doErr(e, "MsaAlbumEnumerate");
		e = MsaSetAlbum(_msaRefNum, _msaAlbum.albumRefNum, &dummy);
//		if (e) doErr(e, "MsaSetAlbum");
		e = MsaGetPBRate(_msaRefNum, &_msaPBRate);
//		if (e) doErr(e, "MsaGetPBRate");
		// TODO : use RMC to control volume
	}
/*
	if (start_frame > 0)
		start_frame += CD_FPS >> 1;
*/
	if (duration > 0)
		duration += 5;

//	debug(0, ">> Request : track %d / loops : %d / start : %d / duration : %d", track, num_loops, start_frame, duration);
	
	_msaLoops = num_loops;
	_msaTrack = track + gVars->music.firstTrack - 1;	// first track >= 1 ?, not 0 (0=album)
	_msaStartFrame = TO_MSECS(start_frame);
	_msaEndFrame = TO_MSECS(duration);

//	debug(0, ">> To MSECS : start : %d / duration : %d", _msaStartFrame, _msaEndFrame);

	// if gVars->MP3 audio track
//	Err e;
	MemHandle trackH;
	
	// stop current play if any
	MsaStop(_msaRefNum, true);
	// retreive track infos
	e = MsaGetTrackInfo(_msaRefNum, _msaTrack, 0, msa_LANG_CODE_ASCII, &trackH);
//	if (e) doErr(e, "MsaGetTrackInfo");
	// track exists
	if (!e && trackH) {
		MsaTime tTime;
		UInt32 SU, fullLength;
		MsaTrackInfo *tiP;
	
		// FIXME : this enable MsaSuToTime to return the right value
		MsaPlay(_msaRefNum, _msaTrack, 0, _msaPBRate);
		MsaStop(_msaRefNum, true);
		
		tiP = (MsaTrackInfo *)MemHandleLock(trackH);	
		MsaSuToTime(_msaRefNum, tiP->totalsu, &tTime);
		SU = tiP->totalsu;
		MemPtrUnlock(tiP);
		MemHandleFree(trackH);
		
//		debug(0, ">> SU of track : %d (%d%:%d.%d)", SU, tTime.minute, tTime.second, tTime.frame);

//		Char buf[200];
//		StrPrintF(buf,"Track : %ld - %ld%:%ld.%ld (%ld)", track, tTime.minute, tTime.second, tTime.frame, SU);
//		FrmCustomAlert(1000,buf,0,0);

		_msaStopTime = 0;
		fullLength = FROM_MIN(tTime.minute) + FROM_SEC(tTime.second) + tTime.frame;

//		debug(0, ">> Full length : %d", fullLength);
		
		if (_msaEndFrame > 0) {
			_msaTrackLength = _msaEndFrame;
		} else if (_msaStartFrame > 0) {
			_msaTrackLength = fullLength;
			_msaTrackLength -= _msaStartFrame;
		} else {
			_msaTrackLength = fullLength;
		}
		
		// try to play the track
		if (start_frame == 0 && duration == 0) {
			MsaPlay(_msaRefNum, _msaTrack, 0, _msaPBRate);
		} else {
			// FIXME : MsaTimeToSu doesn't work ... (may work with previous FIXME)
			_msaTrackStart = (UInt32) ((float)(_msaStartFrame) / ((float)fullLength / (float)SU));
//			debug(0, ">> start at (float) : %d", _msaTrackStart);
/*
UInt32 f;
			FRAMES_TO_MSF(start_frame, &tTime.minute, &tTime.second, &f)
			tTime.frame = 0;
			MsaTimeToSu(_msaRefNum, &tTime, &SU);
			debug(0, ">> start at (MsaTimeToSu) : %d", SU);
*/
			MsaPlay(_msaRefNum, _msaTrack, _msaTrackStart, _msaPBRate);
		}

		_msaEndTime = get_msecs() + _msaTrackLength;
//		debug(0, ">> track length : %d / end : %d", _msaTrackLength, _msaEndTime);
	}
}

bool OSystem_PALMOS::poll_cdrom() {
	if (!_isCDRomAvalaible)
		return false;

	if (_msaRefNum == sysInvalidRefNum)
		return false;

	MsaPBStatus pb;
	MsaGetPBStatus(_msaRefNum, &pb);
	return (_msaLoops != 0 && (get_msecs() < _msaEndTime || pb.status != msa_STOPSTATUS));
}

void OSystem_PALMOS::update_cdrom() {
	if (!_isCDRomAvalaible)
		return;

	if (_msaRefNum == sysInvalidRefNum)
		return;

	// stop replay upon request of stop_cdrom()
	if (_msaStopTime != 0 && get_msecs() >= _msaStopTime) {
		MsaStop(_msaRefNum, true);
		_msaLoops = 0;
		_msaStopTime = 0;
		_msaEndTime = 0;
		return;
	}

	// not fully played
	if (get_msecs() < _msaEndTime)
		return;
		
	if (_msaLoops == 0) {
		MsaStop(_msaRefNum, true);
		return;
	}

	// track ends and last play, force stop if still playing
	if (_msaLoops != 1) {
		MsaPBStatus pb;
		MsaGetPBStatus(_msaRefNum, &pb);
	 	if (pb.status != msa_STOPSTATUS) {
//			debug(0,"Stop It now");
			MsaStop(_msaRefNum, true);
			return;
		}
	}

	// loop again ?
	if (_msaLoops > 0) {
		MsaStop(_msaRefNum, true); // stop if loop=0
		_msaLoops--;
	}

	// loop if needed
	if (_msaLoops != 0) {
		MsaStop(_msaRefNum, true);
//		debug(0,"Next loop : %d", _msaLoops);

		_msaEndTime = get_msecs() + _msaTrackLength;
		if (_msaStartFrame == 0 && _msaEndFrame == 0)
			MsaPlay(_msaRefNum, _msaTrack, 0, _msaPBRate);
		else
			MsaPlay(_msaRefNum, _msaTrack, _msaTrackStart, _msaPBRate);
	}
}

OSystem_PALMOS::OSystem_PALMOS() {
	_quit = false;
	_selfQuit = false; // prevent illegal access to g_scumm
	_current_shake_pos = 0;
	_new_shake_pos = 0;

	_paletteDirtyStart = 0;
	_paletteDirtyEnd = 0;
	
	memset(&_sound, 0, sizeof(SoundDataType));
	memset(_thread, 0, sizeof(_thread));
	_threadID = 0;
	_threadCounter = 0;
	
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
	
	// overlay
	_tmpScreenP = NULL;
	_tmpBackupP = NULL;
	
	// HiRes
	_useHRmode	= (gVars->HRrefNum != sysInvalidRefNum);
	
	// cd-rom
	_isCDRomAvalaible = (gVars->music.MP3 || gVars->music.setDefaultTrackLength); // true by default, set to false if MSA not avalaible
	_msaRefNum = sysInvalidRefNum;
	
	// sound
	_isSndPlaying = false;
	_sndTempP = (UInt8 *)MemPtrNew(SND_BLOCK);
	_sndDataP = NULL;
}

void OSystem_PALMOS::move_screen(int dx, int dy, int height) {

	// Short circuit check - do we have to do anything anyway?
	if ((dx == 0 && dy == 0) || height <= 0)
		return;

	// Hide the mouse
	if (_mouseDrawn)
		undraw_mouse();

	// FIXME : i divide dx/dy by 2 because WinScrollRectangle use low-density coordinates on Hi-Density
	// devices. Hope this will work, if not i will use the SDL-common definition with copy_rect :(

	// FIXME - calling copy_rect repeatedly is horribly inefficient, as it (un)locks the surface repeatedly
	// and it performs unneeded clipping checks etc.
	// Furthermore, this code is not correct, techincally: the pixels members of an SDLSource may be 0
	// while it is not locked (e.g. for HW surfaces which are stored in the graphic card's VRAM).

	RectangleType r, dummy;
	WinSetDrawWindow(_offScreenH);
	
	if (_useHRmode) {
		RctSetRectangle(&r, ((_offScreenH != _screenH) ? 0 : _screenOffset.x), ((_offScreenH != _screenH) ? 0 : _screenOffset.y), _screenWidth, _screenHeight);
	} else {
		RctSetRectangle(&r, ((_offScreenH != _screenH) ? 0 : _screenOffset.x >> 1), ((_offScreenH != _screenH) ? 0 : _screenOffset.y >> 1), _screenWidth >> 1, _screenHeight >> 1);
	}

	// vertical movement
	if (dy > 0) {
		// move down - copy from bottom to top
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, dy, &dummy);
		} else {
			WinScrollRectangle(&r, winDown, dy >> 1, &dummy);
//			for (int y = height - 1; y >= dy; y--)
//				copy_rect((byte *)_offScreenP + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
		}
	} else if (dy < 0) {
		// move up - copy from top to bottom
		dy = -dy;
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, dy, &dummy);
		} else {
			WinScrollRectangle(&r, winUp, dy >> 1, &dummy);
//			for (int y = dy; y < height; y++)
//				copy_rect((byte *)_offScreenP + _screenWidth * y, _screenWidth, 0, y - dy, _screenWidth, 1);
		}
	}

	// horizontal movement
	if (dx > 0) {
		// move right - copy from right to left
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winRight, dx, &dummy);
		} else {
			WinScrollRectangle(&r, winRight, dx >> 1, &dummy);
//			for (int x = _screenWidth - 1; x >= dx; x--)
//				copy_rect((byte *)_offScreenP + x - dx, _screenWidth, x, 0, 1, height);
		}
	} else if (dx < 0)  {
		// move left - copy from left to right
		dx = -dx;
		if (_useHRmode) {
			// need to set the draw window
			HRWinScrollRectangle(gVars->HRrefNum, &r, winLeft, dx, &dummy);
		} else {
			WinScrollRectangle(&r, winLeft, dx >> 1, &dummy);
//			for (int x = dx; x < _screenWidth; x++)
//				copy_rect((byte *)_offScreenP + x, _screenWidth, x - dx, 0, 1, height);
		}
	}
	
	WinSetDrawWindow(_screenH);
	SysTaskDelay(1); // prevent crash on Clie device using successive [HR]WinScrollRectangle !
}

bool OSystem_PALMOS::set_sound_proc(SoundProc *proc, void *param, SoundFormat format) {
	_sound.active = true;
	_sound.proc = proc;
	_sound.param = param;
	_sound.format = format;

	return _sound.active;
}

void OSystem_PALMOS::clear_sound_proc() {
	_sound.active = false;
}

void OSystem_PALMOS::check_sound() {
	// currently not supported
	// but i need to use this function to prevent out of memory
	// on zak256 because the sound buffer growns and it's never
	// freed.
	_sound.proc(_sound.param, _sndTempP, SND_BLOCK);
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
	NewGuiColor color = 255;
	byte nearest = 255;
	byte check;
	byte r2,g2,b2;

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

	return color;
}

void OSystem_PALMOS::ColorToRGB(byte color, uint8 &r, uint8 &g, uint8 &b) {
	r = _currentPalette[color].r;
	g = _currentPalette[color].g;
	b = _currentPalette[color].b;
}