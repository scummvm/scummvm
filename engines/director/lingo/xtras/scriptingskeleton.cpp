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
#include "director/lingo/xtras/scriptingskeleton.h"

/**************************************************
 *
 * USED IN:
 * Barbie as Rapunzel: A Creative Adventure
 * Rescue Heroes: Lava Landslide
 * Rescue Heroes: Tremor Trouble
 *
 **************************************************/

/*
-- xtra ScriptingSkeleton
new object me
-- Template handlers --
* getCPUspeed -- prints global handler message
+ parentHandler  object xtraRef -- prints parent handler message
childHandler object me -- prints child handler message

 */

namespace Director {

const char *ScriptingskeletonXtra::xlibName = "Scriptingskeleton";
const XlibFileDesc ScriptingskeletonXtra::fileNames[] = {
	{ "scriptingskeleton",   nullptr },
	{ "havSpeed",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ScriptingskeletonXtra::m_new,		 0, 0,	500 },
	{ "childHandler",				ScriptingskeletonXtra::m_childHandler,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "getCPUspeed", ScriptingskeletonXtra::m_getCPUspeed, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ScriptingskeletonXtraObject::ScriptingskeletonXtraObject(ObjectType ObjectType) :Object<ScriptingskeletonXtraObject>("Scriptingskeleton") {
	_objType = ObjectType;
}

bool ScriptingskeletonXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ScriptingskeletonXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ScriptingskeletonXtra::xlibName);
	warning("ScriptingskeletonXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ScriptingskeletonXtra::open(ObjectType type, const Common::Path &path) {
    ScriptingskeletonXtraObject::initMethods(xlibMethods);
    ScriptingskeletonXtraObject *xobj = new ScriptingskeletonXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ScriptingskeletonXtra::close(ObjectType type) {
    ScriptingskeletonXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ScriptingskeletonXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ScriptingskeletonXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ScriptingskeletonXtra::m_getCPUspeed, 0)
XOBJSTUB(ScriptingskeletonXtra::m_parentHandler, 0)
XOBJSTUB(ScriptingskeletonXtra::m_childHandler, 0)

}
