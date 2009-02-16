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


struct int_hash_map_node_t {
	int name;
	int value;
	int_hash_map_node_t *next;
};


struct int_hash_map_t {
	int base_value;  /* Starts at zero, counts upwards */
	int_hash_map_node_t *nodes[DCS_INT_HASH_MAX+1];
	int_hash_map_node_t *holes; /* List of freed entries to minimize
				     ** memory operations and modifications
				     ** to base_value  */
};

typedef int_hash_map_t *int_hash_map_ptr;


int_hash_map_t *
new_int_hash_map(void);
/* Creates a new hash map for the specified int
** Parameters: (void)
** Returns   : (int_hash_map_t *) The newly allocated hash map
*/

void
free_int_hash_map(int_hash_map_ptr map);
/* Frees the specified hash map
** Parameters: (int_hash_map_t *) map: The map to free
** Returns   : (void)
*/

void
apply_to_int_hash_map(int_hash_map_ptr map, void *param, void (*note)(void *param, int name, int value));
/* Iterates over all entries in the hash map and invokes 'note'
** Parameters: (int_hash_map_t *) map: The map to iterate over
**             (void *) param: Some parameter to pass to 'note'
**             ((voidptr * int * value) -> void) note: The callback to invoke for each entry
*/

int
int_hash_map_check_value(int_hash_map_ptr map, int value, char add, char *was_added);
/* Checks whether a value is in the map, adds it if neccessary
** Parameters: (int_hash_map_t *) map: The map to look in/modify
**             (int) value: The value to check for/add
**             (char) add: Whether to add the value if it's not in there
**             (char *) was_added: Set to non-zero iff the value is new
**                                 Ignored if NULL.
** Returns   : (int) The new (or old) index, or -1 if add was zero and
**                   the value couldn't be found
** If MUST_FREE is defined and add is set but the value was already in
** there, the value is freed.
*/

int
int_hash_map_remove_value(int_hash_map_ptr map, int value);
/* Removes a value from the hash map
** Parameters: (int_hash_map_t *) map: The map to remove from
**             (int) value: The value to remove
** Returns   : (int) The ID of the value, or -1 if it wasn't presen
*/


#endif /* INT_HASHMAP_H */
