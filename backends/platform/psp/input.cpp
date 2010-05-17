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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/platform/psp/osys_psp.cpp $
 * $Id: osys_psp.cpp 43618 2009-08-21 22:44:49Z joostp $
 *
 */

// Todo: handle events that should fire because of shift going off
// Solution: handle shift on a button-by-button basis, only allowing it when the button is up. Also a inputmap-wide button. At buttonup, shiftstate is inspected per button.

//#define __PSP_DEBUG_FUNCS__	/* Uncomment for debugging the stack */
//#define __PSP_DEBUG_PRINT__ /* Uncomment for debug prints */

#include "backends/platform/psp/trace.h"

#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/input.h"

// Defines for working with PSP buttons
#define CHANGED(x)	 (_buttonsChanged & (x))
#define PRESSED(x)   ((_buttonsChanged & (x)) && (pad.Buttons & (x)))
#define UNPRESSED(x) ((_buttonsChanged & (x)) && !(pad.Buttons & (x)))
#define DOWN(x)		 (pad.Buttons & (x))
#define UP(x)		 (!(pad.Buttons & (x)))
#define PSP_DPAD	 (PSP_CTRL_DOWN|PSP_CTRL_UP|PSP_CTRL_LEFT|PSP_CTRL_RIGHT)
#define PSP_4BUTTONS (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_TRIANGLE | PSP_CTRL_SQUARE)
#define PSP_TRIGGERS (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER)

#define PAD_CHECK_TIME	53

void InputHandler::init() {
	sceCtrlSetSamplingCycle(0);	// set sampling to vsync. n = n usecs
	sceCtrlSetSamplingMode(1);  // analog
}

bool InputHandler::getAllInputs(Common::Event &event) {
	DEBUG_ENTER_FUNC();

	uint32 time = g_system->getMillis();	// may not be necessary with read
	if (time - _lastPadCheckTime < PAD_CHECK_TIME) {
		return false;
	}

	_lastPadCheckTime = time;
	SceCtrlData pad;

	sceCtrlPeekBufferPositive(&pad, 1);	// Peek ignores sleep. Read sleeps thread

	bool haveEvent;

	memset(&event, 0, sizeof(event));

	if (_keyboard->isVisible())
		haveEvent = _keyboard->processInput(event, pad);
	else
		haveEvent = getEvent(event, pad);

	if (haveEvent) {
		PSP_DEBUG_PRINT("Have event[%s]\n", haveEvent ? "true" : "false");
		PSP_DEBUG_PRINT("event.type[%d]\n", event.type);
	}

	return haveEvent;
}

bool InputHandler::getEvent(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	_buttonsChanged = pad.Buttons ^ _prevButtons;
	bool haveEvent = false;

	// Collect events from different sources
	haveEvent = getDpadEvent(event, pad);

	if (!haveEvent)
		haveEvent = getButtonEvent(event, pad);

	if (!haveEvent)
		haveEvent = getNubEvent(event, pad);

	_prevButtons = pad.Buttons;

	return haveEvent;
}

bool InputHandler::getDpadEvent(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	int newDpadX = 0, newDpadY = 0;
	bool haveEvent = false;

	if (DOWN(PSP_CTRL_UP)) {
		newDpadY++;
		if (DOWN(PSP_CTRL_RTRIGGER))	// Shifting causes diagonals
			newDpadX++;
	}
	if (DOWN(PSP_CTRL_RIGHT)) {
		newDpadX++;
		if (DOWN(PSP_CTRL_RTRIGGER))
			newDpadY--;
	}
	if (DOWN(PSP_CTRL_DOWN)) {
		newDpadY--;
		if (DOWN(PSP_CTRL_RTRIGGER))
			newDpadX--;
	}
	if (DOWN(PSP_CTRL_LEFT)) {
		newDpadX--;
		if (DOWN(PSP_CTRL_RTRIGGER))
			newDpadY++;
	}

	if (newDpadX != _dpadX || newDpadY != _dpadY) {
		if (_dpadX == 0 && _dpadY == 0)	{		// We were in the middle so we pressed dpad
			event.type = Common::EVENT_KEYDOWN;
			event.kbd.keycode = translateDpad(newDpadX, newDpadY);
			event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';	// Get ascii
			_dpadX = newDpadX;
			_dpadY = newDpadY;
		} else if (newDpadX == 0 && newDpadY == 0) {// We're now centered so we unpressed dpad
			event.type = Common::EVENT_KEYUP;
			event.kbd.keycode = translateDpad(_dpadX, _dpadY);
			event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			_dpadX = newDpadX;
			_dpadY = newDpadY;
		} else { 	// we moved from one pressed dpad direction to another one
			event.type = Common::EVENT_KEYUP;	// first release the last dpad direction
			event.kbd.keycode = translateDpad(_dpadX, _dpadY);
			event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
			_dpadX = 0; // so that we'll pick up a new dpad movement the next round
			_dpadY = 0;
		}

		PSP_DEBUG_PRINT("Keypad event. DpadX[%d], DpadY[%d]\n", _dpadX, _dpadY);
		haveEvent = true;
	}

	return haveEvent;
}

inline Common::KeyCode InputHandler::translateDpad(int x, int y) {
	DEBUG_ENTER_FUNC();
	Common::KeyCode key;

	if (x == -1) {
		if (y == -1)
			key = Common::KEYCODE_KP1;
		else if (y == 0)
			key = Common::KEYCODE_KP4;
		else /* y == 1 */
			key = Common::KEYCODE_KP7;
	} else if (x == 0) {
		if (y == -1)
			key = Common::KEYCODE_KP2;
		else /* y == 1 */
			key = Common::KEYCODE_KP8;
	} else {/* x == 1 */
		if (y == -1)
			key = Common::KEYCODE_KP3;
		else if (y == 0)
			key = Common::KEYCODE_KP6;
		else /* y == 1 */
			key = Common::KEYCODE_KP9;
	}

	return key;
}


bool InputHandler::getButtonEvent(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();
	bool haveEvent = false;

	if (PRESSED(PSP_CTRL_SELECT))
		_keyboard->setVisible(true);

	else if (CHANGED(PSP_4BUTTONS | PSP_TRIGGERS | PSP_CTRL_START)) {
		if (CHANGED(PSP_CTRL_CROSS)) {
			event.type = DOWN(PSP_CTRL_CROSS) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
			event.mouse.x = _cursor->getX();	// Could this have to do with SCI enter problem?
			event.mouse.y = _cursor->getY();
			PSP_DEBUG_PRINT("%s\n", event.type == Common::EVENT_LBUTTONDOWN ? "LButtonDown" : "LButtonUp");
		} else if (CHANGED(PSP_CTRL_CIRCLE)) {
			event.type = DOWN(PSP_CTRL_CIRCLE) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
			event.mouse.x = _cursor->getX();
			event.mouse.y = _cursor->getY();
			PSP_DEBUG_PRINT("%s\n", event.type == Common::EVENT_LBUTTONDOWN ? "RButtonDown" : "RButtonUp");
		} else {
			//any of the other buttons.
			event.type = _buttonsChanged & pad.Buttons ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
			event.kbd.ascii = 0;
			event.kbd.flags = 0;

			if (CHANGED(PSP_CTRL_LTRIGGER)) {
				event.kbd.keycode = Common::KEYCODE_ESCAPE;
				event.kbd.ascii = 27;
			} else if (CHANGED(PSP_CTRL_START)) {
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				if (DOWN(PSP_CTRL_RTRIGGER)) {
					event.kbd.flags |= Common::KBD_CTRL;	// Main menu to allow RTL
				}
			} else if (CHANGED(PSP_CTRL_SQUARE)) {
				event.kbd.keycode = Common::KEYCODE_PERIOD;
				event.kbd.ascii = '.';
			} else if (CHANGED(PSP_CTRL_TRIANGLE)) {
				event.kbd.keycode = Common::KEYCODE_RETURN;
				event.kbd.ascii = '\r';
			} else if (DOWN(PSP_CTRL_RTRIGGER)) {			// An event
				event.kbd.flags |= Common::KBD_SHIFT;
			}
			PSP_DEBUG_PRINT("Ascii[%d]. Key %s.\n", event.kbd.ascii, event.type == Common::EVENT_KEYDOWN ? "down" : "up");
		}

		haveEvent = true;
	}

	return haveEvent;
}

bool InputHandler::getNubEvent(Common::Event &event, SceCtrlData &pad) {
	DEBUG_ENTER_FUNC();

	bool haveEvent = false;
	int32 analogStepX = pad.Lx;		// Goes up to 255.
	int32 analogStepY = pad.Ly;

	int32 oldX = _cursor->getX();
	int32 oldY = _cursor->getY();

	analogStepX = modifyNubAxisMotion(analogStepX);
	analogStepY = modifyNubAxisMotion(analogStepY);

	if (analogStepX != 0 || analogStepY != 0) {

		PSP_DEBUG_PRINT("raw x[%d], y[%d]\n", analogStepX, analogStepY);

		// If no movement then this has no effect
		if (DOWN(PSP_CTRL_RTRIGGER)) {
			// Fine control mode for analog
			if (analogStepX != 0) {
				if (analogStepX > 0)
					_cursor->increaseXY(2, 0);
				else
					_cursor->increaseXY(-2, 0);
			}

			if (analogStepY != 0) {
				if (analogStepY > 0)
					_cursor->increaseXY(0, 2);
				else
					_cursor->increaseXY(0, -2);
			}
		} else {	// Regular speed movement
			_cursor->increaseXY(analogStepX, 0);
			_cursor->increaseXY(0, analogStepY);
		}

		int32 newX = _cursor->getX();
		int32 newY = _cursor->getY();

		if ((oldX != newX) || (oldY != newY)) {
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse.x = newX;
			event.mouse.y = newY;
			haveEvent = true;

			PSP_DEBUG_PRINT("Nub event. X[%d], Y[%d]\n", newX, newY);
		}
	}
	return haveEvent;
}

inline int32 InputHandler::modifyNubAxisMotion(int32 input) {
	DEBUG_ENTER_FUNC();
	const int MIN_NUB_MOTION = 30;

	input -= 128;	// Center on 0.

	if (input < -MIN_NUB_MOTION - 1)
		input += MIN_NUB_MOTION + 1;	// reduce the velocity
	else if (input > MIN_NUB_MOTION)
		input -= MIN_NUB_MOTION;	// same
	else 				// between these points, dampen the response to 0
		input = 0;

	return input;
}
