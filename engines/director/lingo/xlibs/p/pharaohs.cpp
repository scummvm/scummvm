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
#include "director/lingo/xlibs/p/pharaohs.h"

/**************************************************
 *
 * USED IN:
 * gordak
 *
 **************************************************/

/*
-- Pharaohs External Factory. 16Feb93 PTM
--Pharaohs
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mWindowsdir         --Return the first nchars of string str
ISSSS  mWritestring        --Write a string into an initialization file
SSSS   mGetstring          --Get a string from an initialization file
IS     mCheckattrib        --Check a file's attribute
S      mCheckDrive         --Check the possible CD-ROM drive
 */

namespace Director {

const char *PharaohsXObj::xlibName = "Pharaohs";
const XlibFileDesc PharaohsXObj::fileNames[] = {
	{ "GORDAKCD",   "gordak" },
	{ "Pharaohs",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				PharaohsXObj::m_new,		 0, 0,	400 },
	{ "dispose",				PharaohsXObj::m_dispose,		 0, 0,	400 },
	{ "windowsdir",				PharaohsXObj::m_windowsdir,		 0, 0,	400 },
	{ "writestring",				PharaohsXObj::m_writestring,		 4, 4,	400 },
	{ "getstring",				PharaohsXObj::m_getstring,		 3, 3,	400 },
	{ "checkattrib",				PharaohsXObj::m_checkattrib,		 1, 1,	400 },
	{ "checkDrive",				PharaohsXObj::m_checkDrive,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PharaohsXObject::PharaohsXObject(ObjectType ObjectType) :Object<PharaohsXObject>("Pharaohs") {
	_objType = ObjectType;
}

void PharaohsXObj::open(ObjectType type, const Common::Path &path) {
    PharaohsXObject::initMethods(xlibMethods);
    PharaohsXObject *xobj = new PharaohsXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PharaohsXObj::close(ObjectType type) {
    PharaohsXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PharaohsXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PharaohsXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(PharaohsXObj::m_dispose)
XOBJSTUB(PharaohsXObj::m_windowsdir, "C:\\WINDOWS")
XOBJSTUB(PharaohsXObj::m_writestring, 0)
XOBJSTUB(PharaohsXObj::m_getstring, "")
XOBJSTUB(PharaohsXObj::m_checkattrib, -1)
XOBJSTUB(PharaohsXObj::m_checkDrive, "D")

}
