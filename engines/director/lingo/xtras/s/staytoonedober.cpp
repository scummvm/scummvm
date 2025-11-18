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
#include "director/lingo/xtras/s/staytoonedober.h"

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
* mBuildTable integer SlotNum,integer XOff,integer YOff,integer XSize,integer YSize -- Cast Size
* mInitGame integer NumOfGuns,integer NumOfNukes -- feeds a number bewteen 1-100 to the Random# Array
* mInitGun -- feeds a number bewteen 1-100 to the Random# Array
* mOPERATEGAME integer NUM1, integer NUM2, integer NUM3,integer NUM4 -- feeds a number bewteen 1-100 to the Random# Array
* mReturnData integer NUM1, integer NUM2 -- feeds a number bewteen 1-100 to the Random# Array
* mGactions -- feeds a number bewteen 1-100 to the Random# Array
* mCastStarts integer NUM1 -- feeds a number bewteen 1-100 to the Random# Array

 */

namespace Director {

const char *StayToonedOberXtra::xlibName = "Toon";
const XlibFileDesc StayToonedOberXtra::fileNames[] = {
	{ "ober",   "staytooned" },
	{ "ober_mac",   "staytooned" },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StayToonedOberXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mInitRandom", StayToonedOberXtra::m_mInitRandom, 1, 1, 500, HBLTIN },
	{ "mBuildTable", StayToonedOberXtra::m_mBuildTable, 5, 5, 500, HBLTIN },
	{ "mInitGame", StayToonedOberXtra::m_mInitGame, 2, 2, 500, HBLTIN },
	{ "mInitGun", StayToonedOberXtra::m_mInitGun, 0, 0, 500, HBLTIN },
	{ "mOPERATEGAME", StayToonedOberXtra::m_mOPERATEGAME, 4, 4, 500, HBLTIN },
	{ "mReturnData", StayToonedOberXtra::m_mReturnData, 2, 2, 500, HBLTIN },
	{ "mGactions", StayToonedOberXtra::m_mGactions, 0, 0, 500, HBLTIN },
	{ "mCastStarts", StayToonedOberXtra::m_mCastStarts, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StayToonedOberXtraObject::StayToonedOberXtraObject(ObjectType ObjectType) :Object<StayToonedOberXtraObject>("StayToonedOber") {
	_objType = ObjectType;
}

bool StayToonedOberXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum StayToonedOberXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(StayToonedOberXtra::xlibName);
	warning("StayToonedOberXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void StayToonedOberXtra::open(ObjectType type, const Common::Path &path) {
    StayToonedOberXtraObject::initMethods(xlibMethods);
    StayToonedOberXtraObject *xobj = new StayToonedOberXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StayToonedOberXtra::close(ObjectType type) {
    StayToonedOberXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StayToonedOberXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StayToonedOberXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StayToonedOberXtra::m_mInitRandom, 0)
XOBJSTUB(StayToonedOberXtra::m_mBuildTable, 0)
XOBJSTUB(StayToonedOberXtra::m_mInitGame, 0)
XOBJSTUB(StayToonedOberXtra::m_mInitGun, 0)
XOBJSTUB(StayToonedOberXtra::m_mOPERATEGAME, 0)
XOBJSTUB(StayToonedOberXtra::m_mReturnData, 0)
XOBJSTUB(StayToonedOberXtra::m_mGactions, 0)
XOBJSTUB(StayToonedOberXtra::m_mCastStarts, 0)

}
