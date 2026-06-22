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
#include "director/lingo/xtras/b/basetdisplayex.h"

/**************************************************
 *
 * USED IN:
 * Charlie Church Mouse - Early Elementary
 * Walking With Beasts: Operation Salvage
 *
 **************************************************/

/*
-- xtra baSetDisplayEx
new object me

* baSetDisplayEx integer Width, integer Height, integer Depth, integer Frequency, string Mode, integer Force -- changes screen settings
* baScreenFrequency -- returns screen frequency

-- Beta Version
-- Copyright: Gary Smith, 28 November, 1999.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au
"
 */

namespace Director {

const char *BasetdisplayexXtra::xlibName = "Basetdisplayex";
const XlibFileDesc BasetdisplayexXtra::fileNames[] = {
	{ "basetdisplayex",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BasetdisplayexXtra::m_new,		 0, 0,	500 },
	{ """,				BasetdisplayexXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baSetDisplayEx", BasetdisplayexXtra::m_baSetDisplayEx, 6, 6, 500, HBLTIN },
	{ "baScreenFrequency", BasetdisplayexXtra::m_baScreenFrequency, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BasetdisplayexXtraObject::BasetdisplayexXtraObject(ObjectType ObjectType) :Object<BasetdisplayexXtraObject>("Basetdisplayex") {
	_objType = ObjectType;
}

bool BasetdisplayexXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BasetdisplayexXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BasetdisplayexXtra::xlibName);
	warning("BasetdisplayexXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BasetdisplayexXtra::open(ObjectType type, const Common::Path &path) {
    BasetdisplayexXtraObject::initMethods(xlibMethods);
    BasetdisplayexXtraObject *xobj = new BasetdisplayexXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BasetdisplayexXtra::close(ObjectType type) {
    BasetdisplayexXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BasetdisplayexXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BasetdisplayexXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BasetdisplayexXtra::m_baSetDisplayEx, 0)
XOBJSTUB(BasetdisplayexXtra::m_baScreenFrequency, 0)
XOBJSTUB(BasetdisplayexXtra::m_", 0)

}
