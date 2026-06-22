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
#include "director/lingo/xtras/border.h"

/**************************************************
 *
 * USED IN:
 * Interactive Owner Experience: Lincoln 2004 Model Town Car
 *
 **************************************************/

/*
-- --Border: change stage border status [(C)1997 Neuland]
xtra Border
+ new object me
+ register object me, string code
-- methods:
getAppWin object me                                       -- gets application window (result: int hwnd)
getStageWin object me, integer appwin                     -- gets stage window (result: int hwnd)
getMovieWin object me, integer appwin, string movietitle  -- gets movie in a window (result: int hwnd)
setBorder object me, integer hwnd, integer onoff          -- sets border

 */

namespace Director {

const char *ChangeXtra::xlibName = "Change";
const XlibFileDesc ChangeXtra::fileNames[] = {
	{ "change",   nullptr },
	{ "border",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "xtra",				ChangeXtra::m_xtra,		 0, 0,	500 },
	{ "getAppWin",				ChangeXtra::m_getAppWin,		 0, 0,	500 },
	{ "getStageWin",				ChangeXtra::m_getStageWin,		 1, 0,	500 },
	{ "getMovieWin",				ChangeXtra::m_getMovieWin,		 2, 0,	500 },
	{ "setBorder",				ChangeXtra::m_setBorder,		 2, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ChangeXtraObject::ChangeXtraObject(ObjectType ObjectType) :Object<ChangeXtraObject>("Change") {
	_objType = ObjectType;
}

bool ChangeXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ChangeXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ChangeXtra::xlibName);
	warning("ChangeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ChangeXtra::open(ObjectType type, const Common::Path &path) {
    ChangeXtraObject::initMethods(xlibMethods);
    ChangeXtraObject *xobj = new ChangeXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ChangeXtra::close(ObjectType type) {
    ChangeXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ChangeXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ChangeXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ChangeXtra::m_xtra, 0)
XOBJSTUB(ChangeXtra::m_register, 0)
XOBJSTUB(ChangeXtra::m_getAppWin, 0)
XOBJSTUB(ChangeXtra::m_getStageWin, 0)
XOBJSTUB(ChangeXtra::m_getMovieWin, 0)
XOBJSTUB(ChangeXtra::m_setBorder, 0)

}
