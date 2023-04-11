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

#include "hpl1/engine/input/ActionKeyboard.h"
#include "hpl1/engine/input/Input.h"
#include "hpl1/engine/input/Keyboard.h"

#include "hpl1/engine/system/low_level_system.h"
namespace hpl {
//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cActionKeyboard::IsTriggerd() {
	return mpInput->GetKeyboard()->KeyIsDown(_key.keycode) &&
		   ((mpInput->GetKeyboard()->GetModifier() & _key.flags) > 0 || _key.flags == 0);
}
//-----------------------------------------------------------------------

float cActionKeyboard::GetValue() {
	if (IsTriggerd())
		return 1.0;
	else
		return 0.0;
}

//-----------------------------------------------------------------------

tString cActionKeyboard::GetInputName() {
	tString tsKey = "";
	if (_key.flags & Common::KBD_SHIFT) {
		tsKey += "Shift ";
	}
	if (_key.flags & Common::KBD_ALT) {
		tsKey += "Alt ";
	}
	if (_key.flags & Common::KBD_CTRL) {
		tsKey += "Control ";
	}
	if (_key.flags & Common::KBD_META) {
#ifdef __APPLE__
		tsKey += "Command ";
#else
		tsKey += "Windows ";
#endif
	}

	switch (_key.keycode) {
	case Common::KEYCODE_BACKSPACE:
		tsKey += "BackSpace";
		break;
	case Common::KEYCODE_TAB:
		tsKey += "Tab";
		break;
	case Common::KEYCODE_CLEAR:
		tsKey += "Clear";
		break;
	case Common::KEYCODE_RETURN:
		tsKey += "Return";
		break;
	case Common::KEYCODE_PAUSE:
		tsKey += "Pause";
		break;
	case Common::KEYCODE_ESCAPE:
		tsKey += "Escape";
		break;
	case Common::KEYCODE_SPACE:
		tsKey += "Space";
		break;
	case Common::KEYCODE_EXCLAIM:
		tsKey += "Exclaim";
		break;
	case Common::KEYCODE_QUOTEDBL:
		tsKey += "DblQoute";
		break;
	case Common::KEYCODE_HASH:
		tsKey += "Hash";
		break;
	case Common::KEYCODE_DOLLAR:
		tsKey += "Dollar";
		break;
	case Common::KEYCODE_AMPERSAND:
		tsKey += "Ampersand";
		break;
	case Common::KEYCODE_QUOTE:
		tsKey += "Quote";
		break;
	case Common::KEYCODE_LEFTPAREN:
		tsKey += "LeftParent";
		break;
	case Common::KEYCODE_RIGHTPAREN:
		tsKey += "RightParent";
		break;
	case Common::KEYCODE_ASTERISK:
		tsKey += "Asterisk";
		break;
	case Common::KEYCODE_PLUS:
		tsKey += "Plus";
		break;
	case Common::KEYCODE_COMMA:
		tsKey += "Comma";
		break;
	case Common::KEYCODE_MINUS:
		tsKey += "Minus";
		break;
	case Common::KEYCODE_PERIOD:
		tsKey += "Period";
		break;
	case Common::KEYCODE_SLASH:
		tsKey += "Slash";
		break;
	case Common::KEYCODE_0:
		tsKey += "0";
		break;
	case Common::KEYCODE_1:
		tsKey += "1";
		break;
	case Common::KEYCODE_2:
		tsKey += "2";
		break;
	case Common::KEYCODE_3:
		tsKey += "3";
		break;
	case Common::KEYCODE_4:
		tsKey += "4";
		break;
	case Common::KEYCODE_5:
		tsKey += "5";
		break;
	case Common::KEYCODE_6:
		tsKey += "6";
		break;
	case Common::KEYCODE_7:
		tsKey += "7";
		break;
	case Common::KEYCODE_8:
		tsKey += "8";
		break;
	case Common::KEYCODE_9:
		tsKey += "9";
		break;
	case Common::KEYCODE_COLON:
		tsKey += "Colon";
		break;
	case Common::KEYCODE_SEMICOLON:
		tsKey += "SemiColon";
		break;
	case Common::KEYCODE_LESS:
		tsKey += "Less";
		break;
	case Common::KEYCODE_EQUALS:
		tsKey += "Equals";
		break;
	case Common::KEYCODE_GREATER:
		tsKey += "Greater";
		break;
	case Common::KEYCODE_QUESTION:
		tsKey += "Question";
		break;
	case Common::KEYCODE_AT:
		tsKey += "At";
		break;
	case Common::KEYCODE_LEFTBRACKET:
		tsKey += "LeftBracket";
		break;
	case Common::KEYCODE_BACKSLASH:
		tsKey += "BackSlash";
		break;
	case Common::KEYCODE_RIGHTBRACKET:
		tsKey += "RightBracket";
		break;
	case Common::KEYCODE_CARET:
		tsKey += "Caret";
		break;
	case Common::KEYCODE_UNDERSCORE:
		tsKey += "Underscore";
		break;
	case Common::KEYCODE_BACKQUOTE:
		tsKey += "BackQuote";
		break;
	case Common::KEYCODE_a:
		tsKey += "A";
		break;
	case Common::KEYCODE_b:
		tsKey += "B";
		break;
	case Common::KEYCODE_c:
		tsKey += "C";
		break;
	case Common::KEYCODE_d:
		tsKey += "D";
		break;
	case Common::KEYCODE_e:
		tsKey += "E";
		break;
	case Common::KEYCODE_f:
		tsKey += "F";
		break;
	case Common::KEYCODE_g:
		tsKey += "G";
		break;
	case Common::KEYCODE_h:
		tsKey += "H";
		break;
	case Common::KEYCODE_i:
		tsKey += "I";
		break;
	case Common::KEYCODE_j:
		tsKey += "J";
		break;
	case Common::KEYCODE_k:
		tsKey += "K";
		break;
	case Common::KEYCODE_l:
		tsKey += "L";
		break;
	case Common::KEYCODE_m:
		tsKey += "M";
		break;
	case Common::KEYCODE_n:
		tsKey += "N";
		break;
	case Common::KEYCODE_o:
		tsKey += "O";
		break;
	case Common::KEYCODE_p:
		tsKey += "P";
		break;
	case Common::KEYCODE_q:
		tsKey += "Q";
		break;
	case Common::KEYCODE_r:
		tsKey += "R";
		break;
	case Common::KEYCODE_s:
		tsKey += "S";
		break;
	case Common::KEYCODE_t:
		tsKey += "T";
		break;
	case Common::KEYCODE_u:
		tsKey += "U";
		break;
	case Common::KEYCODE_v:
		tsKey += "V";
		break;
	case Common::KEYCODE_w:
		tsKey += "W";
		break;
	case Common::KEYCODE_x:
		tsKey += "X";
		break;
	case Common::KEYCODE_y:
		tsKey += "Y";
		break;
	case Common::KEYCODE_z:
		tsKey += "Z";
		break;
	case Common::KEYCODE_DELETE:
		tsKey += "Delete";
		break;
	case Common::KEYCODE_KP0:
		tsKey += "Kp0";
		break;
	case Common::KEYCODE_KP1:
		tsKey += "Kp1";
		break;
	case Common::KEYCODE_KP2:
		tsKey += "Kp2";
		break;
	case Common::KEYCODE_KP3:
		tsKey += "Kp3";
		break;
	case Common::KEYCODE_KP4:
		tsKey += "Kp4";
		break;
	case Common::KEYCODE_KP5:
		tsKey += "Kp5";
		break;
	case Common::KEYCODE_KP6:
		tsKey += "Kp6";
		break;
	case Common::KEYCODE_KP7:
		tsKey += "Kp7";
		break;
	case Common::KEYCODE_KP8:
		tsKey += "Kp8";
		break;
	case Common::KEYCODE_KP9:
		tsKey += "Kp9";
		break;
	case Common::KEYCODE_KP_PERIOD:
		tsKey += "Period";
		break;
	case Common::KEYCODE_KP_DIVIDE:
		tsKey += "Divide";
		break;
	case Common::KEYCODE_KP_MULTIPLY:
		tsKey += "Multiply";
		break;
	case Common::KEYCODE_KP_MINUS:
		tsKey += "Minus";
		break;
	case Common::KEYCODE_KP_PLUS:
		tsKey += "Plus";
		break;
	case Common::KEYCODE_KP_ENTER:
		tsKey += "Enter";
		break;
	case Common::KEYCODE_KP_EQUALS:
		tsKey += "Equals";
		break;
	case Common::KEYCODE_UP:
		tsKey += "Up";
		break;
	case Common::KEYCODE_DOWN:
		tsKey += "Down";
		break;
	case Common::KEYCODE_RIGHT:
		tsKey += "Right";
		break;
	case Common::KEYCODE_LEFT:
		tsKey += "Left";
		break;
	case Common::KEYCODE_INSERT:
		tsKey += "Insert";
		break;
	case Common::KEYCODE_HOME:
		tsKey += "Home";
		break;
	case Common::KEYCODE_END:
		tsKey += "End";
		break;
	case Common::KEYCODE_PAGEUP:
		tsKey += "PageUp";
		break;
	case Common::KEYCODE_PAGEDOWN:
		tsKey += "PageDown";
		break;
	case Common::KEYCODE_F1:
		tsKey += "F1";
		break;
	case Common::KEYCODE_F2:
		tsKey += "F2";
		break;
	case Common::KEYCODE_F3:
		tsKey += "F3";
		break;
	case Common::KEYCODE_F4:
		tsKey += "F4";
		break;
	case Common::KEYCODE_F5:
		tsKey += "F5";
		break;
	case Common::KEYCODE_F6:
		tsKey += "F6";
		break;
	case Common::KEYCODE_F7:
		tsKey += "F7";
		break;
	case Common::KEYCODE_F8:
		tsKey += "F8";
		break;
	case Common::KEYCODE_F9:
		tsKey += "F9";
		break;
	case Common::KEYCODE_F10:
		tsKey += "F10";
		break;
	case Common::KEYCODE_F11:
		tsKey += "F11";
		break;
	case Common::KEYCODE_F12:
		tsKey += "F12";
		break;
	case Common::KEYCODE_F13:
		tsKey += "F13";
		break;
	case Common::KEYCODE_F14:
		tsKey += "F14";
		break;
	case Common::KEYCODE_F15:
		tsKey += "F15";
		break;
	case Common::KEYCODE_NUMLOCK:
		tsKey += "NumLock";
		break;
	case Common::KEYCODE_CAPSLOCK:
		tsKey += "CapsLock";
		break;
	case Common::KEYCODE_SCROLLOCK:
		tsKey += "ScrollLock";
		break;
	case Common::KEYCODE_RSHIFT:
		tsKey += "RightShift";
		break;
	case Common::KEYCODE_LSHIFT:
		tsKey += "LeftShift";
		break;
	case Common::KEYCODE_RCTRL:
		tsKey += "RightControl";
		break;
	case Common::KEYCODE_LCTRL:
		tsKey += "LeftControl";
		break;
	case Common::KEYCODE_RALT:
		tsKey += "RightAlt";
		break;
	case Common::KEYCODE_LALT:
		tsKey += "LeftAlt";
		break;
	case Common::KEYCODE_RMETA:
		tsKey += "RightMeta";
		break;
	case Common::KEYCODE_LMETA:
		tsKey += "LeftMeta";
		break;
	case Common::KEYCODE_LSUPER:
		tsKey += "LeftSuper";
		break;
	case Common::KEYCODE_RSUPER:
		tsKey += "RightSuper";
		break;
	case Common::KEYCODE_MODE:
		tsKey += "Mode";
		break;
	case Common::KEYCODE_HELP:
		tsKey += "Help";
		break;
	case Common::KEYCODE_PRINT:
		tsKey += "Print";
		break;
	case Common::KEYCODE_SYSREQ:
		tsKey += "SysReq";
		break;
	case Common::KEYCODE_BREAK:
		tsKey += "Break";
		break;
	case Common::KEYCODE_MENU:
		tsKey += "Menu";
		break;
	case Common::KEYCODE_POWER:
		tsKey += "Power";
		break;
	case Common::KEYCODE_EURO:
		tsKey += "Euro";
		break;
	default:
		break;
	}
	if (tsKey != "") {
		return tsKey;
	} else {
		return "Unknown";
	}
}

//-----------------------------------------------------------------------
} // namespace hpl
