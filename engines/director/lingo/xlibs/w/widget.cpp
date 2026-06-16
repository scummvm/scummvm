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
#include "director/lingo/xlibs/w/widget.h"

/**************************************************
 *
 * USED IN:
 * Alice: An Interactive Museum
 * Yellow Brick Road
 *
 **************************************************/

/*
-- Widget External Factory. 16Feb93 PTM
--Widget
IS     mNew, FileName      --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mName               --Returns the XObject name (Widget)
I      mStatus             --Returns an integer status code
SI     mError, code        --Returns an error string
S      mLastError          --Returns last error string
III    mAdd, arg1, arg2    --Returns arg1+arg2
I      mAskQuit             --Returns an integer status code
SSI    mFirst, str, nchars --Return the first nchars of string str
ISI    mLipSync, str, nchars --Return the first nchars of string str
I      mLoadData             --Returns an integer status code
II     mSaveData place       --Returns an integer status code
I		mHideCursor				--Returns Status
I		mShowCursor				--Returns Status
V      mMul, f1, f2        --Returns f1*f2 as floating point
X      mGlobals            --Sample code to Read & Modify globals
X      mSymbols            --Sample code to work with Symbols
X      mSendPerform        --Sample code to show SendPerform call
X      mFactory            --Sample code to find Factory objects
 */

namespace Director {

const char *WidgetXObj::xlibName = "Widget";
const XlibFileDesc WidgetXObj::fileNames[] = {
	{ "Widget",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				WidgetXObj::m_new,		 1, 1,	300 },
	{ "dispose",				WidgetXObj::m_dispose,		 0, 0,	300 },
	// Yellow Brick Road
	{ "name",				WidgetXObj::m_name,		 0, 0,	300 },
	{ "status",				WidgetXObj::m_status,		 0, 0,	300 },
	{ "error",				WidgetXObj::m_error,		 1, 1,	300 },
	{ "lastError",				WidgetXObj::m_lastError,		 0, 0,	300 },
	{ "add",				WidgetXObj::m_add,		 2, 2,	300 },
	{ "askQuit",				WidgetXObj::m_askQuit,		 0, 0,	300 },
	{ "first",				WidgetXObj::m_first,		 2, 2,	300 },
	{ "lipSync",				WidgetXObj::m_lipSync,		 2, 2,	300 },
	{ "loadData",				WidgetXObj::m_loadData,		 0, 0,	300 },
	{ "saveData",				WidgetXObj::m_saveData,		 1, 1,	300 },
	{ "hideCursor",				WidgetXObj::m_hideCursor,		 0, 0,	300 },
	{ "showCursor",				WidgetXObj::m_showCursor,		 0, 0,	300 },
	{ "mul",				WidgetXObj::m_mul,		 0, 0,	300 },
	{ "globals",				WidgetXObj::m_globals,		 0, 0,	300 },
	{ "symbols",				WidgetXObj::m_symbols,		 0, 0,	300 },
	{ "sendPerform",				WidgetXObj::m_sendPerform,		 0, 0,	300 },
	{ "factory",				WidgetXObj::m_factory,		 0, 0,	300 },
	// Alice
	{ "GetPro",				WidgetXObj::m_getPro,		0, 0,	300 },	// D4

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

WidgetXObject::WidgetXObject(ObjectType ObjectType) :Object<WidgetXObject>("Widget") {
	_objType = ObjectType;
}

void WidgetXObj::open(ObjectType type, const Common::Path &path) {
    WidgetXObject::initMethods(xlibMethods);
    WidgetXObject *xobj = new WidgetXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void WidgetXObj::close(ObjectType type) {
    WidgetXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void WidgetXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("WidgetXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}


XOBJSTUBNR(WidgetXObj::m_dispose)
XOBJSTUB(WidgetXObj::m_name, "")
XOBJSTUB(WidgetXObj::m_status, 0)
XOBJSTUB(WidgetXObj::m_error, "")
XOBJSTUB(WidgetXObj::m_lastError, "")
XOBJSTUB(WidgetXObj::m_add, 0)
XOBJSTUB(WidgetXObj::m_askQuit, 0)
XOBJSTUB(WidgetXObj::m_first, "")

// 100 means the animation is done
XOBJSTUB(WidgetXObj::m_lipSync, 100)

XOBJSTUB(WidgetXObj::m_loadData, 0)
XOBJSTUB(WidgetXObj::m_saveData, 0)
XOBJSTUB(WidgetXObj::m_hideCursor, 0)
XOBJSTUB(WidgetXObj::m_showCursor, 0)
XOBJSTUB(WidgetXObj::m_mul, 0)
XOBJSTUBNR(WidgetXObj::m_globals)
XOBJSTUBNR(WidgetXObj::m_symbols)
XOBJSTUBNR(WidgetXObj::m_sendPerform)
XOBJSTUBNR(WidgetXObj::m_factory)

void WidgetXObj::m_getPro(int nargs) {
	// seems to want a disk drive letter
	g_lingo->push(Datum("D"));
}


}
