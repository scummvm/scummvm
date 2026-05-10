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
#include "director/lingo/xtras/s/staytoonedhigh.h"

/**************************************************
 *
 * USED IN:
 * Stay Tooned
 *
 **************************************************/

/*
-- xtra Toon -- Fridge Fight 7/15/96 Programmer: Keith Laverty FunnyBone Interactive
new object me
* mInitRandom integer SomeThing -- feeds a number bewteen 1-100 to the Random# Array
* mInitGame integer NUM1,integer NUM2,integer NUM3,integer NUM4,integer NUM5 -- Cast Size
* mInitLevel integer NUM1 -- feeds a number bewteen 1-100 to the Random# Array
* mInitBall -- feeds a number bewteen 1-100 to the Random# Array
* mOPERATEGAME integer NUM1, integer NUM2 -- feeds a number bewteen 1-100 to the Random# Array
* mReturnData integer NUM1, integer NUM2, integer NUM3 -- feeds a number bewteen 1-100 to the Random# Array
* mGactions -- feeds a number bewteen 1-100 to the Random# Array

 */

namespace Director {

const char *StayToonedHighXtra::xlibName = "Toon";
const XlibFileDesc StayToonedHighXtra::fileNames[] = {
	{ "high",   nullptr },
	{ "high_mac",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StayToonedHighXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mInitRandom", StayToonedHighXtra::m_mInitRandom, 1, 1, 500, HBLTIN },
	{ "mInitGame", StayToonedHighXtra::m_mInitGame, 5, 5, 500, HBLTIN },
	{ "mInitLevel", StayToonedHighXtra::m_mInitLevel, 1, 1, 500, HBLTIN },
	{ "mInitBall", StayToonedHighXtra::m_mInitBall, 0, 0, 500, HBLTIN },
	{ "mOPERATEGAME", StayToonedHighXtra::m_mOPERATEGAME, 2, 2, 500, HBLTIN },
	{ "mReturnData", StayToonedHighXtra::m_mReturnData, 3, 3, 500, HBLTIN },
	{ "mGactions", StayToonedHighXtra::m_mGactions, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StayToonedHighXtraObject::StayToonedHighXtraObject(ObjectType ObjectType) :Object<StayToonedHighXtraObject>("StayToonedHigh") {
	_objType = ObjectType;
}

bool StayToonedHighXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum StayToonedHighXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(StayToonedHighXtra::xlibName);
	warning("StayToonedHighXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void StayToonedHighXtra::open(ObjectType type, const Common::Path &path) {
    StayToonedHighXtraObject::initMethods(xlibMethods);
    StayToonedHighXtraObject *xobj = new StayToonedHighXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StayToonedHighXtra::close(ObjectType type) {
    StayToonedHighXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StayToonedHighXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StayToonedHighXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StayToonedHighXtra::m_mInitRandom, 0)
XOBJSTUB(StayToonedHighXtra::m_mInitGame, 0)
XOBJSTUB(StayToonedHighXtra::m_mInitLevel, 0)
XOBJSTUB(StayToonedHighXtra::m_mInitBall, 0)
XOBJSTUB(StayToonedHighXtra::m_mOPERATEGAME, 0)
XOBJSTUB(StayToonedHighXtra::m_mReturnData, 0)
XOBJSTUB(StayToonedHighXtra::m_mGactions, 0)

}
