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

/*************************************
 *
 * USED IN:
 * Standard Director Xtra
 * Karma: Curse of the 12 Caves
 *
 *************************************/

/*
 * -- XObject version
 *
 * --FileIO, Tool, 1.5.0 , 31mar92
 * --© 1989-1992 MacroMind, Inc.
 * --by John Thompson and Al McNeil
 * ---------------------------------------------------------------------
 * ---------------------------------------------------------------------
 * --=METHODS=--
 * --
 * ISS mNew, mode, fileNameOrType  --Creates a new instance of the XObject.
 * --  Mode can be :
 * --    "read"  - Read "fileName"
 * --    "?read"   - Select and Read "fileType"
 * --    "write"   - Write "fileName"
 * --    "?write"  - Select and Write "fileName"
 * --    "append"  - Append "fileName"
 * --    "?append" - Select and Append "fileName"
 * --  FileType for ?read can be :
 * --    "TEXT" - standard file type
 * --    "trak" - cd track type
 * --    etc... - Any four character combination.
 * --
 * X mDispose  --Disposes of XObject instance.
 * S mName --Returns the name of the XObject.
 * /IXX  mWrite, countPtr, bufferPtr --Writes out a set of chars. Returns error code.
 * II  mWriteChar, charNum --Writes a single character. Returns error code.
 * IS  mWriteString, string  --Writes out a string of chars. Returns error code.
 * /IXX  mRead, countPtr, bufferPtr  --Reads into buffer. Returns error code.
 * I mReadChar   --Returns a single character.
 * S mReadWord   --Returns the next word of an input file.
 * S mReadLine --Returns the next line of an input file.
 * S mReadFile --Returns the remainder of the file.
 * --
 * SSS mReadToken, breakString, skipString
 * --      --breakstring designates character (or token) that signals to stop reading.
 * --      --skipstring designates what characters (or tokens) not to read.
 * I mGetPosition  --Returns the file position.
 * II  mSetPosition, newPos  --Sets the file position. Returns error code.
 * I mGetLength  --Returns the number of chars in the file.
 * ISS mSetFinderInfo, typeString, creatorString --Sets the finder info. Returns error code.
 * S mGetFinderInfo  --Gets the finder info.
 * S mFileName --Returns the name of the file.
 * S mNativeFileName --Returns the name of the file.
 * I mDelete   --Delete the file and dispose of me.
 * I     mStatus  --Returns result code of the last file io activity
 * --
 * SI  +mError, errorCode  --Returns error message string.
 * -- Possible error codes:
 * --  -33 :: File directory full
 * --  -34 :: Volume full
 * --  -35 :: Volume not found
 * --  -36 :: I/O Error
 * --  -37 :: Bad file name
 * --  -38 :: File not open
 * --  -42 :: Too many files open
 * --  -43 :: File not found
 * --  -56 :: No such drive
 * --  -65 :: No disk in drive
 * --  -120 :: Directory not found
 * V     mReadPICT
 * II     +mSetOverrideDrive, driveLetter --Set override drive letter ('A' - 'Z') to use when loading linked castmembers.  Use 0x00 to clear override.
 *
 * -- Xtra version
-- xtra fileio -- CH May96
new object me -- create a new child instance
-- FILEIO --
fileName object me -- return fileName string of the open file
status object me -- return the error code of the last method called
error object me, int error -- return the error string of the error
setFilterMask me, string mask -- set the filter mask for dialogs
openFile object me, string fileName, int mode -- opens named file. valid modes: 0=r/w 1=r 2=w
closeFile object me -- close the file
displayOpen object me -- displays an open dialog and returns the selected fileName to lingo
displaySave object me, string title, string defaultFileName -- displays save dialog and returns selected fileName to lingo
createFile object me, string fileName -- creates a new file called fileName
setPosition object me, int position -- set the file position
getPosition object me -- get the file position
getLength object me -- get the length of the open file
writeChar object me, string theChar -- write a single character (by ASCII code) to the file
writeString object me, string theString -- write a null-terminated string to the file
readChar object me -- read the next character of the file and return it as an ASCII code value
readLine object me -- read the next line of the file (including the next RETURN) and return as a string
readFile object me -- read from current position to EOF and return as a string
readWord object me -- read the next word of the file and return it as a string
readToken object me, string skip, string break -- read the next token and return it as a string
getFinderInfo object me -- get the finder info for the open file (Mac Only)
setFinderInfo object me, string attributes -- set the finder info for the open file (Mac Only)
delete object me -- deletes the open file
+ version xtraRef -- display fileIO version and build information in the message window
* getOSDirectory -- returns the full path to the Mac System Folder or Windows Directory

 */

#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"

#include "director/director.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/fileio.h"

namespace Director {

const char *const FileIO::xlibName = "FileIO";
const XlibFileDesc FileIO::fileNames[] = {
	{ "FileIO",		nullptr },
	{ "shFILEIO",	nullptr }, // TD loads this up using openXLib("@:shFILEIO.DLL")
	{ "FILE",		nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	// XObject
	{ "delete",					FileIO::m_delete,			 0, 0,	200 },	// D2
	{ "error",					FileIO::m_error,			 1, 1,	200 },	// D2
	{ "fileName",				FileIO::m_fileName,			 0, 0,	200 },	// D2
	{ "getFinderInfo",			FileIO::m_getFinderInfo,	 0, 0,	200 },	// D2
	{ "getLength",				FileIO::m_getLength,		 0, 0,	200 },	// D2
	{ "getPosition",			FileIO::m_getPosition,		 0, 0,	200 },	// D2
	{ "new",					FileIO::m_new,				 0, 2,	200 },	// D2
	{ "readChar",				FileIO::m_readChar,			 0, 0,	200 },	// D2
	{ "readFile",				FileIO::m_readFile,			 0, 0,	200 },	// D2
	{ "readLine",				FileIO::m_readLine,			 0, 0,	200 },	// D2
	{ "readToken",				FileIO::m_readToken,		 2, 2,	200 },	// D2
	{ "readPict",				FileIO::m_readPict,			 0, 0,	300 },	// D3
	{ "readWord",				FileIO::m_readWord,			 0, 0,	200 },	// D2
	{ "setFinderInfo",			FileIO::m_setFinderInfo,	 2, 2,	200 },	// D2
	{ "setPosition",			FileIO::m_setPosition,		 1, 1,	200 },	// D2
	{ "status",					FileIO::m_status,			 0, 0,	200 },	// D2
	{ "writeChar",				FileIO::m_writeChar,		 1, 1,	200 },	// D2
	{ "writeString",			FileIO::m_writeString,		 1, 1,	200 },	// D2
	// Windows only?
	{ "setOverrideDrive",		FileIO::m_setOverrideDrive,	 1, 1,	300 },	// D3
	// Xtra
	{ "closeFile",				FileIO::m_closeFile,		 0, 0,	500 },	// D5
	{ "createFile",				FileIO::m_createFile,		 1, 1,	500 },	// D5
	{ "displayOpen",			FileIO::m_displayOpen,		 0, 0,	500 },	// D5
	{ "displaySave",			FileIO::m_displaySave,		 2, 2,	500 },	// D5
	{ "openFile",				FileIO::m_openFile,			 2, 2,	500 },	// D5
	{ "setFilterMask",			FileIO::m_setFilterMask,	 1, 1,	500 },  // D5
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ "getOSDirectory", FileIO::m_getOSDirectory, 0, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};


void FileIO::open(ObjectType type, const Common::Path &path) {
	FileObject::initMethods(xlibMethods);
	FileObject *xobj = new FileObject(type);
	if (g_director->getVersion() >= 500)
		g_lingo->_openXtras.push_back(xlibName);
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void FileIO::close(ObjectType type) {
	FileObject::cleanupMethods();
	g_lingo->_globalvars[xlibName] = Datum();
}

// Initialization/disposal

FileObject::FileObject(ObjectType objType) : Object<FileObject>("FileIO") {
	_objType = objType;
	_filename = nullptr;
	_inStream = nullptr;
	_outFile = nullptr;
	_outStream = nullptr;
	_lastError = kErrorNone;
}

FileObject::FileObject(const FileObject &obj) : Object<FileObject>(obj) {
	_objType = obj.getObjType();
	_filename = nullptr;
	_inStream = nullptr;
	_outFile = nullptr;
	_outStream = nullptr;
	_lastError = kErrorNone;
}

FileObject::~FileObject() {
	clear();
}

bool FileObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum FileObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(FileIO::xlibName);
	warning("FileIO::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

FileIOError FileObject::open(const Common::String &origpath, const Common::String &mode) {
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	Common::String path = origpath;
	Common::String option = mode;
	char dirSeparator = g_director->_dirSeparator;

	Common::String prefix = savePrefix();

	if (option.hasPrefix("?")) {
		option = option.substr(1);
		path = getFileNameFromModal(option.equalsIgnoreCase("write"), origpath, Common::String(), "txt");
		if (path.empty()) {
			return kErrorFileNotFound;
		}
		dirSeparator = '/';
	} else if (!path.hasSuffixIgnoreCase(".txt")) {
		path += ".txt";
	}

	// Enforce target to the created files so they do not mix up
	Common::String filenameOrig = lastPathComponent(path, dirSeparator);

	Common::String filename = filenameOrig;
	if (!filename.hasPrefixIgnoreCase(prefix))
		filename = prefix + filenameOrig;

	if (option.equalsIgnoreCase("read")) {
		_inStream = saves->openForLoading(filename);
		if (!_inStream) {
			// Maybe we're trying to read one of the game files
			Common::File *f = new Common::File;
			Common::Path location = findPath(origpath);
			if (location.empty() || !f->open(location)) {
				delete f;
				return saveFileError();
			}
			_inStream = f;
		}
	} else if (option.equalsIgnoreCase("write")) {
		// OutSaveFile is not seekable so create a separate seekable stream
		// which will be written to the _outFile upon disposal
		_outFile = saves->openForSaving(filename, false);
		_outStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
		if (!_outFile) {
			return saveFileError();
		}
	} else if (option.equalsIgnoreCase("append")) {
		Common::SeekableReadStream *inFile = saves->openForLoading(filename);
		if (!inFile) {
			Common::File *f = new Common::File;

			if (!f->open(Common::Path(origpath, g_director->_dirSeparator))) {
				delete f;
				return saveFileError();
			}
			inFile = f;
		}
		_outStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
		byte b = inFile->readByte();
		while (!inFile->eos() && !inFile->err()) {
			_outStream->writeByte(b);
			b = inFile->readByte();
		}
		delete inFile;
		_outFile = saves->openForSaving(filename, false);
		if (!_outFile) {
			return saveFileError();
		}
	} else {
		error("Unsupported FileIO option: '%s'", option.c_str());
	}

	_filename = new Common::String(filename);
	return kErrorNone;
}

void FileObject::clear() {
	if (_filename) {
		delete _filename;
		_filename = nullptr;
	}
	if (_inStream) {
		delete _inStream;
		_inStream = nullptr;
	}
	if (_outFile) {
		_outFile->write(_outStream->getData(), _outStream->size());
		_outFile->finalize();
		delete _outFile;
		delete _outStream;
		_outFile = nullptr;
		_outStream = nullptr;
	}
}

void FileObject::dispose() {
	_disposed = true;
	clear();
}

FileIOError FileObject::saveFileError() {
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	if (saves->getError().getCode()) {
		warning("SaveFileManager error %d: %s", saves->getError().getCode(), saves->getErrorDesc().c_str());
		return kErrorIO;
	} else {
		return kErrorFileNotFound;
	}
}

void FileIO::m_new(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);
	Datum result = g_lingo->_state->me;
	if (nargs == 2) {
		if (me->getObjType() != kXObj) {
			warning("FileIO::m_new: called with XObject API however was expecting object type %d", me->getObjType());
		}
		Datum d2 = g_lingo->pop();
		Datum d1 = g_lingo->pop();

		Common::String option = d1.asString();
		Common::String path = d2.asString();
		FileIOError err = me->open(path, option);
		// if there's an error, return an errorcode int instead of an object
		if (err != kErrorNone) {
			me->_lastError = err;
			warning("FileIO::m_new: couldn't open file at path %s, error %d", path.c_str(), err);
			g_lingo->push(Datum(err));
			return;
		}
	} else if (nargs == 0) {
		if (me->getObjType() != kXtraObj) {
			warning("FileIO::m_new: called with Xtra API however was expecting object type %d", me->getObjType());
		}
	} else {
		warning("FileIO::m_new: expected 0 or 2 args, assuming 0");
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(result);
}

void FileIO::m_openFile(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);
	Datum d1 = g_lingo->pop();
	Datum d2 = g_lingo->pop();

	int mode = d1.asInt();
	Common::String option;
	switch (mode) {
	case 1:
		option = "read";
		break;
	case 2:
		option = "write";
		break;
	case 0:
	default:
		warning("FIXME: Mode %d not supported, falling back to read", mode);
		option = "read";
		break;
	}
	Common::String path = d2.asString();
	me->_lastError = me->open(path, option);
}

void FileIO::m_closeFile(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	me->clear();
}

// FIXME: split out filename-to-savegame logic from open() so we can implement createFile
XOBJSTUB(FileIO::m_createFile, 0)

void FileIO::m_displayOpen(int nargs) {
	g_lingo->push(getFileNameFromModal(false, Common::String(), Common::String(), "txt"));
}

void FileIO::m_displaySave(int nargs) {
	Datum defaultFileName = g_lingo->pop();
	Datum title = g_lingo->pop();
	g_lingo->push(getFileNameFromModal(true, Common::String(), title.asString(), "txt"));
}

XOBJSTUB(FileIO::m_setFilterMask, 0)

// Read

void FileIO::m_readChar(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	if (!me->_inStream || me->_inStream->eos() || me->_inStream->err()) {
		g_lingo->push(Datum(kErrorEOF));
		return;
	}

	int ch = me->_inStream->readByte();
	if (me->_inStream->eos() || me->_inStream->err()) {
		ch = kErrorEOF;
	}
	g_lingo->push(Datum(ch));
}

void FileIO::m_readLine(int nargs) {
	// file(mReadLine) is equivalent to file(mReadToken, "", RETURN)
	// See D4 Using Lingo p. 323

	g_lingo->push(Datum(""));
	g_lingo->push(Datum("\r"));
	FileIO::m_readToken(2);
}

void FileIO::m_readWord(int nargs) {
	// file(mReadWord) is equivalent to file(mReadToken, " ", " " & RETURN)
	// See D4 Using Lingo p. 323

	g_lingo->push(Datum(" "));
	g_lingo->push(Datum(" \r"));
	FileIO::m_readToken(2);
}

XOBJSTUB(FileIO::m_readPict, "")

bool FileIO::charInMatchString(char ch, const Common::String &matchString) {
	return matchString.contains(ch);
}

void FileIO::m_readToken(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::String skipString = d1.asString();
	Common::String breakString = d2.asString();

	if (!me->_inStream || me->_inStream->eos() || me->_inStream->err()) {
		g_lingo->push(Datum(""));
		return;
	}

	Common::String tok = "";
	char ch;
	do {
		ch = me->_inStream->readByte();
		if (me->_inStream->eos() || me->_inStream->err()) {
			g_lingo->push(Datum(tok));
			return;
		}
	} while (charInMatchString(ch, skipString));

	while (!charInMatchString(ch, breakString)) {
		tok += ch;
		ch = me->_inStream->readByte();

		if (me->_inStream->eos() || me->_inStream->err()) {
			g_lingo->push(Datum(tok));
			return;
		}
	}

	// Include the break character when the skipString is empty
	if (skipString.size() == 0) {
		tok += ch;
	} else {
		me->_inStream->seek(-1, SEEK_CUR);
	}

	g_lingo->push(Datum(tok));
}

void FileIO::m_readFile(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	if (!me->_inStream || me->_inStream->eos() || me->_inStream->err()) {
		g_lingo->push(Datum(""));
		return;
	}

	Common::String res;
	char ch = me->_inStream->readByte();
	while (!me->_inStream->eos() && !me->_inStream->err()) {
		res += ch;
		ch = me->_inStream->readByte();
	}

	g_lingo->push(res);
}

// Write

void FileIO::m_writeChar(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);
	Datum d = g_lingo->pop();

	if (!me->_outStream) {
		g_lingo->push(Datum(kErrorReadOnly));
		return;
	}

	me->_outStream->writeByte(d.asInt());
	g_lingo->push(Datum(kErrorNone));
}

void FileIO::m_writeString(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);
	Datum d = g_lingo->pop();

	if (!me->_outStream) {
		g_lingo->push(Datum(kErrorReadOnly));
		return;
	}

	me->_outStream->writeString(d.asString());
	g_lingo->push(Datum(kErrorNone));
}

// Getters/Setters

XOBJSTUB(FileIO::m_getFinderInfo, "")
XOBJSTUB(FileIO::m_setFinderInfo, 0)

void FileIO::m_getPosition(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	if (me->_inStream) {
		g_lingo->push(Datum((int)me->_inStream->pos()));
	} else if (me->_outStream) {
		g_lingo->push(Datum((int)me->_outStream->pos()));
	} else {
		warning("FileIO: No file open");
		g_lingo->push(Datum(kErrorFileNotOpen));
	}
}

void FileIO::m_setPosition(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);
	Datum d = g_lingo->pop();
	int pos = d.asInt();

	if (me->_inStream) {
		if (pos <= me->_inStream->size()) {
			me->_inStream->seek(pos, SEEK_SET);
			g_lingo->push(Datum(kErrorNone));
		} else {
			me->_inStream->seek(me->_inStream->size(), SEEK_SET);
			g_lingo->push(Datum(kErrorInvalidPos));
		}
	} else if (me->_outStream) {
		if (pos <= me->_outStream->size()) {
			me->_outStream->seek(pos, SEEK_SET);
			g_lingo->push(Datum(kErrorNone));
		} else {
			me->_outStream->seek(me->_outStream->size(), SEEK_SET);
			g_lingo->push(Datum(kErrorInvalidPos));
		}
	} else {
		warning("FileIO: No file open");
		g_lingo->push(Datum(kErrorFileNotOpen));
	}
}

void FileIO::m_getLength(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	if (me->_inStream) {
		g_lingo->push(Datum((int)me->_inStream->size()));
	} else if (me->_outStream) {
		g_lingo->push(Datum((int)me->_outStream->size()));
	} else {
		warning("FileIO: No file open");
		g_lingo->push(Datum(kErrorFileNotOpen));
	}
}

void FileIO::m_fileName(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	if (me->_filename) {
		Common::String prefix = savePrefix();
		Common::String res = *me->_filename;
		if (res.hasPrefix(prefix)) {
			res = Common::String(&me->_filename->c_str()[prefix.size()]);
		}

		g_lingo->push(Datum(res));
	} else {
		warning("FileIO: No file open");
		g_lingo->push(Datum(kErrorFileNotOpen));
	}
}

void FileIO::m_error(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);
	Datum d = g_lingo->pop();
	Datum result("");
	switch (d.asInt()) {
	case kErrorNone:
		if (me->getObjType() == kXtraObj) {
			result = Datum("OK");
		}
		break;
	case kErrorMemAlloc:
		result = Datum("Memory allocation failure");
		break;
	case kErrorDirectoryFull:
		result = Datum("File directory full");
		break;
	case kErrorVolumeFull:
		result = Datum("Volume full");
		break;
	case kErrorVolumeNotFound:
		result = Datum("Volume not found");
		break;
	case kErrorIO:
		result = Datum("I/O Error");
		break;
	case kErrorBadFileName:
		result = Datum("Bad file name");
		break;
	case kErrorFileNotOpen:
		result = Datum("File not open");
		break;
	case kErrorTooManyFilesOpen:
		result = Datum("Too many files open");
		break;
	case kErrorFileNotFound:
		result = Datum("File not found");
		break;
	case kErrorNoSuchDrive:
		result = Datum("No such drive");
		break;
	case kErrorNoDiskInDrive:
		result = Datum("No disk in drive");
		break;
	case kErrorDirectoryNotFound:
		result = Datum("Directory not found");
		break;
	default:
		result = Datum("Unknown error");
		break;
	}
	g_lingo->push(result);
}

void FileIO::m_status(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	g_lingo->push(Datum(me->_lastError));
}

// Other

void FileIO::m_delete(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_state->me.u.obj);

	if (me->_filename) {
		Common::String filename = *me->_filename;
		me->dispose();
		if (g_system->getSavefileManager()->removeSavefile(filename)) {
			g_lingo->push(Datum(kErrorNone));
		} else {
			g_lingo->push(Datum(kErrorIO));
		}
	} else {
		warning("FileIO: No file open");
		g_lingo->push(Datum(kErrorFileNotOpen));
	}
}

// Non-standard extensions
XOBJSTUBNR(FileIO::m_setOverrideDrive)

XOBJSTUB(FileIO::m_getOSDirectory, "")

} // End of namespace Director
