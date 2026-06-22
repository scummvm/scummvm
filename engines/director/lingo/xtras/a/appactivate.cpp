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
#include "director/lingo/xtras/a/appactivate.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra appActivate -- This is a first release of an Xtra to provide two new handlers to Director, to enable you to know when Director is activated and deactivated.
new object me

-- Installs new Lingo handlers, on appActivate and on appDeactivate

* startAppActivate -- sets up appActivate handler, place in on startMovie handler.
* closeAppActivate -- destroys appActivate handler, place in on stopMovie handler.

-- Copyright Gary Smith, 1999
-- Email: gary@mods.com.au
-- Web: http:www.mods.com.au

-- This Xtra may be freely distributed
"
 */

namespace Director {

const char *AppactivateXtra::xlibName = "Appactivate";
const XlibFileDesc AppactivateXtra::fileNames[] = {
	{ "appactivate",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AppactivateXtra::m_new,		 0, 0,	600 },
	{ """,				AppactivateXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "startAppActivate", AppactivateXtra::m_startAppActivate, 0, 0, 600, HBLTIN },
	{ "closeAppActivate", AppactivateXtra::m_closeAppActivate, 0, 0, 600, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AppactivateXtraObject::AppactivateXtraObject(ObjectType ObjectType) :Object<AppactivateXtraObject>("Appactivate") {
	_objType = ObjectType;
}

bool AppactivateXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AppactivateXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AppactivateXtra::xlibName);
	warning("AppactivateXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AppactivateXtra::open(ObjectType type, const Common::Path &path) {
    AppactivateXtraObject::initMethods(xlibMethods);
    AppactivateXtraObject *xobj = new AppactivateXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AppactivateXtra::close(ObjectType type) {
    AppactivateXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AppactivateXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AppactivateXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AppactivateXtra::m_startAppActivate, 0)
XOBJSTUB(AppactivateXtra::m_closeAppActivate, 0)
XOBJSTUB(AppactivateXtra::m_", 0)

}
