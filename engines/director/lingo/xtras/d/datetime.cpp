/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/d/datetime.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 2 / 3 / 4 / 5 / 6 / 7 / 8 / Adventskalender / Spielebox
 *
 **************************************************/

/*
-- xtra DateTimeXtra    -- Version 1.0.1 (32 Bit)

-- --------------------------------------------------------------------
-- this Xtra returns date and time related information
-- --------------------------------------------------------------------

-- �1997 by Stephan Eichhorn, Scirius Multimedia
-- e-mail:  xtras@scririus.com
-- WWW:     http://www.scirius.com

-- global methods --
--------------------------------------------------------
* GetSeconds         -- get the seconds (0...59) 
* GetMinutes         -- get the minutes (0...59) 
* GetHours           -- get the hours (0...23) 
* GetDay             -- get the day (1...31) 
* GetMonth           -- get the month (1...12) 
* GetYear            -- get the year  
* GetWeekDay         -- get the weekday (0...6) 0=sunday
--------------------------------------------------------


 */

namespace Director {

const char *DateTimeXtra::xlibName = "DateTime";
const XlibFileDesc DateTimeXtra::fileNames[] = {
	{ "DATETIME",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {


	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "GetSeconds", DateTimeXtra::m_GetSeconds, 0, 0, 500, HBLTIN },
	{ "GetMinutes", DateTimeXtra::m_GetMinutes, 0, 0, 500, HBLTIN },
	{ "GetHours", DateTimeXtra::m_GetHours, 0, 0, 500, HBLTIN },
	{ "GetDay", DateTimeXtra::m_GetDay, 0, 0, 500, HBLTIN },
	{ "GetMonth", DateTimeXtra::m_GetMonth, 0, 0, 500, HBLTIN },
	{ "GetYear", DateTimeXtra::m_GetYear, 0, 0, 500, HBLTIN },
	{ "GetWeekDay", DateTimeXtra::m_GetWeekDay, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DateTimeXtraObject::DateTimeXtraObject(ObjectType ObjectType) :Object<DateTimeXtraObject>("DateTime") {
	_objType = ObjectType;
}

bool DateTimeXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DateTimeXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DateTimeXtra::xlibName);
	warning("DateTimeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DateTimeXtra::open(ObjectType type, const Common::Path &path) {
    DateTimeXtraObject::initMethods(xlibMethods);
    DateTimeXtraObject *xobj = new DateTimeXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DateTimeXtra::close(ObjectType type) {
    DateTimeXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DateTimeXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DateTimeXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void DateTimeXtra::m_GetSeconds(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_sec));
}

void DateTimeXtra::m_GetMinutes(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_min));
}

void DateTimeXtra::m_GetHours(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_hour));
}

void DateTimeXtra::m_GetDay(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_mday));
}

void DateTimeXtra::m_GetMonth(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_mon + 1));
}

void DateTimeXtra::m_GetYear(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_year + 1900));
}

void DateTimeXtra::m_GetWeekDay(int nargs) {
	g_lingo->dropStack(nargs);
	TimeDate td;
	g_system->getTimeAndDate(td);
	g_lingo->push(Datum(td.tm_wday));
}

}
