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

CBScriptable *makeSXDate(CBGame *inGame, CScStack *stack) {
	return new CSXDate(inGame, stack);
}

//////////////////////////////////////////////////////////////////////////
CSXDate::CSXDate(CBGame *inGame, CScStack *stack): CBScriptable(inGame) {
	stack->correctParams(6);

	memset(&_tm, 0, sizeof(_tm));

	CScValue *valYear = stack->pop();
	_tm.tm_year = valYear->getInt() - 1900;
	_tm.tm_mon = stack->pop()->getInt() - 1;
	_tm.tm_mday = stack->pop()->getInt();
	_tm.tm_hour = stack->pop()->getInt();
	_tm.tm_min = stack->pop()->getInt();
	_tm.tm_sec = stack->pop()->getInt();

	if (valYear->isNULL()) {
		g_system->getTimeAndDate(_tm);
	}
}


//////////////////////////////////////////////////////////////////////////
CSXDate::~CSXDate() {

}

//////////////////////////////////////////////////////////////////////////
const char *CSXDate::scToString() {
	// TODO: Make this more stringy, and less ISO 8601-like
	_strRep.format("%04d-%02d-%02d - %02d:%02d:%02d", _tm.tm_year, _tm.tm_mon, _tm.tm_mday, _tm.tm_hour, _tm.tm_min, _tm.tm_sec);
	return _strRep.c_str();
#if 0
	return asctime(&_tm);
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetYear
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetYear") == 0) {
		stack->correctParams(0);
		stack->pushInt(_tm.tm_year + 1900);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMonth") == 0) {
		stack->correctParams(0);
		stack->pushInt(_tm.tm_mon + 1);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDate") == 0) {
		stack->correctParams(0);
		stack->pushInt(_tm.tm_mday);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHours") == 0) {
		stack->correctParams(0);
		stack->pushInt(_tm.tm_hour);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetMinutes") == 0) {
		stack->correctParams(0);
		stack->pushInt(_tm.tm_min);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSeconds") == 0) {
		stack->correctParams(0);
		stack->pushInt(_tm.tm_sec);
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// GetWeekday
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetWeekday") == 0) {
		stack->correctParams(0);
		warning("GetWeekday returns a wrong value on purpose");
		stack->pushInt(_tm.tm_mday % 7);
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetYear
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetYear") == 0) {
		stack->correctParams(1);
		_tm.tm_year = stack->pop()->getInt() - 1900;
		stack->pushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMonth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMonth") == 0) {
		stack->correctParams(1);
		_tm.tm_mon = stack->pop()->getInt() - 1;
		stack->pushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetDate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetDate") == 0) {
		stack->correctParams(1);
		_tm.tm_mday = stack->pop()->getInt();
		stack->pushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetHours
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetHours") == 0) {
		stack->correctParams(1);
		_tm.tm_hour = stack->pop()->getInt();
		stack->pushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetMinutes
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetMinutes") == 0) {
		stack->correctParams(1);
		_tm.tm_min = stack->pop()->getInt();
		stack->pushNULL();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// SetSeconds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSeconds") == 0) {
		stack->correctParams(1);
		_tm.tm_sec = stack->pop()->getInt();
		stack->pushNULL();
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetCurrentTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetCurrentTime") == 0) {
		stack->correctParams(0);
		g_system->getTimeAndDate(_tm);
		stack->pushNULL();
		return S_OK;
	}

	else
		return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
CScValue *CSXDate::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("date");
		return _scValue;
	}

	else return _scValue;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::scSetProperty(const char *name, CScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if(strcmp(name, "Name")==0){
	    setName(value->getString());
	    return S_OK;
	}

	else*/ return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CSXDate::persist(CBPersistMgr *persistMgr) {

	CBScriptable::persist(persistMgr);
	persistMgr->transfer(TMEMBER(_tm.tm_year));
	persistMgr->transfer(TMEMBER(_tm.tm_mon));
	persistMgr->transfer(TMEMBER(_tm.tm_mday));
	persistMgr->transfer(TMEMBER(_tm.tm_hour));
	persistMgr->transfer(TMEMBER(_tm.tm_min));
	persistMgr->transfer(TMEMBER(_tm.tm_sec));
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
int CSXDate::scCompare(CBScriptable *Value) {
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
