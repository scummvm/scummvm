/***************************************************************************
 int_hashmap.h Copyright (C) 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#ifndef INT_HASHMAP_H
#define INT_HASHMAP_H

#include "common/scummsys.h"


/*
 * Defines a hash map that maps values to small integers.
 * The name "hashmap" here is a bit misleading, because the
 * value associated to each key are generated automatically,
 * using the lowest "unused" value.
 */


/* Assumes that the ints are relatively evenly distributed */

#define DCS_INT_HASH_MAX 255


struct int_hash_map_t {
	struct node_t {
		int name;
		int value;
		node_t *next;
	};

	int base_value;  /* Starts at zero, counts upwards */
	node_t *nodes[DCS_INT_HASH_MAX+1];
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


#endif /* INT_HASHMAP_H */
