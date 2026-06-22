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
#include "director/lingo/xtras/a/adisplay.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra ADisplay
new object me

* aGetDisplayMode -- Returns current display mode.
* aGetDispalyModes -- Returns current display supported modes.
* aChangeDisplayMode integer Width, integer Height, integer Bit, integer Flag -- Change display mode.
* aChangeDisplayModePlus integer Width, integer Height, integer Bit, integer Freq, integer Flag -- Change display mode with vF.
* aCountDisplay -- Count display monitor 
* aGetVersion -- Returns Xtra and DLL version information.

-- Adisplay Xtra 1.2
-- 
-- Freeware
-- 20001 (c) Andras Kenez
"
 */

namespace Director {

const char *AdisplayXtra::xlibName = "Adisplay";
const XlibFileDesc AdisplayXtra::fileNames[] = {
	{ "adisplay",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AdisplayXtra::m_new,		 0, 0,	500 },
	{ """,				AdisplayXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "aGetDisplayMode", AdisplayXtra::m_aGetDisplayMode, 0, 0, 500, HBLTIN },
	{ "aGetDispalyModes", AdisplayXtra::m_aGetDispalyModes, 0, 0, 500, HBLTIN },
	{ "aChangeDisplayMode", AdisplayXtra::m_aChangeDisplayMode, 4, 4, 500, HBLTIN },
	{ "aChangeDisplayModePlus", AdisplayXtra::m_aChangeDisplayModePlus, 5, 5, 500, HBLTIN },
	{ "aCountDisplay", AdisplayXtra::m_aCountDisplay, 0, 0, 500, HBLTIN },
	{ "aGetVersion", AdisplayXtra::m_aGetVersion, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AdisplayXtraObject::AdisplayXtraObject(ObjectType ObjectType) :Object<AdisplayXtraObject>("Adisplay") {
	_objType = ObjectType;
}

bool AdisplayXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AdisplayXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AdisplayXtra::xlibName);
	warning("AdisplayXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AdisplayXtra::open(ObjectType type, const Common::Path &path) {
    AdisplayXtraObject::initMethods(xlibMethods);
    AdisplayXtraObject *xobj = new AdisplayXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AdisplayXtra::close(ObjectType type) {
    AdisplayXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AdisplayXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AdisplayXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AdisplayXtra::m_aGetDisplayMode, 0)
XOBJSTUB(AdisplayXtra::m_aGetDispalyModes, 0)
XOBJSTUB(AdisplayXtra::m_aChangeDisplayMode, 0)
XOBJSTUB(AdisplayXtra::m_aChangeDisplayModePlus, 0)
XOBJSTUB(AdisplayXtra::m_aCountDisplay, 0)
XOBJSTUB(AdisplayXtra::m_aGetVersion, 0)
XOBJSTUB(AdisplayXtra::m_", 0)

}
