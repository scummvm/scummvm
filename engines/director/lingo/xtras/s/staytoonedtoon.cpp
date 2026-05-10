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
#include "director/lingo/xtras/s/staytoonedtoon.h"

/**************************************************
 *
 * USED IN:
 * Stay Tooned
 *
 **************************************************/

/*
-- xtra ToonBoy -- ToonBoy 7/15/96 Programmer: Keith Laverty FunnyBone Interactive
new object me
* mInitRandom integer SomeThing -- feeds a number bewteen 1-100 to the Random# Array
* mInitGame -- feeds a number bewteen 1-100 to the Random# Array
* mInitScoop -- feeds a number bewteen 1-100 to the Random# Array
* mOPERATEGAME integer NUM1 -- feeds a number bewteen 1-100 to the Random# Array
* mReadData integer NUM1, integer NUM2, integer NUM3-- feeds a number bewteen 1-100 to the Random# Array
* mGactions -- feeds a number bewteen 1-100 to the Random# Array
* mPassArray integer NUM1, integer NUM2, integer NUM3-- feeds a number bewteen 1-100 to the Random# Array
* mPassCastSize integer NUM1, integer NUM2, integer NUM3, integer NUM4, integer NUM5-- feeds a number bewteen 1-100 to the Random# Array

 */

namespace Director {

const char *StayToonedToonXtra::xlibName = "ToonBoy";
const XlibFileDesc StayToonedToonXtra::fileNames[] = {
	{ "toon",   "staytooned" },
	{ "toon_mac",   "staytooned" },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StayToonedToonXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "mInitRandom", StayToonedToonXtra::m_mInitRandom, 1, 1, 500, HBLTIN },
	{ "mInitGame", StayToonedToonXtra::m_mInitGame, 0, 0, 500, HBLTIN },
	{ "mInitScoop", StayToonedToonXtra::m_mInitScoop, 0, 0, 500, HBLTIN },
	{ "mOPERATEGAME", StayToonedToonXtra::m_mOPERATEGAME, 1, 1, 500, HBLTIN },
	{ "mReadData", StayToonedToonXtra::m_mReadData, 3, 3, 500, HBLTIN },
	{ "mGactions", StayToonedToonXtra::m_mGactions, 0, 0, 500, HBLTIN },
	{ "mPassArray", StayToonedToonXtra::m_mPassArray, 3, 3, 500, HBLTIN },
	{ "mPassCastSize", StayToonedToonXtra::m_mPassCastSize, 5, 5, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StayToonedToonXtraObject::StayToonedToonXtraObject(ObjectType ObjectType) :Object<StayToonedToonXtraObject>("StayToonedToon") {
	_objType = ObjectType;
}

bool StayToonedToonXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum StayToonedToonXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(StayToonedToonXtra::xlibName);
	warning("StayToonedToonXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void StayToonedToonXtra::open(ObjectType type, const Common::Path &path) {
    StayToonedToonXtraObject::initMethods(xlibMethods);
    StayToonedToonXtraObject *xobj = new StayToonedToonXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StayToonedToonXtra::close(ObjectType type) {
    StayToonedToonXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StayToonedToonXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StayToonedToonXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StayToonedToonXtra::m_mInitRandom, 0)
XOBJSTUB(StayToonedToonXtra::m_mInitGame, 0)
XOBJSTUB(StayToonedToonXtra::m_mInitScoop, 0)
XOBJSTUB(StayToonedToonXtra::m_mOPERATEGAME, 0)
XOBJSTUB(StayToonedToonXtra::m_mReadData, 0)
XOBJSTUB(StayToonedToonXtra::m_mGactions, 0)
XOBJSTUB(StayToonedToonXtra::m_mPassArray, 0)
XOBJSTUB(StayToonedToonXtra::m_mPassCastSize, 0)

}
