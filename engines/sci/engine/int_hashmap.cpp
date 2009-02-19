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
	// Not found

	if (!add)
		return -1;

	if (was_added)
		*was_added = 1;

	if (holes) { // Re-use old node
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

		oldnode->next = holes; // Old node is now a 'hole'
		holes = oldnode;
		return oldnode->value;
	} else
		return -1; // Not found
}

