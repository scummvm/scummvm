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
 */

#ifndef ARCHETYPE_LINKED_LIST
#define ARCHETYPE_LINKED_LIST

#include "common/list.h"

namespace Glk {
namespace Archetype {

struct NodeType {
	void *data;
	int key;
	NodeType *next;

	NodeType() : data(nullptr), key(0), next(nullptr) {}
};
typedef NodeType *NodePtr;

typedef NodePtr ListType;

/**
 * Allocates a header node and points it to itself
 */
extern void new_list(ListType &the_list);

/**
 * Throws away all the memory that makes up an entire list structure. Is a "shallow dispose";
 * i.e. only disposes of the structure, not the data.
 */
extern void dispose_list(ListType &the_list);

/**
 * Iterates through the given list
 */
extern bool iterate_list(ListType &the_list, NodePtr &index);

/**
 * Appends a new item to the list
 */
extern void append_to_list(ListType &the_list, NodePtr the_node);

/**
 * Permits a linked list to be indexed like an array in O(N) time.
 */
extern NodePtr index_list(ListType &the_list, int number);

/**
 * Ordered insert; average time O(N/2).  Inserts in descending order
 */
extern void insert_item(ListType &the_list, NodePtr the_item);

/**
 * Given a list and a key, finds the first item in the list corresponding to that key.
 * Expects that the elements have been sorted in descending order
 */
extern NodePtr find_item(ListType &the_list, int the_key);

} // End of namespace Archetype
} // End of namespace Glk

#endif
