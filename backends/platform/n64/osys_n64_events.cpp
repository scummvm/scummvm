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

#include <math.h> // Needed for "tan()" function

#include "osys_n64.h"

// Pad buttons
#define START_BUTTON(a) (a & 0x1000)
#define A_BUTTON(a)     (a & 0x8000)
#define B_BUTTON(a)     (a & 0x4000)
#define Z_BUTTON(a)     (a & 0x2000)

// Triggers
#define TL_BUTTON(a)    (a & 0x0020)
#define TR_BUTTON(a)    (a & 0x0010)

// D-Pad
#define DL_BUTTON(a)    (a & 0x0200)
#define DR_BUTTON(a)    (a & 0x0100)
#define DU_BUTTON(a)    (a & 0x0800)
#define DD_BUTTON(a)    (a & 0x0400)

// Yellow C buttons
#define CL_BUTTON(a)    (a & 0x0002)
#define CR_BUTTON(a)    (a & 0x0001)
#define CU_BUTTON(a)    (a & 0x0008)
#define CD_BUTTON(a)    (a & 0x0004)

// Macro for button press checking
#define PRESSED_START(now, before)	(START_BUTTON(now) && !START_BUTTON(before))
#define RELEASED_START(now, before)	(!START_BUTTON(now) && START_BUTTON(before))
#define PRESSED_A(now, before)		(A_BUTTON(now) && !A_BUTTON(before))
#define RELEASED_A(now, before)		(!A_BUTTON(now) && A_BUTTON(before))
#define PRESSED_B(now, before)		(B_BUTTON(now) && !B_BUTTON(before))
#define RELEASED_B(now, before)		(!B_BUTTON(now) && B_BUTTON(before))
#define PRESSED_Z(now, before)		(Z_BUTTON(now) && !Z_BUTTON(before))
#define RELEASED_Z(now, before)		(!Z_BUTTON(now) && Z_BUTTON(before))
#define PRESSED_TL(now, before)		(TL_BUTTON(now) && !TL_BUTTON(before))
#define RELEASED_TL(now, before)	(!TL_BUTTON(now) && TL_BUTTON(before))
#define PRESSED_TR(now, before)		(TR_BUTTON(now) && !TR_BUTTON(before))
#define RELEASED_TR(now, before)	(!TR_BUTTON(now) && TR_BUTTON(before))
#define PRESSED_DL(now, before)		(DL_BUTTON(now) && !DL_BUTTON(before))
#define RELEASED_DL(now, before)	(!DL_BUTTON(now) && DL_BUTTON(before))
#define PRESSED_DR(now, before)		(DR_BUTTON(now) && !DR_BUTTON(before))
#define RELEASED_DR(now, before)	(!DR_BUTTON(now) && DR_BUTTON(before))
#define PRESSED_DU(now, before) 	(DU_BUTTON(now) && !DU_BUTTON(before))
#define RELEASED_DU(now, before)	(!DU_BUTTON(now) && DU_BUTTON(before))
#define PRESSED_DD(now, before)		(DD_BUTTON(now) && !DD_BUTTON(before))
#define RELEASED_DD(now, before)	(!DD_BUTTON(now) && DD_BUTTON(before))
#define PRESSED_CL(now, before)		(CL_BUTTON(now) && !CL_BUTTON(before))
#define RELEASED_CL(now, before)	(!CL_BUTTON(now) && CL_BUTTON(before))
#define PRESSED_CR(now, before)		(CR_BUTTON(now) && !CR_BUTTON(before))
#define RELEASED_CR(now, before)	(!CR_BUTTON(now) && CR_BUTTON(before))
#define PRESSED_CU(now, before)		(CU_BUTTON(now) && !CU_BUTTON(before))
#define RELEASED_CU(now, before)	(!CU_BUTTON(now) && CU_BUTTON(before))
#define PRESSED_CD(now, before)		(CD_BUTTON(now) && !CD_BUTTON(before))
#define RELEASED_CD(now, before)	(!CD_BUTTON(now) && CD_BUTTON(before))


#define MOUSE_DEADZONE 0
#define PAD_DEADZONE 1
#define PAD_ACCELERATION 15
#define PAD_CHECK_TIME 40

static controller_data_buttons _ctrlData;

void OSystem_N64::readControllerAnalogInput(void) {
	int8 pad_analogX, pad_analogY;
	int8 pad_mouseX, pad_mouseY;

	// Read current controller status
	controller_Read_Buttons(&_ctrlData);

	pad_analogX = (_ctrlData.c[_controllerPort].throttle >> 8) & 0xFF;
	pad_analogY = (_ctrlData.c[_controllerPort].throttle >> 0) & 0xFF;

	pad_mouseX = 0;
	pad_mouseY = 0;

	if (_mousePort >= 0) { // If mouse is present, read movement values
		pad_mouseX = (_ctrlData.c[_mousePort].throttle >> 8) & 0xFF;
		pad_mouseY = (_ctrlData.c[_mousePort].throttle >> 0) & 0xFF;
	}

	float mx = _tempMouseX;
	float my = _tempMouseY;

	// Limit the analog range for pad.
	// When moving in diagonal the max/min of 128/-128 was not reached
	// yielding weird results for the tangent acceleration function
	if (pad_analogX > 60) pad_analogX = 60;
	else if (pad_analogX < -60) pad_analogX = -60;
	if (pad_analogY > 60) pad_analogY = 60;
	else if (pad_analogY < -60) pad_analogY = -60;

	// Gamepad
	if (abs(pad_analogX) > PAD_DEADZONE)
		mx += tan(pad_analogX * (PI / 140));
	if (abs(pad_analogY) > PAD_DEADZONE)
		my -= tan(pad_analogY * (PI / 140));

	// Mouse
	if (abs(pad_mouseX) > MOUSE_DEADZONE)
		mx += pad_mouseX;
	if (abs(pad_mouseY) > MOUSE_DEADZONE)
		my -= pad_mouseY;

	if (mx < 0)
		mx = 0;

	if (mx >= _mouseMaxX)
		mx = _mouseMaxX - 1;

	if (my < 0)
		my = 0;

	if (my >= _mouseMaxY)
		my = _mouseMaxY - 1;

	_tempMouseX = mx;
	_tempMouseY = my;
}

bool OSystem_N64::pollEvent(Common::Event &event) {
	// Check Timers. Not the best place, but checking in interrupts proved to be unsafe
	checkTimers();

	// Refill audio buffers, doing this inside interrupts could be harmful
	refillAudioBuffers();

	// Read current controller status
	controller_Read_Buttons(&_ctrlData);

	static uint16 oldButtons = 0; // old button data... used for button press/release
	static uint16 oldMouseButtons = 0;

	uint16 newButtons = _ctrlData.c[_controllerPort].buttons; // Read from controller
	uint16 newMouseButtons = 0;

	if (_mousePort >= 0)
		newMouseButtons = _ctrlData.c[_mousePort].buttons;

	bool buttonPressed = false;
	static bool left_digital = false;
	static bool right_digital = false;
	static bool up_digital = false;
	static bool down_digital = false;

	if (newButtons != oldButtons) { // Check PAD button press
		if (PRESSED_DL(newButtons, oldButtons)) // Pressed LEFT
			left_digital = true;
		else if (RELEASED_DL(newButtons, oldButtons)) // Released LEFT
			left_digital = false;

		if (PRESSED_DR(newButtons, oldButtons)) // Pressed RIGHT
			right_digital = true;
		else if (RELEASED_DR(newButtons, oldButtons)) // Released RIGHT
			right_digital = false;

		if (PRESSED_DU(newButtons, oldButtons)) // Pressed UP
			up_digital = true;
		else if (RELEASED_DU(newButtons, oldButtons)) // Released UP
			up_digital = false;

		if (PRESSED_DD(newButtons, oldButtons)) // Pressed DOWN
			down_digital = true;
		else if (RELEASED_DD(newButtons, oldButtons)) // Released DOWN
			down_digital = false;

		if (PRESSED_B(newButtons, oldButtons)) { // Pressed B - Right Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONDOWN;
		} else if (RELEASED_B(newButtons, oldButtons)) { // Released B
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONUP;
		} else if (PRESSED_A(newButtons, oldButtons)) { // Pressed A - Period
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = '.';
			event.type = Common::EVENT_KEYDOWN;
		} else if (RELEASED_A(newButtons, oldButtons)) { // Released A
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = '.';
			event.type = Common::EVENT_KEYUP;
		} else if (PRESSED_START(newButtons, oldButtons)) { // Pressed START
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = Common::ASCII_F5;
			event.type = Common::EVENT_KEYDOWN;
		} else if (RELEASED_START(newButtons, oldButtons)) { // Released START
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = Common::ASCII_F5;
			event.type = Common::EVENT_KEYUP;
		}  else if (PRESSED_TL(newButtons, oldButtons)) { // Pressed Trigger Left - ESC
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = 27;
			event.type = Common::EVENT_KEYDOWN;
		} else if (RELEASED_TL(newButtons, oldButtons)) { // Released Trigger Left
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = 27;
			event.type = Common::EVENT_KEYUP;
		} else if (PRESSED_TR(newButtons, oldButtons)) { // Pressed Trigger Right - F7
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F7;
			event.kbd.ascii = Common::ASCII_F7;
			event.type = Common::EVENT_KEYDOWN;
		} else if (RELEASED_TL(newButtons, oldButtons)) { // Released Trigger Right
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F7;
			event.kbd.ascii = Common::ASCII_F7;
			event.type = Common::EVENT_KEYUP;
		} else if (PRESSED_Z(newButtons, oldButtons)) { // Pressed Z - Left Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONDOWN;
		} else if (RELEASED_Z(newButtons, oldButtons)) { // Released Z
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONUP;
		}

		// Simulate numpad using yellow C-Buttons on the pad
		bool cur_CU, cur_CD, cur_CL, cur_CR;

		static int8 lastKPad = 0; // Latest simulated keypad button press

		// Check which directions are pressed
		if (CU_BUTTON(newButtons)) // Pressed Yellow Up
			cur_CU = true;
		else if (!CU_BUTTON(newButtons)) // Released Yellow Up
			cur_CU = false;

		if (CD_BUTTON(newButtons)) // Pressed Yellow Down
			cur_CD = true;
		else if (!CD_BUTTON(newButtons)) // Released Yellow Down
			cur_CD = false;

		if (CL_BUTTON(newButtons)) // Pressed Yellow Left
			cur_CL = true;
		else if (!CL_BUTTON(newButtons)) // Released Yellow Left
			cur_CL = false;

		if (CR_BUTTON(newButtons)) // Pressed Yellow Right
			cur_CR = true;
		else if (!CR_BUTTON(newButtons)) // Released Yellow Right
			cur_CR = false;

		switch (lastKPad) {
		case 7: // UP - LEFT
			if (!(cur_CU && cur_CL)) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP7;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 9: // UP - RIGHT
			if (!(cur_CU && cur_CR)) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP9;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 1: // DOWN - LEFT
			if (!(cur_CD && cur_CL)) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP1;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 3: // DOWN - RIGHT
			if (!(cur_CD && cur_CR)) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP3;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 4: // LEFT
			if (!cur_CL) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP4;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 6: // RIGHT
			if (!cur_CR) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP6;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 8: // UP
			if (!cur_CU) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP8;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 2: // DOWN
			if (!cur_CD) {
				buttonPressed = true;
				event.kbd.keycode = Common::KEYCODE_KP2;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				event.type = Common::EVENT_KEYUP;
				lastKPad = 0;
			}
			break;
		case 0: // No previous press
			if (cur_CU && cur_CL) { // UP - LEFT
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP7;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 7;
			} else if (cur_CU && cur_CR) { // UP - RIGHT
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP9;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 9;
			} else if (cur_CD && cur_CL) { // DOWN - LEFT
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP1;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 1;
			} else if (cur_CD && cur_CR) { // DOWN - RIGHT
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP3;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 3;
			} else if (cur_CL) { // LEFT
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP4;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 4;
			} else if (cur_CR) { // RIGHT
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP6;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 6;
			} else if (cur_CU) { // UP
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP8;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 8;
			} else if (cur_CD) { // DOWN
				buttonPressed = true;
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_KP2;
				event.kbd.ascii = event.kbd.keycode - Common::KEYCODE_KP0 + '0';
				lastKPad = 2;
			}
			break;
		default:
			break; // Do nothing.
		}

		oldButtons = newButtons; // Save current button status

		if (buttonPressed) {
			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;
			return true;
		}
	}

	if (newMouseButtons != oldMouseButtons) { // Check mouse button press
		if (PRESSED_B(newMouseButtons, oldMouseButtons)) { // Pressed Right Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONDOWN;
		} else if (RELEASED_B(newMouseButtons, oldMouseButtons)) { // Released RMB
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONUP;
		} else if (PRESSED_A(newMouseButtons, oldMouseButtons)) { // Pressed Left Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONDOWN;
		} else if (RELEASED_A(newMouseButtons, oldMouseButtons)) { // Released LMB
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONUP;
		}

		oldMouseButtons = newMouseButtons; // Save current button status

		if (buttonPressed) {
			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;
			return true;
		}
	}

	static uint32 _lastPadCheck = 0;
	uint32 curTime = getMillis();

	if ((curTime - _lastPadCheck) > PAD_CHECK_TIME) {
		_lastPadCheck = curTime;

		float mx = _tempMouseX;
		float my = _tempMouseY;

		if (left_digital || right_digital || up_digital || down_digital) {
			if (left_digital)
				mx -= 2;
			else if (right_digital)
				mx += 2;
			if (up_digital)
				my -= 2;
			else if (down_digital)
				my += 2;
		}

		if (mx < 0)
			mx = 0;

		if (mx >= _mouseMaxX)
			mx = _mouseMaxX - 1;

		if (my < 0)
			my = 0;

		if (my >= _mouseMaxY)
			my = _mouseMaxY - 1;

		if ((mx != _mouseX) || (my != _mouseY)) {
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse.x = _mouseX = _tempMouseX = mx;
			event.mouse.y = _mouseY = _tempMouseY = my;

			_dirtyOffscreen = true;

			return true;
		}

	}

	return false;
}

