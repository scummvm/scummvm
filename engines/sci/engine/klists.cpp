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
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

namespace Sci {

static bool isSaneNodePointer(SegManager *segMan, reg_t addr) {
	bool havePrev = false;
	reg_t prev = addr;

	do {
		Node *node = segMan->lookupNode(addr);

		if (!node) {
			warning("isSaneNodePointer: Node at %04x:%04x wasn't found", PRINT_REG(addr));
			return false;
		}

		if (havePrev && node->pred != prev) {
			warning("isSaneNodePointer: Node at %04x:%04x points to invalid predecessor %04x:%04x (should be %04x:%04x)",
					PRINT_REG(addr), PRINT_REG(node->pred), PRINT_REG(prev));

			//node->pred = prev;	// fix the problem in the node
			return false;
		}

		prev = addr;
		addr = node->succ;
		havePrev = true;
	} while (!addr.isNull());

	return true;
}

static void checkListPointer(SegManager *segMan, reg_t addr) {
	List *l = segMan->lookupList(addr);

	if (!l) {
		warning("isSaneListPointer (list %04x:%04x): The requested list wasn't found",
				PRINT_REG(addr));
		return;
	}

	if (l->first.isNull() && l->last.isNull()) {
		// Empty list is fine
	} else if (!l->first.isNull() && !l->last.isNull()) {
		// Normal list
		Node *node_a = segMan->lookupNode(l->first);
		Node *node_z = segMan->lookupNode(l->last);

		if (!node_a) {
			warning("isSaneListPointer (list %04x:%04x): missing first node", PRINT_REG(addr));
			return;
		}

		if (!node_z) {
			warning("isSaneListPointer (list %04x:%04x): missing last node", PRINT_REG(addr));
			return;
		}

		if (!node_a->pred.isNull()) {
			warning("isSaneListPointer (list %04x:%04x): First node of the list points to a predecessor node",
					PRINT_REG(addr));

			//node_a->pred = NULL_REG;	// fix the problem in the node

			return;
		}

		if (!node_z->succ.isNull()) {
			warning("isSaneListPointer (list %04x:%04x): Last node of the list points to a successor node",
					PRINT_REG(addr));

			//node_z->succ = NULL_REG;	// fix the problem in the node

			return;
		}

		isSaneNodePointer(segMan, l->first);
	} else {
		// Not sane list... it's missing pointers to the first or last element
		if (l->first.isNull())
			warning("isSaneListPointer (list %04x:%04x): missing pointer to first element",
					PRINT_REG(addr));
		if (l->last.isNull())
			warning("isSaneListPointer (list %04x:%04x): missing pointer to last element",
					PRINT_REG(addr));
	}
}

reg_t kNewList(EngineState *s, int argc, reg_t *argv) {
	reg_t listbase;
	List *l;
	l = s->_segMan->allocateList(&listbase);
	l->first = l->last = NULL_REG;
	debugC(2, kDebugLevelNodes, "New listbase at %04x:%04x", PRINT_REG(listbase));

	return listbase; // Return list base address
}

reg_t kDisposeList(EngineState *s, int argc, reg_t *argv) {
	// This function is not needed in ScummVM. The garbage collector
	// cleans up unused objects automatically

#if 0
	List *l = s->_segMan->lookupList(argv[0]);

	if (!l) {
		// FIXME: This should be an error, but it's turned to a warning for now
		warning("Attempt to dispose non-list at %04x:%04x", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	checkListPointer(s->_segMan, argv[0]);

	if (!l->first.isNull()) {
		reg_t n_addr = l->first;

		while (!n_addr.isNull()) { // Free all nodes
			Node *n = s->_segMan->lookupNode(n_addr);
			n_addr = n->succ;

			//s->_segMan->free_Node(n_addr);	// TODO
		}
	}

	//s->_segMan->free_list(argv[0]);	// TODO
#endif

	return s->r_acc;
}

static reg_t _k_new_node(EngineState *s, reg_t value, reg_t key) {
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

	if (argc == 1)
		s->r_acc = _k_new_node(s, argv[0], argv[0]);
	else
		s->r_acc = _k_new_node(s, argv[0], argv[1]);

	debugC(2, kDebugLevelNodes, "New nodebase at %04x:%04x", PRINT_REG(s->r_acc));

	return s->r_acc;
}

reg_t kFirstNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *l = s->_segMan->lookupList(argv[0]);

	if (l) {
		checkListPointer(s->_segMan, argv[0]);
		return l->first;
	} else {
		return NULL_REG;
	}
}

reg_t kLastNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *l = s->_segMan->lookupList(argv[0]);

	if (l) {
		checkListPointer(s->_segMan, argv[0]);
		return l->last;
	} else {
		return NULL_REG;
	}
}

reg_t kEmptyList(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *l = s->_segMan->lookupList(argv[0]);
	checkListPointer(s->_segMan, argv[0]);

	return make_reg(0, ((l) ? l->first.isNull() : 0));
}

static void _k_add_to_front(EngineState *s, reg_t listbase, reg_t nodebase) {
	List *l = s->_segMan->lookupList(listbase);
	Node *new_n = s->_segMan->lookupNode(nodebase);

	debugC(2, kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodebase), PRINT_REG(listbase));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!new_n)
		warning("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodebase), PRINT_REG(listbase));
	checkListPointer(s->_segMan, listbase);

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

static void _k_add_to_end(EngineState *s, reg_t listbase, reg_t nodebase) {
	List *l = s->_segMan->lookupList(listbase);
	Node *new_n = s->_segMan->lookupNode(nodebase);

	debugC(2, kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodebase), PRINT_REG(listbase));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!new_n)
		warning("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodebase), PRINT_REG(listbase));
	checkListPointer(s->_segMan, listbase);

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
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;

	return n->succ;
}

reg_t kPrevNode(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;

	return n->pred;
}

reg_t kNodeValue(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;

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

	checkListPointer(s->_segMan, argv[0]);

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

	debugC(2, kDebugLevelNodes, "Looking for key %04x:%04x in list %04x:%04x", PRINT_REG(key), PRINT_REG(list_pos));

	checkListPointer(s->_segMan, argv[0]);

	node_pos = s->_segMan->lookupList(list_pos)->first;

	debugC(2, kDebugLevelNodes, "First node at %04x:%04x", PRINT_REG(node_pos));

	while (!node_pos.isNull()) {
		Node *n = s->_segMan->lookupNode(node_pos);
		if (n->key == key) {
			debugC(2, kDebugLevelNodes, " Found key at %04x:%04x", PRINT_REG(node_pos));
			return node_pos;
		}

		node_pos = n->succ;
		debugC(2, kDebugLevelNodes, "NextNode at %04x:%04x", PRINT_REG(node_pos));
	}

	debugC(2, kDebugLevelNodes, "Looking for key without success");
	return NULL_REG;
}

reg_t kDeleteKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos = kFindKey(s, 2, argv);
	Node *n;
	List *l = s->_segMan->lookupList(argv[0]);

	if (node_pos.isNull())
		return NULL_REG; // Signal failure

	n = s->_segMan->lookupNode(node_pos);
	if (l->first == node_pos)
		l->first = n->succ;
	if (l->last == node_pos)
		l->last = n->pred;

	if (!n->pred.isNull())
		s->_segMan->lookupNode(n->pred)->succ = n->succ;
	if (!n->succ.isNull())
		s->_segMan->lookupNode(n->succ)->pred = n->pred;

	//s->_segMan->free_Node(node_pos);	// TODO

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

	int input_size = (int16)GET_SEL32V(segMan, source, SELECTOR(size));
	int i;

	reg_t input_data = GET_SEL32(segMan, source, SELECTOR(elements));
	reg_t output_data = GET_SEL32(segMan, dest, SELECTOR(elements));

	List *list;
	Node *node;

	if (!input_size)
		return s->r_acc;

	if (output_data.isNull()) {
		list = s->_segMan->allocateList(&output_data);
		list->first = list->last = NULL_REG;
		PUT_SEL32(segMan, dest, SELECTOR(elements), output_data);
	}

	PUT_SEL32V(segMan, dest, SELECTOR(size), input_size);

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

// SCI32 list functions
#ifdef ENABLE_SCI32

reg_t kListAt(EngineState *s, int argc, reg_t *argv) {
	if (argc != 2) {
		warning("kListAt called with %d parameters", argc);
		return NULL_REG;
	}

	List *list = s->_segMan->lookupList(argv[0]);
	reg_t curAddress = list->first;
	if (list->first.isNull()) {
		warning("kListAt tried to reference empty list (%04x:%04x)", PRINT_REG(argv[0]));
		return NULL_REG;
	}
	Node *curNode = s->_segMan->lookupNode(curAddress);
	reg_t curObject = curNode->value;
	int16 listIndex = argv[1].toUint16();
	int curIndex = 0;

	while (curIndex != listIndex) {
		if (curNode->succ.isNull()) {	// end of the list?
			return NULL_REG;
		}

		curAddress = curNode->succ;
		curNode = s->_segMan->lookupNode(curAddress);
		curObject = curNode->value;

		curIndex++;
	}

	return curObject;
}

reg_t kListIndexOf(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	reg_t curAddress = list->first;
	Node *curNode = s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 curIndex = 0;

	while (curNode) {
		curObject = curNode->value;
		if (curObject == argv[1])
			return make_reg(0, curIndex);

		curAddress = curNode->succ;
		curNode = s->_segMan->lookupNode(curAddress);
		curIndex++;
	}

	return SIGNAL_REG;
}

reg_t kListEachElementDo(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	while (curNode) {
		// We get the next node here as the current node might be gone after the invoke
		reg_t nextNode = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookup_selector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// This can only happen with 3 params (list, target selector, variable)
			if (argc != 3) {
				warning("kListEachElementDo: Attempted to modify a variable selector with %d params", argc);
			} else {
				write_selector(s->_segMan, curObject, slc, argv[2]);
			}
		} else {
			invoke_selector_argv(s, curObject, slc, kContinueOnInvalidSelector, argc, argv, argc - 2, argv + 2);
		}

		curNode = s->_segMan->lookupNode(nextNode);
	}

	return s->r_acc;
}

reg_t kListFirstTrue(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	s->r_acc = NULL_REG;	// reset the accumulator

	while (curNode) {
		reg_t nextNode = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookup_selector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// Can this happen with variable selectors?
			warning("kListFirstTrue: Attempted to access a variable selector");
		} else {
			invoke_selector_argv(s, curObject, slc, kContinueOnInvalidSelector, argc, argv, argc - 2, argv + 2);

			// Check if the result is true
			if (!s->r_acc.isNull())
				return curObject;
		}

		curNode = s->_segMan->lookupNode(nextNode);
	}

	// No selector returned true
	return NULL_REG;
}

reg_t kListAllTrue(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	s->r_acc = make_reg(0, 1);	// reset the accumulator

	while (curNode) {
		reg_t nextNode = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookup_selector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// Can this happen with variable selectors?
			warning("kListAllTrue: Attempted to access a variable selector");
		} else {
			invoke_selector_argv(s, curObject, slc, kContinueOnInvalidSelector, argc, argv, argc - 2, argv + 2);

			// Check if the result isn't true
			if (s->r_acc.isNull())
				break;
		}

		curNode = s->_segMan->lookupNode(nextNode);
	}

	return s->r_acc;
}

// In SCI2.1, all the list functions were merged in one
reg_t kList(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case 0:
		return kNewList(s, argc - 1, argv + 1);
	case 1:
		return kDisposeList(s, argc - 1, argv + 1);
	case 2:
		return kNewNode(s, argc - 1, argv + 1);
	case 3:
		return kFirstNode(s, argc - 1, argv + 1);
	case 4:
		return kLastNode(s, argc - 1, argv + 1);
	case 5:
		return kEmptyList(s, argc - 1, argv + 1);
	case 6:
		return kNextNode(s, argc - 1, argv + 1);
	case 7:
		return kPrevNode(s, argc - 1, argv + 1);
	case 8:
		return kNodeValue(s, argc - 1, argv + 1);
	case 9:
		return kAddAfter(s, argc - 1, argv + 1);
	case 10:
		return kAddToFront(s, argc - 1, argv + 1);
	case 11:
		return kAddToEnd(s, argc - 1, argv + 1);
	case 12:
		warning("kList: unimplemented subfunction kAddBefore");
		//return kAddBefore(s, argc - 1, argv + 1);
		return NULL_REG;
	case 13:
		warning("kList: unimplemented subfunction kMoveToFront");
		//return kMoveToFront(s, argc - 1, argv + 1);
		return NULL_REG;
	case 14:
		warning("kList: unimplemented subfunction kMoveToEnd");
		//return kMoveToEnd(s, argc - 1, argv + 1);
		return NULL_REG;
	case 15:
		return kFindKey(s, argc - 1, argv + 1);
	case 16:
		return kDeleteKey(s, argc - 1, argv + 1);
	case 17:
		return kListAt(s, argc - 1, argv + 1);
	case 18:
		return kListIndexOf(s, argc - 1, argv + 1);
	case 19:
		return kListEachElementDo(s, argc - 1, argv + 1);
	case 20:
		return kListFirstTrue(s, argc - 1, argv + 1);
	case 21:
		return kListAllTrue(s, argc - 1, argv + 1);
	case 22:
		return kSort(s, argc - 1, argv + 1);
	default:
		warning("kList: Unhandled case %d", argv[0].toUint16());
		return NULL_REG;
	}
}

#endif

} // End of namespace Sci
