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

	memset(&m_tm, 0, sizeof(m_tm));

	CScValue *valYear = Stack->Pop();
	m_tm.tm_year = valYear->GetInt() - 1900;
	m_tm.tm_mon = Stack->Pop()->GetInt() - 1;
	m_tm.tm_mday = Stack->Pop()->GetInt();
	m_tm.tm_hour = Stack->Pop()->GetInt();
	m_tm.tm_min = Stack->Pop()->GetInt();
	m_tm.tm_sec = Stack->Pop()->GetInt();

	if (valYear->IsNULL()) {
		time_t TimeNow;
		time(&TimeNow);
		memcpy(&m_tm, localtime(&TimeNow), sizeof(m_tm));
	}

	mktime(&m_tm);
}


//////////////////////////////////////////////////////////////////////////
CSXDate::~CSXDate() {

}


//////////////////////////////////////////////////////////////////////////
char *CSXDate::ScToString() {
	return asctime(&m_tm);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetYear
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetYear") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_year + 1900);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMonth") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_mon + 1);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDate") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_mday);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHours") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_hour);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMinutes") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_min);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSeconds") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_sec);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetWeekday
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetWeekday") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(m_tm.tm_wday);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetYear
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetYear") == 0) {
		Stack->CorrectParams(1);
		m_tm.tm_year = Stack->Pop()->GetInt() - 1900;
		mktime(&m_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMonth") == 0) {
		Stack->CorrectParams(1);
		m_tm.tm_mon = Stack->Pop()->GetInt() - 1;
		mktime(&m_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetDate") == 0) {
		Stack->CorrectParams(1);
		m_tm.tm_mday = Stack->Pop()->GetInt();
		mktime(&m_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHours") == 0) {
		Stack->CorrectParams(1);
		m_tm.tm_hour = Stack->Pop()->GetInt();
		mktime(&m_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMinutes") == 0) {
		Stack->CorrectParams(1);
		m_tm.tm_min = Stack->Pop()->GetInt();
		mktime(&m_tm);
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSeconds") == 0) {
		Stack->CorrectParams(1);
		m_tm.tm_sec = Stack->Pop()->GetInt();
		mktime(&m_tm);
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
		memcpy(&m_tm, localtime(&TimeNow), sizeof(m_tm));
		mktime(&m_tm);
		Stack->PushNULL();
		return S_OK;
	}

	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXDate::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("date");
		return m_ScValue;
	}

	else return m_ScValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::ScSetProperty(char *Name, CScValue *Value) {
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

	if (PersistMgr->m_Saving)
		PersistMgr->PutBytes((byte  *)&m_tm, sizeof(m_tm));
	else
		PersistMgr->GetBytes((byte  *)&m_tm, sizeof(m_tm));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CSXDate::ScCompare(CBScriptable *Value) {
	time_t time1 = mktime(&m_tm);
	time_t time2 = mktime(&((CSXDate *)Value)->m_tm);

	if (time1 < time2) return -1;
	else if (time1 > time2) return 1;
	else return 0;
}

} // end of namespace WinterMute
