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
#include "director/lingo/xtras/f/fileflex.h"

/**************************************************
 *
 * USED IN:
 * getaheadmath
 *
 **************************************************/

/*
-- xtra FileFlex
new object me
forget object me
getv object me --> any
setv object me, any object
-- FFXtra --
*FileFlex * -- FileFlex entry call
 */

namespace Director {

const char *FileFlexXtra::xlibName = "FileFlex";
const XlibFileDesc FileFlexXtra::fileNames[] = {
	{ "fileflex",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				FileFlexXtra::m_new,		 0, 0,	500 },
	{ "forget",				FileFlexXtra::m_forget,		 0, 0,	500 },
	{ "getv",				FileFlexXtra::m_getv,		 0, 0,	500 },
	{ "setv",				FileFlexXtra::m_setv,		 1, 1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "FileFlex", FileFlexXtra::m_FileFlex, -1, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FileFlexXtraObject::FileFlexXtraObject(ObjectType ObjectType) :Object<FileFlexXtraObject>("FileFlex") {
	_objType = ObjectType;
}

bool FileFlexXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FileFlexXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FileFlexXtra::xlibName);
	warning("FileFlexXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void FileFlexXtra::open(ObjectType type, const Common::Path &path) {
    FileFlexXtraObject::initMethods(xlibMethods);
    FileFlexXtraObject *xobj = new FileFlexXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
        g_lingo->_openXtraObjects.push_back(xobj);
    }
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FileFlexXtra::close(ObjectType type) {
    FileFlexXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FileFlexXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FileFlexXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(FileFlexXtra::m_forget, 0)
XOBJSTUB(FileFlexXtra::m_getv, 0)
XOBJSTUB(FileFlexXtra::m_setv, 0)
XOBJSTUB(FileFlexXtra::m_FileFlex, 0)

}
