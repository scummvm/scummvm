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
#include "director/lingo/xtras/b/badeleteini.h"

/**************************************************
 *
 * USED IN:
 * Charlie Church Mouse - Early Elementary
 *
 **************************************************/

/*
-- xtra baDeleteIni
new object me

* baDeleteIniEntry string Section, string Keyname, string IniFile -- deletes ini entry
* baDeleteIniSection string Section, string IniFile -- deletes ini section

-- Beta Version
-- Copyright: Gary Smith, 7th February, 1999.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au
"
 */

namespace Director {

const char *BadeleteiniXtra::xlibName = "Badeleteini";
const XlibFileDesc BadeleteiniXtra::fileNames[] = {
	{ "badeleteini",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BadeleteiniXtra::m_new,		 0, 0,	500 },
	{ """,				BadeleteiniXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baDeleteIniEntry", BadeleteiniXtra::m_baDeleteIniEntry, 3, 3, 500, HBLTIN },
	{ "baDeleteIniSection", BadeleteiniXtra::m_baDeleteIniSection, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BadeleteiniXtraObject::BadeleteiniXtraObject(ObjectType ObjectType) :Object<BadeleteiniXtraObject>("Badeleteini") {
	_objType = ObjectType;
}

bool BadeleteiniXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BadeleteiniXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BadeleteiniXtra::xlibName);
	warning("BadeleteiniXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BadeleteiniXtra::open(ObjectType type, const Common::Path &path) {
    BadeleteiniXtraObject::initMethods(xlibMethods);
    BadeleteiniXtraObject *xobj = new BadeleteiniXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BadeleteiniXtra::close(ObjectType type) {
    BadeleteiniXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BadeleteiniXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BadeleteiniXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BadeleteiniXtra::m_baDeleteIniEntry, 0)
XOBJSTUB(BadeleteiniXtra::m_baDeleteIniSection, 0)
XOBJSTUB(BadeleteiniXtra::m_", 0)

}
