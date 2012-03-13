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
	m_Type = VAL_NULL;

	m_ValBool = false;
	m_ValInt = 0;
	m_ValFloat = 0.0f;
	m_ValNative = NULL;
	m_ValString = NULL;
	m_ValRef = NULL;
	m_Persistent = false;
	m_IsConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, bool Val): CBBase(inGame) {
	m_Type = VAL_BOOL;
	m_ValBool = Val;

	m_ValInt = 0;
	m_ValFloat = 0.0f;
	m_ValNative = NULL;
	m_ValString = NULL;
	m_ValRef = NULL;
	m_Persistent = false;
	m_IsConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, int Val): CBBase(inGame) {
	m_Type = VAL_INT;
	m_ValInt = Val;

	m_ValFloat = 0.0f;
	m_ValBool = false;
	m_ValNative = NULL;
	m_ValString = NULL;
	m_ValRef = NULL;
	m_Persistent = false;
	m_IsConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, double Val): CBBase(inGame) {
	m_Type = VAL_FLOAT;
	m_ValFloat = Val;

	m_ValInt = 0;
	m_ValBool = false;
	m_ValNative = NULL;
	m_ValString = NULL;
	m_ValRef = NULL;
	m_Persistent = false;
	m_IsConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, const char *Val): CBBase(inGame) {
	m_Type = VAL_STRING;
	m_ValString = NULL;
	SetStringVal(Val);

	m_ValBool = false;
	m_ValInt = 0;
	m_ValFloat = 0.0f;
	m_ValNative = NULL;
	m_ValRef = NULL;
	m_Persistent = false;
	m_IsConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::Cleanup(bool IgnoreNatives) {
	DeleteProps();

	if (m_ValString) delete [] m_ValString;

	if (!IgnoreNatives) {
		if (m_ValNative && !m_Persistent) {
			m_ValNative->m_RefCount--;
			if (m_ValNative->m_RefCount <= 0) {
				delete m_ValNative;
				m_ValNative = NULL;
			}
		}
	}


	m_Type = VAL_NULL;

	m_ValBool = false;
	m_ValInt = 0;
	m_ValFloat = 0.0f;
	m_ValNative = NULL;
	m_ValString = NULL;
	m_ValRef = NULL;
	m_Persistent = false;
	m_IsConstVar = false;
}



//////////////////////////////////////////////////////////////////////////
CScValue::~CScValue() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScValue::GetProp(char *Name) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetProp(Name);

	if (m_Type == VAL_STRING && strcmp(Name, "Length") == 0) {
		Game->m_ScValue->m_Type = VAL_INT;

#if 0 // TODO: Remove FreeType-dependency
		if (Game->m_TextEncoding == TEXT_ANSI) {
#else
		if (true) {
#endif
			Game->m_ScValue->SetInt(strlen(m_ValString));
		} else {
			WideString wstr = StringUtil::Utf8ToWide(m_ValString);
			Game->m_ScValue->SetInt(wstr.size());
		}

		return Game->m_ScValue;
	}

	CScValue *ret = NULL;

	if (m_Type == VAL_NATIVE && m_ValNative) ret = m_ValNative->ScGetProperty(Name);

	if (ret == NULL) {
		m_ValIter = m_ValObject.find(Name);
		if (m_ValIter != m_ValObject.end()) ret = m_ValIter->_value;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::DeleteProp(char *Name) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->DeleteProp(Name);

	m_ValIter = m_ValObject.find(Name);
	if (m_ValIter != m_ValObject.end()) {
		delete m_ValIter->_value;
		m_ValIter->_value = NULL;
	}

	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::SetProp(char *Name, CScValue *Val, bool CopyWhole, bool SetAsConst) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->SetProp(Name, Val);

	HRESULT ret = E_FAIL;
	if (m_Type == VAL_NATIVE && m_ValNative) {
		ret = m_ValNative->ScSetProperty(Name, Val);
	}

	if (FAILED(ret)) {
		CScValue *val = NULL;

		m_ValIter = m_ValObject.find(Name);
		if (m_ValIter != m_ValObject.end()) {
			val = m_ValIter->_value;
		}
		if (!val) val = new CScValue(Game);
		else val->Cleanup();

		val->Copy(Val, CopyWhole);
		val->m_IsConstVar = SetAsConst;
		m_ValObject[Name] = val;

		if (m_Type != VAL_NATIVE) m_Type = VAL_OBJECT;

		/*
		m_ValIter = m_ValObject.find(Name);
		if (m_ValIter != m_ValObject.end()){
		    delete m_ValIter->second;
		    m_ValIter->second = NULL;
		}
		CScValue* val = new CScValue(Game);
		val->Copy(Val, CopyWhole);
		val->m_IsConstVar = SetAsConst;
		m_ValObject[Name] = val;

		if(m_Type!=VAL_NATIVE) m_Type = VAL_OBJECT;
		*/
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::PropExists(char *Name) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->PropExists(Name);
	m_ValIter = m_ValObject.find(Name);

	return (m_ValIter != m_ValObject.end());
}


//////////////////////////////////////////////////////////////////////////
void CScValue::DeleteProps() {
	m_ValIter = m_ValObject.begin();
	while (m_ValIter != m_ValObject.end()) {
		delete(CScValue *)m_ValIter->_value;
		m_ValIter++;
	}
	m_ValObject.clear();
}


//////////////////////////////////////////////////////////////////////////
void CScValue::CleanProps(bool IncludingNatives) {
	m_ValIter = m_ValObject.begin();
	while (m_ValIter != m_ValObject.end()) {
		if (!m_ValIter->_value->m_IsConstVar && (!m_ValIter->_value->IsNative() || IncludingNatives)) m_ValIter->_value->SetNULL();
		m_ValIter++;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::IsNULL() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsNULL();

	return (m_Type == VAL_NULL);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsNative() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsNative();

	return (m_Type == VAL_NATIVE);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsString() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsString();

	return (m_Type == VAL_STRING);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsFloat() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsFloat();

	return (m_Type == VAL_FLOAT);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsInt() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsInt();

	return (m_Type == VAL_INT);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsBool() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsBool();

	return (m_Type == VAL_BOOL);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::IsObject() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->IsObject();

	return (m_Type == VAL_OBJECT);
}


//////////////////////////////////////////////////////////////////////////
TValType CScValue::GetTypeTolerant() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetType();

	return m_Type;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetBool(bool Val) {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetBool(Val);
		return;
	}

	if (m_Type == VAL_NATIVE) {
		m_ValNative->ScSetBool(Val);
		return;
	}

	m_ValBool = Val;
	m_Type = VAL_BOOL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetInt(int Val) {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetInt(Val);
		return;
	}

	if (m_Type == VAL_NATIVE) {
		m_ValNative->ScSetInt(Val);
		return;
	}

	m_ValInt = Val;
	m_Type = VAL_INT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetFloat(double Val) {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetFloat(Val);
		return;
	}

	if (m_Type == VAL_NATIVE) {
		m_ValNative->ScSetFloat(Val);
		return;
	}

	m_ValFloat = Val;
	m_Type = VAL_FLOAT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetString(const char *Val) {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetString(Val);
		return;
	}

	if (m_Type == VAL_NATIVE) {
		m_ValNative->ScSetString(Val);
		return;
	}

	SetStringVal(Val);
	if (m_ValString) m_Type = VAL_STRING;
	else m_Type = VAL_NULL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetStringVal(const char *Val) {
	if (m_ValString) delete [] m_ValString;

	if (Val == NULL) {
		m_ValString = NULL;
		return;
	}

	m_ValString = new char [strlen(Val) + 1];
	if (m_ValString) {
		strcpy(m_ValString, Val);
	}
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetNULL() {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetNULL();
		return;
	}

	if (m_ValNative && !m_Persistent) {
		m_ValNative->m_RefCount--;
		if (m_ValNative->m_RefCount <= 0) delete m_ValNative;
	}
	m_ValNative = NULL;
	DeleteProps();

	m_Type = VAL_NULL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetNative(CBScriptable *Val, bool Persistent) {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetNative(Val, Persistent);
		return;
	}

	if (Val == NULL) {
		SetNULL();
	} else {
		if (m_ValNative && !m_Persistent) {
			m_ValNative->m_RefCount--;
			if (m_ValNative->m_RefCount <= 0) {
				if (m_ValNative != Val) delete m_ValNative;
				m_ValNative = NULL;
			}
		}

		m_Type = VAL_NATIVE;
		m_Persistent = Persistent;

		m_ValNative = Val;
		if (m_ValNative && !m_Persistent) m_ValNative->m_RefCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetObject() {
	if (m_Type == VAL_VARIABLE_REF) {
		m_ValRef->SetObject();
		return;
	}

	DeleteProps();
	m_Type = VAL_OBJECT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetReference(CScValue *Val) {
	m_ValRef = Val;
	m_Type = VAL_VARIABLE_REF;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::GetBool(bool Default) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetBool();

	switch (m_Type) {
	case VAL_BOOL:
		return m_ValBool;

	case VAL_NATIVE:
		return m_ValNative->ScToBool();

	case VAL_INT:
		return (m_ValInt != 0);

	case VAL_FLOAT:
		return (m_ValFloat != 0.0f);

	case VAL_STRING:
		return (scumm_stricmp(m_ValString, "1") == 0 || scumm_stricmp(m_ValString, "yes") == 0 || scumm_stricmp(m_ValString, "true") == 0);

	default:
		return Default;
	}
}


//////////////////////////////////////////////////////////////////////////
int CScValue::GetInt(int Default) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetInt();

	switch (m_Type) {
	case VAL_BOOL:
		return m_ValBool ? 1 : 0;

	case VAL_NATIVE:
		return m_ValNative->ScToInt();

	case VAL_INT:
		return m_ValInt;

	case VAL_FLOAT:
		return (int)m_ValFloat;

	case VAL_STRING:
		return atoi(m_ValString);

	default:
		return Default;
	}
}


//////////////////////////////////////////////////////////////////////////
double CScValue::GetFloat(double Default) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetFloat();

	switch (m_Type) {
	case VAL_BOOL:
		return m_ValBool ? 1.0f : 0.0f;

	case VAL_NATIVE:
		return m_ValNative->ScToFloat();

	case VAL_INT:
		return (double)m_ValInt;

	case VAL_FLOAT:
		return m_ValFloat;

	case VAL_STRING:
		return atof(m_ValString);

	default:
		return Default;
	}
}

//////////////////////////////////////////////////////////////////////////
void *CScValue::GetMemBuffer() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetMemBuffer();

	if (m_Type == VAL_NATIVE) return m_ValNative->ScToMemBuffer();
	else return (void *)NULL;
}


//////////////////////////////////////////////////////////////////////////
char *CScValue::GetString() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetString();

	switch (m_Type) {
	case VAL_OBJECT:
		SetStringVal("[object]");
		break;

	case VAL_NULL:
		SetStringVal("[null]");
		break;

	case VAL_NATIVE: {
		char *StrVal = m_ValNative->ScToString();
		SetStringVal(StrVal);
		return StrVal;
		break;
	}

	case VAL_BOOL:
		SetStringVal(m_ValBool ? "yes" : "no");
		break;

	case VAL_INT: {
		char dummy[50];
		sprintf(dummy, "%d", m_ValInt);
		SetStringVal(dummy);
		break;
	}

	case VAL_FLOAT: {
		char dummy[50];
		sprintf(dummy, "%f", m_ValFloat);
		SetStringVal(dummy);
		break;
	}

	case VAL_STRING:
		break;

	default:
		SetStringVal("");
	}

	return m_ValString;
}


//////////////////////////////////////////////////////////////////////////
CBScriptable *CScValue::GetNative() {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->GetNative();

	if (m_Type == VAL_NATIVE) return m_ValNative;
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
TValType CScValue::GetType() {
	return m_Type;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::Copy(CScValue *orig, bool CopyWhole) {
	Game = orig->Game;

	if (m_ValNative && !m_Persistent) {
		m_ValNative->m_RefCount--;
		if (m_ValNative->m_RefCount <= 0) {
			if (m_ValNative != orig->m_ValNative) delete m_ValNative;
			m_ValNative = NULL;
		}
	}

	if (orig->m_Type == VAL_VARIABLE_REF && orig->m_ValRef && CopyWhole) orig = orig->m_ValRef;

	Cleanup(true);

	m_Type = orig->m_Type;
	m_ValBool = orig->m_ValBool;
	m_ValInt = orig->m_ValInt;
	m_ValFloat = orig->m_ValFloat;
	SetStringVal(orig->m_ValString);

	m_ValRef = orig->m_ValRef;
	m_Persistent = orig->m_Persistent;

	m_ValNative = orig->m_ValNative;
	if (m_ValNative && !m_Persistent) m_ValNative->m_RefCount++;
//!!!! ref->native++

	// copy properties
	if (orig->m_Type == VAL_OBJECT && orig->m_ValObject.size() > 0) {
		orig->m_ValIter = orig->m_ValObject.begin();
		while (orig->m_ValIter != orig->m_ValObject.end()) {
			m_ValObject[orig->m_ValIter->_key] = new CScValue(Game);
			m_ValObject[orig->m_ValIter->_key]->Copy(orig->m_ValIter->_value);
			orig->m_ValIter++;
		}
	} else m_ValObject.clear();
}


//////////////////////////////////////////////////////////////////////////
void CScValue::SetValue(CScValue *Val) {
	if (Val->m_Type == VAL_VARIABLE_REF) {
		SetValue(Val->m_ValRef);
		return;
	}

	// if being assigned a simple type, preserve native state
	if (m_Type == VAL_NATIVE && (Val->m_Type == VAL_INT || Val->m_Type == VAL_STRING || Val->m_Type == VAL_BOOL)) {
		switch (Val->m_Type) {
		case VAL_INT:
			m_ValNative->ScSetInt(Val->GetInt());
			break;
		case VAL_FLOAT:
			m_ValNative->ScSetFloat(Val->GetFloat());
			break;
		case VAL_BOOL:
			m_ValNative->ScSetBool(Val->GetBool());
			break;
		case VAL_STRING:
			m_ValNative->ScSetString(Val->GetString());
			break;
		}
	}
	// otherwise just copy everything
	else Copy(Val);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CScValue::Persist(CBPersistMgr *PersistMgr) {
	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER(m_Persistent));
	PersistMgr->Transfer(TMEMBER(m_IsConstVar));
	PersistMgr->Transfer(TMEMBER_INT(m_Type));
	PersistMgr->Transfer(TMEMBER(m_ValBool));
	PersistMgr->Transfer(TMEMBER(m_ValFloat));
	PersistMgr->Transfer(TMEMBER(m_ValInt));
	PersistMgr->Transfer(TMEMBER(m_ValNative));

	int size;
	char *str;
	if (PersistMgr->m_Saving) {
		size = m_ValObject.size();
		PersistMgr->Transfer("", &size);
		m_ValIter = m_ValObject.begin();
		while (m_ValIter != m_ValObject.end()) {
			str = (char *)m_ValIter->_key.c_str();
			PersistMgr->Transfer("", &str);
			PersistMgr->Transfer("", &m_ValIter->_value);

			m_ValIter++;
		}
	} else {
		CScValue *val;
		PersistMgr->Transfer("", &size);
		for (int i = 0; i < size; i++) {
			PersistMgr->Transfer("", &str);
			PersistMgr->Transfer("", &val);

			m_ValObject[str] = val;
			delete [] str;
		}
	}

	PersistMgr->Transfer(TMEMBER(m_ValRef));
	PersistMgr->Transfer(TMEMBER(m_ValString));

	/*
	FILE* f = fopen("c:\\val.log", "a+");
	switch(m_Type)
	{
	case VAL_STRING:
	    fprintf(f, "str %s\n", m_ValString);
	    break;

	case VAL_INT:
	    fprintf(f, "int %d\n", m_ValInt);
	    break;

	case VAL_BOOL:
	    fprintf(f, "bool %d\n", m_ValBool);
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
	m_ValIter = m_ValObject.begin();
	while (m_ValIter != m_ValObject.end()) {
		Buffer->PutTextIndent(Indent, "PROPERTY {\n");
		Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", (char *)m_ValIter->_key.c_str());
		Buffer->PutTextIndent(Indent + 2, "VALUE=\"%s\"\n", m_ValIter->_value->GetString());
		Buffer->PutTextIndent(Indent, "}\n\n");

		m_ValIter++;
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
	m_ValIter = m_ValObject.begin();
	while (m_ValIter != m_ValObject.end()) {
		Client->OnVariableInit(Type, Script, ScopeID, m_ValIter->_value, m_ValIter->_key.c_str());
		m_ValIter++;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, int Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp((char *)PropName, Val));
	delete Val;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, const char *Value) {
	CScValue *Val = new CScValue(Game, (char *)Value);
	bool Ret =  SUCCEEDED(SetProp((char *)PropName, Val));
	delete Val;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, double Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp((char *)PropName, Val));
	delete Val;
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName, bool Value) {
	CScValue *Val = new CScValue(Game, Value);
	bool Ret =  SUCCEEDED(SetProp((char *)PropName, Val));
	delete Val;
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::SetProperty(const char *PropName) {
	CScValue *Val = new CScValue(Game);
	bool Ret =  SUCCEEDED(SetProp((char *)PropName, Val));
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
	SetString((char *)Value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgSetVal() {
	SetNULL();
	return true;
}


//////////////////////////////////////////////////////////////////////////
int CScValue::DbgGetNumProperties() {
	if (m_ValNative && m_ValNative->m_ScProp) return m_ValNative->m_ScProp->DbgGetNumProperties();
	else return m_ValObject.size();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgGetProperty(int Index, const char **Name, IWmeDebugProp **Value) {
	if (m_ValNative && m_ValNative->m_ScProp) return m_ValNative->m_ScProp->DbgGetProperty(Index, Name, Value);
	else {
		int Count = 0;
		m_ValIter = m_ValObject.begin();
		while (m_ValIter != m_ValObject.end()) {
			if (Count == Index) {
				*Name = m_ValIter->_key.c_str();
				*Value = m_ValIter->_value;
				return true;
			}
			m_ValIter++;
			Count++;
		}
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::DbgGetDescription(char *Buf, int BufSize) {
	if (m_Type == VAL_VARIABLE_REF) return m_ValRef->DbgGetDescription(Buf, BufSize);

	if (m_Type == VAL_NATIVE) {
		m_ValNative->ScDebuggerDesc(Buf, BufSize);
	} else {
		strncpy(Buf, GetString(), BufSize);
	}
	return true;
}

} // end of namespace WinterMute
