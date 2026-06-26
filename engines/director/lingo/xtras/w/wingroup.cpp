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
#include "director/lingo/xtras/w/wingroup.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 2 (D6)
 *
 **************************************************/

/*
-- xtra WinGroup
+ new object me
+ register object me, string code
-- methods:
createGroup object me, string name                                    -- create program manager group (result: int success)
showGroup object me, string name, integer showcmd                     -- show program manager group (result: int success)
deleteGroup object me, string name                                    -- delete program manager group (result: int success)
reloadGroup object me, string name                                    -- reload program manager group (result: int success)
addItem object me, string cmdline, string name, string defdir         -- add item to active group (result: int success)
replaceItem object me, string name                                    -- replace item in active group (result: int success)
deleteItem object me, string name                                     -- delete item from active group (result: int success)
exitProgman object me, integer exittype                               -- exit program manager - shut down windows (result: int success)
listGroups object me                                                  -- get list of groups (result: string list)
listItems object me, string name                                      -- get list of items in a group (result: string list)
attrib object me, string filename, string attribs, integer recursive  -- sets attributes (result: errorcode)
 */

namespace Director {

const char *WinGroupXtra::xlibName = "WinGroup";
const XlibFileDesc WinGroupXtra::fileNames[] = {
	{ "wingroup",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "createGroup",				WinGroupXtra::m_createGroup,		 1, 1,	500 },
	{ "showGroup",				WinGroupXtra::m_showGroup,		 2, 2,	500 },
	{ "deleteGroup",				WinGroupXtra::m_deleteGroup,		 1, 1,	500 },
	{ "reloadGroup",				WinGroupXtra::m_reloadGroup,		 1, 1,	500 },
	{ "addItem",				WinGroupXtra::m_addItem,		 3, 3,	500 },
	{ "replaceItem",				WinGroupXtra::m_replaceItem,		 1, 1,	500 },
	{ "deleteItem",				WinGroupXtra::m_deleteItem,		 1, 1,	500 },
	{ "exitProgman",				WinGroupXtra::m_exitProgman,		 1, 1,	500 },
	{ "listGroups",				WinGroupXtra::m_listGroups,		 0, 0,	500 },
	{ "listItems",				WinGroupXtra::m_listItems,		 1, 1,	500 },
	{ "attrib",				WinGroupXtra::m_attrib,		 3, 3,	500 },
	{ "new",				WinGroupXtra::m_new,		 0, 0,	500 },
	{ "register",				WinGroupXtra::m_register,		 1, 1,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

WinGroupXtraObject::WinGroupXtraObject(ObjectType ObjectType) :Object<WinGroupXtraObject>("WinGroup") {
	_objType = ObjectType;
}

bool WinGroupXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum WinGroupXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(WinGroupXtra::xlibName);
	warning("WinGroupXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void WinGroupXtra::open(ObjectType type, const Common::Path &path) {
    WinGroupXtraObject::initMethods(xlibMethods);
    WinGroupXtraObject *xobj = new WinGroupXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void WinGroupXtra::close(ObjectType type) {
    WinGroupXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void WinGroupXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("WinGroupXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(WinGroupXtra::m_register, 0)
XOBJSTUB(WinGroupXtra::m_createGroup, 0)
XOBJSTUB(WinGroupXtra::m_showGroup, 0)
XOBJSTUB(WinGroupXtra::m_deleteGroup, 0)
XOBJSTUB(WinGroupXtra::m_reloadGroup, 0)
XOBJSTUB(WinGroupXtra::m_addItem, 0)
XOBJSTUB(WinGroupXtra::m_replaceItem, 0)
XOBJSTUB(WinGroupXtra::m_deleteItem, 0)
XOBJSTUB(WinGroupXtra::m_exitProgman, 0)
XOBJSTUB(WinGroupXtra::m_listGroups, 0)
XOBJSTUB(WinGroupXtra::m_listItems, 0)
XOBJSTUB(WinGroupXtra::m_attrib, 0)

}
