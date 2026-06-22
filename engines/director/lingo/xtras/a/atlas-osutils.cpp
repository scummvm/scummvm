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
#include "director/lingo/xtras/a/atlas-osutils.h"

/**************************************************
 *
 * USED IN:
 * Atlas der Schweiz \ Atlas of Switzerland 
 * Atlas der Schweiz \ Atlas of Switzerland 2
 *
 **************************************************/

/*
-- xtra Utils -- OSUtils Xtra provides a number of useful utility functions.
-- http://www.atlasofswitzerland.ch/
-- http://www.swisstopo.ch/en/products/digital/multimedia/ads2/
new object me
* XRegister string str
* XRequirements -- system, expired
* XLanguageCode -- get current language code
* XSetLanguageCode string inCode -- set language code
* XSetCursor integer inID -- set cursor by id
* XSetStageRect object inRect -- set cursor by id
* XFindApplicationPath integer inApp -- find the path of the given application
* XSetApplicationPath string inPath -- set the application path
* XSetAdSParam integer inParam, string inValue -- set an AdS param
* XAdSParam integer inParam -- get an AdS param value
* XText string id -- get text by id
* XMac -- Mac OS
* XMac_X -- Mac OS X
* XMac_9 -- Mac OS 9
* XWin -- Windows
* XVersionOS -- get OS version
* XVersionCarbon -- get Carbon version
* XVersionQT -- get QuickTime version
"
 */

namespace Director {

const char *UtilsXtra::xlibName = "Utils";
const XlibFileDesc UtilsXtra::fileNames[] = {
	{ "utils",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				UtilsXtra::m_new,		 0, 0,	600 },
	{ """,				UtilsXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "XRegister", UtilsXtra::m_XRegister, 1, 1, 600, HBLTIN },
	{ "XRequirements", UtilsXtra::m_XRequirements, 0, 0, 600, HBLTIN },
	{ "XLanguageCode", UtilsXtra::m_XLanguageCode, 0, 0, 600, HBLTIN },
	{ "XSetLanguageCode", UtilsXtra::m_XSetLanguageCode, 1, 1, 600, HBLTIN },
	{ "XSetCursor", UtilsXtra::m_XSetCursor, 1, 1, 600, HBLTIN },
	{ "XSetStageRect", UtilsXtra::m_XSetStageRect, 1, 1, 600, HBLTIN },
	{ "XFindApplicationPath", UtilsXtra::m_XFindApplicationPath, 1, 1, 600, HBLTIN },
	{ "XSetApplicationPath", UtilsXtra::m_XSetApplicationPath, 1, 1, 600, HBLTIN },
	{ "XSetAdSParam", UtilsXtra::m_XSetAdSParam, 2, 2, 600, HBLTIN },
	{ "XAdSParam", UtilsXtra::m_XAdSParam, 1, 1, 600, HBLTIN },
	{ "XText", UtilsXtra::m_XText, 1, 1, 600, HBLTIN },
	{ "XMac", UtilsXtra::m_XMac, 0, 0, 600, HBLTIN },
	{ "XMac_X", UtilsXtra::m_XMac_X, 0, 0, 600, HBLTIN },
	{ "XMac_9", UtilsXtra::m_XMac_9, 0, 0, 600, HBLTIN },
	{ "XWin", UtilsXtra::m_XWin, 0, 0, 600, HBLTIN },
	{ "XVersionOS", UtilsXtra::m_XVersionOS, 0, 0, 600, HBLTIN },
	{ "XVersionCarbon", UtilsXtra::m_XVersionCarbon, 0, 0, 600, HBLTIN },
	{ "XVersionQT", UtilsXtra::m_XVersionQT, 0, 0, 600, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

UtilsXtraObject::UtilsXtraObject(ObjectType ObjectType) :Object<UtilsXtraObject>("Utils") {
	_objType = ObjectType;
}

bool UtilsXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum UtilsXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(UtilsXtra::xlibName);
	warning("UtilsXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void UtilsXtra::open(ObjectType type, const Common::Path &path) {
    UtilsXtraObject::initMethods(xlibMethods);
    UtilsXtraObject *xobj = new UtilsXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void UtilsXtra::close(ObjectType type) {
    UtilsXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void UtilsXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("UtilsXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(UtilsXtra::m_XRegister, 0)
XOBJSTUB(UtilsXtra::m_XRequirements, 0)
XOBJSTUB(UtilsXtra::m_XLanguageCode, 0)
XOBJSTUB(UtilsXtra::m_XSetLanguageCode, 0)
XOBJSTUB(UtilsXtra::m_XSetCursor, 0)
XOBJSTUB(UtilsXtra::m_XSetStageRect, 0)
XOBJSTUB(UtilsXtra::m_XFindApplicationPath, 0)
XOBJSTUB(UtilsXtra::m_XSetApplicationPath, 0)
XOBJSTUB(UtilsXtra::m_XSetAdSParam, 0)
XOBJSTUB(UtilsXtra::m_XAdSParam, 0)
XOBJSTUB(UtilsXtra::m_XText, 0)
XOBJSTUB(UtilsXtra::m_XMac, 0)
XOBJSTUB(UtilsXtra::m_XMac_X, 0)
XOBJSTUB(UtilsXtra::m_XMac_9, 0)
XOBJSTUB(UtilsXtra::m_XWin, 0)
XOBJSTUB(UtilsXtra::m_XVersionOS, 0)
XOBJSTUB(UtilsXtra::m_XVersionCarbon, 0)
XOBJSTUB(UtilsXtra::m_XVersionQT, 0)
XOBJSTUB(UtilsXtra::m_", 0)

}
