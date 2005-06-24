/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "palm.h"
#include "common/scaler.h"

#include <PalmNavigator.h>

#ifndef DISABLE_TAPWAVE
#include <TwChars.h>
#endif

#define EXITDELAY (500) // delay to exit : calc button : double tap 1/500 sec

void OSystem_PALMOS::SimulateArrowKeys(Event &event, Int8 iHoriz, Int8 iVert) {
	Int16 x = _mouseCurState.x;
	Int16 y = _mouseCurState.y;
	Int16 slow;

	if (_lastKeyPressed != kLastKeyNone) {
		_lastKeyRepeat++;

		if (_lastKeyRepeat > 16)
			_lastKeyRepeat = 16;
	}
	else
		_lastKeyRepeat = 0;

	slow = (iHoriz && iVert) ? 2 : 1;

	x += iHoriz * (_lastKeyRepeat >> 1) / slow;
	y += iVert * (_lastKeyRepeat >> 1) / slow;

	x = (x < 0				) ? 0					: x;
	x = (x >= _screenWidth	) ? _screenWidth - 1	: x;
	y = (y < 0				) ? 0					: y;
	y = (y >= _screenHeight	) ? _screenHeight - 1	: y;

	event.type = EVENT_MOUSEMOVE;
	event.mouse.x = x;
	event.mouse.y = y;
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

bool OSystem_PALMOS::pollEvent(Event &event) {
	::EventType ev;
	Boolean handled;
	UInt32 keyCurrentState;
	Coord x, y;

	battery_handler();
	sound_handler();
	timer_handler(getMillis());

	for(;;) {
		EvtGetEvent(&ev, evtNoWait);

		// check for hardkey repeat for mouse emulation
		keyCurrentState = KeyCurrentState();

		// arrow keys emulation
		if ((keyCurrentState & _keyMask)) {
			Int8 sx = 0;
			Int8 sy = 0;

			if (keyCurrentState & _keyMouse.bitButLeft) {
				event.type = EVENT_LBUTTONDOWN;
				event.mouse.x = _mouseCurState.x;
				event.mouse.y = _mouseCurState.y;
				_lastKeyPressed = kLastKeyNone;
				return true;
			}

			if (keyCurrentState & _keyMouse.bitUp)
				sy = -1;
			else if (keyCurrentState & _keyMouse.bitDown)
				sy = +1;
				
			if (keyCurrentState & _keyMouse.bitLeft)
				sx = -1;
			else if (keyCurrentState & _keyMouse.bitRight)
				sx = +1;					
			
			SimulateArrowKeys(event, sx, sy);
			updateScreen();
			updateCD();
			_lastKeyPressed = kLastKeyMouse;
			return true;

		} else if (_lastKeyPressed != kLastKeyCalc) {
			_lastKeyPressed = kLastKeyNone;
		}

		if (ev.eType == nilEvent) {
			// force CD update, useful when the game is paused in some cases
			updateCD();
			return false;
		}

		if (ev.eType == keyDownEvent) {
			switch (ev.data.keyDown.chr) {
			
				// ESC key
				case vchrLaunch:
					_lastKeyPressed = kLastKeyNone;
					event.type = EVENT_KEYDOWN;
					event.kbd.keycode = 27;
					event.kbd.ascii = 27;
					event.kbd.flags = 0;
					return true;
				
				// F5 = menu
				case vchrJogPushRepeat:
				case vchrMenu:
				case vchrThumbWheelBack:	// Tapwave back button
					_lastKeyPressed = kLastKeyNone;
					event.type = EVENT_KEYDOWN;
					event.kbd.keycode = 319;
					event.kbd.ascii = 319;
					event.kbd.flags = 0;
					return true;

				case vchrCalc:
					if (_lastKeyPressed & kLastKeyCalc)
						if ((getMillis() - _exit_delay) <= (EXITDELAY))
							event.type = EVENT_QUIT;

					_exit_delay = getMillis();
					_lastKeyPressed = kLastKeyCalc;
					return true;

				// mouse button
				case vchrJogBack:
				case vchrHard4: // right button
#ifndef DISABLE_TAPWAVE
				case vchrActionRight:
#endif
					event.type = EVENT_RBUTTONDOWN;
					event.mouse.x = _mouseCurState.x;
					event.mouse.y = _mouseCurState.y;
					_lastKeyPressed = kLastKeyNone;
					return true;

				case vchrJogPushedUp:
				case vchrJogPushedDown: // hot swap gfx mode 
					if (_initMode == GFX_WIDE)
						hotswap_gfx_mode(_mode == GFX_WIDE ? GFX_NORMAL: GFX_WIDE);
					else
						setFeatureState(kFeatureFullscreenMode, !_fullscreen);
					return true;

				// wheel
				case vchrJogUp:
					event.type = EVENT_WHEELUP;
					return true;

				case vchrJogDown:
					event.type = EVENT_WHEELDOWN;
					return true;


				// if hotsync pressed, etc...
				case vchrHardCradle:
				case vchrHardCradle2:
				case vchrLowBattery:
				case vchrFind:
				case vchrBrightness:
				case vchrContrast:
					// do nothing
					return true;

				// trun off
				case vchrAutoOff:
				case vchrPowerOff:
					// pause the sound thread if any
					if (OPTIONS_TST(kOptPalmSoundAPI) && _sound.active)
						SndStreamPause(*((SndStreamRef *)_sound.handle), true);
					break;
				
				case vchrLateWakeup:
					// resume the sound thread if any
					if (OPTIONS_TST(kOptPalmSoundAPI) && _sound.active)
						SndStreamPause(*((SndStreamRef *)_sound.handle), false);
					break;
			}

			if (OPTIONS_TST(kOpt5WayNavigator)) {
				// mouse emulation for device with 5-Way navigator
				switch (ev.data.keyDown.chr) {
					// hot swap gfx
					case vchrHard1:
						if (_initMode == GFX_WIDE)
							hotswap_gfx_mode(_mode == GFX_WIDE ? GFX_NORMAL: GFX_WIDE);
						else
							setFeatureState(kFeatureFullscreenMode, !_fullscreen);
						return true;

					// ESC key
					case vchrHard2:
						_lastKeyPressed = kLastKeyNone;
						event.type = EVENT_KEYDOWN;
						event.kbd.keycode = 27;
						event.kbd.ascii = 27;
						event.kbd.flags = 0;
						return true;
					
					// F5 = menu
					case vchrHard3:
						_lastKeyPressed = kLastKeyNone;
						event.type = EVENT_KEYDOWN;
						event.kbd.keycode = 319;
						event.kbd.ascii = 319;
						event.kbd.flags = 0;
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
						draw1BitGfx((kDrawKeyState + _lastKeyModifier - 1), 2, _screenHeight + 2, true);
					else
						draw1BitGfx(kDrawKeyState, 2, _screenHeight + 2, false);

				} else {
					int keycode;
					byte b = 0;

					if (_lastKeyModifier == MD_CMD)  b = KBD_CTRL|KBD_ALT;
					if (_lastKeyModifier == MD_ALT)  b = KBD_ALT;
					if (_lastKeyModifier == MD_CTRL) b = KBD_CTRL;
					
					keycode = ev.data.keyDown.chr;
					
					// F1 -> F10 key
					if  (keycode >= '0' && keycode <= '9' && b == (KBD_CTRL|KBD_ALT)) {
						keycode = keycode == '0' ? 324 : (315 + keycode - '1');
						b = 0;
					
					} else if  ((keycode == 'z' && b == KBD_CTRL) || (b == KBD_ALT && keycode == 'x')) {
						event.type = EVENT_QUIT;
						return true;

					} else if (keycode == 'n' && b == KBD_CTRL) {
						UInt8 *scr = _screenP + _screenWidth * (_screenHeight + 2);
						_useNumPad = !_useNumPad;
						draw1BitGfx(kDrawNumPad, (_screenWidth >> 1) - 32, _screenHeight + 2, _useNumPad);
						return true;

#ifndef DISABLE_TAPWAVE
					// Zodiac only keys
					} else if (keycode == vchrTriggerRight) {
						setFeatureState(kFeatureAspectRatioCorrection, 0);
						return true;

					} else if (keycode == vchrTriggerLeft && _screenWidth != 640) {
						if (_initMode == GFX_WIDE)
							hotswap_gfx_mode(_mode == GFX_WIDE ? GFX_NORMAL: GFX_WIDE);
						else
							setFeatureState(kFeatureFullscreenMode, !_fullscreen);
#endif
					}
					
					event.type = EVENT_KEYDOWN;
					event.kbd.keycode = keycode;
					event.kbd.ascii = keycode; //(keycode>='a' && keycode<='z' && (event.kbd.flags & KBD_SHIFT) ? keycode &~ 0x20 : keycode);
					event.kbd.flags = b;
					
					if (_lastKeyModifier) {
						_lastKeyModifier = MD_NONE;
						draw1BitGfx(kDrawKeyState, 2, getHeight() + 2, false);
					}
				}
				return true;
			}

		case penMoveEvent:
			getCoordinates(&ev, &x, &y);

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return true;

			if (_lastEvent != penMoveEvent && (abs(y - event.mouse.y) <= 2 || abs(x - event.mouse.x) <= 2)) // move only if
				return true;

			_lastEvent = penMoveEvent;
			event.type = EVENT_MOUSEMOVE;
			event.mouse.x = x;
			event.mouse.y = y;
			set_mouse_pos(event.mouse.x, event.mouse.y);
			return true;

		case penDownEvent:
			getCoordinates(&ev, &x, &y);

			if (_useNumPad) {
				Coord x2 = (_screenWidth >> 1) - 20;
				Coord y2 = _screenHeight + 2;

				if (y >= y2 && y < (y2 + 34) && x >= x2 && x < (x2 + 40)) {	// numpad location
					UInt8 key = '1';
					key += 9 - ( (3 - ((x - x2) / 13)) + (3 * ((y - y2) / 11)) );
					
					_lastEvent = keyDownEvent;
					_lastKeyPressed = kLastKeyNone;
					
					event.type = EVENT_KEYDOWN;
					event.kbd.keycode = key;
					event.kbd.ascii = key;
					event.kbd.flags = 0;
					return true;
				}
			}

			_lastEvent = penDownEvent;
			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return true;

			event.type = EVENT_LBUTTONDOWN;
			event.mouse.x = x;
			event.mouse.y = y;
			set_mouse_pos(event.mouse.x, event.mouse.y);
			return true;

		case penUpEvent:
			getCoordinates(&ev, &x, &y);
			event.type = EVENT_LBUTTONUP;

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return true;

			event.mouse.x = x;
			event.mouse.y = y;
			set_mouse_pos(event.mouse.x, event.mouse.y);
			return true;

		default:
			return false;
		}
	}
}
