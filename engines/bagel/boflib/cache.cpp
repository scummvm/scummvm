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

#include "bagel/boflib/cache.h"
#include "bagel/boflib/log.h"
#include "bagel/boflib/debug.h"

namespace Bagel {

// static members
//
CCache *CCache::m_pCacheList;
uint32 CCache::m_lOldest;
uint32 CCache::m_lYoungest;
uint16 CCache::m_nEntries;

void CCache::initialize() {
	m_pCacheList = nullptr;
	m_lOldest = 0xFFFFFFFF;
	m_lYoungest = 0xFFFFFFFF;
	m_nEntries = 0;
}

CCache::CCache() {
	m_bValid = TRUE;

	// Inits
	m_lAge = m_lYoungest;
	m_bCached = FALSE;
	m_nLockCount = 0;

	if (m_pCacheList == nullptr) {
		// There can't be any other entries
		Assert(m_nEntries == 0);

		// This object is the start of the cache list
		m_pCacheList = this;

	} else {
		Assert(m_nEntries != 0);

		m_pCacheList->Insert(this);
	}

	// one more cached entry
	m_nEntries++;
}

CCache::~CCache() {
	// Make sure this object is not deleted twice
	Assert(m_bValid);

	// There can be only one cache list per EXE/DLL
	Assert(this->GetHead() == m_pCacheList);

	// Object must be released by the derived class destructor
	Assert(m_bCached == FALSE);

	m_nEntries--;

	// Remove this object from the linked list
	if (this == m_pCacheList) {
		m_pCacheList = (CCache *)this->GetNext();
	}

	// obsolete?
	this->Delete();

	// obsolete?
	if (m_nEntries == 0) {
		m_pCacheList = nullptr;
	}

	m_bValid = FALSE;
}

bool CCache::Flush() {
	CCache *pCache;
	bool bReleased;

	bReleased = FALSE;
	pCache = m_pCacheList;
	while (pCache != nullptr) {

		if (pCache->Release())
			bReleased = TRUE;

		pCache = (CCache *)(pCache->GetNext());
	}

	return bReleased;
}

bool CCache::Optimize(uint32 lRequestedFreeSpace) {
	CCache *pCache;
	uint32 lAvgAge;
	INT nObjects;
	bool bSuccess;

	LogInfo(BuildString("CCache::Optimize(%ld)", lRequestedFreeSpace));

	bSuccess = TRUE;
	while (GetFreePhysMem() < lRequestedFreeSpace) {
		lAvgAge = ((m_lOldest - m_lYoungest) / 2) + m_lYoungest;
		m_lOldest = lAvgAge;

		// Parse Linked list of cached objects and remove any that are too old
		pCache = m_pCacheList;
		nObjects = 0;
		while (pCache != nullptr) {

			if (pCache->m_bCached && (pCache->m_nLockCount <= 0)) {
				nObjects++;
				if (pCache->m_lAge >= lAvgAge) {
					LogInfo(BuildString("Freeing Object: %08lx from the Cache list", pCache));
					pCache->Release();
				}
			}

			// Next object in list
			pCache = (CCache *)pCache->GetNext();
		}

		// If there are no objects loaded then we really might be out of memory
		if (nObjects == 0) {
			bSuccess = FALSE;
			break;
		}
	}

	return bSuccess;
}

void CCache::Load() {
	// Make sure this object is not used after it is destructed
	Assert(m_bValid);

	// Timestamp this object
	m_lAge = --m_lYoungest;

	// If this object is not already cached
	if (!m_bCached) {
		// We must tell the Cache library that this object is in the cache to avoid recursion
		m_bCached = TRUE;

		// Call the routine that will allocate the memory for this object
		// Alloc() is pure-virtual and must be defined in the derived class.
		m_bCached = Alloc();
	}
}

bool CCache::Release() {
	bool bReleased;

	// Make sure this object is not used after it is destructed
	Assert(m_bValid);

	// If this object is in the cache
	bReleased = FALSE;
	if (m_bCached) {
		bReleased = TRUE;

		// Call the routine that will free the memory used by this object.
		// Free() is pure-virtual and must be defined in the derived class.
		Free();

		m_bCached = FALSE;
	}

	return bReleased;
}

} // namespace Bagel
