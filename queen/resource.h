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

namespace Queen {

struct ResourceEntry {
	char filename[13];
	uint8 inBundle;
	uint32 offset;
	uint32 size;
};

struct GameVersion {
	char versionString[6];
	uint32 resourceEntries;
	bool isFloppy;   
	bool isDemo;
	const struct ResourceEntry *resourceTable;	
};


class QueenResource {

public:
	QueenResource(char *datafilePath);
	~QueenResource(void);
	uint8 *loadFile(const char *filename, uint32 skipBytes = 0);
	bool exists(const char *filename);
	bool isDemo();

protected:
	File *_resourceFile;
	char *_datafilePath;
	const struct GameVersion *_gameVersion;
	static const struct GameVersion _gameVersionPE100v1;
	static const struct GameVersion _gameVersionPE100v2;
	static const struct GameVersion _gameVersionPEM10;
	static const struct GameVersion _gameVersionCEM10;
	static const struct ResourceEntry _resourceTablePE100v1[];
	static const struct ResourceEntry _resourceTablePE100v2[];
	static const struct ResourceEntry _resourceTablePEM10[];
	static const struct ResourceEntry _resourceTableCEM10[];

	int32 resourceIndex(const char *filename);
	uint32 fileSize(const char *filename);
	uint32 fileOffset(const char *filename);
	const char *JASVersion();
};

} // End of namespace Queen

#endif

