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
#include "director/lingo/xtras/b/blurimage.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BlurImage (Smoothware Design) -- this Xtra blurs cast members 
new object me
Blur object me, integer sourceMember, integer destinationMember, integer A1,integer A2,integer A3,integer B1,integer B2,integer B3,integer C1,integer C2,integer C3,integer times -- Blur cast member (cast#1)
"
 */

namespace Director {

const char *BlurimageXtra::xlibName = "Blurimage";
const XlibFileDesc BlurimageXtra::fileNames[] = {
	{ "blurimage",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BlurimageXtra::m_new,		 0, 0,	500 },
	{ "Blur",				BlurimageXtra::m_Blur,		 12, 12,	500 },
	{ """,				BlurimageXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BlurimageXtraObject::BlurimageXtraObject(ObjectType ObjectType) :Object<BlurimageXtraObject>("Blurimage") {
	_objType = ObjectType;
}

bool BlurimageXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BlurimageXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BlurimageXtra::xlibName);
	warning("BlurimageXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BlurimageXtra::open(ObjectType type, const Common::Path &path) {
    BlurimageXtraObject::initMethods(xlibMethods);
    BlurimageXtraObject *xobj = new BlurimageXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BlurimageXtra::close(ObjectType type) {
    BlurimageXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BlurimageXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BlurimageXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BlurimageXtra::m_Blur, 0)
XOBJSTUB(BlurimageXtra::m_", 0)

}
