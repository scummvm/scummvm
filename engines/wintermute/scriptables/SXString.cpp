/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "BGame.h"
#include "ScStack.h"
#include "ScValue.h"
#include "utils.h"
#include "scriptables/SXString.h"
#include "scriptables/SXArray.h"
#include "StringUtil.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXString, false)

//////////////////////////////////////////////////////////////////////////
CSXString::CSXString(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	_string = NULL;
	_capacity = 0;

	Stack->CorrectParams(1);
	CScValue *Val = Stack->Pop();

	if (Val->IsInt()) {
		_capacity = std::max(0, Val->GetInt());
		if (_capacity > 0) {
			_string = new char[_capacity];
			memset(_string, 0, _capacity);
		}
	} else {
		SetStringVal(Val->GetString());
	}

	if (_capacity == 0) SetStringVal("");
}


//////////////////////////////////////////////////////////////////////////
CSXString::~CSXString() {
	if (_string) delete [] _string;
}


//////////////////////////////////////////////////////////////////////////
void CSXString::SetStringVal(const char *Val) {
	int Len = strlen(Val);
	if (Len >= _capacity) {
		_capacity = Len + 1;
		delete[] _string;
		_string = NULL;
		_string = new char[_capacity];
		memset(_string, 0, _capacity);
	}
	strcpy(_string, Val);
}


//////////////////////////////////////////////////////////////////////////
char *CSXString::ScToString() {
	if (_string) return _string;
	else return "[null string]";
}


//////////////////////////////////////////////////////////////////////////
void CSXString::ScSetString(const char *Val) {
	SetStringVal(Val);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXString::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// Substring
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Substring") == 0) {
		Stack->CorrectParams(2);
		int start = Stack->Pop()->GetInt();
		int end   = Stack->Pop()->GetInt();

		if (end < start) CBUtils::Swap(&start, &end);

		try {
			WideString str;
			if (Game->m_TextEncoding == TEXT_UTF8)
				str = StringUtil::Utf8ToWide(_string);
			else
				str = StringUtil::AnsiToWide(_string);

			WideString subStr = str.substr(start, end - start + 1);

			if (Game->m_TextEncoding == TEXT_UTF8)
				Stack->PushString(StringUtil::WideToUtf8(subStr).c_str());
			else
				Stack->PushString(StringUtil::WideToAnsi(subStr).c_str());
		} catch (std::exception &) {
			Stack->PushNULL();
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Substr
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Substr") == 0) {
		Stack->CorrectParams(2);
		int start = Stack->Pop()->GetInt();

		CScValue *val = Stack->Pop();
		int len = val->GetInt();

		if (!val->IsNULL() && len <= 0) {
			Stack->PushString("");
			return S_OK;
		}

		if (val->IsNULL()) len = strlen(_string) - start;

		try {
			WideString str;
			if (Game->m_TextEncoding == TEXT_UTF8)
				str = StringUtil::Utf8ToWide(_string);
			else
				str = StringUtil::AnsiToWide(_string);

			WideString subStr = str.substr(start, len);

			if (Game->m_TextEncoding == TEXT_UTF8)
				Stack->PushString(StringUtil::WideToUtf8(subStr).c_str());
			else
				Stack->PushString(StringUtil::WideToAnsi(subStr).c_str());
		} catch (std::exception &) {
			Stack->PushNULL();
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToUpperCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ToUpperCase") == 0) {
		Stack->CorrectParams(0);

		WideString str;
		if (Game->m_TextEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		StringUtil::ToUpperCase(str);

		if (Game->m_TextEncoding == TEXT_UTF8)
			Stack->PushString(StringUtil::WideToUtf8(str).c_str());
		else
			Stack->PushString(StringUtil::WideToAnsi(str).c_str());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ToLowerCase
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ToLowerCase") == 0) {
		Stack->CorrectParams(0);

		WideString str;
		if (Game->m_TextEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		StringUtil::ToLowerCase(str);

		if (Game->m_TextEncoding == TEXT_UTF8)
			Stack->PushString(StringUtil::WideToUtf8(str).c_str());
		else
			Stack->PushString(StringUtil::WideToAnsi(str).c_str());

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IndexOf
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IndexOf") == 0) {
		Stack->CorrectParams(2);

		char *strToFind = Stack->Pop()->GetString();
		int index = Stack->Pop()->GetInt();

		WideString str;
		if (Game->m_TextEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		WideString toFind;
		if (Game->m_TextEncoding == TEXT_UTF8)
			toFind = StringUtil::Utf8ToWide(strToFind);
		else
			toFind = StringUtil::AnsiToWide(strToFind);

		int indexOf = StringUtil::IndexOf(str, toFind, index);
		Stack->PushInt(indexOf);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Split
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Split") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		char Separators[MAX_PATH] = ",";
		if (!Val->IsNULL()) strcpy(Separators, Val->GetString());

		CSXArray *Array = new CSXArray(Game);
		if (!Array) {
			Stack->PushNULL();
			return S_OK;
		}


		WideString str;
		if (Game->m_TextEncoding == TEXT_UTF8)
			str = StringUtil::Utf8ToWide(_string);
		else
			str = StringUtil::AnsiToWide(_string);

		WideString delims;
		if (Game->m_TextEncoding == TEXT_UTF8)
			delims = StringUtil::Utf8ToWide(Separators);
		else
			delims = StringUtil::AnsiToWide(Separators);

		std::vector<WideString> parts;


		size_t start, pos;
		start = 0;
		do {
			pos = str.find_first_of(delims, start);
			if (pos == start) {
				start = pos + 1;
			} else if (pos == WideString::npos) {
				parts.push_back(str.substr(start));
				break;
			} else {
				parts.push_back(str.substr(start, pos - start));
				start = pos + 1;
			}
			start = str.find_first_not_of(delims, start);

		} while (pos != WideString::npos);

		for (std::vector<WideString>::iterator it = parts.begin(); it != parts.end(); ++it) {
			WideString &part = (*it);

			if (Game->m_TextEncoding == TEXT_UTF8)
				Val = new CScValue(Game, StringUtil::WideToUtf8(part).c_str());
			else
				Val = new CScValue(Game, StringUtil::WideToAnsi(part).c_str());

			Array->Push(Val);
			delete[] Val;
			Val = NULL;
		}

		Stack->PushNative(Array, false);
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXString::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("string");
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Length (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Length") == 0) {
		if (Game->m_TextEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::Utf8ToWide(_string);
			m_ScValue->SetInt(wstr.length());
		} else
			m_ScValue->SetInt(strlen(_string));

		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Capacity") == 0) {
		m_ScValue->SetInt(_capacity);
		return m_ScValue;
	}

	else return m_ScValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXString::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Capacity
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Capacity") == 0) {
		int NewCap = Value->GetInt();
		if (NewCap < strlen(_string) + 1) Game->LOG(0, "Warning: cannot lower string capacity");
		else if (NewCap != _capacity) {
			char *NewStr = new char[NewCap];
			if (NewStr) {
				memset(NewStr, 0, NewCap);
				strcpy(NewStr, _string);
				delete[] _string;
				_string = NewStr;
				_capacity = NewCap;
			}
		}
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXString::Persist(CBPersistMgr *PersistMgr) {

	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_capacity));

	if (PersistMgr->m_Saving) {
		if (_capacity > 0) PersistMgr->PutBytes((byte  *)_string, _capacity);
	} else {
		if (_capacity > 0) {
			_string = new char[_capacity];
			PersistMgr->GetBytes((byte  *)_string, _capacity);
		} else _string = NULL;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CSXString::ScCompare(CBScriptable *Val) {
	return strcmp(_string, ((CSXString *)Val)->_string);
}

} // end of namespace WinterMute
