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
#include "director/lingo/xtras/b/browsercontroller.h"

/**************************************************
 *
 * USED IN:
 * H+a Collection //Found on disk 1 of Celtica, can likely be found on disks of other games released by H+a Entertainment.
 *
 **************************************************/

/*
-- xtra BrowserController
-- BrowserController Xtra for Macromedia Director. 
-- Version 1.0 
--                         Copyright (C) 1997, Magister Ludi s.r.l.
--
-- For other Xtras and multimedia products, check out www.magisterludi.com
--
--
new object me
* BrowserControllerVerifyKey string key -- verifies the key for use in projectors.
                                     -- visit www.magisterludi.com for information
                                     -- about how to obtain a key.
* startProgram string cmdLine -- Launch a program, add parameters after the .EXE filename, if needed. 
                              -- Returns aProgID to use in closeProgram and stillRunningProgram. 
* closeProgram integer aProgID -- close a program 
* stillRunningProgram integer aProgID -- returns TRUE if the program with aProgID is still running. False otherwise.
* getPathFromRegistry string anExeName -- returns the path of an executable as registered in the registry
* findBrowserExecutable string htmlFileName, defaultDir -- returns the path to the default browser or EMPTY if not installed
                                                        -- (it actually returns any executable registered for a given file type)
* getNetscapeInstallDir -- returns the path to the Netscape's installation directory as registered in the registry.
                        -- Notice that this may be the directory where the executable is found if the user
                        -- has moved the installation directory after the installation.
* browserGoToURL string browser, msgStr -- send a URL to the browser
* initDDE -- initialise DDE for communication with the browser
* closeDDE -- close off the communication session
"
 */

namespace Director {

const char *BrowsercontrollerXtra::xlibName = "Browsercontroller";
const XlibFileDesc BrowsercontrollerXtra::fileNames[] = {
	{ "browsercontroller",   nullptr },
	{ "bctl",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BrowsercontrollerXtra::m_new,		 0, 0,	500 },
	{ """,				BrowsercontrollerXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "BrowserControllerVerifyKey", BrowsercontrollerXtra::m_BrowserControllerVerifyKey, 1, 1, 500, HBLTIN },
	{ "startProgram", BrowsercontrollerXtra::m_startProgram, 1, 1, 500, HBLTIN },
	{ "closeProgram", BrowsercontrollerXtra::m_closeProgram, 1, 1, 500, HBLTIN },
	{ "stillRunningProgram", BrowsercontrollerXtra::m_stillRunningProgram, 1, 1, 500, HBLTIN },
	{ "getPathFromRegistry", BrowsercontrollerXtra::m_getPathFromRegistry, 1, 1, 500, HBLTIN },
	{ "findBrowserExecutable", BrowsercontrollerXtra::m_findBrowserExecutable, 2, 2, 500, HBLTIN },
	{ "getNetscapeInstallDir", BrowsercontrollerXtra::m_getNetscapeInstallDir, 0, 0, 500, HBLTIN },
	{ "browserGoToURL", BrowsercontrollerXtra::m_browserGoToURL, 2, 2, 500, HBLTIN },
	{ "initDDE", BrowsercontrollerXtra::m_initDDE, 0, 0, 500, HBLTIN },
	{ "closeDDE", BrowsercontrollerXtra::m_closeDDE, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BrowsercontrollerXtraObject::BrowsercontrollerXtraObject(ObjectType ObjectType) :Object<BrowsercontrollerXtraObject>("Browsercontroller") {
	_objType = ObjectType;
}

bool BrowsercontrollerXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BrowsercontrollerXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BrowsercontrollerXtra::xlibName);
	warning("BrowsercontrollerXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BrowsercontrollerXtra::open(ObjectType type, const Common::Path &path) {
    BrowsercontrollerXtraObject::initMethods(xlibMethods);
    BrowsercontrollerXtraObject *xobj = new BrowsercontrollerXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BrowsercontrollerXtra::close(ObjectType type) {
    BrowsercontrollerXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BrowsercontrollerXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BrowsercontrollerXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BrowsercontrollerXtra::m_BrowserControllerVerifyKey, 0)
XOBJSTUB(BrowsercontrollerXtra::m_startProgram, 0)
XOBJSTUB(BrowsercontrollerXtra::m_closeProgram, 0)
XOBJSTUB(BrowsercontrollerXtra::m_stillRunningProgram, 0)
XOBJSTUB(BrowsercontrollerXtra::m_getPathFromRegistry, 0)
XOBJSTUB(BrowsercontrollerXtra::m_findBrowserExecutable, 0)
XOBJSTUB(BrowsercontrollerXtra::m_getNetscapeInstallDir, 0)
XOBJSTUB(BrowsercontrollerXtra::m_browserGoToURL, 0)
XOBJSTUB(BrowsercontrollerXtra::m_initDDE, 0)
XOBJSTUB(BrowsercontrollerXtra::m_closeDDE, 0)
XOBJSTUB(BrowsercontrollerXtra::m_", 0)

}
