/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#ifndef QUEEN_RESOURCE_H
#define QUEEN_RESOURCE_H

#include "common/file.h"
#include "common/util.h"
#include "queen/defs.h"

namespace Queen {

enum GameFeatures {
	GF_DEMO      = 1 << 0, // demo
	GF_TALKIE    = 1 << 1, // equivalent to cdrom version check
	GF_FLOPPY    = 1 << 2, // floppy, ie. non-talkie version
	GF_INTERVIEW = 1 << 3, // interview demo
	GF_REBUILT   = 1 << 4  // version rebuilt with the 'compression_queen' tool
};

struct RetailGameVersion {
	char str[6];
	uint32 tableOffset;
	uint32 dataFileSize;
};

struct DetectedGameVersion {
	Common::Language language;
	uint8 features;
	uint8 compression;
	char str[6];
	uint32 tableOffset;
};

struct ResourceEntry {
	char filename[13];
	uint8 bundle;
	uint32 offset;
	uint32 size;
};

class Resource {
public:

	Resource();
	~Resource();

	//! loads the specified from the resource file
	uint8 *loadFile(const char *filename, uint32 skipBytes = 0, uint32 *size = NULL);

	//! returns true if the file is present in the resource
	bool fileExists(const char *filename) const { return resourceEntry(filename) != NULL; }

	//! returns a reference to a sound file
	Common::File *giveSound(const char *filename, uint32 *size);

	bool isDemo() const { return (_version.features & GF_DEMO) != 0; }
	bool isInterview() const { return (_version.features & GF_INTERVIEW) != 0; }
	bool isFloppy() const { return (_version.features & GF_FLOPPY) != 0; }
	bool isCD() const { return (_version.features & GF_TALKIE) != 0; }

	//! returns compression type for audio files
	uint8 getCompression() const { return _version.compression; }

	//! returns JAS version string (contains language, platform and version information)
	const char *getJASVersion() const { return _version.str; }

	//! returns the language of the game
	Common::Language getLanguage() const { return _version.language; }

	//! detect game version
	static bool detectVersion(DetectedGameVersion *ver, Common::File *f);

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
		VER_HEB_TALKIE	 = 9,
		VER_DEMO_PCGAMES = 10,
		VER_DEMO         = 11,
		VER_INTERVIEW    = 12,

		VER_COUNT        = 13
	};

	enum {
		CURRENT_TBL_VERSION = 1
	};

	enum {
		JAS_VERSION_OFFSET_DEMO = 0x119A8,
		JAS_VERSION_OFFSET_INTV	= 0xCF8,
		JAS_VERSION_OFFSET_PC	= 0x12484
	};

protected:

	Common::File _resourceFile;

	DetectedGameVersion _version;

	//! number of entries in resource table
	uint32 _resourceEntries;

	ResourceEntry *_resourceTable;

	//! verify the version of the selected game
	void checkJASVersion();

	//! returns a reference to the ReseourceEntry for the specified filename
	ResourceEntry *resourceEntry(const char *filename) const;

	//! extarct the resource table for the specified game version
	void readTableFile(uint32 offset);

	//! read the resource table from the specified file
	void readTableEntries(Common::File *file);

	//! detect game version based on queen.1 datafile size
	static const RetailGameVersion *detectGameVersionFromSize(uint32 size);

	//! resource table filename (queen.tbl)
	static const char *_tableFilename;

	//! known FOTAQ versions
	static const RetailGameVersion _gameVersions[];

#ifndef PALMOS_68K
	//! resource table for english floppy version
	static ResourceEntry _resourceTablePEM10[];
#endif
};

class LineReader {
public:

	LineReader(char *buffer, uint32 bufsize);
	~LineReader();
	char *nextLine();

private:

	char *_buffer;
	uint32 _bufSize;
	int _current;
};

} // End of namespace Queen

#endif
