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
#include "director/lingo/xtras/xsound.h"

/**************************************************
 *
 * USED IN:
 * I Spy
 *
 **************************************************/

/*
-- xtra xsound
new object me, int bufferSize
forget object me
-- Sound Xtra(tm) version 3.0.1
-- Engineered by Scott Kildall, redeye@igc.org
-- Entire program code (c) 1995-97 by Red Eye Software
-- Sound Xtra is a registered trademark of g/matter, inc
-- All rights reserved.
--
-- For more information on Sound Xtra, see our website:
-- http://www.gmatter.com
--
-- For technical support and sales, please email:
-- <support@gmatter.com>
-- <sales@gmatter.com>
--
-- Online help is available under the Xtras menu.
--
-- For information about other Red Eye Software products, visit
-- the Red Eye Software website at http://www.halcyon.com/redeye
--
+ Register object xtraRef, string serialNumber -- Registers the Sound Xtra. THIS MUST BE CALLED FOR YOUR RUNTIME (PROJECTOR) FILES
GetError object me   -- returns the last error number
GetInfo object me, int identifier --  returns specific information
SetInfo object me, int identifier, int setting --  sets specific information
Status object me   -- returns the status of a sound
ConnectInputDevice object me -- connects to the sound recorder
DisconnectInputDevice object me -- disconnects from the sound recorder
SetSoundType object me, string type, string name, string action -- specifies a file for sound recording or playback
ClearSoundType object me   -- clears information about the sound file
Play object me   -- plays a sound
Record object me   -- records a sound
Stop object me   -- stops a sound
Pause object me   -- pauses a sound
Resume object me  -- resumes a paused sound
GetCurrentTime object me   -- returns the playback time of a sound
IsASound object me, string type, string name  -- indicates if this sound exists
DeleteSound object me, string type, string name  -- deletes a sound
SetPlaySegment object me, int start, int end  -- sets the start and end points of a playback sound
ClearPlaySegment object me   -- resets the start and end points
SetSampleRate object me, int rate    -- sets the sample rate of a sound
SetSampleDepth object me, int depth   -- sets the sample depth of a sound
SetCompression object me, int compressor   --  sets a compressor for the sound
GetInputLevel object me   -- returns the input level of the microphone
FreeRecordingTime object me --  returns the free recording time
 */

namespace Director {

const char *XsoundXtra::xlibName = "Xsound";
const XlibFileDesc XsoundXtra::fileNames[] = {
	{ "xsound",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				XsoundXtra::m_new,		 1, 0,	500 },
	{ "forget",				XsoundXtra::m_forget,		 0, 0,	500 },
	{ "Register",				XsoundXtra::m_Register,		 1, 1,	500 },
	{ "GetError",				XsoundXtra::m_GetError,		 0, 0,	500 },
	{ "GetInfo",				XsoundXtra::m_GetInfo,		 1, 0,	500 },
	{ "SetInfo",				XsoundXtra::m_SetInfo,		 2, 0,	500 },
	{ "Status",				XsoundXtra::m_Status,		 0, 0,	500 },
	{ "ConnectInputDevice",				XsoundXtra::m_ConnectInputDevice,		 0, 0,	500 },
	{ "DisconnectInputDevice",				XsoundXtra::m_DisconnectInputDevice,		 0, 0,	500 },
	{ "SetSoundType",				XsoundXtra::m_SetSoundType,		 3, 0,	500 },
	{ "ClearSoundType",				XsoundXtra::m_ClearSoundType,		 0, 0,	500 },
	{ "Play",				XsoundXtra::m_Play,		 0, 0,	500 },
	{ "Record",				XsoundXtra::m_Record,		 0, 0,	500 },
	{ "Stop",				XsoundXtra::m_Stop,		 0, 0,	500 },
	{ "Pause",				XsoundXtra::m_Pause,		 0, 0,	500 },
	{ "Resume",				XsoundXtra::m_Resume,		 0, 0,	500 },
	{ "GetCurrentTime",				XsoundXtra::m_GetCurrentTime,		 0, 0,	500 },
	{ "IsASound",				XsoundXtra::m_IsASound,		 2, 0,	500 },
	{ "DeleteSound",				XsoundXtra::m_DeleteSound,		 2, 0,	500 },
	{ "SetPlaySegment",				XsoundXtra::m_SetPlaySegment,		 2, 0,	500 },
	{ "ClearPlaySegment",				XsoundXtra::m_ClearPlaySegment,		 0, 0,	500 },
	{ "SetSampleRate",				XsoundXtra::m_SetSampleRate,		 1, 0,	500 },
	{ "SetSampleDepth",				XsoundXtra::m_SetSampleDepth,		 1, 0,	500 },
	{ "SetCompression",				XsoundXtra::m_SetCompression,		 1, 0,	500 },
	{ "GetInputLevel",				XsoundXtra::m_GetInputLevel,		 0, 0,	500 },
	{ "FreeRecordingTime",				XsoundXtra::m_FreeRecordingTime,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

XsoundXtraObject::XsoundXtraObject(ObjectType ObjectType) :Object<XsoundXtraObject>("Xsound") {
	_objType = ObjectType;
}

bool XsoundXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum XsoundXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(XsoundXtra::xlibName);
	warning("XsoundXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void XsoundXtra::open(ObjectType type, const Common::Path &path) {
    XsoundXtraObject::initMethods(xlibMethods);
    XsoundXtraObject *xobj = new XsoundXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void XsoundXtra::close(ObjectType type) {
    XsoundXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void XsoundXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("XsoundXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(XsoundXtra::m_forget, 0)

void XsoundXtra::m_Register(int nargs) {
	Common::String serialNumber = g_lingo->pop().asString();
	debugC(1, kDebugXObj, "XsoundXtra::m_register: Registered with serial \"%s\"", serialNumber.c_str());
	g_lingo->push(Datum(0));
}

XOBJSTUB(XsoundXtra::m_GetError, 0)
XOBJSTUB(XsoundXtra::m_GetInfo, 0)
XOBJSTUB(XsoundXtra::m_SetInfo, 0)
XOBJSTUB(XsoundXtra::m_Status, 0)
XOBJSTUB(XsoundXtra::m_ConnectInputDevice, 0)
XOBJSTUB(XsoundXtra::m_DisconnectInputDevice, 0)
XOBJSTUB(XsoundXtra::m_SetSoundType, 0)
XOBJSTUB(XsoundXtra::m_ClearSoundType, 0)
XOBJSTUB(XsoundXtra::m_Play, 0)
XOBJSTUB(XsoundXtra::m_Record, 0)
XOBJSTUB(XsoundXtra::m_Stop, 0)
XOBJSTUB(XsoundXtra::m_Pause, 0)
XOBJSTUB(XsoundXtra::m_Resume, 0)
XOBJSTUB(XsoundXtra::m_GetCurrentTime, 0)
XOBJSTUB(XsoundXtra::m_IsASound, 0)
XOBJSTUB(XsoundXtra::m_DeleteSound, 0)
XOBJSTUB(XsoundXtra::m_SetPlaySegment, 0)
XOBJSTUB(XsoundXtra::m_ClearPlaySegment, 0)
XOBJSTUB(XsoundXtra::m_SetSampleRate, 0)
XOBJSTUB(XsoundXtra::m_SetSampleDepth, 0)
XOBJSTUB(XsoundXtra::m_SetCompression, 0)
XOBJSTUB(XsoundXtra::m_GetInputLevel, 0)
XOBJSTUB(XsoundXtra::m_FreeRecordingTime, 0)

}
