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
#include "director/lingo/xtras/bruce.h"

/**************************************************
 *
 * USED IN:
 * James Halliday Interactive Wine Companion: 1999 Edition
 *
 **************************************************/

/*
-- xtra Bruce
new object me
-- Global Bruce Routines: --
* BruceCreate string defaultPath, string configFilename -- 
* BruceDestroy integer hConfig -- 
* BruceUseIndex integer hConfig, string indexName -- 
* BruceFind integer hConfig, string indexName, string searchText -- 
* BruceFindNext integer hConfig -- 
* BruceFindPrev integer hConfig -- 
* BruceFindAll integer hConfig, string indexName, string searchText -- 
* BruceFindTeledex integer hConfig, string indexName, string startText -- 
* BruceSeek integer hConfig, string fPos -- 
* BruceGet integer hConfig, string fieldName -- 
* BruceGetChoices integer hConfig, string fPosList, string indexName -- 
* BruceGetAll integer hConfig, string fPosList, string fieldName -- 
* BruceGetDataPos integer hConfig -- 
* BruceSetDataPos integer hConfig, string fPos -- 
* BruceGetIndexPos integer hConfig -- 
* BruceSetIndexPos integer hConfig, string fPos -- 
* BruceNextRec integer hConfig -- 
* BruceSetPath integer hConfig, string thePath -- 

 */

namespace Director {

const char *BruceXtra::xlibName = "Bruce";
const XlibFileDesc BruceXtra::fileNames[] = {
	{ "bruce",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BruceXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "BruceCreate", BruceXtra::m_BruceCreate, 2, 2, 500, HBLTIN },
	{ "BruceDestroy", BruceXtra::m_BruceDestroy, 1, 1, 500, HBLTIN },
	{ "BruceUseIndex", BruceXtra::m_BruceUseIndex, 2, 2, 500, HBLTIN },
	{ "BruceFind", BruceXtra::m_BruceFind, 3, 3, 500, HBLTIN },
	{ "BruceFindNext", BruceXtra::m_BruceFindNext, 1, 1, 500, HBLTIN },
	{ "BruceFindPrev", BruceXtra::m_BruceFindPrev, 1, 1, 500, HBLTIN },
	{ "BruceFindAll", BruceXtra::m_BruceFindAll, 3, 3, 500, HBLTIN },
	{ "BruceFindTeledex", BruceXtra::m_BruceFindTeledex, 3, 3, 500, HBLTIN },
	{ "BruceSeek", BruceXtra::m_BruceSeek, 2, 2, 500, HBLTIN },
	{ "BruceGet", BruceXtra::m_BruceGet, 2, 2, 500, HBLTIN },
	{ "BruceGetChoices", BruceXtra::m_BruceGetChoices, 3, 3, 500, HBLTIN },
	{ "BruceGetAll", BruceXtra::m_BruceGetAll, 3, 3, 500, HBLTIN },
	{ "BruceGetDataPos", BruceXtra::m_BruceGetDataPos, 1, 1, 500, HBLTIN },
	{ "BruceSetDataPos", BruceXtra::m_BruceSetDataPos, 2, 2, 500, HBLTIN },
	{ "BruceGetIndexPos", BruceXtra::m_BruceGetIndexPos, 1, 1, 500, HBLTIN },
	{ "BruceSetIndexPos", BruceXtra::m_BruceSetIndexPos, 2, 2, 500, HBLTIN },
	{ "BruceNextRec", BruceXtra::m_BruceNextRec, 1, 1, 500, HBLTIN },
	{ "BruceSetPath", BruceXtra::m_BruceSetPath, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BruceXtraObject::BruceXtraObject(ObjectType ObjectType) :Object<BruceXtraObject>("Bruce") {
	_objType = ObjectType;
}

bool BruceXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BruceXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BruceXtra::xlibName);
	warning("BruceXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BruceXtra::open(ObjectType type, const Common::Path &path) {
    BruceXtraObject::initMethods(xlibMethods);
    BruceXtraObject *xobj = new BruceXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BruceXtra::close(ObjectType type) {
    BruceXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BruceXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BruceXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BruceXtra::m_BruceCreate, 0)
XOBJSTUB(BruceXtra::m_BruceDestroy, 0)
XOBJSTUB(BruceXtra::m_BruceUseIndex, 0)
XOBJSTUB(BruceXtra::m_BruceFind, 0)
XOBJSTUB(BruceXtra::m_BruceFindNext, 0)
XOBJSTUB(BruceXtra::m_BruceFindPrev, 0)
XOBJSTUB(BruceXtra::m_BruceFindAll, 0)
XOBJSTUB(BruceXtra::m_BruceFindTeledex, 0)
XOBJSTUB(BruceXtra::m_BruceSeek, 0)
XOBJSTUB(BruceXtra::m_BruceGet, 0)
XOBJSTUB(BruceXtra::m_BruceGetChoices, 0)
XOBJSTUB(BruceXtra::m_BruceGetAll, 0)
XOBJSTUB(BruceXtra::m_BruceGetDataPos, 0)
XOBJSTUB(BruceXtra::m_BruceSetDataPos, 0)
XOBJSTUB(BruceXtra::m_BruceGetIndexPos, 0)
XOBJSTUB(BruceXtra::m_BruceSetIndexPos, 0)
XOBJSTUB(BruceXtra::m_BruceNextRec, 0)
XOBJSTUB(BruceXtra::m_BruceSetPath, 0)

}
