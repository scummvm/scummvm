/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEEN_RESOURCE_H
#define QUEEN_RESOURCE_H

#include "common/file.h"
#include "defs.h"

namespace Queen {

enum Version {
	VER_ENG_FLOPPY   = 0,
	VER_ENG_TALKIE   = 1,
	VER_FRE_FLOPPY   = 2,
	VER_FRE_TALKIE   = 3,
	VER_GER_FLOPPY   = 4,
	VER_GER_TALKIE   = 5,
	VER_ITA_FLOPPY   = 6,
	VER_ITA_TALKIE   = 7,
	VER_SPA_TALKIE   = 8,
	VER_DEMO_PCGAMES = 9,
	VER_DEMO         = 10,

	VER_NUMBER       = 11
};

struct ResourceEntry {
	char filename[13];
	uint8 inBundle;
	uint32 offset;
	uint32 size;
};

struct GameVersion {
	char versionString[6];
	bool isFloppy;   
	bool isDemo;
	uint32 tableOffset;
	uint32 dataFileSize;
};


class Resource {

public:
	Resource(const Common::String &datafilePath, const char *datafileName);
	~Resource(void);
	uint8 *loadFile(const char *filename, uint32 skipBytes = 0, byte *dstBuf = NULL);
	uint8 *loadFileMalloc(const char *filename, uint32 skipBytes = 0, byte *dstBuf = NULL);
	char *getJAS2Line();
	bool exists(const char *filename);
	bool isDemo();
	bool isFloppy();
	uint8 compression()	{ return _compression; }
	uint32 fileSize(const char *filename);
	uint32 fileOffset(const char *filename);
	File *giveCompressedSound(const char *filename);
	Language getLanguage();
	const char *JASVersion();

protected:
	File *_resourceFile;
	char *_JAS2Ptr;
	uint32 _JAS2Pos;
	uint8 _compression;
	const Common::String _datafilePath;
	const GameVersion *_gameVersion;
	uint32 _resourceEntries;
	ResourceEntry *_resourceTable;
	static const GameVersion _gameVersions[];
	static ResourceEntry _resourceTablePEM10[];

	int32 resourceIndex(const char *filename);
	bool readTableFile();
	void readTableCompResource();
	static const GameVersion *detectGameVersion(uint32 dataFilesize);
};

} // End of namespace Queen

#endif

