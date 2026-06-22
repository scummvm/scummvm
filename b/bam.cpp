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
#include "director/lingo/xtras/b/bam.h"

/**************************************************
 *
 * USED IN:
 * Spy Kids 3-D: Game Over
 *
 **************************************************/

/*
-- xtra BAM
--
-- Arcade Simulation Methods
--
New object me, string sBaseXMLPath, string sPathSeparator, symbol symCPExecuteHAndler, symBAMEventHandlerName
GetState object me                                     -- Returns a property list with a growing number of BAM state data... (#symXtraState, #symGameState, #iCurrentDifficultyLevel, etc...)
Start object me, any plstParameters                    -- Example: [#sPlayer:"buzz", #plstMissionData:[...], #plstFu*els:[...], #symGame:#miner, #iLevel:2, #oOwner:me, ...]. 
Stop object me
Maintenance object me                                  -- Ticks the simulation. g_fGameTimestamp is retrieved from Lingo.
Pause object me
Resume object me
Finish object me                                       -- Fires the Finish Event Artificially. For Debugging Purposes.
EndLevel object me, symbol symReason                   -- Ends the Level. Valid reasons are: #OutOfFu*el, #ExitingGame, and #LevelComplete 
AlterSpeed object me, any plstParameters               -- [#fDesired:47.2 (percentage speed), #fTime:1.0 (optional), #cpFinish:[...] (optional) ]. 
GetSpeed object me                                     -- Returns the current percentage speed of the simulation. 
GetInputChannels object me, any plstParameters
SendInput object me, any plstParameters
SetDebugLevel object me, integer bAlertOnErrors
SetSecretsEnabled object me, integer bEnabled
GetProperties object me, integer iBamID                -- Returns a property list of the object's properties. 
GetAllProperties object me                             -- Returns a property list of all objects' properties. 
GetPointValue object me, integer iBamID                -- Returns a float (the point value of the object). 
GetNewAssetObjects object me                           -- Returns a list of new asset proplists (ex. [#iBamID: 47, #symAssetID: #EnemyShip_1] )
GetGameState object me									-- Returns the current gameplay state as a string ('intro', 'outro', 'fun')
SetGameState object me, string sGameState				-- Sets the game state. Accepted values are: 'intro', 'outro', 'fun'
--
-- Viewport Methods
--
GetCamera object me                            -- Returns the BamID of the current camera, or 0 if the camera is invalid/not attached to an entity. 
GetCameraLocation object me                    -- Returns the world coordinates of the camera as a point. 
SetCameraToEntity object me, integer iBamID    -- Sets the camera to the specified BamID. Returns 0 on success, an error code on failure.
SetCameraToPoint object me, any ptCamera       -- Sets the camera to any point within the worldsize. Returns 0 on success, an error code on failure.
GetCameraOffset object me                      -- Returns the offset from center as a point value.
SetCameraOffset object me, any ptOffset        -- Sets the camera offset from center point(0,0).
GetDisplayRect  object me                      -- Returns the adjusted display rect Director can use to set the drawrect.
GetLocalOrigin object me                       -- Returns the adjusted screen location at 0,0 (top left).
AutoUpdateRect  object me, integer bUpdate     -- Off by default, set this flag to allow the simulation to set the drawrect on maintenance. (NOT IMPLEMENTED)
--
-- Global Library Methods
* PointToDirectorAngle float X, float Y
"
 */

namespace Director {

const char *BamXtra::xlibName = "Bam";
const XlibFileDesc BamXtra::fileNames[] = {
	{ "bam",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "New",				BamXtra::m_New,		 4, 4,	850 },
	{ "GetState",				BamXtra::m_GetState,		 0, 0,	850 },
	{ "Start",				BamXtra::m_Start,		 1, 1,	850 },
	{ "Stop",				BamXtra::m_Stop,		 0, 0,	850 },
	{ "Maintenance",				BamXtra::m_Maintenance,		 0, 0,	850 },
	{ "Pause",				BamXtra::m_Pause,		 0, 0,	850 },
	{ "Resume",				BamXtra::m_Resume,		 0, 0,	850 },
	{ "Finish",				BamXtra::m_Finish,		 0, 0,	850 },
	{ "EndLevel",				BamXtra::m_EndLevel,		 1, 1,	850 },
	{ "AlterSpeed",				BamXtra::m_AlterSpeed,		 1, 1,	850 },
	{ "GetSpeed",				BamXtra::m_GetSpeed,		 0, 0,	850 },
	{ "GetInputChannels",				BamXtra::m_GetInputChannels,		 1, 1,	850 },
	{ "SendInput",				BamXtra::m_SendInput,		 1, 1,	850 },
	{ "SetDebugLevel",				BamXtra::m_SetDebugLevel,		 1, 1,	850 },
	{ "SetSecretsEnabled",				BamXtra::m_SetSecretsEnabled,		 1, 1,	850 },
	{ "GetProperties",				BamXtra::m_GetProperties,		 1, 1,	850 },
	{ "GetAllProperties",				BamXtra::m_GetAllProperties,		 0, 0,	850 },
	{ "GetPointValue",				BamXtra::m_GetPointValue,		 1, 1,	850 },
	{ "GetNewAssetObjects",				BamXtra::m_GetNewAssetObjects,		 0, 0,	850 },
	{ "GetGameState",				BamXtra::m_GetGameState,		 0, 0,	850 },
	{ "SetGameState",				BamXtra::m_SetGameState,		 1, 1,	850 },
	{ "GetCamera",				BamXtra::m_GetCamera,		 0, 0,	850 },
	{ "GetCameraLocation",				BamXtra::m_GetCameraLocation,		 0, 0,	850 },
	{ "SetCameraToEntity",				BamXtra::m_SetCameraToEntity,		 1, 1,	850 },
	{ "SetCameraToPoint",				BamXtra::m_SetCameraToPoint,		 1, 1,	850 },
	{ "GetCameraOffset",				BamXtra::m_GetCameraOffset,		 0, 0,	850 },
	{ "SetCameraOffset",				BamXtra::m_SetCameraOffset,		 1, 1,	850 },
	{ "GetDisplayRect",				BamXtra::m_GetDisplayRect,		 0, 0,	850 },
	{ "GetLocalOrigin",				BamXtra::m_GetLocalOrigin,		 0, 0,	850 },
	{ "AutoUpdateRect",				BamXtra::m_AutoUpdateRect,		 1, 1,	850 },
	{ """,				BamXtra::m_",		 -1, -1,	850 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "PointToDirectorAngle", BamXtra::m_PointToDirectorAngle, 2, 2, 850, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BamXtraObject::BamXtraObject(ObjectType ObjectType) :Object<BamXtraObject>("Bam") {
	_objType = ObjectType;
}

bool BamXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BamXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BamXtra::xlibName);
	warning("BamXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BamXtra::open(ObjectType type, const Common::Path &path) {
    BamXtraObject::initMethods(xlibMethods);
    BamXtraObject *xobj = new BamXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BamXtra::close(ObjectType type) {
    BamXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BamXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BamXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BamXtra::m_New, 0)
XOBJSTUB(BamXtra::m_GetState, 0)
XOBJSTUB(BamXtra::m_Start, 0)
XOBJSTUB(BamXtra::m_Stop, 0)
XOBJSTUB(BamXtra::m_Maintenance, 0)
XOBJSTUB(BamXtra::m_Pause, 0)
XOBJSTUB(BamXtra::m_Resume, 0)
XOBJSTUB(BamXtra::m_Finish, 0)
XOBJSTUB(BamXtra::m_EndLevel, 0)
XOBJSTUB(BamXtra::m_AlterSpeed, 0)
XOBJSTUB(BamXtra::m_GetSpeed, 0)
XOBJSTUB(BamXtra::m_GetInputChannels, 0)
XOBJSTUB(BamXtra::m_SendInput, 0)
XOBJSTUB(BamXtra::m_SetDebugLevel, 0)
XOBJSTUB(BamXtra::m_SetSecretsEnabled, 0)
XOBJSTUB(BamXtra::m_GetProperties, 0)
XOBJSTUB(BamXtra::m_GetAllProperties, 0)
XOBJSTUB(BamXtra::m_GetPointValue, 0)
XOBJSTUB(BamXtra::m_GetNewAssetObjects, 0)
XOBJSTUB(BamXtra::m_GetGameState, 0)
XOBJSTUB(BamXtra::m_SetGameState, 0)
XOBJSTUB(BamXtra::m_GetCamera, 0)
XOBJSTUB(BamXtra::m_GetCameraLocation, 0)
XOBJSTUB(BamXtra::m_SetCameraToEntity, 0)
XOBJSTUB(BamXtra::m_SetCameraToPoint, 0)
XOBJSTUB(BamXtra::m_GetCameraOffset, 0)
XOBJSTUB(BamXtra::m_SetCameraOffset, 0)
XOBJSTUB(BamXtra::m_GetDisplayRect, 0)
XOBJSTUB(BamXtra::m_GetLocalOrigin, 0)
XOBJSTUB(BamXtra::m_AutoUpdateRect, 0)
XOBJSTUB(BamXtra::m_PointToDirectorAngle, 0)
XOBJSTUB(BamXtra::m_", 0)

}
