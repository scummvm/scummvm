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
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-mapper.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-graphics-surface.h"
#ifdef USE_OPENGL
#include "backends/platform/libretro/include/libretro-graphics-opengl.h"
#endif

void OSystem_libretro::updateMouseXY(float deltaAcc, float *cumulativeXYAcc, int doing_x) {
	int *mouseXY;
	int16 screen_wh;
	int *relMouseXY;
	int cumulativeXYAcc_int;

	if (! deltaAcc)
		return;

	if (_cursorStatus & CURSOR_STATUS_DOING_SLOWER)
		deltaAcc /= retro_setting_get_mouse_fine_control_speed_reduction();

	if (doing_x) {
		_cursorStatus |= CURSOR_STATUS_DOING_X;
		mouseXY = &_mouseX;
		screen_wh = getScreenWidth();
		relMouseXY = &_relMouseX;
	} else {
		_cursorStatus |= CURSOR_STATUS_DOING_Y;
		mouseXY = &_mouseY;
		screen_wh = getScreenHeight();
		relMouseXY = &_relMouseY;
	}
	*cumulativeXYAcc += deltaAcc;
	cumulativeXYAcc_int = (int) * cumulativeXYAcc;
	if (cumulativeXYAcc_int != 0) {
		// Set mouse position
		*mouseXY += cumulativeXYAcc_int;
		*mouseXY = (*mouseXY < 0) ? 0 : *mouseXY;
		*mouseXY = (*mouseXY >= screen_wh) ? screen_wh : *mouseXY;
		// Update accumulator
		*cumulativeXYAcc -= (float)cumulativeXYAcc_int;
	}
	*relMouseXY = (int)deltaAcc;
}

void OSystem_libretro::getMouseXYFromAnalog(bool is_x, int16 coor) {

	int16 sign = (coor > 0) - (coor < 0);
	uint16 abs_coor = abs(coor);
	float *mouseAcc;

	if (abs_coor < retro_setting_get_analog_deadzone()) return;

	_cursorStatus |= CURSOR_STATUS_DOING_JOYSTICK;

	if (is_x) {
		mouseAcc = &_mouseXAcc;
	} else {
		mouseAcc = &_mouseYAcc;
	}

	*mouseAcc = ((*mouseAcc > 0) - (*mouseAcc < 0)) == sign ? *mouseAcc : 0;
	float analog_amplitude = (float)(abs_coor - retro_setting_get_analog_deadzone()) / (float)(ANALOG_RANGE - retro_setting_get_analog_deadzone());

	if (retro_setting_get_analog_response_is_quadratic())
		analog_amplitude *= analog_amplitude;

	updateMouseXY(sign * analog_amplitude * _adjusted_cursor_speed, mouseAcc, is_x);
}

void OSystem_libretro::getMouseXYFromButton(bool is_x, int16 sign) {
	float *dpadVel;
	float *dpadAcc;

	if (is_x) {
		dpadVel = &_dpadXVel;
		dpadAcc = &_dpadXAcc;
	} else {
		dpadVel = &_dpadYVel;
		dpadAcc = &_dpadYAcc;
	}

	if ((*dpadAcc && ((*dpadAcc > 0) - (*dpadAcc < 0)) != sign) || ! sign) {
		*dpadVel = 0.0f;
		*dpadAcc = 0.0f;
	}

	if (! sign)
		return;

	_cursorStatus |= CURSOR_STATUS_DOING_JOYSTICK;

	*dpadVel = MIN(*dpadVel + _inverse_acceleration_time, 1.0f);

	updateMouseXY(sign * *dpadVel * _adjusted_cursor_speed, dpadAcc, is_x);
}

void OSystem_libretro::processInputs(void) {
	int16 x, y;
	float deltaAcc;
	int key_modifiers [3][2] = {{RETROKE_SHIFT_MOD, RETROKMOD_SHIFT}, {RETROKE_CTRL_MOD, RETROKMOD_CTRL}, {RETROKE_ALT_MOD, RETROKMOD_ALT}};
	int key_flags = 0;
	int retropad_value = 0;

	static const uint32 retroButtons[2] = {RETRO_DEVICE_ID_MOUSE_LEFT, RETRO_DEVICE_ID_MOUSE_RIGHT};
	static const Common::EventType eventID[2][2] = {{Common::EVENT_LBUTTONDOWN, Common::EVENT_LBUTTONUP}, {Common::EVENT_RBUTTONDOWN, Common::EVENT_RBUTTONUP}};

	_cursorStatus = 0;

	// Process input from RetroPad
	mapper_poll_device();

	// Reduce cursor speed, if required
	if (retro_get_input_device() == RETRO_DEVICE_JOYPAD && mapper_get_mapper_key_status(RETROKE_FINE_CONTROL)) {
		_cursorStatus |= CURSOR_STATUS_DOING_SLOWER;
	} else {
		_cursorStatus &= ~CURSOR_STATUS_DOING_SLOWER;
	}

	// Handle x,y
	int x_coor_cursor = mapper_get_mapper_key_value(RETROKE_RIGHT) - mapper_get_mapper_key_value(RETROKE_LEFT);
	int y_coor_cursor = mapper_get_mapper_key_value(RETROKE_DOWN) - mapper_get_mapper_key_value(RETROKE_UP);

	if (abs(x_coor_cursor) > 1) {
		getMouseXYFromAnalog(true, x_coor_cursor);
	} else
		getMouseXYFromButton(true, x_coor_cursor);

	if (abs(y_coor_cursor) > 1)
		getMouseXYFromAnalog(false, y_coor_cursor);
	else
		getMouseXYFromButton(false, y_coor_cursor);

	if (_cursorStatus & CURSOR_STATUS_DOING_JOYSTICK) {
		Common::Event ev;
		ev.type = Common::EVENT_MOUSEMOVE;
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		ev.relMouse.x = _cursorStatus & CURSOR_STATUS_DOING_X ? _relMouseX : 0;
		ev.relMouse.y = _cursorStatus & CURSOR_STATUS_DOING_Y ? _relMouseY : 0;
		_events.push_back(ev);
		setMousePosition(_mouseX, _mouseY);
	}

	// Handle special functions
	if (mapper_get_mapper_key_value(RETROKE_SCUMMVM_GUI)) {
		Common::Event ev;
		ev.type = Common::EVENT_MAINMENU;
		_events.push_back(ev);
	}

	if ((mapper_get_mapper_key_status(RETROKE_VKBD) & ((1 << RETRO_DEVICE_KEY_STATUS) | (1 << RETRO_DEVICE_KEY_CHANGED))) == ((1 << RETRO_DEVICE_KEY_STATUS) | (1 << RETRO_DEVICE_KEY_CHANGED))) {
		Common::Event ev;
		ev.type = Common::EVENT_VIRTUAL_KEYBOARD;
		_events.push_back(ev);
	}


	// Handle mouse buttons
	retropad_value = mapper_get_mapper_key_status(RETROKE_LEFT_BUTTON);
	if (retropad_value & (1 << RETRO_DEVICE_KEY_CHANGED)) {
		Common::Event ev;
		ev.type = eventID[0][(retropad_value & (1 << RETRO_DEVICE_KEY_STATUS)) ? 0 : 1];
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	}

	retropad_value = mapper_get_mapper_key_status(RETROKE_RIGHT_BUTTON);
	if (retropad_value & (1 << RETRO_DEVICE_KEY_CHANGED)) {
		Common::Event ev;
		ev.type = eventID[1][(retropad_value & (1 << RETRO_DEVICE_KEY_STATUS)) ? 0 : 1];
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		_events.push_back(ev);
	}

	// Handle keyboard buttons
	for (uint8 i = 0; i < sizeof(key_modifiers) / sizeof(key_modifiers[0]); i++) {
		if (mapper_get_mapper_key_value(key_modifiers[i][0]))
			key_flags |= key_modifiers[i][1];
	}

	for (uint8 i = 0; i < RETRO_DEVICE_ID_JOYPAD_LAST; i++) {
		if (mapper_get_device_key_retro_id(i) <= 0)
			continue;
		retropad_value = mapper_get_device_key_status(i);

		if (retropad_value & (1 << RETRO_DEVICE_KEY_CHANGED)) {
			processKeyEvent((retropad_value & (1 << RETRO_DEVICE_KEY_STATUS)), mapper_get_device_key_scummvm_id(i), 0, key_flags);
		}
	}

	if (retro_setting_get_gamepad_cursor_only())
		return;

#if defined(WIIU) || defined(__SWITCH__)
	int p_x = retro_input_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
	int p_y = retro_input_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);
	int p_press = retro_input_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED);
	int px = (int)((p_x + 0x7fff) * getScreenWidth() / 0xffff);
	int py = (int)((p_y + 0x7fff) * getScreenHeight() / 0xffff);

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
		setMousePosition(_mouseX, _mouseY);
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

	// Process input from physical mouse
	x = retro_input_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
	y = retro_input_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);

	// > X Axis
	if (x != 0) {
		_cursorStatus |= (CURSOR_STATUS_DOING_MOUSE | CURSOR_STATUS_DOING_X);
		if (x > 0) {
			// Reset accumulator when changing direction
			_mouseXAcc = (_mouseXAcc < 0.0) ? 0.0 : _mouseXAcc;
		}
		if (x < 0) {
			// Reset accumulator when changing direction
			_mouseXAcc = (_mouseXAcc > 0.0) ? 0.0 : _mouseXAcc;
		}
		deltaAcc = (float)x * retro_setting_get_mouse_speed();
		updateMouseXY(deltaAcc, &_mouseXAcc, 1);
	}
	// > Y Axis
	if (y != 0) {
		_cursorStatus |= (CURSOR_STATUS_DOING_MOUSE | CURSOR_STATUS_DOING_Y);
		if (y > 0) {
			// Reset accumulator when changing direction
			_mouseYAcc = (_mouseYAcc < 0.0) ? 0.0 : _mouseYAcc;
		}
		if (y < 0) {
			// Reset accumulator when changing direction
			_mouseYAcc = (_mouseYAcc > 0.0) ? 0.0 : _mouseYAcc;
		}
		deltaAcc = (float)y * retro_setting_get_mouse_speed();
		updateMouseXY(deltaAcc, &_mouseYAcc, 0);
	}

	if (_cursorStatus & CURSOR_STATUS_DOING_MOUSE) {
		Common::Event ev;
		ev.type = Common::EVENT_MOUSEMOVE;
		ev.mouse.x = _mouseX;
		ev.mouse.y = _mouseY;
		ev.relMouse.x = _cursorStatus & CURSOR_STATUS_DOING_X ? _relMouseX : 0;
		ev.relMouse.y = _cursorStatus & CURSOR_STATUS_DOING_Y ? _relMouseY : 0;
		_events.push_back(ev);
		setMousePosition(_mouseX, _mouseY);
	}

	for (int i = 0; i < 2; i++) {
		Common::Event ev;
		bool down = retro_input_cb(0, RETRO_DEVICE_MOUSE, 0, retroButtons[i]);
		if (down != _mouseButtons[i]) {
			_mouseButtons[i] = down;
			ev.type = eventID[i][down ? 0 : 1];
			ev.mouse.x = _mouseX;
			ev.mouse.y = _mouseY;
			_events.push_back(ev);
		}
	}
}

void OSystem_libretro::processKeyEvent(bool down, unsigned keycode, uint32 character, uint16 key_modifiers) {
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
