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

#ifndef SHERLOCK_RESOURCES_H
#define SHERLOCK_RESOURCES_H

#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/stream.h"

namespace Sherlock {

typedef Common::Array<byte> CacheEntry;
typedef Common::HashMap<Common::String, CacheEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CacheHash;

struct LibraryEntry {
	uint32 _offset, _size;
	int _index;

	LibraryEntry() : _index(0), _offset(0), _size(0) {}
	LibraryEntry(int index, uint32 offset, uint32 size) : 
		_index(index), _offset(offset), _size(size) {}
};
typedef Common::HashMap<Common::String, LibraryEntry, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LibraryIndex;
typedef Common::HashMap<Common::String, LibraryIndex, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> LibraryIndexes;

class SherlockEngine;

class Cache {
private:
	CacheHash _resources;
public:
	Cache();

	bool isCached(const Common::String &filename) const;

	void load(const Common::String &name);
	void load(const Common::String &name, Common::SeekableReadStream &stream);

	Common::SeekableReadStream *get(const Common::String &filename) const;
};

class Resources {
private:
	Cache _cache;
	LibraryIndexes _indexes;
	int _resourceIndex;

	void loadLibraryIndex(const Common::String &libFilename, Common::SeekableReadStream *stream);
public:
	Resources();

	void addToCache(const Common::String &filename);
	void addToCache(const Common::String &filename, const Common::String &libFilename);
	bool isInCache(const Common::String &filename) const { return _cache.isCached(filename); }

	Common::SeekableReadStream *load(const Common::String &filename);

	Common::SeekableReadStream *load(const Common::String &filename, const Common::String &libraryFile);

	int resourceIndex() const;
};


} // End of namespace Sherlock

#endif
