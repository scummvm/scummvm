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
 */

#ifndef NEVERHOOD_RESOURCEMAN_H
#define NEVERHOOD_RESOURCEMAN_H

#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "neverhood/neverhood.h"
#include "neverhood/blbarchive.h"

namespace Neverhood {

struct ResourceFileEntry {
	int resourceHandle;
	BlbArchive *archive;
	BlbArchiveEntry *archiveEntry;
};

struct Resource {
	ResourceFileEntry *entry;
	int useRefCount;
};

struct ResourceData {
	byte *data;
	int dataRefCount;
	ResourceData() : data(NULL), dataRefCount() {}
};

#if 0
class ResourceMan;

struct ResourceHandle {
public:
	ResourceHandle();
	~ResourceHandle();
	const byte *data();
	uint32 size() const { return _archiveEntry ? _archiveEntry->size : 0 };
protected:
	ResourceMan *_res;
	ResourceFileEntry *_resourceFileEntry;
};
#endif

class ResourceMan {
public:
	ResourceMan();
	~ResourceMan();
	void addArchive(const Common::String &filename);
	ResourceFileEntry *findEntrySimple(uint32 fileHash);
	ResourceFileEntry *findEntry(uint32 fileHash);
	int useResource(uint32 fileHash);
	void unuseResource(int resourceHandle);
	uint32 getResourceSize(int resourceHandle) const;
	byte getResourceType(int resourceHandle);
	byte getResourceTypeByHash(uint32 fileHash);
	byte *getResourceExtData(int resourceHandle);
	byte *getResourceExtDataByHash(uint32 fileHash);
	byte *loadResource(int resourceHandle, bool moveToFront = false);
	void unloadResource(int resourceHandle);
	Common::SeekableReadStream *createStream(uint32 fileHash);
	const ResourceFileEntry& getEntry(uint index) { return _entries[index]; }
	uint getEntryCount() { return _entries.size(); }
#if 0	
	ResourceHandle getResource(uint32 fileHash);
#endif
protected:
	typedef Common::HashMap<uint32, ResourceFileEntry> EntriesMap;
	Common::Array<BlbArchive*> _archives;
	EntriesMap _entries;
	Common::HashMap<uint32, ResourceData*> _data;
	Common::Array<Resource*> _resources;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_RESOURCEMAN_H */
