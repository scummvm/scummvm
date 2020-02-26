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

#ifndef ULTIMA8_KERNEL_HIDKEYS_H
#define ULTIMA8_KERNEL_HIDKEYS_H

#include "common/events.h"

namespace Ultima {
namespace Ultima8 {

enum HID_Key {
	HID_FIRST = 0,

	HID_BACKSPACE = '\b',
	HID_TAB = '\t',
	HID_ENTER = '\n',

	HID_SPACE = ' ',
	HID_EXCLAIM = '!',
	HID_DOUBLEQUOTE = '\"',
	HID_HASH = '#',
	HID_DOLLAR = '$',
	HID_PERCENT = '%',
	HID_AMPERSAND = '&',
	HID_QUOTE = '\'',
	HID_LEFTPAREN = '(',
	HID_RIGHTPAREN = ')',
	HID_ASTERISK = '*',
	HID_PLUS = '+',
	HID_COMMA = ',',
	HID_MINUS = '-',
	HID_PERIOD = '.',
	HID_SLASH = '/',

	HID_0 = '0',
	HID_1 = '1',
	HID_2 = '2',
	HID_3 = '3',
	HID_4 = '4',
	HID_5 = '5',
	HID_6 = '6',
	HID_7 = '7',
	HID_8 = '8',
	HID_9 = '9',

	HID_COLON = ':',
	HID_SEMICOLON = ';',
	HID_LEFTANGLE = '<',
	HID_EQUALS = '=',
	HID_RIGHTANGLE = '>',
	HID_QUESTION = '?',
	HID_AT = '@',

	HID_A = 'A',
	HID_B = 'B',
	HID_C = 'C',
	HID_D = 'D',
	HID_E = 'E',
	HID_F = 'F',
	HID_G = 'G',
	HID_H = 'H',
	HID_I = 'I',
	HID_J = 'J',
	HID_K = 'K',
	HID_L = 'L',
	HID_M = 'M',
	HID_N = 'N',
	HID_O = 'O',
	HID_P = 'P',
	HID_Q = 'Q',
	HID_R = 'R',
	HID_S = 'S',
	HID_T = 'T',
	HID_U = 'U',
	HID_V = 'V',
	HID_W = 'W',
	HID_X = 'X',
	HID_Y = 'Y',
	HID_Z = 'Z',

	HID_LEFTBRACKET = '[',
	HID_BACKSLASH = '\\',
	HID_RIGHTBRACKET = ']',
	HID_CARET = '^',
	HID_UNDERSCORE = '_',
	HID_BACKQUOTE = '`',

	HID_LEFTBRACE = '{',
	HID_VERTICALBAR = '|',
	HID_RIGHTBRACE = '}',
	HID_TILDE = '~',

	/* Arrows + Home/End pad */
	HID_DELETE = 127,
	HID_INSERT,
	HID_END,
	HID_HOME,
	HID_PAGEDOWN,
	HID_PAGEUP,
	HID_UP,
	HID_DOWN,
	HID_LEFT,
	HID_RIGHT,

	/* Numeric keypad */
	HID_KP0,
	HID_KP1,
	HID_KP2,
	HID_KP3,
	HID_KP4,
	HID_KP5,
	HID_KP6,
	HID_KP7,
	HID_KP8,
	HID_KP9,
	HID_KP_PERIOD,
	HID_KP_SLASH,
	HID_KP_ASTERISK,
	HID_KP_MINUS,
	HID_KP_PLUS,
	HID_KP_ENTER,
	HID_KP_EQUALS,

	/* Function keys */
	HID_F1,
	HID_F2,
	HID_F3,
	HID_F4,
	HID_F5,
	HID_F6,
	HID_F7,
	HID_F8,
	HID_F9,
	HID_F10,
	HID_F11,
	HID_F12,
	HID_F13,
	HID_F14,
	HID_F15,

	/* Miscellaneous function keys */
	HID_PRINT,
	HID_NUM_LOCK,
	HID_CAPS_LOCK,
	HID_SCROLL_LOCK,
	HID_PAUSE,
	HID_ESCAPE,

	/* Mouse Buttons */
	HID_MOUSE1,
	HID_MOUSE2,
	HID_MOUSE3,
	HID_MOUSE4,
	HID_MOUSE5,
	HID_MOUSE6,
	HID_MOUSE7,
	HID_MOUSE8,
	HID_MOUSE9,
	HID_MOUSE10,
	HID_MOUSE11,
	HID_MOUSE12,
	HID_MOUSE13,
	HID_MOUSE14,
	HID_MOUSE15,
	HID_MOUSE16,

	/* Joystick Buttons */
	HID_JOY1,
	HID_JOY2,
	HID_JOY3,
	HID_JOY4,
	HID_JOY5,
	HID_JOY6,
	HID_JOY7,
	HID_JOY8,
	HID_JOY9,
	HID_JOY10,
	HID_JOY11,
	HID_JOY12,
	HID_JOY13,
	HID_JOY14,
	HID_JOY15,
	HID_JOY16,

	HID_LAST
};

const char *HID_GetKeyName(HID_Key key);
HID_Key HID_GetKeyFromName(istring &name);

HID_Key HID_translateKey(Common::KeyCode key);
HID_Key HID_translateMouseButton(uint8 button);
HID_Key HID_translateJoystickButton(uint8 button);

enum HID_Event {
	HID_EVENT_DEPRESS = 0,
	HID_EVENT_RELEASE,
	HID_EVENT_DOUBLE,
	HID_EVENT_CLICK,
	HID_EVENT_PREEMPT,
	HID_EVENT_LAST,
	HID_FLAGS_CTRL = 0x100,
	HID_FLAGS_ALT = 0x200,
	HID_FLAGS_SHIFT = 0x400,
	HID_FLAGS_META = 0x800,
	HID_FLAGS_LAST = 0xffff
};
typedef uint16 HID_Events;

HID_Events HID_translateKeyFlags(byte flags);
const char *HID_GetEventsName(HID_Events event);
HID_Events HID_GetEventFromName(const istring &name);

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
