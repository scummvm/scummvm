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

#ifndef _SCI_HEAP_H
#define _SCI_HEAP_H

#include "sci/include/resource.h"

namespace Sci {

#define SCI_HEAP_SIZE 0x10000

typedef guint16 heap_ptr;

typedef struct {
	byte *start;
	byte *base;
	unsigned int first_free;
	int old_ff;
} heap_t;

heap_t *heap_new();
/* Allocates a new heap.
** Parameters: (void)
** Returns   : (heap_t *) A new 0xffff-sized heap
*/

void heap_del(heap_t *h);
/* Frees an allocated heap
** Parameters: (heap_t *) h: The heap to unallocate
** Returns   : (void)
*/

int heap_meminfo(heap_t *h);
/* Returns the total number of free bytes on the heap
** Parameters: (heap_t *) h: The heap to check
** Returns   : (int) The total free space in bytes
*/

int heap_largest(heap_t *h);
/* Returns the block size of the largest free block on the heap
** Parameters: (heap_t *) h: The heap to check
** Returns   : (int) The size of the largest free block
*/

heap_ptr heap_allocate(heap_t *h, int size);
/* Allocates memory on a heap.
** Parameters: (heap_t *) h: The heap to work with
**             (int) size: The block size to allocate
** Returns   : (heap_ptr): The heap pointer to the new block, or 0 on failure
*/

void heap_free(heap_t *h, unsigned int m);
/* Frees allocated heap memory.
** Parameters: (heap_t *) h: The heap to work with
**             (int) m: The handle at which memory is to be unallocated
** Returns   : (void)
** This function automatically prevents fragmentation from happening.
*/

void save_ff(heap_t *h);
/* Stores the current first free position
** Parameters: (heap_t *) h: The heap which is to be manipulated
** Returns   : (void)
** This function can be used to save the heap state for later restoration (see
** the next function)
*/

void restore_ff(heap_t *h);
/* Restores the first free heap state
** Parameters: (heap_t *) h: The heap to restore
** Returns   : (void)
** Restoring the first free state will reset the heap to the position stored
** when save_ff() was called, if and only if none of the blocks allocated before
** save_ff() was called was ever freed ("ever" includes "before save_ff() was
** called").
*/

void heap_dump_free(heap_t *h);
/* Dumps debugging information about the stack
** Parameters: (heap_t *) h: The heap to check
** Returns   : (void)
*/

void heap_dump_all(heap_t *h);
/* Dumps all allocated/unallocated zones on the heap
** Parameters: (heap_t *) h: The heap to check
** Returns   : (void)
*/

} // End of namespace Sci

#endif // !_SCI_HEAP_H
