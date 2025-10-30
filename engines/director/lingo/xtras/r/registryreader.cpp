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
#include "director/lingo/xtras/r/registryreader.h"

/**************************************************
 *
 * USED IN:
 * I Spy Spooky Mansion
 *
 **************************************************/

/*
-- xtra RegistryReader
-- RegistryReader Xtra in the Magister Goodie series of free Xtras
-- for Macromedia Director.
--                         Copyright (C) 1997, Magister Ludi s.r.l.
--
-- For other Xtras and multimedia products, check out www.magisterludi.com
--
--
new object me -- do not instantiate this Xtra - it is not needed!
* GetPrivateProfileString string fileName, sectionName, keyName, defaultStr
                           -- a wrapper for Windows' GetPrivateProfileString
* ReadRegistryValue string keyName, valueName
                           -- reads the key/value in the registry
                           -- keyName should include "HKEY_CURRENT_USER"
                           -- or "HKEY_LOCAL_MACHINE"

 */

namespace Director {

const char *RegistryReaderXtra::xlibName = "RegistryReader";
const XlibFileDesc RegistryReaderXtra::fileNames[] = {
	{ "registryreader",   nullptr },
	{ "RegRead",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				RegistryReaderXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "GetPrivateProfileString", RegistryReaderXtra::m_GetPrivateProfileString, 4, 4, 500, HBLTIN },
	{ "ReadRegistryValue", RegistryReaderXtra::m_ReadRegistryValue, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

RegistryReaderXtraObject::RegistryReaderXtraObject(ObjectType ObjectType) :Object<RegistryReaderXtraObject>("RegistryReader") {
	_objType = ObjectType;
}

bool RegistryReaderXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum RegistryReaderXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(RegistryReaderXtra::xlibName);
	warning("RegistryReaderXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void RegistryReaderXtra::open(ObjectType type, const Common::Path &path) {
    RegistryReaderXtraObject::initMethods(xlibMethods);
    RegistryReaderXtraObject *xobj = new RegistryReaderXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void RegistryReaderXtra::close(ObjectType type) {
    RegistryReaderXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void RegistryReaderXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("RegistryReaderXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(RegistryReaderXtra::m_GetPrivateProfileString, 0)
XOBJSTUB(RegistryReaderXtra::m_ReadRegistryValue, 0)

}
