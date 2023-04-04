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

#include "common/keyboard.h"
#include "common/system.h"
#include "hpl1/engine/impl/KeyboardSDL.h"
#include "hpl1/engine/input/LowLevelInput.h"

namespace hpl {

//-----------------------------------------------------------------------

cKeyboardSDL::cKeyboardSDL(iLowLevelInput *apLowLevelInputSDL) : iKeyboard("SDL Portable Keyboard") {
	_lowLevelSystem = apLowLevelInputSDL;

	_downKeys.set_size(eKey_LastEnum);
	_downKeys.clear();
}

//-----------------------------------------------------------------------

static eKey convertKey(int alKey) {
	switch (alKey) {
	case Common::KEYCODE_BACKSPACE:
		return eKey_BACKSPACE;
	case Common::KEYCODE_TAB:
		return eKey_TAB;
	case Common::KEYCODE_CLEAR:
		return eKey_CLEAR;
	case Common::KEYCODE_RETURN:
		return eKey_RETURN;
	case Common::KEYCODE_PAUSE:
		return eKey_PAUSE;
	case Common::KEYCODE_ESCAPE:
		return eKey_ESCAPE;
	case Common::KEYCODE_SPACE:
		return eKey_SPACE;
	case Common::KEYCODE_EXCLAIM:
		return eKey_EXCLAIM;
	case Common::KEYCODE_QUOTEDBL:
		return eKey_QUOTEDBL;
	case Common::KEYCODE_HASH:
		return eKey_HASH;
	case Common::KEYCODE_DOLLAR:
		return eKey_DOLLAR;
	case Common::KEYCODE_AMPERSAND:
		return eKey_AMPERSAND;
	case Common::KEYCODE_QUOTE:
		return eKey_QUOTE;
	case Common::KEYCODE_LEFTPAREN:
		return eKey_LEFTPAREN;
	case Common::KEYCODE_RIGHTPAREN:
		return eKey_RIGHTPAREN;
	case Common::KEYCODE_ASTERISK:
		return eKey_ASTERISK;
	case Common::KEYCODE_PLUS:
		return eKey_PLUS;
	case Common::KEYCODE_COMMA:
		return eKey_COMMA;
	case Common::KEYCODE_MINUS:
		return eKey_MINUS;
	case Common::KEYCODE_PERIOD:
		return eKey_PERIOD;
	case Common::KEYCODE_SLASH:
		return eKey_SLASH;
	case Common::KEYCODE_0:
		return eKey_0;
	case Common::KEYCODE_1:
		return eKey_1;
	case Common::KEYCODE_2:
		return eKey_2;
	case Common::KEYCODE_3:
		return eKey_3;
	case Common::KEYCODE_4:
		return eKey_4;
	case Common::KEYCODE_5:
		return eKey_5;
	case Common::KEYCODE_6:
		return eKey_6;
	case Common::KEYCODE_7:
		return eKey_7;
	case Common::KEYCODE_8:
		return eKey_8;
	case Common::KEYCODE_9:
		return eKey_9;
	case Common::KEYCODE_COLON:
		return eKey_COLON;
	case Common::KEYCODE_SEMICOLON:
		return eKey_SEMICOLON;
	case Common::KEYCODE_LESS:
		return eKey_LESS;
	case Common::KEYCODE_EQUALS:
		return eKey_EQUALS;
	case Common::KEYCODE_GREATER:
		return eKey_GREATER;
	case Common::KEYCODE_QUESTION:
		return eKey_QUESTION;
	case Common::KEYCODE_AT:
		return eKey_AT;
	case Common::KEYCODE_LEFTBRACKET:
		return eKey_LEFTBRACKET;
	case Common::KEYCODE_BACKSLASH:
		return eKey_BACKSLASH;
	case Common::KEYCODE_RIGHTBRACKET:
		return eKey_RIGHTBRACKET;
	case Common::KEYCODE_CARET:
		return eKey_CARET;
	case Common::KEYCODE_UNDERSCORE:
		return eKey_UNDERSCORE;
	case Common::KEYCODE_BACKQUOTE:
		return eKey_BACKQUOTE;
	case Common::KEYCODE_a:
		return eKey_a;
	case Common::KEYCODE_b:
		return eKey_b;
	case Common::KEYCODE_c:
		return eKey_c;
	case Common::KEYCODE_d:
		return eKey_d;
	case Common::KEYCODE_e:
		return eKey_e;
	case Common::KEYCODE_f:
		return eKey_f;
	case Common::KEYCODE_g:
		return eKey_g;
	case Common::KEYCODE_h:
		return eKey_h;
	case Common::KEYCODE_i:
		return eKey_i;
	case Common::KEYCODE_j:
		return eKey_j;
	case Common::KEYCODE_k:
		return eKey_k;
	case Common::KEYCODE_l:
		return eKey_l;
	case Common::KEYCODE_m:
		return eKey_m;
	case Common::KEYCODE_n:
		return eKey_n;
	case Common::KEYCODE_o:
		return eKey_o;
	case Common::KEYCODE_p:
		return eKey_p;
	case Common::KEYCODE_q:
		return eKey_q;
	case Common::KEYCODE_r:
		return eKey_r;
	case Common::KEYCODE_s:
		return eKey_s;
	case Common::KEYCODE_t:
		return eKey_t;
	case Common::KEYCODE_u:
		return eKey_u;
	case Common::KEYCODE_v:
		return eKey_v;
	case Common::KEYCODE_w:
		return eKey_w;
	case Common::KEYCODE_x:
		return eKey_x;
	case Common::KEYCODE_y:
		return eKey_y;
	case Common::KEYCODE_z:
		return eKey_z;
	case Common::KEYCODE_DELETE:
		return eKey_DELETE;
	case Common::KEYCODE_KP0:
		return eKey_KP0;
	case Common::KEYCODE_KP1:
		return eKey_KP1;
	case Common::KEYCODE_KP2:
		return eKey_KP2;
	case Common::KEYCODE_KP3:
		return eKey_KP3;
	case Common::KEYCODE_KP4:
		return eKey_KP4;
	case Common::KEYCODE_KP5:
		return eKey_KP5;
	case Common::KEYCODE_KP6:
		return eKey_KP6;
	case Common::KEYCODE_KP7:
		return eKey_KP7;
	case Common::KEYCODE_KP8:
		return eKey_KP8;
	case Common::KEYCODE_KP9:
		return eKey_KP9;
	case Common::KEYCODE_KP_PERIOD:
		return eKey_KP_PERIOD;
	case Common::KEYCODE_KP_DIVIDE:
		return eKey_KP_DIVIDE;
	case Common::KEYCODE_KP_MULTIPLY:
		return eKey_KP_MULTIPLY;
	case Common::KEYCODE_KP_MINUS:
		return eKey_KP_MINUS;
	case Common::KEYCODE_KP_PLUS:
		return eKey_KP_PLUS;
	case Common::KEYCODE_KP_ENTER:
		return eKey_KP_ENTER;
	case Common::KEYCODE_KP_EQUALS:
		return eKey_KP_EQUALS;
	case Common::KEYCODE_UP:
		return eKey_UP;
	case Common::KEYCODE_DOWN:
		return eKey_DOWN;
	case Common::KEYCODE_RIGHT:
		return eKey_RIGHT;
	case Common::KEYCODE_LEFT:
		return eKey_LEFT;
	case Common::KEYCODE_INSERT:
		return eKey_INSERT;
	case Common::KEYCODE_HOME:
		return eKey_HOME;
	case Common::KEYCODE_END:
		return eKey_END;
	case Common::KEYCODE_PAGEUP:
		return eKey_PAGEUP;
	case Common::KEYCODE_PAGEDOWN:
		return eKey_PAGEDOWN;
	case Common::KEYCODE_F1:
		return eKey_F1;
	case Common::KEYCODE_F2:
		return eKey_F2;
	case Common::KEYCODE_F3:
		return eKey_F3;
	case Common::KEYCODE_F4:
		return eKey_F4;
	case Common::KEYCODE_F5:
		return eKey_F5;
	case Common::KEYCODE_F6:
		return eKey_F6;
	case Common::KEYCODE_F7:
		return eKey_F7;
	case Common::KEYCODE_F8:
		return eKey_F8;
	case Common::KEYCODE_F9:
		return eKey_F9;
	case Common::KEYCODE_F10:
		return eKey_F10;
	case Common::KEYCODE_F11:
		return eKey_F11;
	case Common::KEYCODE_F12:
		return eKey_F12;
	case Common::KEYCODE_F13:
		return eKey_F13;
	case Common::KEYCODE_F14:
		return eKey_F14;
	case Common::KEYCODE_F15:
		return eKey_F15;
	case Common::KEYCODE_NUMLOCK:
		return eKey_NUMLOCK;
	case Common::KEYCODE_CAPSLOCK:
		return eKey_CAPSLOCK;
	case Common::KEYCODE_SCROLLOCK:
		return eKey_SCROLLOCK;
	case Common::KEYCODE_RSHIFT:
		return eKey_RSHIFT;
	case Common::KEYCODE_LSHIFT:
		return eKey_LSHIFT;
	case Common::KEYCODE_RCTRL:
		return eKey_RCTRL;
	case Common::KEYCODE_LCTRL:
		return eKey_LCTRL;
	case Common::KEYCODE_RALT:
		return eKey_RALT;
	case Common::KEYCODE_LALT:
		return eKey_LALT;
	case Common::KEYCODE_RMETA:
		return eKey_RMETA;
	case Common::KEYCODE_LMETA:
		return eKey_LMETA;
	case Common::KEYCODE_LSUPER:
		return eKey_LSUPER;
	case Common::KEYCODE_RSUPER:
		return eKey_RSUPER;
	case Common::KEYCODE_MODE:
		return eKey_MODE;
	case Common::KEYCODE_HELP:
		return eKey_HELP;
	case Common::KEYCODE_PRINT:
		return eKey_PRINT;
	case Common::KEYCODE_SYSREQ:
		return eKey_SYSREQ;
	case Common::KEYCODE_BREAK:
		return eKey_BREAK;
	case Common::KEYCODE_MENU:
		return eKey_MENU;
	case Common::KEYCODE_POWER:
		return eKey_POWER;
	case Common::KEYCODE_EURO:
		return eKey_EURO;
	}
	return eKey_NONE;
}

static eKeyModifier convertModifiers(const int mods) {
	eKeyModifier out = eKeyModifier_NONE;
	if (mods & Common::KBD_CTRL)
		out |= eKeyModifier_CTRL;
	if (mods & Common::KBD_SHIFT)
		out |= eKeyModifier_SHIFT;
	if (mods & Common::KBD_ALT)
		out |= eKeyModifier_ALT;
	if (mods & Common::KBD_META)
		out |= eKeyModifier_META;
	return out;
}

void cKeyboardSDL::processEvent(const Common::Event &ev) {
	if (ev.type != Common::EVENT_KEYDOWN && ev.type != Common::EVENT_KEYUP)
		return;

	eKey key = convertKey(ev.kbd.keycode);
	if (ev.type == Common::EVENT_KEYDOWN) {
		_downKeys.set(key);
		_modifiers = convertModifiers(ev.kbd.flags);
		_pressedKeys.push(cKeyPress(key, ev.kbd.ascii, _modifiers));
	} else
		_downKeys.unset(key);
}

void cKeyboardSDL::Update() {
	_pressedKeys.clear();
	for (const Common::Event &ev : _lowLevelSystem->_events)
		processEvent(ev);
}

//-----------------------------------------------------------------------

bool cKeyboardSDL::KeyIsDown(eKey key) {
	return _downKeys.get(key);
}

//-----------------------------------------------------------------------

cKeyPress cKeyboardSDL::GetKey() {
	return _pressedKeys.pop();
}

//-----------------------------------------------------------------------

bool cKeyboardSDL::KeyIsPressed() {
	return _pressedKeys.empty() == false;
}

//-----------------------------------------------------------------------

eKeyModifier cKeyboardSDL::GetModifier() {
	return _modifiers;
}

//-----------------------------------------------------------------------

tString cKeyboardSDL::KeyToString(eKey) {
	return "None";
}

//-----------------------------------------------------------------------

eKey cKeyboardSDL::StringToKey(tString) {
	return eKey_NONE;
}

//-----------------------------------------------------------------------

} // namespace hpl
