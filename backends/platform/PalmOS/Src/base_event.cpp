/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "be_base.h"
#include "common/events.h"

#ifdef STDLIB_TRACE_MEMORY
#	include <stdlib.h>
#endif

#if defined(COMPILE_OS5) && defined(PALMOS_ARM)
extern "C" void SysEventGet(EventType *eventP, Int32 timeout);
extern "C" void SysEventAddToQueue (const EventType *eventP);
#endif

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
#if defined(COMPILE_OS5) && defined(PALMOS_ARM)
				SysEventAddToQueue(&event);
#else
				EvtAddEventToQueue(&event);
#endif
			}
		}

		_batCheckLast = TimGetTicks();
	}
}

bool OSystem_PalmBase::pollEvent(Common::Event &event) {
	::EventType ev;
	Boolean handled;
	UInt32 keyCurrentState;
	Coord x, y;

	battery_handler();
	timer_handler();
	sound_handler();

	for (;;) {
		// check for hardkey repeat for mouse emulation
		keyCurrentState = KeyCurrentState();

		// if it was a key pressed, let the keyup event raise
		if (_keyExtraPressed) {
			if (gVars->arrowKeys) {
				if (_keyExtraPressed & _keyExtra.bitLeft) {
					if (!(keyCurrentState & _keyExtra.bitLeft)) {
						_keyExtraPressed &= ~_keyExtra.bitLeft;

						event.type = Common::EVENT_KEYUP;
						event.kbd.keycode = Common::KEYCODE_LEFT;
						event.kbd.ascii = event.kbd.keycode;
						event.kbd.flags = 0;
						return true;
					}
				}
				if (_keyExtraPressed & _keyExtra.bitRight) {
					if (!(keyCurrentState & _keyExtra.bitRight)) {
						_keyExtraPressed &= ~_keyExtra.bitRight;

						event.type = Common::EVENT_KEYUP;
						event.kbd.keycode = Common::KEYCODE_RIGHT;
						event.kbd.ascii = event.kbd.keycode;
						event.kbd.flags = 0;
						return true;
					}
				}
				if (_keyExtraPressed & _keyExtra.bitUp) {
					if (!(keyCurrentState & _keyExtra.bitUp)) {
						_keyExtraPressed &= ~_keyExtra.bitUp;

						event.type = Common::EVENT_KEYUP;
						event.kbd.keycode = Common::KEYCODE_UP;
						event.kbd.ascii = event.kbd.keycode;
						event.kbd.flags = 0;
						return true;
					}
				}
				if (_keyExtraPressed & _keyExtra.bitDown) {
					if (!(keyCurrentState & _keyExtra.bitDown)) {
						_keyExtraPressed &= ~_keyExtra.bitDown;

						event.type = Common::EVENT_KEYUP;
						event.kbd.keycode = Common::KEYCODE_DOWN;
						event.kbd.ascii = event.kbd.keycode;
						event.kbd.flags = 0;
						return true;
					}
				}
			}

			if (_keyExtraPressed & _keyExtra.bitActionA) {
				if (!(keyCurrentState & _keyExtra.bitActionA)) {
					_keyExtraPressed &= ~_keyExtra.bitActionA;

					event.type = Common::EVENT_LBUTTONUP;
					event.mouse.x = _mouseCurState.x;
					event.mouse.y = _mouseCurState.y;
					return true;
				}
			}

			if (_keyExtraPressed & _keyExtra.bitActionB) {
				if (!(keyCurrentState & _keyExtra.bitActionB)) {
					_keyExtraPressed &= ~_keyExtra.bitActionB;

					event.type = Common::EVENT_RBUTTONUP;
					event.mouse.x = _mouseCurState.x;
					event.mouse.y = _mouseCurState.y;
					return true;
				}
			}

			// no more event till up is raised
			return false;
		}

		if (!(keyCurrentState & _keyExtraMask)) {
			_lastKeyRepeat = 0;

		} else if (getMillis() >= (_keyExtraRepeat + _keyExtraDelay)) {
			_keyExtraRepeat = getMillis();

			if (gVars->arrowKeys) {
				if (keyCurrentState & _keyExtra.bitLeft) {
					_keyExtraPressed |= _keyExtra.bitLeft;
					event.kbd.keycode = Common::KEYCODE_LEFT;

				} else if (keyCurrentState & _keyExtra.bitRight) {
					_keyExtraPressed |= _keyExtra.bitRight;
					event.kbd.keycode = Common::KEYCODE_RIGHT;

				} else if (keyCurrentState & _keyExtra.bitUp) {
					_keyExtraPressed |= _keyExtra.bitUp;
					event.kbd.keycode = Common::KEYCODE_UP;

				} else if (keyCurrentState & _keyExtra.bitDown) {
					_keyExtraPressed |= _keyExtra.bitDown;
					event.kbd.keycode = Common::KEYCODE_DOWN;
				}

				event.type = Common::EVENT_KEYDOWN;
				event.kbd.ascii = event.kbd.keycode;
				event.kbd.flags = 0;
				return true;

			} else {
				Int8 sx = 0;
				Int8 sy = 0;

				if (keyCurrentState & _keyExtra.bitUp)
					sy = -1;
				else if (keyCurrentState & _keyExtra.bitDown)
					sy = +1;

				if (keyCurrentState & _keyExtra.bitLeft)
					sx = -1;
				else if (keyCurrentState & _keyExtra.bitRight)
					sx = +1;

				if (sx || sy) {
					simulate_mouse(event, sx, sy, &x, &y);
					event.type = Common::EVENT_MOUSEMOVE;
					event.mouse.x = x;
					event.mouse.y = y;
					warpMouse(x, y);

					return true;
				}
			}
		}

#if defined(COMPILE_OS5) && defined(PALMOS_ARM)
		SysEventGet(&ev, evtNoWait);
#else
		EvtGetEvent(&ev, evtNoWait);
#endif

		if (ev.eType == keyUpEvent) {
			Common::KeyCode k = Common::KEYCODE_INVALID;
			switch (ev.data.keyUp.chr) {

			// arrow keys
			case chrUpArrow:
				k = Common::KEYCODE_UP; break;
			case chrDownArrow:
				k = Common::KEYCODE_DOWN; break;
			case chrRightArrow:
				k = Common::KEYCODE_RIGHT; break;
			case chrLeftArrow:
				k = Common::KEYCODE_LEFT; break;
			}

			if (k) {
				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = k;
				event.kbd.ascii = k;
				event.kbd.flags = 0;
				return true;
			}

		} else if (ev.eType == keyDownEvent) {
			Common::KeyCode k = Common::KEYCODE_INVALID;
			switch (ev.data.keyDown.chr) {
			// ESC key
			case vchrLaunch:
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_ESCAPE;
				event.kbd.ascii = Common::ASCII_ESCAPE;
				event.kbd.flags = 0;
				return true;

			// F5 = menu
			case vchrMenu:
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
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
				return true;

			// arrow keys
			case chrUpArrow:
				k = Common::KEYCODE_UP; break;
			case chrDownArrow:
				k = Common::KEYCODE_DOWN; break;
			case chrRightArrow:
				k = Common::KEYCODE_RIGHT; break;
			case chrLeftArrow:
				k = Common::KEYCODE_LEFT; break;

			// return
			case chrLineFeed:
			case chrCarriageReturn:
				k = Common::KEYCODE_RETURN; break;

			}

			if (k) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = k;
				event.kbd.ascii = k;
				event.kbd.flags = 0;
				return true;
			}
		}

		if (check_event(event, &ev))
			return true;

		// prevent crash when alarm is raised
		handled = ((ev.eType == keyDownEvent) &&
						(ev.data.keyDown.modifiers & commandKeyMask) &&
						((ev.data.keyDown.chr == vchrAttnStateChanged) ||
						(ev.data.keyDown.chr == vchrAttnUnsnooze)));

		// graffiti strokes, auto-off, etc...
		if (!handled)
			if (SysHandleEvent(&ev))
				continue;

		switch (ev.eType) {
		case penMoveEvent:
			get_coordinates(&ev, x, y);

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return false;

			if (abs(y - event.mouse.y) <= 2 || abs(x - event.mouse.x) <= 2)
				return false;

			event.type = Common::EVENT_MOUSEMOVE;
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

				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = (Common::KeyCode)num;
				event.kbd.ascii = num;
				event.kbd.flags = 0;

				_wasKey = true;
				return true;
			}

			if (y > _screenHeight || y < 0 || x > _screenWidth || x < 0)
				return false;

			event.type = ((gVars->stylusClick || _overlayVisible) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_MOUSEMOVE);
			event.mouse.x = x;
			event.mouse.y = y;
			warpMouse(x, y);
			return true;

		case penUpEvent:
			get_coordinates(&ev, x, y);

			event.type = ((gVars->stylusClick || _overlayVisible) ? Common::EVENT_LBUTTONUP : Common::EVENT_MOUSEMOVE);
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
				if (ev.data.keyDown.modifiers & shiftKeyMask)	mask |= Common::KBD_SHIFT;
				if (ev.data.keyDown.modifiers & controlKeyMask)	mask |= Common::KBD_CTRL;
				if (ev.data.keyDown.modifiers & optionKeyMask)	mask |= Common::KBD_ALT;
				if (ev.data.keyDown.modifiers & commandKeyMask) mask |= Common::KBD_CTRL|Common::KBD_ALT;
			} else {
				// for grafiti mode
				if (_lastKeyModifier == kModifierCommand)	mask = Common::KBD_CTRL|Common::KBD_ALT;
				if (_lastKeyModifier == kModifierAlt)		mask = Common::KBD_ALT;
				if (_lastKeyModifier == kModifierCtrl)		mask = Common::KBD_CTRL;
			}

			if (_lastKeyModifier)
				draw_osd(kDrawKeyState, 2, _screenDest.h + 2, false);
			_lastKeyModifier = kModifierNone;

			// F1 -> F10 key
			if  (key >= '0' && key <= '9' && mask == (Common::KBD_CTRL|Common::KBD_ALT)) {
				key = (key == '0') ? 324 : (315 + key - '1');
				mask = 0;

#ifdef STDLIB_TRACE_MEMORY
			// print memory
			} else if  (key == 'm' && mask == (Common::KBD_CTRL|Common::KBD_ALT)) {
				printf("Used memory: %d\n", __stdlib_trace_memory);
#endif
			// exit
			} else if  ((key == 'z' && mask == Common::KBD_CTRL) || (mask == Common::KBD_ALT && key == 'x')) {
				event.type = Common::EVENT_QUIT;
				return true;

			// num pad (indy fight mode)
			} else if (key == 'n' && mask == (Common::KBD_CTRL|Common::KBD_ALT) && !_overlayVisible) {
				_useNumPad = !_useNumPad;
				draw_osd(kDrawFight, _screenDest.w - 34, _screenDest.h + 2, _useNumPad, 1);
				displayMessageOnOSD(_useNumPad ? "Fight mode on." : "Fight mode off.");
				return false;
			}

			// other keys
			_wasKey = true;
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = (Common::KeyCode)key;
			event.kbd.ascii = key;
			event.kbd.flags = mask;
			return true;

		default:
			if (_wasKey && ev.eType != keyHoldEvent) {
				event.type = Common::EVENT_KEYUP;
				_wasKey = false;
				return true;
			}
			return false;
		};
	}
}
