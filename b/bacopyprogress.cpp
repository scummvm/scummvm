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
#include "director/lingo/xtras/b/bacopyprogress.h"

/**************************************************
 *
 * USED IN:
 * Charlie Church Mouse - Early Elementary
 * The following games are not Director but the program that installs them is director 8.5.
  * Crosscountry USA 2
   * Crosscountry USA Photo Safari
    * Crosscountry Canada Photo Safari
 **************************************************/

/*
-- xtra baCopyProgress
new object me

* baCopyXFilesProgress string SourceDir, string DestDir, string FileSpec, string Overwrite, string Title, integer Flags -- copies file with progress bar
* baCopyFileProgress string SourceFile, string DestFile, string Overwrite, string Title, integer Flags -- copies file 
* baXCopyProgress string SourceDir, string DestDir, string FileSpec, string Overwrite, integer MakeDir, string Title, integer Flags -- copies all files with progress bar
* baFolderSize string Folder, string FileSpec, integer SubFolders -- returns size of files in folder

-- copyright Magic Modules Pty Ltd, 2001
"
 */

namespace Director {

const char *BacopyprogressXtra::xlibName = "Bacopyprogress";
const XlibFileDesc BacopyprogressXtra::fileNames[] = {
	{ "bacopyprogress",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BacopyprogressXtra::m_new,		 0, 0,	500 },
	{ """,				BacopyprogressXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baCopyXFilesProgress", BacopyprogressXtra::m_baCopyXFilesProgress, 6, 6, 500, HBLTIN },
	{ "baCopyFileProgress", BacopyprogressXtra::m_baCopyFileProgress, 5, 5, 500, HBLTIN },
	{ "baXCopyProgress", BacopyprogressXtra::m_baXCopyProgress, 7, 7, 500, HBLTIN },
	{ "baFolderSize", BacopyprogressXtra::m_baFolderSize, 3, 3, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BacopyprogressXtraObject::BacopyprogressXtraObject(ObjectType ObjectType) :Object<BacopyprogressXtraObject>("Bacopyprogress") {
	_objType = ObjectType;
}

bool BacopyprogressXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BacopyprogressXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BacopyprogressXtra::xlibName);
	warning("BacopyprogressXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BacopyprogressXtra::open(ObjectType type, const Common::Path &path) {
    BacopyprogressXtraObject::initMethods(xlibMethods);
    BacopyprogressXtraObject *xobj = new BacopyprogressXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BacopyprogressXtra::close(ObjectType type) {
    BacopyprogressXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BacopyprogressXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BacopyprogressXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BacopyprogressXtra::m_baCopyXFilesProgress, 0)
XOBJSTUB(BacopyprogressXtra::m_baCopyFileProgress, 0)
XOBJSTUB(BacopyprogressXtra::m_baXCopyProgress, 0)
XOBJSTUB(BacopyprogressXtra::m_baFolderSize, 0)
XOBJSTUB(BacopyprogressXtra::m_", 0)

}
