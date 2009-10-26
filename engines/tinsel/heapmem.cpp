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
 * This file contains the handle based Memory Manager code.
 */

#include "tinsel/heapmem.h"
#include "tinsel/timers.h"	// For DwGetCurrentTime
#include "tinsel/tinsel.h"

namespace Tinsel {

// Specifies the total amount of memory required for DW1 demo, DW1, or DW2 respectively.
// Currently this is set at 5MB for the DW1 demo and DW1 and 10MB for DW2
// This could probably be reduced somewhat
// If the memory is not enough, the engine throws an "Out of memory" error in handle.cpp inside LockMem()
static const uint32 MemoryPoolSize[3] = {5 * 1024 * 1024, 5 * 1024 * 1024, 10 * 1024 * 1024};

// list of all memory nodes
MEM_NODE mnodeList[NUM_MNODES];

// pointer to the linked list of free mnodes
static MEM_NODE *pFreeMemNodes;

#ifdef DEBUG
// diagnostic mnode counters
static int numNodes;
static int maxNodes;
#endif

// the mnode heap sentinel
static MEM_NODE heapSentinel;

//
static MEM_NODE *AllocMemNode(void);


/**
 * Initialises the memory manager.
 */
void MemoryInit(void) {
	MEM_NODE *pNode;

#ifdef DEBUG
	// clear number of nodes in use
	numNodes = 0;
#endif

	// place first node on free list
	pFreeMemNodes = mnodeList;

	// link all other objects after first
	for (int i = 1; i < NUM_MNODES; i++) {
		mnodeList[i - 1].pNext = mnodeList + i;
	}

	// null the last mnode
	mnodeList[NUM_MNODES - 1].pNext = NULL;

	// allocates a big chunk of memory
	uint32 size = MemoryPoolSize[0];
	if (TinselVersion == TINSEL_V1) size = MemoryPoolSize[1];
	else if (TinselVersion == TINSEL_V2) size = MemoryPoolSize[2];
	uint8 *mem = (uint8 *)malloc(size);
	assert(mem);

	// allocate a mnode for this memory
	pNode = AllocMemNode();

	// make sure mnode was allocated
	assert(pNode);

	// convert segment to memory address
	pNode->pBaseAddr = mem;

	// set size of the memory heap
	pNode->size = size;

	// clear the memory
	memset(pNode->pBaseAddr, 0, size);

	// set cyclic links to the sentinel
	heapSentinel.pPrev = pNode;
	heapSentinel.pNext = pNode;
	pNode->pPrev = &heapSentinel;
	pNode->pNext = &heapSentinel;

	// flag sentinel as locked
	heapSentinel.flags = DWM_LOCKED | DWM_SENTINEL;
}


#ifdef DEBUG
/**
 * Shows the maximum number of mnodes used at once.
 */

void MemoryStats(void) {
	printf("%i mnodes of %i used.\n", maxNodes, NUM_MNODES);
}
#endif

/**
 * Allocate a mnode from the free list.
 */
static MEM_NODE *AllocMemNode(void) {
	// get the first free mnode
	MEM_NODE *pMemNode = pFreeMemNodes;

	// make sure a mnode is available
	assert(pMemNode); // Out of memory nodes

	// the next free mnode
	pFreeMemNodes = pMemNode->pNext;

	// wipe out the mnode
	memset(pMemNode, 0, sizeof(MEM_NODE));

#ifdef DEBUG
	// one more mnode in use
	if (++numNodes > maxNodes)
		maxNodes = numNodes;
#endif

	// return new mnode
	return pMemNode;
}

/**
 * Return a mnode back to the free list.
 * @param pMemNode			Node of the memory object
 */
void FreeMemNode(MEM_NODE *pMemNode) {
	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

#ifdef DEBUG
	// one less mnode in use
	--numNodes;
	assert(numNodes >= 0);
#endif

	// place free list in mnode next
	pMemNode->pNext = pFreeMemNodes;

	// add mnode to top of free list
	pFreeMemNodes = pMemNode;
}


/**
 * Tries to make space for the specified number of bytes on the specified heap.
 * @param size			Number of bytes to free up
 * @param bDiscard		When set - will discard blocks to fullfill the request
 */
bool HeapCompact(long size, bool bDiscard) {
	MEM_NODE *pHeap = &heapSentinel;
	MEM_NODE *pPrev, *pCur, *pOldest;
	long largest;		// size of largest free block
	uint32 oldest;		// time of the oldest discardable block

	while (true) {
		bool bChanged;

		do {
			bChanged = false;
			for (pPrev = pHeap->pNext, pCur = pPrev->pNext;
				pCur != pHeap; pPrev = pCur, pCur = pCur->pNext) {
				if (pPrev->flags == 0 && pCur->flags == 0) {
					// set the changed flag
					bChanged = true;

					// both blocks are free - merge them
					pPrev->size += pCur->size;

					// unlink the current mnode
					pPrev->pNext = pCur->pNext;
					pCur->pNext->pPrev = pPrev;

					// free the current mnode
					FreeMemNode(pCur);

					// leave the loop
					break;
				} else if ((pPrev->flags & (DWM_MOVEABLE | DWM_LOCKED | DWM_DISCARDED)) == DWM_MOVEABLE
						&& pCur->flags == 0) {
					// a free block after a moveable block - swap them

					// set the changed flag
					bChanged = true;

					// move the unlocked blocks data up (can overlap)
					memmove(pPrev->pBaseAddr + pCur->size,
						pPrev->pBaseAddr, pPrev->size);

					// swap the order in the linked list
					pPrev->pPrev->pNext = pCur;
					pCur->pNext->pPrev = pPrev;

					pCur->pPrev = pPrev->pPrev;
					pPrev->pPrev = pCur;

					pPrev->pNext = pCur->pNext;
					pCur->pNext = pPrev;

					pCur->pBaseAddr = pPrev->pBaseAddr;
					pPrev->pBaseAddr += pCur->size;

					// leave the loop
					break;
				}
			}
		} while (bChanged);

		// find the largest free block
		for (largest = 0, pCur = pHeap->pNext; pCur != pHeap; pCur = pCur->pNext) {
			if (pCur->flags == 0 && pCur->size > largest)
				largest = pCur->size;
		}

		if (largest >= size)
			// we have freed enough memory
			return true;

		if (!bDiscard)
			// we cannot free enough without discarding blocks
			return false;

		// find the oldest discardable block
		oldest = DwGetCurrentTime();
		pOldest = NULL;
		for (pCur = pHeap->pNext; pCur != pHeap; pCur = pCur->pNext) {
			if ((pCur->flags & (DWM_DISCARDABLE | DWM_DISCARDED | DWM_LOCKED))
				== DWM_DISCARDABLE) {
				// found a non-discarded discardable block
				if (pCur->lruTime < oldest) {
					oldest = pCur->lruTime;
					pOldest = pCur;
				}
			}
		}

		if (pOldest)
			// discard the oldest block
			MemoryDiscard(pOldest);
		else
			// cannot discard any blocks
			return false;
	}
}

/**
 * Allocates the specified number of bytes from the heap.
 * @param flags			Allocation attributes
 * @param size			Number of bytes to allocate
 */
MEM_NODE *MemoryAlloc(int flags, long size) {
	MEM_NODE *pHeap = &heapSentinel;
	MEM_NODE *pNode;
	bool bCompacted = true;	// set when heap has been compacted

#ifdef SCUMM_NEED_ALIGNMENT
	const int alignPadding = sizeof(void*) - 1;
	size = (size + alignPadding) & ~alignPadding;	//round up to nearest multiple of sizeof(void*), this ensures the addresses that are returned are alignment-safe.
#endif

	while ((flags & DWM_NOALLOC) == 0 && bCompacted) {
		// search the heap for a free block

		for (pNode = pHeap->pNext; pNode != pHeap; pNode = pNode->pNext) {
			if (pNode->flags == 0 && pNode->size >= size) {
				// a free block of the required size
				pNode->flags = flags;

				// update the LRU time
				pNode->lruTime = DwGetCurrentTime() + 1;

				if (pNode->size == size) {
					// an exact fit

					// check for zeroing the block
					if (flags & DWM_ZEROINIT)
						memset(pNode->pBaseAddr, 0, size);

					// return the node
					return pNode;
				} else {
					// allocate a node for the remainder of the free block
					MEM_NODE *pTemp = AllocMemNode();

					// calc size of the free block
					long freeSize = pNode->size - size;

					// set size of free block
					pTemp->size = freeSize;

					// set size of node
					pNode->size = size;

					// place the free node before pNode
					pTemp->pBaseAddr = pNode->pBaseAddr;
					pNode->pBaseAddr += freeSize;
					pTemp->pNext = pNode;
					pTemp->pPrev = pNode->pPrev;
					pNode->pPrev->pNext = pTemp;
					pNode->pPrev = pTemp;

					// check for zeroing the block
					if (flags & DWM_ZEROINIT)
						memset(pNode->pBaseAddr, 0, size);

					return pNode;
				}
			}
		}
		// compact the heap if we get to here
		bCompacted = HeapCompact(size, (flags & DWM_NOCOMPACT) ? false : true);
	}

	// not allocated a block if we get to here
	if (flags & DWM_DISCARDABLE) {
		// chain a discarded node onto the end of the heap
		pNode = AllocMemNode();
		pNode->flags = flags | DWM_DISCARDED;

		// set mnode at the end of the list
		pNode->pPrev = pHeap->pPrev;
		pNode->pNext = pHeap;

		// fix links to this mnode
		pHeap->pPrev->pNext = pNode;
		pHeap->pPrev = pNode;

		// return the discarded node
		return pNode;
	}

	// could not allocate a block
	return NULL;
}


void *MemoryAllocFixed(long size) {
	// Allocate a fixed block of data. For now, we simply malloc it!
	// TODO: We really should keep a list of the allocated pointers,
	// so that we can discard them later on, when the engine quits.

#ifdef SCUMM_NEED_ALIGNMENT
	const int alignPadding = sizeof(void*) - 1;
	size = (size + alignPadding) & ~alignPadding;	//round up to nearest multiple of sizeof(void*), this ensures the addresses that are returned are alignment-safe.
#endif

	return malloc(size);
}


/**
 * Discards the specified memory object.
 * @param pMemNode			Node of the memory object
 */
void MemoryDiscard(MEM_NODE *pMemNode) {
	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// object must be discardable
	assert(pMemNode->flags & DWM_DISCARDABLE);

	// object cannot be locked
	assert((pMemNode->flags & DWM_LOCKED) == 0);

	if ((pMemNode->flags & DWM_DISCARDED) == 0) {
		// allocate a free node to replace this node
		MEM_NODE *pTemp = AllocMemNode();

		// copy node data
		memcpy(pTemp, pMemNode, sizeof(MEM_NODE));

		// flag as a free block
		pTemp->flags = 0;

		// link in the free node
		pTemp->pPrev->pNext = pTemp;
		pTemp->pNext->pPrev = pTemp;

		// discard the node
		pMemNode->flags |= DWM_DISCARDED;
		pMemNode->pBaseAddr = NULL;
		pMemNode->size = 0;

		// and place it at the end of the heap
		while ((pTemp->flags & DWM_SENTINEL) != DWM_SENTINEL)
			pTemp = pTemp->pNext;

		// pTemp now points to the heap sentinel
		// set mnode at the end of the list
		pMemNode->pPrev = pTemp->pPrev;
		pMemNode->pNext = pTemp;

		// fix links to this mnode
		pTemp->pPrev->pNext = pMemNode;
		pTemp->pPrev = pMemNode;
	}
}

/**
 * Frees the specified memory object and invalidates its node.
 * @param pMemNode			Node of the memory object
 */
void MemoryFree(MEM_NODE *pMemNode) {
	MEM_NODE *pPrev, *pNext;

	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// get pointer to the next mnode
	pNext = pMemNode->pNext;

	// get pointer to the previous mnode
	pPrev = pMemNode->pPrev;

	if (pPrev->flags == 0) {
		// there is a previous free mnode
		pPrev->size += pMemNode->size;

		// unlink this mnode
		pPrev->pNext = pNext;	// previous to next
		pNext->pPrev = pPrev;	// next to previous

		// free this mnode
		FreeMemNode(pMemNode);

		pMemNode = pPrev;
	}
	if (pNext->flags == 0) {
		// the next mnode is free
		pMemNode->size += pNext->size;

		// flag as a free block
		pMemNode->flags = 0;

		// unlink the next mnode
		pMemNode->pNext = pNext->pNext;
		pNext->pNext->pPrev = pMemNode;

		// free the next mnode
		FreeMemNode(pNext);
	}
}

/**
 * Locks a memory object and returns a pointer to the first byte
 * of the objects memory block.
 * @param pMemNode			Node of the memory object
 */
void *MemoryLock(MEM_NODE *pMemNode) {
	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// make sure memory object is not already locked
	assert((pMemNode->flags & DWM_LOCKED) == 0);

	// check for a discarded or null memory object
	if ((pMemNode->flags & DWM_DISCARDED) || pMemNode->size == 0)
		return NULL;

	// set the lock flag
	pMemNode->flags |= DWM_LOCKED;

	// return memory objects base address
	return pMemNode->pBaseAddr;
}

/**
 * Changes the size or attributes of a specified memory object.
 * @param pMemNode		Node of the memory object
 * @param size			New size of block
 * @param flags			How to reallocate the object
 */
MEM_NODE *MemoryReAlloc(MEM_NODE *pMemNode, long size, int flags) {
	MEM_NODE *pNew;

	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// validate the flags
	// must be moveable
	assert(flags & DWM_MOVEABLE);

	// align the size to machine boundary requirements
	size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

	// validate the size
	assert(size);

	// make sure we want the node on the same heap
	assert((flags & (DWM_SOUND | DWM_GRAPHIC)) == (pMemNode->flags & (DWM_SOUND | DWM_GRAPHIC)));

	if (size == pMemNode->size) {
		// must be just a change in flags

		// update the nodes flags
		pMemNode->flags = flags;
	} else {
		// unlink the mnode from the current heap
		pMemNode->pNext->pPrev = pMemNode->pPrev;
		pMemNode->pPrev->pNext = pMemNode->pNext;

		// allocate a new node
		pNew = MemoryAlloc(flags | DWM_MOVEABLE, size);

		// make sure memory allocated
		assert(pNew != NULL);

		// update the nodes flags
		pNew->flags = flags;

		// copy the node to the current node
		memcpy(pMemNode, pNew, sizeof(MEM_NODE));

		// relink the mnode into the list
		pMemNode->pPrev->pNext = pMemNode;
		pMemNode->pNext->pPrev = pMemNode;

		// free the new node
		FreeMemNode(pNew);
	}

	// return the node
	return pMemNode;
}

/**
 * Unlocks a memory object.
 * @param pMemNode		Node of the memory object
 */
void MemoryUnlock(MEM_NODE *pMemNode) {
	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// make sure memory object is already locked
	assert(pMemNode->flags & DWM_LOCKED);

	// clear the lock flag
	pMemNode->flags &= ~DWM_LOCKED;

	// update the LRU time
	pMemNode->lruTime = DwGetCurrentTime();
}

/**
 * Retrieves the mnode associated with the specified pointer to a memory object.
 * @param pMem			Address of memory object
 */
MEM_NODE *MemoryHandle(void *pMem) {
	MEM_NODE *pNode;
	// search the DOS heap
	for (pNode = heapSentinel.pNext; pNode != &heapSentinel; pNode = pNode->pNext) {
		if (pNode->pBaseAddr == pMem)
			// found it
			return pNode;
	}

	// not found if we get to here
	return NULL;
}

} // End of namespace Tinsel
