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
#include "director/lingo/xlibs/xwin.h"

/**************************************************
 *
 * USED IN:
 * hellcab-win
 *
 **************************************************/

/*
-- XWIN External Factory. 9jul93 JU
--XWIN
I      mNew  --Creates a new instance of the XObject
X      mDispose                --Disposes of XObject instance.
II     mWriteChar, charNum     --Writes a single character. Returns error code
IS     mWriteString, string    --Writes out a string of chars. Returns error code
I      mReadChar       --Returns a single character
S      mReadWord       --Returns the next word of an input file
S      mReadLine       --Returns the next line of an input file
S      mReadFile       --Returns the remainder of the file
SSS    mReadToken, breakString, skipString
I      mGetPosition    --Returns the file position
II     mSetPosition, newPos    --Sets the file position. Returns error code
I      mGetLength      --Returns the number of chars in the file
ISS    mSetFinderInfo, typeString, creatorString
S      mGetFinderInfo  --Gets the finder info
S      mFileName       --Returns the name of the file
I      mDelete         --Delete the file and dispose of me
I      mStatus         --Returns result code of the last file io activity
SI     +mError, errorCode  --Returns error message string
V      mReadPICT           --Return handle to Metafile
S      mNativeFileName     --Returns the native (dos) name of the file
I          mhwnd  --window handle
 */

namespace Director {

const char *const XWINXObj::xlibName = "XWIN";
const XlibFileDesc XWINXObj::fileNames[] = {
	{ "XWIN",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				XWINXObj::m_new,		 0, 0,	300 },
	{ "dispose",				XWINXObj::m_dispose,		 0, 0,	300 },
	{ "writeChar",				XWINXObj::m_writeChar,		 1, 1,	300 },
	{ "writeString",				XWINXObj::m_writeString,		 1, 1,	300 },
	{ "readChar",				XWINXObj::m_readChar,		 0, 0,	300 },
	{ "readWord",				XWINXObj::m_readWord,		 0, 0,	300 },
	{ "readLine",				XWINXObj::m_readLine,		 0, 0,	300 },
	{ "readFile",				XWINXObj::m_readFile,		 0, 0,	300 },
	{ "readToken",				XWINXObj::m_readToken,		 2, 2,	300 },
	{ "getPosition",				XWINXObj::m_getPosition,		 0, 0,	300 },
	{ "setPosition",				XWINXObj::m_setPosition,		 1, 1,	300 },
	{ "getLength",				XWINXObj::m_getLength,		 0, 0,	300 },
	{ "setFinderInfo",				XWINXObj::m_setFinderInfo,		 2, 2,	300 },
	{ "getFinderInfo",				XWINXObj::m_getFinderInfo,		 0, 0,	300 },
	{ "fileName",				XWINXObj::m_fileName,		 0, 0,	300 },
	{ "delete",				XWINXObj::m_delete,		 0, 0,	300 },
	{ "status",				XWINXObj::m_status,		 0, 0,	300 },
	{ "error",				XWINXObj::m_error,		 1, 1,	300 },
	{ "readPICT",				XWINXObj::m_readPICT,		 0, 0,	300 },
	{ "nativeFileName",				XWINXObj::m_nativeFileName,		 0, 0,	300 },
	{ "hwnd",				XWINXObj::m_hwnd,		 0, 0,	300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

XWINXObject::XWINXObject(ObjectType ObjectType) :Object<XWINXObject>("XWIN") {
	_objType = ObjectType;
}

void XWINXObj::open(ObjectType type, const Common::Path &path) {
    XWINXObject::initMethods(xlibMethods);
    XWINXObject *xobj = new XWINXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void XWINXObj::close(ObjectType type) {
    XWINXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void XWINXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("XWINXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(XWINXObj::m_dispose)
XOBJSTUB(XWINXObj::m_writeChar, 0)
XOBJSTUB(XWINXObj::m_writeString, 0)
XOBJSTUB(XWINXObj::m_readChar, 0)
XOBJSTUB(XWINXObj::m_readWord, "")
XOBJSTUB(XWINXObj::m_readLine, "")
XOBJSTUB(XWINXObj::m_readFile, "")
XOBJSTUB(XWINXObj::m_readToken, "")
XOBJSTUB(XWINXObj::m_getPosition, 0)
XOBJSTUB(XWINXObj::m_setPosition, 0)
XOBJSTUB(XWINXObj::m_getLength, 0)
XOBJSTUB(XWINXObj::m_setFinderInfo, 0)
XOBJSTUB(XWINXObj::m_getFinderInfo, "")
XOBJSTUB(XWINXObj::m_fileName, "")
XOBJSTUB(XWINXObj::m_delete, 0)
XOBJSTUB(XWINXObj::m_status, 0)
XOBJSTUB(XWINXObj::m_error, "")
XOBJSTUB(XWINXObj::m_readPICT, 0)
XOBJSTUB(XWINXObj::m_nativeFileName, "")
XOBJSTUB(XWINXObj::m_hwnd, 0)

}
