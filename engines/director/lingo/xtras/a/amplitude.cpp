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
#include "director/lingo/xtras/a/amplitude.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 7 (D8), Löwenzahn 8 (D8.5), Löwenzahn Adventskalender (D8.5),
 * Löwenzahn Spielebox (D9)
 *
 **************************************************/

/*
-- xtra Amplitude
new object me
ampSetSound object me, integer soundChannel, any srcCMem -- define a sound to be scanned for amplitude data, the sound channel will be polled during payback
ampGetCurve object me -- returns a list of amplitude values for the current sound defined by SetSound()
ampSetCurve object me, any ampList, integer soundChannel, integer loopStart, integer loopEnd, integer stereo -- sends a list (ampList) of values to the xtra
ampSetLoopPoints object me, integer loopStart, integer loopEnd -- set loop points (in whole milliseconds) for Windows playback
ampGetError object me -- returns the current error status for the Amplitude xtra.
* ampRegister string userName, string regString -- enter user name and registration string.
 */

namespace Director {

const char *AmplitudeXtra::xlibName = "Amplitude";
const XlibFileDesc AmplitudeXtra::fileNames[] = {
	{ "amplitude",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AmplitudeXtra::m_new,		 0, 0,	500 },
	{ "ampSetSound",				AmplitudeXtra::m_ampSetSound,		 2, 2,	500 },
	{ "ampGetCurve",				AmplitudeXtra::m_ampGetCurve,		 0, 0,	500 },
	{ "ampSetCurve",				AmplitudeXtra::m_ampSetCurve,		 5, 5,	500 },
	{ "ampSetLoopPoints",				AmplitudeXtra::m_ampSetLoopPoints,		 2, 2,	500 },
	{ "ampGetError",				AmplitudeXtra::m_ampGetError,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "ampRegister", AmplitudeXtra::m_ampRegister, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AmplitudeXtraObject::AmplitudeXtraObject(ObjectType ObjectType) :Object<AmplitudeXtraObject>("Amplitude") {
	_objType = ObjectType;
}

bool AmplitudeXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AmplitudeXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AmplitudeXtra::xlibName);
	warning("AmplitudeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AmplitudeXtra::open(ObjectType type, const Common::Path &path) {
    AmplitudeXtraObject::initMethods(xlibMethods);
    AmplitudeXtraObject *xobj = new AmplitudeXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AmplitudeXtra::close(ObjectType type) {
    AmplitudeXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AmplitudeXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AmplitudeXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AmplitudeXtra::m_ampSetSound, 0)
XOBJSTUB(AmplitudeXtra::m_ampGetCurve, 0)
XOBJSTUB(AmplitudeXtra::m_ampSetCurve, 0)
XOBJSTUB(AmplitudeXtra::m_ampSetLoopPoints, 0)
XOBJSTUB(AmplitudeXtra::m_ampGetError, 0)
XOBJSTUB(AmplitudeXtra::m_ampRegister, 0)

}
