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
#include "director/lingo/xlibs/cursorxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 *
 **************************************************/

/*
-- Cursor External Factory. 16/03/95 - AAF - Changed to play concatenated movies
Cursor
I   mNew --Creates a new instance of the XObject
X   mDispose --Disposes of XObject instance
XSS mSetCursor, cursorName, windowName --Sets the window cursor
 */

namespace Director {

const char *const CursorXObj::xlibName = "Cursor";
const XlibFileDesc CursorXObj::fileNames[] = {
	{ "CURSOR",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				CursorXObj::m_new,		 0, 0,	400 },
	{ "dispose",			CursorXObj::m_dispose,		 0, 0,	400 },
	{ "setCursor",			CursorXObj::m_setCursor,		 2, 2,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

CursorXObject::CursorXObject(ObjectType ObjectType) :Object<CursorXObject>("Cursor") {
	_objType = ObjectType;
}

void CursorXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		CursorXObject::initMethods(xlibMethods);
		CursorXObject *xobj = new CursorXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void CursorXObj::close(ObjectType type) {
	if (type == kXObj) {
		CursorXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void CursorXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("CursorXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(CursorXObj::m_dispose)
XOBJSTUBNR(CursorXObj::m_setCursor)

}
