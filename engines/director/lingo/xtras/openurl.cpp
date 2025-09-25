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
#include "director/lingo/xtras/openurl.h"

/**************************************************
 *
 * USED IN:
 * Cracking the Conspiracy
 *
 **************************************************/

/*
-- xtra OpenURL
new object me

* gsOpenURL string URL -- opens URL in default browser; returns 1 for success, 0 for failure.

-- Copyright: Gary Smith, 9th August, 1997.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au
-- This Xtra may be freely distributed.


 */

namespace Director {

const char *OpenURLXtra::xlibName = "OpenURL";
const XlibFileDesc OpenURLXtra::fileNames[] = {
	{ "openurl",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				OpenURLXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "gsOpenURL", OpenURLXtra::m_gsOpenURL, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

OpenURLXtraObject::OpenURLXtraObject(ObjectType ObjectType) :Object<OpenURLXtraObject>("OpenURL") {
	_objType = ObjectType;
}

bool OpenURLXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum OpenURLXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(OpenURLXtra::xlibName);
	warning("OpenURLXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void OpenURLXtra::open(ObjectType type, const Common::Path &path) {
    OpenURLXtraObject::initMethods(xlibMethods);
    OpenURLXtraObject *xobj = new OpenURLXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void OpenURLXtra::close(ObjectType type) {
    OpenURLXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void OpenURLXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("OpenURLXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(OpenURLXtra::m_gsOpenURL, 0)

}
