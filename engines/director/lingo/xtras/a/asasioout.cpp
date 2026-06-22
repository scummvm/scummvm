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
#include "director/lingo/xtras/a/asasioout.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra asASIOout -- This Xtra is a multichannel sound scripting Xtra: it enables Director to output sound to sound hardware through ASIO drivers. Typically, this gives the ability to adress individually more than 2 speakers through specialized sound hardware.
-- http://www.as-ci.net/asASIO/index.html
-- Mac OS Only
new object me
connect object me
-- Props --
setGlobalVolume object me, float volume
getGlobalVolume object me
getChannelCount object me
getError object me -- returns the last error string
-- Sound File --
preloadSound object me, string filePath, bool looped -- returns a soundID if > 0, or an error nb, if < 0
unloadSound object me, integer soundID
getSoundChannelCount object me, integer soundID
playSound object me, integer soundID, list volumes
stopSound object me, integer soundID
pauseSound object me, integer soundID
getSoundState object me, integer soundID
getSoundDuration object me, integer soundID -- returns float seconds
getSoundElapsed object me, integer soundID -- returns float seconds
setVolumes object me, integer soundID, list volumes
getVolumes object me, integer soundID -- Returns a list of volumes
"
 */

namespace Director {

const char *AsasiooutXtra::xlibName = "Asasioout";
const XlibFileDesc AsasiooutXtra::fileNames[] = {
	{ "asasioout",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AsasiooutXtra::m_new,		 0, 0,	600 },
	{ "connect",				AsasiooutXtra::m_connect,		 0, 0,	600 },
	{ "setGlobalVolume",				AsasiooutXtra::m_setGlobalVolume,		 1, 1,	600 },
	{ "getGlobalVolume",				AsasiooutXtra::m_getGlobalVolume,		 0, 0,	600 },
	{ "getChannelCount",				AsasiooutXtra::m_getChannelCount,		 0, 0,	600 },
	{ "getError",				AsasiooutXtra::m_getError,		 0, 0,	600 },
	{ "preloadSound",				AsasiooutXtra::m_preloadSound,		 2, 2,	600 },
	{ "unloadSound",				AsasiooutXtra::m_unloadSound,		 1, 1,	600 },
	{ "getSoundChannelCount",				AsasiooutXtra::m_getSoundChannelCount,		 1, 1,	600 },
	{ "playSound",				AsasiooutXtra::m_playSound,		 2, 2,	600 },
	{ "stopSound",				AsasiooutXtra::m_stopSound,		 1, 1,	600 },
	{ "pauseSound",				AsasiooutXtra::m_pauseSound,		 1, 1,	600 },
	{ "getSoundState",				AsasiooutXtra::m_getSoundState,		 1, 1,	600 },
	{ "getSoundDuration",				AsasiooutXtra::m_getSoundDuration,		 1, 1,	600 },
	{ "getSoundElapsed",				AsasiooutXtra::m_getSoundElapsed,		 1, 1,	600 },
	{ "setVolumes",				AsasiooutXtra::m_setVolumes,		 2, 2,	600 },
	{ "getVolumes",				AsasiooutXtra::m_getVolumes,		 1, 1,	600 },
	{ """,				AsasiooutXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AsasiooutXtraObject::AsasiooutXtraObject(ObjectType ObjectType) :Object<AsasiooutXtraObject>("Asasioout") {
	_objType = ObjectType;
}

bool AsasiooutXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AsasiooutXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AsasiooutXtra::xlibName);
	warning("AsasiooutXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AsasiooutXtra::open(ObjectType type, const Common::Path &path) {
    AsasiooutXtraObject::initMethods(xlibMethods);
    AsasiooutXtraObject *xobj = new AsasiooutXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AsasiooutXtra::close(ObjectType type) {
    AsasiooutXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AsasiooutXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AsasiooutXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AsasiooutXtra::m_connect, 0)
XOBJSTUB(AsasiooutXtra::m_setGlobalVolume, 0)
XOBJSTUB(AsasiooutXtra::m_getGlobalVolume, 0)
XOBJSTUB(AsasiooutXtra::m_getChannelCount, 0)
XOBJSTUB(AsasiooutXtra::m_getError, 0)
XOBJSTUB(AsasiooutXtra::m_preloadSound, 0)
XOBJSTUB(AsasiooutXtra::m_unloadSound, 0)
XOBJSTUB(AsasiooutXtra::m_getSoundChannelCount, 0)
XOBJSTUB(AsasiooutXtra::m_playSound, 0)
XOBJSTUB(AsasiooutXtra::m_stopSound, 0)
XOBJSTUB(AsasiooutXtra::m_pauseSound, 0)
XOBJSTUB(AsasiooutXtra::m_getSoundState, 0)
XOBJSTUB(AsasiooutXtra::m_getSoundDuration, 0)
XOBJSTUB(AsasiooutXtra::m_getSoundElapsed, 0)
XOBJSTUB(AsasiooutXtra::m_setVolumes, 0)
XOBJSTUB(AsasiooutXtra::m_getVolumes, 0)
XOBJSTUB(AsasiooutXtra::m_", 0)

}
