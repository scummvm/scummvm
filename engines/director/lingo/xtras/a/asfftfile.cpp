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
#include "director/lingo/xtras/a/asfftfile.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra asFFTfile -- An Xtra for Macromedia Director that computes a FFT (Fast Fourier Transform) on a QuickTime sound file.
-- Version 1.1
-- http://www.schmittmachine.com
new object me, *
-- All functions return 0 if error--
openQuicktimeFile object me, string qtfilepath -- returns integer fileID
closeFile object me, integer fileID --
getSamples object me, integer fileID, integer startsample, integer nbSamples, * -- 
getAmplitude object me, integer fileID, integer startsample, integer nbSamples-- 
getFFT object me, integer fileID, integer startsample, * --
getSampleRate object me, integer fileID -- returns a float
getEqualizedFreqCount object me, integer binCount -- number of equalized frequencies
getError object me -- returns the last error string
"
 */

namespace Director {

const char *AsfftfileXtra::xlibName = "Asfftfile";
const XlibFileDesc AsfftfileXtra::fileNames[] = {
	{ "asfftfile",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AsfftfileXtra::m_new,		 -1, 0,	900 },
	{ "openQuicktimeFile",				AsfftfileXtra::m_openQuicktimeFile,		 1, 1,	900 },
	{ "closeFile",				AsfftfileXtra::m_closeFile,		 1, 1,	900 },
	{ "getSamples",				AsfftfileXtra::m_getSamples,		 -1, 0,	900 },
	{ "getAmplitude",				AsfftfileXtra::m_getAmplitude,		 3, 3,	900 },
	{ "getFFT",				AsfftfileXtra::m_getFFT,		 -1, 0,	900 },
	{ "getSampleRate",				AsfftfileXtra::m_getSampleRate,		 1, 1,	900 },
	{ "getEqualizedFreqCount",				AsfftfileXtra::m_getEqualizedFreqCount,		 1, 1,	900 },
	{ "getError",				AsfftfileXtra::m_getError,		 0, 0,	900 },
	{ """,				AsfftfileXtra::m_",		 -1, -1,	900 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AsfftfileXtraObject::AsfftfileXtraObject(ObjectType ObjectType) :Object<AsfftfileXtraObject>("Asfftfile") {
	_objType = ObjectType;
}

bool AsfftfileXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AsfftfileXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AsfftfileXtra::xlibName);
	warning("AsfftfileXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AsfftfileXtra::open(ObjectType type, const Common::Path &path) {
    AsfftfileXtraObject::initMethods(xlibMethods);
    AsfftfileXtraObject *xobj = new AsfftfileXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AsfftfileXtra::close(ObjectType type) {
    AsfftfileXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AsfftfileXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AsfftfileXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AsfftfileXtra::m_openQuicktimeFile, 0)
XOBJSTUB(AsfftfileXtra::m_closeFile, 0)
XOBJSTUB(AsfftfileXtra::m_getSamples, 0)
XOBJSTUB(AsfftfileXtra::m_getAmplitude, 0)
XOBJSTUB(AsfftfileXtra::m_getFFT, 0)
XOBJSTUB(AsfftfileXtra::m_getSampleRate, 0)
XOBJSTUB(AsfftfileXtra::m_getEqualizedFreqCount, 0)
XOBJSTUB(AsfftfileXtra::m_getError, 0)
XOBJSTUB(AsfftfileXtra::m_", 0)

}
