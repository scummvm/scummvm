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
#include "director/lingo/xlibs/colorcursorxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- ColorCursor, and X-Object for using color cursor resources
--   by Chris Perkins 1993. (CPerkins@aol.com)
I	mNew
XI	mGetSetCursor, crsrID -- crsrID is the resource ID number of a 'crsr' type resource
X	mReleaseCursor -- CALL THIS BEFORE DISPOSING OBJECT or using the lingo <cursor> routine
*/

namespace Director {

const char *const ColorCursorXObj::xlibName = "colorcursorxobj";
const XlibFileDesc ColorCursorXObj::fileNames[] = {
	{ "CCURSOR.XOB",	nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				ColorCursorXObj::m_new,		 		0, 0,	400 },
	{ "dispose",			ColorCursorXObj::m_dispose,		 	0, 0,	400 },
	{ "getsetcursor",		ColorCursorXObj::m_getSetCursor,	1, 1,	400 },
	{ "releasecursor",		ColorCursorXObj::m_releaseCursor,	0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

ColorCursorXObject::ColorCursorXObject(ObjectType ObjectType) :Object<ColorCursorXObject>("ColorCursorXObj") {
	_objType = ObjectType;
}

void ColorCursorXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		ColorCursorXObject::initMethods(xlibMethods);
		ColorCursorXObject *xobj = new ColorCursorXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void ColorCursorXObj::close(ObjectType type) {
	if (type == kXObj) {
		ColorCursorXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void ColorCursorXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ColorCursorXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(ColorCursorXObj::m_dispose)
XOBJSTUBNR(ColorCursorXObj::m_getSetCursor)
XOBJSTUBNR(ColorCursorXObj::m_releaseCursor)

}
