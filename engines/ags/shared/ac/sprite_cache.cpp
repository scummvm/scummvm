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
#include "ags/shared/util/stream.h"
#include "ags/lib/std/algorithm.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/ac/common.h" // quit
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// [IKM] We have to forward-declare these because their implementations are in the Engine
extern void initialize_sprite(int);
extern void pre_save_sprite(Bitmap *image);
extern void get_new_size_for_sprite(int, int, int, int &, int &);

#define START_OF_LIST -1
#define END_OF_LIST   -1

SpriteInfo::SpriteInfo()
	: Flags(0)
	, Width(0)
	, Height(0) {
}

namespace AGS {
namespace Shared {

SpriteCache::SpriteData::SpriteData()
	: Size(0)
	, Flags(0)
	, Image(nullptr) {
}

SpriteCache::SpriteData::~SpriteData() {
	// TODO: investigate, if it's actually safe/preferred to delete bitmap here
	// (some of these bitmaps may be assigned from outside of the cache)
}

SpriteCache::SpriteCache(std::vector<SpriteInfo> &sprInfos)
	: _sprInfos(sprInfos) {
	Init();
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
	_maxCacheSize = size;
}

void SpriteCache::Init() {
	_cacheSize = 0;
	_lockedSize = 0;
	_maxCacheSize = (size_t)DEFAULTCACHESIZE_KB * 1024;
	_liststart = -1;
	_listend = -1;
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

	_mrulist.clear();
	_mrubacklink.clear();

	Init();
}

void SpriteCache::SetSprite(sprkey_t index, Bitmap *sprite) {
	if (index < 0 || EnlargeTo(index) != index) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: unable to use index %d", index);
		return;
	}
	if (!sprite) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: attempt to assign nullptr to index %d", index);
		return;
	}
	_spriteData[index].Image = sprite;
	_spriteData[index].Flags = SPRCACHEFLAG_LOCKED; // NOT from asset file
	_spriteData[index].Size = 0;
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "SetSprite: (external) %d", index);
#endif
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

void SpriteCache::SubstituteBitmap(sprkey_t index, Bitmap *sprite) {
	if (!DoesSpriteExist(index)) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SubstituteBitmap: attempt to set for non-existing sprite %d", index);
		return;
	}
	_spriteData[index].Image = sprite;
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "SubstituteBitmap: %d", index);
#endif
}

void SpriteCache::RemoveSprite(sprkey_t index, bool freeMemory) {
	if (freeMemory)
		delete _spriteData[index].Image;
	InitNullSpriteParams(index);
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "RemoveSprite: %d", index);
#endif
}

sprkey_t SpriteCache::EnlargeTo(sprkey_t topmost) {
	if (topmost < 0 || topmost > MAX_SPRITE_INDEX)
		return -1;
	if ((size_t)topmost < _spriteData.size())
		return topmost;

	size_t newsize = topmost + 1;
	_sprInfos.resize(newsize);
	_spriteData.resize(newsize);
	_mrulist.resize(newsize);
	_mrubacklink.resize(newsize);
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

Bitmap *SpriteCache::operator [] (sprkey_t index) {
	// invalid sprite slot
	if (index < 0 || (size_t)index >= _spriteData.size())
		return nullptr;

	// Externally added sprite, don't put it into MRU list
	if (_spriteData[index].IsExternalSprite())
		return _spriteData[index].Image;

	// Sprite exists in file but is not in mem, load it
	if ((_spriteData[index].Image == nullptr) && _spriteData[index].IsAssetSprite())
		LoadSprite(index);

	// Locked sprite that shouldn't be put into MRU list
	if (_spriteData[index].IsLocked())
		return _spriteData[index].Image;

	if (_liststart < 0) {
		_liststart = index;
		_listend = index;
		_mrulist[index] = END_OF_LIST;
		_mrubacklink[index] = START_OF_LIST;
	} else if (_listend != index) {
		// this is the oldest element being bumped to newest, so update start link
		if (index == _liststart) {
			_liststart = _mrulist[index];
			_mrubacklink[_liststart] = START_OF_LIST;
		}
		// already in list, link previous to next
		else if (_mrulist[index] > 0) {
			_mrulist[_mrubacklink[index]] = _mrulist[index];
			_mrubacklink[_mrulist[index]] = _mrubacklink[index];
		}

		// set this as the newest element in the list
		_mrulist[index] = END_OF_LIST;
		_mrulist[_listend] = index;
		_mrubacklink[index] = _listend;
		_listend = index;
	}

	return _spriteData[index].Image;
}

void SpriteCache::DisposeOldest() {
	if (_liststart < 0)
		return;

	sprkey_t sprnum = _liststart;

	if ((_spriteData[sprnum].Image != nullptr) && !_spriteData[sprnum].IsLocked()) {
		// Free the memory
		// Sprites that are not from the game resources should not normally be in a MRU list;
		// if such is met here there's something wrong with the internal cache logic!
		if (!_spriteData[sprnum].IsAssetSprite()) {
			quitprintf("SpriteCache::DisposeOldest: attempted to remove sprite %d that was added externally or does not exist", sprnum);
		}
		_cacheSize -= _spriteData[sprnum].Size;

		delete _spriteData[sprnum].Image;
		_spriteData[sprnum].Image = nullptr;
	}

	if (_liststart == _listend) {
		// there was one huge sprite, removing it has now emptied the cache completely
		if (_cacheSize > 0) {
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SPRITE CACHE ERROR: Sprite cache should be empty, but still has %d bytes", _cacheSize);
		}
		_mrulist[_liststart] = 0;
		_liststart = -1;
		_listend = -1;
	} else {
		sprkey_t oldstart = _liststart;
		_liststart = _mrulist[_liststart];
		_mrulist[oldstart] = 0;
		_mrubacklink[_liststart] = START_OF_LIST;
		if (oldstart == _liststart) {
			// Somehow, we have got a recursive link to itself, so we
			// the game will freeze (since it is not actually freeing any
			// memory)
			// There must be a bug somewhere causing this, but for now
			// let's just reset the cache
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: CACHE INCONSISTENT: RESETTING\n\tAt size %d (of %d), start %d end %d  fwdlink=%d",
				_cacheSize, _maxCacheSize, oldstart, _listend, _liststart);
			DisposeAll();
		}
	}

#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "DisposeOldest: disposed %d, size now %d KB", sprnum, _cacheSize / 1024);
#endif
}

void SpriteCache::DisposeAll() {
	_liststart = -1;
	_listend = -1;
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		if (!_spriteData[i].IsLocked() && // not locked
			_spriteData[i].IsAssetSprite()) // sprite from game resource
		{
			delete _spriteData[i].Image;
			_spriteData[i].Image = nullptr;
		}
		_mrulist[i] = 0;
		_mrubacklink[i] = 0;
	}
	_cacheSize = _lockedSize;
}

void SpriteCache::Precache(sprkey_t index) {
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;

	soff_t sprSize = 0;

	if (_spriteData[index].Image == nullptr)
		sprSize = LoadSprite(index);
	else if (!_spriteData[index].IsLocked())
		sprSize = _spriteData[index].Size;

	// make sure locked sprites can't fill the cache
	_maxCacheSize += sprSize;
	_lockedSize += sprSize;

	_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;

#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "Precached %d", index);
#endif
}

sprkey_t SpriteCache::GetDataIndex(sprkey_t index) {
	return (_spriteData[index].Flags & SPRCACHEFLAG_REMAPPED) == 0 ? index : 0;
}

size_t SpriteCache::LoadSprite(sprkey_t index) {
	int hh = 0;

	while (_cacheSize > _maxCacheSize) {
		DisposeOldest();
		hh++;
		if (hh > 1000) {
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: STUCK IN FREE_UP_MEM; RESETTING CACHE");
			DisposeAll();
		}
	}

	if (index < 0 || (size_t)index >= _spriteData.size())
		quit("sprite cache array index out of bounds");

	sprkey_t load_index = GetDataIndex(index);
	Bitmap *image;
	HError err = _file.LoadSprite(load_index, image);
	if (!image) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn,
			"LoadSprite: failed to load sprite %d:\n%s\n - remapping to sprite 0.", index,
			err ? "Sprite does not exist." : err->FullMessage().GetCStr());
		RemapSpriteToSprite0(index);
		return 0;
	}

	// update the stored width/height
	_sprInfos[index].Width = image->GetWidth();
	_sprInfos[index].Height = image->GetHeight();
	_spriteData[index].Image = image;

	// Stop it adding the sprite to the used list just because it's loaded
	// TODO: this messy hack is required, because initialize_sprite calls operator[]
	// which puts the sprite to the MRU list.
	_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;

	// TODO: this is ugly: asks the engine to convert the sprite using its own knowledge.
	// And engine assigns new bitmap using SpriteCache::SubstituteBitmap().
	// Perhaps change to the callback function pointer?
	initialize_sprite(index);

	if (index != 0)  // leave sprite 0 locked
		_spriteData[index].Flags &= ~SPRCACHEFLAG_LOCKED;

	// we need to store this because the main program might
	// alter spritewidth/height if it resizes stuff
	size_t size = _sprInfos[index].Width * _sprInfos[index].Height *
		_spriteData[index].Image->GetBPP();
	_spriteData[index].Size = size;
	_cacheSize += size;

#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "Loaded %d, size now %zu KB", index, _cacheSize / 1024);
#endif

	return size;
}

void SpriteCache::RemapSpriteToSprite0(sprkey_t index) {
	_sprInfos[index].Flags = _sprInfos[0].Flags;
	_sprInfos[index].Width = _sprInfos[0].Width;
	_sprInfos[index].Height = _sprInfos[0].Height;
	_spriteData[index].Image = nullptr;
	_spriteData[index].Size = _spriteData[0].Size;
	_spriteData[index].Flags |= SPRCACHEFLAG_REMAPPED;
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "RemapSpriteToSprite0: %d", index);
#endif
}

int SpriteCache::SaveToFile(const String &filename, int store_flags, SpriteCompression compress, SpriteFileIndex &index) {
	std::vector<std::pair<bool, Bitmap *>> sprites;
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		// NOTE: this is a horrible hack:
		// because Editor expects slightly different RGB order, it swaps colors
		// when loading them (call to initialize_sprite), so here we basically
		// unfix that fix to save the data in a way that engine will expect.
		// TODO: perhaps adjust the editor to NOT need this?!
		pre_save_sprite(_spriteData[i].Image);
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
	_mrulist.resize(newsize);
	_mrubacklink.resize(newsize);
	for (size_t i = 0; i < metrics.size(); ++i) {
		if (!metrics[i].IsNull()) {
			// Existing sprite
			_spriteData[i].Flags = SPRCACHEFLAG_ISASSET;
			get_new_size_for_sprite(i, metrics[i].Width, metrics[i].Height, _sprInfos[i].Width, _sprInfos[i].Height);
		} else {
			// Handle empty slot: remap to sprite 0
			if (i > 0) // FIXME: optimize
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
