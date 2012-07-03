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

#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/BGame.h"

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
CScValue *CScStack::pop() {
	if (_sP < 0) {
		Game->LOG(0, "Fatal: Stack underflow");
		return NULL;
	}

	return _values[_sP--];
}


//////////////////////////////////////////////////////////////////////////
void CScStack::push(CScValue *val) {
	_sP++;

	if (_sP < _values.GetSize()) {
		_values[_sP]->cleanup();
		_values[_sP]->Copy(val);
	} else {
		CScValue *copyVal = new CScValue(Game);
		copyVal->Copy(val);
		_values.Add(copyVal);
	}
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::getPushValue() {
	_sP++;

	if (_sP >= _values.GetSize()) {
		CScValue *val = new CScValue(Game);
		_values.Add(val);
	}
	_values[_sP]->cleanup();
	return _values[_sP];
}



//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::getTop() {
	if (_sP < 0 || _sP >= _values.GetSize()) return NULL;
	else return _values[_sP];
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::getAt(int index) {
	index = _sP - index;
	if (index < 0 || index >= _values.GetSize()) return NULL;
	else return _values[index];
}


//////////////////////////////////////////////////////////////////////////
void CScStack::correctParams(uint32 expected_params) {
	int nu_params = pop()->GetInt();

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
void CScStack::pushNULL() {
	/*
	CScValue* val = new CScValue(Game);
	val->SetNULL();
	Push(val);
	delete val;
	*/
	getPushValue()->SetNULL();
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushInt(int val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetInt(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->SetInt(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushFloat(double val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetFloat(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->SetFloat(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushBool(bool val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetBool(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->SetBool(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushString(const char *val) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetString(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->SetString(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushNative(CBScriptable *val, bool persistent) {
	/*
	CScValue* val = new CScValue(Game);
	val->SetNative(Val, Persistent);
	Push(val);
	delete val;
	*/

	getPushValue()->SetNative(val, persistent);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScStack::persist(CBPersistMgr *persistMgr) {

	persistMgr->transfer(TMEMBER(Game));

	persistMgr->transfer(TMEMBER(_sP));
	_values.persist(persistMgr);

	return S_OK;
}

} // end of namespace WinterMute
