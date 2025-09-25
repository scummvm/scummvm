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
#include "director/lingo/xtras/miles.h"

/**************************************************
 *
 * USED IN:
 * Fisher-Price - Big Action: Garage
 * Sabrina The Animated Series: Magical Adventure
 *
 **************************************************/

/*
-- xtra Miles

--------------------------------------------------
--  The Miles Sound System Xtra - Version 3.6F  --
--  Copyright (C) 1991-97 RAD Game Tools, Inc.  --
--------------------------------------------------

new object

*MilesQuickPlay string, integer
*MilesQuickStream string, integer

*MilesStartup
*MilesReleaseHandles
*MilesReacquireHandles
*MilesShutdown
*MilesHookToSmacker
*MilesSetWaveOutput integer, integer, integer, integer
*MilesGetWaveDescription

MilesOpenWave object, string
MilesOpenWaveCastmember object, string, string
MilesOpenXMIDI object, string
MilesOpenStream object, string
MilesOpenCD object, integer
MilesOpenCopy object, string
MilesCopyFrom object

MilesPlay object
MilesPause object
MilesResume object
MilesClose object

MilesGetStatus object
MilesGetVolume object
MilesGetPosition object
MilesGetSpeed object
MilesGetLoopCount object
MilesGetPan object

MilesSetVolume object, integer
MilesSetPosition object, integer
MilesSetSpeed object, integer
MilesSetLoopCount object, integer
MilesSetPan object, integer

MilesGetCDTracks object
MilesPlayCDTrack object, integer
MilesPlayCDMS object, integer, integer
MilesGetCDStartTrackMS object, integer
MilesGetCDEndTrackMS object, integer
MilesEjectCD object
MilesRetractCD object

 */

namespace Director {

const char *MilesXtra::xlibName = "Miles";
const XlibFileDesc MilesXtra::fileNames[] = {
	{ "miles",   nullptr },
	{ "MSSX32",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				MilesXtra::m_new,		 0, 0,	500 },
	{ "MilesOpenWave",				MilesXtra::m_MilesOpenWave,		 1, 0,	500 },
	{ "MilesOpenWaveCastmember",				MilesXtra::m_MilesOpenWaveCastmember,		 2, 0,	500 },
	{ "MilesOpenXMIDI",				MilesXtra::m_MilesOpenXMIDI,		 1, 0,	500 },
	{ "MilesOpenStream",				MilesXtra::m_MilesOpenStream,		 1, 0,	500 },
	{ "MilesOpenCD",				MilesXtra::m_MilesOpenCD,		 1, 0,	500 },
	{ "MilesOpenCopy",				MilesXtra::m_MilesOpenCopy,		 1, 0,	500 },
	{ "MilesCopyFrom",				MilesXtra::m_MilesCopyFrom,		 0, 0,	500 },
	{ "MilesPlay",				MilesXtra::m_MilesPlay,		 0, 0,	500 },
	{ "MilesPause",				MilesXtra::m_MilesPause,		 0, 0,	500 },
	{ "MilesResume",				MilesXtra::m_MilesResume,		 0, 0,	500 },
	{ "MilesClose",				MilesXtra::m_MilesClose,		 0, 0,	500 },
	{ "MilesGetStatus",				MilesXtra::m_MilesGetStatus,		 0, 0,	500 },
	{ "MilesGetVolume",				MilesXtra::m_MilesGetVolume,		 0, 0,	500 },
	{ "MilesGetPosition",				MilesXtra::m_MilesGetPosition,		 0, 0,	500 },
	{ "MilesGetSpeed",				MilesXtra::m_MilesGetSpeed,		 0, 0,	500 },
	{ "MilesGetLoopCount",				MilesXtra::m_MilesGetLoopCount,		 0, 0,	500 },
	{ "MilesGetPan",				MilesXtra::m_MilesGetPan,		 0, 0,	500 },
	{ "MilesSetVolume",				MilesXtra::m_MilesSetVolume,		 1, 0,	500 },
	{ "MilesSetPosition",				MilesXtra::m_MilesSetPosition,		 1, 0,	500 },
	{ "MilesSetSpeed",				MilesXtra::m_MilesSetSpeed,		 1, 0,	500 },
	{ "MilesSetLoopCount",				MilesXtra::m_MilesSetLoopCount,		 1, 0,	500 },
	{ "MilesSetPan",				MilesXtra::m_MilesSetPan,		 1, 0,	500 },
	{ "MilesGetCDTracks",				MilesXtra::m_MilesGetCDTracks,		 0, 0,	500 },
	{ "MilesPlayCDTrack",				MilesXtra::m_MilesPlayCDTrack,		 1, 0,	500 },
	{ "MilesPlayCDMS",				MilesXtra::m_MilesPlayCDMS,		 2, 0,	500 },
	{ "MilesGetCDStartTrackMS",				MilesXtra::m_MilesGetCDStartTrackMS,		 1, 0,	500 },
	{ "MilesGetCDEndTrackMS",				MilesXtra::m_MilesGetCDEndTrackMS,		 1, 0,	500 },
	{ "MilesEjectCD",				MilesXtra::m_MilesEjectCD,		 0, 0,	500 },
	{ "MilesRetractCD",				MilesXtra::m_MilesRetractCD,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "MilesQuickPlay", MilesXtra::m_MilesQuickPlay, 2, 2, 500, HBLTIN },
	{ "MilesQuickStream", MilesXtra::m_MilesQuickStream, 2, 2, 500, HBLTIN },
	{ "MilesStartup", MilesXtra::m_MilesStartup, 0, 0, 500, HBLTIN },
	{ "MilesReleaseHandles", MilesXtra::m_MilesReleaseHandles, 0, 0, 500, HBLTIN },
	{ "MilesReacquireHandles", MilesXtra::m_MilesReacquireHandles, 0, 0, 500, HBLTIN },
	{ "MilesShutdown", MilesXtra::m_MilesShutdown, 0, 0, 500, HBLTIN },
	{ "MilesHookToSmacker", MilesXtra::m_MilesHookToSmacker, 0, 0, 500, HBLTIN },
	{ "MilesSetWaveOutput", MilesXtra::m_MilesSetWaveOutput, 4, 4, 500, HBLTIN },
	{ "MilesGetWaveDescription", MilesXtra::m_MilesGetWaveDescription, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MilesXtraObject::MilesXtraObject(ObjectType ObjectType) :Object<MilesXtraObject>("Miles") {
	_objType = ObjectType;
}

bool MilesXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum MilesXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(MilesXtra::xlibName);
	warning("MilesXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void MilesXtra::open(ObjectType type, const Common::Path &path) {
    MilesXtraObject::initMethods(xlibMethods);
    MilesXtraObject *xobj = new MilesXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MilesXtra::close(ObjectType type) {
    MilesXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MilesXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MilesXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MilesXtra::m_MilesQuickPlay, 0)
XOBJSTUB(MilesXtra::m_MilesQuickStream, 0)
XOBJSTUB(MilesXtra::m_MilesStartup, 0)
XOBJSTUB(MilesXtra::m_MilesReleaseHandles, 0)
XOBJSTUB(MilesXtra::m_MilesReacquireHandles, 0)
XOBJSTUB(MilesXtra::m_MilesShutdown, 0)
XOBJSTUB(MilesXtra::m_MilesHookToSmacker, 0)
XOBJSTUB(MilesXtra::m_MilesSetWaveOutput, 0)
XOBJSTUB(MilesXtra::m_MilesGetWaveDescription, 0)
XOBJSTUB(MilesXtra::m_MilesOpenWave, 0)
XOBJSTUB(MilesXtra::m_MilesOpenWaveCastmember, 0)
XOBJSTUB(MilesXtra::m_MilesOpenXMIDI, 0)
XOBJSTUB(MilesXtra::m_MilesOpenStream, 0)
XOBJSTUB(MilesXtra::m_MilesOpenCD, 0)
XOBJSTUB(MilesXtra::m_MilesOpenCopy, 0)
XOBJSTUB(MilesXtra::m_MilesCopyFrom, 0)
XOBJSTUB(MilesXtra::m_MilesPlay, 0)
XOBJSTUB(MilesXtra::m_MilesPause, 0)
XOBJSTUB(MilesXtra::m_MilesResume, 0)
XOBJSTUB(MilesXtra::m_MilesClose, 0)
XOBJSTUB(MilesXtra::m_MilesGetStatus, 0)
XOBJSTUB(MilesXtra::m_MilesGetVolume, 0)
XOBJSTUB(MilesXtra::m_MilesGetPosition, 0)
XOBJSTUB(MilesXtra::m_MilesGetSpeed, 0)
XOBJSTUB(MilesXtra::m_MilesGetLoopCount, 0)
XOBJSTUB(MilesXtra::m_MilesGetPan, 0)
XOBJSTUB(MilesXtra::m_MilesSetVolume, 0)
XOBJSTUB(MilesXtra::m_MilesSetPosition, 0)
XOBJSTUB(MilesXtra::m_MilesSetSpeed, 0)
XOBJSTUB(MilesXtra::m_MilesSetLoopCount, 0)
XOBJSTUB(MilesXtra::m_MilesSetPan, 0)
XOBJSTUB(MilesXtra::m_MilesGetCDTracks, 0)
XOBJSTUB(MilesXtra::m_MilesPlayCDTrack, 0)
XOBJSTUB(MilesXtra::m_MilesPlayCDMS, 0)
XOBJSTUB(MilesXtra::m_MilesGetCDStartTrackMS, 0)
XOBJSTUB(MilesXtra::m_MilesGetCDEndTrackMS, 0)
XOBJSTUB(MilesXtra::m_MilesEjectCD, 0)
XOBJSTUB(MilesXtra::m_MilesRetractCD, 0)

}
