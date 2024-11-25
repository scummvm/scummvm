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
#include "director/lingo/xlibs/syscolor.h"

/**************************************************
 *
 * USED IN:
 * Mummy: Tomb of the Pharaoh
 *
 **************************************************/

/*
-- System color Save/Restore XObject
--SysColor
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
X      mSaveColors         --Saves windows system colors, then sets UI to monochrome
X      mRestoreColors      --Restores windows system colors
 */

namespace Director {

const char *const SysColorXObj::xlibName = "SysColor";
const XlibFileDesc SysColorXObj::fileNames[] = {
	{ "SYSCOLOR",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				SysColorXObj::m_new,		 0, 0,	400 },
	{ "dispose",				SysColorXObj::m_dispose,		 0, 0,	400 },
	{ "saveColors",				SysColorXObj::m_saveColors,		 0, 0,	400 },
	{ "restoreColors",				SysColorXObj::m_restoreColors,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SysColorXObject::SysColorXObject(ObjectType ObjectType) :Object<SysColorXObject>("SysColor") {
	_objType = ObjectType;
}

void SysColorXObj::open(ObjectType type, const Common::Path &path) {
    SysColorXObject::initMethods(xlibMethods);
    SysColorXObject *xobj = new SysColorXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SysColorXObj::close(ObjectType type) {
    SysColorXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SysColorXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SysColorXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(SysColorXObj::m_dispose)
XOBJSTUBNR(SysColorXObj::m_saveColors)
XOBJSTUBNR(SysColorXObj::m_restoreColors)

}
