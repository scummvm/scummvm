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

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"

namespace Glk {
namespace Adrift {

/* Assorted definitions and constants. */
static const sc_uint PROP_MAGIC = 0x7927b2e0;
enum {
	PROP_GROW_INCREMENT = 32,
	MAX_INTEGER_KEY = 65535,
	NODE_POOL_CAPACITY = 512
};
static const sc_char NUL = '\0';

/* Properties trace flag. */
static sc_bool prop_trace = FALSE;


/*
 * Property tree node definition, uses a child list representation for
 * fast lookup on indexed nodes.  Name is a variable type, as is property,
 * which is also overloaded to contain the child count for internal nodes.
 */
struct sc_prop_node_s {
	sc_vartype_t name;
	sc_vartype_t property;

	struct sc_prop_node_s **child_list;
};
typedef sc_prop_node_s sc_prop_node_t;
typedef sc_prop_node_t *sc_prop_noderef_t;

/*
 * Properties set structure.  This is a set of properties, on which the
 * properties functions operate (a properties "object").  Node string
 * names are held in a dictionary to help save space.  To avoid excessive
 * malloc'ing of nodes, new nodes are preallocated in pools.
 */
struct sc_prop_set_s {
	sc_uint magic;
	sc_int dictionary_length;
	sc_char **dictionary;
	sc_int node_pools_length;
	sc_prop_noderef_t *node_pools;
	sc_int node_count;
	sc_int orphans_length;
	void **orphans;
	sc_bool is_readonly;
	sc_prop_noderef_t root_node;
	sc_tafref_t taf;
};
typedef sc_prop_set_s sc_prop_set_t;


/*
 * prop_is_valid()
 *
 * Return TRUE if pointer is a valid properties set, FALSE otherwise.
 */
static sc_bool prop_is_valid(sc_prop_setref_t bundle) {
	return bundle && bundle->magic == PROP_MAGIC;
}


/*
 * prop_round_up()
 *
 * Round up a count of elements to the next block of grow increments.
 */
static sc_int prop_round_up(sc_int elements) {
	sc_int extended;

	extended = elements + PROP_GROW_INCREMENT - 1;
	return (extended / PROP_GROW_INCREMENT) * PROP_GROW_INCREMENT;
}


/*
 * prop_ensure_capacity()
 *
 * Ensure that capacity exists in a growable array for a given number of
 * elements, growing if necessary.
 *
 * Some libc's allocate generously on realloc(), and some not.  Those that
 * don't will thrash badly if we realloc() on each grow, so here we try to
 * realloc() in blocks of elements, and thus need to realloc() much less
 * frequently.
 */
static void *prop_ensure_capacity(void *array, sc_int old_size, sc_int new_size, sc_int element_size) {
	sc_int current, required;

	/*
	 * See if there's any resize necessary, that is, does the new size round up
	 * to a larger number of elements than the old size.
	 */
	current = prop_round_up(old_size);
	required = prop_round_up(new_size);
	if (required > current) {
		sc_byte *new_array, *start_clearing;

		/* Grow array to the required size, and zero new elements. */
		new_array = (sc_byte *)sc_realloc(array, required * element_size);
		start_clearing = new_array + current * element_size;
		memset(start_clearing, 0, (required - current) * element_size);

		return new_array;
	}

	/* No resize necessary. */
	return array;
}


/*
 * prop_trim_capacity()
 *
 * Trim an array allocation back to the bare minimum required.  This will
 * "unblock" the allocations of prop_ensure_capacity().  Once trimmed,
 * the array cannot ever be grown safely again.
 */
static void *prop_trim_capacity(void *array, sc_int size, sc_int element_size) {
	if (prop_round_up(size) > size)
		return sc_realloc(array, size * element_size);
	else
		return array;
}


/*
 * prop_compare()
 *
 * String comparison routine for sorting and searching dictionary strings.
 * The function has return type "int" to match the libc implementations of
 * bsearch() and qsort().
 */
static int prop_compare(const void *string1, const void *string2) {
	return strcmp(*(sc_char * const *) string1, *(sc_char * const *) string2);
}


/*
 * prop_dictionary_lookup()
 *
 * Find a string in the dictionary.  If the string is not present, the
 * function will add it.  The function returns the string's address, if
 * either added or already present.  Any new dictionary entry will
 * contain a malloced copy of the string passed in.
 */
static const sc_char *prop_dictionary_lookup(sc_prop_setref_t bundle, const sc_char *string) {
	sc_char *dict_string;

	/*
	 * Search the existing dictionary for the string.  Although not GNU libc,
	 * some libc's loop or crash when given a list of zero length, so we need to
	 * trap that here.
	 */
	if (bundle->dictionary_length > 0) {
		const sc_char *const *dict_search;

		dict_search = (const sc_char * const *)bsearch(&string, bundle->dictionary,
		              bundle->dictionary_length,
		              sizeof(bundle->dictionary[0]), prop_compare);
		if (dict_search)
			return *dict_search;
	}

	/* Not found, so copy the string for dictionary insertion. */
	dict_string = (sc_char *)sc_malloc(strlen(string) + 1);
	strcpy(dict_string, string);

	/* Extend the dictionary if necessary. */
	bundle->dictionary = (sc_char **)prop_ensure_capacity(bundle->dictionary,
	                     bundle->dictionary_length,
	                     bundle->dictionary_length + 1,
	                     sizeof(bundle->dictionary[0]));

	/* Add the new entry to the end of the dictionary array, and sort. */
	bundle->dictionary[bundle->dictionary_length++] = dict_string;
	qsort(bundle->dictionary,
	      bundle->dictionary_length,
	      sizeof(bundle->dictionary[0]), prop_compare);

	/* Return the address of the new string. */
	return dict_string;
}


/*
 * prop_new_node()
 *
 * Return the address of the next free properties node from the node pool.
 * Using a pool gives a performance boost; the number of properties nodes
 * for even a small game is large, and preallocating pools avoids excessive
 * malloc's of small individual nodes.
 */
static sc_prop_noderef_t prop_new_node(sc_prop_setref_t bundle) {
	sc_int node_index;
	sc_prop_noderef_t node;

	/* See if we need to create a new node pool. */
	node_index = bundle->node_count % NODE_POOL_CAPACITY;
	if (node_index == 0) {
		sc_int required;

		/* Extend the node pools array if necessary. */
		bundle->node_pools = (sc_prop_noderef_t *)prop_ensure_capacity(bundle->node_pools,
		                     bundle->node_pools_length,
		                     bundle->node_pools_length + 1,
		                     sizeof(bundle->
		                            node_pools[0]));

		/* Create a new node pool, and increment the length. */
		required = NODE_POOL_CAPACITY * sizeof(*bundle->node_pools[0]);
		bundle->node_pools[bundle->node_pools_length] = (sc_prop_noderef_t)sc_malloc(required);
		bundle->node_pools_length++;
	}

	/* Find the next node address, and increment the node counter. */
	node = bundle->node_pools[bundle->node_pools_length - 1] + node_index;
	bundle->node_count++;

	/* Return the new node. */
	return node;
}


/*
 * prop_find_child()
 *
 * Find a child node of the given parent whose name matches that passed in.
 */
static sc_prop_noderef_t prop_find_child(sc_prop_noderef_t parent, sc_int type, sc_vartype_t name) {
	/* See if this node has any children. */
	if (parent->child_list) {
		sc_int index_;
		sc_prop_noderef_t child = nullptr;

		/* Do the lookup based on name type. */
		switch (type) {
		case PROP_KEY_INTEGER:
			/*
			 * As with adding a child below, here we'll range-check an integer
			 * key just to make sure nobody has any unreal expectations of us.
			 */
			if (name.integer < 0)
				sc_fatal("prop_find_child: integer key cannot be negative\n");
			else if (name.integer > MAX_INTEGER_KEY)
				sc_fatal("prop_find_child: integer key is too large\n");

			/*
			 * For integer lookups, return the child at the indexed offset
			 * directly, provided it exists.
			 */
			if (name.integer >= 0 && name.integer < parent->property.integer) {
				child = parent->child_list[name.integer];
				return child;
			}
			break;

		case PROP_KEY_STRING:
			/* Scan children for a string name match. */
			for (index_ = 0; index_ < parent->property.integer; index_++) {
				child = parent->child_list[index_];
				if (strcmp(child->name.string, name.string) == 0)
					break;
			}

			/* Return child if we found a match. */
			if (index_ < parent->property.integer) {
				/*
				 * Before returning the child, try to improve future scans by
				 * moving the matched entry to index_ 0 -- this gives a key set
				 * sorted by recent usage, helpful as the same string key is
				 * used repeatedly in loops.
				 */
				if (index_ > 0) {
					memmove(parent->child_list + 1,
					        parent->child_list, index_ * sizeof(child));
					parent->child_list[0] = child;
				}
				return child;
			}
			break;

		default:
			sc_fatal("prop_find_child: invalid key type\n");
		}
	}

	/* No matching child found. */
	return NULL;
}


/*
 * prop_add_child()
 *
 * Add a new child node to the given parent.  Return its reference.  Set
 * needs to be passed so that string names can be added to the dictionary.
 */
static sc_prop_noderef_t prop_add_child(sc_prop_noderef_t parent, sc_int type,
		sc_vartype_t name, sc_prop_setref_t bundle) {
	sc_prop_noderef_t child;

	/* Not possible if growable allocations have been trimmed. */
	if (bundle->is_readonly)
		sc_fatal("prop_add_child: can't add to readonly properties\n");

	/* Create the new node. */
	child = prop_new_node(bundle);
	switch (type) {
	case PROP_KEY_INTEGER:
		child->name.integer = name.integer;
		break;
	case PROP_KEY_STRING:
		child->name.string = prop_dictionary_lookup(bundle, name.string);
		break;

	default:
		sc_fatal("prop_add_child: invalid key type\n");
	}

	/* Initialize property and child list to visible nulls. */
	child->property.voidp = NULL;
	child->child_list = NULL;

	/* Make a brief check for obvious overwrites. */
	if (!parent->child_list && parent->property.voidp)
		sc_error("prop_add_child: node overwritten, probable data loss\n");

	/* Add the child to the parent, position dependent on key type. */
	switch (type) {
	case PROP_KEY_INTEGER:
		/*
		 * Range check on integer keys, must be >= 0 for direct indexing to work,
		 * and we'll also apply a reasonableness constraint, to try to catch
		 * errors where string pointers are passed in as integers, which would
		 * otherwise lead to some extreme malloc() attempts.
		 */
		if (name.integer < 0)
			sc_fatal("prop_add_child: integer key cannot be negative\n");
		else if (name.integer > MAX_INTEGER_KEY)
			sc_fatal("prop_add_child: integer key is too large\n");

		/* Resize the parent's child list if necessary. */
		parent->child_list = (sc_prop_noderef_t *)prop_ensure_capacity(parent->child_list,
		                     parent->property.integer,
		                     name.integer + 1,
		                     sizeof(*parent->child_list));

		/* Update the child count if the new node increases it. */
		if (parent->property.integer <= name.integer)
			parent->property.integer = name.integer + 1;

		/* Store the child in its indexed list location. */
		parent->child_list[name.integer] = child;
		break;

	case PROP_KEY_STRING:
		/* Add a single entry to the child list, and resize. */
		parent->child_list = (sc_prop_noderef_t *)prop_ensure_capacity(parent->child_list,
		                     parent->property.integer,
		                     parent->property.integer + 1,
		                     sizeof(*parent->child_list));

		/* Store the child at the end of the list. */
		parent->child_list[parent->property.integer++] = child;
		break;

	default:
		sc_fatal("prop_add_child: invalid key type\n");
	}

	return child;
}


/*
 * prop_put()
 *
 * Add a property to a properties set.  Duplicate entries will replace
 * prior ones.
 *
 * Stores a value of variable type as a property.  The value type is one of
 * 'I', 'B', or 'S', for integer, boolean, and string values, held in the
 * first character of format.  The next two characters of format are "->",
 * and are syntactic sugar.  The remainder of format shows the key makeup,
 * with 'i' indicating integer, and 's' string key elements.  Example format:
 * "I->sssis", stores an integer, with a key composed of three strings, an
 * integer, and another string.
 */
void prop_put(sc_prop_setref_t bundle, const sc_char *format,
		sc_vartype_t vt_value, const sc_vartype_t vt_key[]) {
	sc_prop_noderef_t node;
	sc_int index_;
	assert(prop_is_valid(bundle));

	/* Format check. */
	if (!format || format[0] == NUL
	        || format[1] != '-' || format[2] != '>' || format[3] == NUL)
		sc_fatal("prop_put: format error\n");

	/* Trace property put. */
	if (prop_trace) {
		sc_trace("Property: put ");
		switch (format[0]) {
		case PROP_STRING:
			sc_trace("\"%s\"", vt_value.string);
			break;
		case PROP_INTEGER:
			sc_trace("%ld", vt_value.integer);
			break;
		case PROP_BOOLEAN:
			sc_trace("%s", vt_value.boolean ? "true" : "false");
			break;

		default:
			sc_trace("%p [invalid type]", vt_value.voidp);
			break;
		}
		sc_trace(", key \"%s\" : ", format);
		for (index_ = 0; format[index_ + 3] != NUL; index_++) {
			sc_trace("%s", index_ > 0 ? "," : "");
			switch (format[index_ + 3]) {
			case PROP_KEY_STRING:
				sc_trace("\"%s\"", vt_key[index_].string);
				break;
			case PROP_KEY_INTEGER:
				sc_trace("%ld", vt_key[index_].integer);
				break;

			default:
				sc_trace("%p [invalid type]", vt_key[index_].voidp);
				break;
			}
		}
		sc_trace("\n");
	}

	/*
	 * Iterate keys, finding matching child nodes at each level.  If no matching
	 * child is found, insert one and continue.
	 */
	node = bundle->root_node;
	for (index_ = 0; format[index_ + 3] != NUL; index_++) {
		sc_prop_noderef_t child;
		sc_int type;

		/*
		 * Search this level for a name matching the key.  If found, advance
		 * to that child node.  Otherwise, add the node to the tree, including
		 * the set so that the dictionary can be extended.
		 */
		type = format[index_ + 3];
		child = prop_find_child(node, type, vt_key[index_]);
		if (child)
			node = child;
		else
			node = prop_add_child(node, type, vt_key[index_], bundle);
	}

	/*
	 * Ensure that we're not about to overwrite an internal node child count.
	 */
	if (node->child_list)
		sc_fatal("prop_put: overwrite of internal node\n");

	/* Set our properties in the final node. */
	switch (format[0]) {
	case PROP_INTEGER:
		node->property.integer = vt_value.integer;
		break;
	case PROP_BOOLEAN:
		node->property.boolean = vt_value.boolean;
		break;
	case PROP_STRING:
		node->property.string = vt_value.string;
		break;

	default:
		sc_fatal("prop_put: invalid property type\n");
	}
}


/*
 * prop_get()
 *
 * Retrieve a property from a properties set.  Format stuff as above, except
 * with "->" replaced with "<-".  Returns FALSE if no such property exists.
 */
sc_bool prop_get(sc_prop_setref_t bundle, const sc_char *format, sc_vartype_t *vt_rvalue,
		const sc_vartype_t vt_key[]) {
	sc_prop_noderef_t node;
	sc_int index_;
	assert(prop_is_valid(bundle));

	/* Format check. */
	if (!format || format[0] == NUL
	        || format[1] != '<' || format[2] != '-' || format[3] == NUL)
		sc_fatal("prop_get: format error\n");

	/* Trace property get. */
	if (prop_trace) {
		sc_trace("Property: get, key \"%s\" : ", format);
		for (index_ = 0; format[index_ + 3] != NUL; index_++) {
			sc_trace("%s", index_ > 0 ? "," : "");
			switch (format[index_ + 3]) {
			case PROP_KEY_STRING:
				sc_trace("\"%s\"", vt_key[index_].string);
				break;
			case PROP_KEY_INTEGER:
				sc_trace("%ld", vt_key[index_].integer);
				break;

			default:
				sc_trace("%p [invalid type]", vt_key[index_].voidp);
				break;
			}
		}
		sc_trace("\n");
	}

	/*
	 * Iterate keys, finding matching child nodes at each level.  Stop if no
	 * matching child is found.
	 */
	node = bundle->root_node;
	for (index_ = 0; format[index_ + 3] != NUL; index_++) {
		sc_int type;

		/* Move node down to the matching child, NULL if no match. */
		type = format[index_ + 3 ];
		node = prop_find_child(node, type, vt_key[index_]);
		if (!node)
			break;
	}

	/* If key iteration halted because no child was found, return FALSE. */
	if (!node) {
		if (prop_trace)
			sc_trace("Property: ...get FAILED\n");

		return FALSE;
	}

	/*
	 * Enforce integer-only queries on internal nodes, since this is the only
	 * type of query that makes sense -- any other type is probably a mistake.
	 */
	if (node->child_list && format[0] != PROP_INTEGER)
		sc_fatal("prop_get: only integer gets on internal nodes\n");

	/* Return the properties of the final node. */
	switch (format[0]) {
	case PROP_INTEGER:
		vt_rvalue->integer = node->property.integer;
		break;
	case PROP_BOOLEAN:
		vt_rvalue->boolean = node->property.boolean;
		break;
	case PROP_STRING:
		vt_rvalue->string = node->property.string;
		break;

	default:
		sc_fatal("prop_get: invalid property type\n");
	}

	/* Complete tracing property get. */
	if (prop_trace) {
		sc_trace("Property: ...get returned : ");
		switch (format[0]) {
		case PROP_STRING:
			sc_trace("\"%s\"", vt_rvalue->string);
			break;
		case PROP_INTEGER:
			sc_trace("%ld", vt_rvalue->integer);
			break;
		case PROP_BOOLEAN:
			sc_trace("%s", vt_rvalue->boolean ? "true" : "false");
			break;

		default:
			sc_trace("%p [invalid type]", vt_rvalue->voidp);
			break;
		}
		sc_trace("\n");
	}
	return TRUE;
}


/*
 * prop_trim_node()
 * prop_solidify()
 *
 * Trim excess allocation from growable arrays, and fix the properties set
 * so that no further property insertions are allowed.
 */
static void prop_trim_node(sc_prop_noderef_t node) {
	/* End recursion on null or childless node. */
	if (node && node->child_list) {
		sc_int index_;

		/* Recursively trim allocation on children. */
		for (index_ = 0; index_ < node->property.integer; index_++)
			prop_trim_node(node->child_list[index_]);

		/* Trim allocation on this node. */
		node->child_list = (sc_prop_noderef_t *)prop_trim_capacity(node->child_list,
		                   node->property.integer,
		                   sizeof(*node->child_list));
	}
}

void prop_solidify(sc_prop_setref_t bundle) {
	assert(prop_is_valid(bundle));

	/*
	 * Trim back the dictionary, orphans, pools array, and every internal tree
	 * node.  The one thing _not_ to trim is the final node pool -- there are
	 * references to nodes within it strewn all over the properties tree, and
	 * it's a large job to try to find and update them; instead, we just live
	 * with a little wasted heap memory.
	 */
	bundle->dictionary = (sc_char **)prop_trim_capacity(bundle->dictionary,
	                     bundle->dictionary_length,
	                     sizeof(bundle->dictionary[0]));
	bundle->node_pools = (sc_prop_noderef_t *)prop_trim_capacity(bundle->node_pools,
	                     bundle->node_pools_length,
	                     sizeof(bundle->node_pools[0]));
	bundle->orphans = (void **)prop_trim_capacity(bundle->orphans,
	                  bundle->orphans_length,
	                  sizeof(bundle->orphans[0]));
	prop_trim_node(bundle->root_node);

	/* Set the bundle so that no more properties can be added. */
	bundle->is_readonly = TRUE;
}


/*
 * prop_get_integer()
 * prop_get_boolean()
 * prop_get_string()
 *
 * Convenience functions to retrieve a property of a known type directly.
 * It is an error for the property not to exist on retrieval.
 */
sc_int prop_get_integer(sc_prop_setref_t bundle, const sc_char *format, const sc_vartype_t vt_key[]) {
	sc_vartype_t vt_rvalue;
	assert(format[0] == PROP_INTEGER);

	if (!prop_get(bundle, format, &vt_rvalue, vt_key))
		sc_fatal("prop_get_integer: can't retrieve property\n");

	return vt_rvalue.integer;
}

sc_bool prop_get_boolean(sc_prop_setref_t bundle, const sc_char *format, const sc_vartype_t vt_key[]) {
	sc_vartype_t vt_rvalue;
	assert(format[0] == PROP_BOOLEAN);

	if (!prop_get(bundle, format, &vt_rvalue, vt_key))
		sc_fatal("prop_get_boolean: can't retrieve property\n");

	return vt_rvalue.boolean;
}

const sc_char *prop_get_string(sc_prop_setref_t bundle, const sc_char *format, const sc_vartype_t vt_key[]) {
	sc_vartype_t vt_rvalue;
	assert(format[0] == PROP_STRING);

	if (!prop_get(bundle, format, &vt_rvalue, vt_key))
		sc_fatal("prop_get_string: can't retrieve property\n");

	return vt_rvalue.string;
}


/*
 * prop_get_child_count()
 *
 * Convenience function to retrieve a count of child properties available
 * for a given property.  Returns zero if the property does not exist.
 */
sc_int prop_get_child_count(sc_prop_setref_t bundle, const sc_char *format, const sc_vartype_t vt_key[]) {
	sc_vartype_t vt_rvalue;
	assert(format[0] == PROP_INTEGER);

	if (!prop_get(bundle, format, &vt_rvalue, vt_key))
		return 0;

	/* Return overloaded integer property value, the child count. */
	return vt_rvalue.integer;
}


/*
 * prop_create_empty()
 *
 * Create a new, empty properties set, and return it.
 */
static sc_prop_setref_t prop_create_empty() {
	sc_prop_setref_t bundle;

	/* Create a new, empty set. */
	bundle = (sc_prop_setref_t)sc_malloc(sizeof(*bundle));
	bundle->magic = PROP_MAGIC;

	/* Begin with an empty strings dictionary. */
	bundle->dictionary_length = 0;
	bundle->dictionary = NULL;

	/* Begin with no allocated node pools. */
	bundle->node_pools_length = 0;
	bundle->node_pools = NULL;
	bundle->node_count = 0;

	/* Begin with no adopted addresses. */
	bundle->orphans_length = 0;
	bundle->orphans = NULL;

	/* Leave open for insertions. */
	bundle->is_readonly = FALSE;

	/*
	 * Start the set off with a root node.  This will also kick off node pools,
	 * ensuring that every set has at least one node and one allocated pool.
	 */
	bundle->root_node = prop_new_node(bundle);
	bundle->root_node->child_list = NULL;
	bundle->root_node->name.string = "ROOT";
	bundle->root_node->property.voidp = NULL;

	/* No taf is yet connected with this set. */
	bundle->taf = NULL;

	return bundle;
}


/*
 * prop_destroy_child_list()
 * prop_destroy()
 *
 * Free set memory, and destroy a properties set structure.
 */
static void prop_destroy_child_list(sc_prop_noderef_t node) {
	/* End recursion on null or childless node. */
	if (node && node->child_list) {
		sc_int index_;

		/* Recursively destroy the children's child lists. */
		for (index_ = 0; index_ < node->property.integer; index_++)
			prop_destroy_child_list(node->child_list[index_]);

		/* Free our own child list. */
		sc_free(node->child_list);
	}
}

void prop_destroy(sc_prop_setref_t bundle) {
	sc_int index_;
	assert(prop_is_valid(bundle));

	/* Destroy the dictionary, and free it. */
	for (index_ = 0; index_ < bundle->dictionary_length; index_++)
		sc_free(bundle->dictionary[index_]);
	bundle->dictionary_length = 0;
	sc_free(bundle->dictionary);
	bundle->dictionary = NULL;

	/* Free adopted addresses. */
	for (index_ = 0; index_ < bundle->orphans_length; index_++)
		sc_free(bundle->orphans[index_]);
	bundle->orphans_length = 0;
	sc_free(bundle->orphans);
	bundle->orphans = NULL;

	/* Walk the tree, destroying the child list for each node found. */
	prop_destroy_child_list(bundle->root_node);
	bundle->root_node = NULL;

	/* Destroy each node pool. */
	for (index_ = 0; index_ < bundle->node_pools_length; index_++)
		sc_free(bundle->node_pools[index_]);
	bundle->node_pools_length = 0;
	sc_free(bundle->node_pools);
	bundle->node_pools = NULL;

	/* Destroy any taf associated with the bundle. */
	if (bundle->taf)
		taf_destroy(bundle->taf);

	/* Poison and free the bundle. */
	memset(bundle, 0xaa, sizeof(*bundle));
	sc_free(bundle);
}


/*
 * prop_create()
 *
 * Create a new properties set based on a taf, and return it.
 */
sc_prop_setref_t prop_create(const sc_tafref_t taf) {
	sc_prop_setref_t bundle;

	/* Create a new, empty set. */
	bundle = prop_create_empty();

	/* Populate it with data parsed from the taf file. */
	if (!parse_game(taf, bundle)) {
		prop_destroy(bundle);
		return NULL;
	}

	/* Note the taf for destruction later, and return the new set. */
	bundle->taf = taf;
	return bundle;
}


/*
 * prop_adopt()
 *
 * Adopt a memory address for free'ing on destroy.
 */
void prop_adopt(sc_prop_setref_t bundle, void *addr) {
	assert(prop_is_valid(bundle));

	/* Extend the orphans array if necessary. */
	bundle->orphans = (void **)prop_ensure_capacity(bundle->orphans,
	                  bundle->orphans_length,
	                  bundle->orphans_length + 1,
	                  sizeof(bundle->orphans[0]));

	/* Add the new address to the end of the array. */
	bundle->orphans[bundle->orphans_length++] = addr;
}


/*
 * prop_debug_is_dictionary_string()
 * prop_debug_dump_node()
 * prop_debug_dump()
 *
 * Print out a complete properties set.
 */
static sc_bool prop_debug_is_dictionary_string(sc_prop_setref_t bundle, const sc_char *pointer) {
	const sc_char *const pointer_ = pointer;
	sc_int index_;

	/* Compare by pointer directly, not by string value comparisons. */
	for (index_ = 0; index_ < bundle->dictionary_length; index_++) {
		if (bundle->dictionary[index_] == pointer_)
			return TRUE;
	}

	return FALSE;
}

static void prop_debug_dump_node(sc_prop_setref_t bundle, sc_int depth,
		sc_int child_index, sc_prop_noderef_t node) {
	sc_int index_;

	/* Write node preamble, indented two spaces for each depth count. */
	for (index_ = 0; index_ < depth; index_++)
		sc_trace("  ");
	sc_trace("%ld : %p", child_index, (void *) node);

	/* Write node, or just a newline if none. */
	if (node) {
		/* Print out the node's key, as hex and either string or decimal. */
		sc_trace(", name %p", node->name.voidp);
		if (node != bundle->root_node) {
			if (prop_debug_is_dictionary_string(bundle, node->name.string))
				sc_trace(" \"%s\"", node->name.string);
			else
				sc_trace(" %ld", node->name.integer);
		}

		if (node->child_list) {
			/* Recursively dump children. */
			sc_trace(", child count %ld\n", node->property.integer);
			for (index_ = 0; index_ < node->property.integer; index_++) {
				prop_debug_dump_node(bundle, depth + 1,
				                     index_, node->child_list[index_]);
			}
		} else {
			/* Print out the node's property, again hex and string or decimal. */
			sc_trace(", property %p", node->property.voidp);
			if (taf_debug_is_taf_string(bundle->taf, node->property.string))
				sc_trace(" \"%s\"\n", node->property.string);
			else
				sc_trace(" %ld\n", node->property.integer);
		}
	} else
		sc_trace("\n");
}

void prop_debug_dump(sc_prop_setref_t bundle) {
	sc_int index_;
	assert(prop_is_valid(bundle));

	/* Dump complete structure. */
	sc_trace("Property: debug dump follows...\n");
	sc_trace("bundle->is_readonly = %s\n",
	         bundle->is_readonly ? "true" : "false");
	sc_trace("bundle->dictionary_length = %ld\n", bundle->dictionary_length);

	sc_trace("bundle->dictionary =\n");
	for (index_ = 0; index_ < bundle->dictionary_length; index_++) {
		sc_trace("%3ld : %p \"%s\"\n", index_,
		         (void *)bundle->dictionary[index_], bundle->dictionary[index_]);
	}

	sc_trace("bundle->node_pools_length = %ld\n", bundle->node_pools_length);

	sc_trace("bundle->node_pools =\n");
	for (index_ = 0; index_ < bundle->node_pools_length; index_++)
		sc_trace("%3ld : %p\n", index_, (void *) bundle->node_pools[index_]);

	sc_trace("bundle->node_count = %ld\n", bundle->node_count);
	sc_trace("bundle->root_node = {\n");
	prop_debug_dump_node(bundle, 0, 0, bundle->root_node);
	sc_trace("}\nbundle->taf = %p\n", (void *) bundle->taf);
}


/*
 * prop_debug_trace()
 *
 * Set property tracing on/off.
 */
void prop_debug_trace(sc_bool flag) {
	prop_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
