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
#include "director/lingo/xtras/b/binfile.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BinFile
-- BinFile Xtra v0.2 (c) 2006 Valentin Schmidt
-- contact: fluxus@freenet.de

new object me
bx_fopen object me, string filename, string mode,*
bx_fclose object me
bx_fread object me, integer n
bx_fwrite object me, string str, *
bx_ftell object me
bx_fseek object me, integer pos, *
bx_fsize object me
*bx_display_open * -- string title, string filterMask, string defaultFileName
*bx_display_save * -- string title, string filterMask, string defaultFileName
*bx_file_get_contents string filename
*bx_file_put_contents string filename, string data, *
*bx_file_size string filename
*bx_file_delete string filename
*bx_file_exists string filename
*bx_file_truncate string filename, integer size"

"
 */

namespace Director {

const char *BinfileXtra::xlibName = "Binfile";
const XlibFileDesc BinfileXtra::fileNames[] = {
	{ "binfile",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BinfileXtra::m_new,		 0, 0,	800 },
	{ "bx_fopen",				BinfileXtra::m_bx_fopen,		 -1, 0,	800 },
	{ "bx_fclose",				BinfileXtra::m_bx_fclose,		 0, 0,	800 },
	{ "bx_fread",				BinfileXtra::m_bx_fread,		 1, 1,	800 },
	{ "bx_fwrite",				BinfileXtra::m_bx_fwrite,		 -1, 0,	800 },
	{ "bx_ftell",				BinfileXtra::m_bx_ftell,		 0, 0,	800 },
	{ "bx_fseek",				BinfileXtra::m_bx_fseek,		 -1, 0,	800 },
	{ "bx_fsize",				BinfileXtra::m_bx_fsize,		 0, 0,	800 },
	{ """,				BinfileXtra::m_",		 -1, -1,	800 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "bx_display_open", BinfileXtra::m_bx_display_open, -1, 0, 800, HBLTIN },
	{ "bx_display_save", BinfileXtra::m_bx_display_save, -1, 0, 800, HBLTIN },
	{ "bx_file_get_contents", BinfileXtra::m_bx_file_get_contents, 1, 1, 800, HBLTIN },
	{ "bx_file_put_contents", BinfileXtra::m_bx_file_put_contents, -1, 0, 800, HBLTIN },
	{ "bx_file_size", BinfileXtra::m_bx_file_size, 1, 1, 800, HBLTIN },
	{ "bx_file_delete", BinfileXtra::m_bx_file_delete, 1, 1, 800, HBLTIN },
	{ "bx_file_exists", BinfileXtra::m_bx_file_exists, 1, 1, 800, HBLTIN },
	{ "bx_file_truncate", BinfileXtra::m_bx_file_truncate, 2, 2, 800, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BinfileXtraObject::BinfileXtraObject(ObjectType ObjectType) :Object<BinfileXtraObject>("Binfile") {
	_objType = ObjectType;
}

bool BinfileXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BinfileXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BinfileXtra::xlibName);
	warning("BinfileXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BinfileXtra::open(ObjectType type, const Common::Path &path) {
    BinfileXtraObject::initMethods(xlibMethods);
    BinfileXtraObject *xobj = new BinfileXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BinfileXtra::close(ObjectType type) {
    BinfileXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BinfileXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BinfileXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BinfileXtra::m_bx_fopen, 0)
XOBJSTUB(BinfileXtra::m_bx_fclose, 0)
XOBJSTUB(BinfileXtra::m_bx_fread, 0)
XOBJSTUB(BinfileXtra::m_bx_fwrite, 0)
XOBJSTUB(BinfileXtra::m_bx_ftell, 0)
XOBJSTUB(BinfileXtra::m_bx_fseek, 0)
XOBJSTUB(BinfileXtra::m_bx_fsize, 0)
XOBJSTUB(BinfileXtra::m_bx_display_open, 0)
XOBJSTUB(BinfileXtra::m_bx_display_save, 0)
XOBJSTUB(BinfileXtra::m_bx_file_get_contents, 0)
XOBJSTUB(BinfileXtra::m_bx_file_put_contents, 0)
XOBJSTUB(BinfileXtra::m_bx_file_size, 0)
XOBJSTUB(BinfileXtra::m_bx_file_delete, 0)
XOBJSTUB(BinfileXtra::m_bx_file_exists, 0)
XOBJSTUB(BinfileXtra::m_bx_file_truncate, 0)
XOBJSTUB(BinfileXtra::m_", 0)

}
