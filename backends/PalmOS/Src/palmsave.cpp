/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
#include "common/scummsys.h"
#include "base/engine.h"
#include "scumm/saveload.h"
#include "palm.h"

#define	MAX_BLOCK 64000	// store in memory, before dump to file

// SaveFile class

class PalmSaveFile : public SaveFile {
public:
	PalmSaveFile(const char *filename, bool saveOrLoad);
	~PalmSaveFile();
	
	bool isOpen() const { return file != NULL; }
protected:
	int fread(void *buf, int size, int cnt);
	int fwrite(const void *buf, int size, int cnt);

private :
	FILE *file;
	UInt8 * _readWriteData;
	UInt32 _readWritePos;
	bool _needDump;
};

PalmSaveFile::PalmSaveFile(const char *filename, bool saveOrLoad) {
	_readWriteData = NULL;
	_readWritePos = 0;
	_needDump = false;

	file = ::fopen(filename, (saveOrLoad ? "wb" : "rb"));
}

PalmSaveFile::~PalmSaveFile() {
	if (file) {
		if (_needDump && _readWriteData) {
			::fwrite(_readWriteData, _readWritePos, 1, file);
			free(_readWriteData);
		}

		::fclose(file);
	}
}

int PalmSaveFile::fread(void *buf, int size, int cnt) {
	return ::fread(buf, size, cnt, file);
}

int PalmSaveFile::fwrite(const void *buf, int size, int cnt) {
	UInt32 fullsize = size*cnt;

	if (fullsize <= MAX_BLOCK)
	{
		if (!_readWriteData)
			_readWriteData = (byte *)malloc(MAX_BLOCK);

		if ((_readWritePos+fullsize)>MAX_BLOCK) {
			::fwrite(_readWriteData, _readWritePos, 1, file);
			_readWritePos = 0;
			_needDump = false;
		}
			
		MemMove(_readWriteData + _readWritePos, buf, fullsize);
		_readWritePos += fullsize;
		_needDump = true;

		return cnt;
	}

	return ::fwrite(buf, size, cnt, file);
}

// SaveFileManager class

class PalmSaveFileManager : public SaveFileManager {
public:
	void list_savefiles(const char *prefix, const char *directory, bool *marks, int num);

protected:
	SaveFile *makeSaveFile(const char *filename, bool saveOrLoad);
};

void PalmSaveFileManager::list_savefiles(const char *prefix, const char *directory, bool *marks, int num) {
	FileRef fileRef;
	// try to open the dir
	Err e = VFSFileOpen(gVars->volRefNum, directory, vfsModeRead, &fileRef);
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

SaveFile *PalmSaveFileManager::makeSaveFile(const char *filename, bool saveOrLoad) {
	return new PalmSaveFile(filename, saveOrLoad);
}

// OSystem
SaveFileManager *OSystem_PALMOS::get_savefile_manager() {
	return new PalmSaveFileManager();
}
