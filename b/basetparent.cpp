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
#include "director/lingo/xtras/b/basetparent.h"

/**************************************************
 *
 * USED IN:
 * Hrajeme si s krtkem
 *
 **************************************************/

/*
-- xtra baSetParent
new object me

* baSetParent integer Wnd, integer NewParent -- sets NewParent as parent window of Wnd

-- copyright Gary Smith, 1999
-- email: gary@mods.com.au
"
 */

namespace Director {

const char *BasetparentXtra::xlibName = "Basetparent";
const XlibFileDesc BasetparentXtra::fileNames[] = {
	{ "basetparent",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BasetparentXtra::m_new,		 0, 0,	500 },
	{ """,				BasetparentXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baSetParent", BasetparentXtra::m_baSetParent, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BasetparentXtraObject::BasetparentXtraObject(ObjectType ObjectType) :Object<BasetparentXtraObject>("Basetparent") {
	_objType = ObjectType;
}

bool BasetparentXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BasetparentXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BasetparentXtra::xlibName);
	warning("BasetparentXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BasetparentXtra::open(ObjectType type, const Common::Path &path) {
    BasetparentXtraObject::initMethods(xlibMethods);
    BasetparentXtraObject *xobj = new BasetparentXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BasetparentXtra::close(ObjectType type) {
    BasetparentXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BasetparentXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BasetparentXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BasetparentXtra::m_baSetParent, 0)
XOBJSTUB(BasetparentXtra::m_", 0)

}
