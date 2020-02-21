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

#ifndef ULTIMA8_KERNEL_MEMORYMANAGER_H
#define ULTIMA8_KERNEL_MEMORYMANAGER_H

namespace Ultima {
namespace Ultima8 {

class Allocator;

class MemoryManager {
public:
	MemoryManager();
	~MemoryManager();

	static MemoryManager *get_instance() {
		return _memoryManager;
	}

	//! Allocates memory with the default allocator or malloc
	static void *allocate(size_t size) {
		return _memoryManager ? _memoryManager->_allocate(size) : 0;
	}

	//! Checks all known Allocators to free memory
	static void deallocate(void *ptr) {
		_memoryManager->_deallocate(ptr);
	}

	Allocator *getAllocator(uint16 index) {
		return index < _allocatorCount ? _allocators[index] : 0;
	}

	uint16 getAllocatorCount() {
		return _allocatorCount;
	}

	void freeResources();

private:
	Allocator *_allocators[10];
	uint16 _allocatorCount;

	void *_allocate(size_t size);
	void _deallocate(void *ptr);

	static MemoryManager *_memoryManager;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
