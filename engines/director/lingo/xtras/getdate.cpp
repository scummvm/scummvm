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
#include "director/lingo/xtras/getdate.h"

/**************************************************
 *
 * USED IN:
 * Fisher-Price - Big Action: Construction
 *
 **************************************************/

/*
-- xtra GetDate -- this Xtra provides access to various movie components
new object me
* getDay					-- returns current day
* getMonth				-- returns current month
* getYear				-- returns current year

 */

namespace Director {

const char *GetdateXtra::xlibName = "Getdate";
const XlibFileDesc GetdateXtra::fileNames[] = {
	{ "getdate",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				GetdateXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "getDay", GetdateXtra::m_getDay, 0, 0, 500, HBLTIN },
	{ "getMonth", GetdateXtra::m_getMonth, 0, 0, 500, HBLTIN },
	{ "getYear", GetdateXtra::m_getYear, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

GetdateXtraObject::GetdateXtraObject(ObjectType ObjectType) :Object<GetdateXtraObject>("Getdate") {
	_objType = ObjectType;
}

bool GetdateXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum GetdateXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(GetdateXtra::xlibName);
	warning("GetdateXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void GetdateXtra::open(ObjectType type, const Common::Path &path) {
    GetdateXtraObject::initMethods(xlibMethods);
    GetdateXtraObject *xobj = new GetdateXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void GetdateXtra::close(ObjectType type) {
    GetdateXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void GetdateXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("GetdateXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(GetdateXtra::m_getDay, 0)
XOBJSTUB(GetdateXtra::m_getMonth, 0)
XOBJSTUB(GetdateXtra::m_getYear, 0)

}
