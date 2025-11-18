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
#include "director/lingo/xtras/s/staytoonedball.h"

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
* mInitGame integer NUM1,integer NUM2 -- Cast Size
* mOPERATEGAME integer NUM1, integer NUM2, integer NUM3, integer NUM4 -- feeds a number bewteen 1-100 to the Random# Array
* mGactions -- feeds a number bewteen 1-100 to the Random# Array
* mGetInfo integer NUM1, integer NUM2, integer NUM3 -- feeds a number bewteen 1-100 to the Random# Array
* mSetSize integer NUM1,integer NUM2,integer NUM3,integer NUM4,integer NUM5,integer NUM6 -- Cast Size
* mAddStuff integer NUM1,integer NUM2 -- feeds a number bewteen 1-100 to the Random# Array

 */

namespace Director {

const char *StayToonedBallXtra::xlibName = "Toon";
const XlibFileDesc StayToonedBallXtra::fileNames[] = {
	{ "ball",   "staytooned" },
	{ "ball_mac",   "staytooned" },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StayToonedBallXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mInitRandom", StayToonedBallXtra::m_mInitRandom, 1, 1, 500, HBLTIN },
	{ "mInitGame", StayToonedBallXtra::m_mInitGame, 2, 2, 500, HBLTIN },
	{ "mOPERATEGAME", StayToonedBallXtra::m_mOPERATEGAME, 4, 4, 500, HBLTIN },
	{ "mGactions", StayToonedBallXtra::m_mGactions, 0, 0, 500, HBLTIN },
	{ "mGetInfo", StayToonedBallXtra::m_mGetInfo, 3, 3, 500, HBLTIN },
	{ "mSetSize", StayToonedBallXtra::m_mSetSize, 6, 6, 500, HBLTIN },
	{ "mAddStuff", StayToonedBallXtra::m_mAddStuff, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StayToonedBallXtraObject::StayToonedBallXtraObject(ObjectType ObjectType) :Object<StayToonedBallXtraObject>("StayToonedBall") {
	_objType = ObjectType;
}

bool StayToonedBallXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum StayToonedBallXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(StayToonedBallXtra::xlibName);
	warning("StayToonedBallXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void StayToonedBallXtra::open(ObjectType type, const Common::Path &path) {
    StayToonedBallXtraObject::initMethods(xlibMethods);
    StayToonedBallXtraObject *xobj = new StayToonedBallXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StayToonedBallXtra::close(ObjectType type) {
    StayToonedBallXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StayToonedBallXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StayToonedBallXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StayToonedBallXtra::m_mInitRandom, 0)
XOBJSTUB(StayToonedBallXtra::m_mInitGame, 0)
XOBJSTUB(StayToonedBallXtra::m_mOPERATEGAME, 0)
XOBJSTUB(StayToonedBallXtra::m_mGactions, 0)
XOBJSTUB(StayToonedBallXtra::m_mGetInfo, 0)
XOBJSTUB(StayToonedBallXtra::m_mSetSize, 0)
XOBJSTUB(StayToonedBallXtra::m_mAddStuff, 0)

}
