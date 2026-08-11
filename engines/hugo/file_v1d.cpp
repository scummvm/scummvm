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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/text.h"
#include "hugo/util.h"

namespace Hugo {
FileManager_v1d::FileManager_v1d(HugoEngine *vm) : FileManager(vm) {
}

FileManager_v1d::~FileManager_v1d() {
}

void FileManager_v1d::openDatabaseFiles() {
	debugC(1, kDebugFile, "openDatabaseFiles");
}

void FileManager_v1d::closeDatabaseFiles() {
	debugC(1, kDebugFile, "closeDatabaseFiles");
}

/**
 * Open and read in an overlay file, close file
 */
void FileManager_v1d::readOverlay(const int screenNum, ImagePtr image, const OvlType overlayType) {
	debugC(1, kDebugFile, "readOverlay(%d, ...)", screenNum);

	const char *ovlExt[] = {".b", ".o", ".ob"};
	Common::Path buf(_vm->_text->getScreenNames(screenNum));
	buf.appendInPlace(ovlExt[overlayType]);

	if (!Common::File::exists(buf)) {
		memset(image, 0, kOvlSize);
		warning("File not found: %s", buf.toString().c_str());
		return;
	}

	if (!_sceneryArchive1.open(buf))
		error("File not found: %s", buf.toString().c_str());

	ImagePtr tmpImage = image;                      // temp ptr to overlay file

	_sceneryArchive1.read(tmpImage, kOvlSize);
	_sceneryArchive1.close();
}

/**
 * Read a PCX image into dib_a
 */
void FileManager_v1d::readBackground(const int screenIndex) {
	debugC(1, kDebugFile, "readBackground(%d)", screenIndex);

	Common::Path buf(_vm->_text->getScreenNames(screenIndex));
	buf.appendInPlace(".ART");
	if (!_sceneryArchive1.open(buf))
		error("File not found: %s", buf.toString().c_str());
	// Read the image into dummy seq and static dib_a
	Seq *dummySeq;                                // Image sequence structure for Read_pcx
	dummySeq = readPCX(_sceneryArchive1, nullptr, _vm->_screen->getFrontBuffer(), true, _vm->_text->getScreenNames(screenIndex));
	free(dummySeq);
	_sceneryArchive1.close();
}

const char *FileManager_v1d::fetchString(const int index) {
	debugC(1, kDebugFile, "fetchString(%d)", index);

	return _vm->_text->getStringtData(index);
}

/**
 * Simple instructions given when F1 pressed twice in a row
 * Only in DOS versions
 */
void FileManager_v1d::instructions() const {
	// Note: HELP.DAT uses CRLF line endings. The original used `open()` from
	// Microsoft's CRT in non-binary mode, so `read()` translated these to LF.
	// This is necessary because the DOS message boxes only expect LF characters.
	// The original source code's comments call the surviving character "CR",
	// but it is really LF. We have copied these comments, so when you see a
	// comment refer to "CR", such as later in this function, it means LF.
	Common::File f;
	if (!f.open("help.dat")) {
		warning("help.dat not found");
		return;
	}

	char readBuf;
	while (f.read(&readBuf, 1)) {
		if (readBuf == '\r') {
			continue; // skip '\r'
		}
		char line[1024], *wrkLine;
		wrkLine = line;
		wrkLine[0] = readBuf;
		wrkLine++;
		do {
			f.read(wrkLine, 1);
			if (*wrkLine == '\r') {
				f.read(wrkLine, 1); // skip '\r'
			}
		} while (*wrkLine++ != '#');                // '#' is EOP
		wrkLine[-2] = '\0';                         // Remove EOP and previous CR
		_vm->notifyBox(line);
		wrkLine = line;
		f.read(&readBuf, 1);                        // Remove CR after EOP
		if (readBuf == '\r') {
			f.read(&readBuf, 1); // skip '\r'
		}
	}
	f.close();
}

} // End of namespace Hugo
