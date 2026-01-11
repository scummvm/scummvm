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
#include "director/lingo/xlibs/s/savenrestorexobj.h"

/**************************************************
 *
 * USED IN:
 * wttf
 *
 **************************************************/

/*
--		SaveNRestore XObj v 1.0d2 (c) 1995 Samizdat Productions. All Rights Reserved.
--		written by Christopher P. Kelly
I		mNew
II		mWriteSettingsFile
II		mReadSettingsFile
IS		mSetDirectorMovie
IS		mSetSubLocation1
IS		mSetSubLocation2
IIII		mSetKeys whichList position value
IS		mSetMazeKeys
IS		mSetMazeLocation
S		mGetDirectorMovie
S		mGetSubLocation1
S		mGetSubLocation2
III		mGetKeys whichList position
S		mGetMazeKeys
S		mGetMazeLocation
 */

namespace Director {

const char *SaveNRestoreXObj::xlibName = "SaveNRestoreXObj";
const XlibFileDesc SaveNRestoreXObj::fileNames[] = {
	{ "SaveNRestoreXObj",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				SaveNRestoreXObj::m_new,		 0, 0,	400 },
	{ "writeSettingsFile",				SaveNRestoreXObj::m_writeSettingsFile,		 1, 1,	400 },
	{ "readSettingsFile",				SaveNRestoreXObj::m_readSettingsFile,		 1, 1,	400 },
	{ "setDirectorMovie",				SaveNRestoreXObj::m_setDirectorMovie,		 1, 1,	400 },
	{ "setSubLocation1",				SaveNRestoreXObj::m_setSubLocation1,		 1, 1,	400 },
	{ "setSubLocation2",				SaveNRestoreXObj::m_setSubLocation2,		 1, 1,	400 },
	{ "setKeys",				SaveNRestoreXObj::m_setKeys,		 3, 3,	400 },
	{ "setMazeKeys",				SaveNRestoreXObj::m_setMazeKeys,		 1, 1,	400 },
	{ "setMazeLocation",				SaveNRestoreXObj::m_setMazeLocation,		 1, 1,	400 },
	{ "getDirectorMovie",				SaveNRestoreXObj::m_getDirectorMovie,		 0, 0,	400 },
	{ "getSubLocation1",				SaveNRestoreXObj::m_getSubLocation1,		 0, 0,	400 },
	{ "getSubLocation2",				SaveNRestoreXObj::m_getSubLocation2,		 0, 0,	400 },
	{ "getKeys",				SaveNRestoreXObj::m_getKeys,		 2, 2,	400 },
	{ "getMazeKeys",				SaveNRestoreXObj::m_getMazeKeys,		 0, 0,	400 },
	{ "getMazeLocation",				SaveNRestoreXObj::m_getMazeLocation,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

SaveNRestoreXObject::SaveNRestoreXObject(ObjectType ObjectType) :Object<SaveNRestoreXObject>("SaveNRestoreXObj") {
	_objType = ObjectType;
}

void SaveNRestoreXObj::open(ObjectType type, const Common::Path &path) {
    SaveNRestoreXObject::initMethods(xlibMethods);
    SaveNRestoreXObject *xobj = new SaveNRestoreXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void SaveNRestoreXObj::close(ObjectType type) {
    SaveNRestoreXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void SaveNRestoreXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("SaveNRestoreXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(SaveNRestoreXObj::m_writeSettingsFile, 0)
XOBJSTUB(SaveNRestoreXObj::m_readSettingsFile, 0)
XOBJSTUB(SaveNRestoreXObj::m_setDirectorMovie, 0)
XOBJSTUB(SaveNRestoreXObj::m_setSubLocation1, 0)
XOBJSTUB(SaveNRestoreXObj::m_setSubLocation2, 0)
XOBJSTUB(SaveNRestoreXObj::m_setKeys, 0)
XOBJSTUB(SaveNRestoreXObj::m_setMazeKeys, 0)
XOBJSTUB(SaveNRestoreXObj::m_setMazeLocation, 0)
XOBJSTUB(SaveNRestoreXObj::m_getDirectorMovie, "")
XOBJSTUB(SaveNRestoreXObj::m_getSubLocation1, "")
XOBJSTUB(SaveNRestoreXObj::m_getSubLocation2, "")
XOBJSTUB(SaveNRestoreXObj::m_getKeys, 0)
XOBJSTUB(SaveNRestoreXObj::m_getMazeKeys, "")
XOBJSTUB(SaveNRestoreXObj::m_getMazeLocation, "")

}
