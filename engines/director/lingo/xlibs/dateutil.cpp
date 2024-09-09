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
#include "director/lingo/xlibs/dateutil.h"

/**************************************************
 *
 * USED IN:
 * Amusement Planet Phantasmagoria
 *
 **************************************************/

/*
-- DateUtil v1.0
I 	mNew		--Make a new instance
S 	mGetTime	--Get current Time String
S	mGetDate	--Get current Date String
S	mGetDateTime -- Get concatnated Date and Time String
I	mGetSecond -- Second in number
 */

namespace Director {

const char *DateUtilXObj::xlibName = "DateUtil";
const XlibFileDesc DateUtilXObj::fileNames[] = {
	{ "DateUtil",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DateUtilXObj::m_new,		 0, 0,	400 },
	{ "getTime",				DateUtilXObj::m_getTime,		 0, 0,	400 },
	{ "getDate",				DateUtilXObj::m_getDate,		 0, 0,	400 },
	{ "getDateTime",				DateUtilXObj::m_getDateTime,		 0, 0,	400 },
	{ "getSecond",				DateUtilXObj::m_getSecond,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DateUtilXObject::DateUtilXObject(ObjectType ObjectType) :Object<DateUtilXObject>("DateUtil") {
	_objType = ObjectType;
}

void DateUtilXObj::open(ObjectType type, const Common::Path &path) {
    DateUtilXObject::initMethods(xlibMethods);
    DateUtilXObject *xobj = new DateUtilXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DateUtilXObj::close(ObjectType type) {
    DateUtilXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DateUtilXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DateUtilXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

// Amusement Planet Phantasmagoria calls this method, treating its return
// value as a single string, and reading chars 1-4, 6-7, 9-10, and 12-14
// as the values we set here.
void DateUtilXObj::m_getDate(int nargs) {
	TimeDate time;
	g_system->getTimeAndDate(time);

	Common::String day;
	switch (time.tm_wday) {
	case 0:
		day = Common::String("SUN");
		break;
	case 1:
		day = Common::String("MON");
		break;
	case 2:
		day = Common::String("TUE");
		break;
	case 3:
		day = Common::String("WED");
		break;
	case 4:
		day = Common::String("THU");
		break;
	case 5:
		day = Common::String("FRI");
		break;
	case 6:
		day = Common::String("SAT");
		break;
	}

	// Phantasmagoria's get_season() suggests months start from 1.
	Common::String out = Common::String::format("%04d:%02d:%02d:%s", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, day.c_str());
	g_lingo->push(Datum(out));
}

// Amusement Planet Phantasmagoria calls this method, treating its return
// value as a single string, and reading chars 1-2, 4-5 and 7-8 as the
// values we set here.
void DateUtilXObj::m_getTime(int nargs) {
	TimeDate time;
	g_system->getTimeAndDate(time);

	Common::String out = Common::String::format("%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
	g_lingo->push(Datum(out));
}

// These two methods are never called in Phantasmagoria.
XOBJSTUB(DateUtilXObj::m_getDateTime, "")
XOBJSTUB(DateUtilXObj::m_getSecond, 0)

}
