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
#include "director/lingo/xtras/funnybone.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra FunnyBone -- Access movie comps.
new object me
-- OTHER --
* MakeStageCastMem any CastMember, any Rect-- Make a cast member from a portion of the stage.

 */

namespace Director {

const char *FunnyboneXtra::xlibName = "Funnybone";
const XlibFileDesc FunnyboneXtra::fileNames[] = {
	{ "funnybone",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				FunnyboneXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "MakeStageCastMem", FunnyboneXtra::m_MakeStageCastMem, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FunnyboneXtraObject::FunnyboneXtraObject(ObjectType ObjectType) :Object<FunnyboneXtraObject>("Funnybone") {
	_objType = ObjectType;
}

bool FunnyboneXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FunnyboneXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FunnyboneXtra::xlibName);
	warning("FunnyboneXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void FunnyboneXtra::open(ObjectType type, const Common::Path &path) {
    FunnyboneXtraObject::initMethods(xlibMethods);
    FunnyboneXtraObject *xobj = new FunnyboneXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FunnyboneXtra::close(ObjectType type) {
    FunnyboneXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FunnyboneXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FunnyboneXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(FunnyboneXtra::m_MakeStageCastMem, 0)

}
