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

#ifndef _RAMSAVE_H_
#define _RAMSAVE_H_

#include "common/system.h"
#include "common/savefile.h"

// SaveFileManager class

#define DS_MAX_SAVE_SIZE 150000

class DSSaveFile : public Common::InSaveFile, public Common::OutSaveFile {
	int address;
	int ptr;
	bool ownsData;
	bool saveCompressed;

	struct SCUMMSave {
		u32 magic;		// 4
		bool isValid;	// 5
		bool pad;		// 6
		char name[16];	// 22
		u32 size;		// 26
		u32 compressedSize; // 30
		u16 extraMagic;	// 32
		u32 reserved;	// 36
	} __attribute__ ((packed));

	SCUMMSave save;
	u8 *saveData;
	SCUMMSave *origHeader;
	bool isOpenFlag;
	bool isTempFile;
	bool eosReached;

public:
	DSSaveFile();
	DSSaveFile(SCUMMSave *s, bool saveCompressed, u8 *data);
	~DSSaveFile();

	void reset();

	bool isOpen() const { return isOpenFlag; }
	virtual bool eos() const;
	virtual void clearErr();
	virtual bool skip(uint32 size);

	virtual int32 pos() const;
	virtual int32 size() const;
	virtual bool seek(int32 pos, int whence);

	uint32 read(void *buf, uint32 size);
	uint32 write(const void *buf, uint32 size);

	void setName(char *name);
	char *getName() { return save.name; }

	bool isValid() { return save.isValid; }
	bool isTemp() { return isTempFile; }
	bool matches(const char *prefix, int num);
	bool matches(const char *filename);

	void clearData();
	void compress();

	int getRamUsage() { return sizeof(save) + save.compressedSize; }
	char *getRamImage() { return (char *) &save; }

	int getSize() { return save.size; }

	DSSaveFile *clone();

	bool loadFromSaveRAM(vu8 *address);
	int saveToSaveRAM(vu8 *address);



	void deleteFile();

	void operator delete(void *p) {
//		consolePrintf("Finished! size=%d\n", ((DSSaveFile *) (p))->save->size);
	}



};



class DSSaveFileManager : public Common::SaveFileManager {

	DSSaveFile gbaSave[8];
	static DSSaveFileManager *instancePtr;
	int sramBytesFree;

public:
	DSSaveFileManager();
	~DSSaveFileManager();

	static DSSaveFileManager *instance() { return instancePtr; }

	DSSaveFile *openSavefile(const char *filename, bool saveOrLoad);

	virtual Common::OutSaveFile *openForSaving(const Common::String &filename) { return openSavefile(filename.c_str(), true); }
	virtual Common::InSaveFile *openForLoading(const Common::String &filename) { return openSavefile(filename.c_str(), false); }

	virtual bool removeSavefile(const Common::String &filename);
	virtual Common::StringArray listSavefiles(const Common::String &pattern);

	void flushToSaveRAM();

	void addBytesFree(int size) { sramBytesFree += size; }
	int getBytesFree() { return sramBytesFree; }

	void deleteFile(char *name);
	void listFiles();
	void formatSram();

	void loadAllFromSRAM();

	static bool isExtraDataPresent();
	static int getExtraData();
	static void setExtraData(int data);

protected:
	DSSaveFile *makeSaveFile(const Common::String &filename, bool saveOrLoad);
};

#endif
