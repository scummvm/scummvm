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


#define	NUM_MNODES	192	// the number of memory management nodes (was 128, then 192)


// internal allocation flags
#define	DWM_USED		0x0001	///< the objects memory block is in use
#define	DWM_DISCARDED	0x0002	///< the objects memory block has been discarded
#define	DWM_LOCKED		0x0004	///< the objects memory block is locked
#define	DWM_SENTINEL	0x0008	///< the objects memory block is a sentinel


struct MEM_NODE {
	MEM_NODE *pNext;	// link to the next node in the list
	MEM_NODE *pPrev;	// link to the previous node in the list
	uint8 *pBaseAddr;	// base address of the memory object
	long size;		// size of the memory object
	uint32 lruTime;		// time when memory object was last accessed
	int flags;		// allocation attributes
};


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

// list of all fixed memory nodes
MEM_NODE s_fixedMnodesList[5];

// the mnode heap sentinel
static MEM_NODE heapSentinel;

//
static MEM_NODE *AllocMemNode(void);


/**
 * Initialises the memory manager.
 */
void MemoryInit() {
#ifdef DEBUG
	// clear number of nodes in use
	numNodes = 0;
#endif

	// place first node on free list
	pFreeMemNodes = mnodeList;

	// link all other objects after first
	memset(mnodeList, 0, sizeof(mnodeList));
	for (int i = 1; i < NUM_MNODES; i++) {
		mnodeList[i - 1].pNext = mnodeList + i;
	}

	// null the last mnode
	mnodeList[NUM_MNODES - 1].pNext = NULL;

	// clear list of fixed memory nodes
	memset(s_fixedMnodesList, 0, sizeof(s_fixedMnodesList));

	// set cyclic links to the sentinel
	heapSentinel.pPrev = &heapSentinel;
	heapSentinel.pNext = &heapSentinel;

	// flag sentinel as locked
	heapSentinel.flags = DWM_LOCKED | DWM_SENTINEL;

	// store the current heap size in the sentinel
	uint32 size = MemoryPoolSize[0];
	if (TinselVersion == TINSEL_V1) size = MemoryPoolSize[1];
	else if (TinselVersion == TINSEL_V2) size = MemoryPoolSize[2];
	heapSentinel.size = size;
}

/**
 * Deinitialises the memory manager.
 */
void MemoryDeinit() {
	const MEM_NODE *pHeap = &heapSentinel;
	MEM_NODE *pCur;

	pCur = s_fixedMnodesList;
	for (int i = 0; i < ARRAYSIZE(s_fixedMnodesList); ++i, ++pCur) {
		free(pCur->pBaseAddr);
		pCur->pBaseAddr = 0;
	}

	for (pCur = pHeap->pNext; pCur != pHeap; pCur = pCur->pNext) {
		free(pCur->pBaseAddr);
		pCur->pBaseAddr = 0;
	}
}


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
 * @return true if any blocks were discarded, false otherwise
 */
bool HeapCompact(long size, bool bDiscard) {
	const MEM_NODE *pHeap = &heapSentinel;
	MEM_NODE *pCur, *pOldest;
	uint32 oldest;		// time of the oldest discardable block

	while (heapSentinel.size < size) {

		if (!bDiscard)
			// we cannot free enough without discarding blocks
			return false;

		// find the oldest discardable block
		oldest = DwGetCurrentTime();
		pOldest = NULL;
		for (pCur = pHeap->pNext; pCur != pHeap; pCur = pCur->pNext) {
			if (pCur->flags == DWM_USED) {
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

	// we have freed enough memory
	return true;
}

/**
 * Allocates the specified number of bytes from the heap.
 * @param flags			Allocation attributes
 * @param size			Number of bytes to allocate
 */
static MEM_NODE *MemoryAlloc(long size) {
	MEM_NODE *pHeap = &heapSentinel;

#ifdef SCUMM_NEED_ALIGNMENT
	const int alignPadding = sizeof(void*) - 1;
	size = (size + alignPadding) & ~alignPadding;	//round up to nearest multiple of sizeof(void*), this ensures the addresses that are returned are alignment-safe.
#endif

	// compact the heap to make up room for 'size' bytes, if necessary
	if (!HeapCompact(size, true))
		return 0;

	// success! we may allocate a new node of the right size

	// Allocate a node.
	MEM_NODE *pNode = AllocMemNode();

	// Allocate memory for the node.
	pNode->pBaseAddr = (byte *)malloc(size);

	// Verify that we got the memory.
	// TODO: If this fails, we should first try to compact the heap some further.
	assert(pNode->pBaseAddr);

	// Subtract size of new block from total
	heapSentinel.size -= size;

	// Set flags, LRU time and size
	pNode->flags = DWM_USED;
	pNode->lruTime = DwGetCurrentTime();
	pNode->size = size;

	// set mnode at the end of the list
	pNode->pPrev = pHeap->pPrev;
	pNode->pNext = pHeap;

	// fix links to this mnode
	pHeap->pPrev->pNext = pNode;
	pHeap->pPrev = pNode;

	return pNode;
}

/**
 * Allocate a discarded MEM_NODE. Actual memory can be assigned to it
 * by using MemoryReAlloc().
 */
MEM_NODE *MemoryNoAlloc() {
	MEM_NODE *pHeap = &heapSentinel;

	// chain a discarded node onto the end of the heap
	MEM_NODE *pNode = AllocMemNode();
	pNode->flags = DWM_USED | DWM_DISCARDED;
	pNode->lruTime = DwGetCurrentTime();
	pNode->size = 0;

	// set mnode at the end of the list
	pNode->pPrev = pHeap->pPrev;
	pNode->pNext = pHeap;

	// fix links to this mnode
	pHeap->pPrev->pNext = pNode;
	pHeap->pPrev = pNode;

	// return the discarded node
	return pNode;
}

/**
 * Allocate a fixed block of data.
 * @todo We really should keep track of the allocated pointers,
 *       so that we can discard them later on, when the engine quits.
 */
MEM_NODE *MemoryAllocFixed(long size) {

#ifdef SCUMM_NEED_ALIGNMENT
	const int alignPadding = sizeof(void*) - 1;
	size = (size + alignPadding) & ~alignPadding;	//round up to nearest multiple of sizeof(void*), this ensures the addresses that are returned are alignment-safe.
#endif

	// Search for a free entry in s_fixedMnodesList
	MEM_NODE *pNode = s_fixedMnodesList;
	for (int i = 0; i < ARRAYSIZE(s_fixedMnodesList); ++i, ++pNode) {
		if (!pNode->pBaseAddr) {
			pNode->pNext = 0;
			pNode->pPrev = 0;
			pNode->pBaseAddr = (byte *)malloc(size);
			pNode->size = size;
			pNode->lruTime = DwGetCurrentTime();
			pNode->flags = DWM_USED;

			// Subtract size of new block from total
			heapSentinel.size -= size;

			return pNode;
		}
	}

	return 0;
}


/**
 * Discards the specified memory object.
 * @param pMemNode			Node of the memory object
 */
void MemoryDiscard(MEM_NODE *pMemNode) {
	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// object must be in use and locked
	assert((pMemNode->flags & (DWM_USED | DWM_LOCKED)) == DWM_USED);

	// discard it if it isn't already
	if ((pMemNode->flags & DWM_DISCARDED) == 0) {
		// free memory
		free(pMemNode->pBaseAddr);
		heapSentinel.size += pMemNode->size;

		// mark the node as discarded
		pMemNode->flags |= DWM_DISCARDED;
		pMemNode->pBaseAddr = NULL;
		pMemNode->size = 0;
	}
}

/**
 * Locks a memory object and returns a pointer to the first byte
 * of the objects memory block.
 * @param pMemNode			Node of the memory object
 */
void *MemoryLock(MEM_NODE *pMemNode) {
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
 * Changes the size of a specified memory object and re-allocate it if necessary.
 * @param pMemNode		Node of the memory object
 * @param size			New size of block
 */
void MemoryReAlloc(MEM_NODE *pMemNode, long size) {
	MEM_NODE *pNew;

	// validate mnode pointer
	assert(pMemNode >= mnodeList && pMemNode <= mnodeList + NUM_MNODES - 1);

	// align the size to machine boundary requirements
	size = (size + sizeof(void *) - 1) & ~(sizeof(void *) - 1);

	// validate the size
	assert(size);

	if (size != pMemNode->size) {
		// make sure memory object is discarded and not locked
		assert(pMemNode->flags == (DWM_USED | DWM_DISCARDED));
		assert(pMemNode->size == 0);

		// unlink the mnode from the current heap
		pMemNode->pNext->pPrev = pMemNode->pPrev;
		pMemNode->pPrev->pNext = pMemNode->pNext;

		// allocate a new node
		pNew = MemoryAlloc(size);

		// make sure memory allocated
		assert(pNew != NULL);

		// copy the node to the current node
		memcpy(pMemNode, pNew, sizeof(MEM_NODE));

		// relink the mnode into the list
		pMemNode->pPrev->pNext = pMemNode;
		pMemNode->pNext->pPrev = pMemNode;

		// free the new node
		FreeMemNode(pNew);
	}

	assert(pMemNode->pBaseAddr);
}

/**
 * Unlocks a memory object.
 * @param pMemNode		Node of the memory object
 */
void MemoryUnlock(MEM_NODE *pMemNode) {
	// make sure memory object is already locked
	assert(pMemNode->flags & DWM_LOCKED);

	// clear the lock flag
	pMemNode->flags &= ~DWM_LOCKED;

	// update the LRU time
	pMemNode->lruTime = DwGetCurrentTime();
}

/**
 * Touch a memory object by updating its LRU time.
 * @param pMemNode		Node of the memory object
 */
void MemoryTouch(MEM_NODE *pMemNode) {
	// update the LRU time
	pMemNode->lruTime = DwGetCurrentTime();
}

uint8 *MemoryDeref(MEM_NODE *pMemNode) {
	return pMemNode->pBaseAddr;
}


} // End of namespace Tinsel
