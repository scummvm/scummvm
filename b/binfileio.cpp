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
#include "director/lingo/xtras/b/binfileio.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra binFileIO, version 8.5.2 -- This Xtra allows binary file input / output from Lingo. It supports most of data types (signed and unsigned short, char and long as well as floating point numbers (float and double)) and can also read or write Lingo integer or float lists or strings.
-- Laurent Cozic
new object me
-- binFileIO handlers --
+ openFile object me, string fileName, integer mode --> file instance or error code
+ closeFile object me --> error code
+ readNumeric object me, symbol dataType --> data
+ writeNumeric object me, symbol dataType, i --> error code
+ readChars object me, integer numberOfChars --> string
+ writeChars object me, string sToWrite, integer numberOfChars --> string
+ writeArray object me, symbol dataType, list listToWrite, integer listSize --> error code
+ readArray object me, symbol dataType, integer arraySize --> list
+ setPosition object me, integer position --> error code
+ getPosition object me --> integer position
+ size object me --> integer fileSize
+ eof object me --> boolean endOfFile
"
 */

namespace Director {

const char *BinfileioXtra::xlibName = "Binfileio";
const XlibFileDesc BinfileioXtra::fileNames[] = {
	{ "binfileio",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BinfileioXtra::m_new,		 0, 0,	600 },
	{ """,				BinfileioXtra::m_",		 -1, -1,	600 },
	{ "openFile",				BinfileioXtra::m_openFile,		 2, 2,	600 },
	{ "closeFile",				BinfileioXtra::m_closeFile,		 0, 0,	600 },
	{ "readNumeric",				BinfileioXtra::m_readNumeric,		 1, 1,	600 },
	{ "writeNumeric",				BinfileioXtra::m_writeNumeric,		 2, 2,	600 },
	{ "readChars",				BinfileioXtra::m_readChars,		 1, 1,	600 },
	{ "writeChars",				BinfileioXtra::m_writeChars,		 2, 2,	600 },
	{ "writeArray",				BinfileioXtra::m_writeArray,		 3, 3,	600 },
	{ "readArray",				BinfileioXtra::m_readArray,		 2, 2,	600 },
	{ "setPosition",				BinfileioXtra::m_setPosition,		 1, 1,	600 },
	{ "getPosition",				BinfileioXtra::m_getPosition,		 0, 0,	600 },
	{ "size",				BinfileioXtra::m_size,		 0, 0,	600 },
	{ "eof",				BinfileioXtra::m_eof,		 0, 0,	600 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BinfileioXtraObject::BinfileioXtraObject(ObjectType ObjectType) :Object<BinfileioXtraObject>("Binfileio") {
	_objType = ObjectType;
}

bool BinfileioXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BinfileioXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BinfileioXtra::xlibName);
	warning("BinfileioXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BinfileioXtra::open(ObjectType type, const Common::Path &path) {
    BinfileioXtraObject::initMethods(xlibMethods);
    BinfileioXtraObject *xobj = new BinfileioXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BinfileioXtra::close(ObjectType type) {
    BinfileioXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BinfileioXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BinfileioXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BinfileioXtra::m_openFile, 0)
XOBJSTUB(BinfileioXtra::m_closeFile, 0)
XOBJSTUB(BinfileioXtra::m_readNumeric, 0)
XOBJSTUB(BinfileioXtra::m_writeNumeric, 0)
XOBJSTUB(BinfileioXtra::m_readChars, 0)
XOBJSTUB(BinfileioXtra::m_writeChars, 0)
XOBJSTUB(BinfileioXtra::m_writeArray, 0)
XOBJSTUB(BinfileioXtra::m_readArray, 0)
XOBJSTUB(BinfileioXtra::m_setPosition, 0)
XOBJSTUB(BinfileioXtra::m_getPosition, 0)
XOBJSTUB(BinfileioXtra::m_size, 0)
XOBJSTUB(BinfileioXtra::m_eof, 0)
XOBJSTUB(BinfileioXtra::m_", 0)

}
