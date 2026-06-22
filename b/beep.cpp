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
#include "director/lingo/xtras/beep.h"

/**************************************************
 *
 * USED IN:
 * Home Depot Home Improvement 1-2-3
 *
 **************************************************/

/*
-- xtra beep
new object me
+ ReturnWindowHandle  object xtraRef -- xtra level version of above method

 */

namespace Director {

const char *BeepXtra::xlibName = "Beep";
const XlibFileDesc BeepXtra::fileNames[] = {
	{ "beep",   nullptr },
	{ "winhand",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BeepXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BeepXtraObject::BeepXtraObject(ObjectType ObjectType) :Object<BeepXtraObject>("Beep") {
	_objType = ObjectType;
}

bool BeepXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BeepXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BeepXtra::xlibName);
	warning("BeepXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BeepXtra::open(ObjectType type, const Common::Path &path) {
    BeepXtraObject::initMethods(xlibMethods);
    BeepXtraObject *xobj = new BeepXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BeepXtra::close(ObjectType type) {
    BeepXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BeepXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BeepXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BeepXtra::m_ReturnWindowHandle, 0)

}
