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
#include "director/lingo/xlibs/instobj.h"

/**************************************************
 *
 * USED IN:
 * Virtual Nightclub
 *
 **************************************************/

/*
-- InstObj Installer XObject. Andy Wilson, Tape Gallery Multimedia, 20th March 1996
--InstObj
I      mNew                 --Creates a new instance of the XObject
X      mDispose             --Disposes of XObject instance
S      mName                --Returns the XObject name (InstObj)
I      mStatus              --Returns an integer status code
SI     mError, code         --Returns an error string
S      mLastError           --Returns last error string
S      mGetWinDir           --Returns the Windows directory
S      mGetSysDir           --Returns the System directory
S      mGetWinVer           --Returns the Windows version
S      mGetProcInfo         --Returns the processor type
SI     mGetDriveType, Drive --Make a new directory (A=0, B=1, etc.)
II     mGetFreeSpace, Drive --Returns the free space in Kb (A=0, B=1, etc.)
IS     mMakeDir, DirName    --Make a new directory
IS     mDirExists, DirName  --Checks that a directory exists
ISS    mCopyFile, Source,Dest  --Copy a file
IS     mDeleteFile, File    --Delete a file
IS     mFileExists, File    --Returns 0 if the file exists
IS     mAddPMGroup, Group   --Add a program manager group
ISSSSI mAddPMItem, Group,CmndLine,ItemName,IconPath,IconIndex --Add a program manager item
SSSSS  mReadProfile, File,Section,Item,Default --Read an INI profile string
ISSSS  mWriteProfile, File,Section,Item,NewVal --Write an INI profile string
 */

namespace Director {

const char *const InstObjXObj::xlibName = "InstObj";
const XlibFileDesc InstObjXObj::fileNames[] = {
	{ "INSTOBJ",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				InstObjXObj::m_new,		 0, 0,	400 },
	{ "dispose",				InstObjXObj::m_dispose,		 0, 0,	400 },
	{ "name",				InstObjXObj::m_name,		 0, 0,	400 },
	{ "status",				InstObjXObj::m_status,		 0, 0,	400 },
	{ "error",				InstObjXObj::m_error,		 1, 1,	400 },
	{ "lastError",				InstObjXObj::m_lastError,		 0, 0,	400 },
	{ "getWinDir",				InstObjXObj::m_getWinDir,		 0, 0,	400 },
	{ "getSysDir",				InstObjXObj::m_getSysDir,		 0, 0,	400 },
	{ "getWinVer",				InstObjXObj::m_getWinVer,		 0, 0,	400 },
	{ "getProcInfo",				InstObjXObj::m_getProcInfo,		 0, 0,	400 },
	{ "getDriveType",				InstObjXObj::m_getDriveType,		 1, 1,	400 },
	{ "getFreeSpace",				InstObjXObj::m_getFreeSpace,		 1, 1,	400 },
	{ "makeDir",				InstObjXObj::m_makeDir,		 1, 1,	400 },
	{ "dirExists",				InstObjXObj::m_dirExists,		 1, 1,	400 },
	{ "copyFile",				InstObjXObj::m_copyFile,		 2, 2,	400 },
	{ "deleteFile",				InstObjXObj::m_deleteFile,		 1, 1,	400 },
	{ "fileExists",				InstObjXObj::m_fileExists,		 1, 1,	400 },
	{ "addPMGroup",				InstObjXObj::m_addPMGroup,		 1, 1,	400 },
	{ "addPMItem",				InstObjXObj::m_addPMItem,		 5, 5,	400 },
	{ "readProfile",				InstObjXObj::m_readProfile,		 4, 4,	400 },
	{ "writeProfile",				InstObjXObj::m_writeProfile,		 4, 4,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

InstObjXObject::InstObjXObject(ObjectType ObjectType) :Object<InstObjXObject>("InstObj") {
	_objType = ObjectType;
}

void InstObjXObj::open(ObjectType type, const Common::Path &path) {
    InstObjXObject::initMethods(xlibMethods);
    InstObjXObject *xobj = new InstObjXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void InstObjXObj::close(ObjectType type) {
    InstObjXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void InstObjXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("InstObjXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void InstObjXObj::m_getDriveType(int nargs) {
	g_lingo->printSTUBWithArglist("InstObjXObj::m_getDriveType", nargs);
	Datum result("Undetermined Drive Type");

	if (nargs != 1) {
		warning("InstObjXObj: expected 1 argument");
		g_lingo->dropStack(nargs);
	} else {
		Datum id = g_lingo->pop();
		switch (id.asInt()) {
		case 1: // fall-through
		case 2:
			result = Datum("Floppy Drive");
			break;
		case 3:
			result = Datum("Hard Disk");
			break;
		case 4:
			result = Datum("CD Drive");
			break;
		default:
			break;
		}
	}
	g_lingo->push(result);
}

XOBJSTUBNR(InstObjXObj::m_dispose)
XOBJSTUB(InstObjXObj::m_name, "")
XOBJSTUB(InstObjXObj::m_status, 0)
XOBJSTUB(InstObjXObj::m_error, "")
XOBJSTUB(InstObjXObj::m_lastError, "")
XOBJSTUB(InstObjXObj::m_getWinDir, "")
XOBJSTUB(InstObjXObj::m_getSysDir, "")
XOBJSTUB(InstObjXObj::m_getWinVer, "")
XOBJSTUB(InstObjXObj::m_getProcInfo, "")
XOBJSTUB(InstObjXObj::m_getFreeSpace, 0)
XOBJSTUB(InstObjXObj::m_makeDir, 0)
XOBJSTUB(InstObjXObj::m_dirExists, 0)
XOBJSTUB(InstObjXObj::m_copyFile, 0)
XOBJSTUB(InstObjXObj::m_deleteFile, 0)
XOBJSTUB(InstObjXObj::m_fileExists, 0)
XOBJSTUB(InstObjXObj::m_addPMGroup, 0)
XOBJSTUB(InstObjXObj::m_addPMItem, 0)
XOBJSTUB(InstObjXObj::m_readProfile, "")
XOBJSTUB(InstObjXObj::m_writeProfile, 0)

}
