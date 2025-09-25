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
#include "common/tokenizer.h"

#include "common/formats/quicktime.h"

#include "video/qt_decoder.h"

#include "director/director.h"
#include "director/images.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
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
const char *const QtvrxtraXtra::xlibName = "QTVRXtra";
const XlibFileDesc QtvrxtraXtra::fileNames[] = {
	{ "qtvrxtra",	nullptr },
	{ "Qtvrw32",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",							QtvrxtraXtra::m_new,		 0, 0,	500 },
	{ "forget",							QtvrxtraXtra::m_forget,		 0, 0,	500 },
	{ "QTVREnter",						QtvrxtraXtra::m_QTVREnter,	 0, 0,	500 },
	{ "QTVRExit",						QtvrxtraXtra::m_QTVRExit,	 0, 0,	500 },
	{ "QTVROpen",						QtvrxtraXtra::m_QTVROpen,		 3, 3,	500 },
	{ "QTVRClose",						QtvrxtraXtra::m_QTVRClose,		 0, 0,	500 },
	{ "QTVRUpdate",						QtvrxtraXtra::m_QTVRUpdate,		 0, 0,	500 },
	{ "QTVRGetQTVRType",				QtvrxtraXtra::m_QTVRGetQTVRType,		 0, 0,	500 },
	{ "QTVRIdle",						QtvrxtraXtra::m_QTVRIdle,		 0, 0,	500 },
	{ "QTVRMouseDown",					QtvrxtraXtra::m_QTVRMouseDown,		 0, 0,	500 },
	{ "QTVRMouseOver",					QtvrxtraXtra::m_QTVRMouseOver,		 0, 0,	500 },
	{ "QTVRGetPanAngle",				QtvrxtraXtra::m_QTVRGetPanAngle,		 0, 0,	500 },
	{ "QTVRSetPanAngle",				QtvrxtraXtra::m_QTVRSetPanAngle,		 1, 1,	500 },
	{ "QTVRGetTiltAngle",				QtvrxtraXtra::m_QTVRGetTiltAngle,		 0, 0,	500 },
	{ "QTVRSetTiltAngle",				QtvrxtraXtra::m_QTVRSetTiltAngle,		 1, 1,	500 },
	{ "QTVRGetFOV",						QtvrxtraXtra::m_QTVRGetFOV,		 0, 0,	500 },
	{ "QTVRSetFOV",						QtvrxtraXtra::m_QTVRSetFOV,		 1, 1,	500 },
	{ "QTVRGetClickLoc",				QtvrxtraXtra::m_QTVRGetClickLoc,		 0, 0,	500 },
	{ "QTVRSetClickLoc",				QtvrxtraXtra::m_QTVRSetClickLoc,		 1, 0,	500 },
	{ "QTVRGetClickPanAngles",			QtvrxtraXtra::m_QTVRGetClickPanAngles,		 0, 0,	500 },
	{ "QTVRGetClickPanLoc",				QtvrxtraXtra::m_QTVRGetClickPanLoc,		 0, 0,	500 },
	{ "QTVRGetHotSpotID",				QtvrxtraXtra::m_QTVRGetHotSpotID,		 0, 0,	500 },
	{ "QTVRSetHotSpotID",				QtvrxtraXtra::m_QTVRSetHotSpotID,		 1, 0,	500 },
	{ "QTVRGetHotSpotName",				QtvrxtraXtra::m_QTVRGetHotSpotName,		 0, 0,	500 },
	{ "QTVRGetHotSpotType",				QtvrxtraXtra::m_QTVRGetHotSpotType,		 0, 0,	500 },
	{ "QTVRGetHotSpotViewAngles",		QtvrxtraXtra::m_QTVRGetHotSpotViewAngles,		 0, 0,	500 },
	{ "QTVRGetObjectViewAngles",		QtvrxtraXtra::m_QTVRGetObjectViewAngles,		 0, 0,	500 },
	{ "QTVRGetObjectZoomRect",			QtvrxtraXtra::m_QTVRGetObjectZoomRect,		 0, 0,	500 },
	{ "QTVRGetNodeID",					QtvrxtraXtra::m_QTVRGetNodeID,		 0, 0,	500 },
	{ "QTVRSetNodeID",					QtvrxtraXtra::m_QTVRSetNodeID,		 1, 1,	500 },
	{ "QTVRGetNodeName",				QtvrxtraXtra::m_QTVRGetNodeName,		 0, 0,	500 },
	{ "QTVRGetQuality",					QtvrxtraXtra::m_QTVRGetQuality,		 0, 0,	500 },
	{ "QTVRSetQuality",					QtvrxtraXtra::m_QTVRSetQuality,		 1, 1,	500 },
	{ "QTVRGetTransitionMode",			QtvrxtraXtra::m_QTVRGetTransitionMode,		 0, 0,	500 },
	{ "QTVRSetTransitionMode",			QtvrxtraXtra::m_QTVRSetTransitionMode,		 1, 1,	500 },
	{ "QTVRGetTransitionSpeed",			QtvrxtraXtra::m_QTVRGetTransitionSpeed,		 0, 0,	500 },
	{ "QTVRSetTransitionSpeed",			QtvrxtraXtra::m_QTVRSetTransitionSpeed,		 1, 1,	500 },
	{ "QTVRGetUpdateMode",				QtvrxtraXtra::m_QTVRGetUpdateMode,		 0, 0,	500 },
	{ "QTVRSetUpdateMode",				QtvrxtraXtra::m_QTVRSetUpdateMode,		 1, 1,	500 },
	{ "QTVRGetVisible",					QtvrxtraXtra::m_QTVRGetVisible,		 0, 0,	500 },
	{ "QTVRSetVisible",					QtvrxtraXtra::m_QTVRSetVisible,		 1, 1,	500 },
	{ "QTVRGetWarpMode",				QtvrxtraXtra::m_QTVRGetWarpMode,		 0, 0,	500 },
	{ "QTVRSetWarpMode",				QtvrxtraXtra::m_QTVRSetWarpMode,		 1, 0,	500 },
	{ "QTVRCollapseToHotSpotRgn",		QtvrxtraXtra::m_QTVRCollapseToHotSpotRgn,		 0, 0,	500 },
	{ "QTVRZoomOutEffect",				QtvrxtraXtra::m_QTVRZoomOutEffect,		 3, 0,	500 },
	{ "QTVRGetColumn",					QtvrxtraXtra::m_QTVRGetColumn,		 0, 0,	500 },
	{ "QTVRSetColumn",					QtvrxtraXtra::m_QTVRSetColumn,		 1, 1,	500 },
	{ "QTVRGetRow",						QtvrxtraXtra::m_QTVRGetRow,		 0, 0,	500 },
	{ "QTVRSetRow",						QtvrxtraXtra::m_QTVRSetRow,		 1, 1,	500 },
	{ "QTVRNudge",						QtvrxtraXtra::m_QTVRNudge,		 1, 1,	500 },
	{ "QTVRGetMouseDownHandler",		QtvrxtraXtra::m_QTVRGetMouseDownHandler,		 0, 0,	500 },
	{ "QTVRSetMouseDownHandler",		QtvrxtraXtra::m_QTVRSetMouseDownHandler,		 1, 0,	500 },
	{ "QTVRGetMouseOverHandler",		QtvrxtraXtra::m_QTVRGetMouseOverHandler,		 0, 0,	500 },
	{ "QTVRSetMouseOverHandler",		QtvrxtraXtra::m_QTVRSetMouseOverHandler,		 1, 0,	500 },
	{ "QTVRGetMouseStillDownHandler",	QtvrxtraXtra::m_QTVRGetMouseStillDownHandler,		 0, 0,	500 },
	{ "QTVRSetMouseStillDownHandler",	QtvrxtraXtra::m_QTVRSetMouseStillDownHandler,		 1, 0,	500 },
	{ "QTVRGetNodeLeaveHandler",		QtvrxtraXtra::m_QTVRGetNodeLeaveHandler,		 0, 0,	500 },
	{ "QTVRSetNodeLeaveHandler",		QtvrxtraXtra::m_QTVRSetNodeLeaveHandler,		 1, 0,	500 },
	{ "QTVRGetPanZoomStartHandler",		QtvrxtraXtra::m_QTVRGetPanZoomStartHandler,		 0, 0,	500 },
	{ "QTVRSetPanZoomStartHandler",		QtvrxtraXtra::m_QTVRSetPanZoomStartHandler,		 1, 0,	500 },
	{ "QTVRGetRolloverHotSpotHandler",	QtvrxtraXtra::m_QTVRGetRolloverHotSpotHandler,		 0, 0,	500 },
	{ "QTVRSetRolloverHotSpotHandler",	QtvrxtraXtra::m_QTVRSetRolloverHotSpotHandler,		 1, 0,	500 },
	{ "QTVRExitMouseOver",				QtvrxtraXtra::m_QTVRExitMouseOver,		 0, 0,	500 },
	{ "QTVRPassMouseDown",				QtvrxtraXtra::m_QTVRPassMouseDown,		 0, 0,	500 },
	{ "IsQTVRMovie",					QtvrxtraXtra::m_IsQTVRMovie,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

#define HANDLER_TICKS 500

QtvrxtraXtraObject::QtvrxtraXtraObject(ObjectType ObjectType) :Object<QtvrxtraXtraObject>("Qtvrxtra") {
	_objType = ObjectType;

	_video = nullptr;

	_visible = false;

	_passMouseDown = false;

	_widget = nullptr;
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
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void QtvrxtraXtra::close(ObjectType type) {
	QtvrxtraXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void QtvrxtraXtra::m_new(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_new", nargs);
	ARGNUMCHECK(0);

	g_lingo->push(g_lingo->_state->me);
}

void QtvrxtraXtra::m_forget(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_forget", nargs);
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	if (me->_video) {
		me->_video->close();
		delete me->_video;
	}
}

void QtvrxtraXtra::m_QTVREnter(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVREnter", nargs);
	ARGNUMCHECK(0);
	g_lingo->push(0);
}

void QtvrxtraXtra::m_QTVRExit(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRExit", nargs);
	ARGNUMCHECK(0);
}

static Common::Rect stringToRect(const Common::String &rectStr) {
	Common::StringTokenizer tokenizer(rectStr, Common::String(','));
	Common::StringArray tokens(tokenizer.split());

	if (tokens.size() != 4) {
		error("stringToRect(): The string should contain exactly 4 numbers separated by commas");
		return {};
	}

	Common::Rect rect;
	rect.left = atoi(tokens[0].c_str());
	rect.top = atoi(tokens[1].c_str());
	rect.right = atoi(tokens[2].c_str());
	rect.bottom = atoi(tokens[3].c_str());

	return rect;
}

static Common::Point stringToPoint(const Common::String &pointStr) {
	Common::StringTokenizer tokenizer(pointStr, Common::String(','));
	Common::StringArray tokens(tokenizer.split());

	if (tokens.size() != 2) {
		error("stringToPoint(): The string should contain exactly 2 numbers separated by commas");
		return {};
	}

	Common::Point point;
	point.x = atoi(tokens[0].c_str());
	point.y = atoi(tokens[1].c_str());

	return point;
}

void QtvrxtraXtra::m_QTVROpen(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVROpen", nargs);
	ARGNUMCHECK(3);

	Common::String visiblityStr = g_lingo->pop().asString();
	Common::String rectStr = g_lingo->pop().asString();
	Common::String pathStr = g_lingo->pop().asString();

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	if (visiblityStr.equalsIgnoreCase("visible")) {
		me->_visible = true;
	} else if (visiblityStr.equalsIgnoreCase("invisible")) {
		me->_visible = false;
	} else {
		Common::String error = Common::String::format("Error: Invalid visibility string: ('%s')!", visiblityStr.c_str());
		g_lingo->push(error);
		return;
	}

	me->_rect = stringToRect(rectStr);

	Common::Path path = findMoviePath(pathStr);
	if (path.empty()) {
		Common::String error = Common::String::format("Error: Movie file ('%s') not found!", pathStr.c_str());
		g_lingo->push(error);
		return;
	}

	me->_video = new Video::QuickTimeDecoder();
	debugC(5, kDebugXObj, "QtvrxtraXtra::m_QTVROpen(): Loading QT file ('%s')", path.toString().c_str());
	if (!me->_video->loadFile(path)) {
		Common::String error = Common::String::format("Error: Failed to load movie file ('%s')!", path.toString().c_str());
		g_lingo->push(error);
		return;
	}

	me->_video->setTargetSize(me->_rect.width(), me->_rect.height());
	me->_video->setOrigin(me->_rect.left, me->_rect.top);

	me->_widget = new QtvrxtraWidget(me, g_director->getCurrentWindow(),
			me->_rect.left, me->_rect.top, me->_rect.width(), me->_rect.height(),
			g_director->getMacWindowManager());

	g_lingo->push(Common::String());
}

void QtvrxtraXtra::m_QTVRClose(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	if (me->_video) {
		me->_video->close();
		delete me->_video;
		me->_video = nullptr;

		delete me->_widget;
	}
}

void QtvrxtraXtra::m_QTVRUpdate(int nargs) {
	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;
	me->_visible = true;

	m_QTVRIdle(0);
}


void QtvrxtraXtra::m_QTVRGetQTVRType(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	switch (me->_video->getQTVRType()) {
	case Common::QuickTimeParser::QTVRType::PANORAMA:
		g_lingo->push(Common::String("QTVRPanorama"));
		break;
	case Common::QuickTimeParser::QTVRType::OBJECT:
		g_lingo->push(Common::String("QTVRObject"));
		break;
	case Common::QuickTimeParser::QTVRType::OTHER:
	default:
		g_lingo->push(Common::String("NotAQTVRType"));
		break;
	}
}

void QtvrxtraXtra::m_QTVRIdle(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	if (!me->_visible)
		return;

	Graphics::Surface const *frame = me->_video->decodeNextFrame();

	if (!frame)
		return;

	Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, me->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

	g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
		dither->getPixels(), dither->pitch, me->_rect.left, me->_rect.top, dither->w, dither->h
	);

	dither->free();
	delete dither;
}

void QtvrxtraXtra::m_QTVRMouseDown(int nargs) {
	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;
	const Common::QuickTimeParser::PanoHotSpot *hotspot;

	Common::Event event;
	bool cont = true;

	if (nargs != -1337 && g_system->getEventManager()->pollEvent(event)) {
		if (event.type != Common::EVENT_LBUTTONDOWN)
			cont = false;
	}

	if (!cont) {
		if (me->_video->getQTVRType() == Common::QuickTimeParser::QTVRType::PANORAMA)
			g_lingo->push(Common::String("pan ,0"));
		else
			g_lingo->pushVoid();
		return;
	}

	uint32 nextTick = g_system->getMillis();
	int node;
	bool nodeChanged = false;

	while (true) {
		Graphics::Surface const *frame = me->_video->decodeNextFrame();

		Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, me->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

		g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
			dither->getPixels(), dither->pitch, me->_rect.left, me->_rect.top, dither->w, dither->h
		);

		dither->free();
		delete dither;

		g_director->getCurrentWindow()->setDirty(true);

		node = me->_video->getCurrentNodeID();

		while (g_system->getEventManager()->pollEvent(event)) {
			me->_widget->processEvent(event);

			if (event.type == Common::EVENT_LBUTTONUP)
				break;
		}

		if (me->_video->getCurrentNodeID() != node) {
			if (!me->_nodeLeaveHandler.empty()) {
				g_lingo->push(me->_video->getCurrentNodeID());
				g_lingo->executeHandler(me->_nodeLeaveHandler, 1);
			}

			nodeChanged = true;
		}

		if (g_system->getMillis() > nextTick) {
			nextTick = g_system->getMillis() + HANDLER_TICKS;

			if (!me->_mouseStillDownHandler.empty())
				g_lingo->executeHandler(me->_mouseStillDownHandler);
		}

		LB::b_updateStage(0);

		if (event.type == Common::EVENT_QUIT) {
			g_director->processEventQUIT();
			break;
		}

		if (event.type == Common::EVENT_LBUTTONUP)
			break;

		g_director->delayMillis(10);
	}

	hotspot = me->_video->getClickedHotspot();

	if (!hotspot) {
		if (nodeChanged)
			g_lingo->push(Common::String::format("jump,%d", node));
		else
			g_lingo->push(Common::String("pan ,0"));
		return;
	}

	g_lingo->push(Common::String::format("%s,%d", tag2str((uint32)hotspot->type), hotspot->id));
}

void QtvrxtraXtra::m_QTVRMouseOver(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;
	Common::Point pos = g_director->getCurrentWindow()->getMousePos();

	if (!me->_visible || !me->_rect.contains(pos)) {
		g_lingo->pushVoid();
		return;
	}

	// Execute handler on first call to MouseOver
	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getRolloverHotspot();

	if (!me->_rolloverHotSpotHandler.empty()) {
		g_lingo->push(hotspot ? hotspot->id : 0);
		g_lingo->executeHandler(me->_rolloverHotSpotHandler, 1);
	}

	uint32 nextTick = g_system->getMillis();

	while (true) {
		Graphics::Surface const *frame = me->_video->decodeNextFrame();

		if (!frame) {
			g_lingo->pushVoid();
			return;
		}

		Graphics::Surface *dither = frame->convertTo(g_director->_wm->_pixelformat, me->_video->getPalette(), 256, g_director->getPalette(), 256, Graphics::kDitherNaive);

		g_director->getCurrentWindow()->getSurface()->copyRectToSurface(
			dither->getPixels(), dither->pitch, me->_rect.left, me->_rect.top, dither->w, dither->h
		);

		dither->free();
		delete dither;

		g_director->getCurrentWindow()->setDirty(true);

		Common::Event event;

		while (g_system->getEventManager()->pollEvent(event)) {
			if (Common::isMouseEvent(event)) {
				pos = g_director->getCurrentWindow()->getMousePos();

				if (!me->_rect.contains(pos))
					break;
			}

			hotspot = me->_video->getRolloverHotspot();

			if (event.type == Common::EVENT_LBUTTONDOWN) {
				// MouseDownHandler is processed inside
				me->_widget->processEvent(event);

				if (!me->_passMouseDown) {
					g_lingo->push(0);
					return;
				}

				m_QTVRMouseDown(-1337);

				return; // MouseDown will take care of the return value on the stack
			}

			me->_widget->processEvent(event);

			if (!me->_rolloverHotSpotHandler.empty() && hotspot != me->_video->getRolloverHotspot()) {
				g_lingo->push(hotspot ? hotspot->id : 0);

				g_lingo->executeHandler(me->_rolloverHotSpotHandler, 1);

				if (me->_exitMouseOver)
					break;
			}
		}

		if (g_system->getMillis() > nextTick) {
			nextTick = g_system->getMillis() + HANDLER_TICKS;

			if (!me->_mouseOverHandler.empty())
				g_lingo->executeHandler(me->_mouseOverHandler);
		}

		if (me->_exitMouseOver)
			break;

		LB::b_updateStage(0);

		if (!me->_rect.contains(pos))
			break;

		if (event.type == Common::EVENT_QUIT) {
			g_director->processEventQUIT();
			break;
		}

		g_director->delayMillis(10);
	}

	if (me->_video->getQTVRType() == Common::QuickTimeParser::QTVRType::PANORAMA)
		g_lingo->push(0);
	else
		g_lingo->pushVoid();
}

void QtvrxtraXtra::m_QTVRGetPanAngle(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(Common::String::format("%f", me->_video->getPanAngle()));
}

void QtvrxtraXtra::m_QTVRSetPanAngle(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setPanAngle(atof(g_lingo->pop().asString().c_str()));
}

void QtvrxtraXtra::m_QTVRGetTiltAngle(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(Common::String::format("%f", me->_video->getTiltAngle()));
}

void QtvrxtraXtra::m_QTVRSetTiltAngle(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setTiltAngle(atof(g_lingo->pop().asString().c_str()));
}

void QtvrxtraXtra::m_QTVRGetFOV(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(Common::String::format("%f", me->_video->getFOV()));
}

void QtvrxtraXtra::m_QTVRSetFOV(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setFOV(atof(g_lingo->pop().asString().c_str()));
}

void QtvrxtraXtra::m_QTVRGetClickLoc(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	Common::Point pos = me->_video->getLastClick();

	g_lingo->push(Common::String::format("%d,%d", pos.x, pos.y));
}

void QtvrxtraXtra::m_QTVRSetClickLoc(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	Common::Point pos = stringToPoint(g_lingo->pop().asString());

	me->_video->handleMouseButton(true, pos.x, pos.y);
}

void QtvrxtraXtra::m_QTVRGetClickPanAngles(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	Common::Point pos = me->_video->getLastClick();
	Graphics::FloatPoint loc = me->_video->getPanAngles(pos.x, pos.y);

	g_lingo->push(Common::String::format("%.4f,%.4f", loc.x, loc.y));
}

void QtvrxtraXtra::m_QTVRGetClickPanLoc(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	Common::Point pos = me->_video->getLastClick();
	Common::Point loc = me->_video->getPanLoc(pos.x, pos.y);

	g_lingo->push(Common::String::format("%d,%d", loc.x, loc.y));
}

void QtvrxtraXtra::m_QTVRGetHotSpotID(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	g_lingo->push(hotspot ? hotspot->id : 0);
}

void QtvrxtraXtra::m_QTVRSetHotSpotID(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setClickedHotSpot(g_lingo->pop().asInt());
}

void QtvrxtraXtra::m_QTVRGetHotSpotName(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	if (!hotspot) {
		g_lingo->push(Common::String(""));
		return;
	}

	g_lingo->push(me->_video->getHotSpotName(hotspot->id));
}

void QtvrxtraXtra::m_QTVRGetHotSpotType(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	g_lingo->push(hotspot ? Common::tag2string((uint32)hotspot->type) : "undf");
}

void QtvrxtraXtra::m_QTVRGetHotSpotViewAngles(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	if (hotspot)
		g_lingo->push(Common::String::format("%.4f,%.4f,%.4f", hotspot->viewHPan, hotspot->viewVPan, hotspot->viewZoom));
	else
		g_lingo->push(Common::String(""));
}

void QtvrxtraXtra::m_QTVRGetObjectViewAngles(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	if (hotspot) {
		const Common::QuickTimeParser::PanoNavigation *navg = me->_video->getHotSpotNavByID(hotspot->id);

		if (navg) {
			g_lingo->push(Common::String::format("%.4f,%.4f", navg->navgHPan, navg->navgVPan));
			return;
		}
	}

	g_lingo->pushVoid();
}

void QtvrxtraXtra::m_QTVRGetObjectZoomRect(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	const Common::QuickTimeParser::PanoHotSpot *hotspot = me->_video->getClickedHotspot();

	if (hotspot) {
		const Common::QuickTimeParser::PanoNavigation *navg = me->_video->getHotSpotNavByID(hotspot->id);

		if (navg) {
			g_lingo->push(Common::String::format("%d,%d,%d,%d", navg->zoomRect.left, navg->zoomRect.top, navg->zoomRect.right, navg->zoomRect.bottom));
			return;
		}
	}

	g_lingo->pushVoid();
}

void QtvrxtraXtra::m_QTVRGetNodeID(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push((int)me->_video->getCurrentNodeID());
}

void QtvrxtraXtra::m_QTVRSetNodeID(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->goToNode(g_lingo->pop().asInt());
}

void QtvrxtraXtra::m_QTVRGetNodeName(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getCurrentNodeName());
}

void QtvrxtraXtra::m_QTVRGetQuality(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getQuality());
}

void QtvrxtraXtra::m_QTVRSetQuality(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setQuality(g_lingo->pop().asInt());
}

void QtvrxtraXtra::m_QTVRGetTransitionMode(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getTransitionMode());
}

void QtvrxtraXtra::m_QTVRSetTransitionMode(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setTransitionMode(g_lingo->pop().asString());
}

void QtvrxtraXtra::m_QTVRGetTransitionSpeed(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getTransitionSpeed());
}

void QtvrxtraXtra::m_QTVRSetTransitionSpeed(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setTransitionSpeed(g_lingo->pop().asFloat());
}

void QtvrxtraXtra::m_QTVRGetUpdateMode(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getUpdateMode());
}

void QtvrxtraXtra::m_QTVRSetUpdateMode(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setUpdateMode(g_lingo->pop().asString());
}

void QtvrxtraXtra::m_QTVRGetVisible(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push((int)me->_visible);
}

void QtvrxtraXtra::m_QTVRSetVisible(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_visible = (bool)g_lingo->pop().asInt();

	if (!me->_visible)
		g_director->getCurrentWindow()->render(true);
}

void QtvrxtraXtra::m_QTVRGetWarpMode(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_video->getWarpMode());
}

void QtvrxtraXtra::m_QTVRSetWarpMode(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setWarpMode(g_lingo->pop().asInt());
}

XOBJSTUB(QtvrxtraXtra::m_QTVRCollapseToHotSpotRgn, 0)
XOBJSTUB(QtvrxtraXtra::m_QTVRZoomOutEffect, 0)

void QtvrxtraXtra::m_QTVRGetColumn(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRGetColumn", nargs);
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(Common::String::format("%d", me->_video->getCurrentColumn()));
}

void QtvrxtraXtra::m_QTVRSetColumn(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRSetColumn", nargs);
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setCurrentColumn(atoi(g_lingo->pop().asString().c_str()));
}

void QtvrxtraXtra::m_QTVRGetRow(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRGetRow", nargs);
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(Common::String::format("%d", me->_video->getCurrentRow()));
}

void QtvrxtraXtra::m_QTVRSetRow(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRSetRow", nargs);
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_video->setCurrentRow(atoi(g_lingo->pop().asString().c_str()));
}

void QtvrxtraXtra::m_QTVRNudge(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRNudge", nargs);
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	Common::String direction = g_lingo->pop().asString();

	if (!(direction.equalsIgnoreCase("left") || direction.equalsIgnoreCase("right") ||
		  direction.equalsIgnoreCase("up") || direction.equalsIgnoreCase("down"))) {
		error("QtvrxtraXtra::m_QTVRNudge(): Invald direction: ('%s')!", direction.c_str());
		return;
	}

	me->_video->nudge(direction);
}

void QtvrxtraXtra::m_QTVRGetMouseDownHandler(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRGetMouseDownHandler", nargs);
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_mouseDownHandler);
}

void QtvrxtraXtra::m_QTVRSetMouseDownHandler(int nargs) {
	g_lingo->printArgs("QtvrxtraXtra::m_QTVRSetMouseDownHandler", nargs);
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_mouseDownHandler = g_lingo->pop().asString();
}

void QtvrxtraXtra::m_QTVRGetMouseOverHandler(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_mouseOverHandler);
}

void QtvrxtraXtra::m_QTVRSetMouseOverHandler(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_mouseOverHandler = g_lingo->pop().asString();
}

void QtvrxtraXtra::m_QTVRGetMouseStillDownHandler(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_mouseStillDownHandler);
}

void QtvrxtraXtra::m_QTVRSetMouseStillDownHandler(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_mouseStillDownHandler = g_lingo->pop().asString();
}

void QtvrxtraXtra::m_QTVRGetNodeLeaveHandler(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_nodeLeaveHandler);
}

void QtvrxtraXtra::m_QTVRSetNodeLeaveHandler(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_nodeLeaveHandler = g_lingo->pop().asString();
}

void QtvrxtraXtra::m_QTVRGetPanZoomStartHandler(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_panZoomStartHandler);
}

void QtvrxtraXtra::m_QTVRSetPanZoomStartHandler(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_panZoomStartHandler = g_lingo->pop().asString();
}

void QtvrxtraXtra::m_QTVRGetRolloverHotSpotHandler(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push(me->_rolloverHotSpotHandler);
}

void QtvrxtraXtra::m_QTVRSetRolloverHotSpotHandler(int nargs) {
	ARGNUMCHECK(1);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_rolloverHotSpotHandler = g_lingo->pop().asString();
}

void QtvrxtraXtra::m_QTVRExitMouseOver(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_exitMouseOver = true;
}

void QtvrxtraXtra::m_QTVRPassMouseDown(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	me->_passMouseDown = true;
}

void QtvrxtraXtra::m_IsQTVRMovie(int nargs) {
	ARGNUMCHECK(0);

	QtvrxtraXtraObject *me = (QtvrxtraXtraObject *)g_lingo->_state->me.u.obj;

	g_lingo->push((int)(me->_video && me->_video->isVideoLoaded() && me->_video->isVR()));
}

///////////////
// Widget
///////////////

QtvrxtraWidget::QtvrxtraWidget(QtvrxtraXtraObject *xtra, Graphics::MacWidget *parent, int x, int y, int w, int h, Graphics::MacWindowManager *wm) :
	Graphics::MacWidget(parent, x, y, w, h, wm, true), _xtra(xtra) {

	_priority = 10000; // We stay on top of everything
}

bool QtvrxtraWidget::processEvent(Common::Event &event) {
	if (!_xtra->_visible)
		return false;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		if (_xtra->_mouseDownHandler.empty()) {
			_xtra->_passMouseDown = true;
			_xtra->_video->handleMouseButton(true, event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		} else {
			_xtra->_passMouseDown = false;

			g_lingo->executeHandler(_xtra->_mouseDownHandler);

			if (_xtra->_passMouseDown)
				_xtra->_video->handleMouseButton(true, event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		}
		return true;
	case Common::EVENT_LBUTTONUP:
		_xtra->_video->handleMouseButton(false, event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		return true;
	case Common::EVENT_MOUSEMOVE:
		_xtra->_video->handleMouseMove(event.mouse.x - _xtra->_rect.left, event.mouse.y - _xtra->_rect.top);
		return true;
	case Common::EVENT_QUIT:
		_xtra->_video->handleQuit();
		return false;
	case Common::EVENT_KEYDOWN:
	case Common::EVENT_KEYUP: {
		int zoomState = _xtra->_video->getZoomState();
		_xtra->_video->handleKey(event.kbd, event.type == Common::EVENT_KEYDOWN);

		int newZoomState = _xtra->_video->getZoomState();

		if (zoomState == Video::QuickTimeDecoder::kZoomNone &&
			(newZoomState == Video::QuickTimeDecoder::kZoomIn || newZoomState == Video::QuickTimeDecoder::kZoomOut)) {

			if (!_xtra->_panZoomStartHandler.empty())
				g_lingo->executeHandler(_xtra->_panZoomStartHandler);
		}

		}

		return true;
	default:
		return false;
	}
}

}
