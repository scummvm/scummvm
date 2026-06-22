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
#include "director/lingo/xtras/a/astar.h"

/**************************************************
 *
 * USED IN:
 * Istiden
 *
 **************************************************/

/*
-- xtra Astar
-------------------------------------------------------------------
                         AStar xtra v1.0
               (C)opyright 2001 Vision Park Studios.
-------------------------------------------------------------------
new object me, integer width, integer height
-- Creates a new aStar xtra with the map dimensions width*height

-------------------------------------------------------------------
setForbidden object me, point location
-- Marks a location as forbidden

-------------------------------------------------------------------
isForbidden object me, point location
-- Returns 1 (true) if the location is forbidden, else 0 (false)

-------------------------------------------------------------------
setCost object me, point location, float cost
-- Sets the cost to walk on a tile. Cost must be higher than zero.

-------------------------------------------------------------------
getCost object me, point location
-- Returns the cost to walk on a tile

-------------------------------------------------------------------
getPath object me, point sourceLocation, point destinationLocation
-- Returns a list of points describing the path with the smallest
-- cost from sourceLocation to destinationLocation or void if no
-- path could be found

"
 */

namespace Director {

const char *AstarXtra::xlibName = "Astar";
const XlibFileDesc AstarXtra::fileNames[] = {
	{ "astar",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AstarXtra::m_new,		 2, 2,	800 },
	{ "setForbidden",				AstarXtra::m_setForbidden,		 1, 1,	800 },
	{ "isForbidden",				AstarXtra::m_isForbidden,		 1, 1,	800 },
	{ "setCost",				AstarXtra::m_setCost,		 2, 2,	800 },
	{ "getCost",				AstarXtra::m_getCost,		 1, 1,	800 },
	{ "getPath",				AstarXtra::m_getPath,		 2, 2,	800 },
	{ """,				AstarXtra::m_",		 -1, -1,	800 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AstarXtraObject::AstarXtraObject(ObjectType ObjectType) :Object<AstarXtraObject>("Astar") {
	_objType = ObjectType;
}

bool AstarXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AstarXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AstarXtra::xlibName);
	warning("AstarXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AstarXtra::open(ObjectType type, const Common::Path &path) {
    AstarXtraObject::initMethods(xlibMethods);
    AstarXtraObject *xobj = new AstarXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AstarXtra::close(ObjectType type) {
    AstarXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AstarXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AstarXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AstarXtra::m_AStar, 0)
XOBJSTUB(AstarXtra::m_(C)opyright, 0)
XOBJSTUB(AstarXtra::m_setForbidden, 0)
XOBJSTUB(AstarXtra::m_isForbidden, 0)
XOBJSTUB(AstarXtra::m_setCost, 0)
XOBJSTUB(AstarXtra::m_getCost, 0)
XOBJSTUB(AstarXtra::m_getPath, 0)
XOBJSTUB(AstarXtra::m_", 0)

}
