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

#ifndef __dgMemory__
#define __dgMemory__

#include "dgStdafx.h"

#ifdef _DEBUG
//#define __TRACK_MEMORY_LEAKS__
#endif

class dgMemoryAllocator;

void dgInitMemoryGlobals();

void dgDestroyMemoryGlobals();

void *dgApi dgMalloc(size_t size, dgMemoryAllocator *const allocator);
void  dgApi dgFree(void *const ptr);


void *dgApi dgMallocStack(size_t size);
void *dgApi dgMallocAligned(size_t size, dgInt32 alignmentInBytes);
void  dgApi dgFreeStack(void *const ptr);

typedef void *(dgApi *dgMemAlloc)(dgUnsigned32 size);
typedef void (dgApi *dgMemFree)(void *const ptr, dgUnsigned32 size);


typedef void (dgApi *dgSerialize)(void *const userData, const void *const buffer, size_t size);
typedef void (dgApi *dgDeserialize)(void *const userData, void *buffer, size_t size);

//void dgSetMemoryDrivers (dgMemAlloc alloc, dgMemFree free);
void dgSetGlobalAllocators(dgMemAlloc alloc, dgMemFree free);
dgInt32 dgGetMemoryUsed();


#define DG_CLASS_ALLOCATOR_NEW(allocator)           inline void *operator new (size_t size, dgMemoryAllocator* const allocator) { return dgMalloc(size, allocator);}
#define DG_CLASS_ALLOCATOR_NEW_ARRAY(allocator)     inline void *operator new[] (size_t size, dgMemoryAllocator* const allocator) { return dgMalloc(size, allocator);}
#define DG_CLASS_ALLOCATOR_DELETE(allocator)        inline void operator delete (void* const ptr, dgMemoryAllocator* const allocator) { dgFree(ptr); }
#define DG_CLASS_ALLOCATOR_DELETE_ARRAY(allocator)  inline void operator delete[] (void* const ptr, dgMemoryAllocator* const allocator) { dgFree(ptr); }
#define DG_CLASS_ALLOCATOR_NEW_DUMMY                inline void *operator new (size_t size) { NEWTON_ASSERT (0); return dgMalloc(size, NULL);}
#define DG_CLASS_ALLOCATOR_NEW_ARRAY_DUMMY          inline void *operator new[] (size_t size) { NEWTON_ASSERT (0); return dgMalloc(size, NULL);}
#define DG_CLASS_ALLOCATOR_DELETE_DUMMY             inline void operator delete (void* const ptr) { dgFree(ptr); }
#define DG_CLASS_ALLOCATOR_DELETE_ARRAY_DUMMY       inline void operator delete[] (void* const ptr) { dgFree(ptr); }


#define DG_CLASS_ALLOCATOR(allocator)               \
	DG_CLASS_ALLOCATOR_DELETE(allocator)            \
	DG_CLASS_ALLOCATOR_DELETE_ARRAY(allocator)      \
	DG_CLASS_ALLOCATOR_NEW(allocator)               \
	DG_CLASS_ALLOCATOR_NEW_ARRAY(allocator)         \
	DG_CLASS_ALLOCATOR_NEW_DUMMY                    \
	DG_CLASS_ALLOCATOR_NEW_ARRAY_DUMMY              \
	DG_CLASS_ALLOCATOR_DELETE_DUMMY                 \
	DG_CLASS_ALLOCATOR_DELETE_ARRAY_DUMMY




class dgMemoryAllocator {
	static constexpr dgInt32 memoryGranularity = sizeof(void*) * 8;
	static constexpr dgInt32 memorySize = 1024 - 64;
	static constexpr dgInt32 memoryBinSize = 1024 * 16;
	static constexpr dgInt32 memoryBinEntries = memorySize / memoryGranularity;
public:

	class dgMemoryBin {
	public:
		class dgMemoryBinInfo {
		public:
			dgInt32 m_count;
			dgInt32 m_totalCount;
			dgInt32 m_stepInBites;
			dgMemoryBin *m_next;
			dgMemoryBin *m_prev;
		};

		char m_pool[memoryBinSize - sizeof(dgMemoryBinInfo) - memoryGranularity * 2];
		dgMemoryBinInfo m_info;
	};


	class dgMemoryCacheEntry {
	public:
		dgMemoryCacheEntry *m_next;
		dgMemoryCacheEntry *m_prev;
	};

	class dgMemoryInfo {
	public:
		void *m_ptr;
		dgMemoryAllocator *m_allocator;
		dgInt32 m_size;
		dgInt32 m_enum;
#ifdef _DEBUG
		dgInt32 m_workingSize;
#endif

		DG_INLINE void SaveInfo(dgMemoryAllocator *const allocator, void *const ptr, dgInt32 size, dgInt32 &enumerator, dgInt32 workingSize = 0) {
			m_ptr = ptr;
			m_size = size;
			m_enum = enumerator;
			enumerator ++;
			m_allocator = allocator;
#ifdef _DEBUG
			m_workingSize = workingSize;
#endif
		}
	};

	class dgMemDirectory {
	public:
		dgMemoryBin *m_first;
		dgMemoryCacheEntry *m_cache;
	};


	// this is a simple memory leak tracker, it uses an flat array of two megabyte indexed by a hatch code
#ifdef __TRACK_MEMORY_LEAKS__
	class dgMemoryLeaksTracker {
#define DG_TRACK_MEMORY_LEAKS_ENTRIES (1024 * 1024 * 4)
		class Pool {
		public:
			void *m_ptr;
			dgInt32 m_size;
			dgInt32 m_allocationNumber;
		};

	public:
		dgMemoryLeaksTracker();
		~dgMemoryLeaksTracker();
		void RemoveBlock(void *const ptr);
		void InsertBlock(dgInt32 size, void *const ptr);

		dgInt32 m_density;
		dgInt32 m_totalAllocatedBytes;
		dgInt32 m_totalAllocatedCalls;
		dgInt32 m_leakAllocationCounter;
		Pool m_pool[DG_TRACK_MEMORY_LEAKS_ENTRIES];

	};
#endif

	dgMemoryAllocator();
	~dgMemoryAllocator();
	void *operator new (size_t size);
	void operator delete (void *const ptr);
	dgInt32 GetMemoryUsed() const;
	void SetAllocatorsCallback(dgMemAlloc memAlloc, dgMemFree memFree);
	void *MallocLow(dgInt32 size, dgInt32 alignment = memoryGranularity);
	void FreeLow(void *const retPtr);
	void *Malloc(dgInt32 memsize);
	void Free(void *const retPtr);


protected:
	dgMemoryAllocator(dgMemAlloc memAlloc, dgMemFree memFree);

	dgInt32 m_emumerator;
	dgInt32 m_memoryUsed;
	dgMemFree m_free;
	dgMemAlloc m_malloc;
	dgMemDirectory m_memoryDirectory[memoryBinEntries + 1];

#ifdef __TRACK_MEMORY_LEAKS__
	dgMemoryLeaksTracker m_leaklTracker;
#endif
};


#endif

