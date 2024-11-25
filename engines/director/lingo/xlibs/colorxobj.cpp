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

/**********************************************
 *
 * USED IN:
 * Star Trek TNG Episode Guide
 *
 **********************************************/

/*
 * -- Color External Factory. 16Feb93 PTM
 * Color
 * I      mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * X      mSetOurColors       --Set Trek-happy colors
 * X      mRestoreColors      --Restore real windows colors
 * S      mName               --Returns the XObject name (Widget)
 * I      mStatus             --Returns an integer status code
 * SI     mError, code        --Returns an error string
 * S      mLastError          --Returns last error string
 * III    mAdd, arg1, arg2    --Returns arg1+arg2
 * SSI    mFirst, str, nchars --Return the first nchars of string str
 * V      mMul, f1, f2        --Returns f1*f2 as floating point
 * X      mGlobals            --Sample code to Read & Modify globals
 * X      mSymbols            --Sample code to work with Symbols
 * X      mSendPerform        --Sample code to show SendPerform call
 * X      mFactory            --Sample code to find Factory objects
 * II     mGetSysColor, attrib  -- gets rgb attrib as a long
 * III    mSetSysColor, attrib, rgbVal    -- sets r,g,b as a long
 * IIIII  mSetSysColorRGB, attrib, r, g, b    -- sets r,g,b as a 3 longs
 * III    mSetSysColorIndex, attrib, nIndex    -- sets nth palette entry
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/colorxobj.h"


namespace Director {

const char *const ColorXObj::xlibName = "Color";
const XlibFileDesc ColorXObj::fileNames[] = {
	{ "color",		nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					ColorXObj::m_new,					1, 1,	400 },	// D4
	{ "dispose",				ColorXObj::m_dispose,				0, 0,	400 },	// D4
	{ "setOurColors",			ColorXObj::m_setOurColors,			0, 0,	400 },	// D4
	{ "restoreColors",			ColorXObj::m_restoreColors,			0, 0,	400 },	// D4
	{ "getSysColor",			ColorXObj::m_getSysColor,			1, 1,	400 },	// D4
	{ "setSysColor",			ColorXObj::m_setSysColor,			2, 2,	400 },	// D4
	{ "setSysColorRGB",			ColorXObj::m_setSysColorRGB,		4, 4,	400 },	// D4
	{ "setSysColorIndex",		ColorXObj::m_setSysColorIndex,		2, 2,	400 },	// D4
	{ nullptr,					nullptr,							0, 0,	0 }
};

ColorXObject::ColorXObject(ObjectType ObjectType) :Object<ColorXObject>("Color") {
	_objType = ObjectType;
}

void ColorXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		ColorXObject::initMethods(xlibMethods);
		ColorXObject *xobj = new ColorXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void ColorXObj::close(ObjectType type) {
	if (type == kXObj) {
		ColorXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


void ColorXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ColorXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ColorXObj::m_dispose, 0)
XOBJSTUBNR(ColorXObj::m_setOurColors)
XOBJSTUBNR(ColorXObj::m_restoreColors)
XOBJSTUB(ColorXObj::m_getSysColor, 0)
XOBJSTUB(ColorXObj::m_setSysColor, 0)
XOBJSTUB(ColorXObj::m_setSysColorRGB, 0)
XOBJSTUB(ColorXObj::m_setSysColorIndex, 0)

} // End of namespace Director
