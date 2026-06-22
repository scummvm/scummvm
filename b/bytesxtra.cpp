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
#include "director/lingo/xtras/b/bytesxtra.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BytesXtra
-- Version 1.0
-- https://schmittmachine.com
* newBytes integer size -- creates a bytes object of given size
* newBytesFromString string str -- converts a string (including trailing 0) to a bytes object 
-- bytes.size -- get and set the size
-- bytes[int index] -- get and set value
-- bytes.data[int index1..int index2] -- get and set values
-- bytes.toString() -- creates a string from a bytes object (must be 0 terminated)
-- bytes.offset(bytes2 [,int startIndex]) -- returns the first index (after startIndex if present, default 1) of bytes2 in aBytes, or 0 if not found
-- bytes.copyFrom(bytes srcBytes, int bytesCount, int srcOffset, int dstOffset) -- copies bytesCount bytes from srcOffset in srcBytes into bytes starting at dstOffset
"
 */

namespace Director {

const char *BytesxtraXtra::xlibName = "Bytesxtra";
const XlibFileDesc BytesxtraXtra::fileNames[] = {
	{ "bytesxtra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ """,				BytesxtraXtra::m_",		 -1, -1,	900 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "newBytes", BytesxtraXtra::m_newBytes, 1, 1, 900, HBLTIN },
	{ "newBytesFromString", BytesxtraXtra::m_newBytesFromString, 1, 1, 900, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BytesxtraXtraObject::BytesxtraXtraObject(ObjectType ObjectType) :Object<BytesxtraXtraObject>("Bytesxtra") {
	_objType = ObjectType;
}

bool BytesxtraXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BytesxtraXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BytesxtraXtra::xlibName);
	warning("BytesxtraXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BytesxtraXtra::open(ObjectType type, const Common::Path &path) {
    BytesxtraXtraObject::initMethods(xlibMethods);
    BytesxtraXtraObject *xobj = new BytesxtraXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BytesxtraXtra::close(ObjectType type) {
    BytesxtraXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BytesxtraXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BytesxtraXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BytesxtraXtra::m_newBytes, 0)
XOBJSTUB(BytesxtraXtra::m_newBytesFromString, 0)
XOBJSTUB(BytesxtraXtra::m_", 0)

}
