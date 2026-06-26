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
#include "director/lingo/xtras/d/directmedia.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 4 (D7), Löwenzahn 5 (D8), Löwenzahn 8 (D8.5),
 * Löwenzahn Adventskalender (D8.5), Löwenzahn 3/4/6 + Spielebox (D9)
 *
 **************************************************/

/*
-- xtra DirectMedia Xtra
new object me
videoplay (sprite me) -- Plays the video
videopause (sprite me) -- Pauses the video
videoseek (sprite me, whichtime) -- Seeks to the time specified in milliseconds
videoplaysegment (sprite me, starttime, endtime) -- Plays the specified segment
setvolume (sprite me, whichvolume) -- Sets the volume
getvolume (sprite me) -- Returns the current volume
setbalance (sprite me, whichbalance) -- Sets the balance
getbalance (sprite me) -- Returns the current balance
isPastCuePoint (sprite me, cuePointIndex) -- Returns TRUE if the cue point specified has already passed
mostRecentCuePoint (sprite me) -- Returns the index of the last cuepoint passed

 */

namespace Director {

const char *DirectMediaXtra::xlibName = "DirectMedia";
const XlibFileDesc DirectMediaXtra::fileNames[] = {
	{ "Directme",      nullptr },	// on-disk Xtra filename (DIRECTME.X32)
	{ "directmedia",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DirectMediaXtra::m_new,		 0, 0,	500 },
	{ "videoplay",				DirectMediaXtra::m_videoplay,		 0, 0,	500 },
	{ "videopause",				DirectMediaXtra::m_videopause,		 0, 0,	500 },
	{ "videoseek",				DirectMediaXtra::m_videoseek,		 1, 1,	500 },
	{ "videoplaysegment",				DirectMediaXtra::m_videoplaysegment,		 2, 2,	500 },
	{ "setvolume",				DirectMediaXtra::m_setvolume,		 1, 1,	500 },
	{ "getvolume",				DirectMediaXtra::m_getvolume,		 0, 0,	500 },
	{ "setbalance",				DirectMediaXtra::m_setbalance,		 1, 1,	500 },
	{ "getbalance",				DirectMediaXtra::m_getbalance,		 0, 0,	500 },
	{ "isPastCuePoint",				DirectMediaXtra::m_isPastCuePoint,		 1, 1,	500 },
	{ "mostRecentCuePoint",				DirectMediaXtra::m_mostRecentCuePoint,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "isDirectShowInstalled", DirectMediaXtra::m_isDirectShowInstalled, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DirectMediaXtraObject::DirectMediaXtraObject(ObjectType ObjectType) :Object<DirectMediaXtraObject>("DirectMedia") {
	_objType = ObjectType;
}

bool DirectMediaXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DirectMediaXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DirectMediaXtra::xlibName);
	warning("DirectMediaXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DirectMediaXtra::open(ObjectType type, const Common::Path &path) {
    DirectMediaXtraObject::initMethods(xlibMethods);
    DirectMediaXtraObject *xobj = new DirectMediaXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DirectMediaXtra::close(ObjectType type) {
    DirectMediaXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DirectMediaXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DirectMediaXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DirectMediaXtra::m_videoplay, 0)
XOBJSTUB(DirectMediaXtra::m_videopause, 0)
XOBJSTUB(DirectMediaXtra::m_videoseek, 0)
XOBJSTUB(DirectMediaXtra::m_videoplaysegment, 0)
XOBJSTUB(DirectMediaXtra::m_setvolume, 0)
XOBJSTUB(DirectMediaXtra::m_getvolume, 0)
XOBJSTUB(DirectMediaXtra::m_setbalance, 0)
XOBJSTUB(DirectMediaXtra::m_getbalance, 0)
XOBJSTUB(DirectMediaXtra::m_isPastCuePoint, 0)
XOBJSTUB(DirectMediaXtra::m_mostRecentCuePoint, 0)

void DirectMediaXtra::m_isDirectShowInstalled(int nargs) {
	// Whether DirectShow is available to play the member's video. Löwenzahn 4 gates
	// its MPEG intro on this (MPEG\INTROW, `if isDirectShowInstalled(member(2)) = 0
	// then go("noMP")`), so answer as a Windows install with DirectShow would.
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(1));
}

}
