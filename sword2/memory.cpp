/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// FIXME: We should investigate which problem all this memory managing stuff
// is trying to solve. I'm not convinced that it's really needed.

// memory manager
//   - "remember, it's not good to leave memory locked for a moment longer
//      than necessary" Tony
//   - "actually, in a sequential system theoretically you never need to lock
//      any memory!" Chris ;)
//
// This is a very simple implementation but I see little advantage to being
// any cleverer with the coding - i could have put the mem blocks before the
// defined blocks instead of in an array and then used pointers to
// child/parent blocks. But why bother? I've Kept it simple. When it needs
// updating or customising it will be accessable to anyone who looks at it.
//
// Doesn't have a purgeable/age consituant yet - if anyone wants this then
// I'll add it in.

// MemMan v1.1

#include "stdafx.h"
#include "driver/driver96.h"
#include "debug.h"
#include "memory.h"
#include "resman.h"

uint32 total_blocks;
uint32 total_free_memory;

#define MEMORY_POOL (1024 * 12000)

// address of init malloc to be freed later
uint8 *free_memman;

// list of defined memory handles - each representing a block of memory.
mem mem_list[MAX_mem_blocks];

uint32 base_mem_block;

// #define MEMDEBUG 1

// Used to determine if the required size can be obtained if the defragger is
// allowed to run.

int32 VirtualDefrag(uint32 size);

// Start position of the Defragger as indicated by its sister VirtualDefrag.
int32 suggestedStart = 0;

void Close_memory_manager(void) {	// Tony2Oct96
	free(free_memman);
}

void Init_memory_manager(void) {
	uint32 j;
	uint8 *memory_base;

	total_free_memory = MEMORY_POOL;

	// malloc memory and adjust for long boundaries
	memory_base = (uint8 *) malloc(total_free_memory);

	if (!memory_base) {	//could not grab the memory
		error("Init_memory_manager() couldn't malloc %d bytes", total_free_memory);
	}

	// the original malloc address
	free_memman = memory_base;

#if 0
	// FIXME: I don't think it's necessary to force alignment here,
	// because memory_base is the address returned by malloc(), and
	// according to my C book "every allocated region from malloc must
	// be aligned for any type".

	// force to long word boundary
	memory_base += 3;
	memory_base = (uint8 *) ((uint32) memory_base & 0xfffffffc);	// ** was (int)memory_base
	// total_free_memory -= 3;	//play safe
#endif

	// set all but first handle to unused
	for (j = 1; j < MAX_mem_blocks; j++)
		mem_list[j].state = MEM_null;

	// total used (free, locked or floating)
	total_blocks = 1;

	mem_list[0].ad = memory_base;
	mem_list[0].state = MEM_free;
	mem_list[0].age = 0;
	mem_list[0].size = total_free_memory;
	mem_list[0].parent = -1;		// we are base - for now
	mem_list[0].child = -1;			// we are the end as well
	mem_list[0].uid = UID_memman;		// init id

	base_mem_block = 0;			// for now
}

// This is the low-level memory allocator

mem *Talloc(uint32 size, uint32 type, uint32 unique_id) {
	// allocate a block of memory - locked or float

	// returns 0 if fails to allocate the memory
	// or a pointer to a mem structure

	int32 nu_block;
	uint32 spawn = 0;
	uint32 slack;

	// we must first round the size UP to a dword, so subsequent blocks
	// will start dword alligned

	size += 3;		// move up
	size &= 0xfffffffc;	// and back down to boundary

	// find a free block large enough

	// the defragger returns when its made a big enough block. This is
	// a good time to defrag as we're probably not doing anything super
	// time-critical at the moment

	if ((nu_block = Defrag_mem(size)) == -1) {
		// error - couldn't find a big enough space
		return 0;
	}

	// an exact fit?
	if (mem_list[nu_block].size == size) {
		// no new block is required as the fit is perfect
		mem_list[nu_block].state = type;    // locked or float
		mem_list[nu_block].size = size;	    // set to the required size
		mem_list[nu_block].uid = unique_id; // an identifier

#ifdef	MEMDEBUG
		Mem_debug();
#endif

		return &mem_list[nu_block];
	}

	// nu_block is the free block to split, forming our locked/float block
	// with a new free block in any remaining space

	// If our child is free then is can expand downwards to eat up our
	// chopped space this is good because it doesn't create an extra block
	// so keeping the block count down.
	//
	// Why? Imagine you Talloc 1000k, then free it. Now keep allocating 10
	// bytes less and freeing again you end up with thousands of new free
	// mini blocks. This way avoids that as the free child keeps growing
	// downwards.

	if (mem_list[nu_block].child != -1 && mem_list[mem_list[nu_block].child].state == MEM_free) {
		// our child is free
		// the spare memory is the blocks current size minus the
		// amount we're taking

		slack = mem_list[nu_block].size - size;

		mem_list[nu_block].state = type;    // locked or float
		mem_list[nu_block].size = size;	    // set to the required size
		mem_list[nu_block].uid = unique_id; // an identifier

		// child starts after us
		mem_list[mem_list[nu_block].child].ad = mem_list[nu_block].ad + size;
		// child's size increases
		mem_list[mem_list[nu_block].child].size += slack;

		return &mem_list[nu_block];
	}

	// otherwise we spawn a new block after us and before our child - our
	// child being a proper block that we cannot change

	// we remain a child of our parent
	// we spawn a new child and it inherits our current child

	// find a NULL slot for a new block

	while (mem_list[spawn].state != MEM_null && spawn!=MAX_mem_blocks)
		spawn++;

	if (spawn == MAX_mem_blocks) {
		// run out of blocks - stop the program. this is a major blow
		// up and we need to alert the developer
		// Lets get a printout of this
		Mem_debug();
		error("Out of mem blocks in Talloc()");
	}

	mem_list[spawn].state = MEM_free;	// new block is free
	mem_list[spawn].uid = UID_memman;	// a memman created bloc

	// size of the existing parent free block minus the size of the new
	// space Talloc'ed.

	mem_list[spawn].size = mem_list[nu_block].size - size;

	// IOW the remaining memory is given to the new free block

	// we start 1 byte after the newly allocated block
	mem_list[spawn].ad = mem_list[nu_block].ad + size;

	// the spawned child gets it parent - the newly allocated block
	mem_list[spawn].parent = nu_block;

	// the new child inherits the parents old child (we are its new
	// child "Waaaa")
	mem_list[spawn].child = mem_list[nu_block].child;

	// is the spawn the end block?
	if (mem_list[spawn].child != -1) {
		// the child of the new free-spawn needs to know its new parent
		mem_list[mem_list[spawn].child].parent = spawn;
	}

	mem_list[nu_block].state = type;	// locked or float
	mem_list[nu_block].size = size;		// set to the required size
	mem_list[nu_block].uid = unique_id;	// an identifier

	// the new blocks new child is the newly formed free block
	mem_list[nu_block].child = spawn;

	//we've brought a new block into the world. Ahhh!
	total_blocks++;

#ifdef	MEMDEBUG
	Mem_debug();
#endif

	return &mem_list[nu_block];
}

void Free_mem(mem *block) {
	// kill a block of memory - which was presumably floating or locked
	// once you've done this the memory may be recycled

	block->state = MEM_free;
	block->uid = UID_memman;	// belongs to the memory manager again

#ifdef	MEMDEBUG
	Mem_debug();
#endif
}

void Float_mem(mem *block) {
	// set a block to float
	// wont be trashed but will move around in memory

	block->state = MEM_float;

#ifdef	MEMDEBUG
	Mem_debug();
#endif
}

void Lock_mem(mem *block) {
	// set a block to lock
	// wont be moved - don't lock memory for any longer than necessary
	// unless you know the locked memory is at the bottom of the heap

	// can't move now - this block is now crying out to be floated or
	// free'd again

	block->state = MEM_locked;

#ifdef	MEMDEBUG
	Mem_debug();
#endif
}

int32 Defrag_mem(uint32 req_size) {
	// moves floating blocks down and/or merges free blocks until a large
	// enough space is found or there is nothing left to do and a big
	// enough block cannot be found we stop when we find/create a large
	// enough block - this is enough defragging.

	int32 cur_block;	// block 0 remains the parent block
	int32 original_parent,child, end_child;
	uint32 j;
	uint32 *a;
	uint32 *b;

	// cur_block = base_mem_block;	//the mother of all parents
	cur_block = suggestedStart;

	do {
		// is current block a free block?
		if (mem_list[cur_block].state == MEM_free) {
			if (mem_list[cur_block].size >= req_size) {
				// this block is big enough - return its id
				return cur_block;
			}

			// the child is the end block - stop if the next block
			// along is the end block
			if (mem_list[cur_block].child == -1) {
				// no luck, couldn't find a big enough block
				return -1;
			}

			// current free block is too small, but if its child
			// is *also* free then merge the two together

			if (mem_list[mem_list[cur_block].child].state == MEM_free) {
				// ok, we nuke the child and inherit its child
				child = mem_list[cur_block].child;

				// our size grows by the size of our child
				mem_list[cur_block].size += mem_list[child].size;

				// our new child is our old childs, child
				mem_list[cur_block].child = mem_list[child].child;

				// not if the chld we're nuking is the end
				// child (it has no child)

				if (mem_list[child].child != -1) {
					// the (nuked) old childs childs
					// parent is now us
					mem_list[mem_list[child].child].parent = cur_block;
				}

				// clean up the nuked child, so it can be used
				// again
				mem_list[child].state = MEM_null;

				total_blocks--;
			} else if (mem_list[mem_list[cur_block].child].state == MEM_float) {
				// current free block is too small, but if its
				// child is a float then we move the floating
				// memory block down and the free up but,
				// parent/child relationships must be such
				// that the memory is all continuous between
				// blocks. ie. a childs memory always begins 1
				// byte after its parent finishes. However, the
				// positions in the memory list may become
				// truly random, but, any particular block of
				// locked or floating memory must retain its
				// position within the mem_list - the float
				// stays a float because the handle/pointer
				// has been passed back
				//
				// what this means is that when the physical
				// memory of the foat moves down (and the free
				// up) the child becomes the parent and the
				// parent the child but, remember, the parent
				// had a parent and the child another child -
				// these swap over too as the parent/child swap
				// takes place - phew.

				// our child is currently floating
				child = mem_list[cur_block].child;

				// move the higher float down over the free
				// block
				// memcpy(mem_list[cur_block].ad, mem_list[child].ad, mem_list[child].size);

				a = (uint32*) mem_list[cur_block].ad;
				b = (uint32*) mem_list[child].ad;

				for (j = 0; j < mem_list[child].size / 4; j++)
					*(a++) = *(b++);

				// both *ad's change
				// the float is now where the free was and the
				// free goes up by the size of the float
				// (which has come down)

				mem_list[child].ad = mem_list[cur_block].ad;
				mem_list[cur_block].ad += mem_list[child].size;

				// the status of the mem_list blocks must
				// remain the same, so...

				// our child gets this when we become its
				// child and it our parent
				original_parent = mem_list[cur_block].parent;

				// the free's child becomes its parent
				mem_list[cur_block].parent = child;

				// the new child inherits its previous childs
				// child
				mem_list[cur_block].child = mem_list[child].child;

				// save this - see next line
				end_child = mem_list[child].child;

				// the floats parent becomes its child
				mem_list[child].child = cur_block;
				mem_list[child].parent = original_parent;

				// if the child had a child
				if (end_child != -1) {
					// then its parent is now the new child
					mem_list[end_child].parent = cur_block;
				}

				// if the base block was the true base parent
				if (original_parent == -1) {
					// then the child that has moved down
					// becomes the base block as it sits
					// at the lowest possible memory
					// location
					base_mem_block = child;
				} else {
					// otherwise the parent of the current
					// free block - that is now the child
					// - gets a new child, that child
					// being previously the child of the
					// child of the original parent
					mem_list[original_parent].child = child;
				}
			} else { // if (mem_list[mem_list[cur_block].child].state == MEM_lock)
				// the child of current is locked - move to it
				// move to next one along - either locked or
				// END
				cur_block=mem_list[cur_block].child;
			}
		} else {
			// move to next one along, the current must be
			// floating, locked, or a NULL slot
			cur_block = mem_list[cur_block].child;
		}
	} while (cur_block != -1);	// while the block we've just done is not the final block

	return -1;	//no luck, couldn't find a big enough block
}

void Mem_debug(void) {
	// gets called with Talloc, Mem_free, Mem_lock & Mem_float if
	// MEMDEBUG has been #defined otherwise can be called at any time
	// anywhere else

	int j;
	char inf[][20] = {
		{ "MEM_null" },
		{ "MEM_free" },
		{ "MEM_locked" },
		{ "MEM_float" }
	};

	debug(5, "base %d total %d", base_mem_block, total_blocks);

	// first in mem list order
	for (j = 0; j < MAX_mem_blocks; j++) {
		if (mem_list[j].state == MEM_null)
			debug(5, "%d- NULL", j);
		else
			debug(5, "%d- state %s, ad %d, size %d, p %d, c %d, id %d",
				j, inf[mem_list[j].state], mem_list[j].ad,
				mem_list[j].size, mem_list[j].parent,
				mem_list[j].child, mem_list[j].uid);
	}

	// now in child/parent order
	j = base_mem_block;
	do {
		debug(5, " %d- state %s, ad %d, size %d, p %d, c %d", j, 
			inf[mem_list[j].state], mem_list[j].ad,
			mem_list[j].size, mem_list[j].parent,
			mem_list[j].child, mem_list[j].uid);

		j = mem_list[j].child;
	} while (j != -1);
}

mem *Twalloc(uint32 size, uint32 type, uint32 unique_id) {
	// the high level Talloc
	// can ask the resman to remove old resources to make space - will
	// either do it or halt the system

	mem *membloc;
	int j;
	uint32 free = 0;

	while (VirtualDefrag(size)) {
		// trash the oldest closed resource
		if (!res_man.Help_the_aged_out()) {
			error("Twalloc ran out of memory: size=%d type=%d unique_id=%d", size, type, unique_id);
		}
	}

	membloc = Talloc(size, type, unique_id);

	if (membloc == 0) {
		error("Talloc failed to get memory VirtualDefrag said was there");
	}

	j = base_mem_block;
	do {

		if (mem_list[j].state == MEM_free)
			free += mem_list[j].size;

		j = mem_list[j].child;
	} while (j != -1);

	// return the pointer to the memory
	return membloc;
}

// Maximum allowed wasted memory.
#define MAX_WASTAGE 51200

int32 VirtualDefrag(uint32 size) {
	// Virutually defrags memory...
	//
	// Used to determine if there is potentially are large enough free
	// block available is the real defragger was allowed to run.
	//
	// The idea being that Twalloc will call this and help_the_aged_out
	// until we indicate that it is possible to obtain a large enough
	// free block. This way the defragger need only run once to yield the
	// required block size.
	//
	// The reason for its current slowness is that the defragger is
	// potentially called several times, each time shifting upto 20Megs
	// around, to obtain the required free block.

	int32 cur_block;	
	uint32 currentBubbleSize = 0;

	cur_block = base_mem_block;
	suggestedStart = base_mem_block;

	do {
		if (mem_list[cur_block].state == MEM_free) {
			// Add a little intelligence. At the start the oldest
			// resources are at the bottom of the tube. However
			// there will be some air at the top. Thus bubbles
			// will be created at the bottom and float to the
			// top. If we ignore the top gap then a large enough
			// bubble will form lower down the tube. Thus less
			// memory will need to be shifted.

			if (mem_list[cur_block].child != -1)
				currentBubbleSize += mem_list[cur_block].size;
			else if (mem_list[cur_block].size > MAX_WASTAGE)
				currentBubbleSize += mem_list[cur_block].size;

			if (currentBubbleSize >= size)
				return 0;
		} else if (mem_list[cur_block].state == MEM_locked) {
			currentBubbleSize = 0;
			// Any free block of the correct size will be above
			// this locked block.
			suggestedStart = mem_list[cur_block].child;
		}

		cur_block = mem_list[cur_block].child;
	} while (cur_block != -1);	

	return 1;
}
