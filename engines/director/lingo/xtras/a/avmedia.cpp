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
#include "director/lingo/xtras/a/avmedia.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra AVMedia
-- Mac-Only
-- v0.30 (c) 2017 Valentin Schmidt
-- https://valentin.dasdeck.com/xtras/avmedia_xtra/

-- CONSTRUCTOR
new object me


-- AUDIO
audioOpen object me, any pathOrURLOrData, *allowRateChange, fileTypeHint
audioClose object me
audioPlay object me
audioPause object me
audioGetDuration object me
audioGetCurrentTime object me
audioSetCurrentTime object me, float timeSec
audioGetPan object me
audioSetPan object me, float pan
audioGetRate object me
audioSetRate object me, float rate
audioGetVolume object me
audioSetVolume object me, float volume
audioFileConvert object me, any inputFile, string outputFile, string outputFileType, string outputFormat, float outputSampleRate, *floatStartTimeSec, floatStopTimeSec

-- VIDEO
videoOpen object me, string pathOrURL, object rect, *stayOnTop, windowMask, overlayFlag, windowTitle
videoClose object me

-- get parameters
videoGetCurrentTime object me, *timeScale
videoGetDuration object me
videoGetFPS object me
videoGetImage object me, *timeMS -- returns image as bytearray with JPG data
videoGetNaturalSize object me
videoGetRate object me
videoGetReadyForDisplay object me
videoGetRect object me
videoGetTimeScale object me
videoGetTrackList object me
videoGetVolume object me

-- set parameter
videoSetCurrentTime object me, integer timeMS, *timeScale, zeroToleranceBefore, zeroToleranceAfter
videoSetLoop object me, integer loopFlag
videoSetOverlay object me, any overlayImageOrFile
videoSetOverlayVisibility object me, integer visibilityFlag
videoSetParentWindow  object me, integer movieNum
videoSetRate object me, float rate, *time, atHostTime, timeScale
videoSetRect object me, object rect
videoSetTrackEnabled object me, integer trackNum, integer enabledFlag
videoSetVolume object me, float volume
videoSetWindowOrder object me, integer movieNum, integer orderingMode
videoStepByCount object me, integer stepCount
videoSyncToAudio object me
videoToggleFullScreen object me

-- set callbacks
videoSetDoubleClickCallback object me, *cbHandler, cbTarget
videoSetMouseDownCallback object me, *cbHandler, cbTarget
videoSetReadyStatusChangeCallback object me, *cbHandler, cbTarget
videoSetWindowClosedCallback object me, *cbHandler, cbTarget

-- conversion
videoFileConvert object me, any inputFile, string outputFile, string preset, *intStartTimeMS, intStopTimeMS
videoSetFileConvertCallback object me, *cbHandler, cbTarget
 */

namespace Director {

const char *AvmediaXtra::xlibName = "Avmedia";
const XlibFileDesc AvmediaXtra::fileNames[] = {
	{ "avmedia",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AvmediaXtra::m_new,		 0, 0,	600 },
	{ "audioOpen",				AvmediaXtra::m_audioOpen,		 3, 3,	600 },
	{ "audioClose",				AvmediaXtra::m_audioClose,		 0, 0,	600 },
	{ "audioPlay",				AvmediaXtra::m_audioPlay,		 0, 0,	600 },
	{ "audioPause",				AvmediaXtra::m_audioPause,		 0, 0,	600 },
	{ "audioGetDuration",				AvmediaXtra::m_audioGetDuration,		 0, 0,	600 },
	{ "audioGetCurrentTime",				AvmediaXtra::m_audioGetCurrentTime,		 0, 0,	600 },
	{ "audioSetCurrentTime",				AvmediaXtra::m_audioSetCurrentTime,		 1, 1,	600 },
	{ "audioGetPan",				AvmediaXtra::m_audioGetPan,		 0, 0,	600 },
	{ "audioSetPan",				AvmediaXtra::m_audioSetPan,		 1, 1,	600 },
	{ "audioGetRate",				AvmediaXtra::m_audioGetRate,		 0, 0,	600 },
	{ "audioSetRate",				AvmediaXtra::m_audioSetRate,		 1, 1,	600 },
	{ "audioGetVolume",				AvmediaXtra::m_audioGetVolume,		 0, 0,	600 },
	{ "audioSetVolume",				AvmediaXtra::m_audioSetVolume,		 1, 1,	600 },
	{ "audioFileConvert",				AvmediaXtra::m_audioFileConvert,		 7, 7,	600 },
	{ "videoOpen",				AvmediaXtra::m_videoOpen,		 6, 6,	600 },
	{ "videoClose",				AvmediaXtra::m_videoClose,		 0, 0,	600 },
	{ "videoGetCurrentTime",				AvmediaXtra::m_videoGetCurrentTime,		 1, 1,	600 },
	{ "videoGetDuration",				AvmediaXtra::m_videoGetDuration,		 0, 0,	600 },
	{ "videoGetFPS",				AvmediaXtra::m_videoGetFPS,		 0, 0,	600 },
	{ "videoGetImage",				AvmediaXtra::m_videoGetImage,		 1, 1,	600 },
	{ "videoGetNaturalSize",				AvmediaXtra::m_videoGetNaturalSize,		 0, 0,	600 },
	{ "videoGetRate",				AvmediaXtra::m_videoGetRate,		 0, 0,	600 },
	{ "videoGetReadyForDisplay",				AvmediaXtra::m_videoGetReadyForDisplay,		 0, 0,	600 },
	{ "videoGetRect",				AvmediaXtra::m_videoGetRect,		 0, 0,	600 },
	{ "videoGetTimeScale",				AvmediaXtra::m_videoGetTimeScale,		 0, 0,	600 },
	{ "videoGetTrackList",				AvmediaXtra::m_videoGetTrackList,		 0, 0,	600 },
	{ "videoGetVolume",				AvmediaXtra::m_videoGetVolume,		 0, 0,	600 },
	{ "videoSetCurrentTime",				AvmediaXtra::m_videoSetCurrentTime,		 4, 4,	600 },
	{ "videoSetLoop",				AvmediaXtra::m_videoSetLoop,		 1, 1,	600 },
	{ "videoSetOverlay",				AvmediaXtra::m_videoSetOverlay,		 1, 1,	600 },
	{ "videoSetOverlayVisibility",				AvmediaXtra::m_videoSetOverlayVisibility,		 1, 1,	600 },
	{ "videoSetParentWindow",				AvmediaXtra::m_videoSetParentWindow,		 1, 1,	600 },
	{ "videoSetRate",				AvmediaXtra::m_videoSetRate,		 4, 4,	600 },
	{ "videoSetRect",				AvmediaXtra::m_videoSetRect,		 1, 1,	600 },
	{ "videoSetTrackEnabled",				AvmediaXtra::m_videoSetTrackEnabled,		 2, 2,	600 },
	{ "videoSetVolume",				AvmediaXtra::m_videoSetVolume,		 1, 1,	600 },
	{ "videoSetWindowOrder",				AvmediaXtra::m_videoSetWindowOrder,		 2, 2,	600 },
	{ "videoStepByCount",				AvmediaXtra::m_videoStepByCount,		 1, 1,	600 },
	{ "videoSyncToAudio",				AvmediaXtra::m_videoSyncToAudio,		 0, 0,	600 },
	{ "videoToggleFullScreen",				AvmediaXtra::m_videoToggleFullScreen,		 0, 0,	600 },
	{ "videoSetDoubleClickCallback",				AvmediaXtra::m_videoSetDoubleClickCallback,		 2, 2,	600 },
	{ "videoSetMouseDownCallback",				AvmediaXtra::m_videoSetMouseDownCallback,		 2, 2,	600 },
	{ "videoSetReadyStatusChangeCallback",				AvmediaXtra::m_videoSetReadyStatusChangeCallback,		 2, 2,	600 },
	{ "videoSetWindowClosedCallback",				AvmediaXtra::m_videoSetWindowClosedCallback,		 2, 2,	600 },
	{ "videoFileConvert",				AvmediaXtra::m_videoFileConvert,		 5, 5,	600 },
	{ "videoSetFileConvertCallback",				AvmediaXtra::m_videoSetFileConvertCallback,		 2, 2,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AvmediaXtraObject::AvmediaXtraObject(ObjectType ObjectType) :Object<AvmediaXtraObject>("Avmedia") {
	_objType = ObjectType;
}

bool AvmediaXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AvmediaXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AvmediaXtra::xlibName);
	warning("AvmediaXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AvmediaXtra::open(ObjectType type, const Common::Path &path) {
    AvmediaXtraObject::initMethods(xlibMethods);
    AvmediaXtraObject *xobj = new AvmediaXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AvmediaXtra::close(ObjectType type) {
    AvmediaXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AvmediaXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AvmediaXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AvmediaXtra::m_audioOpen, 0)
XOBJSTUB(AvmediaXtra::m_audioClose, 0)
XOBJSTUB(AvmediaXtra::m_audioPlay, 0)
XOBJSTUB(AvmediaXtra::m_audioPause, 0)
XOBJSTUB(AvmediaXtra::m_audioGetDuration, 0)
XOBJSTUB(AvmediaXtra::m_audioGetCurrentTime, 0)
XOBJSTUB(AvmediaXtra::m_audioSetCurrentTime, 0)
XOBJSTUB(AvmediaXtra::m_audioGetPan, 0)
XOBJSTUB(AvmediaXtra::m_audioSetPan, 0)
XOBJSTUB(AvmediaXtra::m_audioGetRate, 0)
XOBJSTUB(AvmediaXtra::m_audioSetRate, 0)
XOBJSTUB(AvmediaXtra::m_audioGetVolume, 0)
XOBJSTUB(AvmediaXtra::m_audioSetVolume, 0)
XOBJSTUB(AvmediaXtra::m_audioFileConvert, 0)
XOBJSTUB(AvmediaXtra::m_videoOpen, 0)
XOBJSTUB(AvmediaXtra::m_videoClose, 0)
XOBJSTUB(AvmediaXtra::m_videoGetCurrentTime, 0)
XOBJSTUB(AvmediaXtra::m_videoGetDuration, 0)
XOBJSTUB(AvmediaXtra::m_videoGetFPS, 0)
XOBJSTUB(AvmediaXtra::m_videoGetImage, 0)
XOBJSTUB(AvmediaXtra::m_videoGetNaturalSize, 0)
XOBJSTUB(AvmediaXtra::m_videoGetRate, 0)
XOBJSTUB(AvmediaXtra::m_videoGetReadyForDisplay, 0)
XOBJSTUB(AvmediaXtra::m_videoGetRect, 0)
XOBJSTUB(AvmediaXtra::m_videoGetTimeScale, 0)
XOBJSTUB(AvmediaXtra::m_videoGetTrackList, 0)
XOBJSTUB(AvmediaXtra::m_videoGetVolume, 0)
XOBJSTUB(AvmediaXtra::m_videoSetCurrentTime, 0)
XOBJSTUB(AvmediaXtra::m_videoSetLoop, 0)
XOBJSTUB(AvmediaXtra::m_videoSetOverlay, 0)
XOBJSTUB(AvmediaXtra::m_videoSetOverlayVisibility, 0)
XOBJSTUB(AvmediaXtra::m_videoSetParentWindow, 0)
XOBJSTUB(AvmediaXtra::m_videoSetRate, 0)
XOBJSTUB(AvmediaXtra::m_videoSetRect, 0)
XOBJSTUB(AvmediaXtra::m_videoSetTrackEnabled, 0)
XOBJSTUB(AvmediaXtra::m_videoSetVolume, 0)
XOBJSTUB(AvmediaXtra::m_videoSetWindowOrder, 0)
XOBJSTUB(AvmediaXtra::m_videoStepByCount, 0)
XOBJSTUB(AvmediaXtra::m_videoSyncToAudio, 0)
XOBJSTUB(AvmediaXtra::m_videoToggleFullScreen, 0)
XOBJSTUB(AvmediaXtra::m_videoSetDoubleClickCallback, 0)
XOBJSTUB(AvmediaXtra::m_videoSetMouseDownCallback, 0)
XOBJSTUB(AvmediaXtra::m_videoSetReadyStatusChangeCallback, 0)
XOBJSTUB(AvmediaXtra::m_videoSetWindowClosedCallback, 0)
XOBJSTUB(AvmediaXtra::m_videoFileConvert, 0)
XOBJSTUB(AvmediaXtra::m_videoSetFileConvertCallback, 0)

}
