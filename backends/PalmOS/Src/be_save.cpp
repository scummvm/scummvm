/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "be_base.h"
#include "common/savefile.h"

// SaveFile class

class PalmSaveFile : public Common::SaveFile {
private:
	FILE *fh;
public:
	PalmSaveFile(const char *filename, bool saveOrLoad) {
		fh = ::fopen(filename, (saveOrLoad? "wb" : "rb"));
	}
	~PalmSaveFile() {
		if (fh) ::fclose(fh);
	}
	
	bool eos() const { return feof(fh) != 0; }
	bool ioFailed() const { return ferror(fh) != 0; }
	void clearIOFailed() { clearerr(fh); }

	bool isOpen() const { return fh != NULL; }

	uint32 read(void *buf, uint32 size);
	uint32 write(const void *buf, uint32 size);

	uint32 pos() const {
		assert(fh);
		return ftell(fh);
	}
	uint32 size() const {
		assert(fh);
		uint32 oldPos = ftell(fh);
		fseek(fh, 0, SEEK_END);
		uint32 length = ftell(fh);
		fseek(fh, oldPos, SEEK_SET);
		return length;
	}
	void seek(int32 offs, int whence = SEEK_SET) {
		assert(fh);
		fseek(fh, offs, whence);
	}
};

uint32 PalmSaveFile::read(void *buf, uint32 size) {
	// we must return the size, where fread return nitems upon success ( 1 <=> size)
	if (fh) return (::fread(buf, 1, size, fh));
	return 0;
}

uint32 PalmSaveFile::write(const void *buf, uint32 size) {
	// we must return the size, where fwrite return nitems upon success ( 1 <=> size)
	if (fh) return ::fwrite(buf, 1, size, fh);
	return 0;
}





// SaveFileManager class

class PalmSaveFileManager : public Common::SaveFileManager {
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
	strncat(buf, "/", 1);
	strncat(buf, filename, sizeof(buf));

	return makeSaveFile(buf, saveOrLoad);
}

void PalmSaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	FileRef fileRef;
	// try to open the dir
	Err e = VFSFileOpen(gVars->VFS.volRefNum, getSavePath(), vfsModeRead, &fileRef);
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

		if (e != expErrEnumerationEmpty) {									// there is something

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
	PalmSaveFile *sf = new PalmSaveFile(filename, saveOrLoad);

	if (!sf->isOpen()) {
		delete sf;
		sf = 0;
	}
	return sf;
}

Common::SaveFileManager *OSystem_PalmBase::getSavefileManager() {
	return new PalmSaveFileManager();
}
