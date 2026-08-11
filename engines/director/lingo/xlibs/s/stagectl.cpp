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
#include "director/lingo/xlibs/s/stagectl.h"

/**************************************************
 *
 * USED IN:
 * Yellow Brick Road
 *
 **************************************************/

/*
-- Category Misc.
Title Stage Control External Factory
(C)COPYRIGHT IBM CORP. 1993 $Revision
--StageControl
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mName               --Returns the XObject name
II     mNoisy              --Sets noisy flag of the receiver, and returns previous value
I      mNumberOfInstances  --Returns Number of Module Instances
X      mBecomeSystemModal  --StageWindow becomes System Modal
 */

namespace Director {

const char *StageControlXObj::xlibName = "StageControl";
const XlibFileDesc StageControlXObj::fileNames[] = {
	{ "STAGECTL",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				StageControlXObj::m_new,		 0, 0,	300 },
	{ "dispose",				StageControlXObj::m_dispose,		 0, 0,	300 },
	{ "name",				StageControlXObj::m_name,		 0, 0,	300 },
	{ "noisy",				StageControlXObj::m_noisy,		 1, 1,	300 },
	{ "numberOfInstances",				StageControlXObj::m_numberOfInstances,		 0, 0,	300 },
	{ "becomeSystemModal",				StageControlXObj::m_becomeSystemModal,		 0, 0,	300 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

StageControlXObject::StageControlXObject(ObjectType ObjectType) :Object<StageControlXObject>("StageControl") {
	_objType = ObjectType;
}

void StageControlXObj::open(ObjectType type, const Common::Path &path) {
    StageControlXObject::initMethods(xlibMethods);
    StageControlXObject *xobj = new StageControlXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void StageControlXObj::close(ObjectType type) {
    StageControlXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void StageControlXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("StageControlXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(StageControlXObj::m_dispose)
XOBJSTUB(StageControlXObj::m_name, "")
XOBJSTUB(StageControlXObj::m_noisy, 0)
XOBJSTUB(StageControlXObj::m_numberOfInstances, 0)
XOBJSTUBNR(StageControlXObj::m_becomeSystemModal)

}
