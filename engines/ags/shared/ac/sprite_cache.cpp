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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// sprite caching system
//
//=============================================================================

#include "common/system.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/util/stream.h"
#include "common/std/algorithm.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;


// High-verbosity sprite cache log
#if DEBUG_SPRITECACHE
#define SprCacheLog(...) Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, __VA_ARGS__)
#else
#define SprCacheLog(...)
#endif

SpriteInfo::SpriteInfo()
	: Flags(0)
	, Width(0)
	, Height(0) {
}

namespace AGS {
namespace Shared {

SpriteCache::SpriteCache(std::vector<SpriteInfo> &sprInfos, const Callbacks &callbacks)
	: _sprInfos(sprInfos), _maxCacheSize(DEFAULTCACHESIZE_KB * 1024u),
	  _cacheSize(0u), _lockedSize(0u) {
	_callbacks.AdjustSize = (callbacks.AdjustSize) ? callbacks.AdjustSize : DummyAdjustSize;
	_callbacks.InitSprite = (callbacks.InitSprite) ? callbacks.InitSprite : DummyInitSprite;
	_callbacks.PostInitSprite = (callbacks.PostInitSprite) ? callbacks.PostInitSprite : DummyPostInitSprite;
	_callbacks.PrewriteSprite = (callbacks.PrewriteSprite) ? callbacks.PrewriteSprite : DummyPrewriteSprite;
}

SpriteCache::~SpriteCache() {
	Reset();
}

size_t SpriteCache::GetCacheSize() const {
	return _cacheSize;
}

size_t SpriteCache::GetLockedSize() const {
	return _lockedSize;
}

size_t SpriteCache::GetMaxCacheSize() const {
	return _maxCacheSize;
}

size_t SpriteCache::GetSpriteSlotCount() const {
	return _spriteData.size();
}

void SpriteCache::SetMaxCacheSize(size_t size) {
	FreeMem(size);
	_maxCacheSize = size;
}

void SpriteCache::Reset() {
	_file.Close();
	// TODO: find out if it's safe to simply always delete _spriteData.Image with array element
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		if (_spriteData[i].Image) {
			delete _spriteData[i].Image;
			_spriteData[i].Image = nullptr;
		}
	}
	_spriteData.clear();
	_mru.clear();
	_cacheSize = 0;
	_lockedSize = 0;
}

bool SpriteCache::SetSprite(sprkey_t index, Bitmap *sprite, int flags) {
	if (index < 0 || EnlargeTo(index) != index) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: unable to use index %d", index);
		return false;
	}
	if (!sprite) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: attempt to assign nullptr to index %d", index);
		return false;
	}
	_spriteData[index].Image = sprite;
	_spriteData[index].Flags = SPRCACHEFLAG_LOCKED; // NOT from asset file
	_spriteData[index].Size = 0;
	_sprInfos[index].Flags = flags;
	_sprInfos[index].Width = sprite->GetWidth();
	_sprInfos[index].Height = sprite->GetHeight();
	SprCacheLog("SetSprite: (external) %d", index);
	return true;
}

void SpriteCache::SetEmptySprite(sprkey_t index, bool as_asset) {
	if (index < 0 || EnlargeTo(index) != index) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetEmptySprite: unable to use index %d", index);
		return;
	}
	if (as_asset)
		_spriteData[index].Flags = SPRCACHEFLAG_ISASSET;
	RemapSpriteToSprite0(index);
}

Bitmap *SpriteCache::RemoveSprite(sprkey_t index) {
	if (index < 0 || (size_t)index >= _spriteData.size())
		return nullptr;
	Bitmap *image = _spriteData[index].Image;
	InitNullSpriteParams(index);
	SprCacheLog("RemoveSprite: %d", index);
	return image;
}

void SpriteCache::DisposeSprite(sprkey_t index) {
	assert(index >= 0); // out of positive range indexes are valid to fail
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;
	delete _spriteData[index].Image;
	InitNullSpriteParams(index);
	SprCacheLog("RemoveAndDispose: %d", index);
}

sprkey_t SpriteCache::EnlargeTo(sprkey_t topmost) {
	if (topmost < 0 || topmost > MAX_SPRITE_INDEX)
		return -1;
	if ((size_t)topmost < _spriteData.size())
		return topmost;

	size_t newsize = topmost + 1;
	_sprInfos.resize(newsize);
	_spriteData.resize(newsize);
	return topmost;
}

sprkey_t SpriteCache::GetFreeIndex() {
	for (size_t i = MIN_SPRITE_INDEX; i < _spriteData.size(); ++i) {
		// slot empty
		if (!DoesSpriteExist(i)) {
			_sprInfos[i] = SpriteInfo();
			_spriteData[i] = SpriteData();
			return i;
		}
	}
	// enlarge the sprite bank to find a free slot and return the first new free slot
	return EnlargeTo(_spriteData.size());
}

bool SpriteCache::SpriteData::DoesSpriteExist() const {
	return (Image != nullptr) || // HAS loaded bitmap
		((Flags & SPRCACHEFLAG_ISASSET) != 0); // OR found in the game resources
}

bool SpriteCache::SpriteData::IsAssetSprite() const {
	return (Flags & SPRCACHEFLAG_ISASSET) != 0; // found in game resources
}

bool SpriteCache::SpriteData::IsExternalSprite() const {
	return (Image != nullptr) &&  // HAS loaded bitmap
		((Flags & SPRCACHEFLAG_ISASSET) == 0) && // AND NOT found in game resources
		((Flags & SPRCACHEFLAG_REMAPPED) == 0); // AND was NOT remapped to another sprite
}

bool SpriteCache::SpriteData::IsLocked() const {
	return (Flags & SPRCACHEFLAG_LOCKED) != 0;
}

bool SpriteCache::DoesSpriteExist(sprkey_t index) const {
	return index >= 0 && (size_t)index < _spriteData.size() && _spriteData[index].DoesSpriteExist();
}

Size SpriteCache::GetSpriteResolution(sprkey_t index) const {
	return DoesSpriteExist(index) ? _sprInfos[index].GetResolution() : Size();
}

Bitmap *SpriteCache::operator [] (sprkey_t index) {
	// invalid sprite slot
	if (index < 0 || (size_t)index >= _spriteData.size())
		return nullptr;

	// Externally added sprite or locked sprite, don't put it into MRU list
	if (_spriteData[index].IsExternalSprite() || _spriteData[index].IsLocked())
		return _spriteData[index].Image;

	if (_spriteData[index].Image) {
		// Move to the beginning of the MRU list
		_mru.splice(_mru.begin(), _mru, _spriteData[index].MruIt);
	} else {
		// Sprite exists in file but is not in mem, load it
		LoadSprite(index);
		_spriteData[index].MruIt = _mru.insert(_mru.begin(), index);
	}
	return _spriteData[index].Image;
}

void SpriteCache::FreeMem(size_t space) {
	for (int tries = 0; (_mru.size() > 0) && (_cacheSize >= (_maxCacheSize - space)); ++tries) {
		DisposeOldest();
		if (tries > 1000) { // ???
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: STUCK IN FREE_UP_MEM; RESETTING CACHE");
			DisposeAll();
		}
	}
}

void SpriteCache::DisposeOldest() {
	assert(_mru.size() > 0);
	if (_mru.size() == 0)
		return;
	auto it = std::prev(_mru.end());
	const auto sprnum = *it;
	// Safety check: must be a sprite from resources
	// TODO: compare with latest upstream
	// Commented out the assertion, since it triggers for sprites that are in the list but remapped to the placeholder (sprite 0)
	// Whispers of a Machine is affected by this issue (see TRAC #14730)

	// assert(_spriteData[sprnum].IsAssetSprite());

	if (!_spriteData[sprnum].IsAssetSprite()) {
		if (!(_spriteData[sprnum].Flags & SPRCACHEFLAG_REMAPPED))
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SpriteCache::DisposeOldest: in MRU list sprite %d is external or does not exist", sprnum);
		_mru.erase(it);
		// std::list::erase() invalidates iterators to the erased item.
		// But our implementation does not.
		_spriteData[sprnum].MruIt._node = nullptr;
		return;
	}
	// Delete the image, unless is locked
	// NOTE: locked sprites may still occur in MRU list
	if (!_spriteData[sprnum].IsLocked()) {
		_cacheSize -= _spriteData[sprnum].Size;
		delete _spriteData[*it].Image;
		_spriteData[sprnum].Image = nullptr;
		SprCacheLog("DisposeOldest: disposed %d, size now %d KB", sprnum, _cacheSize / 1024);
	}
	// Remove from the mru list
	_mru.erase(it);
	// std::list::erase() invalidates iterators to the erased item.
	// But our implementation does not.
	_spriteData[sprnum].MruIt._node = nullptr;
}

void SpriteCache::DisposeAll() {
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		if (!_spriteData[i].IsLocked() && // not locked
			_spriteData[i].IsAssetSprite()) // sprite from game resource
		{
			delete _spriteData[i].Image;
			_spriteData[i].Image = nullptr;
		}
	}
	_cacheSize = _lockedSize;
	_mru.clear();
}

void SpriteCache::Precache(sprkey_t index) {
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;
	if (!_spriteData[index].IsAssetSprite())
		return; // cannot precache a non-asset sprite

	soff_t sprSize = 0;

	if (_spriteData[index].Image == nullptr) {
		sprSize = LoadSprite(index);
	} else if (!_spriteData[index].IsLocked()) {
		sprSize = _spriteData[index].Size;
		// Remove locked sprite from the MRU list
		_mru.erase(_spriteData[index].MruIt);
		// std::list::erase() invalidates iterators to the erased item.
		// But our implementation does not.
		_spriteData[index].MruIt._node = nullptr;
	}

	// make sure locked sprites can't fill the cache
	_maxCacheSize += sprSize;
	_lockedSize += sprSize;
	_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;
	SprCacheLog("Precached %d", index);
}

sprkey_t SpriteCache::GetDataIndex(sprkey_t index) {
	return (_spriteData[index].Flags & SPRCACHEFLAG_REMAPPED) == 0 ? index : 0;
}

size_t SpriteCache::LoadSprite(sprkey_t index) {
	assert((index >= 0) && ((size_t)index < _spriteData.size()));
	if (index < 0 || (size_t)index >= _spriteData.size())
		return 0;

	sprkey_t load_index = GetDataIndex(index);
	Bitmap *image;
	HError err = _file.LoadSprite(load_index, image);
	if (!image) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn,
			"LoadSprite: failed to load sprite %d:\n%s\n - remapping to sprite 0.", index,
			err ? "Sprite does not exist." : err->FullMessage().GetCStr());
		InitNullSpriteParams(index);
		return 0;
	}

	// Let the external user convert this sprite's image for their needs
	image = _callbacks.InitSprite(index, image, _sprInfos[index].Flags);
	if (!image) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn,
					  "LoadSprite: failed to initialize sprite %d, remapping to sprite 0.", index);
		InitNullSpriteParams(index);
		return 0;
	}

	// save the stored sprite info
	_sprInfos[index].Width = image->GetWidth();
	_sprInfos[index].Height = image->GetHeight();
	_spriteData[index].Image = image;
	if (index == 0) // keep sprite 0 locked
		_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;

	const size_t size = _sprInfos[index].Width * _sprInfos[index].Height *
		_spriteData[index].Image->GetBPP();
	// Clear up space before adding to cache
	FreeMem(size);
	_spriteData[index].Size = size;
	_cacheSize += size;
	SprCacheLog("Loaded %d, size now %zu KB", index, _cacheSize / 1024);

	// Let the external user to react to the new sprite;
	// note that this callback is allowed to modify the sprite's pixels,
	// but not its size or flags.
	_callbacks.PostInitSprite(index);

	return size;
}

void SpriteCache::RemapSpriteToSprite0(sprkey_t index) {
	_sprInfos[index].Flags = _sprInfos[0].Flags;
	_sprInfos[index].Width = _sprInfos[0].Width;
	_sprInfos[index].Height = _sprInfos[0].Height;
	_spriteData[index].Image = nullptr;
	_spriteData[index].Size = _spriteData[0].Size;
	_spriteData[index].Flags |= SPRCACHEFLAG_REMAPPED;
	SprCacheLog("RemapSpriteToSprite0: %d", index);
}

void SpriteCache::InitNullSpriteParams(sprkey_t index) {
	assert(index >= 0);
	if (index > 0) {
		RemapSpriteToSprite0(index);
	} else {
		_sprInfos[index] = SpriteInfo();
		_spriteData[index] = SpriteData();
	}
}

int SpriteCache::SaveToFile(const String &filename, int store_flags, SpriteCompression compress, SpriteFileIndex &index) {
	std::vector<std::pair<bool, Bitmap *>> sprites;
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		_callbacks.PrewriteSprite(_spriteData[i].Image);
		sprites.push_back(std::make_pair(DoesSpriteExist(i), _spriteData[i].Image));
	}
	return SaveSpriteFile(filename, sprites, &_file, store_flags, compress, index);
}

HError SpriteCache::InitFile(const String &filename, const String &sprindex_filename) {
	Reset();

	std::vector<Size> metrics;
	HError err = _file.OpenFile(filename, sprindex_filename, metrics);
	if (!err)
		return err;

	// Initialize sprite infos
	size_t newsize = metrics.size();
	_sprInfos.resize(newsize);
	_spriteData.resize(newsize);
	_mru.clear();
	for (size_t i = 0; i < metrics.size(); ++i) {
		if (!metrics[i].IsNull()) {
			// Existing sprite
			_spriteData[i].Flags = SPRCACHEFLAG_ISASSET;
			Size newsz = _callbacks.AdjustSize(Size(metrics[i].Width, metrics[i].Height), _sprInfos[i].Flags);
			_sprInfos[i].Width = newsz.Width;
			_sprInfos[i].Height = newsz.Height;
		} else {
			// Handle empty slot: remap to sprite 0
			InitNullSpriteParams(i);
		}
	}
	return HError::None();
}

void SpriteCache::DetachFile() {
	_file.Close();
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
