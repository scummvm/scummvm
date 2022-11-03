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

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/system/sys_instance.h"
#include "engines/wintermute/base/scriptables/script_ext_array.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXArray, false)

BaseScriptable *makeSXArray(BaseGame *inGame, ScStack *stack) {
	return new SXArray(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXArray::SXArray(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	_length = 0;
	_values = new ScValue(_gameRef);

	int numParams = stack->pop()->getInt(0);

	if (numParams == 1) {
		_length = stack->pop()->getInt(0);
	} else if (numParams > 1) {
		_length = numParams;
		char paramName[20];
		for (int i = 0; i < numParams; i++) {
			Common::sprintf_s(paramName, "%d", i);
			_values->setProp(paramName, stack->pop());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
SXArray::SXArray(BaseGame *inGame) : BaseScriptable(inGame) {
	_length = 0;
	_values = new ScValue(_gameRef);
}


//////////////////////////////////////////////////////////////////////////
SXArray::~SXArray() {
	delete _values;
	_values = nullptr;
}


//////////////////////////////////////////////////////////////////////////
const char *SXArray::scToString() {
	char dummy[32768];
	dummy[0] = '\0';
	char propName[20];
	for (int i = 0; i < _length; i++) {
		Common::sprintf_s(propName, "%d", i);
		ScValue *val = _values->getProp(propName);
		if (val) {
			if (strlen(dummy) + strlen(val->getString()) < 32768) {
				Common::strcat_s(dummy, val->getString());
			}
		}

		if (i < _length - 1 && strlen(dummy) + 1 < 32768) {
			Common::strcat_s(dummy, ",");
		}
	}
	_strRep = dummy;
	return _strRep.c_str();
}


//////////////////////////////////////////////////////////////////////////
bool SXArray::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Push
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Push") == 0) {
		int numParams = stack->pop()->getInt(0);
		char paramName[20];

		for (int i = 0; i < numParams; i++) {
			_length++;
			Common::sprintf_s(paramName, "%d", _length - 1);
			_values->setProp(paramName, stack->pop(), true);
		}
		stack->pushInt(_length);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pop") == 0) {
		stack->correctParams(0);

		if (_length > 0) {
			char paramName[20];
			Common::sprintf_s(paramName, "%d", _length - 1);
			stack->push(_values->getProp(paramName));
			_values->deleteProp(paramName);
			_length--;
		} else {
			stack->pushNULL();
		}

		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] Delete
	// Removes item from array by index, shifting other elements
	// Used to shuffle arrays and delete found items in various scripts
	// Return value is never used
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Delete") == 0) {
		stack->correctParams(1);

		int shiftPoint = stack->pop()->getInt(0);
		char paramNameFrom[20];
		char paramNameTo[20];

		for (int i = shiftPoint; i < _length - 1 ; i++) {
			Common::sprintf_s(paramNameFrom, "%d", i + 1);
			Common::sprintf_s(paramNameTo, "%d", i);
			_values->setProp(paramNameTo, _values->getProp(paramNameFrom), false);
		}
		_values->deleteProp(paramNameFrom);
		_length--;
		stack->pushNULL();

		return STATUS_OK;
	}
#endif

	else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXArray::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("array");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Length") {
		_scValue->setInt(_length);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// [number]
	//////////////////////////////////////////////////////////////////////////
	else {
		char paramName[20];
		if (validNumber(name.c_str(), paramName)) { // TODO: Change to Common::String
			return _values->getProp(paramName);
		} else {
			return _scValue;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXArray::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Length") == 0) {
		int origLength = _length;
		_length = MAX(value->getInt(0), 0);

		char propName[20];
		if (_length < origLength) {
			for (int i = _length; i < origLength; i++) {
				Common::sprintf_s(propName, "%d", i);
				_values->deleteProp(propName);
			}
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [number]
	//////////////////////////////////////////////////////////////////////////
	else {
		char paramName[20];
		if (validNumber(name, paramName)) {
			int index = atoi(paramName);
			if (index >= _length) {
				_length = index + 1;
			}
			return _values->setProp(paramName, value);
		} else {
			return STATUS_FAILED;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXArray::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	persistMgr->transferSint32(TMEMBER(_length));
	persistMgr->transferPtr(TMEMBER_PTR(_values));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool SXArray::validNumber(const char *origStr, char *outStr) {
	bool isNumber = true;
	for (uint32 i = 0; i < strlen(origStr); i++) {
		if (!(origStr[i] >= '0' && origStr[i] <= '9')) {
			isNumber = false;
			break;
		}
	}

	if (isNumber) {
		int index = atoi(origStr);
		Common::sprintf_s(outStr, 20, "%d", index);
		return true;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool SXArray::push(ScValue *val) {
	char paramName[20];
	_length++;
	Common::sprintf_s(paramName, "%d", _length - 1);
	_values->setProp(paramName, val, true);
	return STATUS_OK;
}

} // End of namespace Wintermute
