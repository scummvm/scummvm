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

#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/StringUtil.h"
#include "engines/wintermute/BScriptable.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(CScValue, false)

//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame): CBBase(inGame) {
	_type = VAL_NULL;

	_valBool = false;
	_valInt = 0;
	_valFloat = 0.0f;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, bool Val): CBBase(inGame) {
	_type = VAL_BOOL;
	_valBool = Val;

	_valInt = 0;
	_valFloat = 0.0f;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, int Val): CBBase(inGame) {
	_type = VAL_INT;
	_valInt = Val;

	_valFloat = 0.0f;
	_valBool = false;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, double Val): CBBase(inGame) {
	_type = VAL_FLOAT;
	_valFloat = Val;

	_valInt = 0;
	_valBool = false;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, const char *Val): CBBase(inGame) {
	_type = VAL_STRING;
	_valString = NULL;
	SetStringVal(Val);

	_valBool = false;
	_valInt = 0;
	_valFloat = 0.0f;
	_valNative = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::Cleanup(bool IgnoreNatives) {
	DeleteProps();

	if (_valString) delete [] _valString;

	if (!IgnoreNatives) {
		if (_valNative && !_persistent) {
			_valNative->_refCount--;
			if (_valNative->_refCount <= 0) {
				delete _valNative;
				_valNative = NULL;
			}
		}
	}


	_type = VAL_NULL;

	_valBool = false;
	_valInt = 0;
	_valFloat = 0.0f;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}



//////////////////////////////////////////////////////////////////////////
CScValue::~CScValue() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScValue::GetProp(const char *Name) {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetProp(Name);

	if (_type == VAL_STRING && strcmp(Name, "Length") == 0) {
		Game->_scValue->_type = VAL_INT;

#if 0 // TODO: Remove FreeType-dependency
		if (Game->_textEncoding == TEXT_ANSI) {
#else
		if (true) {
#endif
			Game->_scValue->SetInt(strlen(_valString));
		} else {
			WideString wstr = StringUtil::Utf8ToWide(_valString);
			Game->_scValue->SetInt(wstr.size());
		}

		return Game->_scValue;
	}

	CScValue *ret = NULL;

	if (_type == VAL_NATIVE && _valNative) ret = _valNative->ScGetProperty(Name);

	if (ret == NULL) {
		_valIter = _valObject.find(Name);
		if (_valIter != _valObject.end()) ret = _valIter->_value;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::DeleteProp(const char *Name) {
	if (_type == VAL_VARIABLE_REF) return _valRef->DeleteProp(Name);

	_valIter = _valObject.find(Name);
	if (_valIter != _valObject.end()) {
		delete _valIter->_value;
		_valIter->_value = NULL;
	}

	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::SetProp(const char *Name, CScValue *Val, bool CopyWhole, bool SetAsConst) {
	if (_type == VAL_VARIABLE_REF) return _valRef->SetProp(Name, Val);

	HRESULT ret = E_FAIL;
	if (_type == VAL_NATIVE && _valNative) {
		ret = _valNative->ScSetProperty(Name, Val);
	}

	if (FAILED(ret)) {
		CScValue *val = NULL;

		_valIter = _valObject.find(Name);
		if (_valIter != _valObject.end()) {
			val = _valIter->_value;
		}
		if (!val) val = new CScValue(Game);
		else val->Cleanup();

		val->Copy(Val, CopyWhole);
		val->_isConstVar = SetAsConst;
		_valObject[Name] = val;

		if (_type != VAL_NATIVE) _type = VAL_OBJECT;

		/*
		_valIter = _valObject.find(Name);
		if (_valIter != _valObject.end()){
		    delete _valIter->_value;
		    _valIter->_value = NULL;
		}
		CScValue* val = new CScValue(Game);
		val->Copy(Val, CopyWhole);
		val->_isConstVar = SetAsConst;
		_valObject[Name] = val;

		if(_type!=VAL_NATIVE) _type = VAL_OBJECT;
		*/
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::PropExists(const char *Name) {
	if (_type == VAL_VARIABLE_REF) return _valRef->PropExists(Name);
	_valIter = _valObject.find(Name);

	return (_valIter != _valObject.end());
}


//////////////////////////////////////////////////////////////////////////
void CScValue::DeleteProps() {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		delete(CScValue *)_valIter->_value;
		_valIter++;
	}
	_valObject.clear();
}


//////////////////////////////////////////////////////////////////////////
void CScValue::CleanProps(bool IncludingNatives) {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		if (!_valIter->_value->_isConstVar && (!_valIter->_value->IsNative() || IncludingNatives)) _valIter->_value->SetNULL();
		_valIter++;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::IsNULL() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsNULL();

	return (_type == VAL_NULL);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsNative() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsNative();

	return (_type == VAL_NATIVE);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsString() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsString();

	return (_type == VAL_STRING);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsFloat() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsFloat();

	return (_type == VAL_FLOAT);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsInt() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsInt();

	return (_type == VAL_INT);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsBool() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsBool();

	return (_type == VAL_BOOL);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsObject() {
	if (_type == VAL_VARIABLE_REF) return _valRef->IsObject();

	return (_type == VAL_OBJECT);
}


//////////////////////////////////////////////////////////////////////////
TValType CScValue::GetTypeTolerant() {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetType();

	return _type;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetBool(bool Val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetBool(Val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->ScSetBool(Val);
		return;
	}

	_valBool = Val;
	_type = VAL_BOOL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetInt(int Val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetInt(Val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->ScSetInt(Val);
		return;
	}

	_valInt = Val;
	_type = VAL_INT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetFloat(double Val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetFloat(Val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->ScSetFloat(Val);
		return;
	}

	_valFloat = Val;
	_type = VAL_FLOAT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetString(const char *Val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetString(Val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->ScSetString(Val);
		return;
	}

	SetStringVal(Val);
	if (_valString) _type = VAL_STRING;
	else _type = VAL_NULL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetStringVal(const char *Val) {
	if (_valString) {
		delete [] _valString;
		_valString = NULL;
	}

	if (Val == NULL) {
		_valString = NULL;
		return;
	}

	_valString = new char [strlen(Val) + 1];
	if (_valString) {
		strcpy(_valString, Val);
	}
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetNULL() {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetNULL();
		return;
	}

	if (_valNative && !_persistent) {
		_valNative->_refCount--;
		if (_valNative->_refCount <= 0) delete _valNative;
	}
	_valNative = NULL;
	DeleteProps();

	_type = VAL_NULL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetNative(CBScriptable *Val, bool Persistent) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetNative(Val, Persistent);
		return;
	}

	if (Val == NULL) {
		SetNULL();
	} else {
		if (_valNative && !_persistent) {
			_valNative->_refCount--;
			if (_valNative->_refCount <= 0) {
				if (_valNative != Val) delete _valNative;
				_valNative = NULL;
			}
		}

		_type = VAL_NATIVE;
		_persistent = Persistent;

		_valNative = Val;
		if (_valNative && !_persistent) _valNative->_refCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetObject() {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->SetObject();
		return;
	}

	DeleteProps();
	_type = VAL_OBJECT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetReference(CScValue *Val) {
	_valRef = Val;
	_type = VAL_VARIABLE_REF;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::GetBool(bool Default) {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetBool();

	switch (_type) {
	case VAL_BOOL:
		return _valBool;

	case VAL_NATIVE:
		return _valNative->ScToBool();

	case VAL_INT:
		return (_valInt != 0);

	case VAL_FLOAT:
		return (_valFloat != 0.0f);

	case VAL_STRING:
		return (scumm_stricmp(_valString, "1") == 0 || scumm_stricmp(_valString, "yes") == 0 || scumm_stricmp(_valString, "true") == 0);

	default:
		return Default;
	}
}


//////////////////////////////////////////////////////////////////////////
int CScValue::GetInt(int Default) {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetInt();

	switch (_type) {
	case VAL_BOOL:
		return _valBool ? 1 : 0;

	case VAL_NATIVE:
		return _valNative->ScToInt();

	case VAL_INT:
		return _valInt;

	case VAL_FLOAT:
		return (int)_valFloat;

	case VAL_STRING:
		return atoi(_valString);

	default:
		return Default;
	}
}


//////////////////////////////////////////////////////////////////////////
double CScValue::GetFloat(double Default) {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetFloat();

	switch (_type) {
	case VAL_BOOL:
		return _valBool ? 1.0f : 0.0f;

	case VAL_NATIVE:
		return _valNative->ScToFloat();

	case VAL_INT:
		return (double)_valInt;

	case VAL_FLOAT:
		return _valFloat;

	case VAL_STRING:
		return atof(_valString);

	default:
		return Default;
	}
}

//////////////////////////////////////////////////////////////////////////
void *CScValue::GetMemBuffer() {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetMemBuffer();

	if (_type == VAL_NATIVE) return _valNative->ScToMemBuffer();
	else return (void *)NULL;
}


//////////////////////////////////////////////////////////////////////////
const char *CScValue::GetString() {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetString();

	switch (_type) {
	case VAL_OBJECT:
		SetStringVal("[object]");
		break;

	case VAL_NULL:
		SetStringVal("[null]");
		break;

	case VAL_NATIVE: {
		const char *StrVal = _valNative->ScToString();
		SetStringVal(StrVal);
		return StrVal;
		break;
	}

	case VAL_BOOL:
		SetStringVal(_valBool ? "yes" : "no");
		break;

	case VAL_INT: {
		char dummy[50];
		sprintf(dummy, "%d", _valInt);
		SetStringVal(dummy);
		break;
	}

	case VAL_FLOAT: {
		char dummy[50];
		sprintf(dummy, "%f", _valFloat);
		SetStringVal(dummy);
		break;
	}

	case VAL_STRING:
		break;

	default:
		SetStringVal("");
	}

	return _valString;
}


//////////////////////////////////////////////////////////////////////////
CBScriptable *CScValue::GetNative() {
	if (_type == VAL_VARIABLE_REF) return _valRef->GetNative();

	if (_type == VAL_NATIVE) return _valNative;
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
TValType CScValue::GetType() {
	return _type;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::Copy(CScValue *orig, bool CopyWhole) {
	Game = orig->Game;

	if (_valNative && !_persistent) {
		_valNative->_refCount--;
		if (_valNative->_refCount <= 0) {
			if (_valNative != orig->_valNative) delete _valNative;
			_valNative = NULL;
		}
	}

	if (orig->_type == VAL_VARIABLE_REF && orig->_valRef && CopyWhole) orig = orig->_valRef;

	Cleanup(true);

	_type = orig->_type;
	_valBool = orig->_valBool;
	_valInt = orig->_valInt;
	_valFloat = orig->_valFloat;
	SetStringVal(orig->_valString);

	_valRef = orig->_valRef;
	_persistent = orig->_persistent;

	_valNative = orig->_valNative;
	if (_valNative && !_persistent) _valNative->_refCount++;
//!!!! ref->native++

	// copy properties
	if (orig->_type == VAL_OBJECT && orig->_valObject.size() > 0) {
		orig->_valIter = orig->_valObject.begin();
		while (orig->_valIter != orig->_valObject.end()) {
			_valObject[orig->_valIter->_key] = new CScValue(Game);
			_valObject[orig->_valIter->_key]->Copy(orig->_valIter->_value);
			orig->_valIter++;
		}
	} else _valObject.clear();
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetValue(CScValue *Val) {
	if (Val->_type == VAL_VARIABLE_REF) {
		SetValue(Val->_valRef);
		return;
	}

	// if being assigned a simple type, preserve native state
	if (_type == VAL_NATIVE && (Val->_type == VAL_INT || Val->_type == VAL_STRING || Val->_type == VAL_BOOL)) {
		switch (Val->_type) {
		case VAL_INT:
			_valNative->ScSetInt(Val->GetInt());
			break;
		case VAL_FLOAT:
			_valNative->ScSetFloat(Val->GetFloat());
			break;
		case VAL_BOOL:
			_valNative->ScSetBool(Val->GetBool());
			break;
		case VAL_STRING:
			_valNative->ScSetString(Val->GetString());
			break;
		}
	}
	// otherwise just copy everything
	else Copy(Val);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(_persistent));
	PersistMgr->Transfer(TMEMBER(_isConstVar));
	PersistMgr->Transfer(TMEMBER_INT(_type));
	PersistMgr->Transfer(TMEMBER(_valBool));
	PersistMgr->Transfer(TMEMBER(_valFloat));
	PersistMgr->Transfer(TMEMBER(_valInt));
	PersistMgr->Transfer(TMEMBER(_valNative));

	int size;
	const char *str;
	if (PersistMgr->_saving) {
		size = _valObject.size();
		PersistMgr->Transfer("", &size);
		_valIter = _valObject.begin();
		while (_valIter != _valObject.end()) {
			str = _valIter->_key.c_str();
			PersistMgr->Transfer("", &str);
			PersistMgr->Transfer("", &_valIter->_value);

			_valIter++;
		}
	} else {
		CScValue *val;
		PersistMgr->Transfer("", &size);
		for (int i = 0; i < size; i++) {
			PersistMgr->Transfer("", &str);
			PersistMgr->Transfer("", &val);

			_valObject[str] = val;
			delete [] str;
		}
	}

	PersistMgr->Transfer(TMEMBER(_valRef));
	PersistMgr->Transfer(TMEMBER(_valString));

	/*
	FILE* f = fopen("c:\\val.log", "a+");
	switch(_type)
	{
	case VAL_STRING:
	    fprintf(f, "str %s\n", _valString);
	    break;

	case VAL_INT:
	    fprintf(f, "int %d\n", _valInt);
	    break;

	case VAL_BOOL:
	    fprintf(f, "bool %d\n", _valBool);
	    break;

	case VAL_NULL:
	    fprintf(f, "null\n");
	    break;

	case VAL_NATIVE:
	    fprintf(f, "native\n");
	    break;

	case VAL_VARIABLE_REF:
	    fprintf(f, "ref\n");
	    break;

	case VAL_OBJECT:
	    fprintf(f, "obj\n");
	    break;

	case VAL_FLOAT:
	    fprintf(f, "float\n");
	    break;

	}
	fclose(f);
	*/

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		Buffer->PutTextIndent(Indent, "PROPERTY {\n");
		Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _valIter->_key.c_str());
		Buffer->PutTextIndent(Indent + 2, "VALUE=\"%s\"\n", _valIter->_value->GetString());
		Buffer->PutTextIndent(Indent, "}\n\n");

		_valIter++;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// -1 ... left is less, 0 ... equals, 1 ... left is greater
int CScValue::Compare(CScValue *Val1, CScValue *Val2) {
	// both natives?
	if (Val1->IsNative() && Val2->IsNative()) {
		// same class?
		if (strcmp(Val1->GetNative()->GetClassName(), Val2->GetNative()->GetClassName()) == 0) {
			return Val1->GetNative()->ScCompare(Val2->GetNative());
		} else return strcmp(Val1->GetString(), Val2->GetString());
	}

	// both objects?
	if (Val1->IsObject() && Val2->IsObject()) return -1;


	// null states
	if (Val1->IsNULL() && !Val2->IsNULL()) return -1;
	else if (!Val1->IsNULL() && Val2->IsNULL()) return 1;
	else if (Val1->IsNULL() && Val2->IsNULL()) return 0;

	// one of them is string? convert both to string
	if (Val1->IsString() || Val2->IsString()) return strcmp(Val1->GetString(), Val2->GetString());

	// one of them is float?
	if (Val1->IsFloat() || Val2->IsFloat()) {
		if (Val1->GetFloat() < Val2->GetFloat()) return -1;
		else if (Val1->GetFloat() > Val2->GetFloat()) return 1;
		else return 0;
	}

	// otherwise compare as int's
	if (Val1->GetInt() < Val2->GetInt()) return -1;
	else if (Val1->GetInt() > Val2->GetInt()) return 1;
	else return 0;
}


//////////////////////////////////////////////////////////////////////////
int CScValue::CompareStrict(CScValue *Val1, CScValue *Val2) {
	if (Val1->GetTypeTolerant() != Val2->GetTypeTolerant()) return -1;
	else return CScValue::Compare(Val1, Val2);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::DbgSendVariables(IWmeDebugClient *Client, EWmeDebuggerVariableType Type, CScScript *Script, unsigned int ScopeID) {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		Client->OnVariableInit(Type, Script, ScopeID, _valIter->_value, _valIter->_key.c_str());
		_valIter++;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, int Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp(PropName, Val));
	delete Val;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, const char *Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp(PropName, Val));
	delete Val;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, double Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp(PropName, Val));
	delete Val;
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, bool Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp(PropName, Val));
	delete Val;
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName) {
	CScValue *Val = new CScValue(Game);
	bool Ret =  SUCCEEDED(SetProp(PropName, Val));
	delete Val;
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugProp
//////////////////////////////////////////////////////////////////////////
EWmeDebuggerPropType CScValue::DbgGetType() {
	switch (GetType()) {
	case VAL_NULL:
		return WME_DBGPROP_NULL;
	case VAL_STRING:
		return WME_DBGPROP_STRING;
	case VAL_INT:
		return WME_DBGPROP_INT;
	case VAL_BOOL:
		return WME_DBGPROP_BOOL;
	case VAL_FLOAT:
		return WME_DBGPROP_FLOAT;
	case VAL_OBJECT:
		return WME_DBGPROP_OBJECT;
	case VAL_NATIVE:
		return WME_DBGPROP_NATIVE;
	default:
		return WME_DBGPROP_UNKNOWN;
	}
}

//////////////////////////////////////////////////////////////////////////
int CScValue::DbgGetValInt() {
	return GetInt();
}

//////////////////////////////////////////////////////////////////////////
double CScValue::DbgGetValFloat() {
	return GetFloat();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgGetValBool() {
	return GetBool();
}

//////////////////////////////////////////////////////////////////////////
const char *CScValue::DbgGetValString() {
	return GetString();
}

//////////////////////////////////////////////////////////////////////////
IWmeDebugObject *CScValue::DbgGetValNative() {
	return GetNative();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgSetVal(int Value) {
	SetInt(Value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgSetVal(double Value) {
	SetFloat(Value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgSetVal(bool Value) {
	SetBool(Value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgSetVal(const char *Value) {
	SetString(Value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgSetVal() {
	SetNULL();
	return true;
}


//////////////////////////////////////////////////////////////////////////
int CScValue::DbgGetNumProperties() {
	if (_valNative && _valNative->_scProp) return _valNative->_scProp->DbgGetNumProperties();
	else return _valObject.size();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgGetProperty(int Index, const char **Name, IWmeDebugProp **Value) {
	if (_valNative && _valNative->_scProp) return _valNative->_scProp->DbgGetProperty(Index, Name, Value);
	else {
		int Count = 0;
		_valIter = _valObject.begin();
		while (_valIter != _valObject.end()) {
			if (Count == Index) {
				*Name = _valIter->_key.c_str();
				*Value = _valIter->_value;
				return true;
			}
			_valIter++;
			Count++;
		}
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgGetDescription(char *Buf, int BufSize) {
	if (_type == VAL_VARIABLE_REF) return _valRef->DbgGetDescription(Buf, BufSize);

	if (_type == VAL_NATIVE) {
		_valNative->ScDebuggerDesc(Buf, BufSize);
	} else {
		strncpy(Buf, GetString(), BufSize);
	}
	return true;
}

} // end of namespace WinterMute
