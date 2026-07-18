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
#include "director/lingo/xtras/b/bamac.h"

/**************************************************
 *
 * USED IN:
 * TKKG 6
 *
 **************************************************/

/*
-- xtra BudAPI_Mac
new object me
* baFileCreator string Filename -- returns the creator of Filename
* baFileType string Filename -- returns the file type of Filename
* baSetFileInfo string Filename, string Type, string Creator -- sets the type and creator of Filename
* baEjectDisk string Disk -- ejects Disk
* baDiskList -- returns list of mounted Disks
* baGestalt string Selector -- returns the gestalt value of Selector
* baGestaltExists string Selector -- checks whether Selector gestalt is available
-- Version 1.1 - contains dummy Buddy API Mac functions for Windows-based authoring
-- Copyright: Gary Smith, 23rd August, 1998.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au

 */

namespace Director {

const char *BudAPIMacXtra::xlibName = "BudAPIMac";
const XlibFileDesc BudAPIMacXtra::fileNames[] = {
	{ "baMac",        nullptr },	// on-disk Xtra filename (baMac.x32)
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BudAPIMacXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baFileCreator", BudAPIMacXtra::m_baFileCreator, 1, 1, 500, HBLTIN },
	{ "baFileType", BudAPIMacXtra::m_baFileType, 1, 1, 500, HBLTIN },
	{ "baSetFileInfo", BudAPIMacXtra::m_baSetFileInfo, 3, 3, 500, HBLTIN },
	{ "baEjectDisk", BudAPIMacXtra::m_baEjectDisk, 1, 1, 500, HBLTIN },
	{ "baDiskList", BudAPIMacXtra::m_baDiskList, 0, 0, 500, HBLTIN },
	{ "baGestalt", BudAPIMacXtra::m_baGestalt, 1, 1, 500, HBLTIN },
	{ "baGestaltExists", BudAPIMacXtra::m_baGestaltExists, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BudAPIMacXtraObject::BudAPIMacXtraObject(ObjectType ObjectType) :Object<BudAPIMacXtraObject>("BudAPIMac") {
	_objType = ObjectType;
}

bool BudAPIMacXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BudAPIMacXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BudAPIMacXtra::xlibName);
	warning("BudAPIMacXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BudAPIMacXtra::open(ObjectType type, const Common::Path &path) {
    BudAPIMacXtraObject::initMethods(xlibMethods);
    BudAPIMacXtraObject *xobj = new BudAPIMacXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BudAPIMacXtra::close(ObjectType type) {
    BudAPIMacXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BudAPIMacXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BudAPIMacXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BudAPIMacXtra::m_baFileCreator, 0)
XOBJSTUB(BudAPIMacXtra::m_baFileType, 0)
XOBJSTUB(BudAPIMacXtra::m_baSetFileInfo, 0)
XOBJSTUB(BudAPIMacXtra::m_baEjectDisk, 0)
XOBJSTUB(BudAPIMacXtra::m_baDiskList, 0)
XOBJSTUB(BudAPIMacXtra::m_baGestalt, 0)
XOBJSTUB(BudAPIMacXtra::m_baGestaltExists, 0)

}
