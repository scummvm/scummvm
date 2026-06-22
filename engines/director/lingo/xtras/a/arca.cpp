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
#include "director/lingo/xtras/a/arca.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra Arca -- version 3.0.0
-- http://xtras.tabuleiro.com/
new object me
-- xtra handlers --
createdb object me, string database, * encryptionkey -- create a database file with optional encryption
opendb object me, string database, * encryptionkey -- open a database file
closedb object me -- close a database file
compactdb object me -- compact the database file
getdbschema object me -- return a list describing the database schema
executesql object me, string sql, * optionalarglist -- executes sql statement
createselection object me, string sql, * optionalarglist -- execute sql statement and return a selection
getrows object me, integer selectionid, integer firstrow, integer lastrow -- retrieve rows from selection
getfield object me, integer selectionid, integer rownumber, string fieldname -- retrieve value of the named field in the specified selection and row
getlastinsertrowid object me -- retrieve the value of the last rowid inserted in the database
freeselection object me, integer selectionid -- free memory allocated to a selection
*arcaregister list registrationid -- register the xtra
explainerror object me, integer errorcode -- return a text explanation of the error message
changekey object me, string encryptionkey -- change the database encryption key
setencoding object me, symbol side, symbol encoding -- change the client or database encoding used by the Xtra to communicate with Lingo
"
 */

namespace Director {

const char *ArcaXtra::xlibName = "Arca";
const XlibFileDesc ArcaXtra::fileNames[] = {
	{ "arca",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ArcaXtra::m_new,		 0, 0,	850 },
	{ "createdb",				ArcaXtra::m_createdb,		 2, 2,	850 },
	{ "opendb",				ArcaXtra::m_opendb,		 2, 2,	850 },
	{ "closedb",				ArcaXtra::m_closedb,		 0, 0,	850 },
	{ "compactdb",				ArcaXtra::m_compactdb,		 0, 0,	850 },
	{ "getdbschema",				ArcaXtra::m_getdbschema,		 0, 0,	850 },
	{ "executesql",				ArcaXtra::m_executesql,		 2, 2,	850 },
	{ "createselection",				ArcaXtra::m_createselection,		 2, 2,	850 },
	{ "getrows",				ArcaXtra::m_getrows,		 3, 3,	850 },
	{ "getfield",				ArcaXtra::m_getfield,		 3, 3,	850 },
	{ "getlastinsertrowid",				ArcaXtra::m_getlastinsertrowid,		 0, 0,	850 },
	{ "freeselection",				ArcaXtra::m_freeselection,		 1, 1,	850 },
	{ "explainerror",				ArcaXtra::m_explainerror,		 1, 1,	850 },
	{ "changekey",				ArcaXtra::m_changekey,		 1, 1,	850 },
	{ "setencoding",				ArcaXtra::m_setencoding,		 2, 2,	850 },
	{ """,				ArcaXtra::m_",		 -1, -1,	850 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "arcaregister", ArcaXtra::m_arcaregister, 1, 1, 850, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ArcaXtraObject::ArcaXtraObject(ObjectType ObjectType) :Object<ArcaXtraObject>("Arca") {
	_objType = ObjectType;
}

bool ArcaXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum ArcaXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(ArcaXtra::xlibName);
	warning("ArcaXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void ArcaXtra::open(ObjectType type, const Common::Path &path) {
    ArcaXtraObject::initMethods(xlibMethods);
    ArcaXtraObject *xobj = new ArcaXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ArcaXtra::close(ObjectType type) {
    ArcaXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ArcaXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ArcaXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ArcaXtra::m_createdb, 0)
XOBJSTUB(ArcaXtra::m_opendb, 0)
XOBJSTUB(ArcaXtra::m_closedb, 0)
XOBJSTUB(ArcaXtra::m_compactdb, 0)
XOBJSTUB(ArcaXtra::m_getdbschema, 0)
XOBJSTUB(ArcaXtra::m_executesql, 0)
XOBJSTUB(ArcaXtra::m_createselection, 0)
XOBJSTUB(ArcaXtra::m_getrows, 0)
XOBJSTUB(ArcaXtra::m_getfield, 0)
XOBJSTUB(ArcaXtra::m_getlastinsertrowid, 0)
XOBJSTUB(ArcaXtra::m_freeselection, 0)
XOBJSTUB(ArcaXtra::m_arcaregister, 0)
XOBJSTUB(ArcaXtra::m_explainerror, 0)
XOBJSTUB(ArcaXtra::m_changekey, 0)
XOBJSTUB(ArcaXtra::m_setencoding, 0)
XOBJSTUB(ArcaXtra::m_", 0)

}
