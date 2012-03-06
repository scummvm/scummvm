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

#include "dcgf.h"
#include "BKeyboardState.h"
#include "ScValue.h"
#include "ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBKeyboardState, false)

//////////////////////////////////////////////////////////////////////////
CBKeyboardState::CBKeyboardState(CBGame *inGame): CBScriptable(inGame) {
	m_CurrentPrintable = false;
	m_CurrentCharCode = 0;
	m_CurrentKeyData = 0;

	m_CurrentShift = false;
	m_CurrentAlt = false;
	m_CurrentControl = false;
}


//////////////////////////////////////////////////////////////////////////
CBKeyboardState::~CBKeyboardState() {

}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// IsKeyDown
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "IsKeyDown") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		int vKey;

		if (val->m_Type == VAL_STRING && strlen(val->GetString()) > 0) {
			char *str = val->GetString();
			if (str[0] >= 'A' && str[0] <= 'Z') str[0] += ('a' - 'A');
			vKey = (int)str[0];
		} else vKey = val->GetInt();

		Uint8 *state = SDL_GetKeyboardState(NULL);
		SDL_Scancode scanCode = SDL_GetScancodeFromKey(VKeyToKeyCode(vKey));
		bool isDown = state[scanCode] > 0;

		Stack->PushBool(isDown);
		return S_OK;
	}

	else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBKeyboardState::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("keyboard");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Key
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Key") == 0) {
		if (m_CurrentPrintable) {
			char key[2];
			key[0] = (char)m_CurrentCharCode;
			key[1] = '\0';
			m_ScValue->SetString(key);
		} else m_ScValue->SetString("");

		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Printable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Printable") == 0) {
		m_ScValue->SetBool(m_CurrentPrintable);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KeyCode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "KeyCode") == 0) {
		m_ScValue->SetInt(m_CurrentCharCode);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsShift
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsShift") == 0) {
		m_ScValue->SetBool(m_CurrentShift);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsAlt
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsAlt") == 0) {
		m_ScValue->SetBool(m_CurrentAlt);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsControl
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsControl") == 0) {
		m_ScValue->SetBool(m_CurrentControl);
		return m_ScValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::ScSetProperty(char *Name, CScValue *Value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(Name, "Name")==0){
	    SetName(Value->GetString());
	    if(m_Renderer) SetWindowText(m_Renderer->m_Window, m_Name);
	    return S_OK;
	}

	else*/ return CBScriptable::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CBKeyboardState::ScToString() {
	return "[keyboard state]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::ReadKey(SDL_Event *event) {
	m_CurrentPrintable = (event->type == SDL_TEXTINPUT);
	m_CurrentCharCode = KeyCodeToVKey(event);
	//m_CurrentKeyData = KeyData;

	m_CurrentControl = IsControlDown();
	m_CurrentAlt     = IsAltDown();
	m_CurrentShift   = IsShiftDown();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBKeyboardState::Persist(CBPersistMgr *PersistMgr) {
	//if(!PersistMgr->m_Saving) Cleanup();
	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_CurrentAlt));
	PersistMgr->Transfer(TMEMBER(m_CurrentCharCode));
	PersistMgr->Transfer(TMEMBER(m_CurrentControl));
	PersistMgr->Transfer(TMEMBER(m_CurrentKeyData));
	PersistMgr->Transfer(TMEMBER(m_CurrentPrintable));
	PersistMgr->Transfer(TMEMBER(m_CurrentShift));

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CBKeyboardState::IsShiftDown() {
	int mod = SDL_GetModState();
	return (mod & KMOD_LSHIFT) || (mod & KMOD_RSHIFT);
}

//////////////////////////////////////////////////////////////////////////
bool CBKeyboardState::IsControlDown() {
	int mod = SDL_GetModState();
	return (mod & KMOD_LCTRL) || (mod & KMOD_RCTRL);
}

//////////////////////////////////////////////////////////////////////////
bool CBKeyboardState::IsAltDown() {
	int mod = SDL_GetModState();
	return (mod & KMOD_LALT) || (mod & KMOD_RALT);
}

//////////////////////////////////////////////////////////////////////////
uint32 CBKeyboardState::KeyCodeToVKey(SDL_Event *event) {
	if (event->type != SDL_KEYDOWN) return 0;

	switch (event->key.keysym.sym) {
	case SDLK_KP_ENTER:
		return SDLK_RETURN;
	default:
		return event->key.keysym.sym;
	}
}

//////////////////////////////////////////////////////////////////////////
SDL_Keycode CBKeyboardState::VKeyToKeyCode(uint32 vkey) {
	// todo
	return (SDL_Keycode)vkey;
}

} // end of namespace WinterMute
