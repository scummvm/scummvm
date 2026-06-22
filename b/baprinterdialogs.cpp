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
#include "director/lingo/xtras/b/baprinterdialogs.h"

/**************************************************
 *
 * USED IN:
 * Charlie Church Mouse - Early Elementary
 *
 **************************************************/

/*
-- xtra baPrinterDialogs
new object me

* baPrintDlg integer Flags -- shows printer dialog
* baPageSetupDlg integer Flags -- shows page setup dialog

-- copyright Gary Smith 2000
-- Email: gary@mods.com.au
-- Web: www.mods.com.au
"
 */

namespace Director {

const char *BaprinterdialogsXtra::xlibName = "Baprinterdialogs";
const XlibFileDesc BaprinterdialogsXtra::fileNames[] = {
	{ "baprinterdialogs",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BaprinterdialogsXtra::m_new,		 0, 0,	500 },
	{ """,				BaprinterdialogsXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baPrintDlg", BaprinterdialogsXtra::m_baPrintDlg, 1, 1, 500, HBLTIN },
	{ "baPageSetupDlg", BaprinterdialogsXtra::m_baPageSetupDlg, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BaprinterdialogsXtraObject::BaprinterdialogsXtraObject(ObjectType ObjectType) :Object<BaprinterdialogsXtraObject>("Baprinterdialogs") {
	_objType = ObjectType;
}

bool BaprinterdialogsXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BaprinterdialogsXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BaprinterdialogsXtra::xlibName);
	warning("BaprinterdialogsXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BaprinterdialogsXtra::open(ObjectType type, const Common::Path &path) {
    BaprinterdialogsXtraObject::initMethods(xlibMethods);
    BaprinterdialogsXtraObject *xobj = new BaprinterdialogsXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BaprinterdialogsXtra::close(ObjectType type) {
    BaprinterdialogsXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BaprinterdialogsXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BaprinterdialogsXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BaprinterdialogsXtra::m_baPrintDlg, 0)
XOBJSTUB(BaprinterdialogsXtra::m_baPageSetupDlg, 0)
XOBJSTUB(BaprinterdialogsXtra::m_", 0)

}
