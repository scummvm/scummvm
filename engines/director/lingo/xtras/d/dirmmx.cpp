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
#include "director/lingo/xtras/d/dirmmx.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9
 *
 **************************************************/

/*
-- xtra DirMMX
-- 
-- Macromedia Director MMX support Xtra
-- Copyright 1996 Macromedia Inc. and Intel Inc.
-- 
* IsDirMMXLoaded                       -- returns TRUE if the DirMMX Xtra loaded successfully
+ cpuidIsGenuineIntel  object xtraRef  -- returns TRUE if the CPU is Genuine Intel
+ cpuidGetCPUType      object xtraRef  -- returns CPU family (3=386, 4=386 etc.)
+ cpuidGetCPUStepping  object xtraRef  -- returns stepping id
+ cpuidGetCPUModel     object xtraRef  -- returns model (first model = 0x1B)
+ cpuidGetCPUFeatureFlags  object xtraRef -- returns feature flags

 */

namespace Director {

const char *DirMMXXtra::xlibName = "DirMMX";
const XlibFileDesc DirMMXXtra::fileNames[] = {
	{ "dirmmx",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DirMMXXtra::m_new,		 0, 0,	500 },
	{ "cpuidIsGenuineIntel",				DirMMXXtra::m_cpuidIsGenuineIntel,		 0, 0,	500 },
	{ "cpuidGetCPUType",				DirMMXXtra::m_cpuidGetCPUType,		 0, 0,	500 },
	{ "cpuidGetCPUStepping",				DirMMXXtra::m_cpuidGetCPUStepping,		 0, 0,	500 },
	{ "cpuidGetCPUModel",				DirMMXXtra::m_cpuidGetCPUModel,		 0, 0,	500 },
	{ "cpuidGetCPUFeatureFlags",				DirMMXXtra::m_cpuidGetCPUFeatureFlags,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "IsDirMMXLoaded", DirMMXXtra::m_IsDirMMXLoaded, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DirMMXXtraObject::DirMMXXtraObject(ObjectType ObjectType) :Object<DirMMXXtraObject>("DirMMX") {
	_objType = ObjectType;
}

bool DirMMXXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DirMMXXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DirMMXXtra::xlibName);
	warning("DirMMXXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DirMMXXtra::open(ObjectType type, const Common::Path &path) {
    DirMMXXtraObject::initMethods(xlibMethods);
    DirMMXXtraObject *xobj = new DirMMXXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DirMMXXtra::close(ObjectType type) {
    DirMMXXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DirMMXXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DirMMXXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DirMMXXtra::m_IsDirMMXLoaded, 0)
XOBJSTUB(DirMMXXtra::m_cpuidIsGenuineIntel, 0)
XOBJSTUB(DirMMXXtra::m_cpuidGetCPUType, 0)
XOBJSTUB(DirMMXXtra::m_cpuidGetCPUStepping, 0)
XOBJSTUB(DirMMXXtra::m_cpuidGetCPUModel, 0)
XOBJSTUB(DirMMXXtra::m_cpuidGetCPUFeatureFlags, 0)

}
