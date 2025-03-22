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
#include "director/lingo/xtras/rtk.h"

/**************************************************
 *
 * USED IN:
 * Cracking the Conspiracy
 *
 **************************************************/

/*
-- xtra Rollover_Toolkit
-- Rollover Toolkit (tm)
-- by Penworks Corporation
-- Copyright (c) 1996 by Penworks Corporation.
-- All rights reserved.
-- Net: admin@penworks.com  Web: http://www.penworks.com
new object me
* CheckForRollovers * -- Put this in your 'on idle' handler
* CurrentRollover   -- Returns number of sprite rolled over
* EndAnyRollovers   -- Forces all activated rollovers to end
* ResetRollovers    -- Clears any pending rollovers
* EnableCastmemberDetection  -- Takes cast member changes into account
* DisableCastmemberDetection -- Ignores cast member changes (the default)
* GetCastmemberDetectionStatus -- Returns TRUE if detecting cast member changes
* AutoSetSpriteRange  -- One-time check to find active sprites
* MonitorAllSprites -- Watches all sprites (default)
* MonitorNoSprites  -- Watches no sprites
* MonitorSpriteRange integer, integer   -- Set specific range of sprites to check
* UnMonitorSpriteRange integer, integer   -- Frees specific range of sprites from checking
* MonitorSprite integer    -- Watches particular sprite
* UnMonitorSprite integer  -- Removes watch from a particular sprite
* GetMonitorStatus integer -- Returns TRUE if specified sprite is monitored
* DumpMonitorStatus -- Dumps the rollover status list to the message window
* ShowRTKVersion -- Displays the current version number and date of the Toolkit
* EnableMatteDetection -- Checks for Matte ink rollovers
* DisableMatteDetection -- Disables Matte ink detection (default)
* GetMatteDetectionStatus -- Returns TRUE if Matte detection is on
* rtkRegisterMac string    -- Registers for use on Macintosh
* rtkRegisterX16 string    -- Registers for use on 16-bit Windows
* rtkRegisterX32 string    -- Registers for use on 32-bit Windows

 */

namespace Director {

const char *RolloverToolkitXtra::xlibName = "RolloverToolkit";
const XlibFileDesc RolloverToolkitXtra::fileNames[] = {
	{ "RTK",   nullptr },
	{ "rollover_toolkit",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				RolloverToolkitXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "CheckForRollovers", RolloverToolkitXtra::m_CheckForRollovers, -1, 0, 500, HBLTIN },
	{ "CurrentRollover", RolloverToolkitXtra::m_CurrentRollover, 0, 0, 500, HBLTIN },
	{ "EndAnyRollovers", RolloverToolkitXtra::m_EndAnyRollovers, 0, 0, 500, HBLTIN },
	{ "ResetRollovers", RolloverToolkitXtra::m_ResetRollovers, 0, 0, 500, HBLTIN },
	{ "EnableCastmemberDetection", RolloverToolkitXtra::m_EnableCastmemberDetection, 0, 0, 500, HBLTIN },
	{ "DisableCastmemberDetection", RolloverToolkitXtra::m_DisableCastmemberDetection, 0, 0, 500, HBLTIN },
	{ "GetCastmemberDetectionStatus", RolloverToolkitXtra::m_GetCastmemberDetectionStatus, 0, 0, 500, HBLTIN },
	{ "AutoSetSpriteRange", RolloverToolkitXtra::m_AutoSetSpriteRange, 0, 0, 500, HBLTIN },
	{ "MonitorAllSprites", RolloverToolkitXtra::m_MonitorAllSprites, 0, 0, 500, HBLTIN },
	{ "MonitorNoSprites", RolloverToolkitXtra::m_MonitorNoSprites, 0, 0, 500, HBLTIN },
	{ "MonitorSpriteRange", RolloverToolkitXtra::m_MonitorSpriteRange, 2, 2, 500, HBLTIN },
	{ "UnMonitorSpriteRange", RolloverToolkitXtra::m_UnMonitorSpriteRange, 2, 2, 500, HBLTIN },
	{ "MonitorSprite", RolloverToolkitXtra::m_MonitorSprite, 1, 1, 500, HBLTIN },
	{ "UnMonitorSprite", RolloverToolkitXtra::m_UnMonitorSprite, 1, 1, 500, HBLTIN },
	{ "GetMonitorStatus", RolloverToolkitXtra::m_GetMonitorStatus, 1, 1, 500, HBLTIN },
	{ "DumpMonitorStatus", RolloverToolkitXtra::m_DumpMonitorStatus, 0, 0, 500, HBLTIN },
	{ "ShowRTKVersion", RolloverToolkitXtra::m_ShowRTKVersion, 0, 0, 500, HBLTIN },
	{ "EnableMatteDetection", RolloverToolkitXtra::m_EnableMatteDetection, 0, 0, 500, HBLTIN },
	{ "DisableMatteDetection", RolloverToolkitXtra::m_DisableMatteDetection, 0, 0, 500, HBLTIN },
	{ "GetMatteDetectionStatus", RolloverToolkitXtra::m_GetMatteDetectionStatus, 0, 0, 500, HBLTIN },
	{ "rtkRegisterMac", RolloverToolkitXtra::m_rtkRegisterMac, 1, 1, 500, HBLTIN },
	{ "rtkRegisterX16", RolloverToolkitXtra::m_rtkRegisterX16, 1, 1, 500, HBLTIN },
	{ "rtkRegisterX32", RolloverToolkitXtra::m_rtkRegisterX32, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

RolloverToolkitXtraObject::RolloverToolkitXtraObject(ObjectType ObjectType) :Object<RolloverToolkitXtraObject>("RolloverToolkit") {
	_objType = ObjectType;
}

bool RolloverToolkitXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum RolloverToolkitXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(RolloverToolkitXtra::xlibName);
	warning("RolloverToolkitXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void RolloverToolkitXtra::open(ObjectType type, const Common::Path &path) {
    RolloverToolkitXtraObject::initMethods(xlibMethods);
    RolloverToolkitXtraObject *xobj = new RolloverToolkitXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void RolloverToolkitXtra::close(ObjectType type) {
    RolloverToolkitXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void RolloverToolkitXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("RolloverToolkitXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(RolloverToolkitXtra::m_CheckForRollovers, 0)
XOBJSTUB(RolloverToolkitXtra::m_CurrentRollover, 0)
XOBJSTUB(RolloverToolkitXtra::m_EndAnyRollovers, 0)
XOBJSTUB(RolloverToolkitXtra::m_ResetRollovers, 0)
XOBJSTUB(RolloverToolkitXtra::m_EnableCastmemberDetection, 0)
XOBJSTUB(RolloverToolkitXtra::m_DisableCastmemberDetection, 0)
XOBJSTUB(RolloverToolkitXtra::m_GetCastmemberDetectionStatus, 0)
XOBJSTUB(RolloverToolkitXtra::m_AutoSetSpriteRange, 0)
XOBJSTUB(RolloverToolkitXtra::m_MonitorAllSprites, 0)
XOBJSTUB(RolloverToolkitXtra::m_MonitorNoSprites, 0)
XOBJSTUB(RolloverToolkitXtra::m_MonitorSpriteRange, 0)
XOBJSTUB(RolloverToolkitXtra::m_UnMonitorSpriteRange, 0)
XOBJSTUB(RolloverToolkitXtra::m_MonitorSprite, 0)
XOBJSTUB(RolloverToolkitXtra::m_UnMonitorSprite, 0)
XOBJSTUB(RolloverToolkitXtra::m_GetMonitorStatus, 0)
XOBJSTUB(RolloverToolkitXtra::m_DumpMonitorStatus, 0)
XOBJSTUB(RolloverToolkitXtra::m_ShowRTKVersion, 0)
XOBJSTUB(RolloverToolkitXtra::m_EnableMatteDetection, 0)
XOBJSTUB(RolloverToolkitXtra::m_DisableMatteDetection, 0)
XOBJSTUB(RolloverToolkitXtra::m_GetMatteDetectionStatus, 0)
XOBJSTUB(RolloverToolkitXtra::m_rtkRegisterMac, 0)
XOBJSTUB(RolloverToolkitXtra::m_rtkRegisterX16, 0)
XOBJSTUB(RolloverToolkitXtra::m_rtkRegisterX32, 0)

}
