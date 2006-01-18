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

#include "be_base.h"

void OSystem_PalmBase::timer_handler() {
	UInt32 msecs = getMillis();

	if (_timer.active && (msecs >= _timer.nextExpiry)) {
		_timer.duration = _timer.callback(_timer.duration);
		_timer.nextExpiry = msecs + _timer.duration;
	}
}

void OSystem_PalmBase::battery_handler() {
	// check battery level every 15secs
	if ((TimGetTicks() - _batCheckLast) > _batCheckTicks) {
		UInt16 voltage, warnThreshold, criticalThreshold;
		Boolean pluggedIn;
		voltage = SysBatteryInfoV20(false, &warnThreshold, &criticalThreshold, NULL, NULL, &pluggedIn);

		if (!pluggedIn) {
			if (voltage <= warnThreshold) {
				if (!_showBatLow) {
					_showBatLow = true;
					draw_osd(kDrawBatLow, _screenDest.w - 18, -16, true, 2);
					displayMessageOnOSD("Battery low.");
				}
			} else {
				if (_showBatLow) {
					_showBatLow = false;
					draw_osd(kDrawBatLow, _screenDest.w - 18, -16, false);
				}
			}

			if (voltage <= criticalThreshold) {
				::EventType event;
				event.eType = keyDownEvent;
				event.data.keyDown.chr = vchrPowerOff;
				event.data.keyDown.modifiers = commandKeyMask;
				EvtAddEventToQueue(&event);
			}
		}

		_batCheckLast = TimGetTicks();
	}
}

bool OSystem_PalmBase::pollEvent(Event &event) {
	::EventType ev;
	Boolean handled;
	UInt32 keyCurrentState;
	Coord x, y;

	battery_handler();
	timer_handler();
	sound_handler();
		
	for(;;) {
#if defined(COMPILE_OS5) && defined(PALMOS_ARM)
		SysEventGet(&ev, evtNoWait);
#else
		EvtGetEvent(&ev, evtNoWait);
#endif

		// check for hardkey repeat for mouse emulation
		keyCurrentState = KeyCurrentState();
		// check_hard_keys();

		if ((keyCurrentState & _keyMouseMask)) {
			Int8 sx = 0;
			Int8 sy = 0;

			if (keyCurrentState & _keyMouse.bitUp)
				sy = -1;
			else if (keyCurrentState & _keyMouse.bitDown)
				sy = +1;
				
			if (keyCurrentState & _keyMouse.bitLeft)
				sx = -1;
			else if (keyCurrentState & _keyMouse.bitRight)
				sx = +1;					

			simulate_mouse(event, sx, sy, &x, &y);
			warpMouse(x, y);
			updateScreen();
	//		updateCD();
			event.type = EVENT_MOUSEMOVE;
			event.mouse.x = x;
			event.mouse.y = y;

			_lastKey = kKeyMouseMove;
			return true;
		}

		if (ev.eType == keyDownEvent) {
			switch (ev.data.keyDown.chr) {
			// ESC key
			case vchrLaunch:
				_lastKey = kKeyNone;
				event.type = EVENT_KEYDOWN;
				event.kbd.keycode = 27;
				event.kbd.ascii = 27;
				event.kbd.flags = 0;
				return true;

			// F5 = menu
			case vchrMenu:
				_lastKey = kKeyNone;
				event.type = EVENT_KEYDOWN;
				event.kbd.keycode = 319;
				event.kbd.ascii = 319;
				event.kbd.flags = 0;
				return true;

			// if hotsync pressed, etc...
			case vchrHardCradle:
			case vchrHardCradle2:
			case vchrLowBattery:
			case vchrFind:
//			case vchrBrightness:	// volume control on Zodiac, let other backends disable it
			case vchrContrast:
				// do nothing
				_lastKey = kKeyNone;
				return true;
			}
		}

		if (check_event(event, &ev))
			return true;
		_lastKey = kKeyNone;

		// prevent crash when alarm is raised
		handled = ((ev.eType == keyDownEvent) && 
						(ev.data.keyDown.modifiers & commandKeyMask) && 
						((ev.data.keyDown.chr == vchrAttnStateChanged) || 
						(ev.data.keyDown.chr == vchrAttnUnsnooze))); 

		// graffiti strokes, auto-off, etc...
		if (!handled)
			if (SysHandleEvent(&ev))
				continue;

		switch(ev.eType) {
		case penMoveEvent:
			get_coordinates(&ev, x, y);

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return false;

			if (_lastEvent != penMoveEvent && (abs(y - event.mouse.y) <= 2 || abs(x - event.mouse.x) <= 2)) // move only if
				return false;

			_lastEvent = penMoveEvent;
			event.type = EVENT_MOUSEMOVE;
			event.mouse.x = x;
			event.mouse.y = y;
			warpMouse(x, y);
			return true;

		case penDownEvent:
			get_coordinates(&ev, x, y);

			// indy fight mode
			if (_useNumPad && !_overlayVisible) {
				char num = '1';
				num += 9 -
						(3 - (3 * x / _screenWidth )) -
						(3 * (3 * y / _screenHeight));
			
				event.type = EVENT_KEYDOWN;
				event.kbd.keycode = num;
				event.kbd.ascii = num;
				event.kbd.flags = 0;

				_lastEvent = keyDownEvent;
				return true;
			}

			_lastEvent = penDownEvent;				
			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return false;

			event.type = ((gVars->stylusClick || _overlayVisible) ? EVENT_LBUTTONDOWN : EVENT_MOUSEMOVE);
			event.mouse.x = x;
			event.mouse.y = y;
			warpMouse(x, y);
			return true;

		case penUpEvent:
			get_coordinates(&ev, x, y);

			event.type = ((gVars->stylusClick || _overlayVisible) ? EVENT_LBUTTONUP : EVENT_MOUSEMOVE);
			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return false;

			event.mouse.x = x;
			event.mouse.y = y;
			warpMouse(x, y);
			return true;
		
		case keyDownEvent:
			if (ev.data.keyDown.chr == vchrCommand &&
				(ev.data.keyDown.modifiers & commandKeyMask)) {

				_lastKeyModifier++;
				_lastKeyModifier %= kModifierCount;

				if (_lastKeyModifier)
					draw_osd((kDrawKeyState + _lastKeyModifier - 1), 2, _screenDest.h + 2, true);
				else
					draw_osd(kDrawKeyState, 2, _screenDest.h + 2, false);

				return false;
			}

			char mask = 0;
			UInt16 key = ev.data.keyDown.chr;

			if (_lastKeyModifier == kModifierNone) {
				// for keyboard mode
				if (ev.data.keyDown.modifiers & shiftKeyMask)	mask |= KBD_SHIFT;
				if (ev.data.keyDown.modifiers & controlKeyMask)	mask |= KBD_CTRL;
				if (ev.data.keyDown.modifiers & optionKeyMask)	mask |= KBD_ALT;
				if (ev.data.keyDown.modifiers & commandKeyMask) mask |= KBD_CTRL|KBD_ALT;
			} else {
				// for grafiti mode
				if (_lastKeyModifier == kModifierCommand)	mask = KBD_CTRL|KBD_ALT;
				if (_lastKeyModifier == kModifierAlt)		mask = KBD_ALT;
				if (_lastKeyModifier == kModifierCtrl)		mask = KBD_CTRL;
			}

			if (_lastKeyModifier)
				draw_osd(kDrawKeyState, 2, _screenDest.h + 2, false);
			_lastKeyModifier = kModifierNone;

			// F1 -> F10 key
			if  (key >= '0' && key <= '9' && mask == (KBD_CTRL|KBD_ALT)) {
				key = (key == '0') ? 324 : (315 + key - '1');
				mask = 0;

			// exit
			} else if  ((key == 'z' && mask == KBD_CTRL) || (mask == KBD_ALT && key == 'x')) {
				event.type = EVENT_QUIT;
				return true;
			
			// num pad (indy fight mode)
			} else if (key == 'n' && mask == KBD_CTRL|KBD_ALT && !_overlayVisible) {
				_useNumPad = !_useNumPad;
				draw_osd(kDrawFight, _screenDest.w - 34, _screenDest.h + 2, _useNumPad, 1);
				displayMessageOnOSD(_useNumPad ? "Fight mode on." : "Fight mode off.");
				return false;
			}
			
			// other keys
			event.type = EVENT_KEYDOWN;
			event.kbd.keycode = key;
			event.kbd.ascii = key;
			event.kbd.flags = mask;
			return true;

		default:
			return false;
		};
	}
}
