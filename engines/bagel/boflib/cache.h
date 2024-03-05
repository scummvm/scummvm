
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
	VIRTUAL BOOL Alloc() = 0;       // These pure-virtuals MUST be defined
	VIRTUAL VOID Free() = 0;        // in the derived class.

	STATIC CCache *m_pCacheList;    // linked cache list
	STATIC ULONG m_lOldest;         // current oldest object in cache
	STATIC ULONG m_lYoungest;       // current youngest object in cache
	STATIC USHORT m_nEntries;       // number of CCache Objects

	ULONG m_lAge;       // age of this object
	INT m_nLockCount;   // # of locks held on this object
	BOOL m_bCached;     // TRUE if object is in the cache

	BOOL m_bValid;      // TRUE if this object is valid

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
	VOID Load();

	/**
	 * Releases current object from cache
	 */
	BOOL Release();

	VOID Lock() {
		m_nLockCount++;
		Load();
	}
	VOID UnLock() {
		m_nLockCount--;
	}
	BOOL IsLocked() {
		return (m_nLockCount > 0);
	}

	/**
	 * Initialize statics
	*/
	static void initStatics();

	/**
	 * Releases all objects from cache
	 */
	STATIC BOOL Flush();

	/**
	 * Releases older objects from cache if need memory
	 * @remarks     Optimizes cache so that the specified ammount of memory is left available.
	 * @param lRequestedFreeSpace   Free space requested to remain available
	 * @return      TRUE if there were any objects to release from the cache.
	 *              FALSE if there were no objects released.
	 */
	STATIC BOOL Optimize(ULONG lRequestedFreeSpace);
};

} // namespace Bagel

#endif
