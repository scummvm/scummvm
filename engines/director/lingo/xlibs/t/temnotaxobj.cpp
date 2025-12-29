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
#include "director/lingo/xlibs/t/temnotaxobj.h"

/**************************************************
 *
 * USED IN:
 * wttf
 *
 **************************************************/

/*
--		TemnotaXObj v 1.0d2 (c) 1995 Samizdat Productions.
--		All Rights Reserved.
--		written by Christopher P. Kelly
I		mNew
I		mPutUp
I		mGoAway
 */

namespace Director {

const char *TemnotaXObj::xlibName = "TemnotaXObj";
const XlibFileDesc TemnotaXObj::fileNames[] = {
	{ "TemnotaXObj",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				TemnotaXObj::m_new,		 0, 0,	400 },
	{ "putUp",				TemnotaXObj::m_putUp,		 0, 0,	400 },
	{ "goAway",				TemnotaXObj::m_goAway,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

TemnotaXObject::TemnotaXObject(ObjectType ObjectType) :Object<TemnotaXObject>("TemnotaXObj") {
	_objType = ObjectType;
}

void TemnotaXObj::open(ObjectType type, const Common::Path &path) {
    TemnotaXObject::initMethods(xlibMethods);
    TemnotaXObject *xobj = new TemnotaXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void TemnotaXObj::close(ObjectType type) {
    TemnotaXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void TemnotaXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("TemnotaXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(TemnotaXObj::m_putUp, 0)
XOBJSTUB(TemnotaXObj::m_goAway, 0)

}
