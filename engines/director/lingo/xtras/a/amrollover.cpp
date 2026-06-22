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
#include "director/lingo/xtras/amrollover.h"

/**************************************************
 *
 * USED IN:
 * Noddy: Let's Get Ready For School
 * Thunderbirds: F.A.B. Action Pack
 * Yoho Ahoy! All Aboard
 *
 **************************************************/

/*
-- xtra AMRollover
new object me
-- AMRollover --
+ loopBack object xtraRef, string -- returns a string
+ addPolygon object xtraRef -- initialises an empty polygon
+ getNoOfPolygons object xtraRef -- returns the number of polygons
+ getNoOfVertices object xtraRef -- returns the number of vertices in the current polygon
+ addVertice object xtraRef, integer, integer -- adds a vertice to the current polygon
+ getPolygon object xtraRef -- returns the current polygon as a string
+ isPointInPolygon object xtraRef, integer n, integer x, integer y -- returns true if point (x,y) is in the nth polygon
+ getPolygonRollover object xtraRef, integer x, integer y -- returns the number of the polygon the point (x,y)is in
+ status object xtraRef -- returns error code of last call
+ error object xtraRef, integer -- returns the error code as a string
+ getVersion object xtraRef -- returns the current version of the xtra

 */

namespace Director {

const char *AmrolloverXtra::xlibName = "Amrollover";
const XlibFileDesc AmrolloverXtra::fileNames[] = {
	{ "amrollover",   nullptr },
	{ "amroll",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AmrolloverXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AmrolloverXtraObject::AmrolloverXtraObject(ObjectType ObjectType) :Object<AmrolloverXtraObject>("Amrollover") {
	_objType = ObjectType;
}

bool AmrolloverXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AmrolloverXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AmrolloverXtra::xlibName);
	warning("AmrolloverXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AmrolloverXtra::open(ObjectType type, const Common::Path &path) {
    AmrolloverXtraObject::initMethods(xlibMethods);
    AmrolloverXtraObject *xobj = new AmrolloverXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AmrolloverXtra::close(ObjectType type) {
    AmrolloverXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AmrolloverXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AmrolloverXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AmrolloverXtra::m_loopBack, 0)
XOBJSTUB(AmrolloverXtra::m_addPolygon, 0)
XOBJSTUB(AmrolloverXtra::m_getNoOfPolygons, 0)
XOBJSTUB(AmrolloverXtra::m_getNoOfVertices, 0)
XOBJSTUB(AmrolloverXtra::m_addVertice, 0)
XOBJSTUB(AmrolloverXtra::m_getPolygon, 0)
XOBJSTUB(AmrolloverXtra::m_isPointInPolygon, 0)
XOBJSTUB(AmrolloverXtra::m_getPolygonRollover, 0)
XOBJSTUB(AmrolloverXtra::m_status, 0)
XOBJSTUB(AmrolloverXtra::m_error, 0)
XOBJSTUB(AmrolloverXtra::m_getVersion, 0)

}
