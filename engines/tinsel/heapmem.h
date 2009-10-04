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
 * This file contains the handle based Memory Manager defines
 */

#ifndef TINSEL_HEAPMEM_H
#define TINSEL_HEAPMEM_H

#include "tinsel/dw.h"		// new data types

namespace Tinsel {

#define	NUM_MNODES	192	// the number of memory management nodes (was 128, then 192)

struct MEM_NODE {
	MEM_NODE *pNext;	// link to the next node in the list
	MEM_NODE *pPrev;	// link to the previous node in the list
	uint8 *pBaseAddr;	// base address of the memory object
	long size;		// size of the memory object
	uint32 lruTime;		// time when memory object was last accessed
	int flags;		// allocation attributes
};

// allocation flags for the MemoryAlloc function
#define	DWM_FIXED	0x0001	// allocates fixed memory
#define	DWM_MOVEABLE	0x0002	// allocates movable memory
#define	DWM_DISCARDABLE	0x0004	// allocates discardable memory
#define	DWM_NOALLOC	0x0008	// when used with discardable memory - allocates a discarded block
#define	DWM_NOCOMPACT	0x0010	// does not discard memory to satisfy the allocation request
#define	DWM_ZEROINIT	0x0020	// initialises memory contents to zero
#define	DWM_SOUND	0x0040	// allocate from the sound pool
#define	DWM_GRAPHIC	0x0080	// allocate from the graphics pool

// return value from the MemoryFlags function
#define	DWM_DISCARDED	0x0100	// the objects memory block has been discarded

// internal allocation flags
#define	DWM_LOCKED	0x0200	// the objects memory block is locked
#define	DWM_SENTINEL	0x0400	// the objects memory block is a sentinel


/*----------------------------------------------------------------------*\
|*			Memory Function Prototypes			*|
\*----------------------------------------------------------------------*/

void MemoryInit(void);		// initialises the memory manager

#ifdef	DEBUG
void MemoryStats(void);		// Shows the maximum number of mnodes used at once
#endif

MEM_NODE *MemoryAlloc(		// allocates the specified number of bytes from the heap
	int flags,		// allocation attributes
	long size);		// number of bytes to allocate

void MemoryDiscard(		// discards the specified memory object
	MEM_NODE *pMemNode);	// node of the memory object

int MemoryFlags(		// returns information about the specified memory object
	MEM_NODE *pMemNode);	// node of the memory object

void MemoryFree(		// frees the specified memory object and invalidates its node
	MEM_NODE *pMemNode);	// node of the memory object

MEM_NODE *MemoryHandle(		// Retrieves the mnode associated with the specified pointer to a memory object
	void *pMem);		// address of memory object

void *MemoryLock(		// locks a memory object and returns a pointer to the first byte of the objects memory block
	MEM_NODE *pMemNode);	// node of the memory object

MEM_NODE *MemoryReAlloc(	// changes the size or attributes of a specified memory object
	MEM_NODE *pMemNode,	// node of the memory object
	long size,		// new size of block
	int flags);		// how to reallocate the object

long MemorySize(		// returns the size, in bytes, of the specified memory object
	MEM_NODE *pMemNode);	// node of the memory object

void MemoryUnlock(		// unlocks a memory object
	MEM_NODE *pMemNode);	// node of the memory object

bool HeapCompact(		// Allocates the specified number of bytes from the specified heap
	long size,		// number of bytes to free up
	bool bDiscard);		// when set - will discard blocks to fullfill the request

} // End of namespace Tinsel

#endif
