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
#include "director/lingo/xtras/b/blinker.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra Blinker
-- Blinker (tm)
-- by Penworks Corporation
-- Copyright (c) 1996 by Penworks Corporation.
-- All rights reserved.
-- Net: admin@penworks.com  Web: www.penworks.com
new object me
* blinkUpdate                    -- Updates any blinks
* blink integer, *               -- Blinks a channel
* blinkSetDefaults integer, *    -- Change default settings for blinker
* blinkKill integer, *           -- Kill blinking on a specified channel
* blinkKillAll *                 -- Kills blinking on all channels
* blinkSuspend integer           -- Suspends blinking on a particular channel
* blinkSuspendAll                -- Suspends blinking on all channels
* blinkResume integer            -- Resumes blinking on a particular channel
* blinkResumeAll                 -- Resumes blinking on all channels
* blinking integer               -- Returns true if channel is blinking
* blinkSuspended integer         -- Returns true if channel is suspended
* blinkRegister string           -- Registers for use in a projector
* blinkVersion                   -- Returns current version of blinker
"

 */

namespace Director {

const char *BlinkerXtra::xlibName = "Blinker";
const XlibFileDesc BlinkerXtra::fileNames[] = {
	{ "blinker",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BlinkerXtra::m_new,		 0, 0,	500 },
	{ """,				BlinkerXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "blinkUpdate", BlinkerXtra::m_blinkUpdate, 0, 0, 500, HBLTIN },
	{ "blink", BlinkerXtra::m_blink, -1, 0, 500, HBLTIN },
	{ "blinkSetDefaults", BlinkerXtra::m_blinkSetDefaults, -1, 0, 500, HBLTIN },
	{ "blinkKill", BlinkerXtra::m_blinkKill, -1, 0, 500, HBLTIN },
	{ "blinkKillAll", BlinkerXtra::m_blinkKillAll, -1, 0, 500, HBLTIN },
	{ "blinkSuspend", BlinkerXtra::m_blinkSuspend, 1, 1, 500, HBLTIN },
	{ "blinkSuspendAll", BlinkerXtra::m_blinkSuspendAll, 0, 0, 500, HBLTIN },
	{ "blinkResume", BlinkerXtra::m_blinkResume, 1, 1, 500, HBLTIN },
	{ "blinkResumeAll", BlinkerXtra::m_blinkResumeAll, 0, 0, 500, HBLTIN },
	{ "blinking", BlinkerXtra::m_blinking, 1, 1, 500, HBLTIN },
	{ "blinkSuspended", BlinkerXtra::m_blinkSuspended, 1, 1, 500, HBLTIN },
	{ "blinkRegister", BlinkerXtra::m_blinkRegister, 1, 1, 500, HBLTIN },
	{ "blinkVersion", BlinkerXtra::m_blinkVersion, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BlinkerXtraObject::BlinkerXtraObject(ObjectType ObjectType) :Object<BlinkerXtraObject>("Blinker") {
	_objType = ObjectType;
}

bool BlinkerXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BlinkerXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BlinkerXtra::xlibName);
	warning("BlinkerXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BlinkerXtra::open(ObjectType type, const Common::Path &path) {
    BlinkerXtraObject::initMethods(xlibMethods);
    BlinkerXtraObject *xobj = new BlinkerXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BlinkerXtra::close(ObjectType type) {
    BlinkerXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BlinkerXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BlinkerXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BlinkerXtra::m_blinkUpdate, 0)
XOBJSTUB(BlinkerXtra::m_blink, 0)
XOBJSTUB(BlinkerXtra::m_blinkSetDefaults, 0)
XOBJSTUB(BlinkerXtra::m_blinkKill, 0)
XOBJSTUB(BlinkerXtra::m_blinkKillAll, 0)
XOBJSTUB(BlinkerXtra::m_blinkSuspend, 0)
XOBJSTUB(BlinkerXtra::m_blinkSuspendAll, 0)
XOBJSTUB(BlinkerXtra::m_blinkResume, 0)
XOBJSTUB(BlinkerXtra::m_blinkResumeAll, 0)
XOBJSTUB(BlinkerXtra::m_blinking, 0)
XOBJSTUB(BlinkerXtra::m_blinkSuspended, 0)
XOBJSTUB(BlinkerXtra::m_blinkRegister, 0)
XOBJSTUB(BlinkerXtra::m_blinkVersion, 0)
XOBJSTUB(BlinkerXtra::m_", 0)

}
