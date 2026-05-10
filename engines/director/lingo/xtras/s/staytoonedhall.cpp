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
#include "director/lingo/xtras/s/staytoonedhall.h"

/**************************************************
 *
 * USED IN:
 * Stay Tooned
 *
 **************************************************/

/*
-- xtra Hall -- Remote Combat Game 7/8/96 Programmer: Keith Laverty
new object me
* mInitRandom integer SomeThing -- feeds a number bewteen 1-100 to the Random# Array
* mInitGame integer NUM1, integer NUM2, integer NUM3-- feeds a number bewteen 1-100 to the Random# Array
* mInitLevel integer NUM1-- feeds a number bewteen 1-100 to the Random# Array
* mOPERATEGAME integer NUM1, integer NUM2, integer NUM3-- feeds a number bewteen 1-100 to the Random# Array
* mReadData integer NUM1, integer NUM2, integer NUM3-- feeds a number bewteen 1-100 to the Random# Array
* mGactions -- feeds a number bewteen 1-100 to the Random# Array
* mPassArray integer NUM1, integer NUM2, integer NUM3-- feeds a number bewteen 1-100 to the Random# Array
* mPassCastSize integer NUM1, integer NUM2, integer NUM3, integer NUM4, integer NUM5-- feeds a number bewteen 1-100 to the Random# Array

 */

namespace Director {

const char *StayToonedHallXtra::xlibName = "Hall";
const XlibFileDesc StayToonedHallXtra::fileNames[] = {
	{ "hall",   "staytooned" },
	{ "hall_mac",   "staytooned" },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StayToonedHallXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mInitRandom", StayToonedHallXtra::m_mInitRandom, 1, 1, 500, HBLTIN },
	{ "mInitGame", StayToonedHallXtra::m_mInitGame, 3, 3, 500, HBLTIN },
	{ "mInitLevel", StayToonedHallXtra::m_mInitLevel, 1, 1, 500, HBLTIN },
	{ "mOPERATEGAME", StayToonedHallXtra::m_mOPERATEGAME, 3, 3, 500, HBLTIN },
	{ "mReadData", StayToonedHallXtra::m_mReadData, 3, 3, 500, HBLTIN },
	{ "mGactions", StayToonedHallXtra::m_mGactions, 0, 0, 500, HBLTIN },
	{ "mPassArray", StayToonedHallXtra::m_mPassArray, 3, 3, 500, HBLTIN },
	{ "mPassCastSize", StayToonedHallXtra::m_mPassCastSize, 5, 5, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StayToonedHallXtraObject::StayToonedHallXtraObject(ObjectType ObjectType) :Object<StayToonedHallXtraObject>("StayToonedHall") {
	_objType = ObjectType;
}

bool StayToonedHallXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum StayToonedHallXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(StayToonedHallXtra::xlibName);
	warning("StayToonedHallXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void StayToonedHallXtra::open(ObjectType type, const Common::Path &path) {
    StayToonedHallXtraObject::initMethods(xlibMethods);
    StayToonedHallXtraObject *xobj = new StayToonedHallXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StayToonedHallXtra::close(ObjectType type) {
    StayToonedHallXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StayToonedHallXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StayToonedHallXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StayToonedHallXtra::m_mInitRandom, 0)
XOBJSTUB(StayToonedHallXtra::m_mInitGame, 0)
XOBJSTUB(StayToonedHallXtra::m_mInitLevel, 0)
XOBJSTUB(StayToonedHallXtra::m_mOPERATEGAME, 0)
XOBJSTUB(StayToonedHallXtra::m_mReadData, 0)
XOBJSTUB(StayToonedHallXtra::m_mGactions, 0)
XOBJSTUB(StayToonedHallXtra::m_mPassArray, 0)
XOBJSTUB(StayToonedHallXtra::m_mPassCastSize, 0)

}
