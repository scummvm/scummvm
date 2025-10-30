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
#include "director/lingo/xtras/s/smacker.h"

/**************************************************
 *
 * USED IN:
 * I Spy Spooky House
 *
 **************************************************/

/*
-- xtra Smacker
-- An Xtra for playing Smacker animations.
--  Version 3.1q
------------------------------------------
new object
*SmackQuickPlay string, integer, integer, string
*SmackQuickPlayTrans string, integer, integer, integer, integer, integer
SmackOpen object, string, integer, integer
SmackOpenTrans object, string, integer, integer, integer, integer, integer
SmackClose object
SmackPlay object
SmackPlayFrames object, integer, integer
SmackPlayLooped object, integer
SmackPlayNext object, integer
SmackRemapToSystemPalette object
SmackRemapToPalette object, string, string
SmackRemapToBitmap object, string, string
SmackSetWindowStyle object, string
SmackSetWindowTitle object, string
SmackSetTransBackground object, string, string, integer, integer, integer
SmackSetInterfaceKeys object, string
SmackSetBitmap object, string, string
SmackSetPosition object, integer, integer
SmackSetAlignment object, integer
SmackGoto object, integer
SmackGetFramesPerSecond object
SmackGetFrameNum object
SmackGetFrames object
SmackGetHeight object
SmackGetWidth object
SmackGetLastKey object
SmackGetMouseX object
SmackGetMouseY object
SmackGetMouseClickX object
SmackGetMouseClickY object
SmackHideVideo object, integer
SmackSetDisplayMode object, integer
SmackGetSummary object
SmackScreenMethod object, integer

 */

namespace Director {

const char *SmackerXtra::xlibName = "Smacker";
const XlibFileDesc SmackerXtra::fileNames[] = {
	{ "smacker",   nullptr },
	{ "Smackx32",   nullptr },
	{ "SmackerXtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				SmackerXtra::m_new,		 0, 0,	500 },
	{ "SmackOpen",				SmackerXtra::m_SmackOpen,		 3, 3,	500 },
	{ "SmackOpenTrans",				SmackerXtra::m_SmackOpenTrans,		 6, 6,	500 },
	{ "SmackClose",				SmackerXtra::m_SmackClose,		 0, 0,	500 },
	{ "SmackPlay",				SmackerXtra::m_SmackPlay,		 0, 0,	500 },
	{ "SmackPlayFrames",				SmackerXtra::m_SmackPlayFrames,		 2, 2,	500 },
	{ "SmackPlayLooped",				SmackerXtra::m_SmackPlayLooped,		 1, 1,	500 },
	{ "SmackPlayNext",				SmackerXtra::m_SmackPlayNext,		 1, 1,	500 },
	{ "SmackRemapToSystemPalette",				SmackerXtra::m_SmackRemapToSystemPalette,		 0, 0,	500 },
	{ "SmackRemapToPalette",				SmackerXtra::m_SmackRemapToPalette,		 2, 2,	500 },
	{ "SmackRemapToBitmap",				SmackerXtra::m_SmackRemapToBitmap,		 2, 2,	500 },
	{ "SmackSetWindowStyle",				SmackerXtra::m_SmackSetWindowStyle,		 1, 1,	500 },
	{ "SmackSetWindowTitle",				SmackerXtra::m_SmackSetWindowTitle,		 1, 1,	500 },
	{ "SmackSetTransBackground",				SmackerXtra::m_SmackSetTransBackground,		 5, 5,	500 },
	{ "SmackSetInterfaceKeys",				SmackerXtra::m_SmackSetInterfaceKeys,		 1, 1,	500 },
	{ "SmackSetBitmap",				SmackerXtra::m_SmackSetBitmap,		 2, 2,	500 },
	{ "SmackSetPosition",				SmackerXtra::m_SmackSetPosition,		 2, 2,	500 },
	{ "SmackSetAlignment",				SmackerXtra::m_SmackSetAlignment,		 1, 1,	500 },
	{ "SmackGoto",				SmackerXtra::m_SmackGoto,		 1, 1,	500 },
	{ "SmackGetFramesPerSecond",				SmackerXtra::m_SmackGetFramesPerSecond,		 0, 0,	500 },
	{ "SmackGetFrameNum",				SmackerXtra::m_SmackGetFrameNum,		 0, 0,	500 },
	{ "SmackGetFrames",				SmackerXtra::m_SmackGetFrames,		 0, 0,	500 },
	{ "SmackGetHeight",				SmackerXtra::m_SmackGetHeight,		 0, 0,	500 },
	{ "SmackGetWidth",				SmackerXtra::m_SmackGetWidth,		 0, 0,	500 },
	{ "SmackGetLastKey",				SmackerXtra::m_SmackGetLastKey,		 0, 0,	500 },
	{ "SmackGetMouseX",				SmackerXtra::m_SmackGetMouseX,		 0, 0,	500 },
	{ "SmackGetMouseY",				SmackerXtra::m_SmackGetMouseY,		 0, 0,	500 },
	{ "SmackGetMouseClickX",				SmackerXtra::m_SmackGetMouseClickX,		 0, 0,	500 },
	{ "SmackGetMouseClickY",				SmackerXtra::m_SmackGetMouseClickY,		 0, 0,	500 },
	{ "SmackHideVideo",				SmackerXtra::m_SmackHideVideo,		 1, 1,	500 },
	{ "SmackSetDisplayMode",				SmackerXtra::m_SmackSetDisplayMode,		 1, 1,	500 },
	{ "SmackGetSummary",				SmackerXtra::m_SmackGetSummary,		 0, 0,	500 },
	{ "SmackScreenMethod",				SmackerXtra::m_SmackScreenMethod,		 1, 1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "SmackQuickPlay", SmackerXtra::m_SmackQuickPlay, 4, 4, 500, HBLTIN },
	{ "SmackQuickPlayTrans", SmackerXtra::m_SmackQuickPlayTrans, 6, 6, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SmackerXtraObject::SmackerXtraObject(ObjectType ObjectType) :Object<SmackerXtraObject>("Smacker") {
	_objType = ObjectType;
}

bool SmackerXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SmackerXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SmackerXtra::xlibName);
	warning("SmackerXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SmackerXtra::open(ObjectType type, const Common::Path &path) {
    SmackerXtraObject::initMethods(xlibMethods);
    SmackerXtraObject *xobj = new SmackerXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SmackerXtra::close(ObjectType type) {
    SmackerXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SmackerXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SmackerXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(SmackerXtra::m_SmackQuickPlay, 0)
XOBJSTUB(SmackerXtra::m_SmackQuickPlayTrans, 0)
XOBJSTUB(SmackerXtra::m_SmackOpen, 0)
XOBJSTUB(SmackerXtra::m_SmackOpenTrans, 0)
XOBJSTUB(SmackerXtra::m_SmackClose, 0)
XOBJSTUB(SmackerXtra::m_SmackPlay, 0)
XOBJSTUB(SmackerXtra::m_SmackPlayFrames, 0)
XOBJSTUB(SmackerXtra::m_SmackPlayLooped, 0)
XOBJSTUB(SmackerXtra::m_SmackPlayNext, 0)
XOBJSTUB(SmackerXtra::m_SmackRemapToSystemPalette, 0)
XOBJSTUB(SmackerXtra::m_SmackRemapToPalette, 0)
XOBJSTUB(SmackerXtra::m_SmackRemapToBitmap, 0)
XOBJSTUB(SmackerXtra::m_SmackSetWindowStyle, 0)
XOBJSTUB(SmackerXtra::m_SmackSetWindowTitle, 0)
XOBJSTUB(SmackerXtra::m_SmackSetTransBackground, 0)
XOBJSTUB(SmackerXtra::m_SmackSetInterfaceKeys, 0)
XOBJSTUB(SmackerXtra::m_SmackSetBitmap, 0)
XOBJSTUB(SmackerXtra::m_SmackSetPosition, 0)
XOBJSTUB(SmackerXtra::m_SmackSetAlignment, 0)
XOBJSTUB(SmackerXtra::m_SmackGoto, 0)
XOBJSTUB(SmackerXtra::m_SmackGetFramesPerSecond, 0)
XOBJSTUB(SmackerXtra::m_SmackGetFrameNum, 0)
XOBJSTUB(SmackerXtra::m_SmackGetFrames, 0)
XOBJSTUB(SmackerXtra::m_SmackGetHeight, 0)
XOBJSTUB(SmackerXtra::m_SmackGetWidth, 0)
XOBJSTUB(SmackerXtra::m_SmackGetLastKey, 0)
XOBJSTUB(SmackerXtra::m_SmackGetMouseX, 0)
XOBJSTUB(SmackerXtra::m_SmackGetMouseY, 0)
XOBJSTUB(SmackerXtra::m_SmackGetMouseClickX, 0)
XOBJSTUB(SmackerXtra::m_SmackGetMouseClickY, 0)
XOBJSTUB(SmackerXtra::m_SmackHideVideo, 0)
XOBJSTUB(SmackerXtra::m_SmackSetDisplayMode, 0)
XOBJSTUB(SmackerXtra::m_SmackGetSummary, 0)
XOBJSTUB(SmackerXtra::m_SmackScreenMethod, 0)

}
