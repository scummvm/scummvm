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
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cActionKeyboard::cActionKeyboard(tString asName, cInput *apInput, int aKey) : iAction(asName) {
	// Split key and modifier
	mKey = (eKey)(aKey & eKey_MASK);
	mMod = (eKeyModifier)(aKey & eKeyModifier_MASK);
	mpInput = apInput;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cActionKeyboard::IsTriggerd() {
	return mpInput->GetKeyboard()->KeyIsDown(mKey) &&
		   ((mpInput->GetKeyboard()->GetModifier() & mMod) > 0 || mMod == eKeyModifier_NONE);
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
	if (mMod & eKeyModifier_SHIFT) {
		tsKey += "Shift ";
	}
	if (mMod & eKeyModifier_ALT) {
		tsKey += "Alt ";
	}
	if (mMod & eKeyModifier_CTRL) {
		tsKey += "Control ";
	}
	if (mMod & eKeyModifier_META) {
#ifdef __APPLE__
		tsKey += "Command ";
#else
		tsKey += "Windows ";
#endif
	}

	switch (mKey) {
	case eKey_BACKSPACE:
		tsKey += "BackSpace";
		break;
	case eKey_TAB:
		tsKey += "Tab";
		break;
	case eKey_CLEAR:
		tsKey += "Clear";
		break;
	case eKey_RETURN:
		tsKey += "Return";
		break;
	case eKey_PAUSE:
		tsKey += "Pause";
		break;
	case eKey_ESCAPE:
		tsKey += "Escape";
		break;
	case eKey_SPACE:
		tsKey += "Space";
		break;
	case eKey_EXCLAIM:
		tsKey += "Exclaim";
		break;
	case eKey_QUOTEDBL:
		tsKey += "DblQoute";
		break;
	case eKey_HASH:
		tsKey += "Hash";
		break;
	case eKey_DOLLAR:
		tsKey += "Dollar";
		break;
	case eKey_AMPERSAND:
		tsKey += "Ampersand";
		break;
	case eKey_QUOTE:
		tsKey += "Quote";
		break;
	case eKey_LEFTPAREN:
		tsKey += "LeftParent";
		break;
	case eKey_RIGHTPAREN:
		tsKey += "RightParent";
		break;
	case eKey_ASTERISK:
		tsKey += "Asterisk";
		break;
	case eKey_PLUS:
		tsKey += "Plus";
		break;
	case eKey_COMMA:
		tsKey += "Comma";
		break;
	case eKey_MINUS:
		tsKey += "Minus";
		break;
	case eKey_PERIOD:
		tsKey += "Period";
		break;
	case eKey_SLASH:
		tsKey += "Slash";
		break;
	case eKey_0:
		tsKey += "0";
		break;
	case eKey_1:
		tsKey += "1";
		break;
	case eKey_2:
		tsKey += "2";
		break;
	case eKey_3:
		tsKey += "3";
		break;
	case eKey_4:
		tsKey += "4";
		break;
	case eKey_5:
		tsKey += "5";
		break;
	case eKey_6:
		tsKey += "6";
		break;
	case eKey_7:
		tsKey += "7";
		break;
	case eKey_8:
		tsKey += "8";
		break;
	case eKey_9:
		tsKey += "9";
		break;
	case eKey_COLON:
		tsKey += "Colon";
		break;
	case eKey_SEMICOLON:
		tsKey += "SemiColon";
		break;
	case eKey_LESS:
		tsKey += "Less";
		break;
	case eKey_EQUALS:
		tsKey += "Equals";
		break;
	case eKey_GREATER:
		tsKey += "Greater";
		break;
	case eKey_QUESTION:
		tsKey += "Question";
		break;
	case eKey_AT:
		tsKey += "At";
		break;
	case eKey_LEFTBRACKET:
		tsKey += "LeftBracket";
		break;
	case eKey_BACKSLASH:
		tsKey += "BackSlash";
		break;
	case eKey_RIGHTBRACKET:
		tsKey += "RightBracket";
		break;
	case eKey_CARET:
		tsKey += "Caret";
		break;
	case eKey_UNDERSCORE:
		tsKey += "Underscore";
		break;
	case eKey_BACKQUOTE:
		tsKey += "BackQuote";
		break;
	case eKey_a:
		tsKey += "A";
		break;
	case eKey_b:
		tsKey += "B";
		break;
	case eKey_c:
		tsKey += "C";
		break;
	case eKey_d:
		tsKey += "D";
		break;
	case eKey_e:
		tsKey += "E";
		break;
	case eKey_f:
		tsKey += "F";
		break;
	case eKey_g:
		tsKey += "G";
		break;
	case eKey_h:
		tsKey += "H";
		break;
	case eKey_i:
		tsKey += "I";
		break;
	case eKey_j:
		tsKey += "J";
		break;
	case eKey_k:
		tsKey += "K";
		break;
	case eKey_l:
		tsKey += "L";
		break;
	case eKey_m:
		tsKey += "M";
		break;
	case eKey_n:
		tsKey += "N";
		break;
	case eKey_o:
		tsKey += "O";
		break;
	case eKey_p:
		tsKey += "P";
		break;
	case eKey_q:
		tsKey += "Q";
		break;
	case eKey_r:
		tsKey += "R";
		break;
	case eKey_s:
		tsKey += "S";
		break;
	case eKey_t:
		tsKey += "T";
		break;
	case eKey_u:
		tsKey += "U";
		break;
	case eKey_v:
		tsKey += "V";
		break;
	case eKey_w:
		tsKey += "W";
		break;
	case eKey_x:
		tsKey += "X";
		break;
	case eKey_y:
		tsKey += "Y";
		break;
	case eKey_z:
		tsKey += "Z";
		break;
	case eKey_DELETE:
		tsKey += "Delete";
		break;
	case eKey_KP0:
		tsKey += "Kp0";
		break;
	case eKey_KP1:
		tsKey += "Kp1";
		break;
	case eKey_KP2:
		tsKey += "Kp2";
		break;
	case eKey_KP3:
		tsKey += "Kp3";
		break;
	case eKey_KP4:
		tsKey += "Kp4";
		break;
	case eKey_KP5:
		tsKey += "Kp5";
		break;
	case eKey_KP6:
		tsKey += "Kp6";
		break;
	case eKey_KP7:
		tsKey += "Kp7";
		break;
	case eKey_KP8:
		tsKey += "Kp8";
		break;
	case eKey_KP9:
		tsKey += "Kp9";
		break;
	case eKey_KP_PERIOD:
		tsKey += "Period";
		break;
	case eKey_KP_DIVIDE:
		tsKey += "Divide";
		break;
	case eKey_KP_MULTIPLY:
		tsKey += "Multiply";
		break;
	case eKey_KP_MINUS:
		tsKey += "Minus";
		break;
	case eKey_KP_PLUS:
		tsKey += "Plus";
		break;
	case eKey_KP_ENTER:
		tsKey += "Enter";
		break;
	case eKey_KP_EQUALS:
		tsKey += "Equals";
		break;
	case eKey_UP:
		tsKey += "Up";
		break;
	case eKey_DOWN:
		tsKey += "Down";
		break;
	case eKey_RIGHT:
		tsKey += "Right";
		break;
	case eKey_LEFT:
		tsKey += "Left";
		break;
	case eKey_INSERT:
		tsKey += "Insert";
		break;
	case eKey_HOME:
		tsKey += "Home";
		break;
	case eKey_END:
		tsKey += "End";
		break;
	case eKey_PAGEUP:
		tsKey += "PageUp";
		break;
	case eKey_PAGEDOWN:
		tsKey += "PageDown";
		break;
	case eKey_F1:
		tsKey += "F1";
		break;
	case eKey_F2:
		tsKey += "F2";
		break;
	case eKey_F3:
		tsKey += "F3";
		break;
	case eKey_F4:
		tsKey += "F4";
		break;
	case eKey_F5:
		tsKey += "F5";
		break;
	case eKey_F6:
		tsKey += "F6";
		break;
	case eKey_F7:
		tsKey += "F7";
		break;
	case eKey_F8:
		tsKey += "F8";
		break;
	case eKey_F9:
		tsKey += "F9";
		break;
	case eKey_F10:
		tsKey += "F10";
		break;
	case eKey_F11:
		tsKey += "F11";
		break;
	case eKey_F12:
		tsKey += "F12";
		break;
	case eKey_F13:
		tsKey += "F13";
		break;
	case eKey_F14:
		tsKey += "F14";
		break;
	case eKey_F15:
		tsKey += "F15";
		break;
	case eKey_NUMLOCK:
		tsKey += "NumLock";
		break;
	case eKey_CAPSLOCK:
		tsKey += "CapsLock";
		break;
	case eKey_SCROLLOCK:
		tsKey += "ScrollLock";
		break;
	case eKey_RSHIFT:
		tsKey += "RightShift";
		break;
	case eKey_LSHIFT:
		tsKey += "LeftShift";
		break;
	case eKey_RCTRL:
		tsKey += "RightControl";
		break;
	case eKey_LCTRL:
		tsKey += "LeftControl";
		break;
	case eKey_RALT:
		tsKey += "RightAlt";
		break;
	case eKey_LALT:
		tsKey += "LeftAlt";
		break;
	case eKey_RMETA:
		tsKey += "RightMeta";
		break;
	case eKey_LMETA:
		tsKey += "LeftMeta";
		break;
	case eKey_LSUPER:
		tsKey += "LeftSuper";
		break;
	case eKey_RSUPER:
		tsKey += "RightSuper";
		break;
	case eKey_MODE:
		tsKey += "Mode";
		break;
	case eKey_HELP:
		tsKey += "Help";
		break;
	case eKey_PRINT:
		tsKey += "Print";
		break;
	case eKey_SYSREQ:
		tsKey += "SysReq";
		break;
	case eKey_BREAK:
		tsKey += "Break";
		break;
	case eKey_MENU:
		tsKey += "Menu";
		break;
	case eKey_POWER:
		tsKey += "Power";
		break;
	case eKey_EURO:
		tsKey += "Euro";
		break;
	case eKey_NONE:
		tsKey += "None";
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
