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

#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/SXDate.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CSXDate, false)

//////////////////////////////////////////////////////////////////////////
CSXDate::CSXDate(CBGame *inGame, CScStack *Stack): CBScriptable(inGame) {
	Stack->CorrectParams(6);

	memset(&_tm, 0, sizeof(_tm));

	CScValue *valYear = Stack->Pop();
	_tm.tm_year = valYear->GetInt() - 1900;
	_tm.tm_mon = Stack->Pop()->GetInt() - 1;
	_tm.tm_mday = Stack->Pop()->GetInt();
	_tm.tm_hour = Stack->Pop()->GetInt();
	_tm.tm_min = Stack->Pop()->GetInt();
	_tm.tm_sec = Stack->Pop()->GetInt();

	if (valYear->IsNULL()) {
		g_system->getTimeAndDate(_tm);
	}
}


//////////////////////////////////////////////////////////////////////////
CSXDate::~CSXDate() {

}

//////////////////////////////////////////////////////////////////////////
const char *CSXDate::ScToString() {
	// TODO: Make this more stringy, and less ISO 8601-like
	_strRep.format("%04d-%02d-%02d - %02d:%02d:%02d", _tm.tm_year, _tm.tm_mon, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return _strRep.c_str();
#if 0
	return asctime(&_tm);
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetYear
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetYear") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.tm_year + 1900);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMonth") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.tm_mon + 1);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDate") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.tm_mday);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHours") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.tm_hour);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetMinutes") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.tm_min);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSeconds") == 0) {
		Stack->CorrectParams(0);
		Stack->PushInt(_tm.tm_sec);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetWeekday
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetWeekday") == 0) {
		Stack->CorrectParams(0);
		warning("GetWeekday returns a wrong value on purpose");
		Stack->PushInt(_tm.tm_mday % 7);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetYear
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetYear") == 0) {
		Stack->CorrectParams(1);
		_tm.tm_year = Stack->Pop()->GetInt() - 1900;
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMonth") == 0) {
		Stack->CorrectParams(1);
		_tm.tm_mon = Stack->Pop()->GetInt() - 1;
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetDate") == 0) {
		Stack->CorrectParams(1);
		_tm.tm_mday = Stack->Pop()->GetInt();
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHours") == 0) {
		Stack->CorrectParams(1);
		_tm.tm_hour = Stack->Pop()->GetInt();
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetMinutes") == 0) {
		Stack->CorrectParams(1);
		_tm.tm_min = Stack->Pop()->GetInt();
		Stack->PushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSeconds") == 0) {
		Stack->CorrectParams(1);
		_tm.tm_sec = Stack->Pop()->GetInt();
		Stack->PushNULL();
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetCurrentTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetCurrentTime") == 0) {
		Stack->CorrectParams(0);
		g_system->getTimeAndDate(_tm);
		Stack->PushNULL();
		return S_OK;
	}

	else
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
	if (PersistMgr->_saving)
		PersistMgr->PutBytes((byte *)&_tm, sizeof(_tm));
	else
		PersistMgr->GetBytes((byte *)&_tm, sizeof(_tm));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CSXDate::ScCompare(CBScriptable *Value) {
	TimeDate time1 = _tm;
	TimeDate time2 = ((CSXDate *)Value)->_tm;

	if (time1.tm_year < time2.tm_year) {
		return -1;
	} else if (time1.tm_year == time2.tm_year) {
		if (time1.tm_mon < time2.tm_mon) {
			return -1;
		} else if (time1.tm_mon == time2.tm_mon) {
			if (time1.tm_mday < time2.tm_mday) {
				return -1;
			} else if (time1.tm_mday == time2.tm_mday) {
				if (time1.tm_hour < time2.tm_hour) {
					return -1;
				} else if (time1.tm_hour == time2.tm_hour) {
					if (time1.tm_min < time2.tm_min) {
						return -1;
					} else if (time1.tm_min == time2.tm_min) {
						if (time1.tm_sec < time2.tm_sec) {
							return -1;
						} else if (time1.tm_sec == time2.tm_sec) {
							return 0; // Equal
						} else {
							return 1; // Sec
						}
					} else {
						return 1; // Minute
					}
				} else {
					return 1; // Hour
				}
			} else {
				return 1; // Day
			}
		} else {
			return 1; // Month
		}
	} else {
		return 1; // Year
	}
}

} // end of namespace WinterMute
