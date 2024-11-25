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
#include "director/lingo/xlibs/movemousejp.h"

/**************************************************
 *
 * USED IN:
 * junglepark
 *
 **************************************************/

/*
-- MoveMouse . Implemented by @Sakai Youichi
--MoveMouse
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mName               --Returns the XObject name (Widget)
I      mStatus             --Returns an integer status code
SI     mError, code        --Returns an error string
S      mLastError          --Returns last error string
III    mAdd, arg1, arg2    --Returns arg1+arg2
SSI    mFirst, str, nchars --Return the first nchars of string str
V      mMul, f1, f2        --Returns f1*f2 as floating point
X      mGlobals            --Sample code to Read & Modify globals
X      mSymbols            --Sample code to work with Symbols
X      mSendPerform        --Sample code to show SendPerform call
X      mFactory            --Sample code to find Factory objects
III    mSetMouseLoc,h,v    --Sample code to find Factory objects
I      mWtop               --Sample code to find Factory objects
I      mWbottom            --Sample code to find Factory objects
I      mWleft              --Sample code to find Factory objects
I      mWright             --Sample code to find Factory objects
I      mCtop               --Sample code to find Factory objects
I      mCbottom            --Sample code to find Factory objects
I      mCleft              --Sample code to find Factory objects
I      mCright             --Sample code to find Factory objects
S      mGetWindowsDir      --GetWindows Directory
 */

namespace Director {

const char *const MoveMouseJPXObj::xlibName = "MoveMouse";
const XlibFileDesc MoveMouseJPXObj::fileNames[] = {
	{ "MOVEWIN",   "junglepark" },
	{ "MOVEMOUSE",   "junglepark" },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				MoveMouseJPXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MoveMouseJPXObj::m_dispose,		 0, 0,	400 },
	{ "setMouseLoc",				MoveMouseJPXObj::m_setMouseLoc,		 2, 2,	400 },
	{ "wtop",				MoveMouseJPXObj::m_wtop,		 0, 0,	400 },
	{ "wbottom",				MoveMouseJPXObj::m_wbottom,		 0, 0,	400 },
	{ "wleft",				MoveMouseJPXObj::m_wleft,		 0, 0,	400 },
	{ "wright",				MoveMouseJPXObj::m_wright,		 0, 0,	400 },
	{ "ctop",				MoveMouseJPXObj::m_ctop,		 0, 0,	400 },
	{ "cbottom",				MoveMouseJPXObj::m_cbottom,		 0, 0,	400 },
	{ "cleft",				MoveMouseJPXObj::m_cleft,		 0, 0,	400 },
	{ "cright",				MoveMouseJPXObj::m_cright,		 0, 0,	400 },
	{ "getWindowsDir",				MoveMouseJPXObj::m_getWindowsDir,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MoveMouseJPXObject::MoveMouseJPXObject(ObjectType ObjectType) :Object<MoveMouseJPXObject>("MoveMouse") {
	_objType = ObjectType;
}

void MoveMouseJPXObj::open(ObjectType type, const Common::Path &path) {
    MoveMouseJPXObject::initMethods(xlibMethods);
    MoveMouseJPXObject *xobj = new MoveMouseJPXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MoveMouseJPXObj::close(ObjectType type) {
    MoveMouseJPXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MoveMouseJPXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MoveMouseJPXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MoveMouseJPXObj::m_dispose)

void MoveMouseJPXObj::m_setMouseLoc(int nargs) {
	if (nargs != 2) {
		warning("MoveMouseJPXObj::m_setMouseLoc: expected 2 arguments");
		g_lingo->dropStack(nargs);
		return;
	}
	int y = g_lingo->pop().asInt();
	int x = g_lingo->pop().asInt();
	g_system->warpMouse(x, y);
}

XOBJSTUB(MoveMouseJPXObj::m_wtop, 0)
XOBJSTUB(MoveMouseJPXObj::m_wbottom, 0)
XOBJSTUB(MoveMouseJPXObj::m_wleft, 0)
XOBJSTUB(MoveMouseJPXObj::m_wright, 0)
XOBJSTUB(MoveMouseJPXObj::m_ctop, 0)
XOBJSTUB(MoveMouseJPXObj::m_cbottom, 0)
XOBJSTUB(MoveMouseJPXObj::m_cleft, 0)
XOBJSTUB(MoveMouseJPXObj::m_cright, 0)
XOBJSTUB(MoveMouseJPXObj::m_getWindowsDir, "")

}
