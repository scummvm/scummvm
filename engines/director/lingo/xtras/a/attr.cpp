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
#include "director/lingo/xtras/a/attr.h"

/**************************************************
 *
 * USED IN:
 * Crayola Paint ’n Play Pony
 * Mary-Kate and Ashley's Dance Party of the Century
 *
 **************************************************/

/*
-- xtra attr -- This Xtra makes it easy to set your files to be read-only and not read-only with Lingo.
new object me
-- Attr Xtra --
-- copyright (c) 1997 codeHorse, all rights reserved.--
+ setReadOnly  object xtraRef, string fs -- 
+ setReadWrite object xtraRef, string fs -- 

 */

namespace Director {

const char *AttrXtra::xlibName = "Attr";
const XlibFileDesc AttrXtra::fileNames[] = {
	{ "attr",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AttrXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AttrXtraObject::AttrXtraObject(ObjectType ObjectType) :Object<AttrXtraObject>("Attr") {
	_objType = ObjectType;
}

bool AttrXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AttrXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AttrXtra::xlibName);
	warning("AttrXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AttrXtra::open(ObjectType type, const Common::Path &path) {
    AttrXtraObject::initMethods(xlibMethods);
    AttrXtraObject *xobj = new AttrXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AttrXtra::close(ObjectType type) {
    AttrXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AttrXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AttrXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AttrXtra::m_setReadOnly, 0)
XOBJSTUB(AttrXtra::m_setReadWrite, 0)

}
