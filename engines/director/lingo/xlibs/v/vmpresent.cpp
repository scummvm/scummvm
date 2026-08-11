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
#include "director/lingo/xlibs/v/vmpresent.h"

/**************************************************
 *
 * USED IN:
 * dazzeloids
 *
 **************************************************/

/*
-- VMPresentObj v1.0
I 	mNew		--Instantiate the XObject
I	mVMPresent	--Returns true (1) if VM is turned on, otherwise 0 (false)
 */

namespace Director {

const char *VMPresentXObj::xlibName = "VMPresent";
const XlibFileDesc VMPresentXObj::fileNames[] = {
	{ "VMPresent",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				VMPresentXObj::m_new,		 0, 0,	400 },
	{ "vMPresent",				VMPresentXObj::m_vMPresent,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

VMPresentXObject::VMPresentXObject(ObjectType ObjectType) :Object<VMPresentXObject>("VMPresent") {
	_objType = ObjectType;
}

void VMPresentXObj::open(ObjectType type, const Common::Path &path) {
    VMPresentXObject::initMethods(xlibMethods);
    VMPresentXObject *xobj = new VMPresentXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void VMPresentXObj::close(ObjectType type) {
    VMPresentXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void VMPresentXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("VMPresentXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

// Dazzeloids won't start unless virtual memory is disabled.
// If another game uses this XObj, we might need to expand this into a title check.
XOBJSTUB(VMPresentXObj::m_vMPresent, 0)

}
