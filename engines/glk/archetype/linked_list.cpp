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

#include "glk/archetype/linked_list.h"
#include "glk/archetype/misc.h"

namespace Glk {
namespace Archetype {

void new_list(ListType &the_list) {
	the_list = new NodeType();
	add_bytes(sizeof(NodeType));
	the_list->next = the_list;
}

void dispose_list(ListType &the_list) {
	NodePtr theNode, axe;
	for (theNode = the_list->next; theNode != the_list; ) {
		axe = theNode;
		theNode = theNode->next;
		add_bytes(-(int)sizeof(*axe));
		delete axe;
	}

}

bool iterate_list(ListType &the_list, NodePtr &index) {
	if (index == nullptr)
		index = the_list->next;
	else
		index = index->next;

	return index != the_list;
}

void append_to_list(ListType &the_list, NodePtr the_node) {
	the_list->data = the_node->data;
	the_list->key = the_node->key;
	the_node->next = the_list->next;
	the_list->next = the_node;

	the_list = the_node;
}

NodePtr index_list(ListType &the_list, int number) {
	int i = 0;
	NodePtr p = the_list->next;

	while (i < number && p != the_list) {
		p = p->next;
		++i;
	}

	return (p == the_list) ? nullptr : p;
}

void insert_item(ListType &the_list, NodePtr the_item) {
	NodePtr p;
	for (p = the_list; p->next != the_list && p->next->key > the_item->key; p = p->next) {}

	the_item->next = p->next;
	p->next = the_item;
}

NodePtr find_item(ListType &the_list, int the_key) {
	NodePtr p;
	for (p = the_list->next; p != the_list && the_key < p->key; p = p->next) {}
	
	return (p == the_list || the_key != p->key) ? nullptr : p;
}

} // End of namespace Archetype
} // End of namespace Glk
