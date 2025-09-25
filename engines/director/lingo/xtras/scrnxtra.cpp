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
#include "director/lingo/xtras/scrnxtra.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra ScrnXtra
-- ScrnXtra v2.0.2 of 3-Apr-97 by Kent Kersten
-- Copyright (c) 1996-1997 Little Planet Publishing.  All rights reserved.
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

const char *ScrnxtraXtra::xlibName = "Scrnxtra";
const XlibFileDesc ScrnxtraXtra::fileNames[] = {
	{ "scrnxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "StageToMember", ScrnxtraXtra::m_StageToMember, -1, 0, 500, HBLTIN },
	{ "StageResizeToMember", ScrnxtraXtra::m_StageResizeToMember, -1, 0, 500, HBLTIN },
	{ "StageToClipboard", ScrnxtraXtra::m_StageToClipboard, 4, 4, 500, HBLTIN },
	{ "StageResizeToClipboard", ScrnxtraXtra::m_StageResizeToClipboard, 6, 6, 500, HBLTIN },
	{ "StageToFile", ScrnxtraXtra::m_StageToFile, 5, 5, 500, HBLTIN },
	{ "StageResizeToFile", ScrnxtraXtra::m_StageResizeToFile, 7, 7, 500, HBLTIN },
	{ "ScreenToMember", ScrnxtraXtra::m_ScreenToMember, -1, 0, 500, HBLTIN },
	{ "ScreenResizeToMember", ScrnxtraXtra::m_ScreenResizeToMember, -1, 0, 500, HBLTIN },
	{ "ScreenToClipboard", ScrnxtraXtra::m_ScreenToClipboard, 4, 4, 500, HBLTIN },
	{ "ScreenResizeToClipboard", ScrnxtraXtra::m_ScreenResizeToClipboard, 6, 6, 500, HBLTIN },
	{ "ScreenToFile", ScrnxtraXtra::m_ScreenToFile, 5, 5, 500, HBLTIN },
	{ "ScreenResizeToFile", ScrnxtraXtra::m_ScreenResizeToFile, 7, 7, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ScrnxtraXtraObject::ScrnxtraXtraObject(ObjectType ObjectType) :Object<ScrnxtraXtraObject>("Scrnxtra") {
	_objType = ObjectType;
}

bool ScrnxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ScrnxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ScrnxtraXtra::xlibName);
	warning("ScrnxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ScrnxtraXtra::open(ObjectType type, const Common::Path &path) {
    ScrnxtraXtraObject::initMethods(xlibMethods);
    ScrnxtraXtraObject *xobj = new ScrnxtraXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ScrnxtraXtra::close(ObjectType type) {
    ScrnxtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ScrnxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ScrnxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ScrnxtraXtra::m_StageToMember, 0)
XOBJSTUB(ScrnxtraXtra::m_StageResizeToMember, 0)
XOBJSTUB(ScrnxtraXtra::m_StageToClipboard, 0)
XOBJSTUB(ScrnxtraXtra::m_StageResizeToClipboard, 0)
XOBJSTUB(ScrnxtraXtra::m_StageToFile, 0)
XOBJSTUB(ScrnxtraXtra::m_StageResizeToFile, 0)
XOBJSTUB(ScrnxtraXtra::m_ScreenToMember, 0)
XOBJSTUB(ScrnxtraXtra::m_ScreenResizeToMember, 0)
XOBJSTUB(ScrnxtraXtra::m_ScreenToClipboard, 0)
XOBJSTUB(ScrnxtraXtra::m_ScreenResizeToClipboard, 0)
XOBJSTUB(ScrnxtraXtra::m_ScreenToFile, 0)
XOBJSTUB(ScrnxtraXtra::m_ScreenResizeToFile, 0)

}
