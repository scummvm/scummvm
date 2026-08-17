#pragma once

#include <os_types.h>
#include <os_lib.h>

// ----------------------------------------------------------------------

#ifndef DEBUG_ALLOCS
#define DEBUG_ALLOCS 0
#endif

#if DEBUG_ALLOCS

	extern void*  AllocateBlock      		 (const char* reason, size_t bytes, bool zero = true);
	extern void*  AllocateBlockInPool		 (const char* reason, size_t bytes, bool zero, OSMemoryPool pool);
	extern void   Free         	  			 (void* block);
	extern void   DumpMemory     			 (uint32_t totalFree = 0, uint32_t largestBlock = 0, uint32_t stackUsed = 0, uint32_t stackTotal = 0);
	extern size_t GetMaxAllocatableBlockSize ();

#else

	static inline void* AllocateBlockInPool(const char* reason, size_t bytes, bool zero, OSMemoryPool pool)
	{
		(void)reason;
		bytes = (bytes + 3) & ~3;
		void* r = OSAlloc(bytes, pool);
		if (r && zero) 
			MemClear(r, bytes);
		return r;
	}

	static inline void* AllocateBlock(const char* reason, size_t bytes, bool zero = true)
	{
		return AllocateBlockInPool(reason, bytes, zero, OSMemoryPool_Any);
	}

	static inline void Free(void* block)
	{
		OSFree(block);
	}

	static inline size_t GetMaxAllocatableBlockSize()
	{
		size_t free = OSGetFree();
		return free & ~(size_t)15;
	}

#endif

template <class T>
T* Allocate(const char* reason, unsigned count = 1, bool zero = true)
{
	return (T*) AllocateBlock(reason, sizeof(T) * count, zero);
}

template <class T>
T* AllocateInPool(const char* reason, OSMemoryPool pool, unsigned count = 1, bool zero = true)
{
	return (T*) AllocateBlockInPool(reason, sizeof(T) * count, zero, pool);
}

// ----------------------------------------------------------------------

struct Arena
{
	const char* name;
	uint8_t*    block;
	size_t      size;
	size_t      used;
	Arena*      current;
	Arena*      next;
};

extern Arena* AllocateArena (const char* reason);
extern void*  AllocateBlock (Arena* arena, size_t bytes, bool zero = true);
extern void   FreeArena     (Arena* arena);

template <class T>
T* Allocate(Arena* arena, unsigned count = 1, bool zero = true)
{
	return (T*) AllocateBlock(arena, sizeof(T) * count, zero);
}
