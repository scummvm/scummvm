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
