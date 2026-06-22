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
#include "director/lingo/xtras/b/bink_video_xtra.h"

/**************************************************
 *
 * USED IN:
 * Bill & Ben: Flowerpot Fun
 * Disney's Search for the Secret Keys
 * Walking With Beasts: Operation Salvage
 *
 **************************************************/

/*
-- xtra Bink
new object me
-- Bink Xtra Handlers --
*binkVersion -- prints version information
binkSetInterfaceKeys object me, string param1 -- sets the interface keys
binkSetPosition object me, integer xoffset, integer yoffset -- sets the screen offset for the bink
* binkUseDirectSound -- use Direct Sound
* binkUseMiles -- use Miles Sound System
* binkQuickPlay string filename, integer soundTracks, integer flags -- loads and plays a Bink
binkOpen object me, string filename, integer soundTracks, integer flags -- opens a Bink file
binkClose object me -- closes a previously opened Bink object
binkPlay object me -- plays a previously opened Bink object
binkGetLastKey object me -- gets the last key pressed to stop a Bink
binkGetFrameNum object me -- gets the current frame number of the Bink
binkGetFrames object me -- gets the total number of frames in the Bink
binkPlayFrames object me, integer startFrame, integer endFrame -- plays the specified frames of a Bink
binkGoto object me, integer frame -- skips to the specified frame number
binkPlayNext object me, integer frames -- plays the next frames
binkSetUpdateStage object me, integer updateStage -- sets the Xtra to call updateStage while playing a Bink
* binkMinorVersion -- gets the minor version number
* binkMajorVersion -- gets the major version number
* binkAbout -- displays info about the Bink xtra in message window

 */

namespace Director {

const char *BinkXtra::xlibName = "Bink";
const XlibFileDesc BinkXtra::fileNames[] = {
	{ "bink",   nullptr },
	{ "xbink",   nullptr }, // a version of this xtra used by few games I've come across (example, Disney Princess Royal Horse Show) It can't be stubbed & likely has undocumented features 
	{ "divbink",   nullptr }, // a version of this xtra that some Disney games, like Search for the Secret Keys, use, It can't be stubbed & likely has undocumented features
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BinkXtra::m_new,		 0, 0,	500 },
	{ "binkSetInterfaceKeys",				BinkXtra::m_binkSetInterfaceKeys,		 1, 0,	500 },
	{ "binkSetPosition",				BinkXtra::m_binkSetPosition,		 2, 0,	500 },
	{ "binkOpen",				BinkXtra::m_binkOpen,		 3, 0,	500 },
	{ "binkClose",				BinkXtra::m_binkClose,		 0, 0,	500 },
	{ "binkPlay",				BinkXtra::m_binkPlay,		 0, 0,	500 },
	{ "binkGetLastKey",				BinkXtra::m_binkGetLastKey,		 0, 0,	500 },
	{ "binkGetFrameNum",				BinkXtra::m_binkGetFrameNum,		 0, 0,	500 },
	{ "binkGetFrames",				BinkXtra::m_binkGetFrames,		 0, 0,	500 },
	{ "binkPlayFrames",				BinkXtra::m_binkPlayFrames,		 2, 0,	500 },
	{ "binkGoto",				BinkXtra::m_binkGoto,		 1, 0,	500 },
	{ "binkPlayNext",				BinkXtra::m_binkPlayNext,		 1, 0,	500 },
	{ "binkSetUpdateStage",				BinkXtra::m_binkSetUpdateStage,		 1, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "binkVersion", BinkXtra::m_binkVersion, 0, 0, 500, HBLTIN },
	{ "binkUseDirectSound", BinkXtra::m_binkUseDirectSound, 0, 0, 500, HBLTIN },
	{ "binkUseMiles", BinkXtra::m_binkUseMiles, 0, 0, 500, HBLTIN },
	{ "binkQuickPlay", BinkXtra::m_binkQuickPlay, 3, 3, 500, HBLTIN },
	{ "binkMinorVersion", BinkXtra::m_binkMinorVersion, 0, 0, 500, HBLTIN },
	{ "binkMajorVersion", BinkXtra::m_binkMajorVersion, 0, 0, 500, HBLTIN },
	{ "binkAbout", BinkXtra::m_binkAbout, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BinkXtraObject::BinkXtraObject(ObjectType ObjectType) :Object<BinkXtraObject>("Bink") {
	_objType = ObjectType;
}

bool BinkXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BinkXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BinkXtra::xlibName);
	warning("BinkXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BinkXtra::open(ObjectType type, const Common::Path &path) {
    BinkXtraObject::initMethods(xlibMethods);
    BinkXtraObject *xobj = new BinkXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BinkXtra::close(ObjectType type) {
    BinkXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BinkXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BinkXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BinkXtra::m_binkVersion, 0)
XOBJSTUB(BinkXtra::m_binkSetInterfaceKeys, 0)
XOBJSTUB(BinkXtra::m_binkSetPosition, 0)
XOBJSTUB(BinkXtra::m_binkUseDirectSound, 0)
XOBJSTUB(BinkXtra::m_binkUseMiles, 0)
XOBJSTUB(BinkXtra::m_binkQuickPlay, 0)
XOBJSTUB(BinkXtra::m_binkOpen, 0)
XOBJSTUB(BinkXtra::m_binkClose, 0)
XOBJSTUB(BinkXtra::m_binkPlay, 0)
XOBJSTUB(BinkXtra::m_binkGetLastKey, 0)
XOBJSTUB(BinkXtra::m_binkGetFrameNum, 0)
XOBJSTUB(BinkXtra::m_binkGetFrames, 0)
XOBJSTUB(BinkXtra::m_binkPlayFrames, 0)
XOBJSTUB(BinkXtra::m_binkGoto, 0)
XOBJSTUB(BinkXtra::m_binkPlayNext, 0)
XOBJSTUB(BinkXtra::m_binkSetUpdateStage, 0)
XOBJSTUB(BinkXtra::m_binkMinorVersion, 0)
XOBJSTUB(BinkXtra::m_binkMajorVersion, 0)
XOBJSTUB(BinkXtra::m_binkAbout, 0)

}
