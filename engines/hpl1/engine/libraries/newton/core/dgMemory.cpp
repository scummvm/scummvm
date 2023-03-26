/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "dgStdafx.h"
#include "dgList.h"
#include "dgDebug.h"
#include "dgMemory.h"

class dgGlobalAllocator: public dgMemoryAllocator, public dgList<dgMemoryAllocator *> {
public:
	dgGlobalAllocator()
		: dgMemoryAllocator(__malloc__, __free__), dgList<dgMemoryAllocator*> (NULL) {
		SetAllocator(this);
	}

	~dgGlobalAllocator() {
		NEWTON_ASSERT(GetCount() == 0);
	}

	static void *dgApi __malloc__(dgUnsigned32 size) {
		return malloc(size);
	}

	static void dgApi __free__(void *const ptr, dgUnsigned32 size) {
		free(ptr);
	}

	void operator delete (void *const ptr) {
		NEWTON_ASSERT(0);
		//::delete (ptr);
		free(ptr);
	}


	dgInt32 GetMemoryUsed() const {
		dgInt32 mem = m_memoryUsed;
		for (dgList<dgMemoryAllocator *>::dgListNode *node = GetFirst(); node; node = node->GetNext()) {
			mem += node->GetInfo()->GetMemoryUsed();
		}
		return mem;
	}

	static dgGlobalAllocator *m_globalAllocator;
};

dgGlobalAllocator *dgGlobalAllocator::m_globalAllocator = nullptr;

void dgInitMemoryGlobals() {
	dgGlobalAllocator::m_globalAllocator = ::new dgGlobalAllocator();
}

void dgDestroyMemoryGlobals() {
	::delete dgGlobalAllocator::m_globalAllocator;
}


dgMemoryAllocator::dgMemoryAllocator() {
	m_memoryUsed = 0;
	m_emumerator = 0;
	SetAllocatorsCallback(dgGlobalAllocator::m_globalAllocator->m_malloc, dgGlobalAllocator::m_globalAllocator->m_free);
	memset(m_memoryDirectory, 0, sizeof(m_memoryDirectory));
	dgGlobalAllocator::m_globalAllocator->Append(this);
}

dgMemoryAllocator::dgMemoryAllocator(dgMemAlloc memAlloc, dgMemFree memFree) {
	m_memoryUsed = 0;
	m_emumerator = 0;
	SetAllocatorsCallback(memAlloc, memFree);
	memset(m_memoryDirectory, 0, sizeof(m_memoryDirectory));
}

dgMemoryAllocator::~dgMemoryAllocator() {
	dgGlobalAllocator::m_globalAllocator->Remove(this);
	NEWTON_ASSERT(m_memoryUsed == 0);
}

void *dgMemoryAllocator::operator new (size_t size) {
	return dgMallocStack(size);
}

void dgMemoryAllocator::operator delete (void *const ptr) {
	dgFreeStack(ptr);
}

dgInt32 dgMemoryAllocator::GetMemoryUsed() const {
	return m_memoryUsed;
}

void dgMemoryAllocator::SetAllocatorsCallback(dgMemAlloc memAlloc, dgMemFree memFree) {
	m_free = memFree;
	m_malloc = memAlloc;
}

void *dgMemoryAllocator::MallocLow(dgInt32 workingSize, dgInt32 alignment) {
	NEWTON_ASSERT(alignment >= memoryGranularity);
	NEWTON_ASSERT(((-alignment) & (alignment - 1)) == 0);
	dgInt32 size = workingSize + alignment * 2;
	void *const ptr = m_malloc(dgUnsigned32(size));
	dgUnsigned64 val = dgUnsigned64(PointerToInt(ptr));
	val = (val & dgUnsigned64(-alignment)) + alignment * 2;
	void *const retPtr = IntToPointer(val);

	dgMemoryInfo *const info = ((dgMemoryInfo *)(retPtr)) - 1;
	info->SaveInfo(this, ptr, size, m_emumerator, workingSize);

	dgAtomicAdd(&m_memoryUsed, size);
	return retPtr;
}

void dgMemoryAllocator::FreeLow(void *const retPtr) {
	dgMemoryInfo *info;
	info = ((dgMemoryInfo *)(retPtr)) - 1;
	NEWTON_ASSERT(info->m_allocator == this);

	dgAtomicAdd(&m_memoryUsed, -info->m_size);

#ifdef _DEBUG
	memset(retPtr, 0, info->m_workingSize);
#endif

	m_free(info->m_ptr, dgUnsigned32(info->m_size));
}

// alloca memory on pool that are quantized to memoryGranularity
// if memory size is larger than memoryBinEntries then the memory is not placed into a pool
void *dgMemoryAllocator::Malloc(dgInt32 memsize) {
	NEWTON_ASSERT(dgInt32(sizeof(dgMemoryCacheEntry) + sizeof(dgInt32) + sizeof(dgInt32)) <= memoryGranularity);

	dgInt32 size = memsize + memoryGranularity - 1;
	size &= -memoryGranularity;

	dgInt32 paddedSize = size + memoryGranularity;
	dgInt32 entry = paddedSize / memoryGranularity;

	void *ptr;
	if (entry >= memoryBinEntries) {
		ptr = MallocLow(size);
	} else {
		if (!m_memoryDirectory[entry].m_cache) {
			dgMemoryBin *const bin = (dgMemoryBin *) MallocLow(sizeof(dgMemoryBin));

			dgInt32 count = dgInt32(sizeof(bin->m_pool) / paddedSize);
			bin->m_info.m_count = 0;
			bin->m_info.m_totalCount = count;
			bin->m_info.m_stepInBites = paddedSize;
			bin->m_info.m_next = m_memoryDirectory[entry].m_first;
			bin->m_info.m_prev = NULL;
			if (bin->m_info.m_next) {
				bin->m_info.m_next->m_info.m_prev = bin;
			}

			m_memoryDirectory[entry].m_first = bin;

			char *charPtr = bin->m_pool;
			m_memoryDirectory[entry].m_cache = (dgMemoryCacheEntry *) charPtr;

//      charPtr = bin->m_pool
			for (dgInt32 i = 0; i < count; i ++) {
				dgMemoryCacheEntry *const cashe = (dgMemoryCacheEntry *) charPtr;
				cashe->m_next = (dgMemoryCacheEntry *)(charPtr + paddedSize);
				cashe->m_prev = (dgMemoryCacheEntry *)(charPtr - paddedSize);
				dgMemoryInfo *const info = ((dgMemoryInfo *)(charPtr + memoryGranularity)) - 1;
				info->SaveInfo(this, bin, entry, m_emumerator, memsize);
				charPtr += paddedSize;
			}
			dgMemoryCacheEntry *const cashe = (dgMemoryCacheEntry *)(charPtr - paddedSize);
			cashe->m_next = NULL;
			m_memoryDirectory[entry].m_cache->m_prev = NULL;
		}

		NEWTON_ASSERT(m_memoryDirectory[entry].m_cache);

		dgMemoryCacheEntry *const cashe = m_memoryDirectory[entry].m_cache;
		m_memoryDirectory[entry].m_cache = cashe->m_next;
		if (cashe->m_next) {
			cashe->m_next->m_prev = NULL;
		}

		ptr = ((char *) cashe) + memoryGranularity;

		dgMemoryInfo *info;
		info = ((dgMemoryInfo *)(ptr)) - 1;
		NEWTON_ASSERT(info->m_allocator == this);

		dgMemoryBin *const bin = (dgMemoryBin *) info->m_ptr;
		bin->m_info.m_count++;

#ifdef __TRACK_MEMORY_LEAKS__
		m_leaklTracker.InsertBlock(dgInt32(memsize), ptr);
#endif

	}
	return ptr;
}

// allocate memory on pool that are quantized to memoryGranularity
// if memory size is larger than memoryBinEntries then the memory is not placed into a pool
void dgMemoryAllocator::Free(void *const retPtr) {
	dgMemoryInfo *const info = ((dgMemoryInfo *)(retPtr)) - 1;
	NEWTON_ASSERT(info->m_allocator == this);

	dgInt32 entry = info->m_size;

	if (entry >= memoryBinEntries) {
		FreeLow(retPtr);
	} else {
#ifdef __TRACK_MEMORY_LEAKS__
		m_leaklTracker.RemoveBlock(retPtr);
#endif

		dgMemoryCacheEntry *const cashe = (dgMemoryCacheEntry *)(((char *)retPtr) - memoryGranularity) ;

		dgMemoryCacheEntry *const tmpCashe = m_memoryDirectory[entry].m_cache;
		if (tmpCashe) {
			NEWTON_ASSERT(!tmpCashe->m_prev);
			tmpCashe->m_prev = cashe;
		}
		cashe->m_next = tmpCashe;
		cashe->m_prev = NULL;

		m_memoryDirectory[entry].m_cache = cashe;

		dgMemoryBin *const bin = (dgMemoryBin *) info->m_ptr;

#ifdef _DEBUG
		NEWTON_ASSERT((bin->m_info.m_stepInBites - memoryGranularity) > 0);
		memset(retPtr, 0, bin->m_info.m_stepInBites - memoryGranularity);
#endif

		bin->m_info.m_count--;
		if (bin->m_info.m_count == 0) {

			dgInt32 count = bin->m_info.m_totalCount;
			dgInt32 sizeInBytes = bin->m_info.m_stepInBites;
			char *charPtr = bin->m_pool;
			for (dgInt32 i = 0; i < count; i ++) {
				dgMemoryCacheEntry *const tmpCache = (dgMemoryCacheEntry *) charPtr;
				charPtr += sizeInBytes;

				if (tmpCache == m_memoryDirectory[entry].m_cache) {
					m_memoryDirectory[entry].m_cache = tmpCache->m_next;
				}

				if (tmpCache->m_prev) {
					tmpCache->m_prev->m_next = tmpCache->m_next;
				}

				if (tmpCache->m_next) {
					tmpCache->m_next->m_prev = tmpCache->m_prev;
				}
			}

			if (m_memoryDirectory[entry].m_first == bin) {
				m_memoryDirectory[entry].m_first = bin->m_info.m_next;
			}
			if (bin->m_info.m_next) {
				bin->m_info.m_next->m_info.m_prev = bin->m_info.m_prev;
			}
			if (bin->m_info.m_prev) {
				bin->m_info.m_prev->m_info.m_next = bin->m_info.m_next;
			}

			FreeLow(bin);
		}
	}
}

// this is a simple memory leak tracker, it uses an flat array of two megabyte indexed by a hatch code
#ifdef __TRACK_MEMORY_LEAKS__

dgMemoryAllocator::dgMemoryLeaksTracker::dgMemoryLeaksTracker() {
	m_density = 0;
	m_totalAllocatedBytes = 0;
	m_totalAllocatedCalls = 0;
	m_leakAllocationCounter = 0;

	memset(m_pool, 0, sizeof(m_pool));
}

dgMemoryAllocator::dgMemoryLeaksTracker::~dgMemoryLeaksTracker() {
//    #ifdef _WIN32
//    _CrtDumpMemoryLeaks();
//    #endif

	if (m_totalAllocatedBytes) {
		for (dgInt32 i = 0; i < DG_TRACK_MEMORY_LEAKS_ENTRIES; i ++) {
			if (m_pool[i].m_ptr) {
				dgTrace(("MemoryLeak: (0x%08x), size (%d)  allocationNumber (%d)\n", m_pool[i].m_ptr, m_pool[i].m_size, m_pool[i].m_allocationNumber));
			}
		}
	}
}

void dgMemoryAllocator::dgMemoryLeaksTracker::InsertBlock(dgInt32 size, void *const ptr) {
	dgInt32 i;
	dgUnsigned32 key;
	dgUnsigned32 index;

	//NEWTON_ASSERT (ptr != (void*)0x04cf8080);
	//NEWTON_ASSERT (ptr != (void*)0x04d38080);
	//NEWTON_ASSERT (ptr != (void*)0x04f48080);
	//NEWTON_ASSERT (ptr != (void*)0x04f78080);
	//if (m_leakAllocationCounter >= 2080840)
	//m_leakAllocationCounter *=1;

	key = dgHash(&ptr, sizeof(void *));
	index = key % DG_TRACK_MEMORY_LEAKS_ENTRIES;

	for (i = 0; m_pool[index].m_ptr && (i < DG_TRACK_MEMORY_LEAKS_ENTRIES); i ++) {
		index = ((index + 1) < DG_TRACK_MEMORY_LEAKS_ENTRIES) ? index + 1 : 0;
	}

	NEWTON_ASSERT(i < 8);
	NEWTON_ASSERT(i < DG_TRACK_MEMORY_LEAKS_ENTRIES);

	m_density ++;
	m_pool[index].m_size = size;
	m_pool[index].m_ptr = ptr;
	m_pool[index].m_allocationNumber = m_leakAllocationCounter;

	m_leakAllocationCounter ++;
	m_totalAllocatedBytes += size;
	m_totalAllocatedCalls ++;
}

void dgMemoryAllocator::dgMemoryLeaksTracker::RemoveBlock(void *const ptr) {
	dgInt32 i;
	dgUnsigned32 key;
	dgUnsigned32 index;

	key = dgHash(&ptr, sizeof(void *));
	index = key % DG_TRACK_MEMORY_LEAKS_ENTRIES;

	for (i = 0; i < DG_TRACK_MEMORY_LEAKS_ENTRIES; i ++) {
		if (m_pool[index].m_ptr == ptr) {
			m_density --;
			m_totalAllocatedCalls--;
			m_totalAllocatedBytes -= m_pool[index].m_size;
			NEWTON_ASSERT(m_totalAllocatedBytes >= 0);
			m_pool[index].m_size = 0;
			m_pool[index].m_ptr = NULL;
			break;
		}
		index = ((index + 1) < DG_TRACK_MEMORY_LEAKS_ENTRIES) ? index + 1 : 0;
	}

	NEWTON_ASSERT(i < DG_TRACK_MEMORY_LEAKS_ENTRIES);
}
#endif

// Set the pointer of memory allocation functions
void dgSetGlobalAllocators(dgMemAlloc malloc, dgMemFree free) {
	dgGlobalAllocator::m_globalAllocator->SetAllocatorsCallback(malloc, free);
}

dgInt32 dgGetMemoryUsed() {
	return dgGlobalAllocator::m_globalAllocator->GetMemoryUsed();
}

// this can be used by function that allocates large memory pools memory locally on the stack
// this by pases the pool allocation because this should only be used for very large memory blocks.
// this was using virtual memory on windows but
// but because of many complaint I changed it to use malloc and free
void *dgApi dgMallocStack(size_t size) {
	return dgGlobalAllocator::m_globalAllocator->MallocLow(dgInt32(size));
}

void *dgApi dgMallocAligned(size_t size, dgInt32 align) {
	return dgGlobalAllocator::m_globalAllocator->MallocLow(dgInt32(size), align);
}

// this can be used by function that allocates large memory pools memory locally on the stack
// this by pases the pool allocation because this should only be used for very large memory blocks.
// this was using virtual memory on windows but
// but because of many complaint I changed it to use malloc and free
void  dgApi dgFreeStack(void *const ptr) {
	dgGlobalAllocator::m_globalAllocator->FreeLow(ptr);
}

// general memory allocation for all data in the library
void *dgApi dgMalloc(size_t size, dgMemoryAllocator *const allocator) {
	void *ptr = NULL;
	NEWTON_ASSERT(allocator);
	if (size) {
		ptr = allocator->Malloc(dgInt32(size));
	}
	return ptr;
}

// general deletion allocation for all data in the library
void dgApi dgFree(void *const ptr) {
	if (ptr) {
		dgMemoryAllocator::dgMemoryInfo *info;
		info = ((dgMemoryAllocator::dgMemoryInfo *) ptr) - 1;
		NEWTON_ASSERT(info->m_allocator);
		info->m_allocator->Free(ptr);
	}
}
