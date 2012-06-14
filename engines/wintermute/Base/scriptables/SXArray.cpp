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
CSXArray::CSXArray(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	_length = 0;
	_values = new CScValue(Game);

	int NumParams = Stack->Pop()->GetInt(0);

	if (NumParams == 1) _length = Stack->Pop()->GetInt(0);
	else if (NumParams > 1) {
		_length = NumParams;
		char ParamName[20];
		for (int i = 0; i < NumParams; i++) {
			sprintf(ParamName, "%d", i);
			_values->SetProp(ParamName, Stack->Pop());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CSXArray::CSXArray(CBGame *inGame): CBScriptable(inGame) {
	_length = 0;
	_values = new CScValue(Game);
}


//////////////////////////////////////////////////////////////////////////
CSXArray::~CSXArray() {
	delete _values;
	_values = NULL;
}


//////////////////////////////////////////////////////////////////////////
const char *CSXArray::ScToString() {
	static char Dummy[32768];
	strcpy(Dummy, "");
	char PropName[20];
	for (int i = 0; i < _length; i++) {
		sprintf(PropName, "%d", i);
		CScValue *val = _values->GetProp(PropName);
		if (val) {
			if (strlen(Dummy) + strlen(val->GetString()) < 32768) {
				strcat(Dummy, val->GetString());
			}
		}

		if (i < _length - 1 && strlen(Dummy) + 1 < 32768) strcat(Dummy, ",");
	}
	return Dummy;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXArray::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// Push
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Push") == 0) {
		int NumParams = Stack->Pop()->GetInt(0);
		char ParamName[20];

		for (int i = 0; i < NumParams; i++) {
			_length++;
			sprintf(ParamName, "%d", _length - 1);
			_values->SetProp(ParamName, Stack->Pop(), true);
		}
		Stack->PushInt(_length);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pop
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Pop") == 0) {

		Stack->CorrectParams(0);

		if (_length > 0) {
			char ParamName[20];
			sprintf(ParamName, "%d", _length - 1);
			Stack->Push(_values->GetProp(ParamName));
			_values->DeleteProp(ParamName);
			_length--;
		} else Stack->PushNULL();

		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXArray::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("array");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Length") == 0) {
		_scValue->SetInt(_length);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// [number]
	//////////////////////////////////////////////////////////////////////////
	else {
		char ParamName[20];
		if (ValidNumber(Name, ParamName)) {
			return _values->GetProp(ParamName);
		} else return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXArray::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Length
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Length") == 0) {
		int OrigLength = _length;
		_length = MAX(Value->GetInt(0), 0);

		char PropName[20];
		if (_length < OrigLength) {
			for (int i = _length; i < OrigLength; i++) {
				sprintf(PropName, "%d", i);
				_values->DeleteProp(PropName);
			}
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// [number]
	//////////////////////////////////////////////////////////////////////////
	else {
		char ParamName[20];
		if (ValidNumber(Name, ParamName)) {
			int Index = atoi(ParamName);
			if (Index >= _length) _length = Index + 1;
			return _values->SetProp(ParamName, Value);
		} else return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXArray::Persist(CBPersistMgr *PersistMgr) {
	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_length));
	PersistMgr->Transfer(TMEMBER(_values));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CSXArray::ValidNumber(const char *OrigStr, char *OutStr) {
	bool IsNumber = true;
	for (int i = 0; i < strlen(OrigStr); i++) {
		if (!(OrigStr[i] >= '0' && OrigStr[i] <= '9')) {
			IsNumber = false;
			break;
		}
	}

	if (IsNumber) {
		int Index = atoi(OrigStr);
		sprintf(OutStr, "%d", Index);
		return true;
	} else return false;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CSXArray::Push(CScValue *Val) {
	char ParamName[20];
	_length++;
	sprintf(ParamName, "%d", _length - 1);
	_values->SetProp(ParamName, Val, true);
	return S_OK;
}

} // end of namespace WinterMute
