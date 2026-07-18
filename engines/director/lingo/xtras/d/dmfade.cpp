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
#include "director/lingo/xtras/d/dmfade.h"

/**************************************************
 *
 * USED IN:
 * JANOSCH: Oh, wie schön ist Panama (D8)
 *
 **************************************************/

/*
xtra DmFade -- this Xtra provides Register for DmFade Tools
new object me
-- MISC --
* DmFadeReg string SerialNumber -- Register for DmFade Tools
 */

namespace Director {

const char *DmFadeXtra::xlibName = "DmFade";
const XlibFileDesc DmFadeXtra::fileNames[] = {
	{ "DmFade",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				DmFadeXtra::m_new,		 0, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "DmFadeReg", DmFadeXtra::m_DmFadeReg, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DmFadeXtraObject::DmFadeXtraObject(ObjectType ObjectType) :Object<DmFadeXtraObject>("DmFade") {
	_objType = ObjectType;
}

bool DmFadeXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum DmFadeXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(DmFadeXtra::xlibName);
	warning("DmFadeXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void DmFadeXtra::open(ObjectType type, const Common::Path &path) {
    DmFadeXtraObject::initMethods(xlibMethods);
    DmFadeXtraObject *xobj = new DmFadeXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DmFadeXtra::close(ObjectType type) {
    DmFadeXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DmFadeXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DmFadeXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

// Serial number check for the DmFade tools. There is nothing to unlock in
// ScummVM, so accept any key. Games call this before anything else in
// startMovie, so it must not raise a Lingo error.
XOBJSTUB(DmFadeXtra::m_DmFadeReg, 0)

}
