/***************************************************************************
 int_hashmap. Copyright (C) 2001 Christoph Reichenbach


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

#include "sci/engine/int_hashmap.h"


#define HASH_MAX DCS_INT_HASH_MAX
#define COMP(x, y) ((x)-(y))
#define HASH(x) (x & 0xff)

int_hash_map_t *
new_int_hash_map(void) {
	int_hash_map_t *map = (int_hash_map_t*)calloc(1, sizeof(int_hash_map_t));

	return map;
}


static void
print_int_nodes(int_hash_map_node_t *node) {
	while (node) {
		fprintf(stderr, "%p  ", (void *)node);
		node = node->next;
	}
}

void
print_int_hash_map(int_hash_map_t *map) {
	int bucket;
	fprintf(stderr, "int map %p: base value=%d\n", (void *)map,
	        map->base_value);
	for (bucket = 0; bucket <= HASH_MAX; bucket++) {
		fprintf(stderr, "bucket %d: ", bucket);
		print_int_nodes(map->nodes[bucket]);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "holes: ");
	print_int_nodes(map->holes);
	fprintf(stderr, "\n");
}

void
apply_to_int_hash_map(int_hash_map_t *map, void *param, void (*note)(void *param, int name, int value)) {
	int i;
	for (i = 0; i < HASH_MAX; i++) {
		int_hash_map_node_t *node = map->nodes[i];
		while (node) {
			note(param, node->name, node->value);
			node = node->next;
		}
	}
}


static void
free_int_hash_map_node_t_recursive(int_hash_map_node_t *node) {
	if (node) {
		free_int_hash_map_node_t_recursive(node->next);
		free(node);
	}
}


void
free_int_hash_map(int_hash_map_t *map) {
	int i;

	for (i = 0; i <= HASH_MAX; i++)
		free_int_hash_map_node_t_recursive(map->nodes[i]);

	free_int_hash_map_node_t_recursive(map->holes);

	map->base_value = -42000; /* Trigger problems for people who
			     ** forget to loose the reference  */
	free(map);
}

int
int_hash_map_check_value(int_hash_map_t *map, int value,
                         char add, char *was_added) {
	int_hash_map_node_t **node = &(map->nodes[HASH(value)]);

	while (*node && COMP(value, (*node)->name))
		node = &((*node)->next);

	if (was_added)
		*was_added = 0;

	if (*node) {
		return (*node)->value;
	}
	/* Not found */

	if (!add)
		return -1;

	if (was_added)
		*was_added = 1;

	if (map->holes) { /* Re-use old node */
		(*node) = map->holes;
		map->holes = (*node)->next;
		(*node)->next = NULL;
		(*node)->name = value;
	} else {
		*node = (int_hash_map_node_t*)malloc(sizeof(int_hash_map_node_t));
		(*node)->name = value;
		(*node)->value = map->base_value++;
		(*node)->next = NULL;
	}

	return (*node)->value;
}


int
int_hash_map_remove_value(int_hash_map_t *map, int value) {
	int_hash_map_node_t **node = &(map->nodes[HASH(value)]);

	while (*node && COMP(value, (*node)->name))
		node = &((*node)->next);

	if (*node) {
		int_hash_map_node_t *oldnode = *node;
		*node = (*node)->next;

		oldnode->next = map->holes; /* Old node is now a 'hole' */
		map->holes = oldnode;
		return oldnode->value;
	} else return -1; /* Not found */
}

