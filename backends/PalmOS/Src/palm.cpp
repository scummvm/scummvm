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

#include "stdafx.h"
#include "scumm.h"
#include "mididrv.h"
#include "gameDetector.h"
#include "common/scaler.h"

#include "palm.h"
#include "starterrsc.h"
#include "pa1lib.h"
#include "sonychars.h"

#define SAVEDELAY	(5 * 60 * 1000) // five minutes
#define EXITDELAY	(100)		// delay to exit : calc button : double tap 1/500 sec

#define SCREEN_OVERLAY 0

static DmOpenRef gStgMemory = NULL;

static void MemStgInit() {
	if (!gStgMemory)
	{
		LocalID localID = DmFindDatabase(0, "Scumm-Memory");
		if (localID) DmDeleteDatabase(0, localID);
		
		if (DmCreateDatabase (0, "Scumm-Memory", 'ScVM', 'DATA', false) != errNone)
			return;

		localID = DmFindDatabase(0, "Scumm-Memory");
		gStgMemory = DmOpenDatabase(0, localID, dmModeReadWrite|dmModeExclusive);
	}
}

static void MemStgCleanup() {
	if (gStgMemory) {
		DmCloseDatabase(gStgMemory);
		LocalID localID = DmFindDatabase(0, "Scumm-Memory");
		if (localID)
			DmDeleteDatabase(0, localID);
	}
}

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
	MemStgInit();

	switch(_mode)
	{
		case GFX_FLIPPING:
			palm_offscreen	= WinScreenLock(winLockErase) + _screeny;
			palm_screen		= palm_offscreen;
			gVars->screenLocked = true;
			_renderer_proc = &update_screen__flipping;
			break;
		case GFX_DOUBLEBUFFER:
			h_palm_screen	= WinGetDisplayWindow();
			palm_screen		= (byte *)(BmpGetBits(WinGetBitmap(h_palm_screen))) + _screeny;
			h_palm_offscreen= WinCreateOffscreenWindow(SCREEN_WIDTH, SCREEN_HEIGHT, screenFormat, &e);
			palm_offscreen	= (byte *)(BmpGetBits(WinGetBitmap(h_palm_offscreen)));
			_renderer_proc = &update_screen__dbuffer;
			break;
		case GFX_NORMAL:
		default:
			h_palm_offscreen= WinGetDisplayWindow();
			palm_offscreen	= (byte *)(BmpGetBits(WinGetBitmap(h_palm_offscreen))) + _screeny;
			palm_screen		= palm_offscreen;
			_renderer_proc = &update_screen__direct;
			break;
	}

//	h_palm_tmpscreen = WinCreateOffscreenWindow(SCREEN_WIDTH, SCREEN_HEIGHT, screenFormat, &e);
//	palm_tmpscreen	= (byte *)(BmpGetBits(WinGetBitmap(h_palm_tmpscreen)));
	UInt16 index = SCREEN_OVERLAY;
	tmpScreenHandle = DmNewRecord(gStgMemory, &index, SCREEN_WIDTH * SCREEN_HEIGHT);
	tmpScreen = (byte *)MemHandleLock(tmpScreenHandle);
	_overlaySaved = false;

}

void OSystem_PALMOS::unload_gfx_mode() {
	switch (_mode)
	{
		case GFX_FLIPPING:
			WinScreenUnlock();
			break;
		case GFX_DOUBLEBUFFER:
			WinDeleteWindow(h_palm_offscreen,false);
			break;
	}

//	WinDeleteWindow(h_palm_tmpscreen,false);
	if (tmpScreenHandle)
		MemPtrUnlock(tmpScreen);
	MemStgCleanup();
}

void OSystem_PALMOS::init_size(uint w, uint h) {

	SCREEN_WIDTH = w;
	SCREEN_HEIGHT = h;

	_overlay_visible = false;
	_quit = false;

	_decaly = (320-h)/2;
	_screeny= _decaly * 320;

	set_mouse_pos(200,150);

	_currentPalette = (RGBColorType*)calloc(sizeof(RGBColorType), 256);
	_mouse_backup = (byte*)malloc(MAX_MOUSE_W * MAX_MOUSE_H);
	
	load_gfx_mode();
}

void OSystem_PALMOS::copy_rect(const byte *buf, int pitch, int x, int y, int w, int h) {

	byte *dst;

	/* FIXME: undraw mouse only if the draw rect intersects with the mouse rect */
	if (_mouse_drawn)
		undraw_mouse();

	dst = palm_offscreen + y * SCREEN_WIDTH + x;

	do {
		memcpy(dst, buf, w);
		dst += SCREEN_WIDTH;
		buf += pitch;
	} while (--h);
}

void OSystem_PALMOS::update_screen__flipping()
{
	RectangleType r;
	UInt8 *screen;
	UInt32 size = SCREEN_WIDTH*SCREEN_HEIGHT + 6400; // 10 pix top and bottom border
	Boolean shaked = false;
	UInt32 move = 0;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		if (gVars->HRrefNum) {
			RctSetRectangle(&r, 0, _decaly - _new_shake_pos, SCREEN_WIDTH, SCREEN_HEIGHT + (_new_shake_pos << 2));
			HRWinScrollRectangle(gVars->HRrefNum, &r, winDown, _new_shake_pos, NULL);
		} else {
			move = (_new_shake_pos * SCREEN_WIDTH);
			screen = palm_offscreen - 3200;
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
	palm_offscreen = WinScreenLock(winLockCopy) + _screeny;
	palm_screen = palm_offscreen;
	if (shaked) {
		if (gVars->HRrefNum) {
			HRWinScrollRectangle(gVars->HRrefNum, &r, winUp, _new_shake_pos, NULL);
		} else {
			screen = palm_offscreen - 3200;
			MemMove(screen, screen + move, size);
		}
	}

}

void OSystem_PALMOS::update_screen__dbuffer()
{
	UInt32 move = 0;
	UInt32 size = SCREEN_WIDTH*SCREEN_HEIGHT;

	// shake screen
	if (_current_shake_pos != _new_shake_pos) {
		move = (_new_shake_pos * SCREEN_WIDTH);
		// copy clear bottom of the screen to top to cover shaking image
		MemMove(palm_screen, palm_screen + size , move);

		if (_vibrate) {
			Boolean active = (_new_shake_pos >= 3);
			HwrVibrateAttributes(1, kHwrVibrateActive, &active);
		}

		_current_shake_pos = _new_shake_pos;
	}
	// update screen
	MemMove(palm_screen + move, palm_offscreen, size - move);
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

void OSystem_PALMOS::update_screen() {
	if(_quit)
		return;

	// Make sure the mouse is drawn, if it should be drawn.
	draw_mouse();

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly. 
	if (_paletteDirtyEnd != 0) {
			if (gVars->stdPalette) {
				WinSetDrawWindow(WinGetDisplayWindow());	// hack by Doug
				WinPalette(winPaletteSet, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart,_currentPalette + _paletteDirtyStart);
			} else {
				HwrDisplayPalette(winPaletteSet, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart,_currentPalette + _paletteDirtyStart);
			}

		_paletteDirtyEnd = 0;

//		_msg.color = RGBToColor(255,255,255);
		gVars->indicator.on = RGBToColor(0,255,0);
//		gVars->indicator.off= 0; //RGBToColor(0,0,0);
		if (lastKeyModifier)
			drawKeyState();
	}

	((this)->*(_renderer_proc))();
}

bool OSystem_PALMOS::show_mouse(bool visible) {
	if (_mouse_visible == visible)
		return visible;
	
	bool last = _mouse_visible;
	_mouse_visible = visible;

	if (visible)
		draw_mouse();
	else
		undraw_mouse();

	return last;
}

void OSystem_PALMOS::warp_mouse(int x, int y) {
}

void OSystem_PALMOS::set_mouse_pos(int x, int y) {
	if (x != _mouse_cur_state.x || y != _mouse_cur_state.y) {
		_mouse_cur_state.x = x;
		_mouse_cur_state.y = y;
		undraw_mouse();
	}
}

void OSystem_PALMOS::set_mouse_cursor(const byte *buf, uint w, uint h, int hotspot_x, int hotspot_y) {
	_mouse_cur_state.w = w;
	_mouse_cur_state.h = h;

	_mouse_hotspot_x = hotspot_x;
	_mouse_hotspot_y = hotspot_y;

	_mouse_data = (byte*)buf;

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
	Int16 x = _mouse_cur_state.x;
	Int16 y = _mouse_cur_state.y;

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
	x = (x >= SCREEN_WIDTH	) ? SCREEN_WIDTH-1	: x;
	y = (y < 0				) ? 0				: y;
	y = (y >= SCREEN_HEIGHT	) ? SCREEN_HEIGHT-1	: y;


	event->event_code = EVENT_MOUSEMOVE;
	event->mouse.x = x;
	event->mouse.y = y;
}

#define MD_NONE 0
#define MD_CTRL 1
#define MD_ALT	2

void OSystem_PALMOS::drawKeyState() {
	UInt8 i,j;
	UInt16 bmpID = 3000 + lastKeyModifier - 1;
	
	MemHandle hTemp;
	BitmapType *bmTemp;
	UInt32 *bmData;
	UInt8 *scr = palm_screen + SCREEN_WIDTH * (SCREEN_HEIGHT + 2) + 2;

	hTemp	= DmGetResource(bitmapRsc,bmpID);
	
	if (hTemp) {
		bmTemp	= (BitmapType *)MemHandleLock(hTemp);
		bmData	= (UInt32 *)BmpGetBits(bmTemp);

		for (i = 0; i < 7; i++) {
			for (j = 0; j < 32; j++) {
				if (*bmData & (1 << (31-j)))
					*scr++ = gVars->indicator.on;
				else
					*scr++ = gVars->indicator.off;
			}
			scr += SCREEN_WIDTH - 32;
			bmData++;
		}

		MemPtrUnlock(bmTemp);
		DmReleaseResource(hTemp);
	} else {
		for (i = 0; i < 7; i++) {
			for (j = 0; j < 32; j++) {
				*scr++ = gVars->indicator.off;
			}
			scr += SCREEN_WIDTH - 32;
		}
	}
}

bool OSystem_PALMOS::poll_event(Event *event) {
	EventType ev;
	Boolean handled;
//	UInt32 button = 0;
	uint32 current_msecs;
	UInt32 keyCurrentState = 0;
	Boolean funcButton = false;

	/* First, handle timers */
	for(;;) {
		EvtGetEvent(&ev, 0);
		
		keyCurrentState = KeyCurrentState();
		current_msecs = get_msecs();
	
		//thread handler
		if (_thread.active)
			_thread.proc(_thread.param);

		// sound handler
		if(_sound.active)
			check_sound();
		
//		if (_msg.state != 0)
//			drawMessage();

		// timer handler
		if (_timer.active && (current_msecs >= _timer.next_expiry)) {
			_timer.duration = _timer.callback(_timer.duration);
			_timer.next_expiry = current_msecs + _timer.duration;
		}

		if (ev.eType == keyDownEvent) {
			switch (ev.data.keyDown.chr) {
				case vchrLaunch:
					lastKeyPressed = -1;
					funcButton = true;
					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = 27;
					event->kbd.ascii = 27;
					event->kbd.flags = 0;
					return true;
				
				case vchrMenu:
					lastKeyPressed = -1;
					funcButton = true;
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
					funcButton = true;
					lastKeyPressed = vchrCalc;
					return true;

				// mouse emulation
				case vchrHard1: // left button
					event->event_code = EVENT_LBUTTONDOWN;
					event->mouse.x = _mouse_cur_state.x;
					event->mouse.y = _mouse_cur_state.y;
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
					event->mouse.x = _mouse_cur_state.x;
					event->mouse.y = _mouse_cur_state.y;
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
		if (!funcButton && !handled)
			if (SysHandleEvent(&ev))
				continue;

		// others events
		switch(ev.eType) {

		case keyDownEvent: {
				lastEvent = keyDownEvent;
				lastKeyPressed = -1;
				//if (ev.data.keyDown.modifiers & shiftKeyMask) b |= KBD_SHIFT;

				if (ev.data.keyDown.chr == 262 && (ev.data.keyDown.modifiers & commandKeyMask)) {
					lastKeyModifier++;
					lastKeyModifier %= 3;
					drawKeyState();				
				} else {
					byte b = 0;
					if (lastKeyModifier == MD_CTRL)	b = KBD_CTRL;
					if (lastKeyModifier == MD_ALT)	b = KBD_ALT;

					event->event_code = EVENT_KEYDOWN;
					event->kbd.keycode = ev.data.keyDown.chr;
					event->kbd.ascii = (ev.data.keyDown.chr>='a' && ev.data.keyDown.chr<='z' && (event->kbd.flags & KBD_SHIFT)?ev.data.keyDown.chr &~ 0x20 : ev.data.keyDown.chr);
					event->kbd.flags = b;
					lastKeyModifier = MD_NONE;
					drawKeyState();				
				}
				return true;
			}

		case penMoveEvent:
			if (ev.screenY*2-_decaly > SCREEN_HEIGHT || ev.screenY*2-_decaly < 0)
				return true;

			if (lastEvent != penMoveEvent && (abs(ev.screenY*2-event->mouse.y) <= 2 || abs(ev.screenX*2-event->mouse.x) <= 2)) // move only if
				return true;

			lastEvent = penMoveEvent;
			event->event_code = EVENT_MOUSEMOVE;
			event->mouse.x = ev.screenX*2;
			event->mouse.y = ev.screenY*2 - _decaly;
			return true;

		case penDownEvent:
			lastEvent = penDownEvent;

			if (ev.screenY*2-_decaly > SCREEN_HEIGHT || ev.screenY*2-_decaly < 0)
				return true;

			event->event_code = EVENT_LBUTTONDOWN;
			event->mouse.x = ev.screenX*2;
			event->mouse.y = ev.screenY*2 - _decaly;
			set_mouse_pos(event->mouse.x, event->mouse.y);
			return true;

		case penUpEvent:
			event->event_code = EVENT_LBUTTONUP;

			if (ev.screenY*2-_decaly > SCREEN_HEIGHT || ev.screenY*2-_decaly < 0)
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
		
		UInt16 w1 = FntCharsWidth(value->caption,StrLen(value->caption));

		if (gVars->HRrefNum != sysInvalidRefNum) {
			Char *caption = "Loading...\0";
			UInt16 h0 = FntLineHeight() + 2;
			UInt16 w1;

			WinSetTextColor(255);
			HRFntSetFont(gVars->HRrefNum,hrTinyBoldFont);
			w1 = FntCharsWidth(caption,StrLen(caption)) * 1;
			w1 = (320 - w1) / 2;
			HRWinDrawChars(gVars->HRrefNum,caption,StrLen(caption),w1,80);

			HRFntSetFont(gVars->HRrefNum,hrTinyFont);
			w1 = FntCharsWidth(value->caption,StrLen(value->caption)) * 1;
			w1 = (320 - w1) / 2;
			HRWinDrawChars(gVars->HRrefNum,value->caption,StrLen(value->caption),w1,80 + h0);
		} else {
			Char *caption = "Loading...\0";
			UInt16 w1;

			WinSetTextColor(255);
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
	if (_mouse_backup)
		free(_mouse_backup);
	if (_sndData)
		MemPtrFree(_sndData);

	unload_gfx_mode();

	_quit = true;
	_currentPalette = NULL;
	_mouse_backup = NULL;
}

void OSystem_PALMOS::draw_mouse() {
	if (_mouse_drawn || !_mouse_visible || _quit)
		return;

	_mouse_cur_state.y = _mouse_cur_state.y>=SCREEN_HEIGHT ? SCREEN_HEIGHT-1 : _mouse_cur_state.y;

	int x = _mouse_cur_state.x - _mouse_hotspot_x;
	int y = _mouse_cur_state.y - _mouse_hotspot_y;
	int w = _mouse_cur_state.w;
	int h = _mouse_cur_state.h;
	byte color;
	byte *src = _mouse_data;		// Image representing the mouse
	byte *bak = _mouse_backup;		// Surface used to backup the area obscured by the mouse
	byte *dst;					// Surface we are drawing into


	// clip the mouse rect, and addjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouse_cur_state.w;
		y = 0;
	}
	if (w > SCREEN_WIDTH - x)
		w = SCREEN_WIDTH - x;
	if (h > SCREEN_HEIGHT - y)
		h = SCREEN_HEIGHT - y;

	// Store the bounding box so that undraw mouse can restore the area the
	// mouse currently covers to its original content.
	_mouse_old_state.x = x;
	_mouse_old_state.y = y;
	_mouse_old_state.w = w;
	_mouse_old_state.h = h;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Draw the mouse cursor; backup the covered area in "bak"
	dst = palm_offscreen + y * SCREEN_WIDTH + x;
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
		src += _mouse_cur_state.w - w;
		bak += MAX_MOUSE_W - w;
		dst += SCREEN_WIDTH - w;
		h--;
	}

	// Finally, set the flag to indicate the mouse has been drawn
	_mouse_drawn = true;
}

void OSystem_PALMOS::undraw_mouse() {
	if (!_mouse_drawn || _quit)
		return;

	_mouse_drawn = false;

	byte *dst, *bak = _mouse_backup;
	const int old_mouse_x = _mouse_old_state.x;
	const int old_mouse_y = _mouse_old_state.y;
	const int old_mouse_w = _mouse_old_state.w;
	const int old_mouse_h = _mouse_old_state.h;
	int x,y;

	// No need to do clipping here, since draw_mouse() did that already

	dst = palm_offscreen + old_mouse_y * SCREEN_WIDTH + old_mouse_x;
	for (y = 0; y < old_mouse_h; ++y, bak += MAX_MOUSE_W, dst += SCREEN_WIDTH) {
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

	memset(&_mouse_old_state,0,sizeof(MousePos));
	memset(&_mouse_cur_state,0,sizeof(MousePos));
	
	_msg.state = 0;

	_paletteDirtyStart = 0;
	_paletteDirtyEnd = 0;
	
	_timer.active = false;
	_thread.active = false;
	_sound.active = false;
	
	_currentPalette = NULL;
	_mouse_backup = NULL;

	lastKeyPressed = -1;
	lastKeyRepeat = 100;
	lastKeyModifier = MD_NONE;
	
	_isPlaying = false;

	_sndData = (UInt8 *)MemPtrNew(512);
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
				copy_rect((byte *)palm_offscreen + SCREEN_WIDTH * (y - dy), SCREEN_WIDTH, 0, y, SCREEN_WIDTH, 1);
		} else {
			// move up
			// copy from top to bottom
			for (int y = 0; y < height + dx; y++)
				copy_rect((byte *)palm_offscreen + SCREEN_WIDTH * (y - dy), SCREEN_WIDTH, 0, y, SCREEN_WIDTH, 1);
		}
	} else if (dy == 0) {
		// horizontal movement
		if (dx > 0) {
			// move right
			// copy from right to left
			for (int x = SCREEN_WIDTH - 1; x >= dx; x--)
				copy_rect((byte *)palm_offscreen + x - dx, SCREEN_WIDTH, x, 0, 1, height);
		} else {
			// move left
			// copy from left to right
			for (int x = 0; x < SCREEN_WIDTH; x++)
				copy_rect((byte *)palm_offscreen + x - dx, SCREEN_WIDTH, x, 0, 1, height);
		}
	} else {
		// free movement
		// not neccessary for now
	}
}

void OSystem_PALMOS::drawMessage() {
	UInt32 msecs = get_msecs();
	
	if ((msecs - _msg.time) >= _msg.wait) {
		Int16 y = _msg.position * _msg.state + (_msg.state == -1 ? 320 : 308);
		 _msg.time = msecs;

		WinSetDrawMode(winPaint);
		WinSetBackColor(0);
		WinSetTextColor(_msg.color);
		HRFntSetFont(gVars->HRrefNum,hrTinyFont);
		HRWinDrawChars(gVars->HRrefNum, _msg.text, StrLen(_msg.text), 2, y);
		
		_msg.position += 2;
		if (_msg.position > 12) {
			_msg.position = 0;
			_msg.state *= -1;
			_msg.wait = 5000;
			_msg.state = (_msg.state == -1 ? 0 : _msg.state);
		} else {
			_msg.wait = 100;
		}
	}
}

void OSystem_PALMOS::deleteMessage() {
	if (_msg.state != 0) {
		Int16 y = _msg.position * _msg.state + (_msg.state == -1 ? 320 : 308);
		WinSetDrawMode(winPaint);
		WinSetBackColor(0);
		WinSetTextColor(0);
		HRFntSetFont(gVars->HRrefNum,hrTinyFont);
		HRWinDrawChars(gVars->HRrefNum, _msg.text, StrLen(_msg.text), 2, y);
	}
}

void OSystem_PALMOS::addMessage(const Char *msg) {

	if (_msg.state != 0)
		deleteMessage();

	_msg.state = -1;
	_msg.position = 0;
	StrCopy(_msg.text,msg);
	_msg.time = get_msecs();
	_msg.wait = 100;
	_msg.color = RGBToColor(255,255,255);
}

bool OSystem_PALMOS::set_sound_proc(void *param, SoundProc *proc, byte format) {

	_sound.active = true;
	_sound.proc = proc;
	_sound.param = param;

	return true;
}

void OSystem_PALMOS::check_sound() {
	// currently not supported
	_sound.proc(_sound.param, _sndData, 512);
}

void OSystem_PALMOS::show_overlay()
{
	// hide the mouse
	undraw_mouse();

	_overlay_visible = true;
	clear_overlay();
}

void OSystem_PALMOS::hide_overlay()
{
	// hide the mouse
	undraw_mouse();

	_overlay_visible = false;
	_overlaySaved = false;
	memmove(palm_offscreen, tmpScreen, SCREEN_WIDTH*SCREEN_HEIGHT);
}

void OSystem_PALMOS::clear_overlay()
{
	if (!_overlay_visible)
		return;
	
	// hide the mouse
	undraw_mouse();
	if (!_overlaySaved)
	{	//memmove(palm_tmpscreen, palm_offscreen, SCREEN_WIDTH*SCREEN_HEIGHT);
		DmWrite(tmpScreen, 0, palm_offscreen, SCREEN_WIDTH*SCREEN_HEIGHT);
		_overlaySaved = true;
	}
}

void OSystem_PALMOS::grab_overlay(byte *buf, int pitch)
{
	if (!_overlay_visible)
		return;

	// hide the mouse
	undraw_mouse();

	byte *src = tmpScreen;
	int h = SCREEN_HEIGHT;

	do {
		memcpy(buf, src, SCREEN_WIDTH);
		src += SCREEN_WIDTH;
		buf += pitch;
	} while (--h);
}

void OSystem_PALMOS::copy_rect_overlay(const byte *buf, int pitch, int x, int y, int w, int h)
{
	if (!_overlay_visible)
		return;

	undraw_mouse();

	byte *dst = palm_offscreen + y * SCREEN_WIDTH + x;

	do {
		memcpy(dst, buf, w);
		dst += SCREEN_WIDTH;
		buf += pitch;
	} while (--h);
}


int16 OSystem_PALMOS::get_height() {
	return SCREEN_HEIGHT;
}

int16 OSystem_PALMOS::get_width() {
	return SCREEN_WIDTH;
}

byte OSystem_PALMOS::RGBToColor(uint8 r, uint8 g, uint8 b)
{
	NewGuiColor color = 255;
	byte nearest = 255;
	byte check;
	byte r2,g2,b2;

	for (int i=0; i<256; i++)
	{
		r2 = _currentPalette[i].r;
		g2 = _currentPalette[i].g;
		b2 = _currentPalette[i].b;

		check = (ABS(r2 - r) + ABS(g2 - g) + ABS(b2 - b))/3;

		if (check == 0)				// perfect match
			return i;
		else if (check<nearest)		// else save and continue
		{	
			color = i;
			nearest = check;
		}
	}

	return color;
}

void OSystem_PALMOS::ColorToRGB(byte color, uint8 &r, uint8 &g, uint8 &b)
{
	r = _currentPalette[color].r;
	g = _currentPalette[color].g;
	b = _currentPalette[color].b;
}