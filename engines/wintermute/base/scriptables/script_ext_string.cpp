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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/base/scriptables/script_ext_string.h"
#include "engines/wintermute/base/scriptables/script_ext_array.h"
#include "engines/wintermute/utils/string_util.h"
#include "common/str.h"
#include "common/tokenizer.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(SXString, false)

BaseScriptable *makeSXString(BaseGame *inGame, ScStack *stack) {
	return new SXString(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
SXString::SXString(BaseGame *inGame, ScStack *stack) : BaseScriptable(inGame) {
	_string = nullptr;
	_capacity = 0;

	stack->correctParams(1);
	ScValue *val = stack->pop();

	if (val->isInt()) {
		_capacity = MAX(0, val->getInt());
		if (_capacity > 0) {
			_string = new char[_capacity];
			memset(_string, 0, _capacity);
		}
	} else {
		setStringVal(val->getString());
	}

	if (_capacity == 0) {
		setStringVal("");
	}
}


//////////////////////////////////////////////////////////////////////////
SXString::~SXString() {
	if (_string) {
		delete[] _string;
	}
}


//////////////////////////////////////////////////////////////////////////
void SXString::setStringVal(const char *val) {
	int len = strlen(val);
	if (len >= _capacity) {
		_capacity = len + 1;
		delete[] _string;
		_string = nullptr;
		_string = new char[_capacity];
		memset(_string, 0, _capacity);
	}
	strcpy(_string, val);
}


//////////////////////////////////////////////////////////////////////////
const char *SXString::scToString() {
	if (_string) {
		return _string;
	} else {
		return "[null string]";
	}
}


//////////////////////////////////////////////////////////////////////////
void SXString::scSetString(const char *val) {
	setStringVal(val);
}


//////////////////////////////////////////////////////////////////////////
bool SXString::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// Substring
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Substring") == 0) {
		stack->correctParams(2);
		int start = stack->pop()->getInt();
		int end   = stack->pop()->getInt();

		if (end < start) {
			BaseUtils::swap(&start, &end);
		}

		//try {
		WideString str;
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			str = StringUtil::utf8ToWide(_string);
		} else {
			str = StringUtil::ansiToWide(_string);
		}

		WideString subStr = str.substr(start, end - start + 1);

		if (_gameRef->_textEncoding == TEXT_UTF8) {
			stack->pushString(StringUtil::wideToUtf8(subStr).c_str());
		} else {
			stack->pushString(StringUtil::wideToAnsi(subStr).c_str());
		}
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

		ScValue *val = stack->pop();
		int len = val->getInt();

		if (!val->isNULL() && len <= 0) {
			stack->pushString("");
			return STATUS_OK;
		}

		if (val->isNULL()) {
			len = strlen(_string) - start;
		}

//		try {
		WideString str;
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			str = StringUtil::utf8ToWide(_string);
		} else {
			str = StringUtil::ansiToWide(_string);
		}

		WideString subStr = str.substr(start, len);

		if (_gameRef->_textEncoding == TEXT_UTF8) {
			stack->pushString(StringUtil::wideToUtf8(subStr).c_str());
		} else {
			stack->pushString(StringUtil::wideToAnsi(subStr).c_str());
		}
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
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			str = StringUtil::utf8ToWide(_string);
		} else {
			str = StringUtil::ansiToWide(_string);
		}

		str.toUppercase();

		if (_gameRef->_textEncoding == TEXT_UTF8) {
			stack->pushString(StringUtil::wideToUtf8(str).c_str());
		} else {
			stack->pushString(StringUtil::wideToAnsi(str).c_str());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToLowerCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ToLowerCase") == 0) {
		stack->correctParams(0);

		WideString str;
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			str = StringUtil::utf8ToWide(_string);
		} else {
			str = StringUtil::ansiToWide(_string);
		}

		str.toLowercase();

		if (_gameRef->_textEncoding == TEXT_UTF8) {
			stack->pushString(StringUtil::wideToUtf8(str).c_str());
		} else {
			stack->pushString(StringUtil::wideToAnsi(str).c_str());
		}

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
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			str = StringUtil::utf8ToWide(_string);
		} else {
			str = StringUtil::ansiToWide(_string);
		}

		WideString toFind;
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			toFind = StringUtil::utf8ToWide(strToFind);
		} else {
			toFind = StringUtil::ansiToWide(strToFind);
		}

		int indexOf = StringUtil::indexOf(str, toFind, index);
		stack->pushInt(indexOf);

		return STATUS_OK;
	}

#ifdef ENABLE_HEROCRAFT
	//////////////////////////////////////////////////////////////////////////
	// [HeroCraft] GetCharCode
	// Returns integer value of char at given position
	// Used at "Pole Chudes" only
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCharCode") == 0) {
		stack->correctParams(1);

		int index = stack->pop()->getInt();
		int result = 0;
		if (strlen(_string) > (uint32)index) {
			result = _string[index];
		}
		stack->pushInt(result);
		
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// Split
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Split") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		char separators[MAX_PATH_LENGTH] = ",";
		if (!val->isNULL()) {
			Common::strlcpy(separators, val->getString(), MAX_PATH_LENGTH);
		}

		SXArray *array = new SXArray(_gameRef);
		if (!array) {
			stack->pushNULL();
			return STATUS_OK;
		}


		WideString str;
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			str = StringUtil::utf8ToWide(_string);
		} else {
			str = StringUtil::ansiToWide(_string);
		}

		WideString delims;
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			delims = StringUtil::utf8ToWide(separators);
		} else {
			delims = StringUtil::ansiToWide(separators);
		}

		Common::Array<WideString> parts;

		uint32 start = 0;
		for(uint32 i = 0; i < str.size() + 1; i++) {
			// The [] operator doesn't allow access to the zero code terminator
			// (bug #6531)
			uint32 ch = (i == str.size()) ? '\0' : str[i];
			if (ch =='\0' || delims.contains(ch)) {
				if (i != start) {
					parts.push_back(str.substr(start, i - start));
				} else {
					parts.push_back(WideString());
				}
				start = i + 1;
			}
		}

		for (Common::Array<WideString>::iterator it = parts.begin(); it != parts.end(); ++it) {
			WideString &part = (*it);

			if (_gameRef->_textEncoding == TEXT_UTF8) {
				val = new ScValue(_gameRef,  StringUtil::wideToUtf8(part).c_str());
			} else {
				val = new ScValue(_gameRef,  StringUtil::wideToAnsi(part).c_str());
			}

			array->push(val);
			delete val;
			val = nullptr;
		}

		stack->pushNative(array, false);
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *SXString::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("string");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Length") {
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::utf8ToWide(_string);
			_scValue->setInt(wstr.size());
		} else {
			_scValue->setInt(strlen(_string));
		}

		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Capacity") {
		_scValue->setInt(_capacity);
		return _scValue;
	} else {
		return _scValue;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXString::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Capacity") == 0) {
		int32 newCap = (uint32)value->getInt();
		if (newCap < (int32)(strlen(_string) + 1)) {
			_gameRef->LOG(0, "Warning: cannot lower string capacity");
		} else if (newCap != _capacity) {
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
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SXString::persist(BasePersistenceManager *persistMgr) {

	BaseScriptable::persist(persistMgr);

	persistMgr->transferSint32(TMEMBER(_capacity));

	if (persistMgr->getIsSaving()) {
		if (_capacity > 0) {
			persistMgr->putBytes((byte *)_string, _capacity);
		}
	} else {
		if (_capacity > 0) {
			_string = new char[_capacity];
			persistMgr->getBytes((byte *)_string, _capacity);
		} else {
			_string = nullptr;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
int SXString::scCompare(BaseScriptable *val) {
	return strcmp(_string, ((SXString *)val)->_string);
}

} // End of namespace Wintermute
