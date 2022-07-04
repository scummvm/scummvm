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
#if 0
  	return mpInput->GetKeyboard()->KeyIsDown(mKey) &&
		   ((mpInput->GetKeyboard()->GetModifier() & mMod) > 0 || mMod == eKeyModifier_NONE);
#endif
	return false;
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
		tsKey.append("Shift ");
	}
	if (mMod & eKeyModifier_ALT) {
		tsKey.append("Alt ");
	}
	if (mMod & eKeyModifier_CTRL) {
		tsKey.append("Control ");
	}
	if (mMod & eKeyModifier_META) {
#ifdef __APPLE__
		tsKey.append("Command ");
#else
		tsKey.append("Windows ");
#endif
	}

	switch (mKey) {
	case eKey_BACKSPACE:
		tsKey.append("BackSpace");
		break;
	case eKey_TAB:
		tsKey.append("Tab");
		break;
	case eKey_CLEAR:
		tsKey.append("Clear");
		break;
	case eKey_RETURN:
		tsKey.append("Return");
		break;
	case eKey_PAUSE:
		tsKey.append("Pause");
		break;
	case eKey_ESCAPE:
		tsKey.append("Escape");
		break;
	case eKey_SPACE:
		tsKey.append("Space");
		break;
	case eKey_EXCLAIM:
		tsKey.append("Exclaim");
		break;
	case eKey_QUOTEDBL:
		tsKey.append("DblQoute");
		break;
	case eKey_HASH:
		tsKey.append("Hash");
		break;
	case eKey_DOLLAR:
		tsKey.append("Dollar");
		break;
	case eKey_AMPERSAND:
		tsKey.append("Ampersand");
		break;
	case eKey_QUOTE:
		tsKey.append("Quote");
		break;
	case eKey_LEFTPAREN:
		tsKey.append("LeftParent");
		break;
	case eKey_RIGHTPAREN:
		tsKey.append("RightParent");
		break;
	case eKey_ASTERISK:
		tsKey.append("Asterisk");
		break;
	case eKey_PLUS:
		tsKey.append("Plus");
		break;
	case eKey_COMMA:
		tsKey.append("Comma");
		break;
	case eKey_MINUS:
		tsKey.append("Minus");
		break;
	case eKey_PERIOD:
		tsKey.append("Period");
		break;
	case eKey_SLASH:
		tsKey.append("Slash");
		break;
	case eKey_0:
		tsKey.append("0");
		break;
	case eKey_1:
		tsKey.append("1");
		break;
	case eKey_2:
		tsKey.append("2");
		break;
	case eKey_3:
		tsKey.append("3");
		break;
	case eKey_4:
		tsKey.append("4");
		break;
	case eKey_5:
		tsKey.append("5");
		break;
	case eKey_6:
		tsKey.append("6");
		break;
	case eKey_7:
		tsKey.append("7");
		break;
	case eKey_8:
		tsKey.append("8");
		break;
	case eKey_9:
		tsKey.append("9");
		break;
	case eKey_COLON:
		tsKey.append("Colon");
		break;
	case eKey_SEMICOLON:
		tsKey.append("SemiColon");
		break;
	case eKey_LESS:
		tsKey.append("Less");
		break;
	case eKey_EQUALS:
		tsKey.append("Equals");
		break;
	case eKey_GREATER:
		tsKey.append("Greater");
		break;
	case eKey_QUESTION:
		tsKey.append("Question");
		break;
	case eKey_AT:
		tsKey.append("At");
		break;
	case eKey_LEFTBRACKET:
		tsKey.append("LeftBracket");
		break;
	case eKey_BACKSLASH:
		tsKey.append("BackSlash");
		break;
	case eKey_RIGHTBRACKET:
		tsKey.append("RightBracket");
		break;
	case eKey_CARET:
		tsKey.append("Caret");
		break;
	case eKey_UNDERSCORE:
		tsKey.append("Underscore");
		break;
	case eKey_BACKQUOTE:
		tsKey.append("BackQuote");
		break;
	case eKey_a:
		tsKey.append("A");
		break;
	case eKey_b:
		tsKey.append("B");
		break;
	case eKey_c:
		tsKey.append("C");
		break;
	case eKey_d:
		tsKey.append("D");
		break;
	case eKey_e:
		tsKey.append("E");
		break;
	case eKey_f:
		tsKey.append("F");
		break;
	case eKey_g:
		tsKey.append("G");
		break;
	case eKey_h:
		tsKey.append("H");
		break;
	case eKey_i:
		tsKey.append("I");
		break;
	case eKey_j:
		tsKey.append("J");
		break;
	case eKey_k:
		tsKey.append("K");
		break;
	case eKey_l:
		tsKey.append("L");
		break;
	case eKey_m:
		tsKey.append("M");
		break;
	case eKey_n:
		tsKey.append("N");
		break;
	case eKey_o:
		tsKey.append("O");
		break;
	case eKey_p:
		tsKey.append("P");
		break;
	case eKey_q:
		tsKey.append("Q");
		break;
	case eKey_r:
		tsKey.append("R");
		break;
	case eKey_s:
		tsKey.append("S");
		break;
	case eKey_t:
		tsKey.append("T");
		break;
	case eKey_u:
		tsKey.append("U");
		break;
	case eKey_v:
		tsKey.append("V");
		break;
	case eKey_w:
		tsKey.append("W");
		break;
	case eKey_x:
		tsKey.append("X");
		break;
	case eKey_y:
		tsKey.append("Y");
		break;
	case eKey_z:
		tsKey.append("Z");
		break;
	case eKey_DELETE:
		tsKey.append("Delete");
		break;
	case eKey_KP0:
		tsKey.append("Kp0");
		break;
	case eKey_KP1:
		tsKey.append("Kp1");
		break;
	case eKey_KP2:
		tsKey.append("Kp2");
		break;
	case eKey_KP3:
		tsKey.append("Kp3");
		break;
	case eKey_KP4:
		tsKey.append("Kp4");
		break;
	case eKey_KP5:
		tsKey.append("Kp5");
		break;
	case eKey_KP6:
		tsKey.append("Kp6");
		break;
	case eKey_KP7:
		tsKey.append("Kp7");
		break;
	case eKey_KP8:
		tsKey.append("Kp8");
		break;
	case eKey_KP9:
		tsKey.append("Kp9");
		break;
	case eKey_KP_PERIOD:
		tsKey.append("Period");
		break;
	case eKey_KP_DIVIDE:
		tsKey.append("Divide");
		break;
	case eKey_KP_MULTIPLY:
		tsKey.append("Multiply");
		break;
	case eKey_KP_MINUS:
		tsKey.append("Minus");
		break;
	case eKey_KP_PLUS:
		tsKey.append("Plus");
		break;
	case eKey_KP_ENTER:
		tsKey.append("Enter");
		break;
	case eKey_KP_EQUALS:
		tsKey.append("Equals");
		break;
	case eKey_UP:
		tsKey.append("Up");
		break;
	case eKey_DOWN:
		tsKey.append("Down");
		break;
	case eKey_RIGHT:
		tsKey.append("Right");
		break;
	case eKey_LEFT:
		tsKey.append("Left");
		break;
	case eKey_INSERT:
		tsKey.append("Insert");
		break;
	case eKey_HOME:
		tsKey.append("Home");
		break;
	case eKey_END:
		tsKey.append("End");
		break;
	case eKey_PAGEUP:
		tsKey.append("PageUp");
		break;
	case eKey_PAGEDOWN:
		tsKey.append("PageDown");
		break;
	case eKey_F1:
		tsKey.append("F1");
		break;
	case eKey_F2:
		tsKey.append("F2");
		break;
	case eKey_F3:
		tsKey.append("F3");
		break;
	case eKey_F4:
		tsKey.append("F4");
		break;
	case eKey_F5:
		tsKey.append("F5");
		break;
	case eKey_F6:
		tsKey.append("F6");
		break;
	case eKey_F7:
		tsKey.append("F7");
		break;
	case eKey_F8:
		tsKey.append("F8");
		break;
	case eKey_F9:
		tsKey.append("F9");
		break;
	case eKey_F10:
		tsKey.append("F10");
		break;
	case eKey_F11:
		tsKey.append("F11");
		break;
	case eKey_F12:
		tsKey.append("F12");
		break;
	case eKey_F13:
		tsKey.append("F13");
		break;
	case eKey_F14:
		tsKey.append("F14");
		break;
	case eKey_F15:
		tsKey.append("F15");
		break;
	case eKey_NUMLOCK:
		tsKey.append("NumLock");
		break;
	case eKey_CAPSLOCK:
		tsKey.append("CapsLock");
		break;
	case eKey_SCROLLOCK:
		tsKey.append("ScrollLock");
		break;
	case eKey_RSHIFT:
		tsKey.append("RightShift");
		break;
	case eKey_LSHIFT:
		tsKey.append("LeftShift");
		break;
	case eKey_RCTRL:
		tsKey.append("RightControl");
		break;
	case eKey_LCTRL:
		tsKey.append("LeftControl");
		break;
	case eKey_RALT:
		tsKey.append("RightAlt");
		break;
	case eKey_LALT:
		tsKey.append("LeftAlt");
		break;
	case eKey_RMETA:
		tsKey.append("RightMeta");
		break;
	case eKey_LMETA:
		tsKey.append("LeftMeta");
		break;
	case eKey_LSUPER:
		tsKey.append("LeftSuper");
		break;
	case eKey_RSUPER:
		tsKey.append("RightSuper");
		break;
	case eKey_MODE:
		tsKey.append("Mode");
		break;
	case eKey_HELP:
		tsKey.append("Help");
		break;
	case eKey_PRINT:
		tsKey.append("Print");
		break;
	case eKey_SYSREQ:
		tsKey.append("SysReq");
		break;
	case eKey_BREAK:
		tsKey.append("Break");
		break;
	case eKey_MENU:
		tsKey.append("Menu");
		break;
	case eKey_POWER:
		tsKey.append("Power");
		break;
	case eKey_EURO:
		tsKey.append("Euro");
		break;
	case eKey_NONE:
		tsKey.append("None");
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
