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

/*************************************
 *
 * USED IN:
 * tivolafall1998
 * tivolaspring1999
 * tivolasummer2000
 *
 *************************************/

/*
-- FindWin. Returns path to Windows directory. Mark_Carolan@aapda.com.au
--FindWin
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mDo                 --Return the System directory path as a string
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/findwin.h"


namespace Director {

// The name is different from the obj filename.
const char *const FindWin::xlibName = "FindWin";
const XlibFileDesc FindWin::fileNames[] = {
	{ "FindWin",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",		FindWin::m_new,			 0, 0,	400 },	// D4
	{ "do",			FindWin::m_do,			 0, 0,  400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void FindWin::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		FindWinXObject::initMethods(xlibMethods);
		FindWinXObject *xobj = new FindWinXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void FindWin::close(ObjectType type) {
	if (type == kXObj) {
		FindWinXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


FindWinXObject::FindWinXObject(ObjectType ObjectType) : Object<FindWinXObject>("FindWin") {
	_objType = ObjectType;
}

void FindWin::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void FindWin::m_do(int nargs) {
	g_lingo->push(Common::String("C:\\WINDOWS\\"));
}

} // End of namespace Director
