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
#include "bagel/spacebar/boflib/debug.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

// Static members
CCache *CCache::_pCacheList;
uint32 CCache::_lOldest;
uint32 CCache::_lYoungest;
uint16 CCache::_nEntries;

void CCache::initialize() {
	_pCacheList = nullptr;
	_lOldest = 0xFFFFFFFF;
	_lYoungest = 0xFFFFFFFF;
	_nEntries = 0;
}

CCache::CCache() {
	_bValid = true;

	// Inits
	_lAge = _lYoungest;
	_bCached = false;
	_nLockCount = 0;

	if (_pCacheList == nullptr) {
		// There can't be any other entries
		assert(_nEntries == 0);

		// This object is the start of the cache list
		_pCacheList = this;

	} else {
		assert(_nEntries != 0);

		_pCacheList->Insert(this);
	}

	// One more cached entry
	_nEntries++;
}

CCache::~CCache() {
	// Make sure this object is not deleted twice
	assert(_bValid);

	// There can be only one cache list per EXE/DLL
	assert(this->getHead() == _pCacheList);

	// Object must be released by the derived class destructor
	assert(_bCached == false);

	_nEntries--;

	// Remove this object from the linked list
	if (this == _pCacheList) {
		_pCacheList = (CCache *)this->getNext();
	}

	this->Delete();

	if (_nEntries == 0) {
		_pCacheList = nullptr;
	}

	_bValid = false;
}

bool CCache::flush() {
	bool bReleased = false;
	CCache *pCache = _pCacheList;
	while (pCache != nullptr) {
		if (pCache->release())
			bReleased = true;

		pCache = (CCache *)(pCache->getNext());
	}

	return bReleased;
}

bool CCache::optimize(uint32 lRequestedFreeSpace) {
	logInfo(buildString("CCache::optimize(%u)", lRequestedFreeSpace));

	bool bSuccess = true;
	while (getFreePhysMem() < lRequestedFreeSpace) {
		uint32 lAvgAge = ((_lOldest - _lYoungest) / 2) + _lYoungest;
		_lOldest = lAvgAge;

		// Parse Linked list of cached objects and remove any that are too old
		CCache *pCache = _pCacheList;
		int nObjects = 0;
		while (pCache != nullptr) {
			if (pCache->_bCached && (pCache->_nLockCount <= 0)) {
				nObjects++;
				if (pCache->_lAge >= lAvgAge) {
					logInfo(buildString("Freeing Object: %p from the Cache list", pCache));
					pCache->release();
				}
			}

			// Next object in list
			pCache = (CCache *)pCache->getNext();
		}

		// If there are no objects loaded then we really might be out of memory
		if (nObjects == 0) {
			bSuccess = false;
			break;
		}
	}

	return bSuccess;
}

void CCache::load() {
	// Make sure this object is not used after it is destructed
	assert(_bValid);

	// Timestamp this object
	_lAge = --_lYoungest;

	// If this object is not already cached
	if (!_bCached) {
		// We must tell the Cache library that this object is in the cache to avoid recursion
		_bCached = true;

		// Call the routine that will allocate the memory for this object
		// alloc() is pure-virtual and must be defined in the derived class.
		_bCached = alloc();
	}
}

bool CCache::release() {
	// Make sure this object is not used after it is destructed
	assert(_bValid);

	// If this object is in the cache
	bool bReleased = false;
	if (_bCached) {
		bReleased = true;

		// Call the routine that will free the memory used by this object.
		// free() is pure-virtual and must be defined in the derived class.
		free();

		_bCached = false;
	}

	return bReleased;
}

} // namespace Bagel
