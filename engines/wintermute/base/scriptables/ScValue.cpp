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

#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/BDynBuffer.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/base/scriptables/ScValue.h"
#include "engines/wintermute/base/scriptables/ScScript.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/base/BScriptable.h"

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
CScValue::CScValue(CBGame *inGame, bool val): CBBase(inGame) {
	_type = VAL_BOOL;
	_valBool = val;

	_valInt = 0;
	_valFloat = 0.0f;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, int val): CBBase(inGame) {
	_type = VAL_INT;
	_valInt = val;

	_valFloat = 0.0f;
	_valBool = false;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, double val): CBBase(inGame) {
	_type = VAL_FLOAT;
	_valFloat = val;

	_valInt = 0;
	_valBool = false;
	_valNative = NULL;
	_valString = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
CScValue::CScValue(CBGame *inGame, const char *val): CBBase(inGame) {
	_type = VAL_STRING;
	_valString = NULL;
	setStringVal(val);

	_valBool = false;
	_valInt = 0;
	_valFloat = 0.0f;
	_valNative = NULL;
	_valRef = NULL;
	_persistent = false;
	_isConstVar = false;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::cleanup(bool ignoreNatives) {
	deleteProps();

	if (_valString) delete [] _valString;

	if (!ignoreNatives) {
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
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
CScValue *CScValue::getProp(const char *name) {
	if (_type == VAL_VARIABLE_REF) return _valRef->getProp(name);

	if (_type == VAL_STRING && strcmp(name, "Length") == 0) {
		_gameRef->_scValue->_type = VAL_INT;

#if 0 // TODO: Remove FreeType-dependency
		if (_gameRef->_textEncoding == TEXT_ANSI) {
#else
		if (true) {
#endif
			_gameRef->_scValue->setInt(strlen(_valString));
		} else {
			WideString wstr = StringUtil::utf8ToWide(_valString);
			_gameRef->_scValue->setInt(wstr.size());
		}

		return _gameRef->_scValue;
	}

	CScValue *ret = NULL;

	if (_type == VAL_NATIVE && _valNative) ret = _valNative->scGetProperty(name);

	if (ret == NULL) {
		_valIter = _valObject.find(name);
		if (_valIter != _valObject.end()) ret = _valIter->_value;
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::deleteProp(const char *name) {
	if (_type == VAL_VARIABLE_REF) return _valRef->deleteProp(name);

	_valIter = _valObject.find(name);
	if (_valIter != _valObject.end()) {
		delete _valIter->_value;
		_valIter->_value = NULL;
	}

	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////////
bool CScValue::setProp(const char *name, CScValue *val, bool copyWhole, bool setAsConst) {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->setProp(name, val);

	bool ret = STATUS_FAILED;
	if (_type == VAL_NATIVE && _valNative) {
		ret = _valNative->scSetProperty(name, val);
	}

	if (DID_FAIL(ret)) {
		CScValue *newVal = NULL;

		_valIter = _valObject.find(name);
		if (_valIter != _valObject.end()) {
			newVal = _valIter->_value;
		}
		if (!newVal)
			newVal = new CScValue(_gameRef);
		else newVal->cleanup();

		newVal->copy(val, copyWhole);
		newVal->_isConstVar = setAsConst;
		_valObject[name] = newVal;

		if (_type != VAL_NATIVE) _type = VAL_OBJECT;

		/*
		_valIter = _valObject.find(Name);
		if (_valIter != _valObject.end()){
		    delete _valIter->_value;
		    _valIter->_value = NULL;
		}
		CScValue* val = new CScValue(_gameRef);
		val->Copy(Val, CopyWhole);
		val->_isConstVar = SetAsConst;
		_valObject[Name] = val;

		if(_type!=VAL_NATIVE) _type = VAL_OBJECT;
		*/
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::propExists(const char *name) {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->propExists(name);
	_valIter = _valObject.find(name);

	return (_valIter != _valObject.end());
}


//////////////////////////////////////////////////////////////////////////
void CScValue::deleteProps() {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		delete(CScValue *)_valIter->_value;
		_valIter++;
	}
	_valObject.clear();
}


//////////////////////////////////////////////////////////////////////////
void CScValue::CleanProps(bool includingNatives) {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		if (!_valIter->_value->_isConstVar && (!_valIter->_value->isNative() || includingNatives)) _valIter->_value->setNULL();
		_valIter++;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::isNULL() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isNULL();

	return (_type == VAL_NULL);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::isNative() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isNative();

	return (_type == VAL_NATIVE);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::isString() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isString();

	return (_type == VAL_STRING);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::isFloat() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isFloat();

	return (_type == VAL_FLOAT);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::isInt() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isInt();

	return (_type == VAL_INT);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::isBool() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isBool();

	return (_type == VAL_BOOL);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::isObject() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->isObject();

	return (_type == VAL_OBJECT);
}


//////////////////////////////////////////////////////////////////////////
TValType CScValue::getTypeTolerant() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->getType();

	return _type;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setBool(bool val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setBool(val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->scSetBool(val);
		return;
	}

	_valBool = val;
	_type = VAL_BOOL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setInt(int val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setInt(val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->scSetInt(val);
		return;
	}

	_valInt = val;
	_type = VAL_INT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setFloat(double val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setFloat(val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->scSetFloat(val);
		return;
	}

	_valFloat = val;
	_type = VAL_FLOAT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setString(const char *val) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setString(val);
		return;
	}

	if (_type == VAL_NATIVE) {
		_valNative->scSetString(val);
		return;
	}

	setStringVal(val);
	if (_valString) _type = VAL_STRING;
	else _type = VAL_NULL;
}

void CScValue::setString(const Common::String &val) {
	setString(val.c_str());
}

//////////////////////////////////////////////////////////////////////////
void CScValue::setStringVal(const char *val) {
	if (_valString) {
		delete [] _valString;
		_valString = NULL;
	}

	if (val == NULL) {
		_valString = NULL;
		return;
	}

	_valString = new char [strlen(val) + 1];
	if (_valString) {
		strcpy(_valString, val);
	}
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setNULL() {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setNULL();
		return;
	}

	if (_valNative && !_persistent) {
		_valNative->_refCount--;
		if (_valNative->_refCount <= 0) delete _valNative;
	}
	_valNative = NULL;
	deleteProps();

	_type = VAL_NULL;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setNative(CBScriptable *val, bool persistent) {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setNative(val, persistent);
		return;
	}

	if (val == NULL) {
		setNULL();
	} else {
		if (_valNative && !_persistent) {
			_valNative->_refCount--;
			if (_valNative->_refCount <= 0) {
				if (_valNative != val) delete _valNative;
				_valNative = NULL;
			}
		}

		_type = VAL_NATIVE;
		_persistent = persistent;

		_valNative = val;
		if (_valNative && !_persistent) _valNative->_refCount++;
	}
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setObject() {
	if (_type == VAL_VARIABLE_REF) {
		_valRef->setObject();
		return;
	}

	deleteProps();
	_type = VAL_OBJECT;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setReference(CScValue *val) {
	_valRef = val;
	_type = VAL_VARIABLE_REF;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::getBool(bool defaultVal) {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->getBool();

	switch (_type) {
	case VAL_BOOL:
		return _valBool;

	case VAL_NATIVE:
		return _valNative->scToBool();

	case VAL_INT:
		return (_valInt != 0);

	case VAL_FLOAT:
		return (_valFloat != 0.0f);

	case VAL_STRING:
		return (scumm_stricmp(_valString, "1") == 0 || scumm_stricmp(_valString, "yes") == 0 || scumm_stricmp(_valString, "true") == 0);

	default:
		return defaultVal;
	}
}


//////////////////////////////////////////////////////////////////////////
int CScValue::getInt(int defaultVal) {
	if (_type == VAL_VARIABLE_REF) return _valRef->getInt();

	switch (_type) {
	case VAL_BOOL:
		return _valBool ? 1 : 0;

	case VAL_NATIVE:
		return _valNative->scToInt();

	case VAL_INT:
		return _valInt;

	case VAL_FLOAT:
		return (int)_valFloat;

	case VAL_STRING:
		return atoi(_valString);

	default:
		return defaultVal;
	}
}


//////////////////////////////////////////////////////////////////////////
double CScValue::getFloat(double defaultVal) {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->getFloat();

	switch (_type) {
	case VAL_BOOL:
		return _valBool ? 1.0f : 0.0f;

	case VAL_NATIVE:
		return _valNative->scToFloat();

	case VAL_INT:
		return (double)_valInt;

	case VAL_FLOAT:
		return _valFloat;

	case VAL_STRING:
		return atof(_valString);

	default:
		return defaultVal;
	}
}

//////////////////////////////////////////////////////////////////////////
void *CScValue::getMemBuffer() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->getMemBuffer();

	if (_type == VAL_NATIVE)
		return _valNative->scToMemBuffer();
	else return (void *)NULL;
}


//////////////////////////////////////////////////////////////////////////
const char *CScValue::getString() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->getString();

	switch (_type) {
	case VAL_OBJECT:
		setStringVal("[object]");
		break;

	case VAL_NULL:
		setStringVal("[null]");
		break;

	case VAL_NATIVE: {
		const char *strVal = _valNative->scToString();
		setStringVal(strVal);
		return strVal;
		break;
	}

	case VAL_BOOL:
		setStringVal(_valBool ? "yes" : "no");
		break;

	case VAL_INT: {
		char dummy[50];
		sprintf(dummy, "%d", _valInt);
		setStringVal(dummy);
		break;
	}

	case VAL_FLOAT: {
		char dummy[50];
		sprintf(dummy, "%f", _valFloat);
		setStringVal(dummy);
		break;
	}

	case VAL_STRING:
		break;

	default:
		setStringVal("");
	}

	return _valString;
}


//////////////////////////////////////////////////////////////////////////
CBScriptable *CScValue::getNative() {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->getNative();

	if (_type == VAL_NATIVE) return _valNative;
	else return NULL;
}


//////////////////////////////////////////////////////////////////////////
TValType CScValue::getType() {
	return _type;
}


//////////////////////////////////////////////////////////////////////////
void CScValue::copy(CScValue *orig, bool copyWhole) {
	_gameRef = orig->_gameRef;

	if (_valNative && !_persistent) {
		_valNative->_refCount--;
		if (_valNative->_refCount <= 0) {
			if (_valNative != orig->_valNative) delete _valNative;
			_valNative = NULL;
		}
	}

	if (orig->_type == VAL_VARIABLE_REF && orig->_valRef && copyWhole) orig = orig->_valRef;

	cleanup(true);

	_type = orig->_type;
	_valBool = orig->_valBool;
	_valInt = orig->_valInt;
	_valFloat = orig->_valFloat;
	setStringVal(orig->_valString);

	_valRef = orig->_valRef;
	_persistent = orig->_persistent;

	_valNative = orig->_valNative;
	if (_valNative && !_persistent) _valNative->_refCount++;
//!!!! ref->native++

	// copy properties
	if (orig->_type == VAL_OBJECT && orig->_valObject.size() > 0) {
		orig->_valIter = orig->_valObject.begin();
		while (orig->_valIter != orig->_valObject.end()) {
			_valObject[orig->_valIter->_key] = new CScValue(_gameRef);
			_valObject[orig->_valIter->_key]->copy(orig->_valIter->_value);
			orig->_valIter++;
		}
	} else _valObject.clear();
}


//////////////////////////////////////////////////////////////////////////
void CScValue::setValue(CScValue *val) {
	if (val->_type == VAL_VARIABLE_REF) {
		setValue(val->_valRef);
		return;
	}

	// if being assigned a simple type, preserve native state
	if (_type == VAL_NATIVE && (val->_type == VAL_INT || val->_type == VAL_STRING || val->_type == VAL_BOOL)) {
		switch (val->_type) {
		case VAL_INT:
			_valNative->scSetInt(val->getInt());
			break;
		case VAL_FLOAT:
			_valNative->scSetFloat(val->getFloat());
			break;
		case VAL_BOOL:
			_valNative->scSetBool(val->getBool());
			break;
		case VAL_STRING:
			_valNative->scSetString(val->getString());
			break;
		default:
			warning("CScValue::setValue - unhandled enum");
			break;
		}
	}
	// otherwise just copy everything
	else copy(val);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::persist(CBPersistMgr *persistMgr) {
	persistMgr->transfer(TMEMBER(_gameRef));

	persistMgr->transfer(TMEMBER(_persistent));
	persistMgr->transfer(TMEMBER(_isConstVar));
	persistMgr->transfer(TMEMBER_INT(_type));
	persistMgr->transfer(TMEMBER(_valBool));
	persistMgr->transfer(TMEMBER(_valFloat));
	persistMgr->transfer(TMEMBER(_valInt));
	persistMgr->transfer(TMEMBER(_valNative));

	int size;
	const char *str;
	if (persistMgr->_saving) {
		size = _valObject.size();
		persistMgr->transfer("", &size);
		_valIter = _valObject.begin();
		while (_valIter != _valObject.end()) {
			str = _valIter->_key.c_str();
			persistMgr->transfer("", &str);
			persistMgr->transfer("", &_valIter->_value);

			_valIter++;
		}
	} else {
		CScValue *val;
		persistMgr->transfer("", &size);
		for (int i = 0; i < size; i++) {
			persistMgr->transfer("", &str);
			persistMgr->transfer("", &val);

			_valObject[str] = val;
			delete [] str;
		}
	}

	persistMgr->transfer(TMEMBER(_valRef));
	persistMgr->transfer(TMEMBER(_valString));

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

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::saveAsText(CBDynBuffer *buffer, int indent) {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		buffer->putTextIndent(indent, "PROPERTY {\n");
		buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _valIter->_key.c_str());
		buffer->putTextIndent(indent + 2, "VALUE=\"%s\"\n", _valIter->_value->getString());
		buffer->putTextIndent(indent, "}\n\n");

		_valIter++;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// -1 ... left is less, 0 ... equals, 1 ... left is greater
int CScValue::compare(CScValue *val1, CScValue *val2) {
	// both natives?
	if (val1->isNative() && val2->isNative()) {
		// same class?
		if (strcmp(val1->getNative()->getClassName(), val2->getNative()->getClassName()) == 0) {
			return val1->getNative()->scCompare(val2->getNative());
		} else return strcmp(val1->getString(), val2->getString());
	}

	// both objects?
	if (val1->isObject() && val2->isObject()) return -1;


	// null states
	if (val1->isNULL() && !val2->isNULL()) return -1;
	else if (!val1->isNULL() && val2->isNULL()) return 1;
	else if (val1->isNULL() && val2->isNULL()) return 0;

	// one of them is string? convert both to string
	if (val1->isString() || val2->isString()) return strcmp(val1->getString(), val2->getString());

	// one of them is float?
	if (val1->isFloat() || val2->isFloat()) {
		if (val1->getFloat() < val2->getFloat()) return -1;
		else if (val1->getFloat() > val2->getFloat()) return 1;
		else return 0;
	}

	// otherwise compare as int's
	if (val1->getInt() < val2->getInt()) return -1;
	else if (val1->getInt() > val2->getInt()) return 1;
	else return 0;
}


//////////////////////////////////////////////////////////////////////////
int CScValue::compareStrict(CScValue *val1, CScValue *val2) {
	if (val1->getTypeTolerant() != val2->getTypeTolerant()) return -1;
	else return CScValue::compare(val1, val2);
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgSendVariables(IWmeDebugClient *client, EWmeDebuggerVariableType type, CScScript *script, unsigned int scopeID) {
	_valIter = _valObject.begin();
	while (_valIter != _valObject.end()) {
		client->onVariableInit(type, script, scopeID, _valIter->_value, _valIter->_key.c_str());
		_valIter++;
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::setProperty(const char *propName, int value) {
	CScValue *val = new CScValue(_gameRef,  value);
	bool ret =  DID_SUCCEED(setProp(propName, val));
	delete val;
	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::setProperty(const char *propName, const char *value) {
	CScValue *val = new CScValue(_gameRef,  value);
	bool ret =  DID_SUCCEED(setProp(propName, val));
	delete val;
	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::setProperty(const char *propName, double value) {
	CScValue *val = new CScValue(_gameRef,  value);
	bool ret =  DID_SUCCEED(setProp(propName, val));
	delete val;
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::setProperty(const char *propName, bool value) {
	CScValue *val = new CScValue(_gameRef,  value);
	bool ret =  DID_SUCCEED(setProp(propName, val));
	delete val;
	return ret;
}


//////////////////////////////////////////////////////////////////////////
bool CScValue::setProperty(const char *propName) {
	CScValue *val = new CScValue(_gameRef);
	bool ret =  DID_SUCCEED(setProp(propName, val));
	delete val;
	return ret;
}


//////////////////////////////////////////////////////////////////////////
// IWmeDebugProp
//////////////////////////////////////////////////////////////////////////
EWmeDebuggerPropType CScValue::dbgGetType() {
	switch (getType()) {
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
int CScValue::dbgGetValInt() {
	return getInt();
}

//////////////////////////////////////////////////////////////////////////
double CScValue::dbgGetValFloat() {
	return getFloat();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgGetValBool() {
	return getBool();
}

//////////////////////////////////////////////////////////////////////////
const char *CScValue::dbgGetValString() {
	return getString();
}

//////////////////////////////////////////////////////////////////////////
IWmeDebugObject *CScValue::dbgGetValNative() {
	return getNative();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgSetVal(int value) {
	setInt(value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgSetVal(double value) {
	setFloat(value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgSetVal(bool value) {
	setBool(value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgSetVal(const char *value) {
	setString(value);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgSetVal() {
	setNULL();
	return true;
}


//////////////////////////////////////////////////////////////////////////
int CScValue::dbgGetNumProperties() {
	if (_valNative && _valNative->_scProp)
		return _valNative->_scProp->dbgGetNumProperties();
	else return _valObject.size();
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgGetProperty(int index, const char **name, IWmeDebugProp **value) {
	if (_valNative && _valNative->_scProp)
		return _valNative->_scProp->dbgGetProperty(index, name, value);
	else {
		int count = 0;
		_valIter = _valObject.begin();
		while (_valIter != _valObject.end()) {
			if (count == index) {
				*name = _valIter->_key.c_str();
				*value = _valIter->_value;
				return true;
			}
			_valIter++;
			count++;
		}
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CScValue::dbgGetDescription(char *buf, int bufSize) {
	if (_type == VAL_VARIABLE_REF)
		return _valRef->dbgGetDescription(buf, bufSize);

	if (_type == VAL_NATIVE) {
		_valNative->scDebuggerDesc(buf, bufSize);
	} else {
		strncpy(buf, getString(), bufSize);
	}
	return true;
}

} // end of namespace WinterMute
