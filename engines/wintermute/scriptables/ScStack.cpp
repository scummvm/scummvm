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

#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/BGame.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CScStack, false)

//////////////////////////////////////////////////////////////////////////
CScStack::CScStack(CBGame *inGame): CBBase(inGame) {
	_sP = -1;
}


//////////////////////////////////////////////////////////////////////////
CScStack::~CScStack() {

#if _DEBUG
	//Game->LOG(0, "STAT: Stack size: %d, SP=%d", _values.GetSize(), _sP);
#endif

	for (int i = 0; i < _values.GetSize(); i++) {
		delete _values[i];
	}
	_values.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::Pop() {
	if (_sP < 0) {
		Game->LOG(0, "Fatal: Stack underflow");
		return NULL;
	}

	return _values[_sP--];
}


//////////////////////////////////////////////////////////////////////////
void CScStack::Push(CScValue *Val) {
	_sP++;

	if (_sP < _values.GetSize()) {
		_values[_sP]->Cleanup();
		_values[_sP]->Copy(Val);
	} else {
		CScValue *val = new CScValue(Game);
		val->Copy(Val);
		_values.Add(val);
	}
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::GetPushValue() {
	_sP++;

	if (_sP >= _values.GetSize()) {
		CScValue *val = new CScValue(Game);
		_values.Add(val);
	}
	_values[_sP]->Cleanup();
	return _values[_sP];
}



//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::GetTop() {
	if (_sP < 0 || _sP >= _values.GetSize()) return NULL;
	else return _values[_sP];
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::GetAt(int Index) {
	Index = _sP - Index;
	if (Index < 0 || Index >= _values.GetSize()) return NULL;
	else return _values[Index];
}


//////////////////////////////////////////////////////////////////////////
void CScStack::CorrectParams(uint32 expected_params) {
	int nu_params = Pop()->GetInt();

	if (expected_params < nu_params) { // too many params
		while (expected_params < nu_params) {
			//Pop();
			delete _values[_sP - expected_params];
			_values.RemoveAt(_sP - expected_params);
			nu_params--;
			_sP--;
		}
	} else if (expected_params > nu_params) { // need more params
		while (expected_params > nu_params) {
			//Push(null_val);
			CScValue *null_val = new CScValue(Game);
			null_val->SetNULL();
			_values.InsertAt(_sP - nu_params + 1, null_val);
			nu_params++;
			_sP++;

			if (_values.GetSize() > _sP + 1) {
				delete _values[_values.GetSize() - 1];
				_values.RemoveAt(_values.GetSize() - 1);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CScStack::PushNULL() {
	/*
	CScValue* val = new CScValue(Game);
	val->SetNULL();
	Push(val);
	delete val;
	*/
	GetPushValue()->SetNULL();
}


//////////////////////////////////////////////////////////////////////////
void CScStack::PushInt(int Val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetInt(Val);
	Push(val);
	delete val;
	*/
	GetPushValue()->SetInt(Val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::PushFloat(double Val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetFloat(Val);
	Push(val);
	delete val;
	*/
	GetPushValue()->SetFloat(Val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::PushBool(bool Val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetBool(Val);
	Push(val);
	delete val;
	*/
	GetPushValue()->SetBool(Val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::PushString(const char *Val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetString(Val);
	Push(val);
	delete val;
	*/
	GetPushValue()->SetString(Val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::PushNative(CBScriptable *Val, bool Persistent) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetNative(Val, Persistent);
	Push(val);
	delete val;
	*/

	GetPushValue()->SetNative(Val, Persistent);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScStack::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(_sP));
	_values.Persist(PersistMgr);

	return S_OK;
}

} // end of namespace WinterMute
