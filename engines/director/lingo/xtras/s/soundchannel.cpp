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
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/s/soundchannel.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6-9, Opera Fatal, Oscar 4, Kommissar Kugelblitz
 *
 **************************************************/

// -- xtra SoundChannel
// new object me
// -- Template handlers --
// * sound * * -- Gets a sound object for the given channel.
// /* _soundXtra * * -- Private routine to get a sound object.

namespace Director {

const char *SoundChannelXtra::xlibName = "SoundChannel";
const XlibFileDesc SoundChannelXtra::fileNames[] = {
	{ "Sound Control",   nullptr },	// on-disk Xtra filename (Sound Control.x32)
	{ "soundchannel",    nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				SoundChannelXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "sound", SoundChannelXtra::m_sound, 1, 3, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SoundChannelXtraObject::SoundChannelXtraObject(ObjectType ObjectType) :Object<SoundChannelXtraObject>("SoundChannel") {
	_objType = ObjectType;
}

bool SoundChannelXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SoundChannelXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SoundChannelXtra::xlibName);
	warning("SoundChannelXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SoundChannelXtra::open(ObjectType type, const Common::Path &path) {
    SoundChannelXtraObject::initMethods(xlibMethods);
    SoundChannelXtraObject *xobj = new SoundChannelXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SoundChannelXtra::close(ObjectType type) {
    SoundChannelXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SoundChannelXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SoundChannelXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void SoundChannelXtra::m_sound(int nargs) {
	if (nargs >= 2) {
		LB::b_sound(nargs);
		g_lingo->push(Datum(0));
		return;
	}

	g_lingo->printSTUBWithArglist("SoundChannelXtra::m_sound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

}
