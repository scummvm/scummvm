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
#include "director/lingo/xtras/s/sharpexport.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 8 (D8.5), Löwenzahn 7 (D8), Löwenzahn 3/4/6 (D9),
 * Löwenzahn Spielebox (D9)
 *
 **************************************************/

/*
-- xtra SharpExport
-- Export Bitmap Cast Member Utility Xtra v1.0
-- Copyright � 2002 Sharp Software and Forest Interactive.
-- This software is provided as-is, no warranty, blah blah blah.
-- This software is based in part on the work of the Independent JPEG Group.
-- Thanks to libpng and zlib for the PNG code.
new object me
-- Error codes for various image export functions:
--  0 - success
-- -1 - destination file can not be written
-- -2 - out of memory
-- -3 - wrong number of args
-- -4 - bad parameter
-- -5 - castmember not found
-- -6 - castmember media not found
-- -7 - castmember is not a bitmap
-- -8 - unsupported bitdepth (JPEG support is 8 bits or higher)
-- -9 - JPEG compression failed (internal error with JPEG library
-- -10 - PNG compression failed (internal error with PNG library
exportBMP object me, object member, string pathName
exportJPG object me, object member, string pathName, integer jpegComp
exportPNG object me, object member, string pathName
------------------------------
 */

namespace Director {

const char *SharpExportXtra::xlibName = "SharpExport";
const XlibFileDesc SharpExportXtra::fileNames[] = {
	{ "sharpexport",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				SharpExportXtra::m_new,		 0, 0,	500 },
	{ "exportBMP",				SharpExportXtra::m_exportBMP,		 2, 2,	500 },
	{ "exportJPG",				SharpExportXtra::m_exportJPG,		 3, 3,	500 },
	{ "exportPNG",				SharpExportXtra::m_exportPNG,		 2, 2,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SharpExportXtraObject::SharpExportXtraObject(ObjectType ObjectType) :Object<SharpExportXtraObject>("SharpExport") {
	_objType = ObjectType;
}

bool SharpExportXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum SharpExportXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(SharpExportXtra::xlibName);
	warning("SharpExportXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void SharpExportXtra::open(ObjectType type, const Common::Path &path) {
    SharpExportXtraObject::initMethods(xlibMethods);
    SharpExportXtraObject *xobj = new SharpExportXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SharpExportXtra::close(ObjectType type) {
    SharpExportXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SharpExportXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SharpExportXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(SharpExportXtra::m_exportBMP, 0)
XOBJSTUB(SharpExportXtra::m_exportJPG, 0)
XOBJSTUB(SharpExportXtra::m_exportPNG, 0)

}
