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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_POOL_H
#define SAGA2_POOL_H

#include "saga2/dlist.h"

namespace Saga2 {

/* ===================================================================== *
   _Pool class
 * ===================================================================== */

//	This is the base class for the resource pool template classes.  This
//	class contains all of the non-inline functions in order to keep the
//	template instantiations from generating unneeded redundant code.

class _Pool {
	DList       free,               //  List of non-allocated nodes
	            list;               //  List of allocated nodes

protected:
	//  Constructor
	_Pool(void *nodeArray, int nodes, size_t nodeSize);

	//  Place a non-allocated node on the allocated list and return
	//  its address
	void *_alloc(void);

	//  Place an allocated node back on the non-allocated list
	void _free(void *p);
};

/* ===================================================================== *
   Pool class template
 * ===================================================================== */

//	This is the direct base class of the RPool class.  This class is aware
//	of the type of the objects in the pool but is unaware of the number of
//	available nodes and is not responsible for allocating the node array.
//	This class is useful for creating a pointer or reference to an RPool
//	without having to specify the size of the node array.

template <class T>
class Pool : public _Pool {
protected:

	//  The base node type
	struct PoolNode : public DNode {
		uint8       buf[sizeof(T)];    //  Defines a buffer large enough to
		//  contain an object of type T.
	};

	//  Constructor
	Pool(void *nodeArray, int nodes) :
		_Pool(nodeArray, nodes, sizeof(PoolNode)) {
	}

public:

	//  Allocate a new object of type T and return its address
	void *alloc(void) {
		PoolNode *node = (PoolNode *)_alloc();
		return node != NULL ? node->buf : NULL;
	}

	//  Deallocate an object of type T given its address
	void free(void *p) {
		_free((uint8 *)p - offsetof(PoolNode, buf));
	}
};

/* ===================================================================== *
   RPool class template
 * ===================================================================== */

//	The is the resource allocator pool which may be instantiated.  This class
//	is responsible for allocating the node array and calling the base
//	class constructor.  All other functionality is provided through the
//	base classes.

template <class T, int s>
class RPool : public Pool<T> {
	typename Pool<T>::PoolNode    array[s];         //  The node array

public:
	//  Constructor
	RPool(void) : Pool<T>(array, s) {}
};

} // end of namespace Saga2

#endif
