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
#include "director/lingo/xtras/b/buddy_trayicon.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra Buddy_TrayIcon
new object me

* btiCreateTrayIcon integer ID, string Icon, string ToolTipText -- adds tray icon
* btiModifyTrayIcon integer ID, string Icon, string ToolTipText -- modifies tray icon
* btiDeleteTrayIcon integer ID -- deletes tray icon
* btiDeleteAllIcons -- removes all tray icons that have been created
* btiRestoreProgram -- restores Director's window
* btiMinimiseProgram -- minimises Director's window
* btiHideProgram -- hides Director's window
* btiShowMenu integer ID, list menu -- shows popup menu
* btiAbout -- shows information about Buddy TrayIcon

-- Version 0.3b
-- Copyright: Gary Smith, 22nd March, 1999.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au

"
 */

namespace Director {

const char *Buddy_trayiconXtra::xlibName = "Buddy_trayicon";
const XlibFileDesc Buddy_trayiconXtra::fileNames[] = {
	{ "buddy_trayicon",   nullptr },
	{ "budtray",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				Buddy_trayiconXtra::m_new,		 0, 0,	500 },
	{ """,				Buddy_trayiconXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "btiCreateTrayIcon", Buddy_trayiconXtra::m_btiCreateTrayIcon, 3, 3, 500, HBLTIN },
	{ "btiModifyTrayIcon", Buddy_trayiconXtra::m_btiModifyTrayIcon, 3, 3, 500, HBLTIN },
	{ "btiDeleteTrayIcon", Buddy_trayiconXtra::m_btiDeleteTrayIcon, 1, 1, 500, HBLTIN },
	{ "btiDeleteAllIcons", Buddy_trayiconXtra::m_btiDeleteAllIcons, 0, 0, 500, HBLTIN },
	{ "btiRestoreProgram", Buddy_trayiconXtra::m_btiRestoreProgram, 0, 0, 500, HBLTIN },
	{ "btiMinimiseProgram", Buddy_trayiconXtra::m_btiMinimiseProgram, 0, 0, 500, HBLTIN },
	{ "btiHideProgram", Buddy_trayiconXtra::m_btiHideProgram, 0, 0, 500, HBLTIN },
	{ "btiShowMenu", Buddy_trayiconXtra::m_btiShowMenu, 2, 2, 500, HBLTIN },
	{ "btiAbout", Buddy_trayiconXtra::m_btiAbout, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

Buddy_trayiconXtraObject::Buddy_trayiconXtraObject(ObjectType ObjectType) :Object<Buddy_trayiconXtraObject>("Buddy_trayicon") {
	_objType = ObjectType;
}

bool Buddy_trayiconXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum Buddy_trayiconXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(Buddy_trayiconXtra::xlibName);
	warning("Buddy_trayiconXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void Buddy_trayiconXtra::open(ObjectType type, const Common::Path &path) {
    Buddy_trayiconXtraObject::initMethods(xlibMethods);
    Buddy_trayiconXtraObject *xobj = new Buddy_trayiconXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void Buddy_trayiconXtra::close(ObjectType type) {
    Buddy_trayiconXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void Buddy_trayiconXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("Buddy_trayiconXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(Buddy_trayiconXtra::m_btiCreateTrayIcon, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiModifyTrayIcon, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiDeleteTrayIcon, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiDeleteAllIcons, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiRestoreProgram, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiMinimiseProgram, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiHideProgram, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiShowMenu, 0)
XOBJSTUB(Buddy_trayiconXtra::m_btiAbout, 0)
XOBJSTUB(Buddy_trayiconXtra::m_", 0)

}
