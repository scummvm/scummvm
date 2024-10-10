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
// ResourceCache is an abstract storage that tracks use history with MRU list.
// Cache is limited to a certain size, in bytes.
// When a total size of items reaches the limit, and more items are put into,
// the Cache uses MRU list to find the least used items and disposes them
// one by one until the necessary space is freed.
// ResourceCache's implementations must provide a method for calculating an
// item's size.
//
// Supports copyable and movable items, have 2 variants of Put function for
// each of them. This lets it store both std::shared_ptr and std::unique_ptr.
//
// TODO: support data Priority, which tells which items may be disposed
// when adding new item and surpassing the cache limit.
//
// TODO: as an option, consider to have Locked items separate from the normal
// cache limit, and probably have their own limit setting as a safety measure.
// (after reaching this limit ResourceCache would simply ignore any further
// Lock commands until some items are unlocked.)
// Rethink this when it's time to design a better resource handling in AGS.
//
// TODO: as an option, consider supporting a specialized container type that
// has an associative container's interface, but is optimized for having most
// keys allocated in large continious sequences by default.
// This may be suitable for e.g. sprites, and may (or may not?) save some mem.
//
// Only for the reference: one of the ideas is for container to have a table
// of arrays of fixed size internally. When getting an item the hash would be
// first divided on array size to find the array the item resides in, then the
// item is taken from item from slot index = (hash - arrsize * arrindex).
// Find out if there is already a hash table kind that follows similar
// principle.
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_RESOURCE_CACHE_H
#define AGS_SHARED_UTIL_RESOURCE_CACHE_H

#include "common/std/list.h"
#include "common/std/map.h"
#include "ags/shared/util/string.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

template<typename TKey, typename TValue,
		 typename TSize = size_t, typename HashFn = std::hash<TKey> >
class ResourceCache {
public:
	// Flags determine management rules for the particular item
	enum ItemFlags {
		// Locked items are temporarily saved from disposal when freeing cache space;
		// they still count towards the cache size though.
		kCacheItem_Locked = 0x0001,
		// External items are managed strictly by the external user;
		// do not count towards the cache size, do not prevent caching normal items.
		// They cannot be locked or released (considered permanently locked),
		// only removed by request.
		kCacheItem_External = 0x0002,
	};

	ResourceCache(TSize max_size = 0u)
		: _maxSize(max_size), _sectionLocked(_mru.end()) {}

	// Get the MRU cache size limit
	inline size_t GetMaxCacheSize() const { return _maxSize; }
	// Get the current total MRU cache size
	inline size_t GetCacheSize() const { return _cacheSize; }
	// Get the summed size of locked items (included in total cache size)
	inline size_t GetLockedSize() const { return _lockedSize; }
	// Get the summed size of external items (excluded from total cache size)
	inline size_t GetExternalSize() const { return _externalSize; }

	// Set the MRU cache size limit
	void SetMaxCacheSize(TSize size) {
		_maxSize = size;
		FreeMem(0u); // makes sure it does not exceed max size
	}

	// Tells if particular key is in the cache
	bool Exists(const TKey &key) const {
		return _storage.find(key) != _storage.end();
	}

	// Gets the item with the given key if it exists;
	// reorders the item as recently used.
	const TValue &Get(const TKey &key) {
		auto it = _storage.find(key);
		if (it == _storage.end())
			return _dummy; // no such key

		// Unless locked, move the item ref to the beginning of the MRU list
		const auto &item = it->second;
		if ((item.Flags & kCacheItem_Locked) == 0)
			_mru.splice(_mru.begin(), _mru, item.MruIt);
		return item.Value;
	}

	// Add particular item into the cache, disposes existing item if such key is already taken.
	// If a new item will exceed the cache size limit, cache will remove oldest items
	// in order to free mem.
	void Put(const TKey &key, const TValue &value, uint32_t flags = 0u) {
		if (_maxSize == 0)
			return; // cache is disabled
		auto it = _storage.find(key);
		if (it != _storage.end()) {
			// Remove previous cached item
			RemoveImpl(it);
		}
		PutImpl(key, TValue(value), flags); // make a temp local copy for safe std::move
	}

	void Put(const TKey &key, TValue &&value, uint32_t flags = 0u) {
		if (_maxSize == 0)
			return; // cache is disabled
		auto it = _storage.find(key);
		if (it != _storage.end()) {
			// Remove previous cached item
			RemoveImpl(it);
		}
		PutImpl(key, std::move(value), flags);
	}

	// Locks the item with the given key,
	// temporarily excluding it from MRU disposal rules
	void Lock(const TKey &key) {
		auto it = _storage.find(key);
		if (it == _storage.end())
			return; // no such key
		auto &item = it->second;
		if ((item.Flags & kCacheItem_Locked) != 0)
			return; // already locked

		// Lock item and move to the locked section
		item.Flags |= kCacheItem_Locked;
		_mru.splice(_sectionLocked, _mru, item.MruIt); // CHECKME: TEST!!
		_sectionLocked = item.MruIt;
		_lockedSize += item.Size;
	}

	// Releases (unlocks) the item with the given key,
	// adds it back to MRU disposal rules
	void Release(const TKey &key) {
		auto it = _storage.find(key);
		if (it == _storage.end())
			return; // no such key

		auto &item = it->second;
		if ((item.Flags & kCacheItem_External) != 0)
			return; // never release external data, must be removed by user
		if ((item.Flags & kCacheItem_Locked) == 0)
			return; // not locked

		// Unlock, and move the item to the beginning of the MRU list
		item.Flags &= ~kCacheItem_Locked;
		if (_sectionLocked == item.MruIt)
			_sectionLocked = std::next(item.MruIt);
		_mru.splice(_mru.begin(), _mru, item.MruIt); // CHECKME: TEST!!
		_lockedSize -= item.Size;
	}

	// Deletes the cached item
	void Dispose(const TKey &key) {
		auto it = _storage.find(key);
		if (it == _storage.end())
			return; // no such key
		RemoveImpl(it);
	}

	// Removes the item from the cache and returns to the caller.
	TValue Remove(const TKey &key) {
		auto it = _storage.find(key);
		if (it == _storage.end())
			return TValue(); // no such key
		TValue value = std::move(it->second.Value);
		RemoveImpl(it);
		return value;
	}

	// Disposes all items that are not locked or external
	void DisposeFreeItems() {
		for (auto mru_it = _sectionLocked; mru_it != _mru.end(); ++mru_it) {
			auto it = _storage.find(*mru_it);
			assert(it != _storage.end());
			auto &item = it->second;
			_cacheSize -= item.Size;
			_storage.erase(it);
			_mru.erase(mru_it);
		}
	}

	// Clear the cache, dispose all items
	void Clear() {
		_storage.clear();
		_mru.clear();
		_sectionLocked = _mru.end();
		_cacheSize = 0u;
		_lockedSize = 0u;
		_externalSize = 0u;
	}

protected:
	struct TItem;
	// MRU list type
	typedef std::list<TKey> TMruList;
	// MRU list reference type
	typedef typename TMruList::iterator TMruIt;
	// Storage type
	typedef std::unordered_map<TKey, TItem, HashFn> TStorage;

	struct TItem {
		TMruIt MruIt; // MRU list reference
		TValue Value;
		TSize Size = 0u;
		uint32_t Flags = 0u; // flags determine management rules for this item

		TItem() = default;
		TItem(const TItem &item) = default;
		TItem(TItem &&item) = default;
		TItem(const TMruIt &mru_it, const TValue &value, const TSize size, uint32_t flags)
			: MruIt(mru_it), Value(value), Size(size), Flags(flags) {}
		TItem(const TMruIt &mru_it, TValue &&value, const TSize size, uint32_t flags)
			: MruIt(mru_it), Value(std::move(value)), Size(size), Flags(flags) {}
		TItem &operator=(const TItem &item) = default;
		TItem &operator=(TItem &&item) = default;
	};

	// Calculates item size; expects to return 0 if an item is invalid
	// and should not be added to the cache.
	virtual TSize CalcSize(const TValue &item) = 0;

private:
	// Add particular item into the cache.
	// If a new item will exceed the cache size limit, cache will remove oldest items
	// in order to free mem.
	void PutImpl(const TKey &key, TValue &&value, uint32_t flags) {
		// Request item's size, and test if it's a valid item
		TSize size = CalcSize(value);
		if (size == 0u)
			return; // invalid item

		if ((flags & kCacheItem_External) == 0) {
			// clear up space before adding
			if (_cacheSize + size > _maxSize)
				FreeMem(size);
			_cacheSize += size;
		} else {
			// always mark external data as locked, easier to handle
			flags |= kCacheItem_Locked;
			_externalSize += size;
		}

		// Prepare a MRU slot, then add an item
		TMruIt mru_it = _mru.end();
		// only normal items are added to MRU at all
		if ((flags & kCacheItem_External) == 0) {
			if ((flags & kCacheItem_Locked) == 0) {
				// normal item, add to the list
				mru_it = _mru.insert(_mru.begin(), key);
			} else {
				// locked item, add to the dedicated list section
				mru_it = _mru.insert(_sectionLocked, key);
				_sectionLocked = mru_it;
				_lockedSize += size;
			}
		}
		TItem item = TItem(mru_it, std::move(value), size, flags);
		_storage[key] = std::move(item);
	}
	// Removes the item from the container
	void RemoveImpl(typename TStorage::iterator it) {
		auto &item = it->second;
		// normal items are removed from MRU, and discounted from cache size
		if ((item.Flags & kCacheItem_External) == 0) {
			TMruIt mru_it = item.MruIt;
			if (_sectionLocked == mru_it)
				_sectionLocked = std::next(mru_it);
			_cacheSize -= item.Size;
			if ((item.Flags & kCacheItem_Locked) != 0)
				_lockedSize -= item.Size;
			_mru.erase(mru_it);
		} else {
			_externalSize -= item.Size;
		}
		_storage.erase(it);
	}
	// Remove the oldest (least recently used) item in cache
	void DisposeOldest() {
		assert(_mru.begin() != _sectionLocked);
		if (_mru.begin() == _sectionLocked)
			return;
		// Remove from the storage and mru list
		auto mru_it = std::prev(_sectionLocked);
		auto it = _storage.find(*mru_it);
		assert(it != _storage.end());
		auto &item = it->second;
		assert((item.Flags & (kCacheItem_Locked | kCacheItem_External)) == 0);
		_cacheSize -= item.Size;
		_storage.erase(it);
		_mru.erase(mru_it);
	}
	// Keep disposing oldest elements until cache has at least the given free space
	void FreeMem(size_t space) {
		// TODO: consider sprite cache's behavior where it would just clear
		// whole cache in case disposing one by one were taking too much iterations
		while ((_mru.begin() != _sectionLocked) && (_cacheSize + space > _maxSize)) {
			DisposeOldest();
		}
	}

	// Size of tracked data stored in this cache;
	// note that this is an abstract value, which may or not refer to an
	// actual size in bytes, and depends on the implementation.
	TSize _cacheSize = 0u;
	// Size of data locked (forbidden from disposal),
	// this size is *included* in _cacheSize; provided for stats.
	TSize _lockedSize = 0u;
	// Size of the external data, that is - data that does not count towards
	// cache limit, and which is not our reponsibility; provided for stats.
	TSize _externalSize = 0u;
	// Maximal size of tracked data.
	// When the inserted item increases the cache size past this limit,
	// the cache will try to free the space by removing oldest items.
	// "External" data does not count towards this limit.
	TSize _maxSize = 0u;
	// MRU list: the way to track which items were used recently.
	// When clearing up space for new items, cache first deletes the items
	// that were last time used long ago.
	TMruList _mru;
	// A locked section border iterator, points to the *last* locked item
	// starting from the end of the list, or equals _mru.end() if there's none.
	TMruIt _sectionLocked;
	// Key-to-mru lookup map
	TStorage _storage;
	// Dummy value, return in case of a missing key
	TValue _dummy;
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif // AGS_SHARED_UTIL_RESOURCE_CACHE_H