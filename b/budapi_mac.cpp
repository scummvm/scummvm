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
#include "director/lingo/xtras/b/budapi_mac.h"

/**************************************************
 *
 * USED IN:
 * Betty Crocker Holiday Celebrations
 * Camp Eatawella
 *
 **************************************************/

/*
-- xtra BudAPI_Mac
new object me

* baFileCreator string Filename -- returns the creator of Filename
* baFileType string Filename -- returns the file type of Filename
* baSetFileInfo string Filename, string Type, string Creator -- sets the type and creator of Filename
* baMsgBoxButtons string OK, string Cancel, string Yes, string No, string Retry, string Ignore, string Abort -- sets MsgBox buttons
* baGestalt string Selector -- returns the gestalt value of Selector
* baGestaltExists string Selector -- checks whether Selector gestalt is available
* baIsBundle string Filename -- checks whether Filename is a bundle
* baFilePermissions string Filename, string User -- returns UNIX permissions of a file
* baSetFilePermissions string Filename, string User, string Permissions -- sets UNIX permissions of a file
* baReturnUnixNames integer On -- specifies whether to return Unix names
* baUnixName string Filename -- returns file name in HFS format
* baHfsName string Filename -- returns name in Unix format
* baLanguage -- return language currently in use

-- Version 1.7 - contains dummy Buddy API Mac functions for Windows-based authoring
-- Copyright: Gary Smith, 25 February, 2005.
-- Email: buddy@mods.com.au
-- Web site: http://www.mods.com.au

"
 */

namespace Director {

const char *Budapi_macXtra::xlibName = "Budapi_mac";
const XlibFileDesc Budapi_macXtra::fileNames[] = {
	{ "budapi_mac",   nullptr },
	{ "bamac",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				Budapi_macXtra::m_new,		 0, 0,	500 },
	{ """,				Budapi_macXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baFileCreator", Budapi_macXtra::m_baFileCreator, 1, 1, 500, HBLTIN },
	{ "baFileType", Budapi_macXtra::m_baFileType, 1, 1, 500, HBLTIN },
	{ "baSetFileInfo", Budapi_macXtra::m_baSetFileInfo, 3, 3, 500, HBLTIN },
	{ "baMsgBoxButtons", Budapi_macXtra::m_baMsgBoxButtons, 7, 7, 500, HBLTIN },
	{ "baGestalt", Budapi_macXtra::m_baGestalt, 1, 1, 500, HBLTIN },
	{ "baGestaltExists", Budapi_macXtra::m_baGestaltExists, 1, 1, 500, HBLTIN },
	{ "baIsBundle", Budapi_macXtra::m_baIsBundle, 1, 1, 500, HBLTIN },
	{ "baFilePermissions", Budapi_macXtra::m_baFilePermissions, 2, 2, 500, HBLTIN },
	{ "baSetFilePermissions", Budapi_macXtra::m_baSetFilePermissions, 3, 3, 500, HBLTIN },
	{ "baReturnUnixNames", Budapi_macXtra::m_baReturnUnixNames, 1, 1, 500, HBLTIN },
	{ "baUnixName", Budapi_macXtra::m_baUnixName, 1, 1, 500, HBLTIN },
	{ "baHfsName", Budapi_macXtra::m_baHfsName, 1, 1, 500, HBLTIN },
	{ "baLanguage", Budapi_macXtra::m_baLanguage, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

Budapi_macXtraObject::Budapi_macXtraObject(ObjectType ObjectType) :Object<Budapi_macXtraObject>("Budapi_mac") {
	_objType = ObjectType;
}

bool Budapi_macXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum Budapi_macXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(Budapi_macXtra::xlibName);
	warning("Budapi_macXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void Budapi_macXtra::open(ObjectType type, const Common::Path &path) {
    Budapi_macXtraObject::initMethods(xlibMethods);
    Budapi_macXtraObject *xobj = new Budapi_macXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void Budapi_macXtra::close(ObjectType type) {
    Budapi_macXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void Budapi_macXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("Budapi_macXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(Budapi_macXtra::m_baFileCreator, 0)
XOBJSTUB(Budapi_macXtra::m_baFileType, 0)
XOBJSTUB(Budapi_macXtra::m_baSetFileInfo, 0)
XOBJSTUB(Budapi_macXtra::m_baMsgBoxButtons, 0)
XOBJSTUB(Budapi_macXtra::m_baGestalt, 0)
XOBJSTUB(Budapi_macXtra::m_baGestaltExists, 0)
XOBJSTUB(Budapi_macXtra::m_baIsBundle, 0)
XOBJSTUB(Budapi_macXtra::m_baFilePermissions, 0)
XOBJSTUB(Budapi_macXtra::m_baSetFilePermissions, 0)
XOBJSTUB(Budapi_macXtra::m_baReturnUnixNames, 0)
XOBJSTUB(Budapi_macXtra::m_baUnixName, 0)
XOBJSTUB(Budapi_macXtra::m_baHfsName, 0)
XOBJSTUB(Budapi_macXtra::m_baLanguage, 0)
XOBJSTUB(Budapi_macXtra::m_", 0)

}
