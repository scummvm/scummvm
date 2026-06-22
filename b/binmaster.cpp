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
#include "director/lingo/xtras/b/binmaster.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BinMaster  -- 	This xtra allows Director developers to read from the binary file and save data in the binary file, so that anybody cannot read the data.
-- Copyright by Intermedia
-- http://malinowo.win.pl/xtras/opisy/BinMaster/EN/info/BinMasterinfoEn.htm                                                               -- 
new object me 
-- FUNKCJE --
ReadBin object me ,string filename ,long type,long seek ,long sizelem,string serialNumber 
SaveBin object me ,string filename ,long type,long seek,long elem 
"
 */

namespace Director {

const char *BinmasterXtra::xlibName = "Binmaster";
const XlibFileDesc BinmasterXtra::fileNames[] = {
	{ "binmaster",   nullptr },
	{ "binarymaster",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BinmasterXtra::m_new,		 0, 0,	600 },
	{ "ReadBin",				BinmasterXtra::m_ReadBin,		 5, 5,	600 },
	{ "SaveBin",				BinmasterXtra::m_SaveBin,		 4, 4,	600 },
	{ """,				BinmasterXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BinmasterXtraObject::BinmasterXtraObject(ObjectType ObjectType) :Object<BinmasterXtraObject>("Binmaster") {
	_objType = ObjectType;
}

bool BinmasterXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BinmasterXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BinmasterXtra::xlibName);
	warning("BinmasterXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BinmasterXtra::open(ObjectType type, const Common::Path &path) {
    BinmasterXtraObject::initMethods(xlibMethods);
    BinmasterXtraObject *xobj = new BinmasterXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BinmasterXtra::close(ObjectType type) {
    BinmasterXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BinmasterXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BinmasterXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BinmasterXtra::m_ReadBin, 0)
XOBJSTUB(BinmasterXtra::m_SaveBin, 0)
XOBJSTUB(BinmasterXtra::m_", 0)

}
