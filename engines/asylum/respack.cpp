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

#include "asylum/respack.h"

namespace Asylum {

//////////////////////////////////////////////////////////////////////////
// ResourceManager
//////////////////////////////////////////////////////////////////////////

ResourceManager::ResourceManager() : _cdNumber(-1), _musicPackId(kResourcePackInvalid) {
}

ResourceEntry *ResourceManager::get(ResourceId id) {
	ResourcePackId packId = RESOURCE_PACK(id);
	uint16 index = RESOURCE_INDEX(id);

	// Check if we need to load a music pack
	bool isMusicPack = (packId == kResourcePackMusic);

	// Check that a music pack has been set
	if (isMusicPack && _musicPackId == kResourcePackInvalid)
		error("[ResourceManager::get] Current music pack Id has not been set!");

	ResourceCache *cache = isMusicPack ? &_music : &_resources;

	// Try getting the resource pack
	if (!cache->contains(packId)) {
		ResourcePack *pack;

		if (isMusicPack) {
			pack = new ResourcePack(Common::String::format("mus.%03d", _musicPackId));
		} else {
			if (packId == kResourcePackSharedSound) {
				if (_cdNumber == -1)
					error("[ResourceManager::get] Cd number has not been set!");

				pack = new ResourcePack(Common::String::format("res.%01d%02d", _cdNumber, packId));
			} else {
				pack = new ResourcePack(Common::String::format("res.%03d", packId));
			}
		}

		cache->setVal(packId, pack);
	}

	return cache->getVal(packId)->get(index);
}

void ResourceManager::unload(ResourcePackId id) {
	if (_resources.contains(id))
		_resources.erase(id);

	if (_music.contains(id))
		_music.erase(id);
}

//////////////////////////////////////////////////////////////////////////
// ResourcePack
//////////////////////////////////////////////////////////////////////////
ResourcePack::ResourcePack(Common::String filename) {
	init(filename);
}

ResourcePack::~ResourcePack() {
	for (uint32 i = 0; i < _resources.size(); i++)
		delete [] _resources[i].data;

	_resources.clear();
	_packFile.close();
}

void ResourcePack::init(Common::String filename) {
	if (!_packFile.open(filename))
		error("[ResourcePack::init] Could not open resource file: %s", filename.c_str());

	uint32 entryCount = _packFile.readUint32LE();
	_resources.resize(entryCount);

	uint32 prevOffset = _packFile.readUint32LE();
	uint32 nextOffset = 0;

	for (uint32 i = 0; i < entryCount; i++) {
		ResourceEntry entry;
		entry.offset = prevOffset;

		// Read the offset of the next entry to determine the size of this one
		nextOffset = (i < entryCount - 1) ? _packFile.readUint32LE() : (uint32)_packFile.size();
		entry.size = (nextOffset > 0) ? nextOffset - prevOffset : (uint32)_packFile.size() - prevOffset;
		entry.data = 0;

		_resources[i] = entry;

		prevOffset = nextOffset;
	}
}

ResourceEntry *ResourcePack::get(uint16 index) {
	if (index > _resources.size() - 1)
		return NULL;

	if (!_resources[index].data) {
		// Load the requested resource if it's not loaded already
		_packFile.seek(_resources[index].offset, SEEK_SET);
		_resources[index].data = new byte[_resources[index].size];
		_packFile.read(_resources[index].data, _resources[index].size);
	}

	return &_resources[index];
}

} // end of namespace Asylum
