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
#define HASH(x) (x & 0xff)

int_hash_map_t::int_hash_map_t() {
	base_value = 0;
	memset(nodes, 0, sizeof(nodes));
	holes = 0;
}

void int_hash_map_t::free_node_recursive(node_t *node) {
	if (node) {
		free_node_recursive(node->next);
		free(node);
	}
}


int_hash_map_t::~int_hash_map_t() {
	int i;

	for (i = 0; i <= HASH_MAX; i++)
		free_node_recursive(nodes[i]);

	free_node_recursive(holes);

	// Trigger problems for people who forget to loose the reference
	base_value = -42000;
}

int int_hash_map_t::check_value(int value, bool add, char *was_added) {
	node_t **node = &(nodes[HASH(value)]);

	while (*node && (value != (*node)->name))
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

	if (holes) { /* Re-use old node */
		(*node) = holes;
		holes = (*node)->next;
		(*node)->next = NULL;
		(*node)->name = value;
	} else {
		*node = (node_t*)malloc(sizeof(node_t));
		(*node)->name = value;
		(*node)->value = base_value++;
		(*node)->next = NULL;
	}

	return (*node)->value;
}


int int_hash_map_t::remove_value(int value) {
	node_t **node = &(nodes[HASH(value)]);

	while (*node && (value != (*node)->name))
		node = &((*node)->next);

	if (*node) {
		node_t *oldnode = *node;
		*node = (*node)->next;

		oldnode->next = holes; /* Old node is now a 'hole' */
		holes = oldnode;
		return oldnode->value;
	} else
		return -1; /* Not found */
}

