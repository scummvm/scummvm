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

#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/SXDate.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXDate, false)

//////////////////////////////////////////////////////////////////////////
CSXDate::CSXDate(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	Stack->CorrectParams(6);
#if 0
	memset(&_tm, 0, sizeof(_tm));

	CScValue *valYear = Stack->Pop();
	_tm.t_year = valYear->GetInt() - 1900;
	_tm.t_mon = Stack->Pop()->GetInt() - 1;
	_tm.t_mday = Stack->Pop()->GetInt();
	_tm.t_hour = Stack->Pop()->GetInt();
	_tm.t_min = Stack->Pop()->GetInt();
	_tm.t_sec = Stack->Pop()->GetInt();

	if (valYear->IsNULL()) {
		time_t TimeNow;
		time(&TimeNow);
		memcpy(&_tm, localtime(&TimeNow), sizeof(_tm));
	}

	mktime(&_tm);
#endif
}


//////////////////////////////////////////////////////////////////////////
CSXDate::~CSXDate() {

}


//////////////////////////////////////////////////////////////////////////
char *CSXDate::ScToString() {
#if 0
	return asctime(&_tm);
#endif
	return "";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
#if 0
	//////////////////////////////////////////////////////////////////////////
	// GetYear
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetYear") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_year + 1900);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMonth") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_mon + 1);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDate") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_mday);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHours") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_hour);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMinutes") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_min);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSeconds") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_sec);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetWeekday
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetWeekday") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.t_wday);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetYear
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetYear") == 0) {
		Stack->CorrectParams(1);
		_tm.t_year = Stack->Pop()->GetInt() - 1900;
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMonth") == 0) {
		Stack->CorrectParams(1);
		_tm.t_mon = Stack->Pop()->GetInt() - 1;
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetDate") == 0) {
		Stack->CorrectParams(1);
		_tm.t_mday = Stack->Pop()->GetInt();
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHours") == 0) {
		Stack->CorrectParams(1);
		_tm.t_hour = Stack->Pop()->GetInt();
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMinutes") == 0) {
		Stack->CorrectParams(1);
		_tm.t_min = Stack->Pop()->GetInt();
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSeconds") == 0) {
		Stack->CorrectParams(1);
		_tm.t_sec = Stack->Pop()->GetInt();
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetCurrentTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetCurrentTime") == 0) {
		Stack->CorrectParams(0);
		time_t TimeNow;
		time(&TimeNow);
		memcpy(&_tm, localtime(&TimeNow), sizeof(_tm));
		mktime(&_tm);
		Stack->PushNULL();
		return S_OK;
	}

	else
#endif
		return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXDate::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("date");
		return _scValue;
	}

	else return _scValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::ScSetProperty(const char *Name, CScValue *Value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(Name, "Name")==0){
	    SetName(Value->GetString());
	    return S_OK;
	}

	else*/ return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::Persist(CBPersistMgr *PersistMgr) {

	CBScriptable::Persist(PersistMgr);
#if 0
	if (PersistMgr->_saving)
		PersistMgr->PutBytes((byte *)&_tm, sizeof(_tm));
	else
		PersistMgr->GetBytes((byte *)&_tm, sizeof(_tm));
#endif
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CSXDate::ScCompare(CBScriptable *Value) {
#if 0
	time_t time1 = mktime(&_tm);
	time_t time2 = mktime(&((CSXDate *)Value)->_tm);

	if (time1 < time2) return -1;
	else if (time1 > time2) return 1;
	else
#endif
		return 0;
}

} // end of namespace WinterMute
