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
#include "director/lingo/xlibs/henry.h"

/**************************************************
 *
 * USED IN:
 * Mummy: Tomb of the Pharaoh
 * Frankenstein: Through the Eyes of the Monster
 *
 **************************************************/

/*
-- Henry XObject. Mar 21, 95 JPY
--Henry
ISS    mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mName
SIIS           mUserHitMouse                   --
XSSS           mStartNewPosition                       --
XSS            mStartNewPositionSubView --
S                      mCommittNewPosition --
S                      mDoLeftTurn                             --
S              mDoRightTurn            --
SII    mCheckCursor            --
SII    mGetRollOverActions --
S              mGetEntryActions        --
S              mPeekEntryActions       --
S              mGetExitActions         --
S              mSetSavedPosition
IS             mGetStateVariable       --
XSI    mSetStateVariable       --
SS             mGetStringVariable      --
XSS    mSetStringVariable      --
XSI    mStartTimer             --
XS             mAbortTimer             --
S              mCheckTimers            --
SS             mGetTimerStatus
SS             mPickUpItem
XS             mDropItem               --
XS             mDumpItem
SS             mReturnItem
SS             mIngestItem
SSS    mPutItemInContainer
SSSSS  mPutItemAtLocation
S              mGetCurrentPosition
S              mGetPlayerProperties
SSS    mSaveGame               --
SS             mLoadGame               --
XI             mShowCursor             --
X              mReleaseCursor          --
S              mGetRoomList            --
SI             mGetItemList --
S              mGetCurrentItem --
S              mGetCurrentContainer --
S              mGetCurrentInteraction --
SS             mGetItemLocation --
SS             mGetContainerContents --
X              mPauseGame --
X              mResumeGame --
 */

namespace Director {

const char *const HenryXObj::xlibName = "Henry";
const XlibFileDesc HenryXObj::fileNames[] = {
	{ "HENRY",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				HenryXObj::m_new,		 2, 2,	400 },
	{ "dispose",				HenryXObj::m_dispose,		 0, 0,	400 },
	{ "name",				HenryXObj::m_name,		 0, 0,	400 },
	{ "userHitMouse",				HenryXObj::m_userHitMouse,		 3, 3,	400 },
	{ "startNewPosition",				HenryXObj::m_startNewPosition,		 3, 3,	400 },
	{ "startNewPositionSubView",				HenryXObj::m_startNewPositionSubView,		 2, 2,	400 },
	{ "committNewPosition",				HenryXObj::m_committNewPosition,		 0, 0,	400 },
	{ "doLeftTurn",				HenryXObj::m_doLeftTurn,		 0, 0,	400 },
	{ "doRightTurn",				HenryXObj::m_doRightTurn,		 0, 0,	400 },
	{ "checkCursor",				HenryXObj::m_checkCursor,		 2, 2,	400 },
	{ "getRollOverActions",				HenryXObj::m_getRollOverActions,		 2, 2,	400 },
	{ "getEntryActions",				HenryXObj::m_getEntryActions,		 0, 0,	400 },
	{ "peekEntryActions",				HenryXObj::m_peekEntryActions,		 0, 0,	400 },
	{ "getExitActions",				HenryXObj::m_getExitActions,		 0, 0,	400 },
	{ "setSavedPosition",				HenryXObj::m_setSavedPosition,		 0, 0,	400 },
	{ "getStateVariable",				HenryXObj::m_getStateVariable,		 1, 1,	400 },
	{ "setStateVariable",				HenryXObj::m_setStateVariable,		 2, 2,	400 },
	{ "getStringVariable",				HenryXObj::m_getStringVariable,		 1, 1,	400 },
	{ "setStringVariable",				HenryXObj::m_setStringVariable,		 2, 2,	400 },
	{ "startTimer",				HenryXObj::m_startTimer,		 2, 2,	400 },
	{ "abortTimer",				HenryXObj::m_abortTimer,		 1, 1,	400 },
	{ "checkTimers",				HenryXObj::m_checkTimers,		 0, 0,	400 },
	{ "getTimerStatus",				HenryXObj::m_getTimerStatus,		 1, 1,	400 },
	{ "pickUpItem",				HenryXObj::m_pickUpItem,		 1, 1,	400 },
	{ "dropItem",				HenryXObj::m_dropItem,		 1, 1,	400 },
	{ "dumpItem",				HenryXObj::m_dumpItem,		 1, 1,	400 },
	{ "returnItem",				HenryXObj::m_returnItem,		 1, 1,	400 },
	{ "ingestItem",				HenryXObj::m_ingestItem,		 1, 1,	400 },
	{ "putItemInContainer",				HenryXObj::m_putItemInContainer,		 2, 2,	400 },
	{ "putItemAtLocation",				HenryXObj::m_putItemAtLocation,		 4, 4,	400 },
	{ "getCurrentPosition",				HenryXObj::m_getCurrentPosition,		 0, 0,	400 },
	{ "getPlayerProperties",				HenryXObj::m_getPlayerProperties,		 0, 0,	400 },
	{ "saveGame",				HenryXObj::m_saveGame,		 2, 2,	400 },
	{ "loadGame",				HenryXObj::m_loadGame,		 1, 1,	400 },
	{ "showCursor",				HenryXObj::m_showCursor,		 1, 1,	400 },
	{ "releaseCursor",				HenryXObj::m_releaseCursor,		 0, 0,	400 },
	{ "getRoomList",				HenryXObj::m_getRoomList,		 0, 0,	400 },
	{ "getItemList",				HenryXObj::m_getItemList,		 1, 1,	400 },
	{ "getCurrentItem",				HenryXObj::m_getCurrentItem,		 0, 0,	400 },
	{ "getCurrentContainer",				HenryXObj::m_getCurrentContainer,		 0, 0,	400 },
	{ "getCurrentInteraction",				HenryXObj::m_getCurrentInteraction,		 0, 0,	400 },
	{ "getItemLocation",				HenryXObj::m_getItemLocation,		 1, 1,	400 },
	{ "getContainerContents",				HenryXObj::m_getContainerContents,		 1, 1,	400 },
	{ "pauseGame",				HenryXObj::m_pauseGame,		 0, 0,	400 },
	{ "resumeGame",				HenryXObj::m_resumeGame,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

HenryXObject::HenryXObject(ObjectType ObjectType) :Object<HenryXObject>("Henry") {
	_objType = ObjectType;
}

void HenryXObj::open(ObjectType type, const Common::Path &path) {
    HenryXObject::initMethods(xlibMethods);
    HenryXObject *xobj = new HenryXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void HenryXObj::close(ObjectType type) {
    HenryXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void HenryXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("HenryXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(HenryXObj::m_dispose)
XOBJSTUB(HenryXObj::m_name, "")
XOBJSTUB(HenryXObj::m_userHitMouse, "")
XOBJSTUBNR(HenryXObj::m_startNewPosition)
XOBJSTUBNR(HenryXObj::m_startNewPositionSubView)
XOBJSTUB(HenryXObj::m_committNewPosition, "")
XOBJSTUB(HenryXObj::m_doLeftTurn, "")
XOBJSTUB(HenryXObj::m_doRightTurn, "")
XOBJSTUB(HenryXObj::m_checkCursor, "")
XOBJSTUB(HenryXObj::m_getRollOverActions, "")
XOBJSTUB(HenryXObj::m_getEntryActions, "")
XOBJSTUB(HenryXObj::m_peekEntryActions, "")
XOBJSTUB(HenryXObj::m_getExitActions, "")
XOBJSTUB(HenryXObj::m_setSavedPosition, "")
XOBJSTUB(HenryXObj::m_getStateVariable, 0)
XOBJSTUBNR(HenryXObj::m_setStateVariable)
XOBJSTUB(HenryXObj::m_getStringVariable, "")
XOBJSTUBNR(HenryXObj::m_setStringVariable)
XOBJSTUBNR(HenryXObj::m_startTimer)
XOBJSTUBNR(HenryXObj::m_abortTimer)
XOBJSTUB(HenryXObj::m_checkTimers, "")
XOBJSTUB(HenryXObj::m_getTimerStatus, "")
XOBJSTUB(HenryXObj::m_pickUpItem, "")
XOBJSTUBNR(HenryXObj::m_dropItem)
XOBJSTUBNR(HenryXObj::m_dumpItem)
XOBJSTUB(HenryXObj::m_returnItem, "")
XOBJSTUB(HenryXObj::m_ingestItem, "")
XOBJSTUB(HenryXObj::m_putItemInContainer, "")
XOBJSTUB(HenryXObj::m_putItemAtLocation, "")
XOBJSTUB(HenryXObj::m_getCurrentPosition, "")
XOBJSTUB(HenryXObj::m_getPlayerProperties, "")
XOBJSTUB(HenryXObj::m_saveGame, "")
XOBJSTUB(HenryXObj::m_loadGame, "")
XOBJSTUBNR(HenryXObj::m_showCursor)
XOBJSTUBNR(HenryXObj::m_releaseCursor)
XOBJSTUB(HenryXObj::m_getRoomList, "")
XOBJSTUB(HenryXObj::m_getItemList, "")
XOBJSTUB(HenryXObj::m_getCurrentItem, "")
XOBJSTUB(HenryXObj::m_getCurrentContainer, "")
XOBJSTUB(HenryXObj::m_getCurrentInteraction, "")
XOBJSTUB(HenryXObj::m_getItemLocation, "")
XOBJSTUB(HenryXObj::m_getContainerContents, "")
XOBJSTUBNR(HenryXObj::m_pauseGame)
XOBJSTUBNR(HenryXObj::m_resumeGame)

}
