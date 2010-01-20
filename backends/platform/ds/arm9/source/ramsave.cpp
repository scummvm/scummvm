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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#ifdef GBA_SRAM_SAVE


#include "ramsave.h"
#include "nds.h"
#include "compressor/lz.h"

#define CART_RAM ((vu8 *) (0x0A000000))
#define SRAM_SAVE_MAX (65533)

DSSaveFile::DSSaveFile() {
	ptr = 0;
	saveCompressed = false;
	save.isValid = false;
	ownsData = false;
	isOpenFlag = true;
	isTempFile = false;
}

DSSaveFile::DSSaveFile(SCUMMSave* s, bool compressed, u8* data) {
	save = *s;
	saveData = data;
	ptr = 0;
	saveCompressed = compressed;
	isOpenFlag = true;

	if (saveCompressed) {
		u8* uncompressed = new unsigned char[save.size];
		if (!uncompressed) consolePrintf("Out of memory allocating %d!\n", save.size);
		LZ_Uncompress(saveData, uncompressed, save.compressedSize);
		saveData = uncompressed;
		ownsData = true;
		saveCompressed = false;
//		consolePrintf("Decompressed. name=%s size=%d (%d)", save.name, save.size, save.compressedSize);

	} else {
		ownsData = false;
		origHeader = s;
	}

	if (save.magic == (int) 0xBEEFCAFE) {
		save.isValid = true;
	} else {
		save.isValid = false;
	}

	isTempFile = false;
	eosReached = false;
}

DSSaveFile::~DSSaveFile() {
	if (!ownsData) {
		*origHeader = save;
		DSSaveFileManager::instance()->flushToSaveRAM();
	}
	if (ownsData) {
		delete[] saveData;
	}
}

bool DSSaveFile::loadFromSaveRAM(vu8* address) {

	SCUMMSave newSave;

	for (int t = 0; t < (int) sizeof(newSave); t++) {
		((char *) (&newSave))[t] = *(address + t);
	}

	if (newSave.magic == 0xBEEFCAFE) {
		newSave.isValid = true;

		*((u16 *) (0x4000204)) |= 0x3;

		saveData = new unsigned char[newSave.compressedSize];

		for (int t = 0; t < (int) newSave.compressedSize; t++) {
			((char *) (saveData))[t] = *(address + t + sizeof(newSave));
		}

		if (ownsData) delete[] this->saveData;
		save = newSave;
		saveCompressed = true;
		this->saveData = saveData;
		ownsData = true;
		ptr = 0;

		return true;
	}

	return false;
}

void DSSaveFile::compress() {
	if (!saveCompressed) {
		unsigned char* compBuffer = new unsigned char[(save.size * 110) / 100];
		int compSize = LZ_Compress((u8 *) saveData, compBuffer, save.size);
		save.compressedSize = compSize;



		delete[] saveData;

		// Make the save smaller
		saveData = (u8 *) realloc(compBuffer, save.compressedSize);
		saveCompressed = true;
	}
}

int DSSaveFile::saveToSaveRAM(vu8* address) {

	unsigned char* compBuffer;
	bool failed;


	int compSize;

	compress();

	compSize = save.compressedSize;
	compBuffer = saveData;

	if (DSSaveFileManager::instance()->getBytesFree() >= getRamUsage()) {

		DSSaveFileManager::instance()->addBytesFree(-getRamUsage());

		// Write header
		for (int t = 0; t < sizeof(save); t++) {
			while (*(address + t) != ((char *) (&save))[t]) {
				*(address + t) = ((char *) (&save))[t];
			}
		}

		// Write compressed buffer
		for (int t = sizeof(save); t < (int) sizeof(save) + compSize; t++) {
			while (*(address + t) != compBuffer[t - sizeof(save)]) {
				*(address + t) = compBuffer[t - sizeof(save)];
			}
		}

		failed = false;
	} else {
		failed = true;
	}


	return failed? 0: compSize + sizeof(save);

}

void DSSaveFile::reset() {
	ptr = 0;
	eosReached = false;
}

uint32 DSSaveFile::read(void *buf, uint32 size) {
	if (ptr + size > save.size) {
		size = save.size - ptr;
		eosReached = true;
		if (size < 0) size = 0;
	}
	memcpy(buf, saveData + ptr, size);
//	consolePrintf("byte: %d ", ((u8 *) (buf))[0]);

	ptr += size;
	return size;
}

int32 DSSaveFile::pos() const {
	return ptr;
}

int32 DSSaveFile::size() const {
	return save.size;
}

bool DSSaveFile::seek(int32 pos, int whence) {
	switch (whence) {
		case SEEK_SET: {
			ptr = pos;
			break;
		}
		case SEEK_CUR: {
			ptr += pos;
			break;
		}
		case SEEK_END: {
			ptr = save.size + pos;
			break;
		}
	}
	eosReached = false;
	return true;
}

bool DSSaveFile::eos() const {
	return eosReached;
}

void DSSaveFile::clearErr() {
	eosReached = false;
}

bool DSSaveFile::skip(uint32 bytes) {
	ptr = ptr + bytes;
	if (ptr > (int) save.size) ptr = save.size;
	return true;
}

uint32 DSSaveFile::write(const void *buf, uint32 size) {

	if (ptr + size > DS_MAX_SAVE_SIZE) {
		size = DS_MAX_SAVE_SIZE - ptr;
	}

	memcpy(saveData + ptr, buf, size);
	ptr += size;
	save.size += size;
	return size;
}

bool DSSaveFile::matches(const char *prefix, int num) {
	char str[16];
	if (isValid()) {
		sprintf(str, "%s%02d", prefix, num);
		if (!strcmp(str, save.name)) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool DSSaveFile::matches(const char *filename) {
	if (isValid()) {
		return !strcmp(save.name, filename);
	} else {
		return false;
	}
}

void DSSaveFile::setName(char *name) {
	save.isValid = true;
	save.magic = 0xBEEFCAFE;
	ownsData = true;
	save.size = 0;
	save.compressedSize = 0;
	saveData = new unsigned char[DS_MAX_SAVE_SIZE];
	strcpy(save.name, name);

	if ((strstr(name, ".s99")) || (strstr(name, ".c"))) {
		isTempFile = true;
	} else {
		isTempFile = false;
	}
}

void DSSaveFile::clearData() {
	save.size = 0;

	if (saveCompressed) {
		if (ownsData) {
			delete[] saveData;
			DSSaveFileManager::instance()->addBytesFree(getRamUsage());
		}
		saveData = new unsigned char[DS_MAX_SAVE_SIZE];
		saveCompressed = false;
		ownsData = true;
	}

}

void DSSaveFile::deleteFile() {
	if (isValid()) {
		if (ownsData) {
			DSSaveFileManager::instance()->addBytesFree(getRamUsage());
			delete[] saveData;
			saveData = NULL;
		}
		ptr = 0;
		saveCompressed = false;
		save.isValid = false;
		ownsData = false;
		isOpenFlag = true;
	}
}

DSSaveFileManager::DSSaveFileManager() {
	instancePtr = this;

	*((u16 *) (0x4000204)) |= 0x3;
	swiWaitForVBlank();

	loadAllFromSRAM();
}

DSSaveFileManager::~DSSaveFileManager() {
	instancePtr = NULL;
}

void DSSaveFileManager::loadAllFromSRAM() {
	int addr = 1;

	for (int r = 0; r < 8; r++) {
		gbaSave[r].deleteFile();
	}

	sramBytesFree = SRAM_SAVE_MAX;

	// Try to find saves in save RAM
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].loadFromSaveRAM(CART_RAM + addr)) {
			addr += gbaSave[r].getRamUsage();
			sramBytesFree -= gbaSave[r].getRamUsage();
		}
	}

}

void DSSaveFileManager::formatSram() {
	for (int r = 0; r < SRAM_SAVE_MAX; r++) {
		*(CART_RAM + r) = 0;
	}

	loadAllFromSRAM();
}

void DSSaveFileManager::listFiles() {
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].isValid()) {
			consolePrintf("'%s': %d bytes\n", gbaSave[r].getName(), gbaSave[r].getRamUsage());
		}
	}
	consolePrintf("SRAM free: %d bytes\n", getBytesFree());
}

DSSaveFileManager* DSSaveFileManager::instancePtr = NULL;

DSSaveFile *DSSaveFileManager::openSavefile(const char *filename, bool saveOrLoad) {
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].isValid() && (gbaSave[r].matches(filename))) {
//			consolePrintf("Matched save %d (%d)\n", r, gbaSave[r].getSize());
			gbaSave[r].reset();
			//consolePrintf("reset ");
			if (saveOrLoad) gbaSave[r].clearData();
//			consolePrintf("cleared ");
			return gbaSave[r].clone();
		}
	}

	if (saveOrLoad) {
		return makeSaveFile(filename, saveOrLoad);
	} else {
		return NULL;
	}
}



DSSaveFile* DSSaveFile::clone() {
//	consolePrintf("Clone %s %d\n", save.name, save.size);
	return new DSSaveFile(&save, saveCompressed, saveData);
}

void DSSaveFileManager::deleteFile(const char* name) {
//	consolePrintf("Deleting %s", name);
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].isValid() && (gbaSave[r].matches(name))) {
			gbaSave[r].deleteFile();
		}
	}
	flushToSaveRAM();
}

bool DSSaveFileManager::removeSavefile(const Common::String &filename) {
	consolePrintf("DSSaveFileManager::removeSavefile : Not implemented yet.\n");
	assert(false);
	//TODO: Implement this. Most likely, you just have to use the code of deleteFile?
	return false;
}


Common::StringList DSSaveFileManager::listSavefiles(const Common::String &pattern) {
	consolePrintf("DSSaveFileManager::listSavefiles : Not implemented yet.\n");
	assert(false);
	return Common::StringList();
	/*
	TODO: Implement this. If you don't understand what it should do, just ask
	(e.g. on scummvm-devel or Fingolfin). It should be pretty simple if you
	use Common::matchString from common/util.h and read the Doxygen docs,
	then combine this with the old code below...
	*/
}


/*
void DSSaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	memset(marks, true, num * sizeof(bool));
	return;

	memset(marks, false, num*sizeof(bool));

	for (int saveNum = 0; saveNum < num; saveNum++) {
		for (int r = 0; r < 8; r++) {
			if (gbaSave[r].isValid() && (gbaSave[r].matches(prefix, saveNum))) {
				marks[saveNum] = true;
			}
		}
	}

}
*/


DSSaveFile *DSSaveFileManager::makeSaveFile(const char *filename, bool saveOrLoad) {

	// Find a free save slot
	int r = 0;

	while ((r < 8) && (gbaSave[r].isValid())) {
		r++;
	}

	if ((r == 8) && (gbaSave[r].isValid())) {
		// No more saves
		return NULL;
	} else {
		// Allocate this save
//		consolePrintf("Allocated save %d\n", r);
		gbaSave[r].setName((char *) filename);
		gbaSave[r].reset();
		return gbaSave[r].clone();
	}
}

void DSSaveFileManager::flushToSaveRAM() {
	int cartAddr = 1;
	int s;
	int extraData = DSSaveFileManager::getExtraData();

	*((u16 *) (0x4000204)) |= 0x3;

	swiWaitForVBlank();

	int size = 0;
	for (int r = 0; (r < 8); r++) {
		if (gbaSave[r].isValid()) {
			gbaSave[r].compress();
			if (!gbaSave[r].isTemp()) size += gbaSave[r].getRamUsage();
		}
	}

	if (size <= SRAM_SAVE_MAX) {

		for (int r = 0; r < SRAM_SAVE_MAX; r++) {
			*(CART_RAM + r) = 0;
		}

		sramBytesFree = SRAM_SAVE_MAX;

		for (int r = 0; (r < 8); r++) {
			if (gbaSave[r].isValid() && (!gbaSave[r].isTemp())) {

				cartAddr += s = gbaSave[r].saveToSaveRAM(CART_RAM + cartAddr);

	/*			if (s == 0) {
					consolePrintf("WARNING: Save didn't fit in cart RAM and has been lost!!  Delete files and save again.", gbaSave[r].getName());
					failed = true;
				}*/
			}
		}
	} else {

		consolePrintf("WARNING: Save didn't fit in cart RAM and has been lost!!  Delete files and save again.");
		loadAllFromSRAM();

	}

	DSSaveFileManager::setExtraData(extraData);
//	consolePrintf("SRAM free: %d bytes\n", getBytesFree());
}

void DSSaveFileManager::setExtraData(int data) {
	// Offset of extra data is 31.  This overlaps the padding and reserved bytes of the first save entry.
	// which have not been used up until now.  So it should be safe.

	vu8* sram = CART_RAM + 31;

	*(sram + 0) = 0xF0;		// This is an identifier to check
	*(sram + 1) = 0x0D;		// that extra data is present.

	*(sram + 2) = (data & 0xFF000000) >> 24;		// Now write the actual data
	*(sram + 3) = (data & 0x00FF0000) >> 16;		// taking care to use single
	*(sram + 4) = (data & 0x0000FF00) >> 8;			// byte writes (it's an 8-bit bus)
	*(sram + 5) = (data & 0x000000FF);
}

bool DSSaveFileManager::isExtraDataPresent() {
	vu8* sram = CART_RAM + 31;

	// Check for the identifier
	return ((*(sram + 0) == 0xF0) && (*(sram + 1) == 0x0D));
}

int DSSaveFileManager::getExtraData() {
	vu8* sram = CART_RAM + 31;

	if (isExtraDataPresent()) {
		int value = (*(sram + 2) << 24) | (*(sram + 3) << 16) | (*(sram + 4) << 8) | (*(sram + 5));
		return value;
	} else {
		return 0;
	}
}

#endif
