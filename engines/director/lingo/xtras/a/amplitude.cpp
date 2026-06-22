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
 * Engie Benjy: Time for Teamwork
 * Karlsson på Taket och Kuckelimuckmedicinen
 * La Máquina de Hacer Tareas
 * Rubbadubbers
 *
 **************************************************/

/*
-- xtra AmplitudePro
-- http://www.marmalademedia.com.au
new object me
--Amplitude Pro Only --
* ampProRegister string userName, string regString -- enter user name and registration string.
ampSetSpectSound object me, integer soundChannel, any srcCMem -- define a sound to be scanned for spectrum data, the sound channel will be polled during playback.
ampGetSpectCurve object me -- returns a list of lists of spectrum values for the current sound defined by ampSetSpectSound().
ampGetSpectMonoCurve object me -- returns a list of lists of mono spectrum values for the current sound defined by ampSetSpectSound(). Always returns a mono list.
ampSetSpectCurve object me, any spectList, integer soundChannel, integer loopStart, integer loopEnd -- sends a list (spectList) of spectrum values to the xtra.
ampSetSpectBands object me, any bandList -- sets the frequency range for each spectrum band. The list must consist of 2-element sublists in the form [[b1Start,b1End],[b2Start,b2End],..].
--Amplitude --
* ampRegister string userName, string regString -- enter user name and registration string.
ampSetSound object me, integer soundChannel, any srcCMem -- define a sound to be scanned for amplitude data, the sound channel will be polled during playback.
ampGetCurve object me -- returns a list of amplitude values for the current sound defined by SetSound(). Returns a stereo or mon list depending on the sound source.
ampGetMonoCurve object meo -- returns a list of mono amplitude values for the current sound defined by SetSound(). Always returns a mono list.
ampSetCurve object me, any ampList, integer soundChannel, integer loopStart, integer loopEnd, integer stereo -- sends a list (ampList) of values to the xtra.
ampSetLoopPoints object me, integer loopStart, integer loopEnd -- set loop points (in whole milliseconds) for Windows playback.
ampGetError object me -- returns the current error status for the Amplitude xtra."

 */

namespace Director {

const char *AmplitudeproXtra::xlibName = "Amplitudepro";
const XlibFileDesc AmplitudeproXtra::fileNames[] = {
	{ "amplitudepro",   nullptr },
	{ "amplitude",   nullptr }
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AmplitudeproXtra::m_new,		 0, 0,	700 },
	{ "ampSetSpectSound",				AmplitudeproXtra::m_ampSetSpectSound,		 2, 2,	700 },
	{ "ampGetSpectCurve",				AmplitudeproXtra::m_ampGetSpectCurve,		 0, 0,	700 },
	{ "ampGetSpectMonoCurve",				AmplitudeproXtra::m_ampGetSpectMonoCurve,		 0, 0,	700 },
	{ "ampSetSpectCurve",				AmplitudeproXtra::m_ampSetSpectCurve,		 4, 4,	700 },
	{ "ampSetSpectBands",				AmplitudeproXtra::m_ampSetSpectBands,		 1, 1,	700 },
	{ "ampSetSound",				AmplitudeproXtra::m_ampSetSound,		 2, 2,	700 },
	{ "ampGetCurve",				AmplitudeproXtra::m_ampGetCurve,		 0, 0,	700 },
	{ "ampGetMonoCurve",				AmplitudeproXtra::m_ampGetMonoCurve,		 0, 0,	700 },
	{ "ampSetCurve",				AmplitudeproXtra::m_ampSetCurve,		 5, 5,	700 },
	{ "ampSetLoopPoints",				AmplitudeproXtra::m_ampSetLoopPoints,		 2, 2,	700 },
	{ "ampGetError",				AmplitudeproXtra::m_ampGetError,		 0, 0,	700 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "ampProRegister", AmplitudeproXtra::m_ampProRegister, 2, 2, 700, HBLTIN },
	{ "ampRegister", AmplitudeproXtra::m_ampRegister, 2, 2, 700, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AmplitudeproXtraObject::AmplitudeproXtraObject(ObjectType ObjectType) :Object<AmplitudeproXtraObject>("Amplitudepro") {
	_objType = ObjectType;
}

bool AmplitudeproXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AmplitudeproXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AmplitudeproXtra::xlibName);
	warning("AmplitudeproXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AmplitudeproXtra::open(ObjectType type, const Common::Path &path) {
    AmplitudeproXtraObject::initMethods(xlibMethods);
    AmplitudeproXtraObject *xobj = new AmplitudeproXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AmplitudeproXtra::close(ObjectType type) {
    AmplitudeproXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AmplitudeproXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AmplitudeproXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AmplitudeproXtra::m_ampProRegister, 0)
XOBJSTUB(AmplitudeproXtra::m_ampSetSpectSound, 0)
XOBJSTUB(AmplitudeproXtra::m_ampGetSpectCurve, 0)
XOBJSTUB(AmplitudeproXtra::m_ampGetSpectMonoCurve, 0)
XOBJSTUB(AmplitudeproXtra::m_ampSetSpectCurve, 0)
XOBJSTUB(AmplitudeproXtra::m_ampSetSpectBands, 0)
XOBJSTUB(AmplitudeproXtra::m_ampRegister, 0)
XOBJSTUB(AmplitudeproXtra::m_ampSetSound, 0)
XOBJSTUB(AmplitudeproXtra::m_ampGetCurve, 0)
XOBJSTUB(AmplitudeproXtra::m_ampGetMonoCurve, 0)
XOBJSTUB(AmplitudeproXtra::m_ampSetCurve, 0)
XOBJSTUB(AmplitudeproXtra::m_ampSetLoopPoints, 0)
XOBJSTUB(AmplitudeproXtra::m_ampGetError, 0)

}
