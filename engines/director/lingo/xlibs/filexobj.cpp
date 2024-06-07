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
#include "director/lingo/xlibs/filexobj.h"

/**************************************************
 *
 * USED IN:
 * Karma: Curse of the 12 Caves
 *
 **************************************************/

/*
 * -- File External Factory. 9feb93 JT
 * --File
 * ISS    mNew, mode, fileNameOrType  --Creates a new instance of the XObject
 * X      mDispose                --Disposes of XObject instance.
 * II     mWriteChar, charNum     --Writes a single character. Returns error code
 * IS     mWriteString, string    --Writes out a string of chars. Returns error code
 * I      mReadChar       --Returns a single character
 * S      mReadWord       --Returns the next word of an input file
 * S      mReadLine       --Returns the next line of an input file
 * S      mReadFile       --Returns the remainder of the file
 * SSS    mReadToken, breakString, skipString
 * I      mGetPosition    --Returns the file position
 * II     mSetPosition, newPos    --Sets the file position. Returns error code
 * I      mGetLength      --Returns the number of chars in the file
 * ISS    mSetFinderInfo, typeString, creatorString
 * S      mGetFinderInfo  --Gets the finder info
 * S      mFileName       --Returns the name of the file
 * I      mDelete         --Delete the file and dispose of me
 * I      mStatus         --Returns result code of the last file io activity
 * SI     +mError, errorCode  --Returns error message string
 * S      mNativeFileName     --Returns the native (dos) name of the file
 * II     +mSetOverrideDrive, driveLetter --Set override drive letter ('A' - 'Z') to use when loading linked castmembers.  Use 0x00 to clear override.
 */

namespace Director {

const char *FileXObj::xlibName = "File";
const char *FileXObj::fileNames[] = {
	"FILE",
	nullptr
};

static MethodProto xlibMethods[] = {
	{"new",					FileXObj::m_new, 2, 2, 400},
	{"writeChar",			FileXObj::m_writeChar, 1, 1, 400},
	{"writeString",			FileXObj::m_writeString, 1, 1, 400},
	{"readChar",			FileXObj::m_readChar, 0, 0, 400},
	{"readWord",			FileXObj::m_readWord, 0, 0, 400},
	{"readLine",			FileXObj::m_readLine, 0, 0, 400},
	{"readFile",			FileXObj::m_readFile, 0, 0, 400},
	{"readToken",			FileXObj::m_readToken, 2, 2, 400},
	{"getPosition",			FileXObj::m_getPosition, 0, 0, 400},
	{"setPosition",			FileXObj::m_setPosition, 1, 1, 400},
	{"getLength",			FileXObj::m_getLength, 0, 0, 400},
	{"setFinderInfo",		FileXObj::m_setFinderInfo, 2, 2, 400},
	{"getFinderInfo",		FileXObj::m_getFinderInfo, 0, 0, 400},
	{"fileName",			FileXObj::m_fileName, 0, 0, 400},
	{"delete",				FileXObj::m_delete, 0, 0, 400},
	{"status",				FileXObj::m_status, 0, 0, 400},
	{"error",				FileXObj::m_error, 1, 1, 400},
	{"nativeFileName",		FileXObj::m_nativeFileName, 0, 0, 400},
	{"setOverrideDrive",	FileXObj::m_setOverrideDrive, 1, 1, 400},
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

FileXObject::FileXObject(ObjectType objType) :Object<FileXObject>("File") {
	_objType = objType;
}

void FileXObj::open(ObjectType type, const Common::Path &path) {
    FileXObject::initMethods(xlibMethods);
    FileXObject *xobj = new FileXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void FileXObj::close(ObjectType type) {
    FileXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void FileXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FileXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(FileXObj::m_writeChar, 0)
XOBJSTUB(FileXObj::m_writeString, 0)
XOBJSTUB(FileXObj::m_readChar, 0)
XOBJSTUB(FileXObj::m_readWord, "")
XOBJSTUB(FileXObj::m_readLine, "")
XOBJSTUB(FileXObj::m_readFile, "")
XOBJSTUB(FileXObj::m_readToken, "")
XOBJSTUB(FileXObj::m_getPosition, 0)
XOBJSTUB(FileXObj::m_setPosition, 0)
XOBJSTUB(FileXObj::m_getLength, 0)
XOBJSTUB(FileXObj::m_setFinderInfo, 0)
XOBJSTUB(FileXObj::m_getFinderInfo, "")
XOBJSTUB(FileXObj::m_fileName, "")
XOBJSTUB(FileXObj::m_delete, 0)
XOBJSTUB(FileXObj::m_status, 0)
XOBJSTUB(FileXObj::m_error, "")
XOBJSTUB(FileXObj::m_nativeFileName, "")
XOBJSTUB(FileXObj::m_setOverrideDrive, 0)

}
