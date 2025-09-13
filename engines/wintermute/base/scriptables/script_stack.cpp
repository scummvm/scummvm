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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/base_game.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(ScStack, false)

//////////////////////////////////////////////////////////////////////////
ScStack::ScStack(BaseGame *inGame) : BaseClass(inGame) {
	_sP = -1;
}


//////////////////////////////////////////////////////////////////////////
ScStack::~ScStack() {
	//_game->LOG(0, "STAT: Stack size: %d, SP=%d", _values.getSize(), _sP);

	for (int32 i = 0; i < _values.getSize(); i++) {
		delete _values[i];
	}
	_values.removeAll();
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::pop() {
	if (_sP < 0) {
		_game->LOG(0, "Fatal: Stack underflow");
		return nullptr;
	}

	return _values[_sP--];
}


//////////////////////////////////////////////////////////////////////////
void ScStack::push(ScValue *val) {
	_sP++;

	if (_sP < _values.getSize()) {
		_values[_sP]->cleanup();
		_values[_sP]->copy(val);
	} else {
		ScValue *copyVal = new ScValue(_game);
		copyVal->copy(val);
		_values.add(copyVal);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::getPushValue() {
	_sP++;

	if (_sP >= _values.getSize()) {
		ScValue *val = new ScValue(_game);
		_values.add(val);
	}
	_values[_sP]->cleanup();
	return _values[_sP];
}



//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::getTop() {
	if (_sP < 0 || _sP >= _values.getSize()) {
		return nullptr;
	} else {
		return _values[_sP];
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *ScStack::getAt(int index) {
	index = _sP - index;
	if (index < 0 || index >= _values.getSize()) {
		return nullptr;
	} else {
		return _values[index];
	}
}


//////////////////////////////////////////////////////////////////////////
void ScStack::correctParams(uint32 expectedParams) {
	uint32 numParams = (uint32)pop()->getInt();

	if (expectedParams < numParams) { // too many params
		while (expectedParams < numParams) {
			//pop();
			delete _values[_sP - expectedParams];
			_values.removeAt(_sP - expectedParams);
			numParams--;
			_sP--;
		}
	} else if (expectedParams > numParams) { // need more params
		while (expectedParams > numParams) {
			//push(nullVal);
			ScValue *nullVal = new ScValue(_game);
			nullVal->setNULL();
			_values.insertAt(_sP - numParams + 1, nullVal);
			numParams++;
			_sP++;

			if (_values.getSize() > _sP + 1) {
				delete _values[_values.getSize() - 1];
				_values.removeAt(_values.getSize() - 1);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushNULL() {
	getPushValue()->setNULL();
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushInt(int val) {
	getPushValue()->setInt(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushFloat(double val) {
	getPushValue()->setFloat(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushBool(bool val) {
	getPushValue()->setBool(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushString(const char *val) {
	getPushValue()->setString(val);
}


//////////////////////////////////////////////////////////////////////////
void ScStack::pushNative(BaseScriptable *val, bool persistent) {
	getPushValue()->setNative(val, persistent);
}


//////////////////////////////////////////////////////////////////////////
bool ScStack::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transferPtr(TMEMBER_PTR(_game));

	persistMgr->transferSint32(TMEMBER(_sP));
	_values.persist(persistMgr);

	return STATUS_OK;
}

} // End of namespace Wintermute
