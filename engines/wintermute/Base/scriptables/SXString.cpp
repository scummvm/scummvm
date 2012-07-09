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
	CScValue *val = stack->pop();

	if (val->isInt()) {
		_capacity = MAX(0, val->getInt());
		if (_capacity > 0) {
			_string = new char[_capacity];
			memset(_string, 0, _capacity);
		}
	} else {
		setStringVal(val->getString());
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
ERRORCODE CSXString::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Substring
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Substring") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		int end   = stack->pop()->getInt();

		if (end < start) CBUtils::swap(&start, &end);

		//try {
		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::utf8ToWide(_string);
		else
			str = StringUtil::ansiToWide(_string);

		//WideString subStr = str.substr(start, end - start + 1);
		WideString subStr(str.c_str() + start, end - start + 1);

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::wideToUtf8(subStr).c_str());
		else
			stack->pushString(StringUtil::wideToAnsi(subStr).c_str());
		//  } catch (std::exception &) {
		//      stack->pushNULL();
		//  }

		return STATUS_OK;
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
			return STATUS_OK;
		}

		if (val->isNULL()) len = strlen(_string) - start;

//		try {
		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::utf8ToWide(_string);
		else
			str = StringUtil::ansiToWide(_string);

//			WideString subStr = str.substr(start, len);
		WideString subStr(str.c_str() + start, len);

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::wideToUtf8(subStr).c_str());
		else
			stack->pushString(StringUtil::wideToAnsi(subStr).c_str());
//		} catch (std::exception &) {
//			stack->pushNULL();
//		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToUpperCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToUpperCase") == 0) {
		stack->correctParams(0);

		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::utf8ToWide(_string);
		else
			str = StringUtil::ansiToWide(_string);

		str.toUppercase();

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::wideToUtf8(str).c_str());
		else
			stack->pushString(StringUtil::wideToAnsi(str).c_str());

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToLowerCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToLowerCase") == 0) {
		stack->correctParams(0);

		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::utf8ToWide(_string);
		else
			str = StringUtil::ansiToWide(_string);

		str.toLowercase();

		if (Game->_textEncoding == TEXT_UTF8)
			stack->pushString(StringUtil::wideToUtf8(str).c_str());
		else
			stack->pushString(StringUtil::wideToAnsi(str).c_str());

		return STATUS_OK;
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
			str = StringUtil::utf8ToWide(_string);
		else
			str = StringUtil::ansiToWide(_string);

		WideString toFind;
		if (Game->_textEncoding == TEXT_UTF8)
			toFind = StringUtil::utf8ToWide(strToFind);
		else
			toFind = StringUtil::ansiToWide(strToFind);

		int indexOf = StringUtil::indexOf(str, toFind, index);
		stack->pushInt(indexOf);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Split
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Split") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		char separators[MAX_PATH_LENGTH] = ",";
		if (!val->isNULL()) strcpy(separators, val->getString());

		CSXArray *array = new CSXArray(Game);
		if (!array) {
			stack->pushNULL();
			return STATUS_OK;
		}


		WideString str;
		if (Game->_textEncoding == TEXT_UTF8)
			str = StringUtil::utf8ToWide(_string);
		else
			str = StringUtil::ansiToWide(_string);

		WideString delims;
		if (Game->_textEncoding == TEXT_UTF8)
			delims = StringUtil::utf8ToWide(separators);
		else
			delims = StringUtil::ansiToWide(separators);

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
				val = new CScValue(Game, StringUtil::wideToUtf8(part).c_str());
			else
				val = new CScValue(Game, StringUtil::wideToAnsi(part).c_str());

			array->push(val);
			delete val;
			val = NULL;
		}

		stack->pushNative(array, false);
		return STATUS_OK;
	}

	else return STATUS_FAILED;
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
			WideString wstr = StringUtil::utf8ToWide(_string);
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
ERRORCODE CSXString::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Capacity") == 0) {
		int32 newCap = (uint32)value->getInt();
		if (newCap < (int32)(strlen(_string) + 1)) Game->LOG(0, "Warning: cannot lower string capacity");
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
		return STATUS_OK;
	}

	else return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
ERRORCODE CSXString::persist(CBPersistMgr *persistMgr) {

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

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int CSXString::scCompare(CBScriptable *val) {
	return strcmp(_string, ((CSXString *)val)->_string);
}

} // end of namespace WinterMute
