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

#include "gbampsave.h"
#include "fat/gba_nds_fat.h"
#include "backends/fs/ds/ds-fs.h"
#include "common/config-manager.h"

/////////////////////////
// GBAMP Save File
/////////////////////////

GBAMPSaveFile::GBAMPSaveFile(char* name, bool saveOrLoad) {
	handle = DS::std_fopen(name, saveOrLoad? "w": "r");
//	consolePrintf("%s handle is %d\n", name, handle);
//	consolePrintf("Created %s\n", name);
	bufferPos = 0;
	saveSize = 0;
	flushed = 0;
}

GBAMPSaveFile::~GBAMPSaveFile() {
	flushSaveBuffer();
	if (handle) DS::std_fclose(handle);
//	consolePrintf("Closed file\n");
}

uint32 GBAMPSaveFile::read(void *buf, uint32 size) {
	saveSize += size;
//	consolePrintf("Read %d %d ", size, saveSize);
	return DS::std_fread(buf, 1, size, handle);
}

bool GBAMPSaveFile::eos() const {
	return DS::std_feof(handle);
}

bool GBAMPSaveFile::skip(uint32 bytes) {
	return DS::std_fseek(handle, bytes, SEEK_CUR) == 0;
}

void GBAMPSaveFile::flushSaveBuffer() {
	if (bufferPos != 0) {
//		consolePrintf("Flushing %d bytes from %x\n", bufferPos, buffer);
		flushed += bufferPos;
		DS::std_fwrite(buffer, 1, bufferPos, handle);
		bufferPos = 0;
	}
}

int32 GBAMPSaveFile::pos() const {
	return DS::std_ftell(handle);
}

int32 GBAMPSaveFile::size() const {
	int position = pos();
	DS::std_fseek(handle, 0, SEEK_END);
	int size = DS::std_ftell(handle);
	DS::std_fseek(handle, position, SEEK_SET);
	return size;
}

void GBAMPSaveFile::seek(int32 pos, int whence) {
	return DS::std_fseek(handle, pos, whence) == 0;
}


uint32 GBAMPSaveFile::write(const void *buf, uint32 size) {
	if (bufferPos + size > SAVE_BUFFER_SIZE) {
		flushSaveBuffer();
		saveSize += size;
//		consolePrintf("Writing %d bytes from %x", size, buf);
//		DS::std_fwrite(buf, 1, size, handle);

		memcpy(buffer + bufferPos, buf, size);
		bufferPos += size;
		
		saveSize += size;


/*		int pos = 0;
		
		int rest = SAVE_BUFFER_SIZE - bufferPos;
		memcpy(buffer + bufferPos, buf, rest);
		bufferPos = 512;
		pos += rest;
		flushSaveBuffer();		
		size -= rest;
//		consolePrintf("First section: %d\n", rest);
		
		while (size >= 512) {
			DS::std_fwrite(((char *) (buf)) + pos, 1, 512, handle);
			size -= 512;
			pos += 512;
//			consolePrintf("Full chunk, %d left ", size);
		}
		
		bufferPos = 0;
		memcpy(buffer + bufferPos, ((char *) (buf)) + pos, size);
		bufferPos += size;
//		consolePrintf("%d left in buffer ", bufferPos);*/
		
	} else {
	
		memcpy(buffer + bufferPos, buf, size);
		bufferPos += size;
		
		saveSize += size;
	}
	
//	if ((size > 100) || (size <= 0)) consolePrintf("Write %d bytes\n", size);
	return size;
}


//////////////////////////
// GBAMP Save File Manager
//////////////////////////

GBAMPSaveFileManager::GBAMPSaveFileManager() {

}

GBAMPSaveFileManager::~GBAMPSaveFileManager() {

}

GBAMPSaveFile* GBAMPSaveFileManager::openSavefile(char const* name, bool saveOrLoad) {
	char fileSpec[128];
	
	strcpy(fileSpec, getSavePath());
	
	if (fileSpec[strlen(fileSpec) - 1] == '/') {
		sprintf(fileSpec, "%s%s", getSavePath(), name);
	} else {
		sprintf(fileSpec, "%s/%s", getSavePath(), name);
	}
	
//	consolePrintf(fileSpec);
	GBAMPSaveFile* sf = new GBAMPSaveFile(fileSpec, saveOrLoad);
	if (sf->isOpen()) {
		return sf;	
	} else {
		delete sf;
		return NULL;	
	}
}

// This method copied from an old version of the savefile.cpp, since it's been removed from there and
// placed in default-saves.cpp, where I cannot call it.
const char *GBAMPSaveFileManager::getSavePath() const {
	const char *dir = NULL;

	// Try to use game specific savepath from config
	dir = ConfMan.get("savepath").c_str();

	// Work around a bug (#999122) in the original 0.6.1 release of
	// ScummVM, which would insert a bad savepath value into config files.
	if (0 == strcmp(dir, "None")) {
		ConfMan.removeKey("savepath", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
		dir = ConfMan.get("savepath").c_str();
	}


	assert(dir);

	return dir;
}

Common::StringList GBAMPSaveFileManager::listSavefiles(const char *pattern) { 

	enum { TYPE_NO_MORE = 0, TYPE_FILE = 1, TYPE_DIR = 2 };
	char name[256];
	
	DS::std_cwd((char*)getSavePath()); //TODO : Check this suspicious const-cast
//	consolePrintf("Save path: '%s', pattern: '%s'\n", getSavePath(),pattern);

	
	int fileType = FAT_FindFirstFileLFN(name);

	Common::StringList list;

	do {
	
		if (fileType == TYPE_FILE) {

			FAT_GetLongFilename(name);

			for (int r = 0; r < strlen(name); r++) {
				name[r] = tolower(name[r]);
			}
			
			
			if (Common::matchString(name, pattern)) {
				list.push_back(name);
			}
		}

	} while ((fileType = FAT_FindNextFileLFN(name)));
	
	FAT_chdir("/");

	return list;
}	


