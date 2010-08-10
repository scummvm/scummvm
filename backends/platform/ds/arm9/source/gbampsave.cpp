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

#include "gbampsave.h"
#include "fat/gba_nds_fat.h"
#include "backends/fs/ds/ds-fs.h"
#include "common/config-manager.h"

/////////////////////////
// GBAMP Save File
/////////////////////////

GBAMPSaveFile::GBAMPSaveFile(char *name, bool saveOrLoad) {
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

uint32 GBAMPSaveFile::read(void *buf, uint32 length) {
	saveSize += length;
//	consolePrintf("Read %d %d ", length, saveSize);
	return DS::std_fread(buf, 1, length, handle);
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
	int length = DS::std_ftell(handle);
	DS::std_fseek(handle, position, SEEK_SET);
	return length;
}

bool GBAMPSaveFile::seek(int32 newPos, int whence) {
	return DS::std_fseek(handle, newPos, whence) == 0;
}


uint32 GBAMPSaveFile::write(const void *buf, uint32 length) {
	if (bufferPos + length > SAVE_BUFFER_SIZE) {
		flushSaveBuffer();
		saveSize += length;
//		consolePrintf("Writing %d bytes from %x", length, buf);
//		DS::std_fwrite(buf, 1, length, handle);

		memcpy(buffer + bufferPos, buf, length);
		bufferPos += length;

		saveSize += length;


/*		int pos = 0;

		int rest = SAVE_BUFFER_SIZE - bufferPos;
		memcpy(buffer + bufferPos, buf, rest);
		bufferPos = 512;
		pos += rest;
		flushSaveBuffer();
		length -= rest;
//		consolePrintf("First section: %d\n", rest);

		while (length >= 512) {
			DS::std_fwrite(((char *) (buf)) + pos, 1, 512, handle);
			length -= 512;
			pos += 512;
//			consolePrintf("Full chunk, %d left ", length);
		}

		bufferPos = 0;
		memcpy(buffer + bufferPos, ((char *) (buf)) + pos, length);
		bufferPos += length;
//		consolePrintf("%d left in buffer ", bufferPos);*/

	} else {

		memcpy(buffer + bufferPos, buf, length);
		bufferPos += length;

		saveSize += length;
	}

//	if ((length > 100) || (length <= 0)) consolePrintf("Write %d bytes\n", length);
	return length;
}


//////////////////////////
// GBAMP Save File Manager
//////////////////////////

GBAMPSaveFileManager::GBAMPSaveFileManager() {

}

GBAMPSaveFileManager::~GBAMPSaveFileManager() {

}

GBAMPSaveFile *GBAMPSaveFileManager::openSavefile(const char *name, bool saveOrLoad) {
	char fileSpec[128];

	strcpy(fileSpec, getSavePath());

	if (fileSpec[strlen(fileSpec) - 1] == '/') {
		sprintf(fileSpec, "%s%s", getSavePath(), name);
	} else {
		sprintf(fileSpec, "%s/%s", getSavePath(), name);
	}

//	consolePrintf("Opening the file: %s\n", fileSpec);
	GBAMPSaveFile *sf = new GBAMPSaveFile(fileSpec, saveOrLoad);
	if (sf->isOpen()) {
//		consolePrintf("Ok");
		return sf;
	} else {
//		consolePrintf("Fail");
		delete sf;
		return NULL;
	}
}

// This method copied from an old version of the savefile.cpp, since it's been removed from there and
// placed in default-saves.cpp, where I cannot call it.
// FIXME: Does it even make sense to change the "savepath" on the NDS? Considering
// that nothing sets a default value for the "savepath" either, wouldn't it better
// to return a fixed path here?
const char *GBAMPSaveFileManager::getSavePath() const {
	// Try to use game specific savepath from config
	const char *dir = ConfMan.get("savepath").c_str();
	assert(dir);

	return dir;
}

Common::StringArray GBAMPSaveFileManager::listSavefiles(const Common::String &pattern) {

	enum { TYPE_NO_MORE = 0, TYPE_FILE = 1, TYPE_DIR = 2 };
	char name[256];

	{
		char dir[128];
		strcpy(dir, getSavePath());
		char *realName = dir;

		if ((strlen(dir) >= 4) && (dir[0] == 'm') && (dir[1] == 'p') && (dir[2] == ':') && (dir[3] == '/')) {
			realName += 4;
		}

	//	consolePrintf("Real cwd:%d\n", realName);

		char *p = realName;
		while (*p) {
			if (*p == '\\') *p = '/';
			p++;
		}

	//	consolePrintf("Real cwd:%d\n", realName);
		FAT_chdir(realName);

	}

//	consolePrintf("Save path: '%s', pattern: '%s'\n", getSavePath(), pattern);


	int fileType = FAT_FindFirstFileLFN(name);

	Common::StringArray list;

	do {

		if (fileType == TYPE_FILE) {

			FAT_GetLongFilename(name);

			for (int r = 0; name[r] != 0; r++) {
				name[r] = tolower(name[r]);
			}


			if (Common::matchString(name, pattern.c_str())) {
				list.push_back(name);
			}
		}

	} while ((fileType = FAT_FindNextFileLFN(name)));

	FAT_chdir("/");

	return list;
}
