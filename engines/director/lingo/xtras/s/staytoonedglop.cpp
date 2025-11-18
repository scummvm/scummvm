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
#include "director/lingo/xtras/s/staytoonedglop.h"

/**************************************************
 *
 * USED IN:
 * Stay Tooned
 *
 **************************************************/

/*
-- xtra Glop -- Fridge Fight 7/15/96 Programmer: Keith Laverty FunnyBone Interactive
new object me
* mInitRandom integer SomeThing -- feeds a number bewteen 1-100 to the Random# Array
* mInitGame integer NUM1,integer NUM2,integer NUM3,integer NUM4 -- Cast Size
* mInitLevel integer NUM1 -- feeds a number bewteen 1-100 to the Random# Array
* mInitDoctor -- feeds a number bewteen 1-100 to the Random# Array
* mOPERATEGAME integer NUM1, integer NUM2, integer NUM3 -- feeds a number bewteen 1-100 to the Random# Array
* mReadData integer NUM1, integer NUM2, integer NUM3 -- feeds a number bewteen 1-100 to the Random# Array
* mGactions -- feeds a number bewteen 1-100 to the Random# Array
* mPassArray integer NUM1, integer NUM2, integer NUM3 -- feeds a number bewteen 1-100 to the Random# Array
* mPassCastSize integer NUM1, integer NUM2, integer NUM3, integer NUM4, integer NUM5 -- feeds a number bewteen 1-100 to the Random# Array

 */

namespace Director {

const char *StayToonedGlopXtra::xlibName = "Glop";
const XlibFileDesc StayToonedGlopXtra::fileNames[] = {
	{ "glop",   "staytooned" },
	{ "glop_mac",   "staytooned" },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StayToonedGlopXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mInitRandom", StayToonedGlopXtra::m_mInitRandom, 1, 1, 500, HBLTIN },
	{ "mInitGame", StayToonedGlopXtra::m_mInitGame, 4, 4, 500, HBLTIN },
	{ "mInitLevel", StayToonedGlopXtra::m_mInitLevel, 1, 1, 500, HBLTIN },
	{ "mInitDoctor", StayToonedGlopXtra::m_mInitDoctor, 0, 0, 500, HBLTIN },
	{ "mOPERATEGAME", StayToonedGlopXtra::m_mOPERATEGAME, 3, 3, 500, HBLTIN },
	{ "mReadData", StayToonedGlopXtra::m_mReadData, 3, 3, 500, HBLTIN },
	{ "mGactions", StayToonedGlopXtra::m_mGactions, 0, 0, 500, HBLTIN },
	{ "mPassArray", StayToonedGlopXtra::m_mPassArray, 3, 3, 500, HBLTIN },
	{ "mPassCastSize", StayToonedGlopXtra::m_mPassCastSize, 5, 5, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StayToonedGlopXtraObject::StayToonedGlopXtraObject(ObjectType ObjectType) :Object<StayToonedGlopXtraObject>("StayToonedGlop") {
	_objType = ObjectType;
}

bool StayToonedGlopXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum StayToonedGlopXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(StayToonedGlopXtra::xlibName);
	warning("StayToonedGlopXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void StayToonedGlopXtra::open(ObjectType type, const Common::Path &path) {
    StayToonedGlopXtraObject::initMethods(xlibMethods);
    StayToonedGlopXtraObject *xobj = new StayToonedGlopXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StayToonedGlopXtra::close(ObjectType type) {
    StayToonedGlopXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StayToonedGlopXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StayToonedGlopXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StayToonedGlopXtra::m_mInitRandom, 0)
XOBJSTUB(StayToonedGlopXtra::m_mInitGame, 0)
XOBJSTUB(StayToonedGlopXtra::m_mInitLevel, 0)
XOBJSTUB(StayToonedGlopXtra::m_mInitDoctor, 0)
XOBJSTUB(StayToonedGlopXtra::m_mOPERATEGAME, 0)
XOBJSTUB(StayToonedGlopXtra::m_mReadData, 0)
XOBJSTUB(StayToonedGlopXtra::m_mGactions, 0)
XOBJSTUB(StayToonedGlopXtra::m_mPassArray, 0)
XOBJSTUB(StayToonedGlopXtra::m_mPassCastSize, 0)

}
