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
#include "director/lingo/xlibs/xcmdglue.h"

/**************************************************
 *
 * USED IN:
 * Standard Macromedia Director XObject
 *
 **************************************************/

/*
--XCMDGlue, Tool, 1.6.4, 14aug93
--Interface to XCMD's and XFCN's
--© 1989, 1990 MacroMind, Inc.
--by John Thompson
ISI 	mNew
X 	mDispose
XI 	mVerbDispose
S 	mName
V	mVerb
XO	mSetHandler
O	mGetHandler
 */

namespace Director {

const char *const XCMDGlueXObj::xlibName = "XCMDGlue";
const XlibFileDesc XCMDGlueXObj::fileNames[] = {
	{ "XCMDGlue",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				XCMDGlueXObj::m_new,		 2, 2,	200 },
	{ "dispose",				XCMDGlueXObj::m_dispose,		 0, 0,	200 },
	{ "verbDispose",				XCMDGlueXObj::m_verbDispose,		 1, 1,	200 },
	{ "name",				XCMDGlueXObj::m_name,		 0, 0,	200 },
	{ "verb",				XCMDGlueXObj::m_verb,		 0, 0,	200 },
	{ "setHandler",				XCMDGlueXObj::m_setHandler,		 1, 1,	200 },
	{ "getHandler",				XCMDGlueXObj::m_getHandler,		 0, 0,	200 },
	{ nullptr, nullptr, 0, 0, 0 }
};

XCMDGlueXObject::XCMDGlueXObject(ObjectType ObjectType) :Object<XCMDGlueXObject>("XCMDGlue") {
	_objType = ObjectType;
}

void XCMDGlueXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		XCMDGlueXObject::initMethods(xlibMethods);
		XCMDGlueXObject *xobj = new XCMDGlueXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void XCMDGlueXObj::close(ObjectType type) {
	if (type == kXObj) {
		XCMDGlueXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void XCMDGlueXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("XCMDGlueXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(XCMDGlueXObj::m_dispose)
XOBJSTUBNR(XCMDGlueXObj::m_verbDispose)
XOBJSTUB(XCMDGlueXObj::m_name, "")
XOBJSTUB(XCMDGlueXObj::m_verb, 0)
XOBJSTUBNR(XCMDGlueXObj::m_setHandler)
XOBJSTUB(XCMDGlueXObj::m_getHandler, 0)

}
