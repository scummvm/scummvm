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
#include "director/lingo/xtras/keypoll.h"

/**************************************************
 *
 * USED IN:
 * Safecracker
 *
 **************************************************/

/*
-- -- KeyPoll Xtra
-- by Brian Gray
-- (c) 1996 Macromedia, Inc.  All Rights Reserved.

xtra KeyPoll
new object me

-- KeyPoll handlers --
* bgOneKey integer keyCode -- returns TRUE if key (argument) is down, else FALSE
* bgAllKeys -- returns a linear list of the keycodes of every key currently down

 */

namespace Director {

const char *const KeypollXtra::xlibName = "Keypoll";
const XlibFileDesc KeypollXtra::fileNames[] = {
	{ "keypoll",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "xtra",				KeypollXtra::m_xtra,		 0, 0,	500 },
	{ "new",				KeypollXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ "bgOneKey", KeypollXtra::m_bgOneKey, 1, 1, 500, HBLTIN },
	{ "bgAllKeys", KeypollXtra::m_bgAllKeys, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

KeypollXtraObject::KeypollXtraObject(ObjectType ObjectType) :Object<KeypollXtraObject>("Keypoll") {
	_objType = ObjectType;
}

bool KeypollXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum KeypollXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(KeypollXtra::xlibName);
	warning("KeypollXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void KeypollXtra::open(ObjectType type, const Common::Path &path) {
    KeypollXtraObject::initMethods(xlibMethods);
    KeypollXtraObject *xobj = new KeypollXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void KeypollXtra::close(ObjectType type) {
    KeypollXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void KeypollXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("KeypollXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(KeypollXtra::m_xtra, 0)
XOBJSTUB(KeypollXtra::m_bgOneKey, 0)
XOBJSTUB(KeypollXtra::m_bgAllKeys, 0)

}
