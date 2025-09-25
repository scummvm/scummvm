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
#include "director/lingo/xtras/setmousextra.h"

/**************************************************
 *
 * USED IN:
 * Fisher-Price - Big Action: Garage
 * Fisher-Price: Little People - Christmas Activity Center 
 * Phonics 1 for Beginners (Eureka Multimedia)
 *
 **************************************************/

/*
-- xtra SetMouseXtra               -- Version 1.0
-- --------------------------------------------------------------------
-- this Xtra allows to set the mouse position through Lingo
-- --------------------------------------------------------------------

-- ©1997 by Stephan Eichhorn, Scirius Multimedia
-- e-mail:  xtras@scririus.com
-- WWW:     http://www.scirius.com

* SetMouse integer x,integer y     -- set the cursor to position x,y 
-- 
-- all x,y are in screen coordinates,
-- not relative to the stage! 
-- 

 */

namespace Director {

const char *SetmousextraXtra::xlibName = "Setmousextra";
const XlibFileDesc SetmousextraXtra::fileNames[] = {
	{ "setmousextra",   nullptr },
	{ "smxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "SetMouse", SetmousextraXtra::m_SetMouse, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SetmousextraXtraObject::SetmousextraXtraObject(ObjectType ObjectType) :Object<SetmousextraXtraObject>("Setmousextra") {
	_objType = ObjectType;
}

bool SetmousextraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SetmousextraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SetmousextraXtra::xlibName);
	warning("SetmousextraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SetmousextraXtra::open(ObjectType type, const Common::Path &path) {
    SetmousextraXtraObject::initMethods(xlibMethods);
    SetmousextraXtraObject *xobj = new SetmousextraXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SetmousextraXtra::close(ObjectType type) {
    SetmousextraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SetmousextraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SetmousextraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(SetmousextraXtra::m_SetMouse, 0)

}
