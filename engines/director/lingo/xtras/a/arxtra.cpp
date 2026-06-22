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
#include "director/lingo/xtras/a/arxtra.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra arXtra
-- arXtra v0.1.9 (c) 2009 Valentin Schmidt
-- contact: fluxus@freenet.de
-- https://valentin.dasdeck.com

new object me, any cameraFileOrList, integer w, integer h
setConfidenceTreshold object me, float minConfidence
loadPatternFile object me, string patternFile, *patternWidth, patternHeight
loadPatternList object me, list patternList, *patternWidth, patternHeight
unloadAllPatterns object me
detectSingleMarker object me, object ímage
detectMarkers object me, object ímage
"
 */

namespace Director {

const char *ArxtraXtra::xlibName = "Arxtra";
const XlibFileDesc ArxtraXtra::fileNames[] = {
	{ "arxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ArxtraXtra::m_new,		 3, 3,	900 },
	{ "setConfidenceTreshold",				ArxtraXtra::m_setConfidenceTreshold,		 1, 1,	900 },
	{ "loadPatternFile",				ArxtraXtra::m_loadPatternFile,		 3, 3,	900 },
	{ "loadPatternList",				ArxtraXtra::m_loadPatternList,		 3, 3,	900 },
	{ "unloadAllPatterns",				ArxtraXtra::m_unloadAllPatterns,		 0, 0,	900 },
	{ "detectSingleMarker",				ArxtraXtra::m_detectSingleMarker,		 1, 1,	900 },
	{ "detectMarkers",				ArxtraXtra::m_detectMarkers,		 1, 1,	900 },
	{ """,				ArxtraXtra::m_",		 -1, -1,	900 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ArxtraXtraObject::ArxtraXtraObject(ObjectType ObjectType) :Object<ArxtraXtraObject>("Arxtra") {
	_objType = ObjectType;
}

bool ArxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ArxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ArxtraXtra::xlibName);
	warning("ArxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ArxtraXtra::open(ObjectType type, const Common::Path &path) {
    ArxtraXtraObject::initMethods(xlibMethods);
    ArxtraXtraObject *xobj = new ArxtraXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ArxtraXtra::close(ObjectType type) {
    ArxtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ArxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ArxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ArxtraXtra::m_setConfidenceTreshold, 0)
XOBJSTUB(ArxtraXtra::m_loadPatternFile, 0)
XOBJSTUB(ArxtraXtra::m_loadPatternList, 0)
XOBJSTUB(ArxtraXtra::m_unloadAllPatterns, 0)
XOBJSTUB(ArxtraXtra::m_detectSingleMarker, 0)
XOBJSTUB(ArxtraXtra::m_detectMarkers, 0)
XOBJSTUB(ArxtraXtra::m_", 0)

}
