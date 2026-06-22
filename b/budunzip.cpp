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
#include "director/lingo/xtras/b/budunzip.h"

/**************************************************
 *
 * USED IN:
 * Baltic Mission
 * Pearson Front Office Management Simulation
 *
 **************************************************/

/*
-- xtra BudUnZip
new object me

* bzUnZip string ZipFile, string OutFolder, string Mode, integer Overwrite, integer Directories, integer Debug

 -- ZipFile is the name of the file to unzip.
 -- OutFolder is the directory the files will be unzipped to.
 -- Mode can be 'Add' - unzips all files
 --             'Update' - unzips newer files, and files that don't exist
 --             'Freshen' - only unzips existing files that are newer.
 -- If Overwrite is true. existing files will be overwritten.
 -- If Directories is true, then the directory structure in the zip file will be recreated.
 -- If Debug is true, a series of debug messages will appear.

 -- Returns the number of files unzipped, or -1 for error.


 -- Version 0.5
 -- Copyright: Gary Smith, 21st September, 1997.
 -- Email: gary@mods.com.au
 -- Web site: http://www.mods.com.au
 -- This Xtra use zip code from the Info-Zip project. http://www.cdrom.com/infozip

"
 */

namespace Director {

const char *BudunzipXtra::xlibName = "Budunzip";
const XlibFileDesc BudunzipXtra::fileNames[] = {
	{ "budunzip",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BudunzipXtra::m_new,		 0, 0,	500 },
	{ """,				BudunzipXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "bzUnZip", BudunzipXtra::m_bzUnZip, 6, 6, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BudunzipXtraObject::BudunzipXtraObject(ObjectType ObjectType) :Object<BudunzipXtraObject>("Budunzip") {
	_objType = ObjectType;
}

bool BudunzipXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BudunzipXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BudunzipXtra::xlibName);
	warning("BudunzipXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BudunzipXtra::open(ObjectType type, const Common::Path &path) {
    BudunzipXtraObject::initMethods(xlibMethods);
    BudunzipXtraObject *xobj = new BudunzipXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BudunzipXtra::close(ObjectType type) {
    BudunzipXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BudunzipXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BudunzipXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BudunzipXtra::m_bzUnZip, 0)
XOBJSTUB(BudunzipXtra::m_", 0)

}
