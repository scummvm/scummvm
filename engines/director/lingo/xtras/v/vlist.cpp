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
#include "director/lingo/xtras/v/vlist.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 8 (D8.5)
 *
 **************************************************/

/*
-- xtra vList -- Save and Read on disk Lingo values in binary format
-- � 2001 Daniel Devolder. Version 1.5.14 (128 bits encryption) --
-- For updates go to: http://www.updateStage.com/xtras/vlist.html
-- GENERAL GLOBAL METHODS --
* vList_register string username, list keycodes
* vList_version
* vList_error -- Last error code as integer.
* vList_errorString * errorCode -- Last error as a small string.
* vList_checkMedia any vListMedia, * encryptKeyList -- check member(#vList).media variable.
* vList_readFromMedia any vListMedia, * encryptKeyList -- returns the content of a member(#vList).media variable.
-- FILE ACCESS --
new object me, string fileName, * relativePath --
compression object me, integer type
write object me, any LingoValue, * encryptKeyList -- write to a vList file. Optional encryption and compression.
read object me, * encryptKeyList -- read from a vList file. Optional encryption and compression.
fileExist object me, * encryptKeyList
deleteFile object me
 */

namespace Director {

const char *VListXtra::xlibName = "VList";
const XlibFileDesc VListXtra::fileNames[] = {
	{ "vlist",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				VListXtra::m_new,		 2, 2,	500 },
	{ "compression",				VListXtra::m_compression,		 1, 1,	500 },
	{ "write",				VListXtra::m_write,		 2, 2,	500 },
	{ "read",				VListXtra::m_read,		 1, 1,	500 },
	{ "fileExist",				VListXtra::m_fileExist,		 1, 1,	500 },
	{ "deleteFile",				VListXtra::m_deleteFile,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "vList_register", VListXtra::m_vList_register, 2, 2, 500, HBLTIN },
	{ "vList_version", VListXtra::m_vList_version, 0, 0, 500, HBLTIN },
	{ "vList_error", VListXtra::m_vList_error, 0, 0, 500, HBLTIN },
	{ "vList_errorString", VListXtra::m_vList_errorString, 1, 1, 500, HBLTIN },
	{ "vList_checkMedia", VListXtra::m_vList_checkMedia, 2, 2, 500, HBLTIN },
	{ "vList_readFromMedia", VListXtra::m_vList_readFromMedia, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

VListXtraObject::VListXtraObject(ObjectType ObjectType) :Object<VListXtraObject>("VList") {
	_objType = ObjectType;
}

bool VListXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum VListXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(VListXtra::xlibName);
	warning("VListXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void VListXtra::open(ObjectType type, const Common::Path &path) {
    VListXtraObject::initMethods(xlibMethods);
    VListXtraObject *xobj = new VListXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void VListXtra::close(ObjectType type) {
    VListXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void VListXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("VListXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(VListXtra::m_vList_register, 0)
XOBJSTUB(VListXtra::m_vList_version, 0)
XOBJSTUB(VListXtra::m_vList_error, 0)
XOBJSTUB(VListXtra::m_vList_errorString, 0)
XOBJSTUB(VListXtra::m_vList_checkMedia, 0)
XOBJSTUB(VListXtra::m_vList_readFromMedia, 0)
XOBJSTUB(VListXtra::m_compression, 0)
XOBJSTUB(VListXtra::m_write, 0)
XOBJSTUB(VListXtra::m_read, 0)
XOBJSTUB(VListXtra::m_fileExist, 0)
XOBJSTUB(VListXtra::m_deleteFile, 0)

}
