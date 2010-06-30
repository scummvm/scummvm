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
			error("isSaneNodePointer: Node at %04x:%04x wasn't found", PRINT_REG(addr));
			return false;
		}

		if (havePrev && node->pred != prev) {
			error("isSaneNodePointer: Node at %04x:%04x points to invalid predecessor %04x:%04x (should be %04x:%04x)",
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
	List *list = segMan->lookupList(addr);

	if (!list) {
		error("checkListPointer (list %04x:%04x): The requested list wasn't found",
				PRINT_REG(addr));
		return;
	}

	if (list->first.isNull() && list->last.isNull()) {
		// Empty list is fine
	} else if (!list->first.isNull() && !list->last.isNull()) {
		// Normal list
		Node *node_a = segMan->lookupNode(list->first);
		Node *node_z = segMan->lookupNode(list->last);

		if (!node_a) {
			error("checkListPointer (list %04x:%04x): missing first node", PRINT_REG(addr));
			return;
		}

		if (!node_z) {
			error("checkListPointer (list %04x:%04x): missing last node", PRINT_REG(addr));
			return;
		}

		if (!node_a->pred.isNull()) {
			error("checkListPointer (list %04x:%04x): First node of the list points to a predecessor node",
					PRINT_REG(addr));

			//node_a->pred = NULL_REG;	// fix the problem in the node

			return;
		}

		if (!node_z->succ.isNull()) {
			error("checkListPointer (list %04x:%04x): Last node of the list points to a successor node",
					PRINT_REG(addr));

			//node_z->succ = NULL_REG;	// fix the problem in the node

			return;
		}

		isSaneNodePointer(segMan, list->first);
	} else {
		// Not sane list... it's missing pointers to the first or last element
		if (list->first.isNull())
			error("checkListPointer (list %04x:%04x): missing pointer to first element",
					PRINT_REG(addr));
		if (list->last.isNull())
			error("checkListPointer (list %04x:%04x): missing pointer to last element",
					PRINT_REG(addr));
	}
}

reg_t kNewList(EngineState *s, int argc, reg_t *argv) {
	reg_t listRef;
	List *list = s->_segMan->allocateList(&listRef);
	list->first = list->last = NULL_REG;
	debugC(2, kDebugLevelNodes, "New listRef at %04x:%04x", PRINT_REG(listRef));

	return listRef; // Return list base address
}

reg_t kDisposeList(EngineState *s, int argc, reg_t *argv) {
	// This function is not needed in ScummVM. The garbage collector
	// cleans up unused objects automatically

	return s->r_acc;
}

reg_t kNewNode(EngineState *s, int argc, reg_t *argv) {
	reg_t nodeValue = argv[0];
	// Some SCI32 games call this with 1 parameter (e.g. the demo of Phantasmagoria).
	// Set the key to be the same as the value in this case
	reg_t nodeKey = (argc == 2) ? argv[1] : argv[0];
	s->r_acc = s->_segMan->newNode(nodeValue, nodeKey);

	debugC(2, kDebugLevelNodes, "New nodeRef at %04x:%04x", PRINT_REG(s->r_acc));

	return s->r_acc;
}

reg_t kFirstNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);

	if (list) {
		checkListPointer(s->_segMan, argv[0]);
		return list->first;
	} else {
		return NULL_REG;
	}
}

reg_t kLastNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);

	if (list) {
		checkListPointer(s->_segMan, argv[0]);
		return list->last;
	} else {
		return NULL_REG;
	}
}

reg_t kEmptyList(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);
	checkListPointer(s->_segMan, argv[0]);

	return make_reg(0, ((list) ? list->first.isNull() : 0));
}

static void _k_add_to_front(EngineState *s, reg_t listRef, reg_t nodeRef) {
	List *list = s->_segMan->lookupList(listRef);
	Node *newNode = s->_segMan->lookupNode(nodeRef);

	debugC(2, kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

	if (!newNode)
		error("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));
	checkListPointer(s->_segMan, listRef);

	newNode->pred = NULL_REG;
	newNode->succ = list->first;

	// Set node to be the first and last node if it's the only node of the list
	if (list->first.isNull())
		list->last = nodeRef;
	else {
		Node *oldNode = s->_segMan->lookupNode(list->first);
		oldNode->pred = nodeRef;
	}
	list->first = nodeRef;
}

static void _k_add_to_end(EngineState *s, reg_t listRef, reg_t nodeRef) {
	List *list = s->_segMan->lookupList(listRef);
	Node *newNode = s->_segMan->lookupNode(nodeRef);

	debugC(2, kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

	if (!newNode)
		error("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));
	checkListPointer(s->_segMan, listRef);

	newNode->pred = list->last;
	newNode->succ = NULL_REG;

	// Set node to be the first and last node if it's the only node of the list
	if (list->last.isNull())
		list->first = nodeRef;
	else {
		Node *old_n = s->_segMan->lookupNode(list->last);
		old_n->succ = nodeRef;
	}
	list->last = nodeRef;
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
	List *list = s->_segMan->lookupList(argv[0]);
	Node *firstnode = argv[1].isNull() ? NULL : s->_segMan->lookupNode(argv[1]);
	Node *newnode = s->_segMan->lookupNode(argv[2]);

	checkListPointer(s->_segMan, argv[0]);

	if (!newnode) {
		error("New 'node' %04x:%04x is not a node", PRINT_REG(argv[2]));
		return NULL_REG;
	}

	if (argc != 3 && argc != 4) {
		error("kAddAfter: Haven't got 3 or 4 arguments, aborting");
		return NULL_REG;
	}

	if (argc == 4)
		newnode->key = argv[3];

	if (firstnode) { // We're really appending after
		reg_t oldnext = firstnode->succ;

		newnode->pred = argv[1];
		firstnode->succ = argv[2];
		newnode->succ = oldnext;

		if (oldnext.isNull())  // Appended after last node?
			// Set new node as last list node
			list->last = argv[2];
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
	List *list = s->_segMan->lookupList(argv[0]);

	if (node_pos.isNull())
		return NULL_REG; // Signal failure

	n = s->_segMan->lookupNode(node_pos);
	if (list->first == node_pos)
		list->first = n->succ;
	if (list->last == node_pos)
		list->last = n->pred;

	if (!n->pred.isNull())
		s->_segMan->lookupNode(n->pred)->succ = n->succ;
	if (!n->succ.isNull())
		s->_segMan->lookupNode(n->succ)->pred = n->pred;

	// Erase references to the predecessor and successor nodes, as the game
	// scripts could reference the node itself again.
	// Happens in the intro of QFG1 and in Longbow, when exiting the cave.
	n->pred = NULL_REG;
	n->succ = NULL_REG;

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

	int input_size = (int16)readSelectorValue(segMan, source, SELECTOR(size));
	reg_t input_data = readSelector(segMan, source, SELECTOR(elements));
	reg_t output_data = readSelector(segMan, dest, SELECTOR(elements));

	List *list;
	Node *node;

	if (!input_size)
		return s->r_acc;

	if (output_data.isNull()) {
		list = s->_segMan->allocateList(&output_data);
		list->first = list->last = NULL_REG;
		writeSelector(segMan, dest, SELECTOR(elements), output_data);
	}

	writeSelectorValue(segMan, dest, SELECTOR(size), input_size);

	list = s->_segMan->lookupList(input_data);
	node = s->_segMan->lookupNode(list->first);

	sort_temp_t *temp_array = (sort_temp_t *)malloc(sizeof(sort_temp_t) * input_size);

	int i = 0;
	while (node) {
		reg_t params[1] = { node->value };
		invokeSelector(s, order_func, SELECTOR(doit), argc, argv, 1, params);
		temp_array[i].key = node->key;
		temp_array[i].value = node->value;
		temp_array[i].order = s->r_acc;
		i++;
		node = s->_segMan->lookupNode(node->succ);
	}

	qsort(temp_array, input_size, sizeof(sort_temp_t), sort_temp_cmp);

	for (i = 0;i < input_size;i++) {
		reg_t lNode = s->_segMan->newNode(temp_array[i].value, temp_array[i].key);
		_k_add_to_end(s, output_data, lNode);
	}

	free(temp_array);

	return s->r_acc;
}

// SCI32 list functions
#ifdef ENABLE_SCI32

reg_t kListAt(EngineState *s, int argc, reg_t *argv) {
	if (argc != 2) {
		error("kListAt called with %d parameters", argc);
		return NULL_REG;
	}

	List *list = s->_segMan->lookupList(argv[0]);
	reg_t curAddress = list->first;
	if (list->first.isNull()) {
		error("kListAt tried to reference empty list (%04x:%04x)", PRINT_REG(argv[0]));
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
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// This can only happen with 3 params (list, target selector, variable)
			if (argc != 3) {
				error("kListEachElementDo: Attempted to modify a variable selector with %d params", argc);
			} else {
				writeSelector(s->_segMan, curObject, slc, argv[2]);
			}
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);
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
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// Can this happen with variable selectors?
			error("kListFirstTrue: Attempted to access a variable selector");
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);

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
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// Can this happen with variable selectors?
			error("kListAllTrue: Attempted to access a variable selector");
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);

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
		error("kList: unimplemented subfunction kAddBefore");
		//return kAddBefore(s, argc - 1, argv + 1);
		return NULL_REG;
	case 13:
		error("kList: unimplemented subfunction kMoveToFront");
		//return kMoveToFront(s, argc - 1, argv + 1);
		return NULL_REG;
	case 14:
		error("kList: unimplemented subfunction kMoveToEnd");
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
		error("kList: Unhandled case %d", argv[0].toUint16());
		return NULL_REG;
	}
}

#endif

} // End of namespace Sci
