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
#include "director/lingo/xtras/a/aspitchdetect.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra asPitchDetect -- An Xtra for Macromedia Director that does a pitch detection on live sound input (i.e. it computes the note played or sung)
-- Version 2.0
-- http://www.schmittmachine.com
new object me, *
-- All functions return 0 if error--
free object me -- frees
-- devices and sources --
getDeviceList object me -- get list of devices
getSourceList object me, * -- get list of sound sources
setParams object me, * -- sets device, source, minPitch
getDevice object me -- current source
getSource object me -- current source
getError object me -- returns the last error string
getVolume object me -- returns the current sound volume
-- pitch tracking --
startPitchDetection object me -- starts pitch tracking 
stopPitchDetection object me -- stops pitch tracking 
getPitch object me -- gets pitch in Hz (0.0 means no pitch) 
setDynamicTracking object me, int onOff -- sets dynamic tracking on or off 
isDynamicTracking object me -- returns if dynamic tracking is on or off 
"
 */

namespace Director {

const char *AspitchdetectXtra::xlibName = "Aspitchdetect";
const XlibFileDesc AspitchdetectXtra::fileNames[] = {
	{ "aspitchdetect",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AspitchdetectXtra::m_new,		 -1, 0,	700 },
	{ "free",				AspitchdetectXtra::m_free,		 0, 0,	700 },
	{ "getDeviceList",				AspitchdetectXtra::m_getDeviceList,		 0, 0,	700 },
	{ "getSourceList",				AspitchdetectXtra::m_getSourceList,		 -1, 0,	700 },
	{ "setParams",				AspitchdetectXtra::m_setParams,		 -1, 0,	700 },
	{ "getDevice",				AspitchdetectXtra::m_getDevice,		 0, 0,	700 },
	{ "getSource",				AspitchdetectXtra::m_getSource,		 0, 0,	700 },
	{ "getError",				AspitchdetectXtra::m_getError,		 0, 0,	700 },
	{ "getVolume",				AspitchdetectXtra::m_getVolume,		 0, 0,	700 },
	{ "startPitchDetection",				AspitchdetectXtra::m_startPitchDetection,		 0, 0,	700 },
	{ "stopPitchDetection",				AspitchdetectXtra::m_stopPitchDetection,		 0, 0,	700 },
	{ "getPitch",				AspitchdetectXtra::m_getPitch,		 0, 0,	700 },
	{ "setDynamicTracking",				AspitchdetectXtra::m_setDynamicTracking,		 1, 1,	700 },
	{ "isDynamicTracking",				AspitchdetectXtra::m_isDynamicTracking,		 0, 0,	700 },
	{ """,				AspitchdetectXtra::m_",		 -1, -1,	700 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AspitchdetectXtraObject::AspitchdetectXtraObject(ObjectType ObjectType) :Object<AspitchdetectXtraObject>("Aspitchdetect") {
	_objType = ObjectType;
}

bool AspitchdetectXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AspitchdetectXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AspitchdetectXtra::xlibName);
	warning("AspitchdetectXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AspitchdetectXtra::open(ObjectType type, const Common::Path &path) {
    AspitchdetectXtraObject::initMethods(xlibMethods);
    AspitchdetectXtraObject *xobj = new AspitchdetectXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AspitchdetectXtra::close(ObjectType type) {
    AspitchdetectXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AspitchdetectXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AspitchdetectXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AspitchdetectXtra::m_free, 0)
XOBJSTUB(AspitchdetectXtra::m_getDeviceList, 0)
XOBJSTUB(AspitchdetectXtra::m_getSourceList, 0)
XOBJSTUB(AspitchdetectXtra::m_setParams, 0)
XOBJSTUB(AspitchdetectXtra::m_getDevice, 0)
XOBJSTUB(AspitchdetectXtra::m_getSource, 0)
XOBJSTUB(AspitchdetectXtra::m_getError, 0)
XOBJSTUB(AspitchdetectXtra::m_getVolume, 0)
XOBJSTUB(AspitchdetectXtra::m_startPitchDetection, 0)
XOBJSTUB(AspitchdetectXtra::m_stopPitchDetection, 0)
XOBJSTUB(AspitchdetectXtra::m_getPitch, 0)
XOBJSTUB(AspitchdetectXtra::m_setDynamicTracking, 0)
XOBJSTUB(AspitchdetectXtra::m_isDynamicTracking, 0)
XOBJSTUB(AspitchdetectXtra::m_", 0)

}
