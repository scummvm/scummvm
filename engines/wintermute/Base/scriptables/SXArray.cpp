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

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Sys/SysInstance.h"
#include "engines/wintermute/Base/scriptables/SXArray.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXArray, false)

CBScriptable *makeSXArray(CBGame *inGame, CScStack *stack) {
	return new CSXArray(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
CSXArray::CSXArray(CBGame *inGame, CScStack *stack): CBScriptable(inGame) {
	_length = 0;
	_values = new CScValue(_gameRef);

	int numParams = stack->pop()->getInt(0);

	if (numParams == 1) _length = stack->pop()->getInt(0);
	else if (numParams > 1) {
		_length = numParams;
		char paramName[20];
		for (int i = 0; i < numParams; i++) {
			sprintf(paramName, "%d", i);
			_values->setProp(paramName, stack->pop());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CSXArray::CSXArray(CBGame *inGame): CBScriptable(inGame) {
	_length = 0;
	_values = new CScValue(_gameRef);
}


//////////////////////////////////////////////////////////////////////////
CSXArray::~CSXArray() {
	delete _values;
	_values = NULL;
}


//////////////////////////////////////////////////////////////////////////
const char *CSXArray::scToString() {
	static char dummy[32768]; // TODO: Get rid of static.
	strcpy(dummy, "");
	char propName[20];
	for (int i = 0; i < _length; i++) {
		sprintf(propName, "%d", i);
		CScValue *val = _values->getProp(propName);
		if (val) {
			if (strlen(dummy) + strlen(val->getString()) < 32768) {
				strcat(dummy, val->getString());
			}
		}

		if (i < _length - 1 && strlen(dummy) + 1 < 32768) strcat(dummy, ",");
	}
	return dummy;
}


//////////////////////////////////////////////////////////////////////////
bool CSXArray::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Push
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Push") == 0) {
		int numParams = stack->pop()->getInt(0);
		char paramName[20];

		for (int i = 0; i < numParams; i++) {
			_length++;
			sprintf(paramName, "%d", _length - 1);
			_values->setProp(paramName, stack->pop(), true);
		}
		stack->pushInt(_length);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pop
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Pop") == 0) {

		stack->correctParams(0);

		if (_length > 0) {
			char paramName[20];
			sprintf(paramName, "%d", _length - 1);
			stack->push(_values->getProp(paramName));
			_values->deleteProp(paramName);
			_length--;
		} else stack->pushNULL();

		return STATUS_OK;
	}

	else return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXArray::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("array");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Length") == 0) {
		_scValue->setInt(_length);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// [number]
	//////////////////////////////////////////////////////////////////////////
	else {
		char ParamName[20];
		if (validNumber(name, ParamName)) {
			return _values->getProp(ParamName);
		} else return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool CSXArray::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Length") == 0) {
		int OrigLength = _length;
		_length = MAX(value->getInt(0), 0);

		char PropName[20];
		if (_length < OrigLength) {
			for (int i = _length; i < OrigLength; i++) {
				sprintf(PropName, "%d", i);
				_values->deleteProp(PropName);
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
			int Index = atoi(paramName);
			if (Index >= _length) _length = Index + 1;
			return _values->setProp(paramName, value);
		} else return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool CSXArray::persist(CBPersistMgr *persistMgr) {
	CBScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_length));
	persistMgr->transfer(TMEMBER(_values));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CSXArray::validNumber(const char *origStr, char *outStr) {
	bool isNumber = true;
	for (uint32 i = 0; i < strlen(origStr); i++) {
		if (!(origStr[i] >= '0' && origStr[i] <= '9')) {
			isNumber = false;
			break;
		}
	}

	if (isNumber) {
		int index = atoi(origStr);
		sprintf(outStr, "%d", index);
		return true;
	} else return false;
}

//////////////////////////////////////////////////////////////////////////
bool CSXArray::push(CScValue *val) {
	char paramName[20];
	_length++;
	sprintf(paramName, "%d", _length - 1);
	_values->setProp(paramName, val, true);
	return STATUS_OK;
}

} // end of namespace WinterMute
