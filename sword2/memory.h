/* Copyright (C) 1994-2004 Revolution Software Ltd
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

#ifndef	MEMORY_H
#define	MEMORY_H

namespace Sword2 {

struct Memory {
	uint32 state;
	uint32 age;	// *not used*
	uint32 size;
	int32 parent;	// who is before us
	int32 child;	// who is after us
	// id of a position in the _resList or some other unique id - for the
	// visual display only
	uint32 uid;
	uint8 *ad;
};

enum {
	MEM_null			= 0,	// null
	MEM_free			= 1,
	MEM_locked			= 2,
	MEM_float			= 3
};

//---------------------------------------
// MEMORY BLOCKS

#define	MAX_mem_blocks			999

// maintain at a good 50% higher than the
// highest recorded value from the on-screen info
//---------------------------------------

enum {
	UID_memman			= 0xffffffff,
	UID_NULL			= 0xfffffffe,	// FREE
	UID_font			= 0xfffffffd,
	UID_temp			= 0xfffffffc,
	UID_decompression_buffer	= 0xfffffffb,
	UID_shrink_buffer		= 0xfffffffa,
	UID_con_sprite			= 0xfffffff9,
	UID_text_sprite			= 0xfffffff8,
	UID_walk_anim			= 0xfffffff7,
	UID_savegame_buffer		= 0xfffffff6,
	UID_restoregame_buffer		= 0xfffffff5
};

class MemoryManager {
private:
	Sword2Engine *_vm;

	// Address of init malloc to be freed later
	uint8 *_freeMemman;

	uint32 _totalFreeMemory;
	uint32 _totalBlocks;

	// Start position of the defragger as indicated by its sister,
	// VirtualDefrag.
	int32 _suggestedStart;

	Memory *lowLevelAlloc(uint32 size, uint32 type, uint32 unique_id);
	int32 defragMemory(uint32 req_size);

	// Used to determine if the required size can be obtained if the
	// defragger is allowed to run.
	int32 virtualDefrag(uint32 size);

	// Debugging functions
	void debugMemory(void);
	const char *fetchOwner(uint32 uid);

public:
	// List of defined memory handles - each representing a block of memory
	Memory _memList[MAX_mem_blocks];
	uint32 _baseMemBlock;

	MemoryManager(Sword2Engine *vm);
	~MemoryManager(void);

	int32 ptrToInt(const uint8 *p);
	uint8 *intToPtr(int32 n);

	Memory *allocMemory(uint32 size, uint32 type, uint32 unique_id);
	void freeMemory(Memory *block);
	void floatMemory(Memory *block);
	void lockMemory(Memory *block);

	// Debugging function
	void displayMemory(void);
	void memoryString(char *string);
};

} // End of namespace Sword2

#endif
