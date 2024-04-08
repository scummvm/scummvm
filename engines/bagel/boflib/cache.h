
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

#ifndef BAGEL_BOFLIB_CACHE_H
#define BAGEL_BOFLIB_CACHE_H

#include "bagel/boflib/llist.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

class CCache : private CLList {
private:
	virtual bool Alloc() = 0;       // These pure-virtuals MUST be defined
	virtual void Free() = 0;        // in the derived class.

	static CCache *m_pCacheList;    // linked cache list
	static uint32 m_lOldest;         // current oldest object in cache
	static uint32 m_lYoungest;       // current youngest object in cache
	static uint16 m_nEntries;       // number of CCache Objects

	uint32 m_lAge;       // age of this object
	int m_nLockCount;   // # of locks held on this object
	bool m_bCached;     // true if object is in the cache

	bool m_bValid;      // true if this object is valid

public:
	/**
	 * Constructor
	 */
	CCache();

	/**
	 * Destructor
	 */
	virtual ~CCache();

	/**
	 * Loads current object into cache
	 */
	void Load();

	/**
	 * Releases current object from cache
	 */
	bool Release();

	void Lock() {
		m_nLockCount++;
		Load();
	}
	void UnLock() {
		m_nLockCount--;
	}
	bool IsLocked() {
		return (m_nLockCount > 0);
	}

	/**
	 * initialize statics
	*/
	static void initialize();

	/**
	 * Releases all objects from cache
	 */
	static bool Flush();

	/**
	 * Releases older objects from cache if need memory
	 * @remarks     Optimizes cache so that the specified ammount of memory is left available.
	 * @param lRequestedFreeSpace   Free space requested to remain available
	 * @return      true if there were any objects to release from the cache.
	 *              FALSE if there were no objects released.
	 */
	static bool Optimize(uint32 lRequestedFreeSpace);
};

} // namespace Bagel

#endif
