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
#include "director/lingo/xtras/qtvrxtra.h"

/**************************************************
 *
 * USED IN:
 * Safecracker
 *
 **************************************************/

/*
-- xtra QTVRXtra -- v. 1.0, Copyright 1997 Apple Computer, Inc. All Rights Reserved.
-- Methods listed as returning |<void> return <void> on error.
-----------------------------------
-- Initialization & Finalization --
-----------------------------------
new object me
forget object me -- for Director use only. Invoke by calling me = 0 
+ QTVREnter object xt --> integer (zero on success)  -- Initialize QTVR
+ QTVRExit  object xt                                -- Release QTVR
-----------------------------------
-- Methods for any QTVR movie    --
-----------------------------------
QTVROpen object me, string filename, string rectStrLTRB, string visibleStr --> string emptyOrError
  -- visibleStr is ("visible"|"invisible")
  -- Opens QTVR movie, e.g.: QTVROpen(vrInst, "qtvrtest.mov", "0,0,320,200", "visible")
  --  (If the movie couldn't be opened, the result is "Error: <message>")
QTVRClose object me  -- Closes QTVR movie
QTVRUpdate object me  -- Redraws QTVR movie with latest parameter settings
QTVRGetQTVRType object me --> string ("QTVRPanorama"|"QTVRObject"|"NotAQTVRType")
QTVRIdle object me -- Provides idle time to QTVR controller
QTVRMouseDown object me --> string mouseDownResult|<void>
QTVRMouseOver object me --> string mouseDownResultOrZero|<void>
  -- mouseDownResult is a two-token string:
  -- "pan ,0"             if panning or zooming
  -- "jump,<newNodeID>"   if the user moved between nodes
  -- "navg,<hotSpotID>"   if the user selected a 'navg' hot spot
  -- "stil,<hotSpotID>"   if the user selected a 'stil' hot spot
  -- "misc,<hotSpotID>"   if the user selected a 'misc' hot spot
  -- "<type>,<hotSpotID>" if the user selected a custom hot spot
  -- "undf,<hotSpotID>"   if the user selected an undefined hot spot
  -- QTVRMouseOver returns "0" if the user did not mouse down, if the cursor
  -- moved outside the panoramic movie, or if QTVRExitMouseOver was called.
QTVRGetPanAngle object me --> string floatStr|<void>
QTVRSetPanAngle object me, string floatOrIntStr
QTVRGetTiltAngle object me --> string floatStr|<void>
QTVRSetTiltAngle object me, string floatOrIntStr
-----------------------------------
-- Methods for QTVR Panoramas    --
-----------------------------------
QTVRGetFOV object me --> string floatStr|<void>
QTVRSetFOV object me, string floatOrIntStr
QTVRGetClickLoc object me --> string pointStr|<void>
QTVRSetClickLoc object me, string pointStr
  -- clickLoc is the click location in window coordinates
QTVRGetClickPanAngles object me --> string anglePair|<void>
  -- returns "<pan>,<tilt>" angle pair
QTVRGetClickPanLoc object me --> string pointStr|<void>
  -- clickPanLoc is the click location in panoramic image coordinates
QTVRGetHotSpotID object me --> integer hotSpotID|<void> -- (0-255)
QTVRSetHotSpotID object me, integer hotSpotID -- (1-255 sets, 0 resets hotSpotID)
QTVRGetHotSpotName object me --> string hotSpotName|<void>
QTVRGetHotSpotType object me --> string hotSpotType|<void>
QTVRGetHotSpotViewAngles object me --> string angleTriple|<void>
  -- returns "<pan>,<tilt>,<FOV>" angle triple
QTVRGetObjectViewAngles object me --> string anglePairOrEmpty|<void>
  -- returns "<pan>,<tilt>" angle pair; empty if no hot spot of type 'navg' selected
QTVRGetObjectZoomRect object me --> string rectStr|<void>
QTVRGetNodeID object me --> integer nodeID|<void>
QTVRSetNodeID object me, integer nodeID
QTVRGetNodeName object me --> string nodeName|<void>
QTVRGetQuality object me --> string floatStr|<void>
QTVRSetQuality object me, string floatOrIntStr -- (0|1|2|4)
QTVRGetTransitionMode object me --> string transitionMode|<void>
QTVRSetTransitionMode object me, string transitionMode
  -- transitionMode is ("normal"|"swing")
QTVRGetTransitionSpeed object me --> string floatStr|<void>
QTVRSetTransitionSpeed object me, string floatOrIntStr
  -- minimum transition speed is 1.0, 4.0 is typical.
QTVRGetUpdateMode object me --> string updateMode|<void>
QTVRSetUpdateMode object me, string updateMode
  -- updateMode is ("normal"|"updateBoth"|"offscreenOnly"|"fromOffscreen"|"directToScreen")
  -- NOTE: QTVRGet/SetUpdateMode are functional only on MacOS.
QTVRGetVisible object me --> integer (non-zero if visible)
QTVRSetVisible object me, integer booleanInt -- false (0) or true (non-zero)
  -- NOTE: QTVRGet/SetVisible both work for QTVR Objects as well.
QTVRGetWarpMode object me --> string warpMode|<void>
QTVRSetWarpMode object me, string warpMode
  -- warpMode is ("2"|"1"|"0") for 2-d, 1-d or no warping;
  -- equivalent to full, partial and no correction in QTVRPlayer
QTVRCollapseToHotSpotRgn object me
  -- NOTE: QTVRCollapseToHotSpotRgn is functional only on MacOS.
-----------------------------------
-- Methods for QTVR Objects      --
-----------------------------------
QTVRZoomOutEffect object me, string startRectStr, string skipFirstFrameBoolean, string clipRectStr
  -- Rect strings are "<left>,<top>,<bottom>,<right>". Boolean string is ("true"|"false").
  -- NOTE: QTVRZoomOutEffect is functional only on MacOS.
QTVRGetColumn object me --> string columnStr
QTVRSetColumn object me, integer column -- (column >= 0)
QTVRGetRow object me --> string rowStr|<void>
QTVRSetRow object me, integer row -- (row >= 0)
QTVRNudge object me, string direction -- ("up"|"down"|"left"|"right")
-----------------------------------
-- Callbacks for QTVR Panoramas  --
-----------------------------------
QTVRGetMouseDownHandler object me --> string handlerName|<void>
QTVRSetMouseDownHandler object me, string handlerName
QTVRGetMouseOverHandler object me --> string handlerName|<void>
QTVRSetMouseOverHandler object me, string handlerName
QTVRGetMouseStillDownHandler object me --> string handlerName|<void>
QTVRSetMouseStillDownHandler object me, string handlerName
QTVRGetNodeLeaveHandler object me --> string handlerName|<void>
QTVRSetNodeLeaveHandler object me, string handlerName
QTVRGetPanZoomStartHandler object me --> string handlerName|<void>
QTVRSetPanZoomStartHandler object me, string handlerName
QTVRGetRolloverHotSpotHandler object me --> string handlerName|<void>
QTVRSetRolloverHotSpotHandler object me, string handlerName
QTVRExitMouseOver object me -- returns control to Director from a QTVRMouseOver handler
QTVRPassMouseDown object me -- passes a MouseDown event to the QTVR controller
-----------------------------------
-- Testing for a QTVR movie      --
-----------------------------------
IsQTVRMovie object me --> integer (non-zero if the movie is a valid, open QTVR movie)

 */

namespace Director {
const char *QtvrxtraXtra::xlibName = "QTVRXtra";
const XlibFileDesc QtvrxtraXtra::fileNames[] = {
	{ "qtvrxtra",	nullptr },
	{ "Qtvrw32",	nullptr },
	{ nullptr,		nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				QtvrxtraXtra::m_new,		 0, 0,	500 },
	{ "forget",				QtvrxtraXtra::m_forget,		 0, 0,	500 },
	{ "QTVREnter",			QtvrxtraXtra::m_QTVREnter,	 1, 1,	500 },
	{ "QTVROpen",				QtvrxtraXtra::m_QTVROpen,		 3, 0,	500 },
	{ "QTVRClose",				QtvrxtraXtra::m_QTVRClose,		 0, 0,	500 },
	{ "QTVRUpdate",				QtvrxtraXtra::m_QTVRUpdate,		 0, 0,	500 },
	{ "QTVRGetQTVRType",				QtvrxtraXtra::m_QTVRGetQTVRType,		 0, 0,	500 },
	{ "QTVRIdle",				QtvrxtraXtra::m_QTVRIdle,		 0, 0,	500 },
	{ "QTVRMouseDown",				QtvrxtraXtra::m_QTVRMouseDown,		 0, 0,	500 },
	{ "QTVRMouseOver",				QtvrxtraXtra::m_QTVRMouseOver,		 0, 0,	500 },
	{ "QTVRGetPanAngle",				QtvrxtraXtra::m_QTVRGetPanAngle,		 0, 0,	500 },
	{ "QTVRSetPanAngle",				QtvrxtraXtra::m_QTVRSetPanAngle,		 1, 0,	500 },
	{ "QTVRGetTiltAngle",				QtvrxtraXtra::m_QTVRGetTiltAngle,		 0, 0,	500 },
	{ "QTVRSetTiltAngle",				QtvrxtraXtra::m_QTVRSetTiltAngle,		 1, 0,	500 },
	{ "QTVRGetFOV",				QtvrxtraXtra::m_QTVRGetFOV,		 0, 0,	500 },
	{ "QTVRSetFOV",				QtvrxtraXtra::m_QTVRSetFOV,		 1, 0,	500 },
	{ "QTVRGetClickLoc",				QtvrxtraXtra::m_QTVRGetClickLoc,		 0, 0,	500 },
	{ "QTVRSetClickLoc",				QtvrxtraXtra::m_QTVRSetClickLoc,		 1, 0,	500 },
	{ "QTVRGetClickPanAngles",				QtvrxtraXtra::m_QTVRGetClickPanAngles,		 0, 0,	500 },
	{ "QTVRGetClickPanLoc",				QtvrxtraXtra::m_QTVRGetClickPanLoc,		 0, 0,	500 },
	{ "QTVRGetHotSpotID",				QtvrxtraXtra::m_QTVRGetHotSpotID,		 0, 0,	500 },
	{ "QTVRSetHotSpotID",				QtvrxtraXtra::m_QTVRSetHotSpotID,		 1, 0,	500 },
	{ "QTVRGetHotSpotName",				QtvrxtraXtra::m_QTVRGetHotSpotName,		 0, 0,	500 },
	{ "QTVRGetHotSpotType",				QtvrxtraXtra::m_QTVRGetHotSpotType,		 0, 0,	500 },
	{ "QTVRGetHotSpotViewAngles",				QtvrxtraXtra::m_QTVRGetHotSpotViewAngles,		 0, 0,	500 },
	{ "QTVRGetObjectViewAngles",				QtvrxtraXtra::m_QTVRGetObjectViewAngles,		 0, 0,	500 },
	{ "QTVRGetObjectZoomRect",				QtvrxtraXtra::m_QTVRGetObjectZoomRect,		 0, 0,	500 },
	{ "QTVRGetNodeID",				QtvrxtraXtra::m_QTVRGetNodeID,		 0, 0,	500 },
	{ "QTVRSetNodeID",				QtvrxtraXtra::m_QTVRSetNodeID,		 1, 0,	500 },
	{ "QTVRGetNodeName",				QtvrxtraXtra::m_QTVRGetNodeName,		 0, 0,	500 },
	{ "QTVRGetQuality",				QtvrxtraXtra::m_QTVRGetQuality,		 0, 0,	500 },
	{ "QTVRSetQuality",				QtvrxtraXtra::m_QTVRSetQuality,		 1, 0,	500 },
	{ "QTVRGetTransitionMode",				QtvrxtraXtra::m_QTVRGetTransitionMode,		 0, 0,	500 },
	{ "QTVRSetTransitionMode",				QtvrxtraXtra::m_QTVRSetTransitionMode,		 1, 0,	500 },
	{ "QTVRGetTransitionSpeed",				QtvrxtraXtra::m_QTVRGetTransitionSpeed,		 0, 0,	500 },
	{ "QTVRSetTransitionSpeed",				QtvrxtraXtra::m_QTVRSetTransitionSpeed,		 1, 0,	500 },
	{ "QTVRGetUpdateMode",				QtvrxtraXtra::m_QTVRGetUpdateMode,		 0, 0,	500 },
	{ "QTVRSetUpdateMode",				QtvrxtraXtra::m_QTVRSetUpdateMode,		 1, 0,	500 },
	{ "QTVRGetVisible",				QtvrxtraXtra::m_QTVRGetVisible,		 0, 0,	500 },
	{ "QTVRSetVisible",				QtvrxtraXtra::m_QTVRSetVisible,		 1, 0,	500 },
	{ "QTVRGetWarpMode",				QtvrxtraXtra::m_QTVRGetWarpMode,		 0, 0,	500 },
	{ "QTVRSetWarpMode",				QtvrxtraXtra::m_QTVRSetWarpMode,		 1, 0,	500 },
	{ "QTVRCollapseToHotSpotRgn",				QtvrxtraXtra::m_QTVRCollapseToHotSpotRgn,		 0, 0,	500 },
	{ "QTVRZoomOutEffect",				QtvrxtraXtra::m_QTVRZoomOutEffect,		 3, 0,	500 },
	{ "QTVRGetColumn",				QtvrxtraXtra::m_QTVRGetColumn,		 0, 0,	500 },
	{ "QTVRSetColumn",				QtvrxtraXtra::m_QTVRSetColumn,		 1, 0,	500 },
	{ "QTVRGetRow",				QtvrxtraXtra::m_QTVRGetRow,		 0, 0,	500 },
	{ "QTVRSetRow",				QtvrxtraXtra::m_QTVRSetRow,		 1, 0,	500 },
	{ "QTVRNudge",				QtvrxtraXtra::m_QTVRNudge,		 1, 0,	500 },
	{ "QTVRGetMouseDownHandler",				QtvrxtraXtra::m_QTVRGetMouseDownHandler,		 0, 0,	500 },
	{ "QTVRSetMouseDownHandler",				QtvrxtraXtra::m_QTVRSetMouseDownHandler,		 1, 0,	500 },
	{ "QTVRGetMouseOverHandler",				QtvrxtraXtra::m_QTVRGetMouseOverHandler,		 0, 0,	500 },
	{ "QTVRSetMouseOverHandler",				QtvrxtraXtra::m_QTVRSetMouseOverHandler,		 1, 0,	500 },
	{ "QTVRGetMouseStillDownHandler",				QtvrxtraXtra::m_QTVRGetMouseStillDownHandler,		 0, 0,	500 },
	{ "QTVRSetMouseStillDownHandler",				QtvrxtraXtra::m_QTVRSetMouseStillDownHandler,		 1, 0,	500 },
	{ "QTVRGetNodeLeaveHandler",				QtvrxtraXtra::m_QTVRGetNodeLeaveHandler,		 0, 0,	500 },
	{ "QTVRSetNodeLeaveHandler",				QtvrxtraXtra::m_QTVRSetNodeLeaveHandler,		 1, 0,	500 },
	{ "QTVRGetPanZoomStartHandler",				QtvrxtraXtra::m_QTVRGetPanZoomStartHandler,		 0, 0,	500 },
	{ "QTVRSetPanZoomStartHandler",				QtvrxtraXtra::m_QTVRSetPanZoomStartHandler,		 1, 0,	500 },
	{ "QTVRGetRolloverHotSpotHandler",				QtvrxtraXtra::m_QTVRGetRolloverHotSpotHandler,		 0, 0,	500 },
	{ "QTVRSetRolloverHotSpotHandler",				QtvrxtraXtra::m_QTVRSetRolloverHotSpotHandler,		 1, 0,	500 },
	{ "QTVRExitMouseOver",				QtvrxtraXtra::m_QTVRExitMouseOver,		 0, 0,	500 },
	{ "QTVRPassMouseDown",				QtvrxtraXtra::m_QTVRPassMouseDown,		 0, 0,	500 },
	{ "IsQTVRMovie",				QtvrxtraXtra::m_IsQTVRMovie,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

QtvrxtraXtraObject::QtvrxtraXtraObject(ObjectType ObjectType) :Object<QtvrxtraXtraObject>("Qtvrxtra") {
	_objType = ObjectType;
}

bool QtvrxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum QtvrxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(QtvrxtraXtra::xlibName);
	warning("QtvrxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void QtvrxtraXtra::open(ObjectType type, const Common::Path &path) {
    QtvrxtraXtraObject::initMethods(xlibMethods);
    QtvrxtraXtraObject *xobj = new QtvrxtraXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void QtvrxtraXtra::close(ObjectType type) {
	QtvrxtraXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void QtvrxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("QtvrxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(QtvrxtraXtra::m_forget, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVREnter, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRExit, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVROpen, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRClose, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRUpdate, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetQTVRType, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRIdle, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRMouseDown, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRMouseOver, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetPanAngle, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetPanAngle, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetTiltAngle, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetTiltAngle, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetFOV, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetFOV, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetClickLoc, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetClickLoc, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetClickPanAngles, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetClickPanLoc, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetHotSpotID, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetHotSpotID, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetHotSpotName, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetHotSpotType, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetHotSpotViewAngles, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetObjectViewAngles, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetObjectZoomRect, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetNodeID, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetNodeID, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetNodeName, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetQuality, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetQuality, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetTransitionMode, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetTransitionMode, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetTransitionSpeed, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetTransitionSpeed, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetUpdateMode, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetUpdateMode, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetVisible, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetVisible, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetWarpMode, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetWarpMode, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRCollapseToHotSpotRgn, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRZoomOutEffect, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetColumn, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetColumn, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetRow, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetRow, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRNudge, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetMouseDownHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetMouseDownHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetMouseOverHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetMouseOverHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetMouseStillDownHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetMouseStillDownHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetNodeLeaveHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetNodeLeaveHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetPanZoomStartHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetPanZoomStartHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRGetRolloverHotSpotHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRSetRolloverHotSpotHandler, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRExitMouseOver, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRPassMouseDown, 0)
XOBJSTUB(QtvrxtraXtra::m_IsQTVRMovie, 0)

}
