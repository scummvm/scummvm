/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/memory_manager.h"
#include "ultima/ultima8/kernel/segmented_allocator.h"

namespace Ultima {
namespace Ultima8 {

MemoryManager *MemoryManager::_memoryManager;

MemoryManager::MemoryManager() {
	_memoryManager = this;

	//!!! CONSTANT !!!!
	_allocatorCount = 2;
	// Tune these with averages from MemoryManager::MemInfo when needed
	_allocators[0] = new SegmentedAllocator(192, 8500);
	_allocators[1] = new SegmentedAllocator(4224, 25);

	setAllocationFunctions(MemoryManager::allocate,
	                                  MemoryManager::deallocate);
}

MemoryManager::~MemoryManager() {
	_memoryManager = nullptr;

	setAllocationFunctions(malloc, free);
	delete _allocators[0];
	delete _allocators[1];
}

void *MemoryManager::_allocate(size_t size) {
	int i;
	// get the memory from the first allocator that can hold "size"
	for (i = 0; i < _allocatorCount; ++i) {
		if (_allocators[i]->getCapacity() >= size) {
			return _allocators[i]->allocate(size);
		}
	}

	// else
	void *ptr = malloc(size);
#ifdef DEBUG
	debugN"MemoryManager::allocate - Allocated %d bytes to 0x%X\n", size, ptr);
#endif

	return ptr;
}

void MemoryManager::_deallocate(void *ptr) {
	Pool *p;
	int i;
	for (i = 0; i < _allocatorCount; ++i) {
		p = _allocators[i]->findPool(ptr);
		if (p) {
			p->deallocate(ptr);
			return;
		}
	}

#ifdef DEBUG
	debugN"MemoryManager::deallocate - deallocating memory at 0x%X\n", ptr);
#endif
	// Pray!
	free(ptr);
}

void MemoryManager::freeResources() {
	int i;
	for (i = 0; i < _allocatorCount; ++i) {
		_allocators[i]->freeResources();
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
