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

#include "scumm.h"
#include "common/scaler.h"

#include "palm.h"
#include "vibrate.h"

#define EXITDELAY		(500) // delay to exit : calc button : double tap 1/500 sec
#define ftrOverlayPtr	(1000)

OSystem *OSystem_PALMOS::create(UInt16 gfx_mode) {

	OSystem_PALMOS *syst = new OSystem_PALMOS();
	syst->_mode = gfx_mode;
	syst->_vibrate = gVars->vibrator;
	return syst;
}

OSystem *OSystem_PALMOS_create(int gfx_mode) {
	return OSystem_PALMOS::create(gfx_mode);
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
	// palette for preload dialog
	set_palette(startupPalette,0,16);

	switch(_mode)
	{
		case GFX_FLIPPING:
			_offScreenP	= WinScreenLock(winLockErase) + _screeny;
			_screenP	= _offScreenP;
			gVars->screenLocked = true;
			_renderer_proc = &update_screen__flipping;
			break;
		case GFX_DOUBLEBUFFER:
			_screenH	= WinGetDisplayWindow();
			_screenP	= (byte *)(BmpGetBits(WinGetBitmap(_screenH))) + _screeny;
			_offScreenH	= WinCreateOffscreenWindow(_screenWidth, _screenHeight, screenFormat, &e);
			_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH)));
			_renderer_proc = &update_screen__dbuffer;
			break;
		case GFX_NORMAL:
		default:
			_offScreenH	= WinGetDisplayWindow();
			_offScreenP	= (byte *)(BmpGetBits(WinGetBitmap(_offScreenH))) + _screeny;
			_screenP	= _offScreenP;
			_renderer_proc = &update_screen__direct;
			break;
	}

	// try to allocate on storage heap
	FtrPtrNew(appFileCreator, ftrOverlayPtr, _screenWidth * _screenHeight, (void **)&_tmpScreenP);
	// failed ? dynamic heap
	if (!_tmpScreenP)
		_tmpScreenP = (byte *)malloc(_screenWidth * _screenHeight);
	_overlaySaved = false;

}

void OSystem_PALMOS::unload_gfx_mode() {
	switch (_mode)
	{
		case GFX_FLIPPING:
			WinScreenUnlock();
			break;
		case GFX_DOUBLEBUFFER:
			WinDeleteWindow(_offScreenH,false);
			break;
	}

	if (_tmpScreenP)
		if (MemPtrDataStorage(_tmpScreenP))
			FtrPtrFree(appFileCreator, ftrOverlayPtr);
		else
			free(_tmpScreenP);
}

void OSystem_PALMOS::init_size(uint w, uint h) {

	_screenWidth = w;
	_screenHeight = h;

	_overlay_visible = false;
	_quit = false;

	_decaly = (320-h)/2;
	_screeny= _decaly * 320;

	set_mouse_pos(200,150);

	_currentPalette = (RGBColorType*)calloc(sizeof(RGBColorType), 256);
	_mouseBackupP = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H);
	
	load_gfx_mode();
}

void OSystem_PALMOS::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {
	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouseDrawn)
		undraw_mouse();

	byte *dst = _offScreenP + y * _screenWidth + x;

	do {
		memcpy(dst, buf, w);
		dst += _screenWidth;
		buf += pitch;
	} while (--h);
}

void OSystem_PALMOS::update_screen__flipping()
{
	RectangleType r;
	UInt8 *screen;
	UInt32 size = _screenWidth * _screenHeight + 6400; // 10 pix top and bottom border
	Boolean shaked = false;
	UInt32 move = 0;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		if (gVars->HRrefNum) {
			RctSetRectangle(&r, 0, _decaly - _new_shake_pos, _screenWidth, _screenHeight + (_new_shake_pos << 2));
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, _new_shake_pos, NULL);
		} else {
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
	_offScreenP = WinScreenLock(winLockCopy) + _screeny;
	_screenP = _offScreenP;
	if (shaked) {
		if (gVars->HRrefNum) {
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, _new_shake_pos, NULL);
		} else {
			screen = _offScreenP - 3200;
			MemMove(screen, screen + move, size);
		}
	}

}

void OSystem_PALMOS::update_screen__dbuffer()
{
	UInt32 move = 0;
	UInt32 size = _screenWidth * _screenHeight;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		move = (_new_shake_pos * _screenWidth);
		// copy clear bottom of the screen to top to cover shaking image
		MemMove(_screenP, _screenP + size , move);

		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}

		_current_shake_pos = _new_shake_pos;
	}
	// update screen
	MemMove(_screenP + move, _offScreenP, size - move);
}

void OSystem_PALMOS::update_screen__direct()
{
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
	
	UInt32 pitch = sys->get_width();
	UInt8 *scr = sys->_screenP + sys->get_width() * (sys->get_height() + 2) + 2;

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
	
	UInt32 pitch = sys->get_width();
	UInt8 *scr = sys->_screenP +sys->get_width() * (sys->get_height() + 2);

	scr += pitch - 66;

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
			if (lastKeyModifier)
				drawKeyState(this, lastKeyModifier);
			
			if(_useNumPad)
				drawNumPad(this, gVars->indicator.on);
		}
	}

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

void *OSystem_PALMOS::create_thread(ThreadProc *proc, void *param) {
	_thread.active	= true;
	_thread.proc	= proc;
	_thread.param	= param;
	
	return 0;
}

void OSystem_PALMOS::set_timer(int timer, int (*callback)(int))
{
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
void *OSystem_PALMOS::create_mutex(void)
{
  return NULL;
}

void OSystem_PALMOS::lock_mutex(void *mutex)
{
}
 
void OSystem_PALMOS::unlock_mutex(void *mutex)
{
}

void OSystem_PALMOS::delete_mutex(void *mutex)
{
}

void OSystem_PALMOS::SimulateArrowKeys(Event *event, Int8 iHoriz, Int8 iVert, Boolean repeat) {
	Int16 x = _mouseCurState.x;
	Int16 y = _mouseCurState.y;

	if (repeat) {
		lastKeyRepeat += 100;
		
		if (lastKeyRepeat > 3200)
			lastKeyRepeat = 3200;
	}
	else
		lastKeyRepeat = 100;

	x = x + iHoriz * (lastKeyRepeat/100);
	y = y + iVert * (lastKeyRepeat/100);

	x = (x < 0				) ? 0				: x;
	x = (x >= _screenWidth	) ? _screenWidth-1	: x;
	y = (y < 0				) ? 0				: y;
	y = (y >= _screenHeight	) ? _screenHeight-1	: y;


	event->event_code = EVENT_MOUSEMOVE;
	event->mouse.x = x;
	event->mouse.y = y;
}

bool OSystem_PALMOS::poll_event(Event *event) {
	EventType ev;
	Boolean handled;
	uint32 current_msecs;
	UInt32 keyCurrentState = 0;
	Coord x, y;
	
	if(_quit)
		return false;

	current_msecs = get_msecs();
	//thread handler
	if (_thread.active)
		_thread.proc(_thread.param);

	// sound handler
	if(_sound.active)
		check_sound();
	
	// timer handler
	if (_timer.active && (current_msecs >= _timer.next_expiry)) {
		_timer.duration = _timer.callback(_timer.duration);
		_timer.next_expiry = current_msecs + _timer.duration;
	}

	for(;;) {
		EvtGetEvent(&ev, evtNoWait);

		if (ev.eType == nilEvent)
			return false;

		keyCurrentState = KeyCurrentState();

		if (ev.eType == keyDownEvent) {
			switch (ev.data.keyDown.chr) {
				case vchrLaunch:
					lastKeyPressed = -1;
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 27;
					event->kbd.ascii = 27;
					event->kbd.flags = 0;
					return true;

				case vchrMenu:
					lastKeyPressed = -1;
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
					if (lastKeyPressed == vchrCalc)
						if ((get_msecs() - _exit_delay) <= (EXITDELAY))
							quit();

					_exit_delay = get_msecs();
					lastKeyPressed = vchrCalc;
					return true;

				// mouse emulation
				case vchrHard1: // left button
					event->event_code = EVENT_LBUTTONDOWN;
					event->mouse.x = _mouseCurState.x;
					event->mouse.y = _mouseCurState.y;
					lastKeyPressed = -1;
					return true;

				case vchrHard2:	// move left
					SimulateArrowKeys(event, -1, 0, (lastKeyPressed == vchrHard2));
					lastKeyPressed = vchrHard2;
					return true;
					
				case vchrPageUp: // move up
					SimulateArrowKeys(event, 0, -1, (lastKeyPressed == vchrPageUp));
					lastKeyPressed = vchrPageUp;
					return true;

				case vchrPageDown: // move down
					SimulateArrowKeys(event, 0, 1, (lastKeyPressed == vchrPageDown));
					lastKeyPressed = vchrPageDown;
					return true;

				case vchrHard3: // move right
					SimulateArrowKeys(event, 1, 0, (lastKeyPressed == vchrHard3));
					lastKeyPressed = vchrHard3;
					return true;

				case vchrHard4: // right button
					event->event_code = EVENT_RBUTTONDOWN;
					event->mouse.x = _mouseCurState.x;
					event->mouse.y = _mouseCurState.y;
					lastKeyPressed = -1;
					return true;


				case vchrJogUp:
					event->event_code = EVENT_WHEELUP;
					return true;

				case vchrJogDown:
					event->event_code = EVENT_WHEELDOWN;
					return true;

				case vchrHardCradle:
					quit();
			}
		}
		// check for hardkey repeat
		if (lastKeyPressed != -1 && lastKeyPressed != vchrCalc &&
			!(	(keyCurrentState & keyBitHard2) ||
				(keyCurrentState & keyBitPageUp) ||
				(keyCurrentState & keyBitPageDown) ||
				(keyCurrentState & keyBitHard3)
				)
			) {
					lastKeyPressed = -1;
		}
		// prevent crash when alarm is raised
		handled = ((ev.eType == keyDownEvent) && 
						(ev.data.keyDown.modifiers & commandKeyMask) && 
						((ev.data.keyDown.chr == vchrAttnStateChanged) || 
						(ev.data.keyDown.chr == vchrAttnUnsnooze))); 

		// graffiti strokes, autooff, etc...
		if (!handled)
			if (SysHandleEvent(&ev))
				continue;

		// others events
		switch(ev.eType) {

		case keyDownEvent: {
				lastEvent = keyDownEvent;
				lastKeyPressed = -1;
				//if (ev.data.keyDown.modifiers & shiftKeyMask) b |= KBD_SHIFT;

				if (ev.data.keyDown.chr == vchrCommand && (ev.data.keyDown.modifiers & commandKeyMask)) {
					lastKeyModifier++;
					lastKeyModifier %= 3;
					drawKeyState(this, lastKeyModifier);			

				} else {
					byte b = 0;
					if (lastKeyModifier == MD_CTRL)	b = KBD_CTRL;
					if (lastKeyModifier == MD_ALT)	b = KBD_ALT;
					
					if  (ev.data.keyDown.chr == 'q' && b == KBD_CTRL) {
						quit();
					} else if (ev.data.keyDown.chr == 'n' && b == KBD_CTRL) {
						UInt8 *scr = _screenP + _screenWidth * (_screenHeight + 2);
						_useNumPad = !_useNumPad;
						drawNumPad(this, _useNumPad ? gVars->indicator.on : 0);
					}

					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = ev.data.keyDown.chr;
					event->kbd.ascii = (ev.data.keyDown.chr>='a' && ev.data.keyDown.chr<='z' && (event->kbd.flags & KBD_SHIFT) ? ev.data.keyDown.chr &~ 0x20 : ev.data.keyDown.chr);
					event->kbd.flags = b;
					lastKeyModifier = MD_NONE;
					drawKeyState(this, lastKeyModifier);			
				}
				return true;
			}

		case penMoveEvent:
			x = ev.screenX << 1;
			y = ev.screenY << 1;

			if ((y -_decaly) > _screenHeight || (y - _decaly) < 0)
				return true;

			if (lastEvent != penMoveEvent && (abs(y - event->mouse.y) <= 2 || abs(x - event->mouse.x) <= 2)) // move only if
				return true;

			lastEvent = penMoveEvent;
			event->event_code = EVENT_MOUSEMOVE;
			event->mouse.x = x;
			event->mouse.y = y - _decaly;
			return true;

		case penDownEvent:
			x = ev.screenX << 1;
			y = ev.screenY << 1;

			if (_useNumPad) {
				Coord y2 = _decaly + _screenHeight + 2;
				if (y >= y2 && y < (y2 + 34) && x >= 254 && x < 318) {	// numpad location
					UInt8 key = '1';
					key += 9 - ( (3 - ((x - 254) / 21)) + (3 * ((y - y2) / 11)) );

					lastEvent = keyDownEvent;
					lastKeyPressed = -1;
					
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = key;
					event->kbd.ascii = key;
					event->kbd.flags = 0;
					return true;
				}
			}

			lastEvent = penDownEvent;
			if ((y -_decaly) > _screenHeight || (y - _decaly) < 0)
				return true;

			event->event_code = EVENT_LBUTTONDOWN;
			event->mouse.x = x;
			event->mouse.y = y - _decaly;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case penUpEvent:
			event->event_code = EVENT_LBUTTONUP;

			if (ev.screenY*2-_decaly > _screenHeight || ev.screenY*2-_decaly < 0)
				return true;

			event->mouse.x = ev.screenX*2;
			event->mouse.y = ev.screenY*2 - _decaly;
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
		Char *build	= "Build on " __DATE__ ", " __TIME__ " GMT+1\0";
		UInt16 h0 = FntLineHeight() + 2;
		UInt16 w1;
		
		WinSetTextColor(255);
		WinSetForeColor(255);

		if (gVars->HRrefNum != sysInvalidRefNum) {
			HRFntSetFont(gVars->HRrefNum,hrTinyBoldFont);
			w1 = FntCharsWidth(caption,StrLen(caption));
			w1 = (320 - w1) / 2;
			HRWinDrawChars(gVars->HRrefNum,caption,StrLen(caption),w1,80);

			HRFntSetFont(gVars->HRrefNum,hrTinyFont);
			w1 = FntCharsWidth(value->caption,StrLen(value->caption));
			w1 = (320 - w1) / 2;
			HRWinDrawChars(gVars->HRrefNum,value->caption,StrLen(value->caption),w1,80 + h0);
			HRWinDrawLine(gVars->HRrefNum, 40, 85 + h0 * 2, 280, 85 + h0 * 2);
			w1 = FntCharsWidth(build,StrLen(build));
			w1 = (320 - w1) / 2;
			HRWinDrawChars(gVars->HRrefNum,build,StrLen(build),w1,90 + h0 * 2);
		} else {
			FntSetFont(boldFont);
			w1 = FntCharsWidth(caption,StrLen(caption));
			w1 = (160 - w1) / 2;
			WinDrawChars(caption,StrLen(caption),w1,40);
		}
		return 1;

	case PROP_OPEN_CD:
		break;

	case PROP_GET_SAMPLE_RATE:
		return SAMPLES_PER_SEC;
	}

	return 0;
}

void OSystem_PALMOS::quit() {
	exit(1);

	if (_quit)
		return;

	if (g_scumm)
		g_scumm->_quit = true;
	if (_currentPalette)
		free(_currentPalette);
	if (_mouseBackupP)
		free(_mouseBackupP);

	if (_sndTempP)
		MemPtrFree(_sndTempP);
	if (_sndDataP)
		MemPtrFree(_sndDataP);

	unload_gfx_mode();
	_quit = true;
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
	dst = _offScreenP + y * _screenWidth + x;
	while (h > 0) {
		int width = w;
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
		dst += _screenWidth - w;
		h--;
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
	const int old_mouse_h = _mouseOldState.h;
	int x,y;

	// No need to do clipping here, since draw_mouse() did that already

	dst = _offScreenP + old_mouse_y * _screenWidth + old_mouse_x;
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += _screenWidth) {
		for (x = 0; x < old_mouse_w; ++x) {
			dst[x] = bak[x];
		}
	}
}

void OSystem_PALMOS::stop_cdrom() {
	return;
}

void OSystem_PALMOS::play_cdrom(int track, int num_loops, int start_frame, int end_frame) {
	return;
}

bool OSystem_PALMOS::poll_cdrom() {
	return false;
}

void OSystem_PALMOS::update_cdrom() {
	return;
}

OSystem_PALMOS::OSystem_PALMOS() {
	_quit = false;
	_current_shake_pos = 0;
	_new_shake_pos = 0;

	memset(&_mouseOldState,0,sizeof(MousePos));
	memset(&_mouseCurState,0,sizeof(MousePos));
	
	_paletteDirtyStart = 0;
	_paletteDirtyEnd = 0;
	
	_timer.active = false;
	_thread.active = false;
	_sound.active = false;
	
	_currentPalette = NULL;
	_mouseBackupP = NULL;

	lastKeyPressed = -1;
	lastKeyRepeat = 100;
	lastKeyModifier = MD_NONE;
	
	_useNumPad = false;
	
	// sound
	_isSndPlaying = false;
	_sndTempP = (UInt8 *)MemPtrNew(4096);
	_sndDataP = (UInt8 *)MemPtrNew(1024);
}

void OSystem_PALMOS::move_screen(int dx, int dy, int height) {

	if ((dx == 0) && (dy == 0))
		return;

	if (dx == 0) {
		// vertical movement
		if (dy > 0) {
			// move down
			// copy from bottom to top
			for (int y = height - 1; y >= dy; y--)
				copy_rect((byte *)_offScreenP + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
		} else {
			// move up
			// copy from top to bottom
			for (int y = 0; y < height + dx; y++)
				copy_rect((byte *)_offScreenP + _screenWidth * (y - dy), _screenWidth, 0, y, _screenWidth, 1);
		}
	} else if (dy == 0) {
		// horizontal movement
		if (dx > 0) {
			// move right
			// copy from right to left
			for (int x = _screenWidth - 1; x >= dx; x--)
				copy_rect((byte *)_offScreenP + x - dx, _screenWidth, x, 0, 1, height);
		} else {
			// move left
			// copy from left to right
			for (int x = 0; x < _screenWidth; x++)
				copy_rect((byte *)_offScreenP + x - dx, _screenWidth, x, 0, 1, height);
		}
	} else {
		// free movement
		// not neccessary for now
	}
}

bool OSystem_PALMOS::set_sound_proc(void *param, SoundProc *proc, byte format) {

	_sound.active = true;
	_sound.proc = proc;
	_sound.param = param;

	return true;
}

void OSystem_PALMOS::check_sound() {
	// currently not supported
	// but i need to use this function to prevent out of memory
	// on zak256 because the sound buffer growns and it's never
	// freed.
	_sound.proc(_sound.param, _sndTempP, 256);
}

void OSystem_PALMOS::show_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlay_visible = true;
	clear_overlay();
}

void OSystem_PALMOS::hide_overlay() {
	// hide the mouse
	undraw_mouse();

	_overlay_visible = false;
	_overlaySaved = false;
	memmove(_offScreenP, _tmpScreenP, _screenWidth * _screenHeight);
}

void OSystem_PALMOS::clear_overlay() {
	if (!_overlay_visible)
		return;
	
	// hide the mouse
	undraw_mouse();
	if (!_overlaySaved) {
		if (MemPtrDataStorage(_tmpScreenP))
			DmWrite(_tmpScreenP, 0, _offScreenP, _screenWidth * _screenHeight);
		else
			MemMove(_tmpScreenP, _offScreenP, _screenWidth * _screenHeight);
		_overlaySaved = true;
	}
}

void OSystem_PALMOS::grab_overlay(byte *buf, int pitch) {
	if (!_overlay_visible)
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
	if (!_overlay_visible)
		return;

	undraw_mouse();

	byte *dst = _offScreenP + y * _screenWidth + x;

	do {
		memcpy(dst, buf, w);
		dst += _screenWidth;
		buf += pitch;
	} while (--h);
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