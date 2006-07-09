/* Ramsave
 * Copyright (C) 2002-2004 Neil Millstone
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
 // Save in order 1,2,3,4,larger 2,5
#include "stdafx.h"
#include "system.h"
#include "ramsave.h"
#include "nds.h"
#include "compressor/lz.h"

#define CART_RAM ((vu8 *) (0x0A000000))

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
}

DSSaveFile::~DSSaveFile() {
	if (!ownsData) {
		*origHeader = save;
		DSSaveFileManager::instance()->flushToSaveRAM();
	}
	if (ownsData) {
		delete saveData;
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
		
		if (ownsData) delete this->saveData;
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
		
		
		
		delete saveData;
		
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
}

uint32 DSSaveFile::read(void *buf, uint32 size) {
	if (ptr + size > save.size) {
		size = save.size - ptr;
		if (size < 0) size = 0;
	}
	memcpy(buf, saveData + ptr, size);
//	consolePrintf("byte: %d ", ((u8 *) (buf))[0]);
	
	ptr += size;
	return size;
}

uint32 DSSaveFile::pos() const {
	return ptr;
}

uint32 DSSaveFile::size() const {
	return save.size;
}

void DSSaveFile::seek(int32 pos, int whence) {
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
			ptr = save.size - pos;
			break;
		}
	}
}

bool DSSaveFile::eos() const {
	return ptr >= (int) save.size;
}

void DSSaveFile::skip(uint32 bytes) {
	ptr = ptr + bytes;
	if (ptr > (int) save.size) ptr = save.size;
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

bool DSSaveFile::matches(char* prefix, int num) {
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

bool DSSaveFile::matches(char* filename) {
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
			delete saveData;
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
			delete saveData;
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

	sramBytesFree = 65533;

	// Try to find saves in save RAM
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].loadFromSaveRAM(CART_RAM + addr)) {
			addr += gbaSave[r].getRamUsage();
			sramBytesFree -= gbaSave[r].getRamUsage();
		}
	}

}

void DSSaveFileManager::formatSram() {
	for (int r = 0; r < 65533; r++) {
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

Common::SaveFile *DSSaveFileManager::openSavefile(const char* filename, bool saveOrLoad) {
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].isValid() && (gbaSave[r].matches((char *) filename))) {
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

void DSSaveFileManager::deleteFile(char* name) {
//	consolePrintf("Deleting %s", name);
	for (int r = 0; r < 8; r++) {
		if (gbaSave[r].isValid() && (gbaSave[r].matches((char *) name))) {
			gbaSave[r].deleteFile();
		}
	}
	flushToSaveRAM();
}

void DSSaveFileManager::listSavefiles(const char *prefix, bool *marks, int num) {
	memset(marks, false, num*sizeof(bool));

	for (int saveNum = 0; saveNum < num; saveNum++) {
		for (int r = 0; r < 8; r++) {
			if (gbaSave[r].isValid() && (gbaSave[r].matches((char *) prefix, saveNum))) {
				marks[saveNum] = true;
			}
		}
	}
	
}

Common::SaveFile *DSSaveFileManager::makeSaveFile(const char *filename, bool saveOrLoad) {
	
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
	
	*((u16 *) (0x4000204)) |= 0x3;
	
	swiWaitForVBlank();

	int size = 0;
	for (int r = 0; (r < 8); r++) {
		if (gbaSave[r].isValid()) {
			gbaSave[r].compress();
			if (!gbaSave[r].isTemp()) size += gbaSave[r].getRamUsage();
		}
	}
	
	if (size <= 65533) {

		for (int r = 0; r < 65533; r++) {
			*(CART_RAM + r) = 0;
		}
		
		sramBytesFree = 65533;
		
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
//	consolePrintf("SRAM free: %d bytes\n", getBytesFree());
}
