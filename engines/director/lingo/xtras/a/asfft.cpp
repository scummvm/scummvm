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
#include "director/lingo/xtras/a/asfft.h"

/**************************************************
 *
 * USED IN:
 * La Máquina de Hacer Tareas, Lab. de Ciencias
 * La Máquina de Hacer Tareas, Electricidad
 *
 **************************************************/

/*
-- xtra asFFT -- An Xtra for Macromedia Director that does a FFT (Fast Fourier Transform) on live sound input.
-- Version 2.6
-- http://www.schmittmachine.com
new object me, *
-- All functions return 0 if error--
getDeviceList object me -- get list of devices
getSourceList object me, * -- get list of sound sources
setParams object me, * -- sets nbbins, timesmooth, device, source ; optional
getBinCount object me -- number of bins
getEqualizedFreqCount object me -- number of equalized frequencies
getTimeSmooth object me -- current accuracy
getDevice object me -- current source
getSource object me -- current source
getError object me -- returns the last error string
getSamples object me, integer nbSamples, float amplMax -- returns the last 'nbSamples' of sound
getFFT object me, * -- returns the FFT values
getVolume object me -- returns the current sound volume
-- Obsolete but still working for backwards compatibility --
asFFTInit object me, integer nbBins, integer accuracy
asFFTClose object me -- closes
asFFTGetError object me
asFFTGetSoundValues object me, integer nbValues
asFFTGetFFTValues object me
"
 */

namespace Director {

const char *AsfftXtra::xlibName = "Asfft";
const XlibFileDesc AsfftXtra::fileNames[] = {
	{ "asfft",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AsfftXtra::m_new,		 -1, 0,	700 },
	{ "getDeviceList",				AsfftXtra::m_getDeviceList,		 0, 0,	700 },
	{ "getSourceList",				AsfftXtra::m_getSourceList,		 -1, 0,	700 },
	{ "setParams",				AsfftXtra::m_setParams,		 -1, 0,	700 },
	{ "getBinCount",				AsfftXtra::m_getBinCount,		 0, 0,	700 },
	{ "getEqualizedFreqCount",				AsfftXtra::m_getEqualizedFreqCount,		 0, 0,	700 },
	{ "getTimeSmooth",				AsfftXtra::m_getTimeSmooth,		 0, 0,	700 },
	{ "getDevice",				AsfftXtra::m_getDevice,		 0, 0,	700 },
	{ "getSource",				AsfftXtra::m_getSource,		 0, 0,	700 },
	{ "getError",				AsfftXtra::m_getError,		 0, 0,	700 },
	{ "getSamples",				AsfftXtra::m_getSamples,		 2, 2,	700 },
	{ "getFFT",				AsfftXtra::m_getFFT,		 -1, 0,	700 },
	{ "getVolume",				AsfftXtra::m_getVolume,		 0, 0,	700 },
	{ "asFFTInit",				AsfftXtra::m_asFFTInit,		 2, 2,	700 },
	{ "asFFTClose",				AsfftXtra::m_asFFTClose,		 0, 0,	700 },
	{ "asFFTGetError",				AsfftXtra::m_asFFTGetError,		 0, 0,	700 },
	{ "asFFTGetSoundValues",				AsfftXtra::m_asFFTGetSoundValues,		 1, 1,	700 },
	{ "asFFTGetFFTValues",				AsfftXtra::m_asFFTGetFFTValues,		 0, 0,	700 },
	{ """,				AsfftXtra::m_",		 -1, -1,	700 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AsfftXtraObject::AsfftXtraObject(ObjectType ObjectType) :Object<AsfftXtraObject>("Asfft") {
	_objType = ObjectType;
}

bool AsfftXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AsfftXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AsfftXtra::xlibName);
	warning("AsfftXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AsfftXtra::open(ObjectType type, const Common::Path &path) {
    AsfftXtraObject::initMethods(xlibMethods);
    AsfftXtraObject *xobj = new AsfftXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AsfftXtra::close(ObjectType type) {
    AsfftXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AsfftXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AsfftXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AsfftXtra::m_getDeviceList, 0)
XOBJSTUB(AsfftXtra::m_getSourceList, 0)
XOBJSTUB(AsfftXtra::m_setParams, 0)
XOBJSTUB(AsfftXtra::m_getBinCount, 0)
XOBJSTUB(AsfftXtra::m_getEqualizedFreqCount, 0)
XOBJSTUB(AsfftXtra::m_getTimeSmooth, 0)
XOBJSTUB(AsfftXtra::m_getDevice, 0)
XOBJSTUB(AsfftXtra::m_getSource, 0)
XOBJSTUB(AsfftXtra::m_getError, 0)
XOBJSTUB(AsfftXtra::m_getSamples, 0)
XOBJSTUB(AsfftXtra::m_getFFT, 0)
XOBJSTUB(AsfftXtra::m_getVolume, 0)
XOBJSTUB(AsfftXtra::m_asFFTInit, 0)
XOBJSTUB(AsfftXtra::m_asFFTClose, 0)
XOBJSTUB(AsfftXtra::m_asFFTGetError, 0)
XOBJSTUB(AsfftXtra::m_asFFTGetSoundValues, 0)
XOBJSTUB(AsfftXtra::m_asFFTGetFFTValues, 0)
XOBJSTUB(AsfftXtra::m_", 0)

}
