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
#include "director/lingo/xlibs/g/getsoundinlevel.h"

/**************************************************
 *
 * USED IN:
 * Microphone Fiend
 *
 **************************************************/

/*
-- GetSoundInLevel,  v1.4 C G.M.Smith 1994/5/6
I	mNew
I	mGetLevel        --> I         -- Get the sound input current level
I	mGetLeftLevel    --> I         -- Get the left sound input current level
I	mGetRightLevel   --> I         -- Get the right sound input current level
I	mGetChannelCount --> I         -- Get the number of sound channels
X    mDispose
--
--  mNew return error values
--  -227 Sound input busy
--  -108 Out of memory, increase partition in info dialog
--  -10  No stereo input
--  -20  Cannot access data - odd hardware?
 */

namespace Director {

const char *GetSoundInLevelXObj::xlibName = "GetSoundInLevel";
const XlibFileDesc GetSoundInLevelXObj::fileNames[] = {
	{ "GetSoundInLevel",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				GetSoundInLevelXObj::m_new,		 0, 0,	400 },
	{ "getLevel",				GetSoundInLevelXObj::m_getLevel,		 0, 0,	400 },
	{ "getLeftLevel",				GetSoundInLevelXObj::m_getLeftLevel,		 0, 0,	400 },
	{ "getRightLevel",				GetSoundInLevelXObj::m_getRightLevel,		 0, 0,	400 },
	{ "getChannelCount",				GetSoundInLevelXObj::m_getChannelCount,		 0, 0,	400 },
	{ "dispose",				GetSoundInLevelXObj::m_dispose,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

GetSoundInLevelXObject::GetSoundInLevelXObject(ObjectType ObjectType) :Object<GetSoundInLevelXObject>("GetSoundInLevel") {
	_objType = ObjectType;
}

void GetSoundInLevelXObj::open(ObjectType type, const Common::Path &path) {
    GetSoundInLevelXObject::initMethods(xlibMethods);
    GetSoundInLevelXObject *xobj = new GetSoundInLevelXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void GetSoundInLevelXObj::close(ObjectType type) {
    GetSoundInLevelXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void GetSoundInLevelXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("GetSoundInLevelXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(GetSoundInLevelXObj::m_getLevel, 100)
XOBJSTUB(GetSoundInLevelXObj::m_getLeftLevel, 100)
XOBJSTUB(GetSoundInLevelXObj::m_getRightLevel, 100)
XOBJSTUB(GetSoundInLevelXObj::m_getChannelCount, 1)
XOBJSTUBNR(GetSoundInLevelXObj::m_dispose)

}
