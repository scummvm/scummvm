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
#include "director/lingo/xtras/a/aollauncher.h"

/**************************************************
 *
 * USED IN:
 * AOL Extras
 *
 **************************************************/

/*
-- xtra Launcher
new object me
-- Template handlers --
* globalHandler -- prints global handler message
+ parentHandler  object xtraRef -- prints parent handler message
childHandler object me -- prints child handler message
LaunchInstaller object me, string str-- launches exe with cmd-line params
StatusInstaller object me -- returns status of current install
"
 */

namespace Director {

const char *LauncherXtra::xlibName = "Launcher";
const XlibFileDesc LauncherXtra::fileNames[] = {
	{ "launcher",   nullptr },
	{ "launch",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				LauncherXtra::m_new,		 0, 0,	800 },
	{ "childHandler",				LauncherXtra::m_childHandler,		 0, 0,	800 },
	{ "LaunchInstaller",				LauncherXtra::m_LaunchInstaller,		 1, 1,	800 },
	{ "StatusInstaller",				LauncherXtra::m_StatusInstaller,		 0, 0,	800 },
	{ """,				LauncherXtra::m_",		 -1, -1,	800 },
	{ "parentHandler",				LauncherXtra::m_parentHandler,		 0, 0,	800 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "globalHandler", LauncherXtra::m_globalHandler, 0, 0, 800, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

LauncherXtraObject::LauncherXtraObject(ObjectType ObjectType) :Object<LauncherXtraObject>("Launcher") {
	_objType = ObjectType;
}

bool LauncherXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum LauncherXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(LauncherXtra::xlibName);
	warning("LauncherXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void LauncherXtra::open(ObjectType type, const Common::Path &path) {
    LauncherXtraObject::initMethods(xlibMethods);
    LauncherXtraObject *xobj = new LauncherXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void LauncherXtra::close(ObjectType type) {
    LauncherXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void LauncherXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("LauncherXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(LauncherXtra::m_globalHandler, 0)
XOBJSTUB(LauncherXtra::m_parentHandler, 0)
XOBJSTUB(LauncherXtra::m_childHandler, 0)
XOBJSTUB(LauncherXtra::m_LaunchInstaller, 0)
XOBJSTUB(LauncherXtra::m_StatusInstaller, 0)
XOBJSTUB(LauncherXtra::m_", 0)

}
