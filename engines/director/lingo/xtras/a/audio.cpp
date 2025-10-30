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
#include "director/lingo/xtras/a/audio.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra audio
--
-- Audio Xtra
-- Published by updateStage
--
-- Sales: sales@updatestage.com, (781) 641-6043
-- Technical support: support@updatestage.com, (781) 641-6043
-- Web: http://www.updatestage.com
-- Written by Scott Kildall, Red Eye Software
--Copyright 1996-1998,  Red Eye Software

new object me,  integer bufferSize
forget object me

+ Register  object xtraRef, string serialNumber -- Registers the Sound Xtra. THIS MUST BE CALLED FOR YOUR RUNTIME (PROJECTOR) FILES
GetError object me   -- returns the last error number
GetInfo object me, integer identifier --  returns specific information
SetInfo object me, integer identifier, integer setting --  sets specific information
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
SetSampleRate object me, integer rate    -- sets the sample rate of a sound
SetSampleDepth object me, integer depth   -- sets the sample depth of a sound
SetCompression object me, integer compressor   --  sets a compressor for the sound
GetInputLevel object me   -- returns the input level of the microphone
FreeRecordingTime object me --  returns the free recording time

-- Wave-plotting functions
SetForegroundColor object me, integer red, integer green, integer blue -- sets the foreground color for wave-plotting
SetBackgroundColor object me, integer red, integer green, integer blue -- sets the background color for wave-plotting
PlotWaveform  object me, member bitmapCastMember, integer width, integer height, integer dotsPerX, integer timebase, integer normalize -- generates a waveform
--

 */

namespace Director {

const char *AudioXtra::xlibName = "Audio";
const XlibFileDesc AudioXtra::fileNames[] = {
	{ "audio",   nullptr },
	{ "Resaudio",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AudioXtra::m_new,		 1, 1,	500 },
	{ "forget",				AudioXtra::m_forget,		 0, 0,	500 },
	{ "GetError",				AudioXtra::m_GetError,		 0, 0,	500 },
	{ "GetInfo",				AudioXtra::m_GetInfo,		 1, 1,	500 },
	{ "SetInfo",				AudioXtra::m_SetInfo,		 2, 2,	500 },
	{ "Status",				AudioXtra::m_Status,		 0, 0,	500 },
	{ "ConnectInputDevice",				AudioXtra::m_ConnectInputDevice,		 0, 0,	500 },
	{ "DisconnectInputDevice",				AudioXtra::m_DisconnectInputDevice,		 0, 0,	500 },
	{ "SetSoundType",				AudioXtra::m_SetSoundType,		 3, 3,	500 },
	{ "ClearSoundType",				AudioXtra::m_ClearSoundType,		 0, 0,	500 },
	{ "Play",				AudioXtra::m_Play,		 0, 0,	500 },
	{ "Record",				AudioXtra::m_Record,		 0, 0,	500 },
	{ "Stop",				AudioXtra::m_Stop,		 0, 0,	500 },
	{ "Pause",				AudioXtra::m_Pause,		 0, 0,	500 },
	{ "Resume",				AudioXtra::m_Resume,		 0, 0,	500 },
	{ "GetCurrentTime",				AudioXtra::m_GetCurrentTime,		 0, 0,	500 },
	{ "IsASound",				AudioXtra::m_IsASound,		 2, 2,	500 },
	{ "DeleteSound",				AudioXtra::m_DeleteSound,		 2, 2,	500 },
	{ "SetPlaySegment",				AudioXtra::m_SetPlaySegment,		 2, 2,	500 },
	{ "ClearPlaySegment",				AudioXtra::m_ClearPlaySegment,		 0, 0,	500 },
	{ "SetSampleRate",				AudioXtra::m_SetSampleRate,		 1, 1,	500 },
	{ "SetSampleDepth",				AudioXtra::m_SetSampleDepth,		 1, 1,	500 },
	{ "SetCompression",				AudioXtra::m_SetCompression,		 1, 1,	500 },
	{ "GetInputLevel",				AudioXtra::m_GetInputLevel,		 0, 0,	500 },
	{ "FreeRecordingTime",				AudioXtra::m_FreeRecordingTime,		 0, 0,	500 },
	{ "SetForegroundColor",				AudioXtra::m_SetForegroundColor,		 3, 3,	500 },
	{ "SetBackgroundColor",				AudioXtra::m_SetBackgroundColor,		 3, 3,	500 },
	{ "PlotWaveform",				AudioXtra::m_PlotWaveform,		 6, 6,	500 },
	{ "Register",				AudioXtra::m_Register,		 1, 1,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AudioXtraObject::AudioXtraObject(ObjectType ObjectType) :Object<AudioXtraObject>("Audio") {
	_objType = ObjectType;
}

bool AudioXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AudioXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AudioXtra::xlibName);
	warning("AudioXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AudioXtra::open(ObjectType type, const Common::Path &path) {
    AudioXtraObject::initMethods(xlibMethods);
    AudioXtraObject *xobj = new AudioXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AudioXtra::close(ObjectType type) {
    AudioXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AudioXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AudioXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AudioXtra::m_forget, 0)
XOBJSTUB(AudioXtra::m_Register, 0)
XOBJSTUB(AudioXtra::m_GetError, 0)
XOBJSTUB(AudioXtra::m_GetInfo, 0)
XOBJSTUB(AudioXtra::m_SetInfo, 0)
XOBJSTUB(AudioXtra::m_Status, 0)
XOBJSTUB(AudioXtra::m_ConnectInputDevice, 0)
XOBJSTUB(AudioXtra::m_DisconnectInputDevice, 0)
XOBJSTUB(AudioXtra::m_SetSoundType, 0)
XOBJSTUB(AudioXtra::m_ClearSoundType, 0)
XOBJSTUB(AudioXtra::m_Play, 0)
XOBJSTUB(AudioXtra::m_Record, 0)
XOBJSTUB(AudioXtra::m_Stop, 0)
XOBJSTUB(AudioXtra::m_Pause, 0)
XOBJSTUB(AudioXtra::m_Resume, 0)
XOBJSTUB(AudioXtra::m_GetCurrentTime, 0)
XOBJSTUB(AudioXtra::m_IsASound, 0)
XOBJSTUB(AudioXtra::m_DeleteSound, 0)
XOBJSTUB(AudioXtra::m_SetPlaySegment, 0)
XOBJSTUB(AudioXtra::m_ClearPlaySegment, 0)
XOBJSTUB(AudioXtra::m_SetSampleRate, 0)
XOBJSTUB(AudioXtra::m_SetSampleDepth, 0)
XOBJSTUB(AudioXtra::m_SetCompression, 0)
XOBJSTUB(AudioXtra::m_GetInputLevel, 0)
XOBJSTUB(AudioXtra::m_FreeRecordingTime, 0)
XOBJSTUB(AudioXtra::m_SetForegroundColor, 0)
XOBJSTUB(AudioXtra::m_SetBackgroundColor, 0)
XOBJSTUB(AudioXtra::m_PlotWaveform, 0)

}
