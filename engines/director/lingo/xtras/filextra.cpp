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
#include "director/lingo/xtras/filextra.h"

/**************************************************
 *
 * USED IN:
 * I Spy
 *
 **************************************************/

/*
-- xtra FileXtra
-- FileXtra v2.0.4 of 18-Apr-97 by Kent Kersten
-- Copyright (c) 1996-97 Little Planet Publishing
-- For updates see http://www.littleplanet.com/kent/kent.html
-- Contact the author at kent@littleplanet.com
-- This no-charge Xtra may be freely distributed as long as it is
-- accompanied by its documentation and sample movie.
--
-- Drive Functions --
* DriveExists string driveName
* DrivesToList
* DriveFreeSpace string driveName
* DriveIsCDROM string driveName
--
-- File Functions --
* FileOpenDialog string initialDir, string filtStr, string dlogTitle, Boolean createPrompt, Boolean fileMustExist
* FileSaveAsDialog string initialDir, string filename, string dlogTitle, Boolean overwritePrompt
* FileExists string fileName
* RenameFile string oldName, string newName
* DeleteFile string fileName
* CopyFile string fromFName, string toFName
* GetFileModDate string fileName
--
-- Directory Functions --
* DirectoryExists string dirName
* CreateDirectory string dirName
* DeleteDirectory string dirName
* XDeleteDirectory string dirName
* CopyDirectory string fromDirName, string toDirName
* XCopyDirectory string fromDirName, string toDirName
* DirectoryToList string dirName

 */

namespace Director {

const char *FileXtra::xlibName = "File";
const XlibFileDesc FileXtra::fileNames[] = {
	{ "filextra",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "DriveExists", FileXtra::m_DriveExists, 1, 1, 500, HBLTIN },
	{ "DrivesToList", FileXtra::m_DrivesToList, 0, 0, 500, HBLTIN },
	{ "DriveFreeSpace", FileXtra::m_DriveFreeSpace, 1, 1, 500, HBLTIN },
	{ "DriveIsCDROM", FileXtra::m_DriveIsCDROM, 1, 1, 500, HBLTIN },
	{ "FileOpenDialog", FileXtra::m_FileOpenDialog, 5, 5, 500, HBLTIN },
	{ "FileSaveAsDialog", FileXtra::m_FileSaveAsDialog, 4, 4, 500, HBLTIN },
	{ "FileExists", FileXtra::m_FileExists, 1, 1, 500, HBLTIN },
	{ "RenameFile", FileXtra::m_RenameFile, 2, 2, 500, HBLTIN },
	{ "DeleteFile", FileXtra::m_DeleteFile, 1, 1, 500, HBLTIN },
	{ "CopyFile", FileXtra::m_CopyFile, 2, 2, 500, HBLTIN },
	{ "GetFileModDate", FileXtra::m_GetFileModDate, 1, 1, 500, HBLTIN },
	{ "DirectoryExists", FileXtra::m_DirectoryExists, 1, 1, 500, HBLTIN },
	{ "CreateDirectory", FileXtra::m_CreateDirectory, 1, 1, 500, HBLTIN },
	{ "DeleteDirectory", FileXtra::m_DeleteDirectory, 1, 1, 500, HBLTIN },
	{ "XDeleteDirectory", FileXtra::m_XDeleteDirectory, 1, 1, 500, HBLTIN },
	{ "CopyDirectory", FileXtra::m_CopyDirectory, 2, 2, 500, HBLTIN },
	{ "XCopyDirectory", FileXtra::m_XCopyDirectory, 2, 2, 500, HBLTIN },
	{ "DirectoryToList", FileXtra::m_DirectoryToList, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FileXtraObject::FileXtraObject(ObjectType ObjectType) :Object<FileXtraObject>("File") {
	_objType = ObjectType;
}

bool FileXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FileXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FileXtra::xlibName);
	warning("FileXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void FileXtra::open(ObjectType type, const Common::Path &path) {
    FileXtraObject::initMethods(xlibMethods);
    FileXtraObject *xobj = new FileXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FileXtra::close(ObjectType type) {
    FileXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FileXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FileXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(FileXtra::m_DriveExists, 0)
XOBJSTUB(FileXtra::m_DrivesToList, 0)
XOBJSTUB(FileXtra::m_DriveFreeSpace, 0)
XOBJSTUB(FileXtra::m_DriveIsCDROM, 0)
XOBJSTUB(FileXtra::m_FileOpenDialog, 0)
XOBJSTUB(FileXtra::m_FileSaveAsDialog, 0)
XOBJSTUB(FileXtra::m_FileExists, 0)
XOBJSTUB(FileXtra::m_RenameFile, 0)
XOBJSTUB(FileXtra::m_DeleteFile, 0)
XOBJSTUB(FileXtra::m_CopyFile, 0)
XOBJSTUB(FileXtra::m_GetFileModDate, 0)
XOBJSTUB(FileXtra::m_DirectoryExists, 0)
XOBJSTUB(FileXtra::m_CreateDirectory, 0)
XOBJSTUB(FileXtra::m_DeleteDirectory, 0)
XOBJSTUB(FileXtra::m_XDeleteDirectory, 0)
XOBJSTUB(FileXtra::m_CopyDirectory, 0)
XOBJSTUB(FileXtra::m_XCopyDirectory, 0)
XOBJSTUB(FileXtra::m_DirectoryToList, 0)

}
