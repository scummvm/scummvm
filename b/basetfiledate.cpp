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
#include "director/lingo/xtras/b/basetfiledate.h"

/**************************************************
 *
 * USED IN:
 * Charlie Church Mouse - Early Elementary
 *
 **************************************************/

/*
-- xtra baSetFileDate
new object me

* baSetFileDate string FileName, integer Year, integer Month, integer Day, integer Hour, integer Minute, integer Second -- sets date and time of file

-- Beta Version
-- Copyright: Gary Smith, 7th February, 1999.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au

"
 */

namespace Director {

const char *BasetfiledateXtra::xlibName = "Basetfiledate";
const XlibFileDesc BasetfiledateXtra::fileNames[] = {
	{ "basetfiledate",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BasetfiledateXtra::m_new,		 0, 0,	500 },
	{ """,				BasetfiledateXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baSetFileDate", BasetfiledateXtra::m_baSetFileDate, 7, 7, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BasetfiledateXtraObject::BasetfiledateXtraObject(ObjectType ObjectType) :Object<BasetfiledateXtraObject>("Basetfiledate") {
	_objType = ObjectType;
}

bool BasetfiledateXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BasetfiledateXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BasetfiledateXtra::xlibName);
	warning("BasetfiledateXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BasetfiledateXtra::open(ObjectType type, const Common::Path &path) {
    BasetfiledateXtraObject::initMethods(xlibMethods);
    BasetfiledateXtraObject *xobj = new BasetfiledateXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BasetfiledateXtra::close(ObjectType type) {
    BasetfiledateXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BasetfiledateXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BasetfiledateXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BasetfiledateXtra::m_baSetFileDate, 0)
XOBJSTUB(BasetfiledateXtra::m_", 0)

}
