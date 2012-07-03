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

#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/Base/scriptables/SXString.h"
#include "engines/wintermute/Base/scriptables/SXArray.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "common/tokenizer.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXString, false)

CBScriptable *makeSXString(CBGame *inGame, CScStack *stack) {
	return new CSXString(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
CSXString::CSXString(CBGame *inGame, CScStack *stack): CBScriptable(inGame) {
	_string = NULL;
	_capacity = 0;

	stack->correctParams(1);
	CScValue *Val = stack->pop();

	if (Val->isInt()) {
		_capacity = MAX(0, Val->getInt());
		if (_capacity > 0) {
			_string = new char[_capacity];
			memset(_string, 0, _capacity);
		}
	} else {
		setStringVal(Val->getString());
	}

	if (_capacity == 0) setStringVal("");
}


//////////////////////////////////////////////////////////////////////////
CSXString::~CSXString() {
	if (_string) delete [] _string;
}


//////////////////////////////////////////////////////////////////////////
void CSXString::setStringVal(const char *val) {
	int len = strlen(val);
	if (len >= _capacity) {
		_capacity = len + 1;
		delete[] _string;
		_string = NULL;
		_string = new char[_capacity];
		memset(_string, 0, _capacity);
	}
	strcpy(_string, val);
}


//////////////////////////////////////////////////////////////////////////
const char *CSXString::scToString() {
	if (_string) return _string;
	else return "[null string]";
}


//////////////////////////////////////////////////////////////////////////
void CSXString::scSetString(const char *val) {
	setStringVal(val);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXString::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Substring
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Substring") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		int end   = stack->pop()->getInt();

		if (end < start) CBUtils::Swap(&start, &end);

		//try {
		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		//WideString subStr = str.substr(start, end - start + 1);
		WideString subStr(str.c_str() + start, end - start + 1);

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::WideToUtf8(subStr).c_str());
		else
			stack->pushString(StringUtil::WideToAnsi(subStr).c_str());
		//  } catch (std::exception &) {
		//      stack->pushNULL();
		//  }

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Substr
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Substr") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();

		CScValue *val = stack->pop();
		int len = val->getInt();

		if (!val->isNULL() && len <= 0) {
			stack->pushString("");
			return S_OK;
		}

		if (val->isNULL()) len = strlen(_string) - start;

//		try {
		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

//			WideString subStr = str.substr(start, len);
		WideString subStr(str.c_str() + start, len);

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::WideToUtf8(subStr).c_str());
		else
			stack->pushString(StringUtil::WideToAnsi(subStr).c_str());
//		} catch (std::exception &) {
//			stack->pushNULL();
//		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToUpperCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToUpperCase") == 0) {
		stack->correctParams(0);

		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		StringUtil::ToUpperCase(str);

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::WideToUtf8(str).c_str());
		else
			stack->pushString(StringUtil::WideToAnsi(str).c_str());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToLowerCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToLowerCase") == 0) {
		stack->correctParams(0);

		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		StringUtil::ToLowerCase(str);

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::WideToUtf8(str).c_str());
		else
			stack->pushString(StringUtil::WideToAnsi(str).c_str());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IndexOf
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IndexOf") == 0) {
		stack->correctParams(2);

		const char *strToFind = stack->pop()->getString();
		int index = stack->pop()->getInt();

		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		WideString toFind;
		if (Game->_textEncoding == TEXT_UTF8)
			toFind = StringUtil::Utf8ToWide(strToFind);
		else
			toFind = StringUtil::AnsiToWide(strToFind);

		int indexOf = StringUtil::IndexOf(str, toFind, index);
		stack->pushInt(indexOf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Split
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Split") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		char Separators[MAX_PATH] = ",";
		if (!Val->isNULL()) strcpy(Separators, Val->getString());

		CSXArray *Array = new CSXArray(Game);
		if (!Array) {
			stack->pushNULL();
			return S_OK;
		}


		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		WideString delims;
		if (Game->_textEncoding == TEXT_UTF8)
			delims = StringUtil::Utf8ToWide(Separators);
		else
			delims = StringUtil::AnsiToWide(Separators);

		Common::Array<WideString> parts;



		Common::StringTokenizer tokenizer(str, delims);
		while (!tokenizer.empty()) {
			Common::String str2 = tokenizer.nextToken();
			parts.push_back(str2);
		}
		// TODO: Clean this up
		/*do {
		    pos = StringUtil::IndexOf(Common::String(str.c_str() + start), delims, start);
		    //pos = str.find_first_of(delims, start);
		    if (pos == start) {
		        start = pos + 1;
		    } else if (pos == str.size()) {
		        parts.push_back(Common::String(str.c_str() + start));
		        break;
		    } else {
		        parts.push_back(Common::String(str.c_str() + start, pos - start));
		        start = pos + 1;
		    }
		    //start = str.find_first_not_of(delims, start);
		    start = StringUtil::LastIndexOf(Common::String(str.c_str() + start), delims, start) + 1;

		} while (pos != str.size());*/

		for (Common::Array<WideString>::iterator it = parts.begin(); it != parts.end(); ++it) {
			WideString &part = (*it);

			if (Game->_textEncoding == TEXT_UTF8)
				Val = new CScValue(Game, StringUtil::WideToUtf8(part).c_str());
			else
				Val = new CScValue(Game, StringUtil::WideToAnsi(part).c_str());

			Array->Push(Val);
			delete Val;
			Val = NULL;
		}

		stack->pushNative(Array, false);
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXString::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("string");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Length") == 0) {
		if (Game->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::Utf8ToWide(_string);
			_scValue->setInt(wstr.size());
		} else
			_scValue->setInt(strlen(_string));

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Capacity") == 0) {
		_scValue->setInt(_capacity);
		return _scValue;
	}

	else return _scValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXString::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Capacity") == 0) {
		int newCap = value->getInt();
		if (newCap < strlen(_string) + 1) Game->LOG(0, "Warning: cannot lower string capacity");
		else if (newCap != _capacity) {
			char *newStr = new char[newCap];
			if (newStr) {
				memset(newStr, 0, newCap);
				strcpy(newStr, _string);
				delete[] _string;
				_string = newStr;
				_capacity = newCap;
			}
		}
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXString::persist(CBPersistMgr *persistMgr) {

	CBScriptable::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_capacity));

	if (persistMgr->_saving) {
		if (_capacity > 0) persistMgr->putBytes((byte *)_string, _capacity);
	} else {
		if (_capacity > 0) {
			_string = new char[_capacity];
			persistMgr->getBytes((byte *)_string, _capacity);
		} else _string = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CSXString::scCompare(CBScriptable *val) {
	return strcmp(_string, ((CSXString *)val)->_string);
}

} // end of namespace WinterMute
