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

#include "ultima8/misc/pent_include.h"
#include "ultima8/kernel/memory_manager.h"
#include "ultima8/kernel/segmented_allocator.h"

namespace Ultima8 {

MemoryManager *MemoryManager::memorymanager = 0;

MemoryManager::MemoryManager() {
	assert(memorymanager == 0);
	memorymanager = this;

	//!!! CONSTANT !!!!
	allocatorCount = 2;
	// Tune these with averages from MemoryManager::MemInfo when needed
	allocators[0] = new SegmentedAllocator(192, 8500);
	allocators[1] = new SegmentedAllocator(4224, 25);

	Pentagram::setAllocationFunctions(MemoryManager::allocate,
	                                  MemoryManager::deallocate);
}

MemoryManager::~MemoryManager() {
	memorymanager = 0;

	Pentagram::setAllocationFunctions(malloc, free);
	delete allocators[0];
	delete allocators[1];
}

void *MemoryManager::_allocate(size_t size) {
	int i;
	// get the memory from the first allocator that can hold "size"
	for (i = 0; i < allocatorCount; ++i) {
		if (allocators[i]->getCapacity() >= size) {
			return allocators[i]->allocate(size);
		}
	}

	// else
	void *ptr = malloc(size);
#ifdef DEBUG
	con.Printf("MemoryManager::allocate - Allocated %d bytes to 0x%X\n", size, ptr);
#endif

	return ptr;
}

void MemoryManager::_deallocate(void *ptr) {
	Pool *p;
	int i;
	for (i = 0; i < allocatorCount; ++i) {
		p = allocators[i]->findPool(ptr);
		if (p) {
			p->deallocate(ptr);
			return;
		}
	}

#ifdef DEBUG
	con.Printf("MemoryManager::deallocate - deallocating memory at 0x%X\n", ptr);
#endif
	// Pray!
	free(ptr);
}

void MemoryManager::freeResources() {
	int i;
	for (i = 0; i < allocatorCount; ++i) {
		allocators[i]->freeResources();
	}
}

void MemoryManager::ConCmd_MemInfo(const Console::ArgvType &argv) {
	MemoryManager *mm = MemoryManager::get_instance();
	int i, count;

	if (!mm)
		return;

	count = mm->getAllocatorCount();
	pout << "Allocators: " << count << std::endl;
	for (i = 0; i < count; ++i) {
		pout << " Allocator " << i << ": " << std::endl;
		mm->getAllocator(i)->printInfo();
		pout << "==============" << std::endl;
	}
}

#ifdef DEBUG

#include <SDL.h>
// Test classes purely here to check the speed of Allocators vs. normal allocation
class TestClassBase {
public:
	TestClassBase() {
		next = 0;
	}

	virtual ~TestClassBase() {
	}

	void setNext(TestClassBase *n) {
		n->next = next;
		next = n;
	}

	void removeNext() {
		TestClassBase *n;
		if (! next)
			return;
		n = next;
		next = n->next;
		delete n;
	}

	ENABLE_RUNTIME_CLASSTYPE()

	TestClassBase *next;
	int arr[32];
};

DEFINE_RUNTIME_CLASSTYPE_CODE_BASE_CLASS(TestClassBase)

class TestClassOne: public TestClassBase {
public:
	TestClassOne() {
	}

	virtual ~TestClassOne() {
	}

	ENABLE_RUNTIME_CLASSTYPE()
};

DEFINE_RUNTIME_CLASSTYPE_CODE(TestClassOne, TestClassBase)


class TestClassTwo: public TestClassBase {
public:
	TestClassTwo() {
	}

	virtual ~TestClassTwo() {
	}

	ENABLE_RUNTIME_CLASSTYPE()
	ENABLE_CUSTOM_MEMORY_ALLOCATION()
};

DEFINE_RUNTIME_CLASSTYPE_CODE(TestClassTwo, TestClassBase)
DEFINE_CUSTOM_MEMORY_ALLOCATION(TestClassTwo)

void MemoryManager::ConCmd_test(const Console::ArgvType &argv) {
	// Just some numbers of classes to allocate and free
	int a[10] = {1000, 1231, 2423, 1233, 3213, 2554, 1123, 2432, 3311, 1022};
	int b[10] = {900, 1111, 2321, 1000, 1321, 1432, 1123, 2144, 2443, 0};
	int i, j, repeat;
	uint32 pooled, unpooled;
	TestClassBase *t;

	t = new TestClassBase();

	unpooled = SDL_GetTicks();
	for (repeat = 0; repeat < 100; ++repeat) {
		for (i = 0; i < 10; ++i) {
			// allocate
			for (j = 0; j < a[i]; ++j) {
				t->setNext(new TestClassOne());
			}
			// free
			for (j = 0; j < b[i]; ++j) {
				t->removeNext();
			}
		}
		while (t->next) {
			t->removeNext();
		}
	}
	unpooled = SDL_GetTicks() - unpooled;

	pooled = SDL_GetTicks();
	for (repeat = 0; repeat < 100; ++repeat) {
		for (i = 0; i < 10; ++i) {
			// allocate
			for (j = 0; j < a[i]; ++j) {
				t->setNext(new TestClassTwo());
			}
			// free
			for (j = 0; j < b[i]; ++j) {
				t->removeNext();
			}
		}
		while (t->next) {
			t->removeNext();
		}
	}
	pooled = SDL_GetTicks() - pooled;

	delete t;

	con.Printf("Unpooled Allocation: %d ms\nPooled Allocation: %d ms\n", unpooled, pooled);
}

#else
void MemoryManager::ConCmd_test(const Console::ArgvType &argv) {
}

#endif

} // End of namespace Ultima8
