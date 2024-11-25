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
#include "director/lingo/xtras/timextra.h"

/**************************************************
 *
 * USED IN:
 * Safecracker
 *
 **************************************************/

/*
-- xtra TimeXtra
new object me, any
* getTime -- add two numbers 

 */

namespace Director {

const char *const TimextraXtra::xlibName = "Timextra";
const XlibFileDesc TimextraXtra::fileNames[] = {
	{ "timextra",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				TimextraXtra::m_new,		 1, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ "getTime", TimextraXtra::m_getTime, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

TimextraXtraObject::TimextraXtraObject(ObjectType ObjectType) :Object<TimextraXtraObject>("Timextra") {
	_objType = ObjectType;
}

bool TimextraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum TimextraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(TimextraXtra::xlibName);
	warning("TimextraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void TimextraXtra::open(ObjectType type, const Common::Path &path) {
    TimextraXtraObject::initMethods(xlibMethods);
    TimextraXtraObject *xobj = new TimextraXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void TimextraXtra::close(ObjectType type) {
    TimextraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void TimextraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("TimextraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(TimextraXtra::m_getTime, 0)

}
