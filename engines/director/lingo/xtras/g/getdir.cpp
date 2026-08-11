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
#include "director/lingo/xtras/g/getdir.h"

/**************************************************
 *
 * USED IN:
 * pinkgear
 *
 **************************************************/

/*
-- xtra GetDir
new object me
forget object me
-- GetDirXtra --
GetDirOne object me, int repeat -- GetDir n times using default system GetDir
* GetDirTwo -- global level GetDir method: GetDirs once
+ GetDirThree  object xtraRef -- xtra level version of above method
* GetWindowsDir -- return path to windows directory
* GetSystemDir -- return path to windows system directory

 */

namespace Director {

const char *GetDirXtra::xlibName = "GetDir";
const XlibFileDesc GetDirXtra::fileNames[] = {
	{ "getdir",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				GetDirXtra::m_new,		 0, 0,	500 },
	{ "forget",				GetDirXtra::m_forget,		 0, 0,	500 },
	{ "GetDirOne",				GetDirXtra::m_GetDirOne,		 1, 1,	500 },
	{ "GetDirThree",				GetDirXtra::m_GetDirThree,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "GetDirTwo", GetDirXtra::m_GetDirTwo, 0, 0, 500, HBLTIN },
	{ "GetWindowsDir", GetDirXtra::m_GetWindowsDir, 0, 0, 500, HBLTIN },
	{ "GetSystemDir", GetDirXtra::m_GetSystemDir, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

GetDirXtraObject::GetDirXtraObject(ObjectType ObjectType) :Object<GetDirXtraObject>("GetDir") {
	_objType = ObjectType;
}

bool GetDirXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum GetDirXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(GetDirXtra::xlibName);
	warning("GetDirXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void GetDirXtra::open(ObjectType type, const Common::Path &path) {
    GetDirXtraObject::initMethods(xlibMethods);
    GetDirXtraObject *xobj = new GetDirXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void GetDirXtra::close(ObjectType type) {
    GetDirXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void GetDirXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("GetDirXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(GetDirXtra::m_forget, 0)
XOBJSTUB(GetDirXtra::m_GetDirOne, 0)
XOBJSTUB(GetDirXtra::m_GetDirTwo, 0)
XOBJSTUB(GetDirXtra::m_GetDirThree, 0)
XOBJSTUB(GetDirXtra::m_GetWindowsDir, Datum("C:\\"))
XOBJSTUB(GetDirXtra::m_GetSystemDir, Datum("@:"))

}
