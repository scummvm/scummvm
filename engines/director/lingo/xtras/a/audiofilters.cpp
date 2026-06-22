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
#include "director/lingo/xtras/a/audiofilters.h"

/**************************************************
 *
 * USED IN:
 * Standard Director Xtra
 *
 **************************************************/

/*
-- xtra AudioFilters -- version 11.5.0.r593
* audioFilter * -- creates new audioFilter
"
 */

namespace Director {

const char *AudiofiltersXtra::xlibName = "Audiofilters";
const XlibFileDesc AudiofiltersXtra::fileNames[] = {
	{ "audiofilters",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ """,				AudiofiltersXtra::m_",		 -1, -1,	1100 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "audioFilter", AudiofiltersXtra::m_audioFilter, -1, 0, 1100, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AudiofiltersXtraObject::AudiofiltersXtraObject(ObjectType ObjectType) :Object<AudiofiltersXtraObject>("Audiofilters") {
	_objType = ObjectType;
}

bool AudiofiltersXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AudiofiltersXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AudiofiltersXtra::xlibName);
	warning("AudiofiltersXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AudiofiltersXtra::open(ObjectType type, const Common::Path &path) {
    AudiofiltersXtraObject::initMethods(xlibMethods);
    AudiofiltersXtraObject *xobj = new AudiofiltersXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AudiofiltersXtra::close(ObjectType type) {
    AudiofiltersXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AudiofiltersXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AudiofiltersXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AudiofiltersXtra::m_audioFilter, 0)
XOBJSTUB(AudiofiltersXtra::m_", 0)

}
