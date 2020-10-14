/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "engines/grim/lua.h"
#include "engines/grim/grim.h"
#include "engines/grim/debugger.h"

namespace Grim {

// Entries in the system.controls table

const ControlDescriptor controls[] = {
	{ "KEY_ESCAPE", Common::KEYCODE_ESCAPE },
	{ "KEY_1", Common::KEYCODE_1 },
	{ "KEY_2", Common::KEYCODE_2 },
	{ "KEY_3", Common::KEYCODE_3 },
	{ "KEY_4", Common::KEYCODE_4 },
	{ "KEY_5", Common::KEYCODE_5 },
	{ "KEY_6", Common::KEYCODE_6 },
	{ "KEY_7", Common::KEYCODE_7 },
	{ "KEY_8", Common::KEYCODE_8 },
	{ "KEY_9", Common::KEYCODE_9 },
	{ "KEY_0", Common::KEYCODE_0 },
	{ "KEY_MINUS", Common::KEYCODE_MINUS },
	{ "KEY_EQUALS", Common::KEYCODE_EQUALS },
	{ "KEY_BACK", Common::KEYCODE_BACKSPACE },
	{ "KEY_TAB", Common::KEYCODE_TAB },
	{ "KEY_Q", Common::KEYCODE_q },
	{ "KEY_W", Common::KEYCODE_w },
	{ "KEY_E", Common::KEYCODE_e },
	{ "KEY_R", Common::KEYCODE_r },
	{ "KEY_T", Common::KEYCODE_t },
	{ "KEY_Y", Common::KEYCODE_y },
	{ "KEY_U", Common::KEYCODE_u },
	{ "KEY_I", Common::KEYCODE_i },
	{ "KEY_O", Common::KEYCODE_o },
	{ "KEY_P", Common::KEYCODE_p },
	{ "KEY_LBRACKET", Common::KEYCODE_LEFTBRACKET },
	{ "KEY_RBRACKET", Common::KEYCODE_RIGHTBRACKET },
	{ "KEY_RETURN", Common::KEYCODE_RETURN },
	{ "KEY_LCONTROL", Common::KEYCODE_LCTRL },
	{ "KEY_A", Common::KEYCODE_a },
	{ "KEY_S", Common::KEYCODE_s },
	{ "KEY_D", Common::KEYCODE_d },
	{ "KEY_F", Common::KEYCODE_f },
	{ "KEY_G", Common::KEYCODE_g },
	{ "KEY_H", Common::KEYCODE_h },
	{ "KEY_J", Common::KEYCODE_j },
	{ "KEY_K", Common::KEYCODE_k },
	{ "KEY_L", Common::KEYCODE_l },
	{ "KEY_SEMICOLON", Common::KEYCODE_SEMICOLON },
	{ "KEY_APOSTROPHE", Common::KEYCODE_QUOTEDBL },
	{ "KEY_GRAVE", Common::KEYCODE_BACKQUOTE },
	{ "KEY_LSHIFT", Common::KEYCODE_LSHIFT },
	{ "KEY_BACKSLASH", Common::KEYCODE_BACKSLASH },
	{ "KEY_Z", Common::KEYCODE_z },
	{ "KEY_X", Common::KEYCODE_x },
	{ "KEY_C", Common::KEYCODE_c },
	{ "KEY_V", Common::KEYCODE_v },
	{ "KEY_B", Common::KEYCODE_b },
	{ "KEY_N", Common::KEYCODE_n },
	{ "KEY_M", Common::KEYCODE_m },
	{ "KEY_COMMA", Common::KEYCODE_COMMA },
	{ "KEY_PERIOD", Common::KEYCODE_PERIOD },
	{ "KEY_SLASH", Common::KEYCODE_SLASH },
	{ "KEY_RSHIFT", Common::KEYCODE_RSHIFT },
	{ "KEY_MULTIPLY", Common::KEYCODE_ASTERISK },
	{ "KEY_LMENU", Common::KEYCODE_LALT },
	{ "KEY_SPACE", Common::KEYCODE_SPACE },
	{ "KEY_CAPITAL", Common::KEYCODE_CAPSLOCK },
	{ "KEY_F1", Common::KEYCODE_F1 },
	{ "KEY_F2", Common::KEYCODE_F2 },
	{ "KEY_F3", Common::KEYCODE_F3 },
	{ "KEY_F4", Common::KEYCODE_F4 },
	{ "KEY_F5", Common::KEYCODE_F5 },
	{ "KEY_F6", Common::KEYCODE_F6 },
	{ "KEY_F7", Common::KEYCODE_F7 },
	{ "KEY_F8", Common::KEYCODE_F8 },
	{ "KEY_F9", Common::KEYCODE_F9 },
	{ "KEY_F10", Common::KEYCODE_F10 },
	{ "KEY_NUMLOCK", Common::KEYCODE_NUMLOCK },
	{ "KEY_SCROLL", Common::KEYCODE_SCROLLOCK },
	{ "KEY_NUMPAD7", Common::KEYCODE_KP7 },
	{ "KEY_NUMPAD8", Common::KEYCODE_KP8 },
	{ "KEY_NUMPAD9", Common::KEYCODE_KP9 },
	{ "KEY_SUBTRACT", Common::KEYCODE_KP_MINUS },
	{ "KEY_NUMPAD4", Common::KEYCODE_KP4 },
	{ "KEY_NUMPAD5", Common::KEYCODE_KP5 },
	{ "KEY_NUMPAD6", Common::KEYCODE_KP6 },
	{ "KEY_ADD", Common::KEYCODE_KP_PLUS },
	{ "KEY_NUMPAD1", Common::KEYCODE_KP1 },
	{ "KEY_NUMPAD2", Common::KEYCODE_KP2 },
	{ "KEY_NUMPAD3", Common::KEYCODE_KP3 },
	{ "KEY_NUMPAD0", Common::KEYCODE_KP0 },
	{ "KEY_DECIMAL", Common::KEYCODE_KP_PERIOD },
	{ "KEY_F11", Common::KEYCODE_F11 },
	{ "KEY_F12", Common::KEYCODE_F12 },
	{ "KEY_F13", Common::KEYCODE_F13 },
	{ "KEY_F14", Common::KEYCODE_F14 },
	{ "KEY_F15", Common::KEYCODE_F15 },
	{ "KEY_NUMPADEQUALS", Common::KEYCODE_KP_EQUALS },
	{ "KEY_AT", Common::KEYCODE_AT },
	{ "KEY_COLON", Common::KEYCODE_COLON },
	{ "KEY_UNDERLINE", Common::KEYCODE_UNDERSCORE },
	{ "KEY_STOP", Common::KEYCODE_BREAK },
	{ "KEY_NUMPADENTER", Common::KEYCODE_KP_ENTER },
	{ "KEY_RCONTROL", Common::KEYCODE_RCTRL },
	{ "KEY_NUMPADCOMMA", Common::KEYCODE_KP_PERIOD },
	{ "KEY_DIVIDE", Common::KEYCODE_KP_DIVIDE },
	{ "KEY_SYSRQ", Common::KEYCODE_SYSREQ },
	{ "KEY_RMENU", Common::KEYCODE_RALT },
	{ "KEY_HOME", Common::KEYCODE_HOME },
	{ "KEY_UP", Common::KEYCODE_UP },
	{ "KEY_PRIOR", Common::KEYCODE_PAGEUP },
	{ "KEY_LEFT", Common::KEYCODE_LEFT },
	{ "KEY_RIGHT", Common::KEYCODE_RIGHT },
	{ "KEY_END", Common::KEYCODE_END },
	{ "KEY_DOWN", Common::KEYCODE_DOWN },
	{ "KEY_NEXT", Common::KEYCODE_PAGEDOWN },
	{ "KEY_INSERT", Common::KEYCODE_INSERT },
	{ "KEY_DELETE", Common::KEYCODE_DELETE },
	{ "KEY_LWIN", Common::KEYCODE_LSUPER },
	{ "KEY_RWIN", Common::KEYCODE_RSUPER },
	{ "KEY_APPS", Common::KEYCODE_MENU },
	{ "KEY_EQUAL", Common::KEYCODE_EQUALS },
	{ "KEY_CONTROL", Common::KEYCODE_LCTRL },
	{ "KEY_SHIFT", Common::KEYCODE_LSHIFT },
	{ "KEY_ALT", Common::KEYCODE_LALT },
	{ "KEY_BACKSPACE", Common::KEYCODE_BACKSPACE },
	{ "KEY_NP9", Common::KEYCODE_KP9 },
	{ "KEY_NP8", Common::KEYCODE_KP8 },
	{ "KEY_NP7", Common::KEYCODE_KP7 },
	{ "KEY_NP6", Common::KEYCODE_KP6 },
	{ "KEY_NP5", Common::KEYCODE_KP5 },
	{ "KEY_NP4", Common::KEYCODE_KP4 },
	{ "KEY_NP3", Common::KEYCODE_KP3 },
	{ "KEY_NP2", Common::KEYCODE_KP2 },
	{ "KEY_NP1", Common::KEYCODE_KP1 },
	{ "KEY_NP0", Common::KEYCODE_KP0 },
	{ "KEY_NPENTER", Common::KEYCODE_KP_ENTER },
	{ "KEY_PAGEUP", Common::KEYCODE_PAGEUP },
	{ "KEY_PAGEDOWN", Common::KEYCODE_PAGEDOWN },
	{ "KEY_SCROLL", Common::KEYCODE_SCROLLOCK },
	{ "KEY_CAPSLOCK", Common::KEYCODE_CAPSLOCK },
	{ "KEY_LEFTBRACKET", Common::KEYCODE_LEFTBRACKET },
	{ "KEY_RIGHTBRACKET", Common::KEYCODE_RIGHTBRACKET },
	{ "KEY_TILDE", Common::KEYCODE_TILDE },
	{ "KEY_QUOTE", Common::KEYCODE_QUOTE },
	{ "KEY_PAUSE", Common::KEYCODE_PAUSE },
	{ "KEY_CLEAR", Common::KEYCODE_CLEAR },

	{ "KEY_JOY1_B1", KEYCODE_JOY1_A },
	{ "KEY_JOY1_B2", KEYCODE_JOY1_B },
	{ "KEY_JOY1_B3", KEYCODE_JOY1_X },
	{ "KEY_JOY1_B4", KEYCODE_JOY1_Y },
	{ "KEY_JOY1_B5", KEYCODE_JOY1_R1 },
	{ "KEY_JOY1_B6", KEYCODE_JOY1_L1 },
	{ "KEY_JOY1_B7", KEYCODE_JOY1_R2 },
	{ "KEY_JOY1_B8", KEYCODE_JOY1_L2 },
	{ "KEY_JOY1_B9", KEYCODE_JOY1_START },
	{ "KEY_JOY1_B10", KEYCODE_JOY1_BACK },
	{ "KEY_JOY1_B11", KEYCODE_JOY1_RIGHTSTICK },
	{ "KEY_JOY1_B12", KEYCODE_JOY1_LEFTSTICK },
	{ "KEY_JOY1_B13", KEYCODE_JOY1_GUIDE },
	{ "KEY_JOY1_B14", KEYCODE_JOY1_B14 },
	{ "KEY_JOY1_B15", KEYCODE_JOY1_B15 },
	{ "KEY_JOY1_B16", KEYCODE_JOY1_B16 },
	{ "KEY_JOY1_B17", KEYCODE_JOY1_B17 },
	{ "KEY_JOY1_B18", KEYCODE_JOY1_B18 },
	{ "KEY_JOY1_B19", KEYCODE_JOY1_B19 },
	{ "KEY_JOY1_B20", KEYCODE_JOY1_B20 },
	{ "KEY_JOY1_HLEFT", KEYCODE_JOY1_HLEFT },
	{ "KEY_JOY1_HUP", KEYCODE_JOY1_HUP },
	{ "KEY_JOY1_HRIGHT", KEYCODE_JOY1_HRIGHT },
	{ "KEY_JOY1_HDOWN", KEYCODE_JOY1_HDOWN },
	{ "KEY_JOY2_B1", KEYCODE_JOY2_A },
	{ "KEY_JOY2_B2", KEYCODE_JOY2_B },
	{ "KEY_JOY2_B3", KEYCODE_JOY2_X },
	{ "KEY_JOY2_B4", KEYCODE_JOY2_Y },
	{ "KEY_JOY2_B5", KEYCODE_JOY2_R1 },
	{ "KEY_JOY2_B6", KEYCODE_JOY2_L1 },
	{ "KEY_JOY2_B7", KEYCODE_JOY2_R2 },
	{ "KEY_JOY2_B8", KEYCODE_JOY2_L2 },
	{ "KEY_JOY2_B9", KEYCODE_JOY2_START },
	{ "KEY_JOY2_B10", KEYCODE_JOY2_BACK },
	{ "KEY_JOY2_HLEFT", KEYCODE_JOY2_HLEFT },
	{ "KEY_JOY2_HUP", KEYCODE_JOY2_HUP },
	{ "KEY_JOY2_HRIGHT", KEYCODE_JOY2_HRIGHT },
	{ "KEY_JOY2_HDOWN", KEYCODE_JOY2_HDOWN },
	{ "KEY_MOUSE_B1", KEYCODE_MOUSE_B1 },
	{ "KEY_MOUSE_B2", KEYCODE_MOUSE_B2 },
	{ "KEY_MOUSE_B3", KEYCODE_MOUSE_B3 },
	{ "KEY_MOUSE_B4", KEYCODE_MOUSE_B4 },
	{ "KEY_MOUSE_LONG", KEYCODE_MOUSE_B4 }, // TODO: Actually map this
	{ "KEY_MOUSE_PING", KEYCODE_MOUSE_B4 }, // TODO: Actually map this
	{ "AXIS_JOY1_X", KEYCODE_AXIS_JOY1_X },
	{ "AXIS_JOY1_Y", KEYCODE_AXIS_JOY1_Y },
	{ "AXIS_JOY1_Z", KEYCODE_AXIS_JOY1_Z },
	{ "AXIS_JOY1_R", KEYCODE_AXIS_JOY1_R },
	{ "AXIS_JOY1_U", KEYCODE_AXIS_JOY1_U },
	{ "AXIS_JOY1_V", KEYCODE_AXIS_JOY1_V },
	{ "AXIS_JOY2_X", KEYCODE_AXIS_JOY2_X },
	{ "AXIS_JOY2_Y", KEYCODE_AXIS_JOY2_Y },
	{ "AXIS_JOY2_Z", KEYCODE_AXIS_JOY2_Z },
	{ "AXIS_JOY2_R", KEYCODE_AXIS_JOY2_R },
	{ "AXIS_JOY2_U", KEYCODE_AXIS_JOY2_U },
	{ "AXIS_JOY2_V", KEYCODE_AXIS_JOY2_V },
	{ "AXIS_MOUSE_X", KEYCODE_AXIS_MOUSE_X },
	{ "AXIS_MOUSE_Y", KEYCODE_AXIS_MOUSE_Y },
	{ "AXIS_MOUSE_Z", KEYCODE_AXIS_MOUSE_Z },

//PS2
	{ "KEY_JOY1_SQUARE", KEYCODE_JOY1_X },
	{ "KEY_JOY1_TRIANGLE", KEYCODE_JOY1_Y },
	{ "KEY_JOY1_CIRCLE", KEYCODE_JOY1_B },
	{ "KEY_JOY1_X", KEYCODE_JOY1_A },
	{ "KEY_JOY1_R1", KEYCODE_JOY1_R1 },
	{ "KEY_JOY1_L1", KEYCODE_JOY1_L1 },
	{ "KEY_JOY1_R2", KEYCODE_JOY1_R2 },
	{ "KEY_JOY1_L2", KEYCODE_JOY1_L2 },
	{ "KEY_JOY1_START", KEYCODE_JOY1_START },
	{ "KEY_JOY1_SELECT", KEYCODE_JOY1_BACK },
	{ "KEY_JOY1_DPAD_U", KEYCODE_JOY1_HUP },
	{ "KEY_JOY1_DPAD_D", KEYCODE_JOY1_HDOWN },
	{ "KEY_JOY1_DPAD_L", KEYCODE_JOY1_HLEFT },
	{ "KEY_JOY1_DPAD_R", KEYCODE_JOY1_HRIGHT },
	{ "KEY_JOY1_LMUSHROOM", KEYCODE_JOY1_LEFTSTICK },
	{ "KEY_JOY1_RMUSHROOM", KEYCODE_JOY1_RIGHTSTICK },
// Joy2
	{ "KEY_JOY2_SQUARE", KEYCODE_JOY2_X },
	{ "KEY_JOY2_TRIANGLE", KEYCODE_JOY2_Y },
	{ "KEY_JOY2_CIRCLE", KEYCODE_JOY2_B },
	{ "KEY_JOY2_X", KEYCODE_JOY2_A },
	{ "KEY_JOY2_R1", KEYCODE_JOY2_R1 },
	{ "KEY_JOY2_L1", KEYCODE_JOY2_L1 },
	{ "KEY_JOY2_R2", KEYCODE_JOY2_R2 },
	{ "KEY_JOY2_L2", KEYCODE_JOY2_L2 },
	{ "KEY_JOY2_START", KEYCODE_JOY2_START },
	{ "KEY_JOY2_SELECT", KEYCODE_JOY2_BACK },
	{ "KEY_JOY2_DPAD_U", KEYCODE_JOY2_HUP },
	{ "KEY_JOY2_DPAD_D", KEYCODE_JOY2_HDOWN },
	{ "KEY_JOY2_DPAD_L", KEYCODE_JOY2_HLEFT },
	{ "KEY_JOY2_DPAD_R", KEYCODE_JOY2_HRIGHT },
	{ "KEY_JOY2_LMUSHROOM", KEYCODE_JOY2_LEFTSTICK },
	{ "KEY_JOY2_RMUSHROOM", KEYCODE_JOY2_RIGHTSTICK },

// tell EMI that the joystick can be used
	{ "joy_selected", 0 },

	{ nullptr, 0 }
};

// CHAR_KEY tests to see whether a keycode is for
// a "character" handler or a "button" handler
#define CHAR_KEY(k) ((k >= 'a' && k <= 'z') || (k >= 'A' && k <= 'Z') || (k >= '0' && k <= '9') || k == ' ')

void GrimEngine::handleChars(Common::EventType operation, const Common::KeyState &key) {
	if (!CHAR_KEY(key.ascii))
		return;

	char keychar[2];
	keychar[0] = key.ascii;
	keychar[1] = 0;

	LuaObjects objects;
	objects.add(keychar);

	if (!LuaBase::instance()->callback("characterHandler", objects)) {
		error("handleChars: invalid handler");
	}
}

void GrimEngine::handleControls(Common::EventType operation, const Common::KeyState &key) {
	// If we're not supposed to handle the key then don't
	if (!_controlsEnabled[key.keycode])
		return;

	LuaObjects objects;
	objects.add(key.keycode);
	if (operation == Common::EVENT_KEYDOWN) {
		objects.add(1);
		objects.add(1);
	} else {
		objects.addNil();
		objects.add(0);
	}
	objects.add(0);
	if (!LuaBase::instance()->callback("buttonHandler", objects)) {
		error("handleControls: invalid keys handler");
	}

	if (operation == Common::EVENT_KEYDOWN)
		_controlsState[key.keycode] = true;
	else if (operation == Common::EVENT_KEYUP)
		_controlsState[key.keycode] = false;
}

void GrimEngine::handleJoyAxis(byte axis, int16 position) {
	if (axis > NUM_JOY_AXES)
		return;

	int keycode = KEYCODE_AXIS_JOY1_X + axis;
	if (!_controlsEnabled[keycode])
		return;

	float fpos;
	if (position < Common::JOYAXIS_MIN / 2) {
		fpos = -1;
	} else if (position > Common::JOYAXIS_MAX / 2) {
		fpos = 1;
	} else {
		fpos = 0;
	}

	if (fabsf(fpos - _joyAxisPosition[axis]) > 0.1) {
		LuaObjects objects;
		objects.add(keycode);
		objects.add(fpos);
		if (!LuaBase::instance()->callback("axisHandler", objects)) {
			error("handleJoyAxis: invalid joystick handler");
		}
		_joyAxisPosition[axis] = fpos;
	}
}

void GrimEngine::handleMouseAxis(byte axis, int16 position) {
	int keycode = KEYCODE_AXIS_MOUSE_X;
	if (!_controlsEnabled[keycode])
		return;

	LuaObjects objects;
	objects.add(keycode);
	objects.add(position);
	if (!LuaBase::instance()->callback("axisHandler", objects)) {
		error("handleJoyAxis: invalid joystick handler");
	}
}

void GrimEngine::handleJoyButton(Common::EventType operation, byte button) {
	if (button > NUM_JOY_BUTTONS)
		return;

	int keycode = KEYCODE_JOY1_A + button;
	if (!_controlsEnabled[keycode])
		return;

	LuaObjects objects;
	objects.add(keycode);
	if (operation == Common::EVENT_JOYBUTTON_DOWN) {
		objects.add(1);
		objects.add(1);
	} else if (operation == Common::EVENT_JOYBUTTON_UP) {
		objects.addNil();
		objects.add(0);
	}
	objects.add(0);
	if (!LuaBase::instance()->callback("buttonHandler", objects)) {
		error("handleControls: invalid keys handler");
	}

	if (operation == Common::EVENT_JOYBUTTON_DOWN)
		_controlsState[keycode] = true;
	else if (operation == Common::EVENT_JOYBUTTON_UP)
		_controlsState[keycode] = false;
}

} // end of namespace Grim

