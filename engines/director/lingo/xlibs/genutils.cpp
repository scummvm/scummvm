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
#include "director/lingo/xlibs/genutils.h"

/**************************************************
 *
 * USED IN:
 * Isis
 *
 **************************************************/

/*
-- GenUtils External Factory. 16Feb93 PTM
--GenUtils
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mName               --Returns the XObject name (GenUtils)
I      mStatus             --Returns an integer status code
S      mGetWindowsFolder   --Retrieves the location of the windows folder
I      mFlushEvents        --Clears the event queue.
III     mMoveCursor         --Moves the cursor to x,y
III     mClickCursor         --Moves the cursor to x,y and clicks it.

 */

namespace Director {

const char *const GenUtilsXObj::xlibName = "GenUtils";
const XlibFileDesc GenUtilsXObj::fileNames[] = {
	{ "GENUTILS",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				GenUtilsXObj::m_new,		 0, 0,	400 },
	{ "dispose",				GenUtilsXObj::m_dispose,		 0, 0,	400 },
	{ "name",				GenUtilsXObj::m_name,		 0, 0,	400 },
	{ "status",				GenUtilsXObj::m_status,		 0, 0,	400 },
	{ "getWindowsFolder",				GenUtilsXObj::m_getWindowsFolder,		 0, 0,	400 },
	{ "flushEvents",				GenUtilsXObj::m_flushEvents,		 0, 0,	400 },
	{ "moveCursor",				GenUtilsXObj::m_moveCursor,		 2, 2,	400 },
	{ "clickCursor",				GenUtilsXObj::m_clickCursor,		 2, 2,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

GenUtilsXObject::GenUtilsXObject(ObjectType ObjectType) :Object<GenUtilsXObject>("GenUtils") {
	_objType = ObjectType;
}

void GenUtilsXObj::open(ObjectType type, const Common::Path &path) {
    GenUtilsXObject::initMethods(xlibMethods);
    GenUtilsXObject *xobj = new GenUtilsXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void GenUtilsXObj::close(ObjectType type) {
    GenUtilsXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void GenUtilsXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("GenUtilsXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(GenUtilsXObj::m_dispose)
XOBJSTUB(GenUtilsXObj::m_name, "")
XOBJSTUB(GenUtilsXObj::m_status, 0)
XOBJSTUB(GenUtilsXObj::m_getWindowsFolder, "")
XOBJSTUB(GenUtilsXObj::m_flushEvents, 0)
XOBJSTUB(GenUtilsXObj::m_moveCursor, 0)
XOBJSTUB(GenUtilsXObj::m_clickCursor, 0)

}
