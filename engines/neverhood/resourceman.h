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

class ResourceMan;

struct ResourceHandle {
friend class ResourceMan;
public:
	ResourceHandle();
	~ResourceHandle();
	bool isValid() const { return _resourceFileEntry != NULL && _resourceFileEntry->archiveEntry != NULL; }
	byte type() const { return isValid() ? _resourceFileEntry->archiveEntry->type : 0; };
	const byte *data() const { return _data; }
	uint32 size() const { return isValid() ? _resourceFileEntry->archiveEntry->size : 0; };
	const byte *extData() const { return _extData; };
	uint32 fileHash() const { return isValid() ? _resourceFileEntry->archiveEntry->fileHash : 0; };
protected:
	ResourceFileEntry *_resourceFileEntry;
	const byte *_extData;
	const byte *_data;
};

class ResourceMan {
public:
	ResourceMan();
	~ResourceMan();
	void addArchive(const Common::String &filename);
	ResourceFileEntry *findEntrySimple(uint32 fileHash);
	ResourceFileEntry *findEntry(uint32 fileHash, ResourceFileEntry **firstEntry = NULL);
	Common::SeekableReadStream *createStream(uint32 fileHash);
	const ResourceFileEntry& getEntry(uint index) { return _entries[index]; }
	uint getEntryCount() { return _entries.size(); }
	void queryResource(uint32 fileHash, ResourceHandle &resourceHandle);
	void loadResource(ResourceHandle &resourceHandle, bool applyResourceFixes);
	void unloadResource(ResourceHandle &resourceHandle);
	void purgeResources();
protected:
	typedef Common::HashMap<uint32, ResourceFileEntry> EntriesMap;
	Common::Array<BlbArchive*> _archives;
	EntriesMap _entries;
	Common::HashMap<uint32, ResourceData*> _data;
	Common::Array<Resource*> _resources;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_RESOURCEMAN_H */
