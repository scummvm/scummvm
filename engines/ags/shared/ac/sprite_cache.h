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

//
// Sprite caching system.
//
// SpriteFile handles sprite serialization and streaming.
// SpriteCache provides bitmaps by demand; it uses SpriteFile to load sprites
// and does MRU (most-recent-use) caching.
//
// TODO: store sprite data in a specialized container type that is optimized
// for having most keys allocated in large continious sequences by default.
//
// Only for the reference: one of the ideas is for container to have a table
// of arrays of fixed size internally. When getting an item the hash would be
// first divided on array size to find the array the item resides in, then the
// item is taken from item from slot index = (hash - arrsize * arrindex).
// TODO: find out if there is already a hash table kind that follows similar
// principle.
//
//=============================================================================

#ifndef AGS_SHARED_AC_SPRITE_CACHE_H
#define AGS_SHARED_AC_SPRITE_CACHE_H

#include "common/std/memory.h"
#include "common/std/vector.h"
#include "common/std/list.h"
#include "ags/shared/ac/sprite_file.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/error.h"
#include "ags/shared/util/geometry.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class String;
class Stream;
class Bitmap;
} // namespace AGS3
} // namespace AGS

using namespace AGS; // FIXME later
typedef AGS::Shared::HError HAGSError;

struct SpriteInfo;

// Max size of the sprite cache, in bytes
#if AGS_PLATFORM_OS_ANDROID || AGS_PLATFORM_OS_IOS
#define DEFAULTCACHESIZE_KB (32 * 1024)
#else
#define DEFAULTCACHESIZE_KB (128 * 1024)
#endif

struct SpriteInfo;

namespace AGS {
namespace Shared {

class SpriteCache {
public:
	static const sprkey_t MIN_SPRITE_INDEX = 1; // 0 is reserved for "empty sprite"
	static const sprkey_t MAX_SPRITE_INDEX = INT32_MAX - 1;
	static const size_t   MAX_SPRITE_SLOTS = INT32_MAX;

	typedef Size (*PfnAdjustSpriteSize)(const Size &size, const uint32_t sprite_flags);
	typedef Bitmap *(*PfnInitSprite)(sprkey_t index, Bitmap *image, uint32_t &sprite_flags);
	typedef void (*PfnPostInitSprite)(sprkey_t index);
	typedef void (*PfnPrewriteSprite)(Bitmap *image);

	struct Callbacks {
		PfnAdjustSpriteSize AdjustSize;
		PfnInitSprite InitSprite;
		PfnPostInitSprite PostInitSprite;
		PfnPrewriteSprite PrewriteSprite;
	};

	SpriteCache(std::vector<SpriteInfo> &sprInfos, const Callbacks &callbacks);
	~SpriteCache() = default;

	// Loads sprite reference information and inits sprite stream
	HError      InitFile(const String &filename, const String &sprindex_filename);
	// Saves current cache contents to the file
	int         SaveToFile(const String &filename, int store_flags, SpriteCompression compress, SpriteFileIndex &index);
	// Closes an active sprite file stream
	void        DetachFile();

	inline int GetStoreFlags() const {
		return _file.GetStoreFlags();
	}
	inline SpriteCompression GetSpriteCompression() const {
		return _file.GetSpriteCompression();
	}

	// Tells if there is a sprite registered for the given index;
	// this includes sprites that were explicitly assigned but failed to init and were remapped
	bool        DoesSpriteExist(sprkey_t index) const;
	// Returns sprite's resolution; or empty Size if sprite does not exist
	Size		GetSpriteResolution(sprkey_t index) const;
	// Makes sure sprite cache has allocated slots for all sprites up to the given inclusive limit;
	// returns requested index on success, or -1 on failure.
	sprkey_t    EnlargeTo(sprkey_t topmost);
	// Finds a free slot index, if all slots are occupied enlarges sprite bank; returns index
	sprkey_t    GetFreeIndex();
	// Returns current size of the cache, in bytes; this includes locked size too!
	size_t      GetCacheSize() const;
	// Gets the total size of the locked sprites, in bytes
	size_t      GetLockedSize() const;
	// Returns maximal size limit of the cache, in bytes; this includes locked size too!
	size_t      GetMaxCacheSize() const;
	// Returns number of sprite slots in the bank (this includes both actual sprites and free slots)
	size_t      GetSpriteSlotCount() const;
	// Tells if the sprite storage still has unoccupied slots to put new sprites in
	bool		HasFreeSlots() const;
	// Tells if the given slot is reserved for the asset sprite, that is a "static"
	// sprite cached from the game assets
	bool		IsAssetSprite(sprkey_t index) const;
	// Loads sprite using SpriteFile if such index is known,
	// frees the space if cache size reaches the limit
	void        PrecacheSprite(sprkey_t index);
	// Locks sprite, preventing it from getting removed by the normal cache limit.
	// If this is a registered sprite from the game assets, then loads it first.
	// If this is a sprite with SPRCACHEFLAG_EXTERNAL flag, then does nothing,
	// as these are always "locked".
	// If such sprite does not exist, then fails silently.
	void        LockSprite(sprkey_t index);
	// Unlocks sprite, putting it back into the cache logic,
	// where it counts towards normal limit may be deleted to free space.
	// NOTE: sprites with SPRCACHEFLAG_EXTERNAL flag cannot be unlocked,
	// only explicitly removed.
	// If such sprite was not present in memory, then fails silently.
	void        UnlockSprite(sprkey_t index);
	// Unregisters sprite from the bank and returns the bitmap
	Bitmap		*RemoveSprite(sprkey_t index);
	// Deletes particular sprite, marks slot as unused
	void		DisposeSprite(sprkey_t index);
	// Deletes all loaded asset (non-locked, non-external) images from the cache;
	// this keeps all the auxiliary sprite information intact
	void        DisposeAllCached();
	// Deletes all data and resets cache to the clear state
	void        Reset();
	// Assigns new sprite for the given index; this sprite won't be auto disposed.
	// *Deletes* the previous sprite if one was found at the same index.
	// "flags" are SPF_* constants that define sprite's behavior in game.
	bool        SetSprite(sprkey_t index, std::unique_ptr<Bitmap> image, int flags = 0);
	// Assigns new dummy for the given index, silently remapping it to placeholder;
	// optionally marks it as an asset placeholder.
	// *Deletes* the previous sprite if one was found at the same index.
	void        SetEmptySprite(sprkey_t index, bool as_asset);
	// Sets max cache size in bytes
	void        SetMaxCacheSize(size_t size);

	// Loads (if it's not in cache yet) and returns bitmap by the sprite index
	Bitmap *operator[](sprkey_t index);

private:
	// Load sprite from game resource
	size_t      LoadSprite(sprkey_t index, bool lock = false);
	// Remap the given index to the placeholder
	void        RemapSpriteToPlaceholder(sprkey_t index);
	// Delete the oldest (least recently used) image in cache
	void        DisposeOldest();
	// Keep disposing oldest elements until cache has at least the given free space
	void        FreeMem(size_t space);
	// Initialize the empty sprite slot
	void 		InitNullSprite(sprkey_t index);
	//
    // Dummy no-op variants for callbacks
    //
	static Size   DummyAdjustSize(const Size &size, const uint32_t) { return size; }
	static Bitmap *DummyInitSprite(sprkey_t, Bitmap *image, uint32_t &) { return image; }
	static void   DummyPostInitSprite(sprkey_t) { /* do nothing */ }
	static void   DummyPrewriteSprite(Bitmap *) { /* do nothing */ }

	// Information required for the sprite streaming
	struct SpriteData {
		size_t	 Size  = 0;			   // to track cache size, 0 = means don't track
		uint32_t Flags = 0;			   // SPRCACHEFLAG* flags
		std::unique_ptr<Bitmap> Image; // actual bitmap

		// MRU list reference
		std::list<sprkey_t>::iterator MruIt;

		SpriteData() = default;
		SpriteData(SpriteData &&other) = default;
		SpriteData(Bitmap *image, size_t size, uint32_t flags) : Size(size), Flags(flags), Image(image) {}

		SpriteData &operator=(SpriteData &&other) = default;

		// Tells if this slot has a valid sprite assigned (not empty slot)
		bool IsValid() const { return Flags != 0u; }
		// Tells if there actually is a registered sprite in this slot
		bool DoesSpriteExist() const;
		// Tells if there's a game resource corresponding to this slot
		bool IsAssetSprite() const;
		// Tells if a sprite failed to load from assets, and should not be used
		bool IsError() const;
		// Tells if sprite was added externally, not loaded from game resources
		bool IsExternalSprite() const;
		// Tells if sprite is locked and should not be disposed by cache logic
		bool IsLocked() const;
	};

	// Provided map of sprite infos, to fill in loaded sprite properties
	std::vector<SpriteInfo> &_sprInfos;
	// Array of sprite references
	std::vector<SpriteData> _spriteData;
	// Placeholder sprite, returned from operator[] for a non-existing sprite
	std::unique_ptr<Bitmap> _placeholder;

	Callbacks _callbacks;
	SpriteFile _file;

	size_t _maxCacheSize;  // cache size limit
	size_t _lockedSize;    // size in bytes of currently locked images
	size_t _cacheSize;     // size in bytes of currently cached images

	// MRU list: the way to track which sprites were used recently.
	// When clearing up space for new sprites, cache first deletes the sprites
	// that were last time used long ago.
	std::list<sprkey_t> _mru;

};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
