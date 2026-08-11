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
#include "director/lingo/xlibs/e/ecdctrl.h"

/**************************************************
 *
 * USED IN:
 * The Legend of Lotus Spring
 *
 **************************************************/

/*
-- ECDCTRL External Factory v0.94 20oct95 rcj
--ECDCTRL
II     mNew, deviceNum                   -- Creates a new instance of the XObject for deviceNum
X      mDispose                          -- Disposes of an XObject instance
S      mName                             -- Returns the XObject name
SS     mCapability, capsStr              -- Returns true if caps in capsStr available
SS     mInfo, infoStr                    -- Returns device information
IS     mPlay, positionStr                -- Plays from/to positionStr
IS     mSeek, positionStr                -- Repositions playback to positionStr
IS     mSet, setStr                      -- Sets attributes in setStr
SS     mStatus, statusStr                -- Returns status requested by statusStr
I      mStop                             -- Stops the device
SI     mError, errorNum                  -- Returns an error string corresponding to errorNum
SS     mInterpretMCI, sendStr            -- Sends the string sendStr directly to MCI
III    mSetVolume, leftNum, rightNum     -- Sets the cdaudio volume
I      mGetVolume                        -- Returns the cdaudio volume,
                                         -- upper word is left, lower word is right
I      mTotalDevices                     -- Returns the number of cdaudio devices
II     mSetMixer, channelNum             -- Sets the mixer channel to channelNum,
                                         -- returns the current channel
II     mTest, testNum                    -- Tests MCI commands
II     mIdle, delayNum                   -- Restarts stalled CD playback after a minimum delay
                                         -- where delayNum is the delay in milliseconds
 */

namespace Director {

const char *ECDCtrlXObj::xlibName = "ECDCtrl";
const XlibFileDesc ECDCtrlXObj::fileNames[] = {
	{ "ECDCTRL",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ECDCtrlXObj::m_new,		 1, 1,	400 },
	{ "dispose",				ECDCtrlXObj::m_dispose,		 0, 0,	400 },
	{ "name",				ECDCtrlXObj::m_name,		 0, 0,	400 },
	{ "capability",				ECDCtrlXObj::m_capability,		 1, 1,	400 },
	{ "info",				ECDCtrlXObj::m_info,		 1, 1,	400 },
	{ "play",				ECDCtrlXObj::m_play,		 1, 1,	400 },
	{ "seek",				ECDCtrlXObj::m_seek,		 1, 1,	400 },
	{ "set",				ECDCtrlXObj::m_set,		 1, 1,	400 },
	{ "status",				ECDCtrlXObj::m_status,		 1, 1,	400 },
	{ "stop",				ECDCtrlXObj::m_stop,		 0, 0,	400 },
	{ "error",				ECDCtrlXObj::m_error,		 1, 1,	400 },
	{ "interpretMCI",				ECDCtrlXObj::m_interpretMCI,		 1, 1,	400 },
	{ "setVolume",				ECDCtrlXObj::m_setVolume,		 2, 2,	400 },
	{ "getVolume",				ECDCtrlXObj::m_getVolume,		 0, 0,	400 },
	{ "totalDevices",				ECDCtrlXObj::m_totalDevices,		 0, 0,	400 },
	{ "setMixer",				ECDCtrlXObj::m_setMixer,		 1, 1,	400 },
	{ "test",				ECDCtrlXObj::m_test,		 1, 1,	400 },
	{ "idle",				ECDCtrlXObj::m_idle,		 1, 1,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ECDCtrlXObject::ECDCtrlXObject(ObjectType ObjectType) :Object<ECDCtrlXObject>("ECDCtrl") {
	_objType = ObjectType;
}

void ECDCtrlXObj::open(ObjectType type, const Common::Path &path) {
    ECDCtrlXObject::initMethods(xlibMethods);
    ECDCtrlXObject *xobj = new ECDCtrlXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
        g_lingo->_openXtraObjects.push_back(xobj);
    }
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ECDCtrlXObj::close(ObjectType type) {
    ECDCtrlXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ECDCtrlXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ECDCtrlXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(ECDCtrlXObj::m_dispose)
XOBJSTUB(ECDCtrlXObj::m_name, "")
XOBJSTUB(ECDCtrlXObj::m_capability, "")
XOBJSTUB(ECDCtrlXObj::m_info, "")
XOBJSTUB(ECDCtrlXObj::m_play, 0)
XOBJSTUB(ECDCtrlXObj::m_seek, 0)
XOBJSTUB(ECDCtrlXObj::m_set, 0)
XOBJSTUB(ECDCtrlXObj::m_status, "")
XOBJSTUB(ECDCtrlXObj::m_stop, 0)
XOBJSTUB(ECDCtrlXObj::m_error, "")
XOBJSTUB(ECDCtrlXObj::m_interpretMCI, "")
XOBJSTUB(ECDCtrlXObj::m_setVolume, 0)
XOBJSTUB(ECDCtrlXObj::m_getVolume, 0)
XOBJSTUB(ECDCtrlXObj::m_totalDevices, 0)
XOBJSTUB(ECDCtrlXObj::m_setMixer, 0)
XOBJSTUB(ECDCtrlXObj::m_test, 0)
XOBJSTUB(ECDCtrlXObj::m_idle, 0)

}
