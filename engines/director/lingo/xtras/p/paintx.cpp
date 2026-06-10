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
#include "director/lingo/xtras/p/paintx.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 2
 *
 **************************************************/

/*
-- xtra PaintX    -- Version 1.0 (WIN/32 Bit)

-- --------------------------------------------------------------------
-- Painting Xtra
-- --------------------------------------------------------------------

-- �1998 by Stephan Eichhorn, Scirius Development
-- e-mail:  xtras@scririus.com
-- WWW:     http://www.scirius.com

-- special version for <Loewenzahn 2> only
-- --------------------------------------------------------------------
new object me                           -- create a new instance
-- Methods --
setRunMode object me, integer Mode      -- set the runMode (1,2,3)
           -- 1: normal
           -- 2: framed
           -- 3: mirrored
getRunMode object me                    -- returns the runMode
setRunState object me, integer State    -- set the runState (0,1)
           -- 0: stopped
           -- 1: running
getRunState object me                   -- returns the runState
setData object me, string Data          -- set the session data
getPos object me                        -- get position in data
getDataSize object me                   -- get the size of data
SetDrawRect object me, rect DrawRect    -- set the drawing rectangle
SetClipRect object me, rect ClipRect    -- set the clipping rectangle
Draw object me                          -- draw the next stroke
StageToCast object me, rect CaptureRect, integer MemberNum  
StageToBuffer object me, rect CaptureRect
BufferToStage object me, rect DestRect
setFile object me, string filePath      -- reads file as data
Paint object me, integer Pensize, integer R, integer G, integer B, integer SL, integer ST

 */

namespace Director {

const char *PaintXXtra::xlibName = "PaintX";
const XlibFileDesc PaintXXtra::fileNames[] = {
	{ "paintx",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				PaintXXtra::m_new,		 0, 0,	500 },
	{ "setRunMode",				PaintXXtra::m_setRunMode,		 1, 1,	500 },
	{ "getRunMode",				PaintXXtra::m_getRunMode,		 0, 0,	500 },
	{ "setRunState",				PaintXXtra::m_setRunState,		 1, 1,	500 },
	{ "getRunState",				PaintXXtra::m_getRunState,		 0, 0,	500 },
	{ "setData",				PaintXXtra::m_setData,		 1, 1,	500 },
	{ "getPos",				PaintXXtra::m_getPos,		 0, 0,	500 },
	{ "getDataSize",				PaintXXtra::m_getDataSize,		 0, 0,	500 },
	{ "SetDrawRect",				PaintXXtra::m_SetDrawRect,		 1, 1,	500 },
	{ "SetClipRect",				PaintXXtra::m_SetClipRect,		 1, 1,	500 },
	{ "Draw",				PaintXXtra::m_Draw,		 0, 0,	500 },
	{ "StageToCast",				PaintXXtra::m_StageToCast,		 2, 2,	500 },
	{ "StageToBuffer",				PaintXXtra::m_StageToBuffer,		 1, 1,	500 },
	{ "BufferToStage",				PaintXXtra::m_BufferToStage,		 1, 1,	500 },
	{ "setFile",				PaintXXtra::m_setFile,		 1, 1,	500 },
	{ "Paint",				PaintXXtra::m_Paint,		 6, 6,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PaintXXtraObject::PaintXXtraObject(ObjectType ObjectType) :Object<PaintXXtraObject>("PaintX") {
	_objType = ObjectType;
}

bool PaintXXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum PaintXXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(PaintXXtra::xlibName);
	warning("PaintXXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void PaintXXtra::open(ObjectType type, const Common::Path &path) {
    PaintXXtraObject::initMethods(xlibMethods);
    PaintXXtraObject *xobj = new PaintXXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PaintXXtra::close(ObjectType type) {
    PaintXXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PaintXXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PaintXXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(PaintXXtra::m_setRunMode, 0)
XOBJSTUB(PaintXXtra::m_getRunMode, 0)
XOBJSTUB(PaintXXtra::m_setRunState, 0)
XOBJSTUB(PaintXXtra::m_getRunState, 0)
XOBJSTUB(PaintXXtra::m_setData, 0)
XOBJSTUB(PaintXXtra::m_getPos, 0)
XOBJSTUB(PaintXXtra::m_getDataSize, 0)
XOBJSTUB(PaintXXtra::m_SetDrawRect, 0)
XOBJSTUB(PaintXXtra::m_SetClipRect, 0)
XOBJSTUB(PaintXXtra::m_Draw, 0)
XOBJSTUB(PaintXXtra::m_StageToCast, 0)
XOBJSTUB(PaintXXtra::m_StageToBuffer, 0)
XOBJSTUB(PaintXXtra::m_BufferToStage, 0)
XOBJSTUB(PaintXXtra::m_setFile, 0)
XOBJSTUB(PaintXXtra::m_Paint, 0)

}
