/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
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

#include "common/file.h"

#include "cine/cine.h"
#include "cine/resource.h"
#include "cine/unpack.h"
#include "cine/various.h"

namespace Cine {

void checkDataDisk(int16 param) {
}

/* FW specific */
static Common::File *snd_baseSndFile = NULL;
static uint16 snd_numBasesonEntries = 0;
static BasesonEntry *snd_basesonEntries = NULL;

int snd_loadBasesonEntries(const char *fileName) {
	int i;

	snd_baseSndFile = new Common::File();
	snd_baseSndFile->open(fileName);
	if (!snd_baseSndFile->isOpen())
		return -1;

	snd_numBasesonEntries = snd_baseSndFile->readUint16BE();
	snd_baseSndFile->readUint16BE();	/* entry_size */
	snd_basesonEntries = (BasesonEntry *)malloc(snd_numBasesonEntries * sizeof(BasesonEntry));
	if (snd_basesonEntries) {
		for (i = 0; i < snd_numBasesonEntries; ++i) {
			BasesonEntry *be = &snd_basesonEntries[i];
			snd_baseSndFile->read(be->name, 14);
			be->offset = snd_baseSndFile->readUint32BE();
			be->size = snd_baseSndFile->readUint32BE();
			be->unpackedSize = snd_baseSndFile->readUint32BE();
			snd_baseSndFile->readUint32BE();	/* unused */
		}
	}
	return 0;
}

void snd_clearBasesonEntries() {
	snd_baseSndFile->close();
	delete snd_baseSndFile;
	free(snd_basesonEntries);
	snd_basesonEntries = NULL;
	snd_numBasesonEntries = 0;
}

static int snd_findBasesonEntry(const char *entryName) {
	int i;
	char *p;
	char basesonEntryName[20];

	assert(strlen(entryName) < 20);
	strcpy(basesonEntryName, entryName);
	for (p = basesonEntryName; *p; ++p) {
		if (*p >= 'a' && *p <= 'z')
			*p += 'A' - 'a';
	}

	for (i = 0; i < snd_numBasesonEntries; ++i) {
		if (strcmp(snd_basesonEntries[i].name, basesonEntryName) == 0)
			return i;
	}
	return -1;
}

byte *snd_loadBasesonEntry(const char *entryName) {
	int entryNum;
	byte *entryData = NULL;

	if (gameType == Cine::GID_OS) {
		entryNum = findFileInBundle((const char *)entryName);
		if (entryNum != -1)
			entryData = readBundleFile(entryNum);
	} else {
		entryNum = snd_findBasesonEntry(entryName);
		if (entryNum != -1 && entryNum < snd_numBasesonEntries) {
			const BasesonEntry *be = &snd_basesonEntries[entryNum];
			entryData = (byte *)malloc(be->unpackedSize);
			if (entryData) {
				if (be->unpackedSize > be->size) {
					byte *tempData = (byte *)malloc(be->size);
					if (tempData) {
						snd_baseSndFile->seek(be->offset, SEEK_SET);
						snd_baseSndFile->read(tempData, be->size);
						delphineUnpack(entryData, tempData, be->size);
						free(tempData);
					}
				} else {
					snd_baseSndFile->seek(be->offset, SEEK_SET);
					snd_baseSndFile->read(entryData, be->size);
				}
			}
		}
	}

	return entryData;
}

} // End of namespace Cine
