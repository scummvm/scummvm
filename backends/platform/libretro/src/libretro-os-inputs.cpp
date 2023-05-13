/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <libretro.h>
#include "backends/platform/libretro/include/libretro-os.h"

#define ANALOG_RANGE 0x8000
#define BASE_CURSOR_SPEED 4
#define PI 3.141592653589793238

void OSystem_libretro::updateMouseXY(float deltaAcc, float * cumulativeXYAcc, int doing_x){
	int * mouseXY;
	int16 * screen_wh;
	int * relMouseXY;
	int cumulativeXYAcc_int;
	if (doing_x) {
		mouseXY = &_mouseX;
		screen_wh = &_screen.w;
		relMouseXY = &_relMouseX;
	} else {
		mouseXY = &_mouseY;
		screen_wh = &_screen.h;
		relMouseXY = &_relMouseY;
	}
	*cumulativeXYAcc += deltaAcc;
	cumulativeXYAcc_int = (int)*cumulativeXYAcc;
	if (cumulativeXYAcc_int != 0) {
		// Set mouse position
		*mouseXY += cumulativeXYAcc_int;
		*mouseXY = (*mouseXY < 0) ? 0 : *mouseXY;
		*mouseXY = (*mouseXY >= *screen_wh) ? *screen_wh : *mouseXY;
		// Update accumulator
		*cumulativeXYAcc -= (float)cumulativeXYAcc_int;
	}
	*relMouseXY = (int)deltaAcc;
}

void OSystem_libretro::processMouse(retro_input_state_t aCallback, int device, float gampad_cursor_speed, float gamepad_acceleration_time, bool analog_response_is_quadratic, int analog_deadzone, float mouse_speed) {
	enum processMouse_status {
		STATUS_DOING_JOYSTICK  = (1 << 0),
		STATUS_DOING_MOUSE     = (1 << 1),
		STATUS_DOING_X         = (1 << 2),
		STATUS_DOING_Y         = (1 << 3)
	};
	uint8_t status = 0;
	int16_t joy_x, joy_y, joy_rx, joy_ry, x, y;
	float analog_amplitude_x, analog_amplitude_y;
	float deltaAcc;
	bool  down;
	float screen_adjusted_cursor_speed = (float)_screen.w / 320.0f; // Dpad cursor speed should always be based off a 320 wide screen, to keep speeds consistent
	float adjusted_cursor_speed = (float)BASE_CURSOR_SPEED * gampad_cursor_speed * screen_adjusted_cursor_speed;
	float inverse_acceleration_time = (gamepad_acceleration_time > 0.0) ? (1.0 / 60.0) * (1.0 / gamepad_acceleration_time) : 1.0;
	int dpad_cursor_offset;
	double rs_radius, rs_angle;
	unsigned numpad_index;

	static const uint32_t retroButtons[2] = {RETRO_DEVICE_ID_MOUSE_LEFT, RETRO_DEVICE_ID_MOUSE_RIGHT};
	static const Common::EventType eventID[2][2] = {{Common::EVENT_LBUTTONDOWN, Common::EVENT_LBUTTONUP}, {Common::EVENT_RBUTTONDOWN, Common::EVENT_RBUTTONUP}};

	static const unsigned gampad_key_map[8][4] = {
		{RETRO_DEVICE_ID_JOYPAD_X, (unsigned)Common::KEYCODE_ESCAPE, (unsigned)Common::ASCII_ESCAPE, 0},           // Esc
		{RETRO_DEVICE_ID_JOYPAD_Y, (unsigned)Common::KEYCODE_PERIOD, 46, 0},                                       // .
		{RETRO_DEVICE_ID_JOYPAD_L, (unsigned)Common::KEYCODE_RETURN, (unsigned)Common::ASCII_RETURN, 0},           // Enter
		{RETRO_DEVICE_ID_JOYPAD_R, (unsigned)Common::KEYCODE_KP5, 53, 0},                                          // Numpad 5
		{RETRO_DEVICE_ID_JOYPAD_L2, (unsigned)Common::KEYCODE_BACKSPACE, (unsigned)Common::ASCII_BACKSPACE, 0},    // Backspace
		{RETRO_DEVICE_ID_JOYPAD_L3, (unsigned)Common::KEYCODE_F10, (unsigned)Common::ASCII_F10, 0},                // F10
		{RETRO_DEVICE_ID_JOYPAD_R3, (unsigned)Common::KEYCODE_KP0, 48, 0},                                         // Numpad 0
		{RETRO_DEVICE_ID_JOYPAD_SELECT, (unsigned)Common::KEYCODE_F7, (unsigned)Common::ASCII_F7, RETROKMOD_CTRL}, // CTRL+F7 (virtual keyboard)
	};

	// Right stick circular wrap around: 1 -> 2 -> 3 -> 6 -> 9 -> 8 -> 7 -> 4
	static const unsigned gampad_numpad_map[8][2] = {
		{(unsigned)Common::KEYCODE_KP1, 49},
		{(unsigned)Common::KEYCODE_KP2, 50},
		{(unsigned)Common::KEYCODE_KP3, 51},
		{(unsigned)Common::KEYCODE_KP6, 54},
		{(unsigned)Common::KEYCODE_KP9, 57},
		{(unsigned)Common::KEYCODE_KP8, 56},
		{(unsigned)Common::KEYCODE_KP7, 55},
		{(unsigned)Common::KEYCODE_KP4, 52},
	};

	// Reduce gamepad cursor speed, if required
	if (device == RETRO_DEVICE_JOYPAD && aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2)) {
		adjusted_cursor_speed = adjusted_cursor_speed * (1.0f / 5.0f);
	}

	status = 0;
	x = aCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
	y = aCallback(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
	joy_x = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);
	joy_y = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);

	// Left Analog X Axis
	if (joy_x > analog_deadzone || joy_x < -analog_deadzone) {
		status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_X);
		if (joy_x > analog_deadzone) {
			// Reset accumulator when changing direction
			_mouseXAcc = (_mouseXAcc < 0.0) ? 0.0 : _mouseXAcc;
			joy_x = joy_x - analog_deadzone;
		}
		if (joy_x < -analog_deadzone) {
			// Reset accumulator when changing direction
			_mouseXAcc = (_mouseXAcc > 0.0) ? 0.0 : _mouseXAcc;
			joy_x = joy_x + analog_deadzone;
		}
		// Update accumulator
		analog_amplitude_x = (float)joy_x / (float)(ANALOG_RANGE - analog_deadzone);
		if (analog_response_is_quadratic) {
			if (analog_amplitude_x < 0.0)
				analog_amplitude_x = -(analog_amplitude_x * analog_amplitude_x);
			else
				analog_amplitude_x = analog_amplitude_x * analog_amplitude_x;
		}
		// printf("analog_amplitude_x: %f\n", analog_amplitude_x);
		deltaAcc = analog_amplitude_x * adjusted_cursor_speed;
		updateMouseXY(deltaAcc, &_mouseXAcc, 1);
	}

	// Left Analog Y Axis
	if (joy_y > analog_deadzone || joy_y < -analog_deadzone) {
		status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_Y);
		if (joy_y > analog_deadzone) {
			// Reset accumulator when changing direction
			_mouseYAcc = (_mouseYAcc < 0.0) ? 0.0 : _mouseYAcc;
			joy_y = joy_y - analog_deadzone;
		}
		if (joy_y < -analog_deadzone) {
			// Reset accumulator when changing direction
			_mouseYAcc = (_mouseYAcc > 0.0) ? 0.0 : _mouseYAcc;
			joy_y = joy_y + analog_deadzone;
		}
		// Update accumulator
		analog_amplitude_y = (float)joy_y / (float)(ANALOG_RANGE - analog_deadzone);
		if (analog_response_is_quadratic) {
			if (analog_amplitude_y < 0.0)
				analog_amplitude_y = -(analog_amplitude_y * analog_amplitude_y);
			else
				analog_amplitude_y = analog_amplitude_y * analog_amplitude_y;
		}
		// printf("analog_amplitude_y: %f\n", analog_amplitude_y);
		deltaAcc = analog_amplitude_y * adjusted_cursor_speed;
		updateMouseXY(deltaAcc, &_mouseYAcc, 0);
	}

	if (device == RETRO_DEVICE_JOYPAD) {
		bool dpadLeft = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
		bool dpadRight = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
		bool dpadUp = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
		bool dpadDown = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);

		if (dpadLeft || dpadRight) {
			status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_X);
			_dpadXVel = MIN(_dpadXVel + inverse_acceleration_time, 1.0f);

			if (dpadLeft) {
				deltaAcc = -(_dpadXVel * adjusted_cursor_speed);
				_dpadXAcc = _dpadXAcc < deltaAcc ? _dpadXAcc : 0.0f;
			} else { //dpadRight
				deltaAcc = _dpadXVel * adjusted_cursor_speed;
				_dpadXAcc = _dpadXAcc > deltaAcc ? _dpadXAcc : 0.0f;
			}

			updateMouseXY(deltaAcc, &_dpadXAcc, 1);
		} else {
			_dpadXVel = 0.0f;
		}


		if (dpadUp || dpadDown) {
			status |= (STATUS_DOING_JOYSTICK | STATUS_DOING_Y);
			_dpadYVel = MIN(_dpadYVel + inverse_acceleration_time, 1.0f);

			if (dpadUp) {
				deltaAcc = -(_dpadYVel * adjusted_cursor_speed);
				_dpadYAcc = _dpadYAcc < deltaAcc ? _dpadYAcc : 0.0f;
			} else { //dpadDown
				deltaAcc = _dpadYVel * adjusted_cursor_speed;
				_dpadYAcc = _dpadYAcc > deltaAcc ? _dpadYAcc : 0.0f;
			}

			updateMouseXY(deltaAcc, &_dpadYAcc, 0);


		} else {
			_dpadYVel = 0.0f;
		}

		if (aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START)) {
			Common::Event ev;
			ev.type = Common::EVENT_MAINMENU;
			_events.push_back(ev);
		}
	}

#if defined(WIIU) || defined(__SWITCH__)
	int p_x = aCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
	int p_y = aCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);
	int p_press = aCallback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED);
	int px = (int)((p_x + 0x7fff) * _screen.w / 0xffff);
	int py = (int)((p_y + 0x7fff) * _screen.h / 0xffff);
	// printf("(%d,%d) p:%d\n",px,py,pp);

	static int ptrhold = 0;

	if (p_press)
		ptrhold++;
	else
		ptrhold = 0;

	if (ptrhold > 0) {
		_mouseX = px;
		_mouseY = py;

		Common::Event ev;
		ev.type = Common::EVENT_MOUSEMOVE;
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	}

	if (ptrhold > 10 && _ptrmouseButton == 0) {
		_ptrmouseButton = 1;
		Common::Event ev;
		ev.type = eventID[0][_ptrmouseButton ? 0 : 1];
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	} else if (ptrhold == 0 && _ptrmouseButton == 1) {
		_ptrmouseButton = 0;
		Common::Event ev;
		ev.type = eventID[0][_ptrmouseButton ? 0 : 1];
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	}

#endif

	if (status & STATUS_DOING_JOYSTICK) {
		Common::Event ev;
		ev.type = Common::EVENT_MOUSEMOVE;
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		ev.relMouse.x = status & STATUS_DOING_X ? _relMouseX : 0;
		ev.relMouse.y = status & STATUS_DOING_Y ? _relMouseY : 0;
		_events.push_back(ev);
	}

	// Gampad mouse buttons
	down = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
	if (down != _joypadmouseButtons[0]) {
		_joypadmouseButtons[0] = down;

		Common::Event ev;
		ev.type = eventID[0][down ? 0 : 1];
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	}

	down = aCallback(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
	if (down != _joypadmouseButtons[1]) {
		_joypadmouseButtons[1] = down;

		Common::Event ev;
		ev.type = eventID[1][down ? 0 : 1];
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	}

	// Gamepad keyboard buttons
	for (int i = 0; i < 8; i++) {
		down = aCallback(0, RETRO_DEVICE_JOYPAD, 0, gampad_key_map[i][0]);
		if (down != _joypadkeyboardButtons[i]) {
			_joypadkeyboardButtons[i] = down;
			bool state = down ? true : false;
			processKeyEvent(state, gampad_key_map[i][1], (uint32_t)gampad_key_map[i][2], (uint32_t)gampad_key_map[i][3]);
		}
	}

	// Gamepad right stick numpad emulation
	joy_rx = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_X);
	joy_ry = aCallback(0, RETRO_DEVICE_ANALOG, RETRO_DEVICE_INDEX_ANALOG_RIGHT, RETRO_DEVICE_ID_ANALOG_Y);

	if (joy_rx > analog_deadzone)
		joy_rx = joy_rx - analog_deadzone;
	else if (joy_rx < -analog_deadzone)
		joy_rx = joy_rx + analog_deadzone;
	else
		joy_rx = 0;

	if (joy_ry > analog_deadzone)
		joy_ry = joy_ry - analog_deadzone;
	else if (joy_ry < -analog_deadzone)
		joy_ry = joy_ry + analog_deadzone;
	else
		joy_ry = 0;

	// This is very ugly, but I don't have time to make it nicer...
	if (joy_rx != 0 || joy_ry != 0) {
		analog_amplitude_x = (float)joy_rx / (float)(ANALOG_RANGE - analog_deadzone);
		analog_amplitude_y = (float)joy_ry / (float)(ANALOG_RANGE - analog_deadzone);

		// Convert to polar coordinates: part 1
		rs_radius = sqrt((double)(analog_amplitude_x * analog_amplitude_x) + (double)(analog_amplitude_y * analog_amplitude_y));

		// Check if radius is above threshold
		if (rs_radius > 0.5) {
			// Convert to polar coordinates: part 2
			rs_angle = atan2((double)analog_amplitude_y, (double)analog_amplitude_x);

			// Adjust rotation offset...
			rs_angle = (2.0 * PI) - (rs_angle + PI);
			rs_angle = fmod(rs_angle - (0.125 * PI), 2.0 * PI);
			if (rs_angle < 0)
				rs_angle += 2.0 * PI;

			// Convert angle into numpad key index
			numpad_index = (unsigned)((rs_angle / (2.0 * PI)) * 8.0);
			// Unnecessary safety check...
			numpad_index = (numpad_index > 7) ? 7 : numpad_index;
			// printf("numpad_index: %u\n", numpad_index);

			if (numpad_index != _joypadnumpadLast) {
				// Unset last key, if required
				if (_joypadnumpadActive)
					processKeyEvent(false, gampad_numpad_map[_joypadnumpadLast][0], (uint32_t)gampad_numpad_map[_joypadnumpadLast][1], 0);

				// Set new key
				processKeyEvent(true, gampad_numpad_map[numpad_index][0], (uint32_t)gampad_numpad_map[numpad_index][1], 0);

				_joypadnumpadLast = numpad_index;
				_joypadnumpadActive = true;
			}
		} else if (_joypadnumpadActive) {
			processKeyEvent(false, gampad_numpad_map[_joypadnumpadLast][0], (uint32_t)gampad_numpad_map[_joypadnumpadLast][1], 0);
			_joypadnumpadActive = false;
			_joypadnumpadLast = 8;
		}
	} else if (_joypadnumpadActive) {
		processKeyEvent(false, gampad_numpad_map[_joypadnumpadLast][0], (uint32_t)gampad_numpad_map[_joypadnumpadLast][1], 0);
		_joypadnumpadActive = false;
		_joypadnumpadLast = 8;
	}

	// Process input from physical mouse
	// > X Axis
	if (x != 0) {
		status |= (STATUS_DOING_MOUSE | STATUS_DOING_X);
		if (x > 0) {
			// Reset accumulator when changing direction
			_mouseXAcc = (_mouseXAcc < 0.0) ? 0.0 : _mouseXAcc;
		}
		if (x < 0) {
			// Reset accumulator when changing direction
			_mouseXAcc = (_mouseXAcc > 0.0) ? 0.0 : _mouseXAcc;
		}
		deltaAcc = (float)x * mouse_speed;
		updateMouseXY(deltaAcc, &_mouseXAcc, 1);
	}
	// > Y Axis
	if (y != 0) {
		status |= (STATUS_DOING_MOUSE | STATUS_DOING_Y);
		if (y > 0) {
			// Reset accumulator when changing direction
			_mouseYAcc = (_mouseYAcc < 0.0) ? 0.0 : _mouseYAcc;
		}
		if (y < 0) {
			// Reset accumulator when changing direction
			_mouseYAcc = (_mouseYAcc > 0.0) ? 0.0 : _mouseYAcc;
		}
		deltaAcc = (float)y * mouse_speed;
		updateMouseXY(deltaAcc, &_mouseYAcc, 0);
	}

	if (status & STATUS_DOING_MOUSE) {
		Common::Event ev;
		ev.type = Common::EVENT_MOUSEMOVE;
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		ev.relMouse.x = status & STATUS_DOING_X ? _relMouseX : 0;
		ev.relMouse.y = status & STATUS_DOING_Y ? _relMouseY : 0;
		_events.push_back(ev);
	}

	for (int i = 0; i < 2; i++) {
		Common::Event ev;
		bool down = aCallback(0, RETRO_DEVICE_MOUSE, 0, retroButtons[i]);
		if (down != _mouseButtons[i]) {
			_mouseButtons[i] = down;

			ev.type = eventID[i][down ? 0 : 1];
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		}
	}
}

void OSystem_libretro::processKeyEvent(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers) {
	int _keyflags = 0;
	_keyflags |= (key_modifiers & RETROKMOD_CTRL) ? Common::KBD_CTRL : 0;
	_keyflags |= (key_modifiers & RETROKMOD_ALT) ? Common::KBD_ALT : 0;
	_keyflags |= (key_modifiers & RETROKMOD_SHIFT) ? Common::KBD_SHIFT : 0;
	_keyflags |= (key_modifiers & RETROKMOD_META) ? Common::KBD_META : 0;
	_keyflags |= (key_modifiers & RETROKMOD_CAPSLOCK) ? Common::KBD_CAPS : 0;
	_keyflags |= (key_modifiers & RETROKMOD_NUMLOCK) ? Common::KBD_NUM : 0;
	_keyflags |= (key_modifiers & RETROKMOD_SCROLLOCK) ? Common::KBD_SCRL : 0;

	Common::Event ev;
	ev.type = down ? Common::EVENT_KEYDOWN : Common::EVENT_KEYUP;
	ev.kbd.keycode = (Common::KeyCode)keycode;
	ev.kbd.flags = _keyflags;
	ev.kbd.ascii = keycode;

	/* If shift was down then send upper case letter to engine */
	if (ev.kbd.ascii >= 97 && ev.kbd.ascii <= 122 && (_keyflags & Common::KBD_SHIFT))
		ev.kbd.ascii = ev.kbd.ascii & ~0x20;

	_events.push_back(ev);
}
