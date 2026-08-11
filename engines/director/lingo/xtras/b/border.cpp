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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xtras/b/border.h"

/**************************************************
 *
 * USED IN:
 * Physikus (l'Espresso, Italian; Ruske & Pühretmaier)
 * Bioscopia / Biolab
 *
 **************************************************/

/*
 * Border() -- registers the Border Xtra instance.
 * new object me
 * Register object me, integer key -- validate the registration key
 */

namespace Director {

const char *BorderXtra::xlibName = "Border";
const XlibFileDesc BorderXtra::fileNames[] = {
	{ "Border",	nullptr },
	{ nullptr,	nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",		BorderXtra::m_new,		0, 0,	500 },
	{ "Register",	BorderXtra::m_register,	1, 1,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BorderXtraObject::BorderXtraObject(ObjectType ObjectType) :Object<BorderXtraObject>("Border") {
	_objType = ObjectType;
}

bool BorderXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BorderXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BorderXtra::xlibName);
	warning("BorderXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BorderXtra::open(ObjectType type, const Common::Path &path) {
	BorderXtraObject::initMethods(xlibMethods);
	BorderXtraObject *xobj = new BorderXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void BorderXtra::close(ObjectType type) {
	BorderXtraObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void BorderXtra::m_new(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BorderXtra::m_register, 0)

} // End of namespace Director
