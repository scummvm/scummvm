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
#include "director/lingo/xtras/f/fileutil.h"

/**************************************************
 *
 * USED IN:
 * getaheadmath
 *
 **************************************************/

/*
-- xtra FileUtil
-- FileUtil Xtra of 24-Jun-96 copyright (c) 1996 Little Planet Publishing
-- File System Functions --
* FileExists string fileName
* RenameFile string oldName, string newName
* DeleteFile string fileName
* CopyFile string fromFName, string toFName
* DirectoryExists string dirName
* CreateDirectory string dirName
* DeleteDirectory string dirName
* XDeleteDirectory string dirName
* CopyDirectory string fromDirName, string toDirName
* XCopyDirectory string fromDirName, string toDirName
* DirectoryToList string dirName
 */

namespace Director {

const char *FileUtilXtra::xlibName = "FileUtil";
const XlibFileDesc FileUtilXtra::fileNames[] = {
	{ "fileutil",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "FileExists", FileUtilXtra::m_FileExists, 1, 1, 500, HBLTIN },
	{ "RenameFile", FileUtilXtra::m_RenameFile, 2, 2, 500, HBLTIN },
	{ "DeleteFile", FileUtilXtra::m_DeleteFile, 1, 1, 500, HBLTIN },
	{ "CopyFile", FileUtilXtra::m_CopyFile, 2, 2, 500, HBLTIN },
	{ "DirectoryExists", FileUtilXtra::m_DirectoryExists, 1, 1, 500, HBLTIN },
	{ "CreateDirectory", FileUtilXtra::m_CreateDirectory, 1, 1, 500, HBLTIN },
	{ "DeleteDirectory", FileUtilXtra::m_DeleteDirectory, 1, 1, 500, HBLTIN },
	{ "XDeleteDirectory", FileUtilXtra::m_XDeleteDirectory, 1, 1, 500, HBLTIN },
	{ "CopyDirectory", FileUtilXtra::m_CopyDirectory, 2, 2, 500, HBLTIN },
	{ "XCopyDirectory", FileUtilXtra::m_XCopyDirectory, 2, 2, 500, HBLTIN },
	{ "DirectoryToList", FileUtilXtra::m_DirectoryToList, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FileUtilXtraObject::FileUtilXtraObject(ObjectType ObjectType) :Object<FileUtilXtraObject>("FileUtil") {
	_objType = ObjectType;
}

bool FileUtilXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FileUtilXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FileUtilXtra::xlibName);
	warning("FileUtilXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void FileUtilXtra::open(ObjectType type, const Common::Path &path) {
    FileUtilXtraObject::initMethods(xlibMethods);
    FileUtilXtraObject *xobj = new FileUtilXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
        g_lingo->_openXtraObjects.push_back(xobj);
    }
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FileUtilXtra::close(ObjectType type) {
    FileUtilXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

XOBJSTUB(FileUtilXtra::m_FileExists, 0)
XOBJSTUB(FileUtilXtra::m_RenameFile, 0)
XOBJSTUB(FileUtilXtra::m_DeleteFile, 0)
XOBJSTUB(FileUtilXtra::m_CopyFile, 0)
XOBJSTUB(FileUtilXtra::m_DirectoryExists, 0)
XOBJSTUB(FileUtilXtra::m_CreateDirectory, 0)
XOBJSTUB(FileUtilXtra::m_DeleteDirectory, 0)
XOBJSTUB(FileUtilXtra::m_XDeleteDirectory, 0)
XOBJSTUB(FileUtilXtra::m_CopyDirectory, 0)
XOBJSTUB(FileUtilXtra::m_XCopyDirectory, 0)
XOBJSTUB(FileUtilXtra::m_DirectoryToList, 0)

}
