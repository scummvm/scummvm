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
#include "director/lingo/xtras/b/bootstrap.h"

/**************************************************
 *
 * USED IN:
 * ABC For Kids Activity Center
 *
 **************************************************/

/*
-- xtra Bootstrap -- version 8.0.0.r196
new object me
-- Bootstrap Xtra --
runURL object me, string server, string file
getStatus object me
"
 */

namespace Director {

const char *BootstrapXtra::xlibName = "Bootstrap";
const XlibFileDesc BootstrapXtra::fileNames[] = {
	{ "bootstrap",   nullptr },
	{ "shockwave updater",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BootstrapXtra::m_new,		 0, 0,	500 },
	{ "runURL",				BootstrapXtra::m_runURL,		 2, 2,	500 },
	{ "getStatus",				BootstrapXtra::m_getStatus,		 0, 0,	500 },
	{ """,				BootstrapXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BootstrapXtraObject::BootstrapXtraObject(ObjectType ObjectType) :Object<BootstrapXtraObject>("Bootstrap") {
	_objType = ObjectType;
}

bool BootstrapXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BootstrapXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BootstrapXtra::xlibName);
	warning("BootstrapXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BootstrapXtra::open(ObjectType type, const Common::Path &path) {
    BootstrapXtraObject::initMethods(xlibMethods);
    BootstrapXtraObject *xobj = new BootstrapXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BootstrapXtra::close(ObjectType type) {
    BootstrapXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BootstrapXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BootstrapXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BootstrapXtra::m_runURL, 0)
XOBJSTUB(BootstrapXtra::m_getStatus, 0)
XOBJSTUB(BootstrapXtra::m_", 0)

}
