/***************************************************************************
 hashmap.h Copyright (C) 2001 Christoph Reichenbach


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
/* Defines a hash map that maps values to small integers.
** Preprocessor parameters:
** TYPE: The type to hash
** HASH_MAX: Maximum hash value
** HASH(x): Hashes a value of type TYPE to an int from 0 to HASH_MAX
** COMP(x, y): Compares x, y, returns zero(!) iff they are equal
** MUST_FREE: Define if 'lost' values are simple pointers and should be freed
** DUPLICATOR: Usually needed when MUST_FREE is being used, means that
**             new values must be dup'd before being added.
**             Define it to the name of the function to use for duplication
**             (e.g. '#define DUPLICATOR strdup').
*/

/**-- WARNING!!! --**/

/* This file uses a lot of dark magic (aka preprocessor macros) to define
** its semantics. It is not to be taken lightly. It is not to be taken
** as an example for good programming. Most importantly, it is not to
** be read by mere mortals, except for the author and other people who
** have undergone appropriate psychological preparation. Keep out of
** reach of small children. Do not shake. Do not put in a microwave. Do
** not feed to animals.
*/

/* OK, it's not that bad. */


#ifndef HASH_MAX
#  error "Must define the maximum hash value (HASH_MAX) before including hashmap.h!"
#endif

#ifndef HASH
#  error "Must define the hash function (HASH) before including hashmap.h!"
#endif

#ifndef COMP
#  error "Must define a comparison function (COMP) before including hashmap.h!"
#endif


#define DECLARE_STRUCTS(TYPE) \
typedef struct _##TYPE##_hash_map_node {                                    \
	TYPE name;                                                          \
	int value;                                                          \
	struct _##TYPE##_hash_map_node *next;                               \
} TYPE##_hash_map_node_t;                                                   \
                                                                            \
                                                                            \
typedef struct {                                                            \
	int base_value;  /* Starts at zero, counts upwards */               \
	TYPE##_hash_map_node_t *nodes[HASH_MAX+1];                          \
	TYPE##_hash_map_node_t *holes; /* List of freed entries to minimize \
				     ** memory operations and modifications \
				     ** to base_value  */                   \
} TYPE##_hash_map_t, *TYPE##_hash_map_ptr;


#define DECLARE_FUNCTIONS(TYPE) \
                                                                            \
TYPE##_hash_map_t *                                                         \
new_##TYPE##_hash_map(void);                                                \
/* Creates a new hash map for the specified TYPE                            \
** Parameters: (void)                                                       \
** Returns   : (TYPE##_hash_map_t *) The newly allocated hash map           \
*/                                                                          \
                                                                            \
void                                                                        \
free_##TYPE##_hash_map(TYPE##_hash_map_ptr map);                            \
/* Frees the specified hash map                                             \
** Parameters: (TYPE##_hash_map_t *) map: The map to free                   \
** Returns   : (void)                                                       \
*/                                                                          \
														    \
void														    \
apply_to_##TYPE##_hash_map(TYPE##_hash_map_ptr map, void *param, void (*note) (void *param, TYPE name, int value)); \
/* Iterates over all entries in the hash map and invokes 'note'							    \
** Parameters: (TYPE##_hash_map_t *) map: The map to iterate over						    \
**             (void *) param: Some parameter to pass to 'note'							    \
**             ((voidptr * TYPE * value) -> void) note: The callback to invoke for each entry			    \
*/                                                                            					    \
                                                                            \
int                                                                         \
TYPE##_hash_map_check_value(TYPE##_hash_map_ptr map, TYPE value, char add, char *was_added);  \
/* Checks whether a value is in the map, adds it if neccessary              \
** Parameters: (TYPE##_hash_map_t *) map: The map to look in/modify         \
**             (TYPE) value: The value to check for/add                     \
**             (char) add: Whether to add the value if it's not in there    \
**             (char *) was_added: Set to non-zero iff the value is new     \
**                                 Ignored if NULL.                         \
** Returns   : (int) The new (or old) index, or -1 if add was zero and      \
**                   the value couldn't be found                            \
** If MUST_FREE is defined and add is set but the value was already in      \
** there, the value is freed.                                               \
*/                                                                          \
                                                                            \
int                                                                         \
TYPE##_hash_map_remove_value(TYPE##_hash_map_ptr map, TYPE value);          \
/* Removes a value from the hash map                                        \
** Parameters: (TYPE##_hash_map_t *) map: The map to remove from            \
**             (TYPE) value: The value to remove                            \
** Returns   : (int) The ID of the value, or -1 if it wasn't presen         \
*/




