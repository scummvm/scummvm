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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Base/BKeyboardState.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "common/system.h"
#include "common/keyboard.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBKeyboardState, false)

//////////////////////////////////////////////////////////////////////////
CBKeyboardState::CBKeyboardState(CBGame *inGame): CBScriptable(inGame) {
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
CBKeyboardState::~CBKeyboardState() {
	delete[] _keyStates;
}

void CBKeyboardState::handleKeyPress(Common::Event *event) {
	if (event->type == Common::EVENT_KEYDOWN) {
		_keyStates[event->kbd.keycode] = true;
	}
}

void CBKeyboardState::handleKeyRelease(Common::Event *event) {
	if (event->type == Common::EVENT_KEYUP) {
		_keyStates[event->kbd.keycode] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// IsKeyDown
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "IsKeyDown") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		int vKey;

		if (val->_type == VAL_STRING && strlen(val->GetString()) > 0) {
			const char *str = val->GetString();
			char temp = str[0];
			if (temp >= 'A' && temp <= 'Z') temp += ('a' - 'A');
			vKey = (int)temp;
		} else vKey = val->GetInt();

		warning("BKeyboardState doesnt yet have state-support %d", vKey); //TODO;
//		Uint8 *state = SDL_GetKeyboardState(NULL);
//		SDL_Scancode scanCode = SDL_GetScancodeFromKey(VKeyToKeyCode(vKey));
		bool isDown = _keyStates[VKeyToKeyCode(vKey)];

		Stack->PushBool(isDown);
		return S_OK;
	}

	else return CBScriptable::scCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBKeyboardState::scGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("keyboard");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Key
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Key") == 0) {
		if (_currentPrintable) {
			char key[2];
			key[0] = (char)_currentCharCode;
			key[1] = '\0';
			_scValue->SetString(key);
		} else _scValue->SetString("");

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Printable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Printable") == 0) {
		_scValue->SetBool(_currentPrintable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KeyCode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "KeyCode") == 0) {
		_scValue->SetInt(_currentCharCode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsShift
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsShift") == 0) {
		_scValue->SetBool(_currentShift);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsAlt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsAlt") == 0) {
		_scValue->SetBool(_currentAlt);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsControl
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsControl") == 0) {
		_scValue->SetBool(_currentControl);
		return _scValue;
	}

	else return CBScriptable::scGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::scSetProperty(const char *Name, CScValue *Value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
	    SetName(Value->GetString());
	    if (_renderer) SetWindowText(_renderer->_window, _name);
	    return S_OK;
	}

	else*/ return CBScriptable::scSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBKeyboardState::scToString() {
	return "[keyboard state]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::ReadKey(Common::Event *event) {
	//_currentPrintable = (event->type == SDL_TEXTINPUT); // TODO
	_currentCharCode = KeyCodeToVKey(event);
	if ((_currentCharCode <= Common::KEYCODE_z && _currentCharCode >= Common::KEYCODE_a) ||
	        (_currentCharCode <= Common::KEYCODE_9 && _currentCharCode >= Common::KEYCODE_0)) {
		_currentPrintable = true;
	} else {
		_currentPrintable = false;
	}
	//_currentKeyData = KeyData;

	_currentControl = IsControlDown();
	_currentAlt     = IsAltDown();
	_currentShift   = IsShiftDown();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::persist(CBPersistMgr *persistMgr) {
	//if(!persistMgr->_saving) Cleanup();
	CBScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_currentAlt));
	persistMgr->transfer(TMEMBER(_currentCharCode));
	persistMgr->transfer(TMEMBER(_currentControl));
	persistMgr->transfer(TMEMBER(_currentKeyData));
	persistMgr->transfer(TMEMBER(_currentPrintable));
	persistMgr->transfer(TMEMBER(_currentShift));

	if (!persistMgr->_saving) {
		_keyStates = new uint8[323]; // Hardcoded size for the common/keyboard.h enum
		for (int i = 0; i < 323; i++) {
			_keyStates[i] = false;
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBKeyboardState::IsShiftDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_SHIFT);
}

//////////////////////////////////////////////////////////////////////////
bool CBKeyboardState::IsControlDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_CTRL);
}

//////////////////////////////////////////////////////////////////////////
bool CBKeyboardState::IsAltDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_ALT);
}

//////////////////////////////////////////////////////////////////////////
uint32 CBKeyboardState::KeyCodeToVKey(Common::Event *event) {
	if (event->type != Common::EVENT_KEYDOWN) return 0;

	switch (event->kbd.keycode) {
	case Common::KEYCODE_KP_ENTER:
		return Common::KEYCODE_RETURN;
	default:
		return (uint32)event->kbd.keycode;
	}
}

//////////////////////////////////////////////////////////////////////////
Common::KeyCode CBKeyboardState::VKeyToKeyCode(uint32 vkey) {
	// todo
	return (Common::KeyCode)vkey;
}

} // end of namespace WinterMute
