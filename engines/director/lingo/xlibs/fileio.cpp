/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gui/filebrowser-dialog.h"

#include "common/memstream.h"
#include "common/savefile.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/fileio.h"

namespace Director {

static const char *xlibName = "FileIO";

static MethodProto xlibMethods[] = {
	{ "delete",					FileIO::m_delete,			 0, 0,	200 },	// D2
	{ "fileName",				FileIO::m_fileName,			 0, 0,	200 },	// D2
	{ "getLength",				FileIO::m_getLength,		 0, 0,	200 },	// D2
	{ "getPosition",			FileIO::m_getPosition,		 0, 0,	200 },	// D2
	{ "new",					FileIO::m_new,				 2, 2,	200 },	// D2
	{ "readChar",				FileIO::m_readChar,			 0, 0,	200 },	// D2
	{ "readLine",				FileIO::m_readLine,			 0, 0,	200 },	// D2
	{ "readToken",				FileIO::m_readToken,		 2, 2,	200 },	// D2
	{ "readWord",				FileIO::m_readWord,			 0, 0,	200 },	// D2
	{ "setPosition",			FileIO::m_setPosition,		 1, 1,	200 },	// D2
	{ "writeChar",				FileIO::m_writeChar,		 1, 1,	200 },	// D2
	{ "writeString",			FileIO::m_writeString,		 1, 1,	200 },	// D2
	{ 0, 0, 0, 0, 0 }
};

void FileIO::initialize(int type) {
	FileObject::initMethods(xlibMethods);
	if (type & kXObj) {
		if (!g_lingo->_globalvars.contains(xlibName)) {
			FileObject *xobj = new FileObject(kXObj);
			g_lingo->_globalvars[xlibName] = xobj;
		} else {
			warning("FileIO XObject already initialized");
		}
	}
	if (type & kXtraObj) {
		// TODO - Implement Xtra
	}
}

// Initialization/disposal

FileObject::FileObject(ObjectType objType) : Object<FileObject>("FileIO") {
	_objType = objType;
	_filename = nullptr;
	_inFile = nullptr;
	_inStream = nullptr;
	_outFile = nullptr;
	_outStream = nullptr;
}

FileObject::FileObject(const FileObject &obj) : Object<FileObject>(obj) {
	_filename = nullptr;
	_inFile = nullptr;
	_inStream = nullptr;
	_outFile = nullptr;
	_outStream = nullptr;
}

FileObject::~FileObject() {
	clear();
}

void FileObject::clear() {
	if (_filename) {
		delete _filename;
		_filename = nullptr;
	}
	if (_inFile) {
		delete _inFile;
		if (_inStream != _inFile)
			delete _inStream;
		_inFile = nullptr;
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

void FileIO::saveFileError() {
	Common::SaveFileManager *saves = g_system->getSavefileManager();
	if (saves->getError().getCode()) {
		warning("SaveFileManager error %d: %s", saves->getError().getCode(), saves->getErrorDesc().c_str());
		g_lingo->push(Datum(kErrorIO));
	} else {
		g_lingo->push(Datum(kErrorFileNotFound));
	}
}

void FileIO::m_new(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	Common::SaveFileManager *saves = g_system->getSavefileManager();
	Common::String option = d1.asString();
	Common::String filename = d2.asString();

	if (option.hasPrefix("?")) {
		option = option.substr(1);
		GUI::FileBrowserDialog browser(0, "txt", option.equalsIgnoreCase("write") ? GUI::kFBModeSave : GUI::kFBModeLoad);
		if (browser.runModal() <= 0) {
			g_lingo->push(Datum(kErrorFileNotFound));
			return;
		}
		filename = browser.getResult();
	} else if (!filename.hasSuffixIgnoreCase(".txt")) {
		filename += ".txt";
	}

	if (option.equalsIgnoreCase("read")) {
		me->_inFile = saves->openForLoading(filename);
		me->_inStream = me->_inFile;
		if (!me->_inFile) {
			saveFileError();
			me->dispose();
			return;
		}
	} else if (option.equalsIgnoreCase("write")) {
		// OutSaveFile is not seekable so create a separate seekable stream
		// which will be written to the _outFile upon disposal
		me->_outFile = saves->openForSaving(filename, false);
		me->_outStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
		if (!me->_outFile) {
			saveFileError();
			me->dispose();
			return;
		}
	} else if (option.equalsIgnoreCase("append")) {
		Common::InSaveFile *_inFile = saves->openForLoading(filename);
		if (!_inFile) {
			saveFileError();
			me->dispose();
			return;
		}
		me->_outStream = new Common::MemoryWriteStreamDynamic(DisposeAfterUse::YES);
		byte b = _inFile->readByte();
		while (!_inFile->eos() && !_inFile->err()) {
			me->_outStream->writeByte(b);
			b = _inFile->readByte();
		}
		delete _inFile;
		me->_outFile = saves->openForSaving(filename, false);
		if (!me->_outFile) {
			saveFileError();
			me->dispose();
			return;
		}
	} else {
		error("Unsupported FileIO option: '%s'", option.c_str());
	}

	me->_filename = new Common::String(filename);

	g_lingo->push(g_lingo->_currentMe);
}

// Read

void FileIO::m_readChar(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

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
	g_lingo->push(Datum("\n"));
	FileIO::m_readToken(2);
}

void FileIO::m_readWord(int nargs) {
	// file(mReadWord) is equivalent to file(mReadToken, " ", " " & RETURN)
	// See D4 Using Lingo p. 323

	g_lingo->push(Datum(" "));
	g_lingo->push(Datum(" \n"));
	FileIO::m_readToken(2);
}

bool FileIO::charInMatchString(char ch, const Common::String &matchString) {
	if (ch == '\r')
		ch = '\n';
	return matchString.contains(ch);
}

void FileIO::m_readToken(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

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

// Write

void FileIO::m_writeChar(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);
	Datum d = g_lingo->pop();

	if (!me->_outStream) {
		g_lingo->push(Datum(kErrorReadOnly));
		return;
	}

	me->_outStream->writeByte(d.asInt());
	g_lingo->push(Datum(kErrorNone));
}

void FileIO::m_writeString(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);
	Datum d = g_lingo->pop();

	if (!me->_outStream) {
		g_lingo->push(Datum(kErrorReadOnly));
		return;
	}

	me->_outStream->writeString(d.asString());
	g_lingo->push(Datum(kErrorNone));
}

// Other

void FileIO::m_getPosition(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

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
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);
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
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

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
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

	if (me->_filename) {
		g_lingo->push(Datum(*me->_filename));
	} else {
		warning("FileIO: No file open");
		g_lingo->push(Datum(kErrorFileNotOpen));
	}
}

void FileIO::m_delete(int nargs) {
	FileObject *me = static_cast<FileObject *>(g_lingo->_currentMe.u.obj);

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

} // End of namespace Director
