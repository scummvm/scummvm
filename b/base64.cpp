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
#include "director/lingo/xtras/b/base64.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra Base64 -- This xtra performs base64 encoding & decoding of arbitrary binary strings into text strings that can be safely emailed or posted. The encoding scheme is defined in RFC1521.
-- (c) 2001 by: David Hu
new object me
*base64register string registercode -- Register Base64 Xtra
base64encode object me, string input, string output -- perform base64 encode
base64decode object me, string input, string output -- perform base64 decode
"eckdx object me -- check if DirectX is installed
"
 */

namespace Director {

const char *Base64Xtra::xlibName = "Base64";
const XlibFileDesc Base64Xtra::fileNames[] = {
	{ "base64",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				Base64Xtra::m_new,		 0, 0,	700 },
	{ "base64encode",				Base64Xtra::m_base64encode,		 2, 2,	700 },
	{ "base64decode",				Base64Xtra::m_base64decode,		 2, 2,	700 },
	{ ""eckdx",				Base64Xtra::m_"eckdx,		 0, 0,	700 },
	{ """,				Base64Xtra::m_",		 -1, -1,	700 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "base64register", Base64Xtra::m_base64register, 1, 1, 700, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

Base64XtraObject::Base64XtraObject(ObjectType ObjectType) :Object<Base64XtraObject>("Base64") {
	_objType = ObjectType;
}

bool Base64XtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum Base64XtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(Base64Xtra::xlibName);
	warning("Base64Xtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void Base64Xtra::open(ObjectType type, const Common::Path &path) {
    Base64XtraObject::initMethods(xlibMethods);
    Base64XtraObject *xobj = new Base64XtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void Base64Xtra::close(ObjectType type) {
    Base64XtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void Base64Xtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("Base64Xtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(Base64Xtra::m_base64register, 0)
XOBJSTUB(Base64Xtra::m_base64encode, 0)
XOBJSTUB(Base64Xtra::m_base64decode, 0)
XOBJSTUB(Base64Xtra::m_"eckdx, 0)
XOBJSTUB(Base64Xtra::m_", 0)

}
