/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#ifndef QUEENRESOURCE_H
#define QUEENRESOURCE_H

#include "common/file.h"
#include "common/util.h"
#include "queen/defs.h"

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
	// VER_SPA_FLOPPY
	VER_SPA_TALKIE   = 8,
	// VER_HEB_FLOPPY
	// VER_HEB_TALKIE
	VER_DEMO_PCGAMES = 9,
	VER_DEMO         = 10,
	VER_INTERVIEW    = 11,

	VER_COUNT        = 12
};

struct ResourceEntry {
	char filename[13];
	uint8 bundle;
	uint32 offset;
	uint32 size;
};

struct GameVersion {
	char versionString[6];
	uint32 tableOffset;
	uint32 dataFileSize;
};

class LineReader {

public:
	LineReader(char *buffer);
	~LineReader();
	char* nextLine();

private:
	char *_buffer;
	int _current;
};

class Resource {

public:
	Resource(const Common::String &datafilePath);
	~Resource(void);

	uint8 *loadFile(const char *filename, uint32 skipBytes = 0, byte *dstBuf = NULL);
	uint8 *loadFileMalloc(const char *filename, uint32 skipBytes = 0, byte *dstBuf = NULL);
	bool fileExists(const char *filename) const { return resourceEntry(filename) != NULL; }
	uint32 fileSize(const char *filename) const { return resourceEntry(filename)->size; }

	File *giveCompressedSound(const char *filename);

	bool isDemo() const { return !strcmp(_versionString, "PE100"); }
	bool isInterview() const { return !strcmp(_versionString, "PEint"); }
	bool isFloppy() const { return _versionString[0] == 'P'; }
	uint8 compression() const { return _compression; }
	const char *JASVersion() const { return _versionString; }
	Language getLanguage() const;

	enum {
		JAS_VERSION_OFFSET_DEMO = 0x119A8,
		JAS_VERSION_OFFSET_INTV = 0xCF8,
		JAS_VERSION_OFFSET	    = 0x12484
	};

protected:
	File *_resourceFile;
	uint8 _compression;
	const Common::String _datafilePath;
	char _versionString[6];
	const char *_savePath;
	uint32 _resourceEntries;
	ResourceEntry *_resourceTable;

	bool findNormalVersion();
	bool findCompressedVersion();
	void checkJASVersion();
	int32 resourceIndex(const char *filename) const;
	ResourceEntry *resourceEntry(const char *filename) const;
	bool readTableFile(const GameVersion *gameVersion);
	void readTableCompResource();
	void readTableEntries(File *file);
	const GameVersion *detectGameVersion(uint32 size) const;

	static const char *_tableFilename;
	static const GameVersion _gameVersions[];
	static ResourceEntry _resourceTablePEM10[];
};

} // End of namespace Queen

#endif

