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

#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "director/director.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/lingo.h"
#include "director/lingo/xlibs/g/getpath.h"

/**************************************************
 *
 * USED IN:
 * Night Light
 *
 **************************************************/

/*
-- External Factory. 30JAN95
--GetPath
S       mName                           --
I       mStatus                         --
I       mNew                            --
SS      mGetPath, hDirName              --
SS      mGetCDPath, hCDFileName         --
X       mDispose                        --
 */

namespace Director {

const char *GetPathXObj::xlibName = "GetPath";
const XlibFileDesc GetPathXObj::fileNames[] = {
	{"GETPATH", nullptr},
	{nullptr, nullptr},
};

static MethodProto xlibMethods[] = {
	{"name",	  GetPathXObj::m_name,		0, 0, 400},
	{"status",	  GetPathXObj::m_status,	0, 0, 400},
	{"new",		  GetPathXObj::m_new,		0, 0, 400},
	{"getPath",	  GetPathXObj::m_getPath,	1, 1, 400},
	{"getCDPath", GetPathXObj::m_getCDPath, 1, 1, 400},
	{"dispose",	  GetPathXObj::m_dispose,	0, 0, 400},
	{nullptr,	  nullptr,					0, 0, 0  }
};

static BuiltinProto xlibBuiltins[] = {
	{nullptr, nullptr, 0, 0, 0, VOIDSYM}};

GetPathXObject::GetPathXObject(ObjectType ObjectType) : Object<GetPathXObject>("GetPath") {
	_objType = ObjectType;
}

void GetPathXObj::open(ObjectType type, const Common::Path &path) {
	GetPathXObject::initMethods(xlibMethods);
	GetPathXObject *xobj = new GetPathXObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void GetPathXObj::close(ObjectType type) {
	GetPathXObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

void GetPathXObj::m_new(int nargs) {
	debugC(5, kDebugLingoExec, "GetPathXObj::m_new");

	// Return the object instance
	g_lingo->push(g_lingo->_state->me);
}

void GetPathXObj::m_name(int nargs) {
	debugC(5, kDebugLingoExec, "GetPathXObj::m_name");

	// Return "GetPath" string
	g_lingo->push(Datum("GetPath"));
}

void GetPathXObj::m_status(int nargs) {
	// Return no error
	g_lingo->push(Datum(0));
}


void GetPathXObj::m_getPath(int nargs) {
	// Pop the subfolder argument
	Common::String subpath = g_lingo->pop().asString();

	debugC(5, kDebugLingoExec, "GetPathXObj::m_getPath: subfolder='%s'", subpath.c_str());

	Common::String fullPath = ""; // savePrefix()  + subpath;

	debugC(5, kDebugLingoExec, "GetPathXObj::m_getPath: Returning path '%s'", fullPath.c_str());

	// Return the full path
	g_lingo->push(Datum(fullPath));
}


void GetPathXObj::m_getCDPath(int nargs) {
	// Pop the subpath argument
	Common::String subpath = g_lingo->pop().asString();

	debugC(5, kDebugLingoExec, "GetPathXObj::m_getCDPath: subfolder='%s'", subpath.c_str());

	// Returns the drive letter of the CD-ROM drive
	g_lingo->push(Datum("E"));
}

void GetPathXObj::m_dispose(int nargs) {
	debugC(5, kDebugLingoExec, "GetPathXObj::m_dispose");
}

} // namespace Director
