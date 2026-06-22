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
#include "director/lingo/xtras/amsystem.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra AMSystem
new object me
-- AMSystem --
+ loopBack object xtraRef, string -- returns a string
+ getValue object xtraRef, string, string, string -- reads in a string
+ setCursor object xtraRef, int, int -- moves the mouse
+ status object xtraRef -- returns error code of last call
+ error object xtraRef, integer -- returns the error code as a string

 */

namespace Director {

const char *AmsystemXtra::xlibName = "Amsystem";
const XlibFileDesc AmsystemXtra::fileNames[] = {
	{ "amsystem",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AmsystemXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AmsystemXtraObject::AmsystemXtraObject(ObjectType ObjectType) :Object<AmsystemXtraObject>("Amsystem") {
	_objType = ObjectType;
}

bool AmsystemXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AmsystemXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AmsystemXtra::xlibName);
	warning("AmsystemXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AmsystemXtra::open(ObjectType type, const Common::Path &path) {
    AmsystemXtraObject::initMethods(xlibMethods);
    AmsystemXtraObject *xobj = new AmsystemXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AmsystemXtra::close(ObjectType type) {
    AmsystemXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AmsystemXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AmsystemXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AmsystemXtra::m_loopBack, 0)
XOBJSTUB(AmsystemXtra::m_getValue, 0)
XOBJSTUB(AmsystemXtra::m_setCursor, 0)
XOBJSTUB(AmsystemXtra::m_status, 0)
XOBJSTUB(AmsystemXtra::m_error, 0)

}
