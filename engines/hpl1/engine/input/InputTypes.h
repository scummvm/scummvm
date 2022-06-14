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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_INPUT_TYPES_H
#define HPL_INPUT_TYPES_H

namespace hpl {

//-------------------------------------------------

enum eInputDeviceType {
	eInputDeviceType_Keyboard,
	eInputDeviceType_Mouse,
	eInputDeviceType_Gamepad,
	eInputDeviceType_LastEnum
};

//-------------------------------------------------

enum eMButton {
	eMButton_Left,
	eMButton_Middle,
	eMButton_Right,
	eMButton_WheelUp,
	eMButton_WheelDown,
	eMButton_6,
	eMButton_7,
	eMButton_8,
	eMButton_9,
	eMButton_LastEnum
};

//-------------------------------------------------

enum eKey {
	eKey_BACKSPACE,
	eKey_TAB,
	eKey_CLEAR,
	eKey_RETURN,
	eKey_PAUSE,
	eKey_ESCAPE,
	eKey_SPACE,
	eKey_EXCLAIM,
	eKey_QUOTEDBL,
	eKey_HASH,
	eKey_DOLLAR,
	eKey_AMPERSAND,
	eKey_QUOTE,
	eKey_LEFTPAREN,
	eKey_RIGHTPAREN,
	eKey_ASTERISK,
	eKey_PLUS,
	eKey_COMMA,
	eKey_MINUS,
	eKey_PERIOD,
	eKey_SLASH,
	eKey_0,
	eKey_1,
	eKey_2,
	eKey_3,
	eKey_4,
	eKey_5,
	eKey_6,
	eKey_7,
	eKey_8,
	eKey_9,
	eKey_COLON,
	eKey_SEMICOLON,
	eKey_LESS,
	eKey_EQUALS,
	eKey_GREATER,
	eKey_QUESTION,
	eKey_AT,
	eKey_LEFTBRACKET,
	eKey_BACKSLASH,
	eKey_RIGHTBRACKET,
	eKey_CARET,
	eKey_UNDERSCORE,
	eKey_BACKQUOTE,
	eKey_a,
	eKey_b,
	eKey_c,
	eKey_d,
	eKey_e,
	eKey_f,
	eKey_g,
	eKey_h,
	eKey_i,
	eKey_j,
	eKey_k,
	eKey_l,
	eKey_m,
	eKey_n,
	eKey_o,
	eKey_p,
	eKey_q,
	eKey_r,
	eKey_s,
	eKey_t,
	eKey_u,
	eKey_v,
	eKey_w,
	eKey_x,
	eKey_y,
	eKey_z,
	eKey_DELETE,
	eKey_KP0,
	eKey_KP1,
	eKey_KP2,
	eKey_KP3,
	eKey_KP4,
	eKey_KP5,
	eKey_KP6,
	eKey_KP7,
	eKey_KP8,
	eKey_KP9,
	eKey_KP_PERIOD,
	eKey_KP_DIVIDE,
	eKey_KP_MULTIPLY,
	eKey_KP_MINUS,
	eKey_KP_PLUS,
	eKey_KP_ENTER,
	eKey_KP_EQUALS,
	eKey_UP,
	eKey_DOWN,
	eKey_RIGHT,
	eKey_LEFT,
	eKey_INSERT,
	eKey_HOME,
	eKey_END,
	eKey_PAGEUP,
	eKey_PAGEDOWN,
	eKey_F1,
	eKey_F2,
	eKey_F3,
	eKey_F4,
	eKey_F5,
	eKey_F6,
	eKey_F7,
	eKey_F8,
	eKey_F9,
	eKey_F10,
	eKey_F11,
	eKey_F12,
	eKey_F13,
	eKey_F14,
	eKey_F15,
	eKey_NUMLOCK,
	eKey_CAPSLOCK,
	eKey_SCROLLOCK,
	eKey_RSHIFT,
	eKey_LSHIFT,
	eKey_RCTRL,
	eKey_LCTRL,
	eKey_RALT,
	eKey_LALT,
	eKey_RMETA,
	eKey_LMETA,
	eKey_LSUPER,
	eKey_RSUPER,
	eKey_MODE,
	eKey_HELP,
	eKey_PRINT,
	eKey_SYSREQ,
	eKey_BREAK,
	eKey_MENU,
	eKey_POWER,
	eKey_EURO,
	eKey_NONE,
	eKey_LastEnum
};
#define eKey_MASK 0x000ff
//-------------------------------------------------

typedef unsigned int eKeyModifier;

enum eKeyModifier_Enums {
	eKeyModifier_NONE = 0x00000,
	eKeyModifier_CTRL = 0x00100,
	eKeyModifier_SHIFT = 0x00200,
	eKeyModifier_ALT = 0x00400,
	eKeyModifier_META = 0x00800,
	eKeyModifier_LastEnum = 5
};
#define eKeyModifier_MASK 0x00f00

//-------------------------------------------------

struct cKeyPress {
	cKeyPress() {}
	cKeyPress(eKey aKey, int alUnicode, int alModifier)
		: mKey(aKey), mlUnicode(alUnicode), mlModifier(alModifier) {}

	eKey mKey;
	int mlUnicode;
	int mlModifier;
};

}; // namespace hpl

#endif // HPL_INPUT_TYPES_H
