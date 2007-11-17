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

#include "common/scummsys.h"
//#include "graphics/scaler.h"
#include "common/system.h"
#include "backends/intern.h"

#include "engines/engine.h"

#include "gp32std.h"
#include "gp32std_grap.h"
#include "gp32std_memory.h"

/////////////////////
//Memory management

#define USER_MEMORY_SIZE (256 * 1024)
#define USER_BLOCK_SIZE 24
#define NUM_BLOCK (USER_MEMORY_SIZE / USER_BLOCK_SIZE)
// use fixed block size for small allocations
// consumes about 128k for block array
// consumes 256k for user memory

class MemBlock {
protected:
	static byte *userMem;
//	static size_t allocSize;
//	static MemBlock *head;
	static int numBlock;
	static int prevBlock;

	// Linked list is slow for this task. :)
	static MemBlock *block;

	byte *blockData;
	size_t size;

//	MemBlock *next;

	int used;

public:
	static void init();
	static void deinit();

	static void *addBlock(size_t size);
	static void deleteBlock(void *dstBlock);
};

byte *MemBlock::userMem = NULL;
//MemBlock *MemBlock::head = NULL;
//size_t MemBlock::allocSize = 0;
int MemBlock::numBlock = 0;
int MemBlock::prevBlock = 0;
MemBlock *MemBlock::block = NULL;

void MemBlock::init()
{
	userMem = (byte *)gm_malloc(USER_MEMORY_SIZE + USER_BLOCK_SIZE);
	block = (MemBlock *)gm_malloc(NUM_BLOCK * sizeof(MemBlock));

	if (!(userMem && block)) {
		//error
	}

	memset(userMem, 0, USER_MEMORY_SIZE + USER_BLOCK_SIZE);
	memset(block, 0, NUM_BLOCK * sizeof(MemBlock));
}

void MemBlock::deinit()
{
	if (!(userMem && block)) {
		//error
	}
	gm_free(userMem);
	gm_free(block);
	userMem = NULL;
	block = NULL;
}

void *MemBlock::addBlock(size_t size)
{
	int i;
	MemBlock *blk = &block[prevBlock];

	// some optimizaion with loop
	for (i = prevBlock; i < NUM_BLOCK; i++) {
		if (!blk->used) {
			break;
		}
		blk++;
	}
	if (i == NUM_BLOCK) {
		blk = &block[0];
		for (i = 0; i < prevBlock; i++) {
			if (!blk->used) {
				break;
			}
			blk++;
		}
		if (i == prevBlock) {
			prevBlock = 0;
			return gm_malloc(size);
		}
	}

	byte *ptr = userMem + (i * USER_BLOCK_SIZE);

	blk->size = size;
	blk->blockData = ptr;
	blk->used = 1;

	prevBlock = i;

	return (void *) ptr;
}

void MemBlock::deleteBlock(void *dstBlock)
{
	// Faster method
	uint32 np = (uint32) dstBlock - (uint32) userMem;

	if ((np / USER_BLOCK_SIZE) * USER_BLOCK_SIZE != np) {
		gm_free(dstBlock);
//		warning("wrong block! (%d / %d)", (np / USER_BLOCK_SIZE) * USER_BLOCK_SIZE, np);
		return;
	}
	int i = np / USER_BLOCK_SIZE;
	if (i > NUM_BLOCK) {
		gm_free(dstBlock);
		return;
	}
	block[i].used = 0;

/*
	int i = 0;
	for (i = 0; i < NUM_BLOCK; i++) {
		if (block[i].blockData == dstBlock)
			break;
	}
	if (i == NUM_BLOCK) {
		gm_free(dstBlock);	//fixme?
		//warning("wrong block! %x", (uint32)block - (uint32)userMem);
	} else {
		GPDEBUG("deleteing block %d", i);
		block[i].used = 0;
	}*/
}

// HACK not to include struct MemBlock
void memBlockInit()
{
	MemBlock::init();
}

void memBlockDeinit()
{
	MemBlock::deinit();
}

#undef memcpy
#undef memset
void *gp_memcpy(void *dst, const void *src, size_t count) {
	return memcpy(dst, src, count);
}

void *gp_memset(void *dst, int val, size_t count) {
	return memset(dst, val, count);
}

#define MALLOC_MASK 0xAB800000

// WE HAVE TO ALIGN MEMORY ADDRESS ON THE ARM PROCESSOR!
#define ALIGNED_SIZE(size) ((size) + (4 - ((size) & 3)))

void *gp_malloc(size_t size) {
	uint32 np;
	uint32 *up;

	// size + 8 bytes : stores block size
	int allocSize = ALIGNED_SIZE(size) + sizeof(uint32) + sizeof(uint32);
	if (allocSize <= USER_BLOCK_SIZE) {
		np = (uint32) MemBlock::addBlock(allocSize);
	} else {
		np = (uint32) gm_malloc(allocSize);
	}

	if (np) {
		up = (uint32 *)np;
		*up = ALIGNED_SIZE(size) | MALLOC_MASK;	// mem size: up to 8mb
		up = (uint32 *)(np + ALIGNED_SIZE(size) + sizeof(uint32));
		*up = 0x1234;	// catches oob acess
		return (void *)(np + sizeof(uint32));
	}

	return NULL;
}

void gp_free(void *block) {
	uint32 np;
	uint32 *up;

	if (!block) {
		return;
	}

	np = ((uint32) block) - sizeof(uint32);
	up = (uint32 *) np;
	if (*up == 0x4321) {
		warning("%s: double deallocation", __FUNCTION__);
		return;
	}

	if (*up & MALLOC_MASK != MALLOC_MASK) {
		warning("%s: corrupt block", __FUNCTION__);
		return;
	}

	int blockSize = (*up & 0x7fffff);
	up = (uint32 *)(np + blockSize + sizeof(uint32));
	if (*up != 0x1234) {
		warning("gp_free: corrupt block - OOB access", __FUNCTION__);
		return;
	}

	np = ((uint32) block) - sizeof(uint32);
	up = (uint32 *) np;
	*up = 0x4321;

	if (blockSize + 8 <= USER_BLOCK_SIZE) {
		MemBlock::deleteBlock(up);
	} else {
		gm_free(up);
	}
}

void *gp_calloc(size_t nitems, size_t size) {
	void *p = gp_malloc(nitems * size);	//gpcalloc doesnt clear?

	gp_memset(p, 0, nitems * size);

//	if (*(uint8 *)p != 0)
//		warning("%s: calloc doesn't clear", __FUNCTION__);	//fixme: was error

	return p;
}

//////////////////////////////////////////////////
// GP32 stuff
//////////////////////////////////////////////////
static char usedMemStr[16];
int gUsedMem = 1024 * 1024;

//#define CLEAN_MEMORY_WITH_0xE7
//#define CHECK_USED_MEMORY
//#define CHECK_NEW_TIME
//#define CHECK_NEW_SIZE

void *operator new(size_t size) {
#if defined(CHECK_NEW_TIME)
	static int ftick;
	ftick = GpTickCountGet();
#endif
//	printf("BP:operator new(%d)", size);
	void *ptr = gp_malloc(size);

#if defined(CLEAN_MEMORY_WITH_0xE7)
	if (ptr != NULL) {
		gp_memset(ptr, 0xE7, size);
	}
#endif
#if defined(CHECK_USED_MEMORY)
	// Check free memory.
	gUsedMem = ((int)(ptr) + size) - 0xc000000;

	sprintf(usedMemStr, "%8d", gUsedMem);
	gp_fillRect(frameBuffer1, 0, 0, 64, 12, 0);
	gp_textOut(frameBuffer1, 0, 0, usedMemStr, 0xfffff);
#endif
#if defined(CHECK_NEW_TIME)
	sprintf(usedMemStr, "%2d", GpTickCountGet() - ftick);
	gp_fillRect(frameBuffer1, 72, 0, 24, 12, 0);
	gp_textOut(frameBuffer1, 72, 0, usedMemStr, 0xfffff);
#endif
#if defined(CHECK_NEW_SIZE)
	sprintf(usedMemStr, "%8d", size);
	gp_fillRect(frameBuffer1, 108, 0, 64, 12, 0);
	gp_textOut(frameBuffer1, 108, 0, usedMemStr, 0xfffff);
#endif
	return ptr;
}

void operator delete(void *ptr) {
//	printf("operator delete(%x)", ptr);
	gp_free(ptr);
}
