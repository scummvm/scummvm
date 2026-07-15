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
 * This program is distributed /in the hope that it will be useful,
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
#include "director/lingo/xtras/s/setmouse.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 2 / 3 / 4
 * Webmaster
 * Janosch Panama - Oh wie schön ist Panama
 *
 **************************************************/

/*
-- xtra SetMouseXtra               -- Version 1.0
-- --------------------------------------------------------------------
-- this Xtra allows to set the mouse position through Lingo
-- --------------------------------------------------------------------

-- �1997 by Stephan Eichhorn, Scirius Multimedia
-- e-mail:  xtras@scririus.com
-- WWW:     http://www.scirius.com

* SetMouse integer x,integer y     -- set the cursor to position x,y 
-- 
-- all x,y are in screen coordinates,
-- not relative to the stage! 
-- 

 */

namespace Director {

const char *SetMouseXtra::xlibName = "SetMouse";
const XlibFileDesc SetMouseXtra::fileNames[] = {
	{ "SMXTRA",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {


	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "SetMouse", SetMouseXtra::m_SetMouse, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SetMouseXtraObject::SetMouseXtraObject(ObjectType ObjectType) :Object<SetMouseXtraObject>("SetMouse") {
	_objType = ObjectType;
}

bool SetMouseXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SetMouseXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SetMouseXtra::xlibName);
	warning("SetMouseXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SetMouseXtra::open(ObjectType type, const Common::Path &path) {
    SetMouseXtraObject::initMethods(xlibMethods);
    SetMouseXtraObject *xobj = new SetMouseXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SetMouseXtra::close(ObjectType type) {
    SetMouseXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SetMouseXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SetMouseXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void SetMouseXtra::m_SetMouse(int nargs) {
	if (nargs != 2) {
		warning("SetMouseXtra::m_SetMouse: expected 2 arguments, got %d", nargs);
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	int y = g_lingo->pop().asInt();
	int x = g_lingo->pop().asInt();
	g_system->warpMouse(x, y);
	g_lingo->push(Datum(0));
}

}
