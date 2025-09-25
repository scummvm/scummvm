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
#include "director/lingo/xtras/funcolorbucket.h"

/**************************************************
 *
 * USED IN:
 * Barbie as Rapunzel: A Creative Adventure
 * Fisher-Price - Little People: Discovery Airport
 * Frankie Time Traveller
 * Rescue Heroes: Lava Landslide
 *
 **************************************************/

/*
-- xtra funColorBucket
new object me
-- Template handlers --
* fbiPaintBucket integer x, integer y, integer red, integer grn, integer blue-- paint fill
* fbiScreenGrab integer left, integer top, integer right, integer bottom, string filenm
* fbiScreenGrabCast integer left, integer top, integer right, integer bottom, int membernum, * castlibnum

 */

namespace Director {

const char *FuncolorbucketXtra::xlibName = "Funcolorbucket";
const XlibFileDesc FuncolorbucketXtra::fileNames[] = {
	{ "funcolorbucket",   nullptr },
	{ "rmlpaint",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				FuncolorbucketXtra::m_new,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "fbiPaintBucket", FuncolorbucketXtra::m_fbiPaintBucket, 5, 5, 500, HBLTIN },
	{ "fbiScreenGrab", FuncolorbucketXtra::m_fbiScreenGrab, 5, 5, 500, HBLTIN },
	{ "fbiScreenGrabCast", FuncolorbucketXtra::m_fbiScreenGrabCast, 6, 6, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FuncolorbucketXtraObject::FuncolorbucketXtraObject(ObjectType ObjectType) :Object<FuncolorbucketXtraObject>("Funcolorbucket") {
	_objType = ObjectType;
}

bool FuncolorbucketXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FuncolorbucketXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FuncolorbucketXtra::xlibName);
	warning("FuncolorbucketXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void FuncolorbucketXtra::open(ObjectType type, const Common::Path &path) {
    FuncolorbucketXtraObject::initMethods(xlibMethods);
    FuncolorbucketXtraObject *xobj = new FuncolorbucketXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FuncolorbucketXtra::close(ObjectType type) {
    FuncolorbucketXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FuncolorbucketXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FuncolorbucketXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(FuncolorbucketXtra::m_fbiPaintBucket, 0)
XOBJSTUB(FuncolorbucketXtra::m_fbiScreenGrab, 0)
XOBJSTUB(FuncolorbucketXtra::m_fbiScreenGrabCast, 0)

}
