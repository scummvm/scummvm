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
#include "director/lingo/xtras/a/absfile.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra AbsFile -- version 1.0.14
-- http://g5tools.com
new object me
forget object me
-- Template handlers --
* AbsFileRegister string key -- Unlock xtra, return version string
GetError object me -- Return last error code value
GetErrorText object me, integer code -- Retrieve the message text for the errorCode
SetIntegerFormat object me, integer zeroForWindows -- Set integer format to little/big endian
Open object me, string fileName, integer openMode, integer creationMode -- Open a file
Close object me -- Close the file
ReadByte object me -- Read a signed byte
ReadUByte object me -- Read an unsigned byte
ReadShort object me -- Read a 16 bits signed integer
ReadUShort object me -- Read a 16 bits unsigned integer
ReadInt object me -- Read a 32 bits signed integer
ReadUInt object me -- Read a 32 bits unsigned integer
ReadFloat object me -- Read a float (64 bits IEEE)
ReadPoint object me -- Read a point
ReadRect object me -- Read a rect
ReadBinary object me, integer numberOfByte -- Read numberOfByte in a string
ReadString object me, integer maxLenght -- Read a string ended by CR, CRLF, Ctrl+Z or 0
ReadText object me, integer maxLenght -- Read a string ended by ctrl+z or 0
ReadSymbol object me -- Read a symbol
ReadList object me, string listName -- Find and read a list or property list
BytesReaded object me -- Return the number of bytes readed by the last function
SetPosition object me, integer offset, integer mode -- Move the file pointer
GetPosition object me -- Get the file pointer absolute position
SetMarker object me, string markerName -- Write a marker in the file
GotoMarker object me, string markerName -- Move the file to the specified marker
Length object me -- Return the file size
SetEndOfFile object me -- Set end of file at the current position of the file pointer
Write object me, * -- Write all parameters
WriteByte object me, integer octet -- Write a signed byte
WriteUByte object me, integer octet -- Write an unsigned byte
WriteShort object me, int value -- Write a 16 bits signed integer
WriteUShort object me, int value -- Write a 16 bits unsigned integer
WriteInt object me, integer value -- Write a 32 bits signed integer
WriteUInt object me, float value -- Write a 32 bits unsigned integer
WriteFloat object me, float value -- Write a float (64 bits IEEE)
WritePoint object me, point aPoint -- Write a point
WriteRect object me, rect aRect -- Write a rect
WriteString object me, string aString, symbol terminator -- Write a string ended by 0 or CR
WriteSymbol object me, symbol aSymbol -- Write a symbol
WriteList object me, string listName, list aList -- Write a list or property list
Flush object me -- Causes all buffered data to be written to the file
OpenDialog object me, string extension -- Display a dialog for selecting a file and open it
SaveDialog object me, string extension -- Display a dialog for selecting a file and open it
FileOpenDialog object me, string title, string filter, string defaultExt, string initDir, string defaultName, integer multiSelect, integer openFile
FileSaveDialog object me, string title, string filter, string defaultExt, string initDir, string defaultName, integer multiSelect, integer openFile
FolderDialog object me, string title -- Display a dialog for selecting a folder
ComputerDialog object me, string title -- Display a dialog for selecting a computer
PrinterDialog object me, string title -- Display a dialog for selecting a printer
"
 */

namespace Director {

const char *AbsfileXtra::xlibName = "Absfile";
const XlibFileDesc AbsfileXtra::fileNames[] = {
	{ "absfile",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AbsfileXtra::m_new,		 0, 0,	700 },
	{ "forget",				AbsfileXtra::m_forget,		 0, 0,	700 },
	{ "GetError",				AbsfileXtra::m_GetError,		 0, 0,	700 },
	{ "GetErrorText",				AbsfileXtra::m_GetErrorText,		 1, 1,	700 },
	{ "SetIntegerFormat",				AbsfileXtra::m_SetIntegerFormat,		 1, 1,	700 },
	{ "Open",				AbsfileXtra::m_Open,		 3, 3,	700 },
	{ "Close",				AbsfileXtra::m_Close,		 0, 0,	700 },
	{ "ReadByte",				AbsfileXtra::m_ReadByte,		 0, 0,	700 },
	{ "ReadUByte",				AbsfileXtra::m_ReadUByte,		 0, 0,	700 },
	{ "ReadShort",				AbsfileXtra::m_ReadShort,		 0, 0,	700 },
	{ "ReadUShort",				AbsfileXtra::m_ReadUShort,		 0, 0,	700 },
	{ "ReadInt",				AbsfileXtra::m_ReadInt,		 0, 0,	700 },
	{ "ReadUInt",				AbsfileXtra::m_ReadUInt,		 0, 0,	700 },
	{ "ReadFloat",				AbsfileXtra::m_ReadFloat,		 0, 0,	700 },
	{ "ReadPoint",				AbsfileXtra::m_ReadPoint,		 0, 0,	700 },
	{ "ReadRect",				AbsfileXtra::m_ReadRect,		 0, 0,	700 },
	{ "ReadBinary",				AbsfileXtra::m_ReadBinary,		 1, 1,	700 },
	{ "ReadString",				AbsfileXtra::m_ReadString,		 1, 1,	700 },
	{ "ReadText",				AbsfileXtra::m_ReadText,		 1, 1,	700 },
	{ "ReadSymbol",				AbsfileXtra::m_ReadSymbol,		 0, 0,	700 },
	{ "ReadList",				AbsfileXtra::m_ReadList,		 1, 1,	700 },
	{ "BytesReaded",				AbsfileXtra::m_BytesReaded,		 0, 0,	700 },
	{ "SetPosition",				AbsfileXtra::m_SetPosition,		 2, 2,	700 },
	{ "GetPosition",				AbsfileXtra::m_GetPosition,		 0, 0,	700 },
	{ "SetMarker",				AbsfileXtra::m_SetMarker,		 1, 1,	700 },
	{ "GotoMarker",				AbsfileXtra::m_GotoMarker,		 1, 1,	700 },
	{ "Length",				AbsfileXtra::m_Length,		 0, 0,	700 },
	{ "SetEndOfFile",				AbsfileXtra::m_SetEndOfFile,		 0, 0,	700 },
	{ "Write",				AbsfileXtra::m_Write,		 -1, 0,	700 },
	{ "WriteByte",				AbsfileXtra::m_WriteByte,		 1, 1,	700 },
	{ "WriteUByte",				AbsfileXtra::m_WriteUByte,		 1, 1,	700 },
	{ "WriteShort",				AbsfileXtra::m_WriteShort,		 1, 1,	700 },
	{ "WriteUShort",				AbsfileXtra::m_WriteUShort,		 1, 1,	700 },
	{ "WriteInt",				AbsfileXtra::m_WriteInt,		 1, 1,	700 },
	{ "WriteUInt",				AbsfileXtra::m_WriteUInt,		 1, 1,	700 },
	{ "WriteFloat",				AbsfileXtra::m_WriteFloat,		 1, 1,	700 },
	{ "WritePoint",				AbsfileXtra::m_WritePoint,		 1, 1,	700 },
	{ "WriteRect",				AbsfileXtra::m_WriteRect,		 1, 1,	700 },
	{ "WriteString",				AbsfileXtra::m_WriteString,		 2, 2,	700 },
	{ "WriteSymbol",				AbsfileXtra::m_WriteSymbol,		 1, 1,	700 },
	{ "WriteList",				AbsfileXtra::m_WriteList,		 2, 2,	700 },
	{ "Flush",				AbsfileXtra::m_Flush,		 0, 0,	700 },
	{ "OpenDialog",				AbsfileXtra::m_OpenDialog,		 1, 1,	700 },
	{ "SaveDialog",				AbsfileXtra::m_SaveDialog,		 1, 1,	700 },
	{ "FileOpenDialog",				AbsfileXtra::m_FileOpenDialog,		 7, 7,	700 },
	{ "FileSaveDialog",				AbsfileXtra::m_FileSaveDialog,		 7, 7,	700 },
	{ "FolderDialog",				AbsfileXtra::m_FolderDialog,		 1, 1,	700 },
	{ "ComputerDialog",				AbsfileXtra::m_ComputerDialog,		 1, 1,	700 },
	{ "PrinterDialog",				AbsfileXtra::m_PrinterDialog,		 1, 1,	700 },
	{ """,				AbsfileXtra::m_",		 -1, -1,	700 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "AbsFileRegister", AbsfileXtra::m_AbsFileRegister, 1, 1, 700, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AbsfileXtraObject::AbsfileXtraObject(ObjectType ObjectType) :Object<AbsfileXtraObject>("Absfile") {
	_objType = ObjectType;
}

bool AbsfileXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AbsfileXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AbsfileXtra::xlibName);
	warning("AbsfileXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AbsfileXtra::open(ObjectType type, const Common::Path &path) {
    AbsfileXtraObject::initMethods(xlibMethods);
    AbsfileXtraObject *xobj = new AbsfileXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AbsfileXtra::close(ObjectType type) {
    AbsfileXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AbsfileXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AbsfileXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AbsfileXtra::m_forget, 0)
XOBJSTUB(AbsfileXtra::m_AbsFileRegister, 0)
XOBJSTUB(AbsfileXtra::m_GetError, 0)
XOBJSTUB(AbsfileXtra::m_GetErrorText, 0)
XOBJSTUB(AbsfileXtra::m_SetIntegerFormat, 0)
XOBJSTUB(AbsfileXtra::m_Open, 0)
XOBJSTUB(AbsfileXtra::m_Close, 0)
XOBJSTUB(AbsfileXtra::m_ReadByte, 0)
XOBJSTUB(AbsfileXtra::m_ReadUByte, 0)
XOBJSTUB(AbsfileXtra::m_ReadShort, 0)
XOBJSTUB(AbsfileXtra::m_ReadUShort, 0)
XOBJSTUB(AbsfileXtra::m_ReadInt, 0)
XOBJSTUB(AbsfileXtra::m_ReadUInt, 0)
XOBJSTUB(AbsfileXtra::m_ReadFloat, 0)
XOBJSTUB(AbsfileXtra::m_ReadPoint, 0)
XOBJSTUB(AbsfileXtra::m_ReadRect, 0)
XOBJSTUB(AbsfileXtra::m_ReadBinary, 0)
XOBJSTUB(AbsfileXtra::m_ReadString, 0)
XOBJSTUB(AbsfileXtra::m_ReadText, 0)
XOBJSTUB(AbsfileXtra::m_ReadSymbol, 0)
XOBJSTUB(AbsfileXtra::m_ReadList, 0)
XOBJSTUB(AbsfileXtra::m_BytesReaded, 0)
XOBJSTUB(AbsfileXtra::m_SetPosition, 0)
XOBJSTUB(AbsfileXtra::m_GetPosition, 0)
XOBJSTUB(AbsfileXtra::m_SetMarker, 0)
XOBJSTUB(AbsfileXtra::m_GotoMarker, 0)
XOBJSTUB(AbsfileXtra::m_Length, 0)
XOBJSTUB(AbsfileXtra::m_SetEndOfFile, 0)
XOBJSTUB(AbsfileXtra::m_Write, 0)
XOBJSTUB(AbsfileXtra::m_WriteByte, 0)
XOBJSTUB(AbsfileXtra::m_WriteUByte, 0)
XOBJSTUB(AbsfileXtra::m_WriteShort, 0)
XOBJSTUB(AbsfileXtra::m_WriteUShort, 0)
XOBJSTUB(AbsfileXtra::m_WriteInt, 0)
XOBJSTUB(AbsfileXtra::m_WriteUInt, 0)
XOBJSTUB(AbsfileXtra::m_WriteFloat, 0)
XOBJSTUB(AbsfileXtra::m_WritePoint, 0)
XOBJSTUB(AbsfileXtra::m_WriteRect, 0)
XOBJSTUB(AbsfileXtra::m_WriteString, 0)
XOBJSTUB(AbsfileXtra::m_WriteSymbol, 0)
XOBJSTUB(AbsfileXtra::m_WriteList, 0)
XOBJSTUB(AbsfileXtra::m_Flush, 0)
XOBJSTUB(AbsfileXtra::m_OpenDialog, 0)
XOBJSTUB(AbsfileXtra::m_SaveDialog, 0)
XOBJSTUB(AbsfileXtra::m_FileOpenDialog, 0)
XOBJSTUB(AbsfileXtra::m_FileSaveDialog, 0)
XOBJSTUB(AbsfileXtra::m_FolderDialog, 0)
XOBJSTUB(AbsfileXtra::m_ComputerDialog, 0)
XOBJSTUB(AbsfileXtra::m_PrinterDialog, 0)
XOBJSTUB(AbsfileXtra::m_", 0)

}
