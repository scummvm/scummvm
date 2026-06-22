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
#include "director/lingo/xtras/b/baregbinary.h"

/**************************************************
 *
 * USED IN:
 * Charlie Church Mouse - Early Elementary
 *
 **************************************************/

/*
-- xtra baRegBinary
new object me

* baReadRegBinary string Key, string Value, string Default, string Branch -- reads registry value
* baWriteRegBinary string Key, string Value, list Data, string Branch -- writes registry value

-- Beta Version
-- Copyright: Gary Smith, 18th December, 1998.
-- Email: gary@mods.com.au
-- Web site: http://www.mods.com.au
"
 */

namespace Director {

const char *BaregbinaryXtra::xlibName = "Baregbinary";
const XlibFileDesc BaregbinaryXtra::fileNames[] = {
	{ "baregbinary",   nullptr },
	{ "baregbin",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BaregbinaryXtra::m_new,		 0, 0,	500 },
	{ """,				BaregbinaryXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baReadRegBinary", BaregbinaryXtra::m_baReadRegBinary, 4, 4, 500, HBLTIN },
	{ "baWriteRegBinary", BaregbinaryXtra::m_baWriteRegBinary, 4, 4, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BaregbinaryXtraObject::BaregbinaryXtraObject(ObjectType ObjectType) :Object<BaregbinaryXtraObject>("Baregbinary") {
	_objType = ObjectType;
}

bool BaregbinaryXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BaregbinaryXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BaregbinaryXtra::xlibName);
	warning("BaregbinaryXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BaregbinaryXtra::open(ObjectType type, const Common::Path &path) {
    BaregbinaryXtraObject::initMethods(xlibMethods);
    BaregbinaryXtraObject *xobj = new BaregbinaryXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BaregbinaryXtra::close(ObjectType type) {
    BaregbinaryXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BaregbinaryXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BaregbinaryXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BaregbinaryXtra::m_baReadRegBinary, 0)
XOBJSTUB(BaregbinaryXtra::m_baWriteRegBinary, 0)
XOBJSTUB(BaregbinaryXtra::m_", 0)

}
