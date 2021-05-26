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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/pool.h"

namespace Saga2 {

_Pool::_Pool(void *nodeArray, int nodes, size_t nodeSize) {
	int     i;

	//  Link all of the nodes into the free list
	for (i = 0; i < nodes; i++) {
		free.addTail(*((DNode *)nodeArray));
		nodeArray = (uint8 *)nodeArray + nodeSize;
	}
}

//----------------------------------------------------------------------
//	Place a non-allocated node on the allocated list and return its
//	address

void *_Pool::_alloc(void) {
	DNode   *node = free.remHead();

	if (node != NULL) list.addTail(*node);

	return node;
}

//----------------------------------------------------------------------
//	Place an allocated node back on the non-allocated list

void _Pool::_free(void *p) {
	DNode   *node = (DNode *)p;

	node->remove();
	free.addTail(*node);
}

} // end of namespace Saga2
