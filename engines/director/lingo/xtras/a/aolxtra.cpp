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
#include "director/lingo/xtras/aolxtra.h"

/**************************************************
 *
 * USED IN:
 * Wishbone and the Amazing Odyssey
 *
 **************************************************/

/*
-- xtra AOLXtra
--   
--    (C) 1996 Human Code, Inc.
--    http://www.humancode.com
-- 
new object me
--    new handler: present for all Xtras
--   
* AOLLaunch any theList 
--    Launches (and installs if necessary) AOL.
--    Accepts a property list of parameters - 
--    those parameters can be any combination of the following:
--      name : screen name of AOL user
--      pass : password of AOL user
--      keyword : keyword to go to when AOL is launched
--      url : URL to go to (requires AOL Browser installation)
--      install_path : complete path to AOL installer.
--   
* AOLErrString integer error 
--    For a given error, return a human readable error string
--   
* AOLVersion 
--    Return the version of AOL that's installed, 
--    or an empty string if the AOL client is not found.
--   
* AOLInstall string installerPath 
--    Launch the AOL installer.
--   

 */

namespace Director {

const char *AolxtraXtra::xlibName = "Aolxtra";
const XlibFileDesc AolxtraXtra::fileNames[] = {
	{ "aolxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AolxtraXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "AOLLaunch", AolxtraXtra::m_AOLLaunch, 1, 1, 500, HBLTIN },
	{ "AOLErrString", AolxtraXtra::m_AOLErrString, 1, 1, 500, HBLTIN },
	{ "AOLVersion", AolxtraXtra::m_AOLVersion, 0, 0, 500, HBLTIN },
	{ "AOLInstall", AolxtraXtra::m_AOLInstall, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AolxtraXtraObject::AolxtraXtraObject(ObjectType ObjectType) :Object<AolxtraXtraObject>("Aolxtra") {
	_objType = ObjectType;
}

bool AolxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AolxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AolxtraXtra::xlibName);
	warning("AolxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AolxtraXtra::open(ObjectType type, const Common::Path &path) {
    AolxtraXtraObject::initMethods(xlibMethods);
    AolxtraXtraObject *xobj = new AolxtraXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AolxtraXtra::close(ObjectType type) {
    AolxtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AolxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AolxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AolxtraXtra::m_AOLLaunch, 0)
XOBJSTUB(AolxtraXtra::m_AOLErrString, 0)
XOBJSTUB(AolxtraXtra::m_AOLVersion, 0)
XOBJSTUB(AolxtraXtra::m_AOLInstall, 0)

}
