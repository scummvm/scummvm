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

#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define VK_BACK           0x08
#define VK_TAB            0x09

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14


#define VK_ESCAPE         0x1B

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE


// Some additional virtual keycodes...
#define VK_OEM_COMMA	0xBC
#define VK_OEM_PERIOD	0xBE
#define VK_OEM_SCROLL	0x91
#define VK_OEM_MINUS	0xBD
#define VK_OEM_5_		0x0C
#define VK_OEM_PLUS		0xBB
#define VK_OEM_ALT		0x12

#define VK_SLASH	0xBF
#define VK_BKSLASH	0x5C
#define VK_TILDE	0xC0
#define VK_LBR		0x5B
#define VK_RBR		0x5D


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
	Common::KEYCODE_LCTRL,			VK_LCONTROL,
	Common::KEYCODE_RALT,			VK_RMENU,
	Common::KEYCODE_LALT,			VK_LMENU,
	Common::KEYCODE_RMETA,			VK_RWIN,
	Common::KEYCODE_LMETA,			VK_LWIN,
	Common::KEYCODE_LSUPER,			-1,
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
	VK_CONTROL,

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
