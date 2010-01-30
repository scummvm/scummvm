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

#include "osys_n64.h"

#include <math.h>

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

	if (pad_analogX > 60) pad_analogX = 60;
	else if (pad_analogX < -60) pad_analogX = -60;

	if (pad_analogY > 60) pad_analogY = 60;
	else if (pad_analogY < -60) pad_analogY = -60;

	if (abs(pad_analogX) > PAD_DEADZONE)
		mx += tan(pad_analogX * (PI / 140));

	if (abs(pad_analogY) > PAD_DEADZONE)
		my -= tan(pad_analogY * (PI / 140));

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
		if (DL_BUTTON(newButtons) && !DL_BUTTON(oldButtons)) // Pressed LEFT
			left_digital = true;
		else if (!DL_BUTTON(newButtons) && DL_BUTTON(oldButtons)) // Released LEFT
			left_digital = false;

		if (DR_BUTTON(newButtons) && !DR_BUTTON(oldButtons)) // Pressed RIGHT
			right_digital = true;
		else if (!DR_BUTTON(newButtons) && DR_BUTTON(oldButtons)) // Released RIGHT
			right_digital = false;

		if (DU_BUTTON(newButtons) && !DU_BUTTON(oldButtons)) // Pressed UP
			up_digital = true;
		else if (!DU_BUTTON(newButtons) && DU_BUTTON(oldButtons)) // Released UP
			up_digital = false;

		if (DD_BUTTON(newButtons) && !DD_BUTTON(oldButtons)) // Pressed DOWN
			down_digital = true;
		else if (!DD_BUTTON(newButtons) && DD_BUTTON(oldButtons)) // Released DOWN
			down_digital = false;

		if (B_BUTTON(newButtons) && !B_BUTTON(oldButtons)) { // Pressed B - Right Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONDOWN;
		} else if (!B_BUTTON(newButtons) && B_BUTTON(oldButtons)) { // Released B
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONUP;
		} else if (A_BUTTON(newButtons) && !A_BUTTON(oldButtons)) { // Pressed A - Period
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = '.';
			event.type = Common::EVENT_KEYDOWN;
		} else if (!A_BUTTON(newButtons) && A_BUTTON(oldButtons)) { // Released A
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = '.';
			event.type = Common::EVENT_KEYUP;
		} else if (START_BUTTON(newButtons) && !START_BUTTON(oldButtons)) { // Pressed START - F5
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = Common::ASCII_F5;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!START_BUTTON(newButtons) && START_BUTTON(oldButtons)) { // Released START
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = Common::ASCII_F5;
			event.type = Common::EVENT_KEYUP;
		} else if (CU_BUTTON(newButtons) && !CU_BUTTON(oldButtons)) { // Pressed Yellow Up - UP
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_UP;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!CU_BUTTON(newButtons) && CU_BUTTON(oldButtons)) { // Released Yellow Up
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_UP;
			event.type = Common::EVENT_KEYUP;
		} else if (CD_BUTTON(newButtons) && !CD_BUTTON(oldButtons)) { // Pressed Yellow Down - DOWN
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_DOWN;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!CD_BUTTON(newButtons) && CD_BUTTON(oldButtons)) { // Released Yellow Down
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_DOWN;
			event.type = Common::EVENT_KEYUP;
		} else if (CL_BUTTON(newButtons) && !CL_BUTTON(oldButtons)) { // Pressed Yellow Left - LEFT
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_LEFT;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!CL_BUTTON(newButtons) && CL_BUTTON(oldButtons)) { // Released Yellow Left
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_LEFT;
			event.type = Common::EVENT_KEYUP;
		} else if (CR_BUTTON(newButtons) && !CR_BUTTON(oldButtons)) { // Pressed Yellow Right - RIGHT
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_RIGHT;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!CR_BUTTON(newButtons) && CR_BUTTON(oldButtons)) { // Released Yellow Right
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_RIGHT;
			event.type = Common::EVENT_KEYUP;
		} else if (TL_BUTTON(newButtons) && !TL_BUTTON(oldButtons)) { // Pressed Trigger Left - ESC
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = 27;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!TL_BUTTON(newButtons) && TL_BUTTON(oldButtons)) { // Released Trigger Left
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = 27;
			event.type = Common::EVENT_KEYUP;
		} else if (TR_BUTTON(newButtons) && !TR_BUTTON(oldButtons)) { // Pressed Trigger Right - F7
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F7;
			event.kbd.ascii = Common::ASCII_F7;
			event.type = Common::EVENT_KEYDOWN;
		} else if (!TR_BUTTON(newButtons) && TR_BUTTON(oldButtons)) { // Released Trigger Right
			buttonPressed = true;
			event.kbd.keycode = Common::KEYCODE_F7;
			event.kbd.ascii = Common::ASCII_F7;
			event.type = Common::EVENT_KEYUP;
		} else if (Z_BUTTON(newButtons) && !Z_BUTTON(oldButtons)) { // Pressed Z - Left Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONDOWN;
		} else if (!Z_BUTTON(newButtons) && Z_BUTTON(oldButtons)) { // Released Z
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONUP;
		}

		oldButtons = newButtons; // Save current button status

		if (buttonPressed) {
			event.mouse.x = _mouseX;
			event.mouse.y = _mouseY;
			return true;
		}
	}

	if (newMouseButtons != oldMouseButtons) { // Check mouse button press
		if (B_BUTTON(newMouseButtons) && !B_BUTTON(oldMouseButtons)) { // Pressed Right Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONDOWN;
		} else if (!B_BUTTON(newMouseButtons) && B_BUTTON(oldMouseButtons)) { // Released RMB
			buttonPressed = true;
			event.type = Common::EVENT_RBUTTONUP;
		} else if (A_BUTTON(newMouseButtons) && !A_BUTTON(oldMouseButtons)) { // Pressed Left Mouse Button
			buttonPressed = true;
			event.type = Common::EVENT_LBUTTONDOWN;
		} else if (!A_BUTTON(newMouseButtons) && A_BUTTON(oldMouseButtons)) { // Released LMB
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

