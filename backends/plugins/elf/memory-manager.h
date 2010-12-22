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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ELF_MEMORY_MANAGER_H
#define ELF_MEMORY_MANAGER_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#include "common/singleton.h"
#include "common/list.h"
#include "common/mutex.h"
 
/**
 * A 'foolproof' way to prevent memory fragmentation. This class is used to 
 * serve as a permanent allocation to prevent the process of loading and 
 * unloading plugins from causing heavy fragmentation.
 **/
 
#define ELFMemMan		ELFMemoryManager::instance()
 
class ELFMemoryManager : public Common::Singleton<ELFMemoryManager> {
public:	
	void trackPlugin(bool value);
	void trackAlloc(size_t align, size_t size);

	void allocateHeap();

	void *pluginAllocate(size_t size);
	void *pluginAllocate(size_t align, size_t size);
	void pluginDeallocate(void *ptr);
	
private:
   friend class Common::Singleton<ELFMemoryManager>;

	ELFMemoryManager();
	~ELFMemoryManager();

	void *allocateOnHeap(size_t align, size_t size);
	void deallocateFromHeap(void *ptr);
	
	struct Allocation {
		byte *start;
		size_t size;
		byte *end() { return start + size; }
		Allocation(byte *a, size_t b) : start(a), size(b) {}
	};

	// heap
	void *_heap;
	size_t _heapAlign;			// alignment of the heap
	size_t _heapSize;			// size of the heap
	
	// tracking allocations
	bool _trackAllocs;		// whether we are currently tracking
	size_t _measuredSize; 
	size_t _measuredAlign;	
	
	// real allocations
	Common::List<Allocation> _allocList;
	uint32 _bytesAllocated;
};
 
#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) */

#endif /* ELF_MEMORY_MANAGER_H */
