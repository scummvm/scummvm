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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"

#define NO_LOCAL_MEMORY_OVERRIDES 1
#include "saga2/rmembase.h"
#include "saga2/errors.h"

namespace Saga2 {

#define VMM_lock(p,s) LocalLock(p)
#define VMM_unlock(p,s) LocalUnlock(p)

/* ===================================================================== */
/* External Dependencies                                                 */
/*   System calls                                                        */
/*     assert(bool);                                                     */
/*     malloc(size_t);                                                   */
/*     free(void*);                                                      */
/*   DGI Library calls                                                   */
/*     STD.H                                                             */
/*   Others                                                              */
/*     VMM_Lock() or LocalLock() (DOS/WIN)                               */
/*     VMM_Unlock() or LocalUnlock() (DOS/WIN)                           */
/*     initRMemLogging() (in RMEMLOG.CPP)                                */
/*                                                                       */
/* ===================================================================== */

#define SysAlloc    malloc
#define SysFree( m,s ) free( m )

/* ===================================================================== *
   Debugging variables which control the heap integrity checking
     NOTE: If you want to change these settings you should do so
           in RMEM.H, not here.
 * ===================================================================== */

// most of the checks are only enabled in the DEBUG version
#if DEBUG
#  define WARN_LEAKS RMEM_WARN_LEAKS

#  define WIPE_ALLOC RMEM_WIPE_ALLOC
#  define WIPE_FREE RMEM_WIPE_FREE

#  define wallSize  RMEM_WALL_SIZE                      // memory wall size around allocs
#  define mungEnable    RMEM_MUNG_ENABLE                        // enable munging of free blocks
#  define prefixOffset (sizeof( RMemPrefix ) + wallSize)

void checkWalls(uint8 *begin, uint32 length);
void setWalls(uint8 *begin, uint32 length);
#else
#  define WARN_LEAKS 0

#  define WIPE_ALLOC FALSE
#  define WIPE_FREE FALSE

#  define wallSize  0                       // compile wall size as zero
#  define mungEnable    0                       // disable munging
#  define prefixOffset sizeof( RMemPrefix )
//  If debugging off, checkwalls is an inline function which does nothing.
#  define checkWalls( begin, length ) ((void)0)
#  define setWalls( begin, length ) ((void)0)
#endif

/* ===================================================================== *
   Debugging variables which control the "visible memory" display
 * ===================================================================== */

#if RMEM_VISIBLE

void RShowMem(void);
#define SHOWMEM RShowMem();

#else

#define SHOWMEM ((void)0)

#endif

/* ===================================================================== *
   Global variables
 * ===================================================================== */

static RHeapPtr     heapList = NULL;            // pointer to heap chain

/* ===================================================================== *
   Debugging variables which control the heap integrity checking
 * ===================================================================== */

#define QUANTIZE(x) ((x) + 7 & ~7)          // quantize to multiple of 8

//  These should be provided by App to call malloc() or whatever

/* ============================================================================ *
    Prototypes
 * ============================================================================ */

static void FreeHandle(RHeap *heap, RHandle *handle);
static void *NewPtr(int32 size, RHeapPtr heap, bool high, const char []);
#if DEBUG
static bool _RMemCleanHeap(RHeapPtr heap);
#endif
static RHeapPtr whichHeap(void *ptr);
void RDumpMem(RHeapPtr, int);

typedef void *MEMERRHANDLER(size_t s, const char d[]);
typedef MEMERRHANDLER *pMEMERRHANDLER;

static pMEMERRHANDLER onError = NULL;

pMEMERRHANDLER RMemSetFailHandler(pMEMERRHANDLER nh) {
	pMEMERRHANDLER oh = onError;
	onError = nh;
	return oh;
}

/* ============================================================================ *
    General internal allocation & free routines
 * ============================================================================ */

static void *rmemAlloc(size_t s, const char desc[]) {
	void *r = NULL;
	if (heapList != NULL)
		r = _RNewPtr(s, NULL, desc);
	else
		r = SysAlloc(s);
	if (r == NULL && onError != NULL)
		r = (*onError)(s, desc);
	return r;
}


//quick dealloc

static bool complicatedFree = TRUE;

static void rmemFree(void *mem) {
	if (whichHeap(mem) != NULL) {
		if (complicatedFree)
			RDisposePtr(mem);
	} else
		SysFree(mem, RPtrSize(mem));
}

void RMemFastCleanup(void) {
#if !DEBUG
	complicatedFree = FALSE;
#endif
}

/* ============================================================================ *
    These calls are used to override malloc and free
 * ============================================================================ */

#if DEBUG

void *localMalloc(size_t s, const int l, const char f[]) {
	char tBuf[256];
	sprintf(tBuf, "%d %s", l, f);
	return rmemAlloc(s, tBuf);
}

#else

void *localMalloc(size_t s, const int, const char []) {
	return rmemAlloc(s, "malloc");
}

#endif

void *namedMalloc(size_t s, const char d[]) {
	return rmemAlloc(s, d);
}

void localFree(void *mem) {
	rmemFree(mem);
}

/* ===================================================================== *
   VMM Locking code
 * ===================================================================== */

void VMM_Lock_Mem(void *ptr, size_t size) {
	warning("STUB: VMM_Lock_Mem()");
#if 0
	if (!VMM_lock(ptr, size))
		error("VMM Locking failed");
#endif
}

void VMM_Unlock_Mem(void *ptr, size_t size) {
	warning("STUB: VMM_Unlock_mem");
#if 0
	if (!VMM_unlock(ptr, size))
		error("VMM Locking failed");
#endif
}

/* ============================================================================ *
    Heap management
 * ============================================================================ */

//  RNewHeap: create a new heap
//      Arguments:

RHeapPtr _RNewHeap(uint8 *memory, int32 size) {
	RHeapPtr            heap;               // pointer to new heap
#if DEBUG
	initRMemLogging();
#endif

	heap = (RHeapPtr) SysAlloc(sizeof * heap);
	if (heap == NULL)
		error("Memory allocation (%ld bytes) failed", sizeof * heap);

	heap->next = heapList;
	heapList = heap;

	memset(heap, 0, sizeof * heap);

	//  Create empty circular list of cached blocks
	heap->cacheHead = heap->cacheTail = (RHandle *)&heap->cacheHead;

	heap->size = size;                      // size of memory buffer
	heap->free = size;                      // free space in heap
	heap->memory = memory;                  // pointer to raw memory
	heap->firstFree = (RMemFreeBlock *)memory; // first free block in buffer
	heap->firstFree->next = NULL;           // initialize free block to whole blk
	heap->firstFree->size = heap->size;
	SHOWMEM;
	return heap;                            // return heap we created
}

//  RDisposeHeap: Gets rid of a heap.
//  Note the actual heap memory isn't freed since we didn't allocate it

void _RDisposeHeap(RHeapPtr heap) {
	RHeap           **prev;                     // indirect pointer to this heap

#if WARN_LEAKS
	if (complicatedFree)
		_RMemCleanHeap(heap);
#endif

	//  search heap list, find pointer to the heap we want
	for (prev = &heapList; *prev; prev = &(*prev)->next) {
		if (*prev == heap) {                    // if we find it
			*prev = heap->next;                 // delink
			SysFree(heap, sizeof * heap);        // and free
			heap = NULL;
			break;
		}
	}
}

//  whichHeap: Determines which heap a particular pointer belongs to.

static RHeapPtr whichHeap(void *ptr) {
	RHeapPtr        h;

	// run through the list of heaps, and figure out which one the pointer is in

	for (h = heapList; h; h = h->next) {    // for each heap
		// if ptr is within range of addresses
		if ((uint8 *)ptr >= (uint8 *)h->memory
		        && (uint8 *)ptr < (uint8 *)h->memory + h->size) {
			return h;
		}
	}
	return NULL;
}

RHeapPtr getHeapList(void) {
	return heapList;
}

/* ============================================================================ *
    Low-level memory free routines
 * ============================================================================ */

bool IsCacheable(RMemPrefix *pr) {
	return (pr->flags & rMemPurgeable);
}

bool IsVMMLocked(RMemPrefix *pr) {
	return (pr->flags & rMemVMMLocked);
}

bool IsMovable(RMemPrefix *pr) {
	return (bool)(pr->handle && pr->lockNestCount == 0);
}

//  Low-level routine to merge adjacent free blocks

static void MergeBlock(RHeapPtr heap, RMemFreeBlock *blk) {
	//assert(whichHeap(blk)==heap);
	if (blk != (RMemFreeBlock *)&heap->firstFree && blk->next != NULL) {
		//  If the end of the block is equal to the beginning of
		//  the next block, then merge the blocks into one.

		if ((uint8 *)blk + blk->size == (uint8 *)blk->next) {
			blk->size += blk->next->size;   // expand 1st block
			blk->next = blk->next->next;    // remove 2nd block from list
		}
	}
}

//  Free a block of memory from a particular heap.
//  It returns the "merged" size of the free block, so that the
//  purging algorithm can determine if it has done enough work.

static uint32 FreeBlockHeap(
    uint8               *mem,
    uint32              size,
    RHeapPtr            heap) {
	RMemFreeBlock       *blk,
	                    *nblk,
	                    *prev;

//	assert(whichHeap(mem)==heap);
//	assert(whichHeap(mem+size)==heap);

	size = QUANTIZE(size);               // quantize the size

	//  We always keep the list of free blocks in order by address;
	//  So search through the list and find where to insert the new block

	prev = (RMemFreeBlock *)&heap->firstFree;
	for (blk = prev->next; blk; prev = blk, blk = blk->next) {
		if ((uint8 *)blk > mem) {           // next free block is after mem
#if 0
			//  This is commented out because, once again, I feel like the code
			//  is compacting just for the sake of moving stuff around, rather than
			//  any real need for it at this time. My intent is to go with the
			//  minimum possible compaction; I will only compact when I need to,
			//  and even then only when it will work.

			//  This does a simplistic compaction of movable blocks.
			//  It attempts to put the newly freed memory as high
			//  as possible by moving other blocks around.

			//  No point in doing this, however, if the newly freed
			//  block will end up merging with the previous block

			if ((uint8 *)prev + prev->size != mem) {
				//  For each movable allocation which comes before
				//  the next free block, move it down into the
				//  newly freed area, and move the free area up in
				//  exchange.

				while (mem + size < (uint8 *)blk) {
					RMemPrefix *pr = (RMemPrefix *)(mem + size)

					if (IsMovable(pr)) {
						RMemPrefix *new_pr = (RMemPrefix *)mem;
						// move block data down
						memcpy(new_pr, pr, pr->size);
						// fix up handle
						new_pr->handle->data = (uint8 *)new_pr + prefixOffset;
						// move free area up
						mem += QUANTIZE(new_pr->size);
					} else break;
				}
			}
#endif

			nblk = (struct RMemFreeBlock *)mem; // make a new free block
			nblk->size = size;              // initialize it
			nblk->next = blk;
			prev->next = nblk;
			heap->free += size;

			MergeBlock(heap, nblk);          // try merge with previous block
			MergeBlock(heap, prev);          // try merge with next block

			//  return the size of this block after merging,
			//  unless it got subsumed into the previous block
			//  in which case return the size of the previous block.

			if (prev->next == nblk) return nblk->size;
			else return prev->size;
		}
	}

	nblk = (struct RMemFreeBlock *)mem;     // make new free block at heaps end
	nblk->size = size;                      // initialize it
	nblk->next = NULL;
	prev->next = nblk;
	heap->free += size;

	MergeBlock(heap, prev);                  // try merge with previous block

	//  return the size of this block after merging,
	//  unless it got subsumed into the previous block
	//  in which case return the size of the previous block.

	if (prev->next == nblk) return nblk->size;
	else return prev->size;
}

//  RDisposePtrHeap: free memory pointer to by the pointer

static void DisposePtrHeap(void *ptr, RHeapPtr heap) {
#if DEBUG
#if WIPE_FREE
	uint8 *cptr;
#endif
	if (whichHeap(ptr) == NULL) {
		error("Cannot dispose pointers in global heap");
	}
	if (heap != whichHeap(ptr)) {
		error("bad heap specified on dispose");
	}
#endif
	if (ptr) {                              // if pointer is non-null
		// get memory prefix address
		RMemPrefix      *pr = PrefixBaseAddr(ptr);
		// debugging check
#if DEBUG
		if (pr->size < sizeof * pr + wallSize * 2 ||
		        pr->magic != RMEM_MAGIC_ID) {
			if (pr->magic == RMEM_FREED_ID)
				error("Double deletion detected");
			else if (pr->magic == RMEM_LEAK_ID) {
				char tDesc[20];
				strncpy(tDesc, pr->description, 16);
				tDesc[16] = '\0';
				warning("Block '%s' deleted after heap cleanup.", tDesc);
			} else
				error("Invalid Memory Prefix");
		} else {
			pr->magic = RMEM_FREED_ID;
			strncpy(pr->description, "freed", 16);
		}
#if WIPE_FREE
		cptr = (uint8 *) ptr;
		memset(cptr, RMEM_FREE_WIPE_ID, pr->size - (sizeof * pr + wallSize * 2));        // clear prefix to zero
#endif
#endif

		//checkWalls( (uint8 *)(pr + 1), pr->size - sizeof *pr );
		FreeBlockHeap((uint8 *)pr, pr->size, heap);
	}
}

#if DEBUG
void _RCheckPtr(void *ptr) {
	if (ptr) {                              // if pointer is non-null
		// get memory prefix address
		RMemPrefix      *pr;
		if (whichHeap(ptr) == NULL) {
			error("Cannot check pointers in global heap");
		}
		pr = PrefixBaseAddr(ptr);
		// debugging check
		if (pr->size < sizeof * pr + wallSize * 2 ||
		        pr->magic != RMEM_MAGIC_ID) {
			error("Invalid Memory Prefix");
		}
		//checkWalls( (uint8 *)(pr + 1), pr->size - sizeof *pr );
	}
}
#else
void _RCheckPtr(void *) {}
#endif

/* ============================================================================ *
    Low-level memory allocation
 * ============================================================================ */

//  RAllocBlockHeap: Allocates a raw block of memory from the heap,
//  removing it from one of the free blocks.

#ifdef __WATCOMC__
#pragma off (unreferenced) ;
#endif

static void *RAllocBlockHeap(RHeapPtr heap, int32 size, bool high, const char desc[]) {
	RMemFreeBlock       *blk, *nblk, *prev, *lastBlk = NULL;
	uint32              qSize;
	RMemPrefix          *pr;
#if WIPE_ALLOC
	uint8 *ptr;
#endif

//	assert(heap);

	qSize = QUANTIZE(size);

	if (qSize > heap->free) return NULL;

	prev = (RMemFreeBlock *)&heap->firstFree;

	for (blk = prev->next; blk; prev = blk, blk = blk->next) {
		if (blk->size >= qSize) {
			lastBlk = prev;
			if (!high) break;
		}
	}

	if (lastBlk == NULL) return NULL;

	prev = lastBlk;
	blk = prev->next;

	if (blk->size == qSize) {
		prev->next = blk->next;
		pr = (RMemPrefix *)blk;
	} else {
		if (high) {
			blk->size -= qSize;
			pr = (RMemPrefix *)((uint8 *)blk + blk->size);
		} else {
			nblk = (struct RMemFreeBlock *)((char *)blk + qSize);
			nblk->size = blk->size - qSize;
			nblk->next = blk->next;
			prev->next = nblk;

			pr = (RMemPrefix *)blk;
		}
	}

	memset(pr, 0, sizeof * pr);          // clear prefix to zero
#if DEBUG
	pr->magic = RMEM_MAGIC_ID;
	if (high) {
		strncpy(pr->description, desc, 14);
		pr->description[14] = '\0';
		strcat(pr->description, "->");
	} else {
		strncpy(pr->description, desc, 16);
	}
#if WIPE_ALLOC
	ptr = (uint8 *)pr + prefixOffset;
	memset(ptr, RMEM_ALLOC_WIPE_ID, size - (sizeof * pr + wallSize * 2));        // clear prefix to zero
#endif
#endif
	pr->size = size;                        // size of the block
	heap->free -= qSize;                    // subtract size of block
	// return pointer to real data
	return (uint8 *)pr + prefixOffset;
}

#ifdef __WATCOMC__
#pragma on (unreferenced) ;
#endif

/* ============================================================================ *
    Heap Compaction & block Purging
 * ============================================================================ */

//  This returns the size of the largest free block

static int32 LargestFree(RHeapPtr heap) {
	RMemFreeBlock   *blk;
	int32           largest = 0;

	assert(heap);

	for (blk = heap->firstFree; blk; blk = blk->next) {
		largest = MAX<uint>(blk->size, largest);
	}
	return largest;
}

//  This returns the size of the largest contiguous free block which
//  could be produced by simplistic compaction (The type of compaction
//  which does not change the ordering of allocated blocks in the heap)

static int32 LargestMerged(RHeapPtr heap) {
	RMemFreeBlock   *blk;
	int32           largest = 0;
	uint8           *nextAlloc,         // address of next allocation
	                *nextFree;          // start of next free block
	uint32          freeSize = 0;       // amount of coalesced free space

//	assert(heap);

	nextAlloc = heap->memory;
	nextFree = (uint8 *)(heap->firstFree);
	blk = heap->firstFree;

	//  compact all blocks which come before the next immovable block.

	while (blk) {
		if (nextAlloc < nextFree) {     // if alloc is before next free blk
			RMemPrefix  *pr = (RMemPrefix *)nextAlloc;

			if (!IsMovable(pr)) {
				largest = MAX<uint>(freeSize, largest);
				freeSize = 0;
			}
			nextAlloc += QUANTIZE(pr->size);
		} else {
			freeSize += blk->size;      // add to count of free bytes
			// compute addr of next allocation
			nextAlloc = nextFree + blk->size;
			blk = blk->next;            // get address of next free block
			nextFree = blk ? (uint8 *)blk : heap->memory + heap->size;
		}
	}
	return largest;
}

//  Attempts to defragment the heap by moving blocks around.
//  Problem with this routine: I feel like the code is moving
//  blocks around just for the sake of moving them around, and
//  not doing anything particularly useful.

static void CompactHeap(RHeapPtr heap, uint32 needed) {
	RMemFreeBlock   *prev, *blk, *nblk /* , *sblk */ ;
	uint8           *lastFixed = heap->memory;

	prev = (RMemFreeBlock *)&heap->firstFree;

	//  compact all blocks which come before the next immovable block.

	while (prev->next) {
		uint32      freeSize = 0;           // amount of coalesced free space
		uint8       *nextAlloc,             // address of next allocation
		            *putAlloc,              // where to copy allocation to
		            *nextFree;              // start of next free block

		blk = prev->next;
		nextAlloc = putAlloc = lastFixed;
		nextFree = (uint8 *)blk;

		while (freeSize < needed) {
			if (nextAlloc < nextFree) {     // if alloc is before next free blk
				RMemPrefix  *pr = (RMemPrefix *)nextAlloc;

				if (!IsMovable(pr)) break;

				if (putAlloc < nextAlloc) { // if there are holes to fill
					RMemPrefix *new_pr = (RMemPrefix *)putAlloc;
					// move block data down
					memcpy(new_pr, pr, pr->size);
					// fix up handle
					new_pr->handle->data = (uint8 *)new_pr + prefixOffset;
					pr = new_pr;
				}
				// point to next alloc
				nextAlloc += QUANTIZE(pr->size);
				putAlloc += QUANTIZE(pr->size);
			} else {
				if (blk == NULL) break;     // if at end of memory, break

				freeSize += blk->size;      // add to count of free bytes
				// compute addr of next allocation
				nextAlloc = nextFree + blk->size;
				blk = blk->next;            // get address of next free block
				prev->next = blk;           // repair chain from deleted blocks
				nextFree = blk ? (uint8 *)blk : heap->memory + heap->size;
			}
		}

		//  At this point, we ran into an immovable block.
		//  Now, search for other, later blocks which might
		//  fit in the remaining space.

		//  The variable 'blk' at this point should be
		//  pointing to the next free block after the immovable
		//  block.

#if 0
		if (blk && freeSize < needed) {     // if we're not at the end
			laterAlloc = nextAlloc;         // address of a later allocation
			sblk = blk;                     // address of a later free block
			nextFree = (uint8 *)sblk;       // next free space

			while (laterAlloc < nextFree       // search each block
			        && freeSize > 0) {       // for something to fill hole with
				int32   qSize;

				//  We now have the address of a memory prefix.
				pr = (RMemPrefix *)laterAlloc;
				qSize = QUANTIZE(pr->size);

#if DEBUG
				//  Check to see if size field is too large or too small
				if (pr->size > next_free - alloc
				        || alloc < sizeof * pr + wallSize * 2) {
					error("Invalid Memory prefix");
				}

				//  Check memory wall integrity
				//checkWalls( alloc + sizeof *pr, pr->size - sizeof *pr );
#endif
				if (isMovable(pr) && qSize <= freeSize) {
					memcpy(putAlloc, (uint8 *)pr, qSize);
					pr->handle->data = putAlloc + prefixOffset;
					freeSize -= qSize;
					putAlloc += qSize;

					//  Delete the old block
					FreeBlockHeap(laterAlloc, qSize, heap);
				}

				//  Go to the next block
				laterAlloc += qSize;
			}
		}
#endif

		//  Build a free block just before the immovable block.

		if (freeSize > 0) {
			nblk = (RMemFreeBlock *)putAlloc;
			nblk->next = blk;
			nblk->size = freeSize;

			prev->next = nblk;
			prev = nblk;

			if (freeSize >= needed) return;
		}

		if (blk) {
			RMemPrefix  *pr = (RMemPrefix *)nextAlloc;

			lastFixed = nextAlloc + QUANTIZE(pr->size);
		}
	}
}

static bool PurgeBlocks(RHeapPtr heap, uint32 needed) {
	int32               freedUp = heap->free;
	RHandle             *rh, *prevHandle;

	for (rh = heap->cacheTail;
	        rh && rh != (RHandle *)&heap->cacheHead;
	        rh = prevHandle
	    ) {
		RMemPrefix  *pr = PrefixBaseAddr(rh->data);

		prevHandle = rh->prev;

		freedUp += QUANTIZE(pr->size);

		RDisposePtr(rh->data);           // Free the block
		rh->data = NULL;                    // NULL the handle.
		RDelinkHandle(rh);               // delink from chain.

		if (freedUp > needed) return TRUE;
	}
	return FALSE;
}

//  This function attempts to get free up enough space for a new
//  allocation.

static void GetSpace(RHeapPtr heap, uint32 needed) {
	for (;;) {
		//  First, try a simple compaction. If it works, then return;

		if (LargestMerged(heap) >= needed) {
			CompactHeap(heap, needed);
			return;
		}
		// We need more space!

		// REM: try more sophisticated compaction

		// check to see if it worked directly, otherwise try another
		// simple compaction.

		if (LargestMerged(heap) >= needed) {
			CompactHeap(heap, needed);
			return;
		}

		//  See if there are any blocks in the cache. If not, return
		//  (we failed).

		if (heap->cacheTail == (RHandle *)&heap->cacheHead)
			return;

		//  Try deleting some blocks from the cache

		PurgeBlocks(heap, needed);

		//  See if we got what we wanted without compaction.

		if (LargestFree(heap) >= needed) return;

		//  Otherwise, go through the loop again and compact.
	}
}

/* ============================================================================ *
    Higher-level Allocate and Free routines
 * ============================================================================ */

//  RNewPtr: Allocates a new pointer. Also sets the "wall" magic cookie
//  around the allocation.

//  Rem: Instead of a heap pointer, how about a memory type, which
//  can include things like "zeroed"

static void *NewPtr(int32 size, RHeapPtr heap, bool high, const char desc[]) {
	uint8               *mem;
	bool                triedCompact = FALSE;
	int                 tries;

	if (heap == NULL) heap = heapList;

	size += wallSize * 2 + sizeof(RMemPrefix);

	for (tries = 0; tries < 2; tries++) {
		if ((mem = (uint8 *) RAllocBlockHeap(heap, size, high, desc)) != NULL) {
			setWalls(mem - wallSize, size - sizeof(RMemPrefix));
			return mem;
		}
		if (triedCompact) return NULL;

		GetSpace(heap, size);
		triedCompact = TRUE;
	}
	return NULL;
}

void *_RNewPtr(int32 size, RHeapPtr heap, const char desc[]) {
#if RMEM_VISIBLE
	void    *ptr = NewPtr(size, heap, TRUE, desc);
	SHOWMEM;
	return ptr;
#else
	return NewPtr(size, heap, TRUE, desc);
#endif
}

void *_RNewClearPtr(int32 size, RHeapPtr heap, const char desc[]) {
	uint8               *mem;

	if ((mem = (uint8 *) NewPtr(size, heap, TRUE, desc)) != NULL)
		memset(mem, 0, size);

	SHOWMEM;
	return mem;
}

//  RDisposePtr: free memory pointer to by the pointer
void VMM_Lock_Mem(void *ptr, size_t size);
void VMM_Unlock_Mem(void *ptr, size_t size);


void _RVMMLockPtr(void *ptr) {
	if (ptr) {
		RMemPrefix      *pr = PrefixBaseAddr(ptr);
		VMM_Lock_Mem(ptr, _RPtrSize(ptr));
		pr->flags |= rMemVMMLocked;
	}
}

void _RVMMUnlockPtr(void *ptr) {
	if (ptr) {
		RMemPrefix      *pr = PrefixBaseAddr(ptr);
		VMM_Unlock_Mem(ptr, _RPtrSize(ptr));
		pr->flags &= ~rMemVMMLocked;
	}
}

bool _RPtrVMMLocked(void *ptr) {
	if (ptr) {
		RMemPrefix      *pr = PrefixBaseAddr(ptr);
		return (pr->flags & rMemVMMLocked);
	}
	return 0;
}

void _RDisposePtr(void *ptr) {
	RHeapPtr            heap;

	if (ptr                                            // if pointer is non-null
	        && (heap = whichHeap(ptr)) != NULL) {            // and in a heap
		if (_RPtrVMMLocked(ptr)) {
			_RVMMUnlockPtr(ptr);
		}
		DisposePtrHeap(ptr, heap);               // then free from heap
	}
	SHOWMEM;
}

/* ============================================================================ *
    Handle blocks
 * ============================================================================ */

void _RLinkHandle(RHandle *r, RHandle *prev) {
	r->next = prev->next;                   // link handle into chain
	r->prev = prev;
	if (r->next) r->next->prev = r;
	prev->next = r;
}

void _RDelinkHandle(RHandle *r) {
	if (r->prev) r->prev->next = r->next;   //  unlink the handle from it's chain
	if (r->next) r->next->prev = r->prev;

	//  link the handle to itself, so that multiple calls to RDelinkHandle
	//  will not crash.

	r->next = r->prev = r;
}

//  Create a new block of handles

static RHandleBlock *NewHandleBlock(RHeapPtr heap, const char desc[]) {
	RHandleBlock        *hBlock, *prevBlock;
	int                 i;
	// allocate space for handle block
	if ((hBlock = (RHandleBlock *) NewPtr(sizeof * hBlock, heap, TRUE, desc)) != NULL) {
		SHOWMEM;
		memset(hBlock, 0, sizeof * hBlock);

		//  Search through the list of handle blocks until we find
		//  one that points to a block after the address of the
		//  newly allocated block, or we find one that points to
		//  the end of the list. This should result in the HandleBlocks
		//  being placed on the list in address order;

		//  The purpose of this is to bias the use of the handle blocks
		//  so that only blocks near the beginning of memory will be
		//  full most of the time, and blocks in other areas will
		//  tend to be freed. We want handle blocks to concentrate
		//  near the beginning of their memory blocks.

		for (prevBlock = (RHandleBlock *)&heap->handleBlocks;
		        prevBlock->next && prevBlock->next < hBlock;
		        prevBlock = prevBlock->next)
			;

		//  Link the handle block into the chain of blocks

		RLinkHandle((RHandle *)hBlock, (RHandle *)prevBlock);

		hBlock->freeHandleCount = HANDLES_PER_BLOCK;
//		hBlock->freeHandles = NULL;

		//  Initialize each of the handles in the block.

		for (i = 0; i < HANDLES_PER_BLOCK; i++) { // link each handle into chain
			RHandle *rh = &hBlock->handles[ i ];

//			rh->data = NULL;
			RLinkHandle(rh, (RHandle *)&hBlock->freeHandles);
		}
	}
	return hBlock;
}

//  Find an unused handle so that we can use it.

static RHandle *FindFreeHandle(RHeapPtr heap) {
	RHandleBlock        *hBlock;
	RHandle             *handle;

	//  Search for a handle in an existing block that's already free

	for (hBlock = heap->handleBlocks; hBlock; hBlock = hBlock->next) {
		if ((handle = hBlock->freeHandles) != NULL) {
			RDelinkHandle(handle);
			hBlock->freeHandleCount--;
			return handle;
		}
	}

	//  instead we need to create a new block.
	//  assume for now we'll create it on the same heap.

	if ((hBlock = NewHandleBlock(heap, "internal")) != NULL) {
		if ((handle = hBlock->freeHandles) != NULL) {
			RDelinkHandle(handle);
			hBlock->freeHandleCount--;
			return handle;
		}
	}

	return NULL;
}

static void FreeHandleBlock(RHeap *heap, RHandleBlock *hBlock) {
	if (hBlock && heap) {                           // if pointer is non-null
		//  We want to encourage handle blocks to be allocated near
		//  the beginning of the memory buffer. One way to do this is
		//  to never free any handles near the beginning of the buffer.

		if ((uint8 *)hBlock >= (uint8 *)heap + (heap->size >> 3)) {
			// unlink from heap list
			RDelinkHandle((RHandle *)hBlock);
			DisposePtrHeap(hBlock, heap);    // then free from heap
		}
	}
}

static void FreeHandle(RHeap *heap, RHandle *handle) {
	RHandleBlock        *hBlock;

	//  Search for a handle in an existing block that's already free

	for (hBlock = heap->handleBlocks; hBlock; hBlock = hBlock->next) {
		if (handle >= &hBlock->handles[ 0 ]
		        && handle <  &hBlock->handles[ HANDLES_PER_BLOCK ]) {
			RDelinkHandle(handle);

			handle->data = NULL;
			RLinkHandle(handle, (RHandle *)&hBlock->freeHandles);

			//  If all the handles in this block are free, then consider
			//  perhaps freeing the block

			if (++hBlock->freeHandleCount >= HANDLES_PER_BLOCK)
				FreeHandleBlock(heap, hBlock);

			return;
		}
	}
}

/* ============================================================================ *
    Higher-level handle allocate and deallocate.
 * ============================================================================ */

//  Allocate a block of memory and return a handle to it.

RHANDLE _RNewHandle(int32 size, RHeapPtr heap, const char desc[]) {
	void            *mem;
	RHandle         *handle;
	RMemPrefix      *pr;

	if (heap == NULL) heap = heapList;

	if ((handle = FindFreeHandle(heap)) != NULL) {
		if ((mem = NewPtr(size, heap, FALSE, desc)) != NULL) {
			pr = PrefixBaseAddr(mem);        // get memory prefix
			pr->handle = handle;            // point prefix back to handle
			handle->data = mem;             // point handle to prefix

			SHOWMEM;
			return &handle->data;
		}
		FreeHandle(heap, handle);            // Free the handle
	}
	return NULL;
}

//  Allocate a block of memory, clear it, and return a handle to it.

RHANDLE _RNewClearHandle(int32 size, RHeapPtr heap, const char desc[]) {
	RHANDLE handle;

	if (heap == NULL) heap = heapList;

	if (handle = RNewHandle(size, heap, desc))
		memset(*handle, 0, size);

	SHOWMEM;
	return handle;
}

void _RDisposeHandle(RHANDLE handle) {
	RHandle         *hb = HandleBaseAddr(handle);
	RMemPrefix      *pr;
	RHeapPtr        heap;

	if (handle && hb->data) {
		if ((heap = whichHeap(hb)) == NULL) {
			// REM: Error!
			return;
		}

		pr = PrefixBaseAddr(hb->data);
		RDisposePtr(hb->data);           // Free the block
		FreeHandle(heap, hb);                // Free the handle
	}
	SHOWMEM;
}

/* ============================================================================ *
    Functions to allocate data associated with handle without changing
    the handle.
 * ============================================================================ */

void _RDisposeHandleData(RHANDLE handle) {
	RHandle         *hb = HandleBaseAddr(handle);
	RHeapPtr        heap;
	RMemPrefix      *pr;

	if (handle && hb->data) {
		if ((heap = whichHeap(hb)) == NULL) {
			// REM: Error!
			return;
		}

		pr = PrefixBaseAddr(hb->data);
		RDisposePtr(hb->data);       // Free the block
		hb->data = NULL;
	}
	SHOWMEM;
}

void *_RAllocHandleData(RHANDLE handle, int32 size, const char desc[]) {
	RHandle         *hb = HandleBaseAddr(handle);
	void            *mem;
	RMemPrefix      *pr;
	RHeapPtr        heap;

	if ((heap = whichHeap(hb)) == NULL) {
		// REM: Error!
		return NULL;
	}

	if (hb->data != NULL) RDisposeHandleData(handle);

	if ((mem = NewPtr(size, heap, FALSE, desc)) != NULL) {
		pr = PrefixBaseAddr(mem);        // get memory prefix
		pr->handle = hb;                // point prefix back to handle
		hb->data = mem;                 // point handle to prefix

		SHOWMEM;
		return mem;
	}
	return NULL;
}

/* ============================================================================ *
    Functions to get handle attributes
 * ============================================================================ */

uint32 _RPtrSize(void *ptr) {
	if (ptr) {
		RMemPrefix      *pr = PrefixBaseAddr(ptr);
		return pr->size - sizeof * pr - wallSize * 2;
	}
	return 0;
}

uint16 _RGetHandleFlags(RHANDLE handle) {
	RHandle         *hb = HandleBaseAddr(handle);
	RMemPrefix      *pr;

	if (hb->data) {
		pr = PrefixBaseAddr(hb->data);
		return (uint16)pr->flags;
	}
	return (uint16)0;
}

/* ============================================================================ *
    Set block as cacheable
 * ============================================================================ */

void _RCacheHandle(RHANDLE handle) {
	RHandle         *hb = HandleBaseAddr(handle);
	RMemPrefix      *pr;
	RHeapPtr        heap;

	if (handle && hb->data) {               // if handle exists
		pr = PrefixBaseAddr(hb->data);      // get address of data

		if (!(pr->flags & rMemPurgeable)) { // if not already purgeable
			// determine which heap
			if ((heap = whichHeap(hb)) == NULL) return;

			pr->flags |= rMemPurgeable;     // mark as purgeable

			if (pr->lockNestCount == 0) {   // if unlocked, then...
				RDelinkHandle(hb);
				RLinkHandle(hb, (RHandle *)&heap->cacheHead);
			}
		}
	}
}

/* ============================================================================ *
    Functions to manipulate handle attributes
 * ============================================================================ */

//  RLockHandle: Prevents memory handle from being relocated.

void *_RLockHandle(RHANDLE handle) {
	RHandle         *hb = HandleBaseAddr(handle);
	RMemPrefix      *pr;

	if (handle && hb->data) {
		pr = PrefixBaseAddr(hb->data);

		if (pr->lockNestCount == 0         // if not locked
		        && (pr->flags & rMemPurgeable)) { // and purgeable
			RDelinkHandle(hb);           // remove from cache list
		}

		pr->lockNestCount++;
	}
	return hb->data;
}

//  RUnlockHandle: Enabled handle relocation (and caching if
//  the handle is cacheable.

void _RUnlockHandle(RHANDLE handle) {
	RHandle         *hb = HandleBaseAddr(handle);
	RMemPrefix      *pr;

	if (handle && hb->data) {
		if (whichHeap(hb->data) == NULL)
			error("Handle has invalid data pointer");
		pr = PrefixBaseAddr(hb->data);
		pr->lockNestCount--;

		if (pr->lockNestCount == 0         // if not locked
		        && (pr->flags & rMemPurgeable)) { // and purgeable
			RHeapPtr        heap;

			if ((heap = whichHeap(hb)) != NULL) {
				RDelinkHandle(hb);
				RLinkHandle(hb, (RHandle *)&heap->cacheHead);
			}
		}
		//checkWalls( (uint8 *)(pr + 1), pr->size - sizeof *pr );
	}
}

/*
void RSetHandleFlags( RHANDLE handle, uint16 newflags )
{
    RHandle         *hb = HandleBaseAddr( handle );
    RMemPrefix      *pr;

    if (hb->data)
    {
        pr = PrefixBaseAddr(hb->data);
        pr->flags = (pr->flags & ~rMemUserFlags) | (newflags & rMemUserFlags);
    }
}
*/

/* ============================================================================ *
    Deferred Loading functions
 * ============================================================================ */

void _RHandleStartLoading(RHANDLE handle) {
	RMemPrefix      *pr;
	RHandle         *hb = HandleBaseAddr(handle);

	if (handle == NULL || hb->data == NULL) return;
	pr = PrefixBaseAddr(hb->data);
	pr->flags |= rMemLoading;
}

void _RHandleDoneLoading(RHANDLE handle) {
	RMemPrefix      *pr;
	RHandle         *hb = HandleBaseAddr(handle);

	if (handle == NULL || hb->data == NULL) return;
	pr = PrefixBaseAddr(hb->data);
	pr->flags &= ~rMemLoading;
}

bool _RHandleLoaded(RHANDLE handle) {
	RMemPrefix      *pr;
	RHandle         *hb = HandleBaseAddr(handle);

	if (handle == NULL || hb->data == NULL) return FALSE;
	pr = PrefixBaseAddr(hb->data);
	return (pr->flags & rMemLoading) ? 0 : 1;
}

bool _RHandleLoading(RHANDLE handle) {
	RMemPrefix      *pr;
	RHandle         *hb = HandleBaseAddr(handle);

	if (handle == NULL || hb->data == NULL) return FALSE;
	pr = PrefixBaseAddr(hb->data);
	return (pr->flags & rMemLoading) ? 1 : 0;
}

/* ============================================================================ *
    Debugging functions
 * ============================================================================ */

#if DEBUG

//	Rem: We should also do a stack backtrace here...?

//  This function is called whenever a wall corruption is detected.
//  Probably should be set as a breakpoint in the debugger.
void wallHit(void) {
	error("Wall Hit");
}

//  Checks to see if the memory cookie is still in place around each allocation.
void checkWalls(uint8 *begin, uint32 length) {
	uint8           *end = begin + length;
	int16           i;

	RHeapPtr            heap = whichHeap(begin);            // pointer to new heap

	if (heap == NULL) {
		error("invalid wall size");
	}
	if (length > heap->size) {
		error("invalid wall size");
	}
	if (whichHeap(end - 1) != heap) {
		error("invalid wall size");
	}
	for (i = 0; i < wallSize; i++) {
		--end;
		if (*begin != RMEM_WALL_ID || *end != RMEM_WALL_ID) {
			wallHit();
			*begin = *end = RMEM_WALL_ID;
			return;
		}
		begin++;
	}
}

//  Sets the memory cookie surrounding each allocation.
void setWalls(uint8 *begin, uint32 length) {
	RHeapPtr            heap = whichHeap(begin);            // pointer to new heap
	uint8           *end = begin + length;
	int             i;

	if (heap == NULL || length > heap->size || whichHeap(end - 1) != heap) {
		error("invalid wallocation");
	}
	for (i = 0; i < wallSize; i++) {
		*begin++ = RMEM_WALL_ID;
		*--end = RMEM_WALL_ID;
	}
}

#endif

void heapCheck(void) {
	if (heapList->firstFree
	        && (uint8 *)heapList->firstFree < (uint8 *)heapList->memory) {
		error("HeapFailed: %d %d\n", heapList->size, heapList->free);
	}
}

//  Check the integtrity of all memory

bool _RMemIntegrity(void) {
	RHeapPtr        heap;
	RMemPrefix      *pr;
	RMemFreeBlock   *blk;
	int16           heapNum;

	// run through the list of heaps, and figure out which one the pointer is in

	for (heap = heapList, heapNum = 0; heap; heap = heap->next, heapNum++) {
		uint8   *last_free,
		        *next_free,
		        *alloc;

		last_free = heap->memory;

		blk = heap->firstFree;

		heapCheck();


		for (;;) {
			next_free = blk ? (uint8 *)blk : heap->memory + heap->size;

			alloc = last_free;
			while (alloc < next_free) {
				uint32  bytesToNext = (next_free - alloc);

				//  We now have the address of a memory prefix.
				pr = (RMemPrefix *)alloc;

				//  Check to see if size field is too large or too small
				if (pr->size > bytesToNext
				        || pr->size < sizeof * pr + wallSize * 2) {

					error("Bad prefix size %d : %s", pr->size,
#if DEBUG
					            pr->description
#else
					            ""
#endif
					           );
					return FALSE;
				}

				//  Check memory wall integrity
				checkWalls(alloc + sizeof * pr, pr->size - sizeof * pr);

				//  Go to the next block
				alloc += QUANTIZE(pr->size);
			}

			if (blk == NULL) break;
			last_free = (uint8 *)blk + blk->size;
			blk = blk->next;
		}
	}
	return TRUE;
}


//  Check the integtrity of all memory

#if DEBUG

static uint32 leakWarnings = 0;
const uint32 maxLeakWarnings = 32;

static bool _RMemCleanHeap(RHeapPtr heap) {
	RMemPrefix      *pr;
	RMemFreeBlock   *blk;
	char            *desc;
	uint8           *base = heap->memory,
	                 *last_free = base,
	                  *next_free,
	                  *alloc;

	next_free = heap->memory;
	alloc = last_free;

	for (blk = heap->firstFree;; blk = blk->next) {
		next_free = blk ? (uint8 *)blk : (uint8 *)heap->memory + heap->size;

		alloc = last_free;
		while (alloc < next_free) {
			//  We now have the address of a memory prefix.
			pr = (RMemPrefix *)alloc;

			desc = pr->description;
			if (pr->magic == RMEM_MAGIC_ID) {
				char tDesc[20];
				strncpy(tDesc, pr->description, 16);
				tDesc[16] = '\0';
				memoryWarning("Warning: possible memory leak; block named '%s' was in the deleted heap.\n", tDesc);
				pr->magic = RMEM_LEAK_ID;
			} else {
				leakWarnings++;
				if (leakWarnings < maxLeakWarnings)
					memoryWarning("Warning: unknown memory arena ID %d in disposed heap.\n", pr->magic);
				else if (leakWarnings == maxLeakWarnings)
					memoryWarning("Warning: too many memory leak warnings. Remaining warnings ignored\n");
				else
					return TRUE;

			}
			//  Go to the next block
			alloc += QUANTIZE(pr->size);
		}

		if (blk == NULL) break;

		last_free = (uint8 *)blk + blk->size;

	}
	return TRUE;
}
#endif


void RShowMem(void) {
#if DEBUG
	RDumpMem(heapList, wallSize);
#endif
}


#define COMPACTER_RESOLUTION 1000000

void RHeapsAMess(void) {
	RHeapPtr heap = heapList;
	if (heap) {
		int32 compactSize = heap->size / 2;
		for (int32 i = compactSize; i > 0; i -= COMPACTER_RESOLUTION)
			CompactHeap(heap, i);
	}
}

} // end of namespace Saga2
