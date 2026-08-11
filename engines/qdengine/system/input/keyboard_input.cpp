/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "qdengine/system/input/keyboard_input.h"

namespace QDEngine {

static const int keyMapTable[] = {
	Common::KEYCODE_BACKSPACE,		VK_BACK,
	Common::KEYCODE_TAB,			VK_TAB,
	Common::KEYCODE_CLEAR,			VK_CLEAR,
	Common::KEYCODE_RETURN,			VK_RETURN,
	Common::KEYCODE_PAUSE,			VK_PAUSE,
	Common::KEYCODE_ESCAPE,			VK_ESCAPE,
	Common::KEYCODE_SPACE,			VK_SPACE,
	Common::KEYCODE_EXCLAIM,		'!',
	Common::KEYCODE_QUOTEDBL,		'"',
	Common::KEYCODE_HASH,			'#',
	Common::KEYCODE_DOLLAR,			'$',
	Common::KEYCODE_PERCENT,		'%',
	Common::KEYCODE_AMPERSAND,		'&',
	Common::KEYCODE_QUOTE,			'\'',
	Common::KEYCODE_LEFTPAREN,		'(',
	Common::KEYCODE_RIGHTPAREN,		')',
	Common::KEYCODE_ASTERISK,		'*',
	Common::KEYCODE_PLUS,			'+',
	Common::KEYCODE_COMMA,			',',
	Common::KEYCODE_MINUS,			'-',
	Common::KEYCODE_PERIOD,			'.',
	Common::KEYCODE_SLASH,			VK_SLASH,
	Common::KEYCODE_0,				'0',
	Common::KEYCODE_1,				'1',
	Common::KEYCODE_2,				'2',
	Common::KEYCODE_3,				'3',
	Common::KEYCODE_4,				'4',
	Common::KEYCODE_5,				'5',
	Common::KEYCODE_6,				'6',
	Common::KEYCODE_7,				'7',
	Common::KEYCODE_8,				'8',
	Common::KEYCODE_9,				'9',
	Common::KEYCODE_COLON,			':',
	Common::KEYCODE_SEMICOLON,		';',
	Common::KEYCODE_LESS,			'<',
	Common::KEYCODE_EQUALS,			'=',
	Common::KEYCODE_GREATER,		'>',
	Common::KEYCODE_QUESTION,		'?',
	Common::KEYCODE_AT,				'@',
	Common::KEYCODE_LEFTBRACKET,	VK_LBR,
	Common::KEYCODE_BACKSLASH,		VK_BKSLASH,
	Common::KEYCODE_RIGHTBRACKET,	VK_RBR,
	Common::KEYCODE_CARET,			'^',
	Common::KEYCODE_UNDERSCORE,		'_',
	Common::KEYCODE_BACKQUOTE,		'`',
	Common::KEYCODE_a,				'a',
	Common::KEYCODE_b,				'b',
	Common::KEYCODE_c,				'c',
	Common::KEYCODE_d,				'd',
	Common::KEYCODE_e,				'e',
	Common::KEYCODE_f,				'f',
	Common::KEYCODE_g,				'g',
	Common::KEYCODE_h,				'h',
	Common::KEYCODE_i,				'i',
	Common::KEYCODE_j,				'j',
	Common::KEYCODE_k,				'k',
	Common::KEYCODE_l,				'l',
	Common::KEYCODE_m,				'm',
	Common::KEYCODE_n,				'n',
	Common::KEYCODE_o,				'o',
	Common::KEYCODE_p,				'p',
	Common::KEYCODE_q,				'q',
	Common::KEYCODE_r,				'r',
	Common::KEYCODE_s,				's',
	Common::KEYCODE_t,				't',
	Common::KEYCODE_u,				'u',
	Common::KEYCODE_v,				'v',
	Common::KEYCODE_w,				'w',
	Common::KEYCODE_x,				'x',
	Common::KEYCODE_y,				'y',
	Common::KEYCODE_z,				'z',
	Common::KEYCODE_DELETE,			VK_DELETE,
	Common::KEYCODE_TILDE,			VK_TILDE,

	// Numeric keypad
	Common::KEYCODE_KP0,			VK_NUMPAD0,
	Common::KEYCODE_KP1,			VK_NUMPAD1,
	Common::KEYCODE_KP2,			VK_NUMPAD2,
	Common::KEYCODE_KP3,			VK_NUMPAD3,
	Common::KEYCODE_KP4,			VK_NUMPAD4,
	Common::KEYCODE_KP5,			VK_NUMPAD5,
	Common::KEYCODE_KP6,			VK_NUMPAD6,
	Common::KEYCODE_KP7,			VK_NUMPAD7,
	Common::KEYCODE_KP8,			VK_NUMPAD8,
	Common::KEYCODE_KP9,			VK_NUMPAD9,
	Common::KEYCODE_KP_PERIOD,		VK_DECIMAL,
	Common::KEYCODE_KP_DIVIDE,		VK_DIVIDE,
	Common::KEYCODE_KP_MULTIPLY,	VK_MULTIPLY,
	Common::KEYCODE_KP_MINUS,		VK_SUBTRACT,
	Common::KEYCODE_KP_PLUS,		VK_ADD,
	Common::KEYCODE_KP_ENTER,		-1,
	Common::KEYCODE_KP_EQUALS,		-1,

	// Arrows + Home/End pad
	Common::KEYCODE_UP,				VK_UP,
	Common::KEYCODE_DOWN,			VK_DOWN,
	Common::KEYCODE_RIGHT,			VK_RIGHT,
	Common::KEYCODE_LEFT,			VK_LEFT,
	Common::KEYCODE_INSERT,			VK_INSERT,
	Common::KEYCODE_HOME,			VK_HOME,
	Common::KEYCODE_END,			VK_END,
	Common::KEYCODE_PAGEUP,			VK_PRIOR,
	Common::KEYCODE_PAGEDOWN,		VK_NEXT,

	// Function keys
	Common::KEYCODE_F1,				VK_F1,
	Common::KEYCODE_F2,				VK_F2,
	Common::KEYCODE_F3,				VK_F3,
	Common::KEYCODE_F4,				VK_F4,
	Common::KEYCODE_F5,				VK_F5,
	Common::KEYCODE_F6,				VK_F6,
	Common::KEYCODE_F7,				VK_F7,
	Common::KEYCODE_F8,				VK_F8,
	Common::KEYCODE_F9,				VK_F9,
	Common::KEYCODE_F10,			VK_F10,
	Common::KEYCODE_F11,			VK_F11,
	Common::KEYCODE_F12,			VK_F12,
	Common::KEYCODE_F13,			VK_F13,
	Common::KEYCODE_F14,			VK_F14,
	Common::KEYCODE_F15,			VK_F15,
	Common::KEYCODE_F16,			VK_F16,
	Common::KEYCODE_F17,			VK_F17,
	Common::KEYCODE_F18,			VK_F18,

	// Key state modifier keys
	Common::KEYCODE_NUMLOCK,		VK_NUMLOCK,
	Common::KEYCODE_CAPSLOCK,		VK_CAPITAL,
	Common::KEYCODE_SCROLLOCK,		VK_SCROLL,
	Common::KEYCODE_RSHIFT,			VK_RSHIFT,
	Common::KEYCODE_LSHIFT,			VK_LSHIFT,
	Common::KEYCODE_RCTRL,			VK_RCONTROL,
	Common::KEYCODE_LCTRL,			VK_CONTROL,
	Common::KEYCODE_RALT,			VK_RMENU,
	Common::KEYCODE_LALT,			VK_LMENU,
	Common::KEYCODE_RMETA,			VK_RWIN,
	Common::KEYCODE_LMETA,			VK_LWIN,
	Common::KEYCODE_LSUPER,			VK_LWIN,
	Common::KEYCODE_RSUPER,			-1,
	Common::KEYCODE_MODE,			-1,
	Common::KEYCODE_COMPOSE,		-1,

	// Miscellaneous function keys
	Common::KEYCODE_HELP,			VK_HELP,
	Common::KEYCODE_PRINT,			VK_PRINT,
	Common::KEYCODE_SYSREQ,			-1,
	Common::KEYCODE_BREAK,			-1,
	Common::KEYCODE_MENU,			VK_MENU,
	Common::KEYCODE_POWER,			-1,
	Common::KEYCODE_EURO,			-1,
	Common::KEYCODE_UNDO,			-1,
	Common::KEYCODE_SLEEP,			-1,
	Common::KEYCODE_MUTE,			-1,
	Common::KEYCODE_EJECT,			-1,
	Common::KEYCODE_VOLUMEUP,		-1,
	Common::KEYCODE_VOLUMEDOWN,		-1,
	Common::KEYCODE_LEFTSOFT,		-1,
	Common::KEYCODE_RIGHTSOFT,		-1,
	Common::KEYCODE_CALL,			-1,
	Common::KEYCODE_HANGUP,			-1,
	Common::KEYCODE_CAMERA,			-1,
	Common::KEYCODE_WWW,			-1,
	Common::KEYCODE_MAIL,			-1,
	Common::KEYCODE_CALCULATOR,		-1,
	Common::KEYCODE_CUT,			-1,
	Common::KEYCODE_COPY,			-1,
	Common::KEYCODE_PASTE,			-1,
	Common::KEYCODE_SELECT,			VK_SELECT,
	Common::KEYCODE_CANCEL,			-1,

	// Action keys
	Common::KEYCODE_AC_SEARCH,		-1,
	Common::KEYCODE_AC_HOME,		-1,
	Common::KEYCODE_AC_BACK,		-1,
	Common::KEYCODE_AC_FORWARD,		-1,
	Common::KEYCODE_AC_STOP,		-1,
	Common::KEYCODE_AC_REFRESH,		-1,
	Common::KEYCODE_AC_BOOKMARKS,	-1,

	// Audio keys
	Common::KEYCODE_AUDIONEXT,		-1,
	Common::KEYCODE_AUDIOPREV,		-1,
	Common::KEYCODE_AUDIOSTOP,		-1,
	Common::KEYCODE_AUDIOPLAY,		-1,
	Common::KEYCODE_AUDIOPAUSE,		-1,
	Common::KEYCODE_AUDIOPLAYPAUSE,	-1,
	Common::KEYCODE_AUDIOMUTE,		-1,
	Common::KEYCODE_AUDIOREWIND,	-1,
	Common::KEYCODE_AUDIOFASTFORWARD,	-1,
};

static const int unmappedTable[] {
	VK_LBUTTON,
	VK_RBUTTON,
	VK_CANCEL,
	VK_MBUTTON,

	VK_SHIFT,
	VK_LCONTROL,

	VK_EXECUTE,
	VK_SNAPSHOT,

	VK_APPS,

	VK_SEPARATOR,

	VK_F19,
	VK_F20,
	VK_F21,
	VK_F22,
	VK_F23,
	VK_F24,

	VK_OEM_COMMA,
	VK_OEM_PERIOD,
	VK_OEM_SCROLL,
	VK_OEM_MINUS,
	VK_OEM_5_,
	VK_OEM_PLUS,
	VK_OEM_ALT,
};

static int keyMap[Common::KEYCODE_LAST];
static bool unmapped[256];

keyboardDispatcher::keyboardDispatcher() : _handler(0) {
	for (int i = 0; i < 256; i++)
		_key_states[i] = false;

	memset(unmapped, 0, sizeof(unmapped));
	memset(keyMap, 0, sizeof(keyMap));

	// ASCII
	for (int i = 32; i < 126; i++)
		keyMap[i] = i;

	for (int i = 0; i < ARRAYSIZE(keyMapTable); i += 2)
		keyMap[keyMapTable[i]] = keyMapTable[i + 1];

	for (int i = 0; i < ARRAYSIZE(unmappedTable); i++)
		unmapped[unmappedTable[i]] = true;
}

keyboardDispatcher::~keyboardDispatcher() {
}

keyboardDispatcher *keyboardDispatcher::instance() {
	static keyboardDispatcher dsp;
	return &dsp;
}

bool keyboardDispatcher::handle_event(Common::KeyCode vkey, bool event) {
	if (keyMap[vkey] == -1) {
		debug("keyboardDispatcher::handle_event(%d, %d): key is not mapped", vkey, event);
		return false;
	}
	_key_states[keyMap[vkey]] = event;

	if (_handler)
		return (*_handler)(vkey, event);

	return false;
}

//! Возвращает true, если кнопка с кодом vkey в данный момент нажата.
bool keyboardDispatcher::is_pressed(int vkey) const {
	assert(vkey >= 0 && vkey < 256);

	if (unmapped[vkey])
		warning("keyboardDispatcher::is_pressed(%d): key is not mapped", vkey);

	return _key_states[vkey];
}

} // namespace QDEngine
