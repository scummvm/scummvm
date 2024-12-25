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

#include "watchmaker/3d/mem_management.h"
#include "watchmaker/types.h"
#include "watchmaker/ll/ll_system.h"

namespace Watchmaker {

static int8    *MemoryPool = nullptr;
static uint32   MemoryPoolPointer = 0;
static uint32   MaxAllocatedMem = 0;

// Memory Management
// Only linear (non dynamic) stack allocations


/* -----------------10/06/99 16.06-------------------
 *              t3dAllocMemoryPool
 * --------------------------------------------------*/
uint8 t3dAllocMemoryPool(uint32 pool) {
	uint8 allocated = 0;

	MemoryPoolPointer = 0;
	while ((allocated == 0) && (pool > 0)) {
		if (!(MemoryPool = t3dCalloc<int8>(pool)))
			pool -= 10000;
		else
			allocated = 1;
	}

	if (allocated == 0)
		return 0;
	else {
		MaxAllocatedMem = pool;
		return 1;
	}
}

/* -----------------10/06/99 16.07-------------------
 *                  t3dDeallocMemoryPool
 * --------------------------------------------------*/
void t3dDeallocMemoryPool() {
	if (MemoryPool == nullptr) return;
	t3dFree(MemoryPool);
	MemoryPool = nullptr;
	MaxAllocatedMem = 0;
}

/* -----------------10/06/99 16.07-------------------
 *                      t3dAlloc
 * --------------------------------------------------*/
void *t3dAlloc(uint32 size) {
	if (MemoryPool == nullptr) return nullptr;
	MemoryPoolPointer += size;
	if (MemoryPoolPointer > MaxAllocatedMem) {
		return nullptr;
	}

	return &MemoryPool[MemoryPoolPointer - size];
}

/* -----------------10/06/99 16.07-------------------
 *                  t3dDealloc
 * --------------------------------------------------*/
void *t3dDealloc(uint32 size) {
	if (MemoryPool == nullptr) return nullptr;
	MemoryPoolPointer -= size;
	return &MemoryPool[MemoryPoolPointer];
}

} // End of namespace Watchmaker
