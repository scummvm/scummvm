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
#include "director/lingo/xtras/a/aolrestart.h"

/**************************************************
 *
 * USED IN:
 * AOL Extras
 *
 **************************************************/

/*
-- xtra Restart
new object me
-- Template handlers --
* globalHandler -- prints global handler message
+ parentHandler  object xtraRef -- prints parent handler message
childHandler object me -- prints child handler message
RestartWindows object me -- Force the machine to restart
"
 */

namespace Director {

const char *RestartXtra::xlibName = "Restart";
const XlibFileDesc RestartXtra::fileNames[] = {
	{ "restart",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				RestartXtra::m_new,		 0, 0,	800 },
	{ "childHandler",				RestartXtra::m_childHandler,		 0, 0,	800 },
	{ "RestartWindows",				RestartXtra::m_RestartWindows,		 0, 0,	800 },
	{ """,				RestartXtra::m_",		 -1, -1,	800 },
	{ "parentHandler",				RestartXtra::m_parentHandler,		 0, 0,	800 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "globalHandler", RestartXtra::m_globalHandler, 0, 0, 800, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

RestartXtraObject::RestartXtraObject(ObjectType ObjectType) :Object<RestartXtraObject>("Restart") {
	_objType = ObjectType;
}

bool RestartXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum RestartXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(RestartXtra::xlibName);
	warning("RestartXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void RestartXtra::open(ObjectType type, const Common::Path &path) {
    RestartXtraObject::initMethods(xlibMethods);
    RestartXtraObject *xobj = new RestartXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void RestartXtra::close(ObjectType type) {
    RestartXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void RestartXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("RestartXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(RestartXtra::m_globalHandler, 0)
XOBJSTUB(RestartXtra::m_parentHandler, 0)
XOBJSTUB(RestartXtra::m_childHandler, 0)
XOBJSTUB(RestartXtra::m_RestartWindows, 0)
XOBJSTUB(RestartXtra::m_", 0)

}
