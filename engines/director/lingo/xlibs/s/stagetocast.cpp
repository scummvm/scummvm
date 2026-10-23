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
#include "director/lingo/xlibs/s/stagetocast.h"

/**************************************************
 *
 * USED IN:
 * frankenstein-mac
 *
 **************************************************/

/*
-- StageToCast.XObj vers. 1.0.1
--             Returns a handle to a bitmap of a
--             particular region of the active screen,
--             so you can assign it to a CastMember.
--
IIIII mNew  -- creates the object in RAM,
--             and is used to specify the region
--             of the screen. Four integers are in
--             order of: Top, Left, Bottom, Right.
--             Example of Lingo syntax:
-- set myObj =  StageToCast( mNew, 65, 86, 65+128, 86+240 )
--
P mGetHandle -- returns a handle to the bitmap.
--              Example of Lingo syntax:
-- set the picture of cast A11 = myObj( mGetHandle )
 */

namespace Director {

const char *StageToCastXObj::xlibName = "StageToCast";
const XlibFileDesc StageToCastXObj::fileNames[] = {
	{ "StageToCast",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StageToCastXObj::m_new,		 4, 4,	400 },
	{ "getHandle",				StageToCastXObj::m_getHandle,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StageToCastXObject::StageToCastXObject(ObjectType ObjectType) :Object<StageToCastXObject>("StageToCast") {
	_objType = ObjectType;
}

void StageToCastXObj::open(ObjectType type, const Common::Path &path) {
    StageToCastXObject::initMethods(xlibMethods);
    StageToCastXObject *xobj = new StageToCastXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
        g_lingo->_openXtraObjects.push_back(xobj);
    }
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StageToCastXObj::close(ObjectType type) {
    StageToCastXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StageToCastXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StageToCastXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(StageToCastXObj::m_getHandle, 0)

}
