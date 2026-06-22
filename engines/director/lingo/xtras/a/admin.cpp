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
#include "director/lingo/xtras/admin.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra Admin
new object me
-- AdminXtra --
* CDRomPath -- global method to get the CDROM's path.
* SetKey string KeyName, string KeyValue  -- global method to set a registry key in SmartFriend.

 */

namespace Director {

const char *AdminXtra::xlibName = "Admin";
const XlibFileDesc AdminXtra::fileNames[] = {
	{ "admin",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AdminXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "CDRomPath", AdminXtra::m_CDRomPath, 0, 0, 500, HBLTIN },
	{ "SetKey", AdminXtra::m_SetKey, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AdminXtraObject::AdminXtraObject(ObjectType ObjectType) :Object<AdminXtraObject>("Admin") {
	_objType = ObjectType;
}

bool AdminXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AdminXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AdminXtra::xlibName);
	warning("AdminXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AdminXtra::open(ObjectType type, const Common::Path &path) {
    AdminXtraObject::initMethods(xlibMethods);
    AdminXtraObject *xobj = new AdminXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AdminXtra::close(ObjectType type) {
    AdminXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AdminXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AdminXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AdminXtra::m_CDRomPath, 0)
XOBJSTUB(AdminXtra::m_SetKey, 0)

}
