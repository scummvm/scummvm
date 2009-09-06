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

#include "sci/engine/intmap.h"

namespace Sci {

#define HASH(x) (x & 0xff)

IntMapper::IntMapper() {
	base_value = 0;
	memset(nodes, 0, sizeof(nodes));
	holes = 0;
}

void IntMapper::free_node_recursive(Node *node) {
	if (node) {
		free_node_recursive(node->next);
		node->next = 0;
		free(node);
	}
}


IntMapper::~IntMapper() {
	clear();

	// Trigger problems for people who forget to loose the reference
	base_value = -42000;
}

void IntMapper::clear() {
	for (int i = 0; i < DCS_INT_HASH_MAX; i++) {
		free_node_recursive(nodes[i]);
		nodes[i] = 0;
	}

	free_node_recursive(holes);
	holes = 0;
}

int IntMapper::checkKey(int key, bool add, bool *was_added) {
	Node **node = &(nodes[HASH(key)]);

	while (*node && (key != (*node)->key))
		node = &((*node)->next);

	if (was_added)
		*was_added = false;

	if (*node)
		return (*node)->idx;

	// Not found

	if (!add)
		return -1;

	if (was_added)
		*was_added = true;

	if (holes) { // Re-use old node
		(*node) = holes;
		holes = (*node)->next;
		(*node)->next = NULL;
		(*node)->key = key;
	} else {
		*node = (Node*)malloc(sizeof(Node));
		(*node)->key = key;
		(*node)->idx = base_value++;
		(*node)->next = NULL;
	}

	return (*node)->idx;
}

int IntMapper::lookupKey(int key) const {
	Node *const *node = &(nodes[HASH(key)]);

	while (*node && (key != (*node)->key))
		node = &((*node)->next);

	if (*node)
		return (*node)->idx;

	return -1;
}

void IntMapper::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(base_value);
	if (s.isLoading()) {
		uint32 key = 0, idx = 0;
		clear();
		while (true) {
			s.syncAsSint32LE(key);
			if (key == INTMAPPER_MAGIC_KEY)
				break;
			s.syncAsSint32LE(idx);
			// Insert into the IntMapper
			insert(key, idx);
		}
	} else {
		// Just write out all mapped pairs
		// We terminate by writing 4 times the value 0xFF
		for (int i = 0; i < DCS_INT_HASH_MAX; ++i) {
			Node *node = nodes[i];

			while (node) {
				s.syncAsSint32LE(node->key);
				s.syncAsSint32LE(node->idx);
				node = node->next;
			}
		}
		uint32 tmp = INTMAPPER_MAGIC_KEY;
		s.syncAsSint32LE(tmp);
	}
}

void IntMapper::insert(int key, int idx) {
	Node **node = &(nodes[HASH(key)]);

	while (*node && (key != (*node)->key))
		node = &((*node)->next);

	assert(0 == *node);	// Error out if the key was already present.

	*node = (Node*)malloc(sizeof(Node));
	(*node)->key = key;
	(*node)->idx = idx;
	(*node)->next = NULL;
}


int IntMapper::removeKey(int key) {
	Node **node = &(nodes[HASH(key)]);

	while (*node && (key != (*node)->key))
		node = &((*node)->next);

	if (*node) {
		Node *oldnode = *node;
		*node = (*node)->next;

		oldnode->next = holes; // Old node is now a 'hole'
		holes = oldnode;
		return oldnode->key;
	} else
		return -1; // Not found
}

} // End of namespace Sci
