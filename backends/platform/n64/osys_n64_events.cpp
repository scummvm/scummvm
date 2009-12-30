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
 *
 */

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

#define PAD_DEADZONE 5
#define PAD_ACCELERATION 10
#define PAD_CHECK_TIME 40

bool OSystem_N64::pollEvent(Common::Event &event) {
	// Check Timers. Not the best place, but checking in interrupts proved to be unsafe
	checkTimers();

	// Refill audio buffers, doing this inside interrupts could be harmful
	refillAudioBuffers();

	// Read current controller status
	controller_Read_Buttons(_ctrlData);

	static uint16 oldButtons = 0; // old button data... used for button press/release
	uint16 newButtons = _ctrlData->c[0].buttons; // Read from controller 0

	bool buttonPressed = false;
	static bool left_digital = false;
	static bool right_digital = false;
	static bool up_digital = false;
	static bool down_digital = false;

	int8 analogX = (_ctrlData->c[0].throttle >> 8) & 0xFF;
	int8 analogY = (_ctrlData->c[0].throttle >> 0) & 0xFF;

	if (newButtons != oldButtons) {
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

	static uint32 _lastPadCheck = 0;
	uint32 curTime = getMillis();

	if ((curTime - _lastPadCheck) > PAD_CHECK_TIME) {
		_lastPadCheck = curTime;

		int32 mx = _mouseX;
		int32 my = _mouseY;

		if (left_digital || right_digital || up_digital || down_digital) {
			if (left_digital)
				mx -= 5;
			else if (right_digital)
				mx += 5;
			if (up_digital)
				my -= 5;
			else if (down_digital)
				my += 5;
		}

		if (abs(analogX) > PAD_DEADZONE)
			mx += analogX / (PAD_ACCELERATION - (abs(analogX) / 20));

		if (abs(analogY) > PAD_DEADZONE)
			my -= analogY / (PAD_ACCELERATION - (abs(analogY) / 20));

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
			event.mouse.x = _mouseX = mx;
			event.mouse.y = _mouseY = my;

			_dirtyOffscreen = true;

			return true;
		}

	}

	return false;
}

