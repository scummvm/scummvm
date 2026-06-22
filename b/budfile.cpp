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
#include "director/lingo/xtras/b/budfile.h"

/**************************************************
 *
 * USED IN:
 * Computer Classroom Tackling Tables 2
 * It's The Big Game, Charlie Brown!
 * Virtual-LOI Craft
 *
 **************************************************/

/*
-- xtra BudFile
new object me

* baFileResult -- returns result of last function

-- Text file functions
* baReadFile string FileName, * -- reads entire text file, with optional encryption
* baWriteFile string FileName, string Text, * -- writes entire text file, with optional encryption
* baAppendFile string FileName, string Text -- appends text to end of text file
* baInsertFile string FileName, string Text -- inserts text at start of text file

-- Binary file functions
* baReadBinFile string FileName -- reads entire binary file
* baWriteBinFile string FileName, list Data -- writes entire binary file
* baAppendBinFile string FileName, list Data -- appends binary data to end of file
* baInsertBinFile string FileName, list Data -- inserts binary data to start of file

-- List file functions
* baReadList string FileName, * -- reads list from disk, with optional encryption
* baWriteList string FileName, list Data, * -- writes list to disk, with optional encryption

-- BlowFish encryption functions
* baEncryptBF string FileName, string Key, * -- encrypts file, optionally to new file
* baDecryptBF string FileName, string Key, * -- decrypts file, optionally to new file
* baReadFileBF string FileName, string Key -- reads entire encrypted text file
* baWriteFileBF string FileName, string Text, Key -- writes entire encrypted text file
* baIsEncryptedBF string FileName -- returns 1 if file is encrypted

-- File reading/writing functions
* baGetFile string FileName, string Mode -- opens file
* baMovePointer integer fileNum, integer move, string from -- moves file pointer
* baGetPointer integer fileNum -- returns current file pointer position
* baSeekTo integer fileNum, string Text -- moves pointer to start of Text
* baSeekAfter integer fileNum, string Text -- moves pointer after Text
* baClearFile integer fileNum -- deletes everything from file
* baEndFile integer fileNum -- deletes everything after current pointer position
* baCloseFile integer fileNum -- closes file

* baReadText integer fileNum, integer Characters -- reads text from file
* baWriteText integer fileNum, string Text -- writes text to file
* baInsertText integer fileNum, string Text -- inserts text into file

* baReturnStyle string type -- sets the return character style
* baReadLine integer fileNum -- reads next line from file
* baWriteLine integer fileNum, string Text -- writes line to file
* baInsertLine integer fileNum, string Text -- inserts line into file
* baReadChunk integer fileNum, string Chunk -- reads the next chunk from the file

* baReadBin integer fileNum, integer Bytes -- reads data from file
* baWriteBin integer fileNum, list Data -- writes data to file
* baInsertBin integer fileNum, list Data -- inserts data into file

* baByteOrder string order -- sets byte order to use
* baReadByte integer fileNum -- reads signed byte from file
* baReadUByte integer fileNum -- reads unsigned byte from file
* baWriteByte integer fileNum, integer Data -- writes signed byte to file
* baWriteUByte integer fileNum, integer Data -- writes unsigned byte to file
* baInsertByte integer fileNum, integer Data -- inserts signed byte to file
* baInsertUByte integer fileNum, integer Data -- inserts unsigned byte to file
* baReadShort integer fileNum -- reads signed short from file
* baReadUShort integer fileNum -- reads unsigned short from file
* baWriteShort integer fileNum, integer Data -- writes signed short to file
* baWriteUShort integer fileNum, integer Data -- writes unsigned short to file
* baInsertShort integer fileNum, integer Data -- inserts signed short to file
* baInsertUShort integer fileNum, integer Data -- inserts unsigned short to file
* baReadLong integer fileNum -- reads signed long from file
* baReadULong integer fileNum -- reads unsigned long from file
* baWriteLong integer fileNum, integer Data -- writes signed long to file
* baWriteULong integer fileNum, float Data -- writes unsigned long to file
* baInsertLong integer fileNum, integer Data -- inserts signed long to file
* baInsertULong integer fileNum, float Data -- inserts unsigned long to file


-- Version 2.1
-- Copyright 2003-2005 Magic Modules Pty Ltd
-- http://www.mods.com.au
-- buddy@mods.com.au

"
 */

namespace Director {

const char *BudfileXtra::xlibName = "Budfile";
const XlibFileDesc BudfileXtra::fileNames[] = {
	{ "budfile",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BudfileXtra::m_new,		 0, 0,	500 },
	{ """,				BudfileXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "baFileResult", BudfileXtra::m_baFileResult, 0, 0, 500, HBLTIN },
	{ "baReadFile", BudfileXtra::m_baReadFile, -1, 0, 500, HBLTIN },
	{ "baWriteFile", BudfileXtra::m_baWriteFile, -1, 0, 500, HBLTIN },
	{ "baAppendFile", BudfileXtra::m_baAppendFile, 2, 2, 500, HBLTIN },
	{ "baInsertFile", BudfileXtra::m_baInsertFile, 2, 2, 500, HBLTIN },
	{ "baReadBinFile", BudfileXtra::m_baReadBinFile, 1, 1, 500, HBLTIN },
	{ "baWriteBinFile", BudfileXtra::m_baWriteBinFile, 2, 2, 500, HBLTIN },
	{ "baAppendBinFile", BudfileXtra::m_baAppendBinFile, 2, 2, 500, HBLTIN },
	{ "baInsertBinFile", BudfileXtra::m_baInsertBinFile, 2, 2, 500, HBLTIN },
	{ "baReadList", BudfileXtra::m_baReadList, -1, 0, 500, HBLTIN },
	{ "baWriteList", BudfileXtra::m_baWriteList, -1, 0, 500, HBLTIN },
	{ "baEncryptBF", BudfileXtra::m_baEncryptBF, -1, 0, 500, HBLTIN },
	{ "baDecryptBF", BudfileXtra::m_baDecryptBF, -1, 0, 500, HBLTIN },
	{ "baReadFileBF", BudfileXtra::m_baReadFileBF, 2, 2, 500, HBLTIN },
	{ "baWriteFileBF", BudfileXtra::m_baWriteFileBF, 3, 3, 500, HBLTIN },
	{ "baIsEncryptedBF", BudfileXtra::m_baIsEncryptedBF, 1, 1, 500, HBLTIN },
	{ "baGetFile", BudfileXtra::m_baGetFile, 2, 2, 500, HBLTIN },
	{ "baMovePointer", BudfileXtra::m_baMovePointer, 3, 3, 500, HBLTIN },
	{ "baGetPointer", BudfileXtra::m_baGetPointer, 1, 1, 500, HBLTIN },
	{ "baSeekTo", BudfileXtra::m_baSeekTo, 2, 2, 500, HBLTIN },
	{ "baSeekAfter", BudfileXtra::m_baSeekAfter, 2, 2, 500, HBLTIN },
	{ "baClearFile", BudfileXtra::m_baClearFile, 1, 1, 500, HBLTIN },
	{ "baEndFile", BudfileXtra::m_baEndFile, 1, 1, 500, HBLTIN },
	{ "baCloseFile", BudfileXtra::m_baCloseFile, 1, 1, 500, HBLTIN },
	{ "baReadText", BudfileXtra::m_baReadText, 2, 2, 500, HBLTIN },
	{ "baWriteText", BudfileXtra::m_baWriteText, 2, 2, 500, HBLTIN },
	{ "baInsertText", BudfileXtra::m_baInsertText, 2, 2, 500, HBLTIN },
	{ "baReturnStyle", BudfileXtra::m_baReturnStyle, 1, 1, 500, HBLTIN },
	{ "baReadLine", BudfileXtra::m_baReadLine, 1, 1, 500, HBLTIN },
	{ "baWriteLine", BudfileXtra::m_baWriteLine, 2, 2, 500, HBLTIN },
	{ "baInsertLine", BudfileXtra::m_baInsertLine, 2, 2, 500, HBLTIN },
	{ "baReadChunk", BudfileXtra::m_baReadChunk, 2, 2, 500, HBLTIN },
	{ "baReadBin", BudfileXtra::m_baReadBin, 2, 2, 500, HBLTIN },
	{ "baWriteBin", BudfileXtra::m_baWriteBin, 2, 2, 500, HBLTIN },
	{ "baInsertBin", BudfileXtra::m_baInsertBin, 2, 2, 500, HBLTIN },
	{ "baByteOrder", BudfileXtra::m_baByteOrder, 1, 1, 500, HBLTIN },
	{ "baReadByte", BudfileXtra::m_baReadByte, 1, 1, 500, HBLTIN },
	{ "baReadUByte", BudfileXtra::m_baReadUByte, 1, 1, 500, HBLTIN },
	{ "baWriteByte", BudfileXtra::m_baWriteByte, 2, 2, 500, HBLTIN },
	{ "baWriteUByte", BudfileXtra::m_baWriteUByte, 2, 2, 500, HBLTIN },
	{ "baInsertByte", BudfileXtra::m_baInsertByte, 2, 2, 500, HBLTIN },
	{ "baInsertUByte", BudfileXtra::m_baInsertUByte, 2, 2, 500, HBLTIN },
	{ "baReadShort", BudfileXtra::m_baReadShort, 1, 1, 500, HBLTIN },
	{ "baReadUShort", BudfileXtra::m_baReadUShort, 1, 1, 500, HBLTIN },
	{ "baWriteShort", BudfileXtra::m_baWriteShort, 2, 2, 500, HBLTIN },
	{ "baWriteUShort", BudfileXtra::m_baWriteUShort, 2, 2, 500, HBLTIN },
	{ "baInsertShort", BudfileXtra::m_baInsertShort, 2, 2, 500, HBLTIN },
	{ "baInsertUShort", BudfileXtra::m_baInsertUShort, 2, 2, 500, HBLTIN },
	{ "baReadLong", BudfileXtra::m_baReadLong, 1, 1, 500, HBLTIN },
	{ "baReadULong", BudfileXtra::m_baReadULong, 1, 1, 500, HBLTIN },
	{ "baWriteLong", BudfileXtra::m_baWriteLong, 2, 2, 500, HBLTIN },
	{ "baWriteULong", BudfileXtra::m_baWriteULong, 2, 2, 500, HBLTIN },
	{ "baInsertLong", BudfileXtra::m_baInsertLong, 2, 2, 500, HBLTIN },
	{ "baInsertULong", BudfileXtra::m_baInsertULong, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BudfileXtraObject::BudfileXtraObject(ObjectType ObjectType) :Object<BudfileXtraObject>("Budfile") {
	_objType = ObjectType;
}

bool BudfileXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BudfileXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BudfileXtra::xlibName);
	warning("BudfileXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BudfileXtra::open(ObjectType type, const Common::Path &path) {
    BudfileXtraObject::initMethods(xlibMethods);
    BudfileXtraObject *xobj = new BudfileXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BudfileXtra::close(ObjectType type) {
    BudfileXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BudfileXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BudfileXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BudfileXtra::m_baFileResult, 0)
XOBJSTUB(BudfileXtra::m_baReadFile, 0)
XOBJSTUB(BudfileXtra::m_baWriteFile, 0)
XOBJSTUB(BudfileXtra::m_baAppendFile, 0)
XOBJSTUB(BudfileXtra::m_baInsertFile, 0)
XOBJSTUB(BudfileXtra::m_baReadBinFile, 0)
XOBJSTUB(BudfileXtra::m_baWriteBinFile, 0)
XOBJSTUB(BudfileXtra::m_baAppendBinFile, 0)
XOBJSTUB(BudfileXtra::m_baInsertBinFile, 0)
XOBJSTUB(BudfileXtra::m_baReadList, 0)
XOBJSTUB(BudfileXtra::m_baWriteList, 0)
XOBJSTUB(BudfileXtra::m_baEncryptBF, 0)
XOBJSTUB(BudfileXtra::m_baDecryptBF, 0)
XOBJSTUB(BudfileXtra::m_baReadFileBF, 0)
XOBJSTUB(BudfileXtra::m_baWriteFileBF, 0)
XOBJSTUB(BudfileXtra::m_baIsEncryptedBF, 0)
XOBJSTUB(BudfileXtra::m_baGetFile, 0)
XOBJSTUB(BudfileXtra::m_baMovePointer, 0)
XOBJSTUB(BudfileXtra::m_baGetPointer, 0)
XOBJSTUB(BudfileXtra::m_baSeekTo, 0)
XOBJSTUB(BudfileXtra::m_baSeekAfter, 0)
XOBJSTUB(BudfileXtra::m_baClearFile, 0)
XOBJSTUB(BudfileXtra::m_baEndFile, 0)
XOBJSTUB(BudfileXtra::m_baCloseFile, 0)
XOBJSTUB(BudfileXtra::m_baReadText, 0)
XOBJSTUB(BudfileXtra::m_baWriteText, 0)
XOBJSTUB(BudfileXtra::m_baInsertText, 0)
XOBJSTUB(BudfileXtra::m_baReturnStyle, 0)
XOBJSTUB(BudfileXtra::m_baReadLine, 0)
XOBJSTUB(BudfileXtra::m_baWriteLine, 0)
XOBJSTUB(BudfileXtra::m_baInsertLine, 0)
XOBJSTUB(BudfileXtra::m_baReadChunk, 0)
XOBJSTUB(BudfileXtra::m_baReadBin, 0)
XOBJSTUB(BudfileXtra::m_baWriteBin, 0)
XOBJSTUB(BudfileXtra::m_baInsertBin, 0)
XOBJSTUB(BudfileXtra::m_baByteOrder, 0)
XOBJSTUB(BudfileXtra::m_baReadByte, 0)
XOBJSTUB(BudfileXtra::m_baReadUByte, 0)
XOBJSTUB(BudfileXtra::m_baWriteByte, 0)
XOBJSTUB(BudfileXtra::m_baWriteUByte, 0)
XOBJSTUB(BudfileXtra::m_baInsertByte, 0)
XOBJSTUB(BudfileXtra::m_baInsertUByte, 0)
XOBJSTUB(BudfileXtra::m_baReadShort, 0)
XOBJSTUB(BudfileXtra::m_baReadUShort, 0)
XOBJSTUB(BudfileXtra::m_baWriteShort, 0)
XOBJSTUB(BudfileXtra::m_baWriteUShort, 0)
XOBJSTUB(BudfileXtra::m_baInsertShort, 0)
XOBJSTUB(BudfileXtra::m_baInsertUShort, 0)
XOBJSTUB(BudfileXtra::m_baReadLong, 0)
XOBJSTUB(BudfileXtra::m_baReadULong, 0)
XOBJSTUB(BudfileXtra::m_baWriteLong, 0)
XOBJSTUB(BudfileXtra::m_baWriteULong, 0)
XOBJSTUB(BudfileXtra::m_baInsertLong, 0)
XOBJSTUB(BudfileXtra::m_baInsertULong, 0)
XOBJSTUB(BudfileXtra::m_", 0)

}
