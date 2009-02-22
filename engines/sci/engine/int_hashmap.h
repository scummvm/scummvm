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

#ifndef INT_HASHMAP_H
#define INT_HASHMAP_H

#include "common/scummsys.h"

namespace Sci {

/*
 * Defines a hash map that maps values to small integers.
 * The name "hashmap" here is a bit misleading, because the
 * value associated to each key are generated automatically,
 * using the lowest "unused" value.
 */


// Assumes that the ints are relatively evenly distributed

#define DCS_INT_HASH_MAX 255


struct int_hash_map_t {
	struct node_t {
		int name;
		int value;
		node_t *next;
	};

	int base_value;  // Starts at zero, counts upwards
	node_t *nodes[DCS_INT_HASH_MAX + 1];
	node_t *holes; /* List of freed entries to minimize
				     ** memory operations and modifications
				     ** to base_value  */

	void free_node_recursive(node_t *node);

public:
	int_hash_map_t();
	~int_hash_map_t();

	/**
	 * Checks whether a value is in the map, adds it if neccessary.
	 * @param value		The value to check for/add
	 * @param add		Whether to add the value if it's not in there
	 * @param was_added	Set to non-zero iff the value is new, ignored if NULL.
	 * @return The new (or old) index, or -1 if add was zero and
	 *                   the value couldn't be found
	 */
	int check_value(int value, bool add, char *was_added = 0);

	/**
	 * Removes a value from the hash map.
	 * @param value		The value to remove
	 * @return	The ID of the value, or -1 if it wasn't present
	 */
	int remove_value(int value);

};

typedef int_hash_map_t *int_hash_map_ptr;

} // End of namespace Sci

#endif // INT_HASHMAP_H
