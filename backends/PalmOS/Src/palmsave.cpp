/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include <ctype.h>
#include "common/stdafx.h"
#include "palm.h"

#define	MAX_BLOCK	64000			// store in memory, before dump to file

// SaveFile class

class PalmSaveFile : public Common::SaveFile {
public:
	PalmSaveFile(const char *filename, bool saveOrLoad);
	~PalmSaveFile();

	bool isOpen() const { return file != NULL; }

	uint32 read(void *buf, uint32 size);
	uint32 write(const void *buf, uint32 size);

private :
	FILE *file;
	UInt8 *_readWriteData;
	UInt32 _readWritePos;
	bool _needDump;
	UInt32 length;
};

PalmSaveFile::PalmSaveFile(const char *filename, bool saveOrLoad) {
	_readWriteData = NULL;
	_readWritePos = 0;
	_needDump = false;

	file = ::fopen(filename, (saveOrLoad ? "wb" : "rb"));

	if (file) {
		if (saveOrLoad) {
			_readWriteData = (byte *)malloc(MAX_BLOCK);

		} else {
			// read : cache the whole file
			::fseek(file, 0, SEEK_END);
			length = ::ftell(file);
			::fseek(file, 0, SEEK_SET);

			_readWriteData = (byte *)malloc(length);
			_readWritePos = 0;

			if (_readWriteData)
				::fread(_readWriteData, 1, length, file);
		}
	}
}

PalmSaveFile::~PalmSaveFile() {
	if (file) {
		if (_needDump)
			::fwrite(_readWriteData, _readWritePos, 1, file);

		if (_readWriteData)
			free(_readWriteData);

		::fclose(file);
	}
}

uint32 PalmSaveFile::read(void *buf, uint32 size) {
	if (!_readWriteData)
		// we must return the size, where fread return nitems upon success ( 1 <=> size)
		return (::fread(buf, 1, size, file));

	if (_readWritePos < length) {
		MemMove(buf, _readWriteData + _readWritePos, size);
		_readWritePos += size;
		return size;
	}

	return 0;
}

uint32 PalmSaveFile::write(const void *buf, uint32 size) {
	if (_readWriteData) {

		if ((_readWritePos + size) > MAX_BLOCK) {
			if (_readWritePos > 0)
				::fwrite(_readWriteData, _readWritePos, 1, file);

			_readWritePos = 0;
			_needDump = false;

		} else {
			// save new block
			MemMove(_readWriteData + _readWritePos, buf, size);
			_readWritePos += size;
			_needDump = true;

			return size;
		}
	}

	// we must return the size, where fwrite return nitems upon success ( 1 <=> size)
	return ::fwrite(buf, 1, size, file);
}

// SaveFileManager class

class PalmSaveFileManager : public SaveFileManager {
public:
	virtual Common::OutSaveFile *openForSaving(const char *filename) {
		return openSavefile(filename, true);
	}
	virtual Common::InSaveFile *openForLoading(const char *filename) {
		return openSavefile(filename, false);
	}

	Common::SaveFile *openSavefile(const char *filename, bool saveOrLoad);
	void listSavefiles(const char *prefix, bool *marks, int num);

protected:
	Common::SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

Common::SaveFile *PalmSaveFileManager::openSavefile(const char *filename, bool saveOrLoad) {
	char buf[256];

	strncpy(buf, getSavePath(), sizeof(buf));
	strncat(buf, filename, sizeof(buf));

	Common::SaveFile *sf = makeSaveFile(buf, saveOrLoad);
	if (!sf->isOpen()) {
		delete sf;
		sf = NULL;
	}

	return sf;
}

void PalmSaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	FileRef fileRef;
	// try to open the dir
	Err e = VFSFileOpen(gVars->volRefNum, getSavePath(), vfsModeRead, &fileRef);
	memset(marks, false, num*sizeof(bool));

	if (e != errNone)
		return;

	// enumerate all files
	UInt32 dirEntryIterator = vfsIteratorStart;
	Char filename[32];
	FileInfoType info = {0, filename, 32};
	UInt16 length = StrLen(prefix);
	int slot = 0;

	while (dirEntryIterator != vfsIteratorStop) {
		e = VFSDirEntryEnumerate (fileRef, &dirEntryIterator, &info);

		if (e != expErrEnumerationEmpty) {										// there is something

			if (StrLen(info.nameP) == (length + 2)) {						// consider max 99, filename length is ok
				if (StrNCaselessCompare(prefix, info.nameP, length) == 0) { // this seems to be a save file
					if (isdigit(info.nameP[length]) && isdigit(info.nameP[length+1])) {

						slot = StrAToI(filename + length);
						if (slot >= 0 && slot < num)
							*(marks+slot) = true;

					}
				}
			}

		}
	}

	VFSFileClose(fileRef);
}

Common::SaveFile *PalmSaveFileManager::makeSaveFile(const char *filename, bool saveOrLoad) {
	return new PalmSaveFile(filename, saveOrLoad);
}

// OSystem
Common::SaveFileManager *OSystem_PALMOS::getSavefileManager() {
	return new PalmSaveFileManager();
}
