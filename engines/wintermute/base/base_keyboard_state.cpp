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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "common/system.h"
#include "common/keyboard.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseKeyboardState, false)

//////////////////////////////////////////////////////////////////////////
BaseKeyboardState::BaseKeyboardState(BaseGame *inGame) : BaseScriptable(inGame) {
	_currentPrintable = false;
	_currentCharCode = 0;
	_currentKeyData = 0;

	_currentShift = false;
	_currentAlt = false;
	_currentControl = false;

	_keyStates = new uint8[323]; // Hardcoded size for the common/keyboard.h enum
	for (int i = 0; i < 323; i++) {
		_keyStates[i] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
BaseKeyboardState::~BaseKeyboardState() {
	delete[] _keyStates;
}

void BaseKeyboardState::handleKeyPress(Common::Event *event) {
	if (event->type == Common::EVENT_KEYDOWN) {
		_keyStates[event->kbd.keycode] = true;
	}
}

void BaseKeyboardState::handleKeyRelease(Common::Event *event) {
	if (event->type == Common::EVENT_KEYUP) {
		_keyStates[event->kbd.keycode] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// IsKeyDown
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "IsKeyDown") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		int vKey;

		if (val->_type == VAL_STRING && strlen(val->getString()) > 0) {
			const char *str = val->getString();
			char temp = str[0];
			if (temp >= 'A' && temp <= 'Z') {
				temp += ('a' - 'A');
			}
			vKey = (int)temp;
		} else {
			vKey = val->getInt();
		}

		bool isDown = _keyStates[vKeyToKeyCode(vKey)];

		stack->pushBool(isDown);
		return STATUS_OK;
	} else {
		return BaseScriptable::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseKeyboardState::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("keyboard");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Key
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Key") {
		if (_currentPrintable) {
			char key[2];
			key[0] = (char)_currentCharCode;
			key[1] = '\0';
			_scValue->setString(key);
		} else {
			_scValue->setString("");
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Printable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Printable") {
		_scValue->setBool(_currentPrintable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KeyCode
	//////////////////////////////////////////////////////////////////////////
	else if (name == "KeyCode") {
		_scValue->setInt(_currentCharCode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsShift
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IsShift") {
		_scValue->setBool(_currentShift);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsAlt
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IsAlt") {
		_scValue->setBool(_currentAlt);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsControl
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IsControl") {
		_scValue->setBool(_currentControl);
		return _scValue;
	} else {
		return BaseScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::scSetProperty(const char *name, ScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
	    setName(value->getString());
	    if (_renderer) SetWindowText(_renderer->_window, _name);
	    return STATUS_OK;
	}

	else*/ return BaseScriptable::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *BaseKeyboardState::scToString() {
	return "[keyboard state]";
}


//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::readKey(Common::Event *event) {

	Common::KeyCode code = event->kbd.keycode;

	if (event->type != Common::EVENT_KEYDOWN) {
		_currentCharCode = 0;
		_currentPrintable = false;
	}

	// use ASCII value if key pressed is an alphanumeric or punctuation key
	// keys pressed on numpad are handled in next 2 blocks
	else if (code > Common::KEYCODE_SPACE && code < Common::KEYCODE_DELETE) {
		_currentCharCode = event->kbd.ascii;
		_currentPrintable = true;
	}

	// use ASCII value for numpad '/', '*', '-', '+'
	else if (code >= Common::KEYCODE_KP_DIVIDE && code <= Common::KEYCODE_KP_PLUS) {
		_currentCharCode = event->kbd.ascii;
		_currentPrintable = true;
	}

	// if NumLock is active, use ASCII for numpad keys '0'~'9' and '.'
	// keys pressed on numpad without NumLock are considered as normal keycodes, handled in the next block
	else if ((code >= Common::KEYCODE_KP0 && code <= Common::KEYCODE_KP_PERIOD) && ((event->kbd.flags & Common::KBD_NUM) != 0)) {
		_currentCharCode = event->kbd.ascii;
		_currentPrintable = true;
	}

	// use keyCodeToVKey mapping for all other events
	// some keys are printable from those keys
	else {
		_currentCharCode = keyCodeToVKey(event);
		_currentPrintable = code == Common::KEYCODE_BACKSPACE || 
		                    code == Common::KEYCODE_TAB || 
		                    code == Common::KEYCODE_RETURN || 
		                    code == Common::KEYCODE_KP_ENTER || 
		                    code == Common::KEYCODE_ESCAPE || 
		                    code == Common::KEYCODE_SPACE;
	}

	//_currentKeyData = KeyData;

	_currentControl = isControlDown();
	_currentAlt     = isAltDown();
	_currentShift   = isShiftDown();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::persist(BasePersistenceManager *persistMgr) {
	//if (!persistMgr->getIsSaving()) cleanup();
	BaseScriptable::persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_currentAlt));
	persistMgr->transferUint32(TMEMBER(_currentCharCode));
	persistMgr->transferBool(TMEMBER(_currentControl));
	persistMgr->transferUint32(TMEMBER(_currentKeyData));
	persistMgr->transferBool(TMEMBER(_currentPrintable));
	persistMgr->transferBool(TMEMBER(_currentShift));

	if (!persistMgr->getIsSaving()) {
		_keyStates = new uint8[323]; // Hardcoded size for the common/keyboard.h enum
		for (int i = 0; i < 323; i++) {
			_keyStates[i] = false;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isShiftDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_SHIFT);
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isControlDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_CTRL);
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isAltDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_ALT);
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isCurrentPrintable() const {
	return _currentPrintable;
}

//////////////////////////////////////////////////////////////////////////
enum VKeyCodes {
	kVkBack       = 8,  //printable
	kVkTab        = 9,  //printable
	kVkReturn     = 13, //printable
	kVkPause      = 19,
	kVkEscape     = 27, //printable
	kVkSpace      = 32, //printable

	kVkEnd        = 35,
	kVkHome       = 36,
	kVkLeft       = 37,
	kVkUp         = 38,
	kVkRight      = 39,
	kVkDown       = 40,
	kVkInsert     = 45,

	kVkF1         = 112,
	kVkF2         = 113,
	kVkF3         = 114,
	kVkF4         = 115,
	kVkF5         = 116,
	kVkF6         = 117,
	kVkF7         = 118,
	kVkF8         = 119,
	kVkF9         = 120,
	kVkF10        = 121,
	kVkF11        = 122,
	kVkF12        = 123

};

//////////////////////////////////////////////////////////////////////////
uint32 BaseKeyboardState::keyCodeToVKey(Common::Event *event) {
	// todo
	switch (event->kbd.keycode) {
	case Common::KEYCODE_BACKSPACE:
		return kVkBack;
	case Common::KEYCODE_TAB:
		return kVkTab;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		return kVkReturn;
	case Common::KEYCODE_PAUSE:
		return kVkPause;
	case Common::KEYCODE_ESCAPE:
		return kVkEscape;
	case Common::KEYCODE_SPACE:
		return kVkSpace;
	case Common::KEYCODE_END:
	case Common::KEYCODE_KP1:
		return kVkEnd;
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_KP7:
		return kVkHome;
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		return kVkLeft;
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		return kVkRight;
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		return kVkUp;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		return kVkDown;
	case Common::KEYCODE_INSERT:
	case Common::KEYCODE_KP0:
		return kVkInsert;
	case Common::KEYCODE_F1:
		return kVkF1;
	case Common::KEYCODE_F2:
		return kVkF2;
	case Common::KEYCODE_F3:
		return kVkF3;
	case Common::KEYCODE_F4:
		return kVkF4;
	case Common::KEYCODE_F5:
		return kVkF5;
	case Common::KEYCODE_F6:
		return kVkF6;
	case Common::KEYCODE_F7:
		return kVkF7;
	case Common::KEYCODE_F8:
		return kVkF8;
	case Common::KEYCODE_F9:
		return kVkF9;
	case Common::KEYCODE_F10:
		return kVkF10;
	case Common::KEYCODE_F11:
		return kVkF11;
	case Common::KEYCODE_F12:
		return kVkF12;
	default:
		// check if any non-sticky keys were used, otherwise key is unknown to us
		if ((event->kbd.flags & Common::KBD_NON_STICKY) == 0) {
			warning("Key pressed is not recognized, ASCII returned (%d '%c').", event->kbd.keycode, event->kbd.keycode);
		}
		// return ASCII if no match, since it could be used for typing
		return event->kbd.ascii;
		break;
	}

}

//////////////////////////////////////////////////////////////////////////
Common::KeyCode BaseKeyboardState::vKeyToKeyCode(uint32 vkey) {
	// todo
	switch (vkey) {
	case kVkEscape:
		return Common::KEYCODE_ESCAPE;
	case kVkSpace:
		return Common::KEYCODE_SPACE;
	case kVkHome:
		return Common::KEYCODE_HOME;
	case kVkLeft:
		return Common::KEYCODE_LEFT;
	case kVkRight:
		return Common::KEYCODE_RIGHT;
	case kVkUp:
		return Common::KEYCODE_UP;
	case kVkDown:
		return Common::KEYCODE_DOWN;
	case kVkF1:
		return Common::KEYCODE_F1;
	case kVkF2:
		return Common::KEYCODE_F2;
	case kVkF3:
		return Common::KEYCODE_F3;
	case kVkF4:
		return Common::KEYCODE_F4;
	case kVkF5:
		return Common::KEYCODE_F5;
	case kVkF6:
		return Common::KEYCODE_F6;
	case kVkF7:
		return Common::KEYCODE_F7;
	case kVkF8:
		return Common::KEYCODE_F8;
	case kVkF9:
		return Common::KEYCODE_F9;
	case kVkF10:
		return Common::KEYCODE_F10;
	case kVkF11:
		return Common::KEYCODE_F11;
	case kVkF12:
		return Common::KEYCODE_F12;
	default:
		warning("Unknown VKEY: %d", vkey);
		return (Common::KeyCode)vkey;
		break;
	}

}

} // End of namespace Wintermute
