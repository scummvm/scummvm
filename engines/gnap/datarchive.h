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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GNAP_DATARCHIVE_H
#define GNAP_DATARCHIVE_H

#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "engines/engine.h"

namespace Gnap {

struct DatEntry {
	uint32 _ofs;
	uint32 _outSize1;
	uint32 _type;
	uint32 _outSize2;
};

class DatArchive {
public:
	DatArchive(const char *filename);
	~DatArchive();
	byte *load(int index);
	int getCount() const { return _entriesCount; }
	uint32 getEntryType(int index) { return _entries[index]._type; }
	uint32 getEntrySize(int index) { return _entries[index]._outSize1; }
protected:
	Common::File *_fd;
	int _entriesCount;
	DatEntry *_entries;
};

const int kMaxDatArchives = 2;

class DatManager {
public:
	DatManager();
	~DatManager();
	void open(int index, const char *filename);
	void close(int index);
	byte *loadResource(int resourceId);
	uint32 getResourceType(int resourceId);
	uint32 getResourceSize(int resourceId);
protected:
	DatArchive *_datArchives[kMaxDatArchives];
};

int ridToDatIndex(int resourceId);
int ridToEntryIndex(int resourceId);
int makeRid(int datIndex, int entryIndex);

} // End of namespace Gnap

#endif // GNAP_DATARCHIVE_H
