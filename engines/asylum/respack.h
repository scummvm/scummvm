/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_RESPACK_H
#define ASYLUM_RESPACK_H

#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"

#include "asylum/asylum.h"
#include "asylum/shared.h"

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
		if (data == NULL)
			error("[ResourceEntry::getData] Invalid data");

		return READ_LE_UINT32(data + off);
	}
};

class ResourcePack {
public:
	ResourceEntry *get(uint16 index);

protected:
	ResourcePack(const Common::String &filename);
	~ResourcePack();

private:
	Common::Array<ResourceEntry> _resources;
	Common::File _packFile;

	void init(const Common::String &filename);

	friend class ResourceManager;
};

class ResourceManager {
public:
	ResourceManager(AsylumEngine *vm);
	~ResourceManager();

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

	int getCdNumber() { return _cdNumber; }
	void setCdNumber(int cdNumber) { _cdNumber = cdNumber; }
	void setMusicPackId(ResourcePackId id) { _musicPackId = id; }
	void clearSharedSoundCache() { _resources.erase(kResourcePackSharedSound); }
	void clearMusicCache() { _music.erase(kResourcePackMusic); }

private:
	struct ResourcePackId_EqualTo {
		bool operator()(const ResourcePackId &x, const ResourcePackId &y) const { return x == y; }
	};

	struct ResourcePackId_Hash {
		uint operator()(const ResourcePackId &x) const { return x; }
	};

	typedef Common::HashMap<ResourcePackId, ResourcePack *, ResourcePackId_Hash, ResourcePackId_EqualTo> ResourceCache;

	ResourceCache _resources;
	ResourceCache _music;

	int            _cdNumber;
	ResourcePackId _musicPackId;
	AsylumEngine  *_vm;
};

} // end of namespace Asylum

#endif // ASYLUM_RESPACK_H
