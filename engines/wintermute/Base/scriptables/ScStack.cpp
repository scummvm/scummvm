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
	//_gameRef->LOG(0, "STAT: Stack size: %d, SP=%d", _values.getSize(), _sP);
#endif

	for (int i = 0; i < _values.getSize(); i++) {
		delete _values[i];
	}
	_values.removeAll();
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::pop() {
	if (_sP < 0) {
		_gameRef->LOG(0, "Fatal: Stack underflow");
		return NULL;
	}

	return _values[_sP--];
}


//////////////////////////////////////////////////////////////////////////
void CScStack::push(CScValue *val) {
	_sP++;

	if (_sP < _values.getSize()) {
		_values[_sP]->cleanup();
		_values[_sP]->copy(val);
	} else {
		CScValue *copyVal = new CScValue(_gameRef);
		copyVal->copy(val);
		_values.add(copyVal);
	}
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::getPushValue() {
	_sP++;

	if (_sP >= _values.getSize()) {
		CScValue *val = new CScValue(_gameRef);
		_values.add(val);
	}
	_values[_sP]->cleanup();
	return _values[_sP];
}



//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::getTop() {
	if (_sP < 0 || _sP >= _values.getSize()) return NULL;
	else return _values[_sP];
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScStack::getAt(int index) {
	index = _sP - index;
	if (index < 0 || index >= _values.getSize()) return NULL;
	else return _values[index];
}


//////////////////////////////////////////////////////////////////////////
void CScStack::correctParams(uint32 expectedParams) {
	uint32 nuParams = (uint32)pop()->getInt();

	if (expectedParams < nuParams) { // too many params
		while (expectedParams < nuParams) {
			//Pop();
			delete _values[_sP - expectedParams];
			_values.removeAt(_sP - expectedParams);
			nuParams--;
			_sP--;
		}
	} else if (expectedParams > nuParams) { // need more params
		while (expectedParams > nuParams) {
			//Push(null_val);
			CScValue *nullVal = new CScValue(_gameRef);
			nullVal->setNULL();
			_values.insertAt(_sP - nuParams + 1, nullVal);
			nuParams++;
			_sP++;

			if (_values.getSize() > _sP + 1) {
				delete _values[_values.getSize() - 1];
				_values.removeAt(_values.getSize() - 1);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushNULL() {
	/*
	CScValue* val = new CScValue(_gameRef);
	val->setNULL();
	Push(val);
	delete val;
	*/
	getPushValue()->setNULL();
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushInt(int val) {
	/*
	CScValue* val = new CScValue(_gameRef);
	val->setInt(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setInt(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushFloat(double val) {
	/*
	CScValue* val = new CScValue(_gameRef);
	val->setFloat(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setFloat(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushBool(bool val) {
	/*
	CScValue* val = new CScValue(_gameRef);
	val->setBool(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setBool(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushString(const char *val) {
	/*
	CScValue* val = new CScValue(_gameRef);
	val->setString(Val);
	Push(val);
	delete val;
	*/
	getPushValue()->setString(val);
}


//////////////////////////////////////////////////////////////////////////
void CScStack::pushNative(CBScriptable *val, bool persistent) {
	/*
	CScValue* val = new CScValue(_gameRef);
	val->setNative(Val, Persistent);
	Push(val);
	delete val;
	*/

	getPushValue()->setNative(val, persistent);
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CScStack::persist(CBPersistMgr *persistMgr) {

	persistMgr->transfer(TMEMBER(_gameRef));

	persistMgr->transfer(TMEMBER(_sP));
	_values.persist(persistMgr);

	return STATUS_OK;
}

} // end of namespace WinterMute
