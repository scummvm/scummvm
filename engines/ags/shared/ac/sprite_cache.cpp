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

// Tells that the sprite is found in the game resources.
#define SPRCACHEFLAG_ISASSET  0x01
// Tells that the sprite is assigned externally and cannot be autodisposed.
#define SPRCACHEFLAG_EXTERNAL 0x02
// Tells that the asset sprite failed to load
#define SPRCACHEFLAG_ERROR	  0x04
// Locked sprites are ones that should not be freed when out of cache space.
#define SPRCACHEFLAG_LOCKED	  0x08

// High-verbosity sprite cache log
#if DEBUG_SPRITECACHE
#define SprCacheLog(...) Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, __VA_ARGS__)
#else
#define SprCacheLog(...)
#endif

namespace AGS {
namespace Shared {

SpriteCache::SpriteCache(std::vector<SpriteInfo> &sprInfos, const Callbacks &callbacks)
	: _sprInfos(sprInfos), _maxCacheSize(DEFAULTCACHESIZE_KB * 1024u),
	  _cacheSize(0u), _lockedSize(0u) {
	_callbacks.AdjustSize = (callbacks.AdjustSize) ? callbacks.AdjustSize : DummyAdjustSize;
	_callbacks.InitSprite = (callbacks.InitSprite) ? callbacks.InitSprite : DummyInitSprite;
	_callbacks.PostInitSprite = (callbacks.PostInitSprite) ? callbacks.PostInitSprite : DummyPostInitSprite;
	_callbacks.PrewriteSprite = (callbacks.PrewriteSprite) ? callbacks.PrewriteSprite : DummyPrewriteSprite;

	// Generate a placeholder sprite: 1x1 transparent bitmap
	_placeholder.reset(BitmapHelper::CreateTransparentBitmap(1, 1, 8));
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

bool SpriteCache::HasFreeSlots() const {
	return !((_spriteData.size() == SIZE_MAX) || (_spriteData.size() > MAX_SPRITE_INDEX));
}

bool SpriteCache::IsAssetSprite(sprkey_t index) const {
	return index >= 0 && (size_t)index < _spriteData.size() && // in the valid range
		   _spriteData[index].IsAssetSprite();                 // found in the game resources
}

void SpriteCache::Reset() {
	_file.Close();
	_spriteData.clear();
	_mru.clear();
	_cacheSize = 0;
	_lockedSize = 0;
}

bool SpriteCache::SetSprite(sprkey_t index, std::unique_ptr<Bitmap> image, int flags) {
	if (index < 0 || EnlargeTo(index) != index) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: unable to use index %d", index);
		return false;
	}
	if (!image || image->GetSize().IsNull() || image->GetColorDepth() <= 0) {
		DisposeSprite(index); // free previous item in this slot anyway
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: attempt to assign an invalid bitmap to index %d", index);
		return false;
	}

	const int spf_flags = flags
		| (SPF_HICOLOR * image->GetColorDepth() > 8)
		| (SPF_TRUECOLOR * image->GetColorDepth() > 16);
	_sprInfos[index] = SpriteInfo(image->GetWidth(), image->GetHeight(), spf_flags);
	// Assign sprite with 0 size, as it will not be included into the cache size
	_spriteData[index] = SpriteData(image.release(), 0, SPRCACHEFLAG_EXTERNAL | SPRCACHEFLAG_LOCKED);
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
	RemapSpriteToPlaceholder(index);
}

Bitmap *SpriteCache::RemoveSprite(sprkey_t index) {
	if (index < 0 || (size_t)index >= _spriteData.size())
		return nullptr;
	Bitmap *image = _spriteData[index].Image.release();
	InitNullSprite(index);
	SprCacheLog("RemoveSprite: %d", index);
	return image;
}

void SpriteCache::DisposeSprite(sprkey_t index) {
	assert(index >= 0); // out of positive range indexes are valid to fail
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;
	InitNullSprite(index);
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
	// FIXME: inefficient if large number of sprites were created in game;
	// use "available ids" stack, see managed pool for an example;
	// NOTE: this is shared with the Editor, which means we cannot rely on the
	// number of "static" sprites and search for slots after... this may be
	// resolved by splitting SpriteCache class further on "cache builder" and
	// "runtime cache".
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
	return (Flags & SPRCACHEFLAG_ISASSET) != 0;
}

bool SpriteCache::SpriteData::IsError() const {
	return (Flags & SPRCACHEFLAG_ERROR) != 0;
}

bool SpriteCache::SpriteData::IsExternalSprite() const {
	return (Flags & SPRCACHEFLAG_EXTERNAL) != 0;
}

bool SpriteCache::SpriteData::IsLocked() const {
	return (Flags & SPRCACHEFLAG_LOCKED) != 0;
}

bool SpriteCache::DoesSpriteExist(sprkey_t index) const {
	return (index >= 0 && (size_t)index < _spriteData.size()) &&  // in the valid range
		   _spriteData[index].IsValid();  // has assigned sprite
}

Size SpriteCache::GetSpriteResolution(sprkey_t index) const {
	return DoesSpriteExist(index) ? _sprInfos[index].GetResolution() : Size();
}

Bitmap *SpriteCache::operator[](sprkey_t index) {
	// invalid sprite slot
	if (!DoesSpriteExist(index) || _spriteData[index].IsError())
		return _placeholder.get();

	// Externally added sprite or locked sprite, don't put it into MRU list
	if (_spriteData[index].IsExternalSprite() || _spriteData[index].IsLocked())
		return _spriteData[index].Image.get();
	// Either use ready image, or load one from assets
	if (_spriteData[index].Image) {
		// Move to the beginning of the MRU list
		_mru.splice(_mru.begin(), _mru, _spriteData[index].MruIt);
		return _spriteData[index].Image.get();
	} else {
		// Sprite exists in file but is not in mem, load it and add to MRU list
		if (LoadSprite(index)) {
			_spriteData[index].MruIt = _mru.insert(_mru.begin(), index);
			return _spriteData[index].Image.get();
		}
	}
	return _placeholder.get();
}

void SpriteCache::FreeMem(size_t space) {
	for (int tries = 0; (_mru.size() > 0) && (_cacheSize >= (_maxCacheSize - space)); ++tries) {
		DisposeOldest();
		if (tries > 1000) { // ???
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: STUCK IN FREE_UP_MEM; RESETTING CACHE");
			DisposeAllCached();
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
		_spriteData[sprnum].Image.reset();
		SprCacheLog("DisposeOldest: disposed %d, size now %d KB", sprnum, _cacheSize / 1024);
	}
	// Remove from the mru list
	_mru.erase(it);
	// std::list::erase() invalidates iterators to the erased item.
	// But our implementation does not.
	_spriteData[sprnum].MruIt._node = nullptr;
}

void SpriteCache::DisposeAllCached() {
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		if (!_spriteData[i].IsLocked() &&   // not locked
			_spriteData[i].IsAssetSprite()) // sprite from game resource
		{
			_spriteData[i].Image.reset();
		}
	}
	_cacheSize = _lockedSize;
	_mru.clear();
}

void SpriteCache::PrecacheSprite(sprkey_t index) {
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;
	if (!_spriteData[index].IsAssetSprite())
		return; // cannot precache a non-asset sprite

	size_t size = 0;

	if (_spriteData[index].Image == nullptr) {
		size = LoadSprite(index);
	} else if (!_spriteData[index].IsLocked()) {
		size = _spriteData[index].Size;
		// Remove locked sprite from the MRU list
		_mru.erase(_spriteData[index].MruIt);
		// std::list::erase() invalidates iterators to the erased item.
		// But our implementation does not.
		_spriteData[index].MruIt._node = nullptr;
	}

	// make sure locked sprites can't fill the cache
	_maxCacheSize += size;
	_lockedSize += size;
	_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;
	SprCacheLog("Precached %d", index);
}

void SpriteCache::LockSprite(sprkey_t index) {
	assert(index >= 0); // out of positive range indexes are valid to fail
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;
	if (!_spriteData[index].IsAssetSprite())
		return; // cannot lock a non-asset sprite

	if (_spriteData[index].DoesSpriteExist()) {
		_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;
	} else {
		LoadSprite(index, true);
	}
	SprCacheLog("Locked %d", index);
}

void SpriteCache::UnlockSprite(sprkey_t index) {
	assert(index >= 0); // out of positive range indexes are valid to fail
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;
	if (!_spriteData[index].IsAssetSprite() ||
		!_spriteData[index].IsLocked())
		return; // cannot unlock a non-asset sprite, or non-locked sprite

	_spriteData[index].Flags &= ~SPRCACHEFLAG_LOCKED;
	SprCacheLog("Unlocked %d", index);
}

size_t SpriteCache::LoadSprite(sprkey_t index, bool lock) {
	assert((index >= 0) && ((size_t)index < _spriteData.size()));
	if (index < 0 || (size_t)index >= _spriteData.size())
		return 0;
	assert((_spriteData[index].Flags & SPRCACHEFLAG_ISASSET) != 0);

	Bitmap *image;
	HError err = _file.LoadSprite(index, image);
	if (!image) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn,
			"LoadSprite: failed to load sprite %d:\n%s\n - remapping to placeholder", index,
			err ? "Sprite does not exist." : err->FullMessage().GetCStr());
		RemapSpriteToPlaceholder(index);
		return 0;
	}

	// Let the external user convert this sprite's image for their needs
	image = _callbacks.InitSprite(index, image, _sprInfos[index].Flags);
	if (!image) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn,
					  "LoadSprite: failed to initialize sprite %d, remapping to placeholder", index);
		RemapSpriteToPlaceholder(index);
		return 0;
	}

	// save the stored sprite info
	_sprInfos[index].Width = image->GetWidth();
	_sprInfos[index].Height = image->GetHeight();
	// Clear up space before adding to cache
	const size_t size = image->GetWidth() * image->GetHeight() * image->GetBPP();
	FreeMem(size);
	// Add to the cache, lock if requested or if it's sprite 0
	const bool should_lock = lock || (index == 0);
	_spriteData[index] = SpriteData(image, size, SPRCACHEFLAG_ISASSET);
	_spriteData[index].Flags |= (SPRCACHEFLAG_LOCKED * should_lock);
	_cacheSize += size;
	SprCacheLog("Loaded %d, size now %zu KB", index, _cacheSize / 1024);

	// Let the external user to react to the new sprite;
	// note that this callback is allowed to modify the sprite's pixels,
	// but not its size or flags.
	_callbacks.PostInitSprite(index);

	return size;
}

void SpriteCache::RemapSpriteToPlaceholder(sprkey_t index) {
	assert((index > 0) && ((size_t)index < _spriteData.size()));
	_sprInfos[index] = SpriteInfo(_placeholder->GetWidth(), _placeholder->GetHeight(), _placeholder->GetColorDepth());
	_spriteData[index].Flags |= SPRCACHEFLAG_ERROR;
	SprCacheLog("RemapSpriteToPlaceholder: %d", index);
}

void SpriteCache::InitNullSprite(sprkey_t index) {
	assert(index >= 0);
	_sprInfos[index] = SpriteInfo();
	_spriteData[index] = SpriteData();
}

int SpriteCache::SaveToFile(const String &filename, int store_flags, SpriteCompression compress, SpriteFileIndex &index) {
	std::vector<std::pair<bool, Bitmap *>> sprites;
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		_callbacks.PrewriteSprite(_spriteData[i].Image.get());
		sprites.push_back(std::make_pair(DoesSpriteExist(i), _spriteData[i].Image.get()));
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
			// Mark as empty slot
			InitNullSprite(i);
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
