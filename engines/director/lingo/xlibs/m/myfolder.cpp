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
#include "director/lingo/xlibs/m/myfolder.h"

/**************************************************
 *
 * USED IN:
 * dimensionq
 *
 **************************************************/

/*
-- MyFolder XObject. Windows version. (c) 1995 Mark Carolan
--MyFolder
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
SS     mDo, folderName     --Returns pathname as a string
-- If folderName is empty string, returns path to preferences directory.
-- If folderName is supplied, folder folderName is created in preferences.
-- If folder folderName already exists in preferences, full pathname is returned.
-- Check for errors by looking for non-string return value or string starting with: <Error
-- Shareware. $US15 per title gets you Mac & Win version plus a good night's sleep.
-- Free use for demos. Contact Mark_Carolan@aapda.com.au Compuserve 100242,1154
 */

namespace Director {

const char *MyFolderXObj::xlibName = "MyFolder";
const XlibFileDesc MyFolderXObj::fileNames[] = {
	{ "MYFOLDER",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				MyFolderXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MyFolderXObj::m_dispose,		 0, 0,	400 },
	{ "do",				MyFolderXObj::m_do,		 1, 1,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "MyFolder", MyFolderXObj::m_myFolder, 1, 1, 400, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MyFolderXObject::MyFolderXObject(ObjectType ObjectType) :Object<MyFolderXObject>("MyFolder") {
	_objType = ObjectType;
}

void MyFolderXObj::open(ObjectType type, const Common::Path &path) {
    MyFolderXObject::initMethods(xlibMethods);
    MyFolderXObject *xobj = new MyFolderXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MyFolderXObj::close(ObjectType type) {
    MyFolderXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MyFolderXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MyFolderXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void MyFolderXObj::m_myFolder(int nargs) {
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void MyFolderXObj::m_dispose(int nargs) {
	g_lingo->_state->me.u.obj->dispose();
	g_lingo->push(g_lingo->_state->me);
}
XOBJSTUB(MyFolderXObj::m_do, "")

}
