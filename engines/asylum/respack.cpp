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

#include "asylum/respack.h"

namespace Asylum {

const struct {
	int  cdNumber;
	uint resourceId;
	uint size;
} patchedSizes[] = {
	{1, 0x800402B0, 40146626}, {3, 0x800403EB, 18177962}, {2, 0x8004071D, 40501676},
	{1, 0x8004072D,  7349518}, {2, 0x80040733, 40367314}, {1, 0x8004073B,  5534658},
	{2, 0x8004073C, 40347616}, {1, 0x80040745,  4333670}, {2, 0x80040746, 40214368},
	{3, 0x8004074A, 17247084}, {2, 0x8004074C, 40072902}, {3, 0x80040756, 15741212},
	{2, 0x8004075E, 39099030}, {1, 0x8004076E,  1122128}, {2, 0x80040781, 36131104},
	{3, 0x80040782, 15468752}, {2, 0x80040783, 36119940}, {1, 0x80040786,   755152},
	{2, 0x800408B9, 18430980}, {3, 0x8004093A,  6679208}, {1, 0x8004093D,   383318},
	{3, 0x80040942,  4502532}, {2, 0x80040968,  3920338}, {3, 0x80040970,   654212},
	{2, 0x8004097D,   524576}, {1, 0x8004097F,    52574}, {2, 0x80040983,   289832},
};

//////////////////////////////////////////////////////////////////////////
// ResourceManager
//////////////////////////////////////////////////////////////////////////

ResourceManager::ResourceManager(AsylumEngine *vm) : _cdNumber(-1), _musicPackId(kResourcePackInvalid), _vm(vm) {
}

ResourceManager::~ResourceManager() {
	for (ResourceCache::const_iterator it = _resources.begin(); it != _resources.end(); it++)
		delete it->_value;
	for (ResourceCache::const_iterator it = _music.begin(); it != _music.end(); it++)
		delete it->_value;
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
			if (_vm->checkGameVersion("Demo"))
				pack = new ResourcePack("res.002");
			else
				pack = new ResourcePack(Common::String::format("mus.%03d", _musicPackId));
		} else {
			if (packId == kResourcePackSharedSound) {
				if (_vm->checkGameVersion("Demo")) {
					pack = new ResourcePack("res.004");
					cache->setVal(packId, pack);
					return cache->getVal(packId)->get(index);
				}

				if (_cdNumber == -1)
					error("[ResourceManager::get] Cd number has not been set!");

				pack = new ResourcePack(Common::String::format("res.%01d%02d", _cdNumber, packId));

				// WORKAROUND to support combined resource packs (used by GOG and Steam versions)
				if (pack->_packFile.size() == 299872422)
					for (int i = 0; i < ARRAYSIZE(patchedSizes); i++)
						if (_cdNumber == patchedSizes[i].cdNumber)
							pack->_resources[RESOURCE_INDEX(patchedSizes[i].resourceId)].size = patchedSizes[i].size;
			} else {
				pack = new ResourcePack(Common::String::format("res.%03d", packId));
			}
		}

		cache->setVal(packId, pack);
	}

	return cache->getVal(packId)->get(index);
}

void ResourceManager::unload(ResourcePackId id) {
	if (_resources.contains(id)) {
		delete _resources.getVal(id);
		_resources.erase(id);
	}

	if (_music.contains(id)) {
		delete _music.getVal(id);
		_music.erase(id);
	}
}

//////////////////////////////////////////////////////////////////////////
// ResourcePack
//////////////////////////////////////////////////////////////////////////
ResourcePack::ResourcePack(const Common::String &filename) {
	init(filename);
}

ResourcePack::~ResourcePack() {
	for (uint32 i = 0; i < _resources.size(); i++)
		delete [] _resources[i].data;

	_resources.clear();
	_packFile.close();
}

void ResourcePack::init(const Common::String &filename) {
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
		entry.data = nullptr;

		_resources[i] = entry;

		prevOffset = nextOffset;
	}
}

ResourceEntry *ResourcePack::get(uint16 index) {
	if (index > _resources.size() - 1)
		return nullptr;

	if (!_resources[index].data) {
		// Load the requested resource if it's not loaded already
		_packFile.seek(_resources[index].offset, SEEK_SET);
		_resources[index].data = new byte[_resources[index].size];
		_packFile.read(_resources[index].data, _resources[index].size);
	}

	return &_resources[index];
}

} // end of namespace Asylum
