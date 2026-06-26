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
#include "director/lingo/xtras/s/scrnxtra.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 2 (D6), Löwenzahn 8 (D8.5), Löwenzahn Adventskalender (D8.5),
 * Löwenzahn 7 (D8), Löwenzahn Spielebox (D9)
 *
 **************************************************/

/*
-- xtra ScrnXtra
-- ScrnXtra v2.0.4 of 17-Feb-98 by Kent Kersten
-- Copyright (c) 1996,97,98 Little Planet Publishing.  All rights reserved.
-- For updates see http://www.littleplanet.com/kent/kent.html
-- Contact the author at kent@littleplanet.com
-- This no-charge Xtra may be freely distributed as long as it is
-- accompanied by its documentation and sample movie.
--
-- Stage Capture Functions --
* StageToMember int left, int top, int right, int bottom, any memberNumOrName, *
* StageResizeToMember int left, int top, int right, int bottom, int newWidth, int newHeight, any memberNumOrName, *
* StageToClipboard int left, int top, int right, int bottom
* StageResizeToClipboard int left, int top, int right, int bottom, int newWidth, int newHeight
* StageToFile int left, int top, int right, int bottom, string filename
* StageResizeToFile int left, int top, int right, int bottom, int newWidth, int newHeight, string filename
--
-- Screen Capture Functions --
* ScreenToMember int left, int top, int right, int bottom, any memberNumOrName, *
* ScreenResizeToMember int left, int top, int right, int bottom, int newWidth, int newHeight, any memberNumOrName, *
* ScreenToClipboard int left, int top, int right, int bottom
* ScreenResizeToClipboard int left, int top, int right, int bottom, int newWidth, int newHeight
* ScreenToFile int left, int top, int right, int bottom, string filename
* ScreenResizeToFile int left, int top, int right, int bottom, int newWidth, int newHeight, string filename
 */

namespace Director {

const char *ScrnXtraXtra::xlibName = "ScrnXtra";
const XlibFileDesc ScrnXtraXtra::fileNames[] = {
	{ "scrnxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ScrnXtraXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "StageToMember", ScrnXtraXtra::m_StageToMember, -1, 0, 500, HBLTIN },
	{ "StageResizeToMember", ScrnXtraXtra::m_StageResizeToMember, -1, 0, 500, HBLTIN },
	{ "StageToClipboard", ScrnXtraXtra::m_StageToClipboard, 4, 4, 500, HBLTIN },
	{ "StageResizeToClipboard", ScrnXtraXtra::m_StageResizeToClipboard, 6, 6, 500, HBLTIN },
	{ "StageToFile", ScrnXtraXtra::m_StageToFile, 5, 5, 500, HBLTIN },
	{ "StageResizeToFile", ScrnXtraXtra::m_StageResizeToFile, 7, 7, 500, HBLTIN },
	{ "ScreenToMember", ScrnXtraXtra::m_ScreenToMember, -1, 0, 500, HBLTIN },
	{ "ScreenResizeToMember", ScrnXtraXtra::m_ScreenResizeToMember, -1, 0, 500, HBLTIN },
	{ "ScreenToClipboard", ScrnXtraXtra::m_ScreenToClipboard, 4, 4, 500, HBLTIN },
	{ "ScreenResizeToClipboard", ScrnXtraXtra::m_ScreenResizeToClipboard, 6, 6, 500, HBLTIN },
	{ "ScreenToFile", ScrnXtraXtra::m_ScreenToFile, 5, 5, 500, HBLTIN },
	{ "ScreenResizeToFile", ScrnXtraXtra::m_ScreenResizeToFile, 7, 7, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ScrnXtraXtraObject::ScrnXtraXtraObject(ObjectType ObjectType) :Object<ScrnXtraXtraObject>("ScrnXtra") {
	_objType = ObjectType;
}

bool ScrnXtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ScrnXtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ScrnXtraXtra::xlibName);
	warning("ScrnXtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ScrnXtraXtra::open(ObjectType type, const Common::Path &path) {
    ScrnXtraXtraObject::initMethods(xlibMethods);
    ScrnXtraXtraObject *xobj = new ScrnXtraXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ScrnXtraXtra::close(ObjectType type) {
    ScrnXtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ScrnXtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ScrnXtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ScrnXtraXtra::m_StageToMember, 0)
XOBJSTUB(ScrnXtraXtra::m_StageResizeToMember, 0)
XOBJSTUB(ScrnXtraXtra::m_StageToClipboard, 0)
XOBJSTUB(ScrnXtraXtra::m_StageResizeToClipboard, 0)
XOBJSTUB(ScrnXtraXtra::m_StageToFile, 0)
XOBJSTUB(ScrnXtraXtra::m_StageResizeToFile, 0)
XOBJSTUB(ScrnXtraXtra::m_ScreenToMember, 0)
XOBJSTUB(ScrnXtraXtra::m_ScreenResizeToMember, 0)
XOBJSTUB(ScrnXtraXtra::m_ScreenToClipboard, 0)
XOBJSTUB(ScrnXtraXtra::m_ScreenResizeToClipboard, 0)
XOBJSTUB(ScrnXtraXtra::m_ScreenToFile, 0)
XOBJSTUB(ScrnXtraXtra::m_ScreenResizeToFile, 0)

}
