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

#ifndef	MEMORY_H
#define	MEMORY_H

#include "common/scummsys.h"

typedef	struct {
	uint32 state;
	uint32 age;	// *not used*
	uint32 size;
	int32 parent;	// who is before us
	int32 child;	// who is after us
	// id of a position in the resList or some other unique id - for the
	// visual display only
	uint32 uid;
	uint8 *ad;
} mem;

#define	MEM_null			0	// null
#define	MEM_free			1
#define	MEM_locked			2
#define	MEM_float			3

//---------------------------------------
// MEMORY BLOCKS

#define	MAX_mem_blocks			999

// maintain at a good 50% higher than the
// highest recorded value from the on-screen info
//---------------------------------------

#define	UID_memman			0xffffffff
#define	UID_NULL			0xfffffffe	// FREE
#define	UID_font			0xfffffffd
#define	UID_temp			0xfffffffc
#define	UID_decompression_buffer	0xfffffffb
#define	UID_shrink_buffer		0xfffffffa
#define	UID_con_sprite			0xfffffff9
#define	UID_text_sprite			0xfffffff8
#define	UID_walk_anim			0xfffffff7
#define	UID_savegame_buffer		0xfffffff6
#define UID_restoregame_buffer		0xfffffff5

void Init_memory_manager(void);
void Close_memory_manager(void);				// Tony2Oct96
mem *Twalloc(uint32 size, uint32 type, uint32 unique_id);	// high level
void Free_mem(mem *block);
void Float_mem(mem *block);
void Lock_mem(mem *block);
void Mem_debug(void);
void Visual_mem_display(void);
int32 Defrag_mem(uint32 req_size);				// Tony10Apr96

extern uint32 total_blocks;
extern uint32 base_mem_block;
extern mem mem_list[MAX_mem_blocks];
extern uint32 total_free_memory;

#endif
