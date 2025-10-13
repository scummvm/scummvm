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
#include "director/lingo/xlibs/l/listdev.h"

/**************************************************
 *
 * USED IN:
 * Microphone Fiend
 *
 **************************************************/

/*
I       mNew
I       mListDev
I       mDispose
 */

namespace Director {

const char *ListDevXObj::xlibName = "ListDev";
const XlibFileDesc ListDevXObj::fileNames[] = {
	{ "ListDev",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				ListDevXObj::m_new,		 0, 0,	500 },
	{ "listDev",				ListDevXObj::m_listDev,		 0, 0,	500 },
	{ "dispose",				ListDevXObj::m_dispose,		 0, 0,	500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

ListDevXObject::ListDevXObject(ObjectType ObjectType) :Object<ListDevXObject>("ListDev") {
	_objType = ObjectType;
}

void ListDevXObj::open(ObjectType type, const Common::Path &path) {
    ListDevXObject::initMethods(xlibMethods);
    ListDevXObject *xobj = new ListDevXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void ListDevXObj::close(ObjectType type) {
    ListDevXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void ListDevXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ListDevXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(ListDevXObj::m_listDev, 0)
XOBJSTUB(ListDevXObj::m_dispose, 0)

}
