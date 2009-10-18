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

#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

#ifdef DISABLE_VALIDATIONS

#define sane_nodep(a, b) 1
#define sane_listp(a, b) 1

#else

static int sane_nodep(EngineState *s, reg_t addr) {
	int have_prev = 0;
	reg_t prev = addr;

	do {
		Node *node = s->_segMan->lookupNode(addr);

		if (!node)
			return 0;

		if ((have_prev) && node->pred != prev)
			return 0;

		prev = addr;
		addr = node->succ;
		have_prev = 1;
	} while (!addr.isNull());

	return 1;
}

int sane_listp(EngineState *s, reg_t addr) {
	List *l = s->_segMan->lookupList(addr);
	int empties = 0;

	if (l->first.isNull())
		++empties;
	if (l->last.isNull())
		++empties;

	// Either none or both must be set
	if (empties == 1)
		return 0;

	if (!empties) {
		Node *node_a, *node_z;

		node_a = s->_segMan->lookupNode(l->first);
		node_z = s->_segMan->lookupNode(l->last);

		if (!node_a || !node_z)
			return 0;

		if (!node_a->pred.isNull())
			return 0;

		if (!node_z->succ.isNull())
			return 0;

		return sane_nodep(s, l->first);
	}

	return 1; // Empty list is fine
}
#endif

reg_t kNewList(EngineState *s, int argc, reg_t *argv) {
	reg_t listbase;
	List *l;
	l = s->_segMan->allocateList(&listbase);
	l->first = l->last = NULL_REG;
	debugC(2, kDebugLevelNodes, "New listbase at %04x:%04x\n", PRINT_REG(listbase));

	return listbase; // Return list base address
}

reg_t kDisposeList(EngineState *s, int argc, reg_t *argv) {
	List *l = s->_segMan->lookupList(argv[0]);

	if (!l) {
		// FIXME: This should be an error, but it's turned to a warning for now
		warning("Attempt to dispose non-list at %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	if (!sane_listp(s, argv[0]))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(argv[0]));

/*	if (!l->first.isNull()) {
		reg_t n_addr = l->first;

		while (!n_addr.isNull()) { // Free all nodes
			Node *n = s->_segMan->lookupNode(n_addr);
			s->_segMan->free_Node(n_addr);
			n_addr = n->succ;
		}
	}

	s->_segMan->free_list(argv[0]);
*/
	return s->r_acc;
}

reg_t _k_new_node(EngineState *s, reg_t value, reg_t key) {
	reg_t nodebase;
	Node *n = s->_segMan->allocateNode(&nodebase);

	if (!n) {
		error("[Kernel] Out of memory while creating a node");
		return NULL_REG;
	}

	n->pred = n->succ = NULL_REG;
	n->key = key;
	n->value = value;

	return nodebase;
}

reg_t kNewNode(EngineState *s, int argc, reg_t *argv) {
	s->r_acc = _k_new_node(s, argv[0], argv[1]);

	debugC(2, kDebugLevelNodes, "New nodebase at %04x:%04x\n", PRINT_REG(s->r_acc));

	return s->r_acc;
}

reg_t kFirstNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;
	List *l = s->_segMan->lookupList(argv[0]);

	if (l && !sane_listp(s, argv[0]))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(argv[0]));

	if (l)
		return l->first;
	else
		return NULL_REG;
}

reg_t kLastNode(EngineState *s, int argc, reg_t *argv) {
	List *l = s->_segMan->lookupList(argv[0]);

	if (l && !sane_listp(s, argv[0]))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(argv[0]));

	if (l)
		return l->last;
	else
		return NULL_REG;
}

reg_t kEmptyList(EngineState *s, int argc, reg_t *argv) {
	List *l = s->_segMan->lookupList(argv[0]);

	if (!l || !sane_listp(s, argv[0]))
		warning("List at %04x:%04x is invalid or not sane anymore", PRINT_REG(argv[0]));

	return make_reg(0, ((l) ? l->first.isNull() : 0));
}

void _k_add_to_front(EngineState *s, reg_t listbase, reg_t nodebase) {
	List *l = s->_segMan->lookupList(listbase);
	Node *new_n = s->_segMan->lookupNode(nodebase);

	debugC(2, kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x\n", PRINT_REG(nodebase), PRINT_REG(listbase));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!new_n)
		warning("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodebase), PRINT_REG(listbase));
	if (!l || !sane_listp(s, listbase))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(listbase));

	new_n->succ = l->first;
	new_n->pred = NULL_REG;
	// Set node to be the first and last node if it's the only node of the list
	if (l->first.isNull())
		l->last = nodebase;
	else {
		Node *old_n = s->_segMan->lookupNode(l->first);
		old_n->pred = nodebase;
	}
	l->first = nodebase;
}

void _k_add_to_end(EngineState *s, reg_t listbase, reg_t nodebase) {
	List *l = s->_segMan->lookupList(listbase);
	Node *new_n = s->_segMan->lookupNode(nodebase);

	debugC(2, kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x\n", PRINT_REG(nodebase), PRINT_REG(listbase));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!new_n)
		warning("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodebase), PRINT_REG(listbase));
	if (!l || !sane_listp(s, listbase))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(listbase));

	new_n->succ = NULL_REG;
	new_n->pred = l->last;
	// Set node to be the first and last node if it's the only node of the list
	if (l->last.isNull())
		l->first = nodebase;
	else {
		Node *old_n = s->_segMan->lookupNode(l->last);
		old_n->succ = nodebase;
	}
	l->last = nodebase;
}

reg_t kNextNode(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);
	if (!sane_nodep(s, argv[0])) {
		warning("List node at %04x:%04x is not sane anymore", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	return n->succ;
}

reg_t kPrevNode(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);
	if (!sane_nodep(s, argv[0]))
		warning("List node at %04x:%04x is not sane anymore", PRINT_REG(argv[0]));

	return n->pred;
}

reg_t kNodeValue(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);
	if (!sane_nodep(s, argv[0])) {
		warning("List node at %04x:%04x is not sane", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	return n->value;
}

reg_t kAddToFront(EngineState *s, int argc, reg_t *argv) {
	_k_add_to_front(s, argv[0], argv[1]);
	return s->r_acc;
}

reg_t kAddAfter(EngineState *s, int argc, reg_t *argv) {
	List *l = s->_segMan->lookupList(argv[0]);
	Node *firstnode = argv[1].isNull() ? NULL : s->_segMan->lookupNode(argv[1]);
	Node *newnode = s->_segMan->lookupNode(argv[2]);

	if (!l || !sane_listp(s, argv[0]))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(argv[0]));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!newnode) {
		warning("New 'node' %04x:%04x is not a node", PRINT_REG(argv[2]));
		return NULL_REG;
	}

	if (argc != 3) {
		warning("kAddAfter: Haven't got 3 arguments, aborting");
		return NULL_REG;
	}

	if (firstnode) { // We're really appending after
		reg_t oldnext = firstnode->succ;

		newnode->pred = argv[1];
		firstnode->succ = argv[2];
		newnode->succ = oldnext;

		if (oldnext.isNull())  // Appended after last node?
			// Set new node as last list node
			l->last = argv[2];
		else
			s->_segMan->lookupNode(oldnext)->pred = argv[2];

	} else { // !firstnode
		_k_add_to_front(s, argv[0], argv[2]); // Set as initial list node
	}

	return s->r_acc;
}

reg_t kAddToEnd(EngineState *s, int argc, reg_t *argv) {
	_k_add_to_end(s, argv[0], argv[1]);
	return s->r_acc;
}

reg_t kFindKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos;
	reg_t key = argv[1];
	reg_t list_pos = argv[0];

	debugC(2, kDebugLevelNodes, "Looking for key %04x:%04x in list %04x:%04x\n", PRINT_REG(key), PRINT_REG(list_pos));

	if (!sane_listp(s, list_pos))
		warning("List at %04x:%04x is not sane anymore", PRINT_REG(list_pos));

	node_pos = s->_segMan->lookupList(list_pos)->first;

	debugC(2, kDebugLevelNodes, "First node at %04x:%04x\n", PRINT_REG(node_pos));

	while (!node_pos.isNull()) {
		Node *n = s->_segMan->lookupNode(node_pos);
		if (n->key == key) {
			debugC(2, kDebugLevelNodes, " Found key at %04x:%04x\n", PRINT_REG(node_pos));
			return node_pos;
		}

		node_pos = n->succ;
		debugC(2, kDebugLevelNodes, "NextNode at %04x:%04x\n", PRINT_REG(node_pos));
	}

	debugC(2, kDebugLevelNodes, "Looking for key without success\n");
	return NULL_REG;
}

reg_t kDeleteKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos = kFindKey(s, 2, argv);
	Node *n;
	List *l = s->_segMan->lookupList(argv[0]);

	if (node_pos.isNull())
		return NULL_REG; // Signal falure

	n = s->_segMan->lookupNode(node_pos);
	if (l->first == node_pos)
		l->first = n->succ;
	if (l->last == node_pos)
		l->last = n->pred;

	if (!n->pred.isNull())
		s->_segMan->lookupNode(n->pred)->succ = n->succ;
	if (!n->succ.isNull())
		s->_segMan->lookupNode(n->succ)->pred = n->pred;

	//s->_segMan->free_Node(node_pos);

	return make_reg(0, 1); // Signal success
}

struct sort_temp_t {
	reg_t key, value;
	reg_t order;
};

int sort_temp_cmp(const void *p1, const void *p2) {
	const sort_temp_t *st1 = (const sort_temp_t *)p1;
	const sort_temp_t *st2 = (const sort_temp_t *)p2;

	if (st1->order.segment < st1->order.segment || (st1->order.segment == st1->order.segment && st1->order.offset < st2->order.offset))
		return -1;

	if (st1->order.segment > st2->order.segment || (st1->order.segment == st2->order.segment && st1->order.offset > st2->order.offset))
		return 1;

	return 0;
}

reg_t kSort(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	reg_t source = argv[0];
	reg_t dest = argv[1];
	reg_t order_func = argv[2];

	int input_size = (int16)GET_SEL32V(segMan, source, size);
	int i;

	reg_t input_data = GET_SEL32(segMan, source, elements);
	reg_t output_data = GET_SEL32(segMan, dest, elements);

	List *list;
	Node *node;

	if (!input_size)
		return s->r_acc;

	if (output_data.isNull()) {
		list = s->_segMan->allocateList(&output_data);
		list->first = list->last = NULL_REG;
		PUT_SEL32(segMan, dest, elements, output_data);
	}

	PUT_SEL32V(segMan, dest, size, input_size);

	list = s->_segMan->lookupList(input_data);
	node = s->_segMan->lookupNode(list->first);

	sort_temp_t *temp_array = (sort_temp_t *)malloc(sizeof(sort_temp_t) * input_size);

	i = 0;
	while (node) {
		invoke_selector(INV_SEL(order_func, doit, kStopOnInvalidSelector), 1, node->value);
		temp_array[i].key = node->key;
		temp_array[i].value = node->value;
		temp_array[i].order = s->r_acc;
		i++;
		node = s->_segMan->lookupNode(node->succ);
	}

	qsort(temp_array, input_size, sizeof(sort_temp_t), sort_temp_cmp);

	for (i = 0;i < input_size;i++) {
		reg_t lNode = _k_new_node(s, temp_array[i].key, temp_array[i].value);
		_k_add_to_end(s, output_data, lNode);
	}

	free(temp_array);

	return s->r_acc;
}

} // End of namespace Sci
