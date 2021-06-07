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

#ifndef SAGA2_RMEMBASE_H
#define SAGA2_RMEMBASE_H

namespace Saga2 {

/* ===================================================================== *
    Optional memory manager debugging settings
      Note that none of these has any effect if DEBUG is not 1
 * ====================================================================== */

// memory block magic ID's
#define RMEM_MAGIC_ID ('E' + 'O'*256)
#define RMEM_FREED_ID ('O' + 'E'*256)

// Wiping allocated blocks
#define RMEM_WIPE_ALLOC 1
#define RMEM_ALLOC_WIPE_ID 0x90

// Wiping freed blocks
#define RMEM_WIPE_FREE  1
#define RMEM_FREE_WIPE_ID 0xCD

//  Set this to 1 to display the memory blocks on the screen...
//  (You will need to supply the display code)
#define RMEM_VISIBLE        0

#define RMEM_WALL_SIZE      8                       // memory wall size around allocs
#define RMEM_MUNG_ENABLE    1                       // enable munging of free blocks


/* ===================================================================== *
    types
 * ====================================================================== */

#ifndef __cplusplus
typedef struct RMemPrefix       RMemPrefix;
typedef struct RHandleBlock     RHandleBlock;
typedef struct RMemFreeBlock    RMemFreeBlock;
typedef struct RMemHeader       RMemHeader;
#endif

//  Flags which apply to a memory block prefix
enum memPrefixFlags {
	rMemRelocatable = (1 << 0),             // block is of relocatable type
	rMemPurgeable = (1 << 1),               // block is purgeable

	// settable by appliprog
//	rMemUserFlags = (rMemPurgeable | rMemCacheable)

	//  A flag which indicates that the block is not
	//  yet ready for use (mark as "loading");
	rMemLoading = (1 << 2),

	// This flag indicates has been locked against VMM swapping
	//   in addition to the internal memory locking

	rMemVMMLocked = (1 << 3),
};


//  ISSUE: For integration with the database functions, it would be
//  useful to add in a database key or resource ID number, so that the
//  database routines could search for this data in the cache. Note
//  however, that the search would still be linear, which might not be
//  the best method. There might be a way to combine the LRU algorithm
//  with some kind of hash chain, so that only a few cache entries
//  need be searched.

//  RMemPrefix: This small structure precedes every allocated block

struct RMemPrefix {
#if DEBUG
	int16           magic;
#endif
	uint32          size;                   // length of this block, incl. prefix
	RHandle         *handle;                // back pointer to handle
	uint8           flags,                  // flags: purgeable, etc.
	                lockNestCount;          // nest count of locks
#if DEBUG
	char description[16];
#endif
// 10 bytes ( + 18 if DEBUG )
};



//  A Memory Handle, which is an indirect reference to a block of memory.
//  This is used so that blocks of memory may be reorganized and compacted
//  while still keeping external references to the memory valid.
//  The handle has a DNode for two reasons: One, to keep a list of all
//  unused handles, and also to keep a least-recently-used list of all
//  purgeable handles.

struct _RHandle {
	RHandle         *next,                  // next handle in chain
	                *prev;                  // previous handle in chain
	void            *data;                  // pointer to real data for handle
	// NULL if no data allocated
// 12 bytes
};

#define HandleBaseAddr(h) ((RHandle *)((uint8 *)(h) - offsetof( RHandle, data )))
#define PrefixBaseAddr(d) ((RMemPrefix *)((uint8 *)(d) - prefixOffset))

//  RHandleBlock: A block of memory handles. It's based on the DNode so that
//  blocks can be linked and delinked independently of the others.
//  This block is itself allocated as a non-relocatable block.

#define HANDLES_PER_BLOCK   64              // number of handles per block

struct RHandleBlock {
	RHandleBlock    *next,
	                *prev;

	uint16          freeHandleCount;        // number of handles in use
	RHandle         *freeHandles;           // the first free handle in block
	RHandle         handles[HANDLES_PER_BLOCK];   // array of handles
};

//  An "RMemFreeBlock" keeps track of which areas are unused.

struct RMemFreeBlock  {
	RMemFreeBlock   *next;                  // pointer to next free block
	uint32          size;                   // size of this arena
};


//  A "heap" consists of a linked lists of MemHeaders, such that
//  allocations within an Arena will be drawn from any of the
//  MemHeaders.

struct _RHeap {
	struct _RHeap   *next;                  // pointer to next heap
	uint32          size,                   // size of memory space for heap
	                free;                   // number of bytes of free memory
	uint8           *memory;                // ptr to actual memory
	RMemFreeBlock   *firstFree;             // ptr to first free block
	RHandleBlock    *handleBlocks;          // list of blocks of handles
	RHandle         *cacheHead,             // list of allocations in the cache
	                *cacheTail;             // end of cache list
};

/*  Realm memory management functions:

        RMemAvail...
        RMemCompact...

// database stuff...
        handle = RFindCacheItem( heap, keys?? );
        RSetCacheItem( handle, keys?? );

        RGetResource( keys... );
*/

} // end of namespace Saga2

#endif
