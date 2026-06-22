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
#include "director/lingo/xtras/binaryio.h"

/**************************************************
 *
 * USED IN:
 * Cosmopolitan Fashion Makeover Deluxe
 *
 **************************************************/

/*
-- xtra BinaryIO
-- BinaryIO Xtra, for use with Macromedia Director 5 and later (Windows; now using 32-bit code).
-- 
-- Version 1.08, September 26, 1998. Registered version-- thanks!
-- 
-- Copyright © 1997-1998, Glenn M. Picher, Dirigo Multimedia
-- 
-- Published and supported by:
-- 
-- updateStage
-- 1341 Mass Ave., Suite 124
-- Arlington, MA USA 02174
-- Web: http://www.updatestage.com
-- Email: sales@updatestage.com, support@updatestage.com
-- Voice: 781-641-6043
-- Fax: 781-641-7068
-- Email/phone/fax support hours: 10AM - 6PM US EST
-- 
-- Developed by:
-- 
-- Glenn M. Picher
-- Dirigo Multimedia
-- 142 High Street, Suite 321
-- Portland, ME USA 04101
-- Web: http://www.maine.com/shops/gpicher
-- Email: gpicher@maine.com
-- Voice: 207-761-6535
-- Fax: 207-775-4372
-- 
new object me
-- Create new object. Multiple files, each with their own object, can be
-- used at the same time.
openFile object me, integer permissions, string fileName, *
-- Opens file 'fileName' (which should include a full pathname). If 'permissions' is 1,
-- file is opened for read only. If 'permissions' is 2, file is opened for both read and
-- write (and will be created if it does not exist). Returns 'OK' or string with word 1
-- 'Error:' . On the Mac, you can also supply a 4-character string for the Macintosh Finder
-- creator signature, and another 4-character string for the Mac Finder file type, which
-- are used if the file doesn't exist and must be opened for writing. The two Mac strings
-- are ignored if supplied on Windows, and are not required on either platform. Defaults
-- on the Mac are for SimpleText (signature 'ttxt', type 'TEXT') when a file is created.
getFileSize object me
-- Returns total bytes in the file as an integer, or a string with word 1 'Error:' .
setFileSize object me, integer newSize
-- Sets total bytes in the file to 'newSize'. Returns 'OK' or a string with word 1 'Error:' .
-- Note that writeBytes() will automatically enlarge the file size if necessary; setFileSize()
-- is just useful for reserving all required disk space in advance, or shrinking the file.
-- Note that added bytes are garbage values. They are not set to any value.
getFilePosition object me
-- Returns current read/write position as an integer, or a string with word 1 'Error:'.
-- The first byte in the file is at position 0. Maximum value is what getFileSize()
-- returns; that value would mean that no more bytes can be read.
setFilePosition object me, integer newPosition
-- Sets the current read/write position in the file to 'newPosition'. The first byte in
-- the file is at position 0. Returns 'OK' or a string with word 1 'Error:'. Maximum value
-- for newPosition is what getFileSize() returns; that value would mean new
-- bytes will be added to the end of the file on the next writeBytes() .
readBytes object me, integer byteCount
-- Reads 'byteCount' bytes from the file, starting at the current file position. Returns
-- a symbol if there is an error, or a string with the data. Note that the string can
-- contain numToChar(0) characters, which Lingo does not always  handle well. Use Lingo
-- syntax like 'the number of chars in s' and 'char 23 of s' to get all the information
-- from this string. Note: on Windows 3.1, Lingo strings are limited to 64K; maximum value
-- for byteCount on Windows 3.1 is therefore 64000 bytes. For identical behavior on all
-- platforms, this 64000 byte limit is also enforced on all other platforms, but can be
-- relaxed for better performance if desired with the 'relax64kLimit' handler.
writeBytes object me, string binaryData
-- Writes the string to the file, starting at the current file position. The number of
-- bytes written depends on the size of the string. If you must write numToChar(0)
-- characters as part of the string, use Lingo syntax like...
--     set s = "xxxxx"
--     put numToChar(0) into char 3 of s
-- Returns 'OK' or a string with word 1 'Error:' . Will enlarge file size if necessary.
-- Note: on Windows 3.1, Lingo strings are limited to 64K; maximum 'number of chars in
-- binaryData' on Windows 3.1 is therefore 64000 bytes. For identical behavior on all
-- platforms, this 64000 byte limit is also enforced on all other platforms, but can be
-- relaxed for better performance if desired with the 'relax64kLimit' handler.
relax64kLimit object me
enforce64kLimit object me
-- Relaxes or enforces a 64000 byte limit on read and write sizes on all platforms. On Win 3.1,
-- a 64000 byte limit is *always* in effect, regardless of these methods. On other platforms,
-- by default when an object is created, the limit is enforced. For better performance,
-- at the cost of having to write more complicated multiplatform Lingo, you can relax the limit
-- on Windows 95, Windows NT, and the Mac. Use Lingo's 'the platform' to decide what to do.
writeChar object me, integer theValue
-- Writes a byte. Values -128 to -1 are written as signed bytes; values 0 to
-- 255 are written as unsigned bytes. (Values 0 to 127 are written identically in
-- signed and unsigned byte formats.) Returns 'OK' or string with word 1 'Error'.
readSignedChar object me
-- Reads a byte and returns it as an integer -128 to 127, or a string with
-- word 1 'Error:'.
readUnsignedChar object me
-- Reads a byte and returns it as an integer 0 to 255, or a string with
-- word 1 'Error:'.
usePlatformByteOrder object me
-- Uses Mac standard byte order on the Mac, Windows standard byte order on Windows,
-- for short, long, and float reads and writes. This is the default for new objects.
useMacByteOrder object me
-- Uses Mac standard byte order for short, long, and float reads and writes.
useWinByteOrder object me
-- Uses Windows standard byte order for short, long, and float reads and writes.
writeShort object me, integer theValue
-- Writes a two-byte short integer value. Values -32768 to -1 are written as
-- signed shorts; values 0 to 65535 are written as unsigned shorts. (Values 0 to 32767
-- are written identically in signed and unsigned short formats.) Returns 'OK' or string
-- with word 1 'Error:'.
readSignedShort object me
-- Reads a two-byte short integer value and returns it as an integer -32768 to 32767,
-- or a string with word 1 'Error:'.
readUnsignedShort object me
-- Reads a two-byte short integer value and returns it as an integer 0 to 65535,
-- or a string with word 1 'Error:'.
writeSignedLong object me, integer theValue
-- Writes a four-byte signed long integer value in the range -2147483648 to 2147483647.
-- (Values 0 to 2147483647 are written identically in signed and unsigned long formats.)
-- Returns 'OK' or a string with word 1 'Error:'
writeUnsignedLong object me, any theValue
-- Writes a four-byte unsigned long integer value in the range 0 to 4294967295.
-- (Values 0 to 2147483647 are written identically in signed and unsigned long formats.)
-- Note that all numbers larger than 2147483647 must be provided as float values,
-- not as integer values, due to Director's limits on 'the maxInteger'. Returns 'OK'
-- or a string with word 1 'Error:' .
readSignedLong object me
-- Reads a four-byte signed long integer value and returns it as an integer in the
-- range -2147483648 to 2147483647, or a string with word 1 'Error:' .
readUnsignedLong object me
-- Reads a four-byte unsigned long integer value and returns it as an integer in the
-- range 0 to 2147483647 or as a float in the range 2147483648.0 to 4294967295.0, or a 
-- string with word 1 'Error:' .
writeFloat object me, float theValue
-- Writes an eight-byte floating point value (using the 64-bit IEEE-754 format). Returns
-- 'OK' or a string with word 1 'Error:' .
readFloat object me
-- Reads an eight-byte floating point value (using the 64-bit IEEE-754 format) and returns
-- a float value or a string with word 1 'Error:' .
writeFloat32 object me, float theValue
-- Writes a four-byte floating point value (using the 32-bit IEEE-754 format). Returns
-- 'OK' or a string with word 1 'Error:' Note that precision may be lost!
readFloat32 object me
-- Reads a four-byte floating point value (using the 32-bit IEEE-754 format) and returns
-- a float value or a string with word 1 'Error:' .
closeFile object me
-- Closes the file. Returns 'OK' or string with word 1 'Error:' .
--
* bnioGetFileCreationTime string fileName
-- Returns the file's creation date and time stamp in the format 'yyyy/mm/dd hh:mm:ss',
-- or a string with word 1 'Error:' .
* bnioSetFileCreationTime string fileName, string timeStamp
-- Sets the file's creation date and time stamp in the format 'yyyy/mm/dd hh:mm:ss'.
-- Returns 'OK' or a string with word 1 'Error:'.
* bnioGetFileModificationTime string fileName
-- Returns the file's modification date and time stamp in the format 'yyyy/mm/dd hh:mm:ss',
-- or a string with word 1 'Error:' .
* bnioSetFileModificationTime string fileName, string timeStamp
-- Sets the file's modification date and time stamp in the format 'yyyy/mm/dd hh:mm:ss'.
-- Returns 'OK' or a string with word 1 'Error:'.
* bnioDeleteFile string fileName
-- Deletes a file. Returns 'OK' or a string with word 1 'Error:' .
--
+ register object xtraReference, any registrationCode
-- A copyright alert will be shown once per Director session (or once per openxlib/closexlib)
-- if the Xtra has never been registered with Lingo syntax like this....
--   register(xtra "BinaryIO", "your serial number here")
-- ...but, of course, you have to supply an actually valid serial number!
-- There will be a ten second delay if you get the registration code incorrect.
-- Returns 'OK' or a string with word 1 'Error:' .

 */

namespace Director {

const char *BinaryioXtra::xlibName = "Binaryio";
const XlibFileDesc BinaryioXtra::fileNames[] = {
	{ "binaryio",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BinaryioXtra::m_new,		 0, 0,	600 },
	{ "openFile",				BinaryioXtra::m_openFile,		 -1, 0,	600 },
	{ "getFileSize",				BinaryioXtra::m_getFileSize,		 0, 0,	600 },
	{ "setFileSize",				BinaryioXtra::m_setFileSize,		 1, 0,	600 },
	{ "getFilePosition",				BinaryioXtra::m_getFilePosition,		 0, 0,	600 },
	{ "setFilePosition",				BinaryioXtra::m_setFilePosition,		 1, 0,	600 },
	{ "readBytes",				BinaryioXtra::m_readBytes,		 1, 0,	600 },
	{ "writeBytes",				BinaryioXtra::m_writeBytes,		 1, 0,	600 },
	{ "relax64kLimit",				BinaryioXtra::m_relax64kLimit,		 0, 0,	600 },
	{ "enforce64kLimit",				BinaryioXtra::m_enforce64kLimit,		 0, 0,	600 },
	{ "writeChar",				BinaryioXtra::m_writeChar,		 1, 0,	600 },
	{ "readSignedChar",				BinaryioXtra::m_readSignedChar,		 0, 0,	600 },
	{ "readUnsignedChar",				BinaryioXtra::m_readUnsignedChar,		 0, 0,	600 },
	{ "usePlatformByteOrder",				BinaryioXtra::m_usePlatformByteOrder,		 0, 0,	600 },
	{ "useMacByteOrder",				BinaryioXtra::m_useMacByteOrder,		 0, 0,	600 },
	{ "useWinByteOrder",				BinaryioXtra::m_useWinByteOrder,		 0, 0,	600 },
	{ "writeShort",				BinaryioXtra::m_writeShort,		 1, 0,	600 },
	{ "readSignedShort",				BinaryioXtra::m_readSignedShort,		 0, 0,	600 },
	{ "readUnsignedShort",				BinaryioXtra::m_readUnsignedShort,		 0, 0,	600 },
	{ "writeSignedLong",				BinaryioXtra::m_writeSignedLong,		 1, 0,	600 },
	{ "writeUnsignedLong",				BinaryioXtra::m_writeUnsignedLong,		 1, 0,	600 },
	{ "readSignedLong",				BinaryioXtra::m_readSignedLong,		 0, 0,	600 },
	{ "readUnsignedLong",				BinaryioXtra::m_readUnsignedLong,		 0, 0,	600 },
	{ "writeFloat",				BinaryioXtra::m_writeFloat,		 1, 0,	600 },
	{ "readFloat",				BinaryioXtra::m_readFloat,		 0, 0,	600 },
	{ "writeFloat32",				BinaryioXtra::m_writeFloat32,		 1, 0,	600 },
	{ "readFloat32",				BinaryioXtra::m_readFloat32,		 0, 0,	600 },
	{ "closeFile",				BinaryioXtra::m_closeFile,		 0, 0,	600 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "bnioGetFileCreationTime", BinaryioXtra::m_bnioGetFileCreationTime, 1, 1, 600, HBLTIN },
	{ "bnioSetFileCreationTime", BinaryioXtra::m_bnioSetFileCreationTime, 2, 2, 600, HBLTIN },
	{ "bnioGetFileModificationTime", BinaryioXtra::m_bnioGetFileModificationTime, 1, 1, 600, HBLTIN },
	{ "bnioSetFileModificationTime", BinaryioXtra::m_bnioSetFileModificationTime, 2, 2, 600, HBLTIN },
	{ "bnioDeleteFile", BinaryioXtra::m_bnioDeleteFile, 1, 1, 600, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BinaryioXtraObject::BinaryioXtraObject(ObjectType ObjectType) :Object<BinaryioXtraObject>("Binaryio") {
	_objType = ObjectType;
}

bool BinaryioXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BinaryioXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BinaryioXtra::xlibName);
	warning("BinaryioXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BinaryioXtra::open(ObjectType type, const Common::Path &path) {
    BinaryioXtraObject::initMethods(xlibMethods);
    BinaryioXtraObject *xobj = new BinaryioXtraObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BinaryioXtra::close(ObjectType type) {
    BinaryioXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BinaryioXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BinaryioXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BinaryioXtra::m_openFile, 0)
XOBJSTUB(BinaryioXtra::m_getFileSize, 0)
XOBJSTUB(BinaryioXtra::m_setFileSize, 0)
XOBJSTUB(BinaryioXtra::m_getFilePosition, 0)
XOBJSTUB(BinaryioXtra::m_setFilePosition, 0)
XOBJSTUB(BinaryioXtra::m_readBytes, 0)
XOBJSTUB(BinaryioXtra::m_writeBytes, 0)
XOBJSTUB(BinaryioXtra::m_relax64kLimit, 0)
XOBJSTUB(BinaryioXtra::m_enforce64kLimit, 0)
XOBJSTUB(BinaryioXtra::m_writeChar, 0)
XOBJSTUB(BinaryioXtra::m_readSignedChar, 0)
XOBJSTUB(BinaryioXtra::m_readUnsignedChar, 0)
XOBJSTUB(BinaryioXtra::m_usePlatformByteOrder, 0)
XOBJSTUB(BinaryioXtra::m_useMacByteOrder, 0)
XOBJSTUB(BinaryioXtra::m_useWinByteOrder, 0)
XOBJSTUB(BinaryioXtra::m_writeShort, 0)
XOBJSTUB(BinaryioXtra::m_readSignedShort, 0)
XOBJSTUB(BinaryioXtra::m_readUnsignedShort, 0)
XOBJSTUB(BinaryioXtra::m_writeSignedLong, 0)
XOBJSTUB(BinaryioXtra::m_writeUnsignedLong, 0)
XOBJSTUB(BinaryioXtra::m_readSignedLong, 0)
XOBJSTUB(BinaryioXtra::m_readUnsignedLong, 0)
XOBJSTUB(BinaryioXtra::m_writeFloat, 0)
XOBJSTUB(BinaryioXtra::m_readFloat, 0)
XOBJSTUB(BinaryioXtra::m_writeFloat32, 0)
XOBJSTUB(BinaryioXtra::m_readFloat32, 0)
XOBJSTUB(BinaryioXtra::m_closeFile, 0)
XOBJSTUB(BinaryioXtra::m_bnioGetFileCreationTime, 0)
XOBJSTUB(BinaryioXtra::m_bnioSetFileCreationTime, 0)
XOBJSTUB(BinaryioXtra::m_bnioGetFileModificationTime, 0)
XOBJSTUB(BinaryioXtra::m_bnioSetFileModificationTime, 0)
XOBJSTUB(BinaryioXtra::m_bnioDeleteFile, 0)
XOBJSTUB(BinaryioXtra::m_register, 0)

}
