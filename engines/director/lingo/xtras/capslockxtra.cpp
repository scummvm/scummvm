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
#include "director/lingo/xtras/capslockxtra.h"

/**************************************************
 *
 * USED IN:
 * Barbie as Rapunzel: A Creative Adventure
 * Butt-Ugly Martians: Martian Boot Camp
 * Fisher-Price - Power Wheels: Off Road Adventure 
 * Frankie Time Traveller
 * Rescue Heroes: Meteor Madness
 *
 **************************************************/

/*
-- xtra CapsLockXtra               -- Version 1.0
-- --------------------------------------------------------------------
-- this Xtra returns the state of the CapsLock key
-- --------------------------------------------------------------------

-- ©1997 by Stephan Eichhorn, Scirius Multimedia
-- e-mail:  xtras@scririus.com
-- WWW:     http://www.scirius.com

* GetCapsLockState                 -- returns the state of CapsLock 

 */

namespace Director {

const char *CapslockxtraXtra::xlibName = "Capslockxtra";
const XlibFileDesc CapslockxtraXtra::fileNames[] = {
	{ "capslockxtra",   nullptr },
	{ "clxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "GetCapsLockState", CapslockxtraXtra::m_GetCapsLockState, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

CapslockxtraXtraObject::CapslockxtraXtraObject(ObjectType ObjectType) :Object<CapslockxtraXtraObject>("Capslockxtra") {
	_objType = ObjectType;
}

bool CapslockxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum CapslockxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(CapslockxtraXtra::xlibName);
	warning("CapslockxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void CapslockxtraXtra::open(ObjectType type, const Common::Path &path) {
    CapslockxtraXtraObject::initMethods(xlibMethods);
    CapslockxtraXtraObject *xobj = new CapslockxtraXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void CapslockxtraXtra::close(ObjectType type) {
    CapslockxtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void CapslockxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("CapslockxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(CapslockxtraXtra::m_GetCapsLockState, 0)

}
