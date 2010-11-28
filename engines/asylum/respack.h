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
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_RESOURCEPACK_H
#define ASYLUM_RESOURCEPACK_H

#include "asylum/shared.h"

#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"

namespace Asylum {

class ResourceManager;

struct ResourceEntry {
	byte   *data;
	uint32  size;
	uint32  offset;

	ResourceEntry() {
		data = NULL;
		size = 0;
		offset = 0;
	}

	uint32 getData(uint32 off) {
		return READ_UINT32((byte *)this + off);
	}
};

class ResourcePack {
public:
	ResourceEntry *get(uint16 index);
	int count();

protected:
	ResourcePack(Common::String filename);
	~ResourcePack();

private:
	Common::Array<ResourceEntry> _resources;
	Common::File _packFile;

	void init(Common::String filename);

	friend class ResourceManager;
};

class ResourceManager {
public:
	ResourceManager();
	virtual ~ResourceManager() {};

	/**
	 * Get a resource entry
	 *
	 * @param id The ResourceId to get.
	 *
	 * @return the resource entry
	 */
	ResourceEntry *get(ResourceId id);

	/**
	 * Unloads the resources associated with the id
	 *
	 * @param id The identifier.
	 */
	void unload(ResourcePackId id);

	//int count(ResourceId id);

	void setCdNumber(int cdNumber) { _cdNumber = cdNumber; }
	void setMusicPackId(ResourcePackId id) { _musicPackId = id; }

private:
	struct ResourcePackId_EqualTo {
		bool operator()(const ResourcePackId &x, const ResourcePackId &y) const { return x == y; }
	};

	struct ResourcePackId_Hash {
		uint operator()(const ResourcePackId &x) const { return x; }
	};

	typedef Common::HashMap<ResourcePackId, ResourcePack*, ResourcePackId_Hash, ResourcePackId_EqualTo> ResourceCache;

	ResourceCache _resources;
	ResourceCache _music;

	int            _cdNumber;
	ResourcePackId _musicPackId;
};

} // end of namespace Asylum

#endif
