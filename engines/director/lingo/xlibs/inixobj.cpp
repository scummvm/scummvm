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
#include "director/lingo/xlibs/inixobj.h"

/**************************************************
 *
 * USED IN:
 * Karma: Curse of the 12 Caves
 *
 **************************************************/

/*
-- Ini File External Factory. 9feb93 JT
--Ini
I     mNew         -- Creates a new instance of the XObject
SSSSS mReadString  fileName, SecName, EntryName, DefaultStr -- Read .INI
ISSSS mWriteString fileName, SecName, EntryName, String     -- Write .INI
I     mDispose     -- Disposes of XObject instance.
 */

namespace Director {

const char *const IniXObj::xlibName = "Ini";
const XlibFileDesc IniXObj::fileNames[] = {
	{ "INI",	"karma" },	// "Karma: Curse of the 12 Caves" conflicts with JWXIni in "www" gameid
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{"new",			IniXObj::m_new, 0, 0, 400},
	{"readString",	IniXObj::m_readString, 4, 4, 400},
	{"writeString",	IniXObj::m_writeString, 4, 4, 400},
	{nullptr, nullptr, 0, 0, 0}
};

static const BuiltinProto xlibBuiltins[] = {
	{nullptr, nullptr, 0, 0, 0, VOIDSYM}
};

IniXObject::IniXObject(const ObjectType objType) : Object<IniXObject>("Ini") {
	_objType = objType;
}

void IniXObj::open(const ObjectType type, const Common::Path &path) {
	IniXObject::initMethods(xlibMethods);
	IniXObject *xobj = new IniXObject(type);
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void IniXObj::close(ObjectType type) {
	IniXObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void IniXObj::m_new(const int nargs) {
	g_lingo->printSTUBWithArglist("IniXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(IniXObj::m_readString, "")
XOBJSTUB(IniXObj::m_writeString, 0)

}
