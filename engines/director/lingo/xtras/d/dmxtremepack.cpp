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
#include "director/lingo/xtras/d/dmxtremepack.h"

/**************************************************
 *
 * USED IN:
 * Oscar 4, Kommissar Kugelblitz (Vermisst am Mississippi)
 *
 **************************************************/

/*
-- xtra DMXtremePack -- this Xtra provides Register for DmTools
new object me
-- MISC --
* DMXtremePackReg string SerialNumber --    Register for DmFade Tools
 */

namespace Director {

const char *DMXtremePackXtra::xlibName = "DMXtremePack";
const XlibFileDesc DMXtremePackXtra::fileNames[] = {
	{ "dmxtremepack",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DMXtremePackXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "DMXtremePackReg", DMXtremePackXtra::m_DMXtremePackReg, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DMXtremePackXtraObject::DMXtremePackXtraObject(ObjectType ObjectType) :Object<DMXtremePackXtraObject>("DMXtremePack") {
	_objType = ObjectType;
}

bool DMXtremePackXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DMXtremePackXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DMXtremePackXtra::xlibName);
	warning("DMXtremePackXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DMXtremePackXtra::open(ObjectType type, const Common::Path &path) {
    DMXtremePackXtraObject::initMethods(xlibMethods);
    DMXtremePackXtraObject *xobj = new DMXtremePackXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DMXtremePackXtra::close(ObjectType type) {
    DMXtremePackXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DMXtremePackXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DMXtremePackXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(DMXtremePackXtra::m_DMXtremePackReg, 0)

}
