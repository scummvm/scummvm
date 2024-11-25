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
#include "director/lingo/xlibs/fsutil.h"

/**************************************************
 *
 * USED IN:
 * Amusement Planet Phantasmagoria
 * Ursa Minor Blue
 *
 **************************************************/

/*
-- FSUtil XObj v0.1d ©1995 Halfmoon Systems Co. Ltd.
I mNew
X mDispose
S mGetSystemFolder
IS mMakeFolder, <FolderPath>
 */

namespace Director {

const char *const FSUtilXObj::xlibName = "FSUtil";
const XlibFileDesc FSUtilXObj::fileNames[] = {
	{ "FSUtil",   nullptr },
	{ nullptr,        nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				FSUtilXObj::m_new,		 0, 0,	400 },
	{ "dispose",				FSUtilXObj::m_dispose,		 0, 0,	400 },
	{ "getSystemFolder",				FSUtilXObj::m_getSystemFolder,		 0, 0,	400 },
	{ "makeFolder",				FSUtilXObj::m_makeFolder,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FSUtilXObject::FSUtilXObject(ObjectType ObjectType) :Object<FSUtilXObject>("FSUtil") {
	_objType = ObjectType;
}

void FSUtilXObj::open(ObjectType type, const Common::Path &path) {
    FSUtilXObject::initMethods(xlibMethods);
    FSUtilXObject *xobj = new FSUtilXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FSUtilXObj::close(ObjectType type) {
    FSUtilXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FSUtilXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FSUtilXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(FSUtilXObj::m_dispose)
// This stub is fine: a system folder of "" roots the files it creates in
// ScummVM's save directory.
XOBJSTUB(FSUtilXObj::m_getSystemFolder, "")
// This stub is also fine; the only folders it creates are within the
// save directory, and since ScummVM encodes the path inside the filename
// instead of actually creating directories, we don't have to do anything.
XOBJSTUB(FSUtilXObj::m_makeFolder, 0)

}
