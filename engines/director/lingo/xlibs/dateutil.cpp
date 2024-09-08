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

XOBJSTUB(DateUtilXObj::m_getTime, "")
XOBJSTUB(DateUtilXObj::m_getDate, "")
XOBJSTUB(DateUtilXObj::m_getDateTime, "")
XOBJSTUB(DateUtilXObj::m_getSecond, 0)

}
