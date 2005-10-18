/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef SKYDISK_H
#define SKYDISK_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "sky/rnc_deco.h"

namespace Common {
	class File;
}

namespace Sky {

#define MAX_FILES_IN_LIST 60

struct PrefFile {
	uint8 *data;
	uint16 fileNr;
	uint32 fileSize;
	PrefFile *next;
};

class Disk {
public:
	Disk(const Common::String &gameDataPath);
	~Disk(void);

	uint8 *loadFile(uint16 fileNr);
	bool fileExists(uint16 fileNr);

	void prefetchFile(uint16 fileNr);
	void flushPrefetched(void);

	uint32 determineGameVersion();

	uint32 _lastLoadedFileSize;

	void fnMiniLoad(uint16 fileNum);
	void fnCacheFast(uint16 *fList);
	void fnCacheChip(uint16 *fList);
	void fnCacheFiles(void);
	void fnFlushBuffers(void);
	uint32 *giveLoadedFilesList(void) { return _loadedFilesList; };
	void refreshFilesList(uint32 *list);

protected:

	PrefFile *_prefRoot;
	uint8 *givePrefetched(uint16 fileNr, uint32 *fSize);

	uint8 *getFileInfo(uint16 fileNr);
	void dumpFile(uint16 fileNr);

	uint32 _dinnerTableEntries;
	uint8 *_dinnerTableArea;
	Common::File *_dataDiskHandle;
	Common::File *_dnrHandle;
	RncDecoder _rncDecoder;

	uint16 _buildList[MAX_FILES_IN_LIST];
	uint32 _loadedFilesList[MAX_FILES_IN_LIST];
};

} // End of namespace Sky

#endif
