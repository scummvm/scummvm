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
#include "director/lingo/xtras/a/apresentationtrakker.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra APresentationTrakker -- this Xtra provides web tracking features
--Copyright 1999 - 2006, Cyber Sea, Inc. All rights reserve  (http://www.apresentationtrakker.com)
new object me
-- Template handlers --
PT_SetINIFile object me, string FileName, * -- Set tracking INI file
PT_SetTrackingURL object me, string Server, string URLName, * -- Set tracking Domain and URL
PT_GetVersion object me, * -- Get tracking version
PT_SetOSVersion object me, string OSVersion, * -- Set operating system
PT_SetRunLocation object me, integer Location, * -- Set run location
PT_SetDistributionGroup object me, integer Group, * -- Set distribution group
PT_SetAppVersion object me, string AppVersion, * -- Set application version
PT_SetExtraValue object me, string EValueId, string EValue, * -- Set other tracking codes
PT_SetETrackLocation object me, integer Location, * -- Set what location to track
PT_PostTracking object me, * -- Post tracking to script
PT_Register object me, string Serial, * -- Register tracking xtra
PT_TrackWebLink object me, integer WebLink, * -- Track web link
PT_GetTrackURL object me, * -- Get final tracking URL
PT_Reset object me, * -- Reset tracking variables to initial settings
PT_GetStatus object me, * -- Get status of tracking calls
PT_SetTraceFile object me, string FileName, * -- Set debug tracing file
PT_TraceMsg object me, string MsgStr, * -- Write custom message to trace file
PT_SetUserId object me, string UserId, * -- Set unique user id
PT_ClearTracking object me, * -- Reset tracking counters and INI file backup tracking
PT_SetViewCounter object me, integer Counter, * -- Set the number of hits to track (bypass defaults)
"
 */

namespace Director {

const char *ApresentationtrakkerXtra::xlibName = "Apresentationtrakker";
const XlibFileDesc ApresentationtrakkerXtra::fileNames[] = {
	{ "apresentationtrakker",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ApresentationtrakkerXtra::m_new,		 0, 0,	600 },
	{ "PT_SetINIFile",				ApresentationtrakkerXtra::m_PT_SetINIFile,		 -1, 0,	600 },
	{ "PT_SetTrackingURL",				ApresentationtrakkerXtra::m_PT_SetTrackingURL,		 -1, 0,	600 },
	{ "PT_GetVersion",				ApresentationtrakkerXtra::m_PT_GetVersion,		 -1, 0,	600 },
	{ "PT_SetOSVersion",				ApresentationtrakkerXtra::m_PT_SetOSVersion,		 -1, 0,	600 },
	{ "PT_SetRunLocation",				ApresentationtrakkerXtra::m_PT_SetRunLocation,		 -1, 0,	600 },
	{ "PT_SetDistributionGroup",				ApresentationtrakkerXtra::m_PT_SetDistributionGroup,		 -1, 0,	600 },
	{ "PT_SetAppVersion",				ApresentationtrakkerXtra::m_PT_SetAppVersion,		 -1, 0,	600 },
	{ "PT_SetExtraValue",				ApresentationtrakkerXtra::m_PT_SetExtraValue,		 -1, 0,	600 },
	{ "PT_SetETrackLocation",				ApresentationtrakkerXtra::m_PT_SetETrackLocation,		 -1, 0,	600 },
	{ "PT_PostTracking",				ApresentationtrakkerXtra::m_PT_PostTracking,		 -1, 0,	600 },
	{ "PT_Register",				ApresentationtrakkerXtra::m_PT_Register,		 -1, 0,	600 },
	{ "PT_TrackWebLink",				ApresentationtrakkerXtra::m_PT_TrackWebLink,		 -1, 0,	600 },
	{ "PT_GetTrackURL",				ApresentationtrakkerXtra::m_PT_GetTrackURL,		 -1, 0,	600 },
	{ "PT_Reset",				ApresentationtrakkerXtra::m_PT_Reset,		 -1, 0,	600 },
	{ "PT_GetStatus",				ApresentationtrakkerXtra::m_PT_GetStatus,		 -1, 0,	600 },
	{ "PT_SetTraceFile",				ApresentationtrakkerXtra::m_PT_SetTraceFile,		 -1, 0,	600 },
	{ "PT_TraceMsg",				ApresentationtrakkerXtra::m_PT_TraceMsg,		 -1, 0,	600 },
	{ "PT_SetUserId",				ApresentationtrakkerXtra::m_PT_SetUserId,		 -1, 0,	600 },
	{ "PT_ClearTracking",				ApresentationtrakkerXtra::m_PT_ClearTracking,		 -1, 0,	600 },
	{ "PT_SetViewCounter",				ApresentationtrakkerXtra::m_PT_SetViewCounter,		 -1, 0,	600 },
	{ """,				ApresentationtrakkerXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ApresentationtrakkerXtraObject::ApresentationtrakkerXtraObject(ObjectType ObjectType) :Object<ApresentationtrakkerXtraObject>("Apresentationtrakker") {
	_objType = ObjectType;
}

bool ApresentationtrakkerXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ApresentationtrakkerXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ApresentationtrakkerXtra::xlibName);
	warning("ApresentationtrakkerXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ApresentationtrakkerXtra::open(ObjectType type, const Common::Path &path) {
    ApresentationtrakkerXtraObject::initMethods(xlibMethods);
    ApresentationtrakkerXtraObject *xobj = new ApresentationtrakkerXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ApresentationtrakkerXtra::close(ObjectType type) {
    ApresentationtrakkerXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ApresentationtrakkerXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ApresentationtrakkerXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetINIFile, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetTrackingURL, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_GetVersion, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetOSVersion, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetRunLocation, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetDistributionGroup, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetAppVersion, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetExtraValue, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetETrackLocation, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_PostTracking, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_Register, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_TrackWebLink, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_GetTrackURL, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_Reset, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_GetStatus, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetTraceFile, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_TraceMsg, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetUserId, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_ClearTracking, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_PT_SetViewCounter, 0)
XOBJSTUB(ApresentationtrakkerXtra::m_", 0)

}
