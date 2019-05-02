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

#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"

namespace Sci {
//#define CHECK_LISTS	// adds sanity checking for lists and errors out when problems are found

#ifdef CHECK_LISTS

static bool isSaneNodePointer(SegManager *segMan, reg_t addr) {
	bool havePrev = false;
	reg_t prev = addr;

	do {
		Node *node = segMan->lookupNode(addr, false);

		if (!node) {
			if ((g_sci->getGameId() == GID_ICEMAN) && (g_sci->getEngineState()->currentRoomNumber() == 40)) {
				// ICEMAN: when plotting course, unDrawLast is called by startPlot::changeState
				//  there is no previous entry so we get 0 in here
			} else if ((g_sci->getGameId() == GID_HOYLE1) && (g_sci->getEngineState()->currentRoomNumber() == 3)) {
				// HOYLE1: after sorting cards in hearts, in the next round
				// we get an invalid node - bug #3038433
			} else {
				error("isSaneNodePointer: Node at %04x:%04x wasn't found", PRINT_REG(addr));
			}
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
		Node *node_a = segMan->lookupNode(list->first, false);
		Node *node_z = segMan->lookupNode(list->last, false);

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

#endif

reg_t kNewList(EngineState *s, int argc, reg_t *argv) {
	reg_t listRef;
	List *list = s->_segMan->allocateList(&listRef);
	list->first = list->last = NULL_REG;
	debugC(kDebugLevelNodes, "New listRef at %04x:%04x", PRINT_REG(listRef));

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

	debugC(kDebugLevelNodes, "New nodeRef at %04x:%04x", PRINT_REG(s->r_acc));

	return s->r_acc;
}

reg_t kFirstNode(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);

	if (list) {
#ifdef CHECK_LISTS
		checkListPointer(s->_segMan, argv[0]);
#endif
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
#ifdef CHECK_LISTS
		checkListPointer(s->_segMan, argv[0]);
#endif
		return list->last;
	} else {
		return NULL_REG;
	}
}

reg_t kEmptyList(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;

	List *list = s->_segMan->lookupList(argv[0]);
#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif
	return make_reg(0, ((list) ? list->first.isNull() : 0));
}

static void addToFront(EngineState *s, reg_t listRef, reg_t nodeRef) {
	List *list = s->_segMan->lookupList(listRef);
	Node *newNode = s->_segMan->lookupNode(nodeRef);

	debugC(kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

	if (!newNode)
		error("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, listRef);
#endif

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

static void addToEnd(EngineState *s, reg_t listRef, reg_t nodeRef) {
	List *list = s->_segMan->lookupList(listRef);
	Node *newNode = s->_segMan->lookupNode(nodeRef);

	debugC(kDebugLevelNodes, "Adding node %04x:%04x to end of list %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

	if (!newNode)
		error("Attempt to add non-node (%04x:%04x) to list at %04x:%04x", PRINT_REG(nodeRef), PRINT_REG(listRef));

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, listRef);
#endif

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

#ifdef CHECK_LISTS
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;
#endif

	return n->succ;
}

reg_t kPrevNode(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);

#ifdef CHECK_LISTS
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;
#endif

	return n->pred;
}

reg_t kNodeValue(EngineState *s, int argc, reg_t *argv) {
	Node *n = s->_segMan->lookupNode(argv[0]);

#ifdef CHECK_LISTS
	if (!isSaneNodePointer(s->_segMan, argv[0]))
		return NULL_REG;
#endif

	// ICEMAN: when plotting a course in room 40, unDrawLast is called by
	// startPlot::changeState, but there is no previous entry, so we get 0 here
	return n ? n->value : NULL_REG;
}

reg_t kAddToFront(EngineState *s, int argc, reg_t *argv) {
	addToFront(s, argv[0], argv[1]);

	if (argc == 3)
		s->_segMan->lookupNode(argv[1])->key = argv[2];

	return s->r_acc;
}

reg_t kAddToEnd(EngineState *s, int argc, reg_t *argv) {
	addToEnd(s, argv[0], argv[1]);

	if (argc == 3)
		s->_segMan->lookupNode(argv[1])->key = argv[2];

	return s->r_acc;
}

reg_t kAddAfter(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);
	Node *firstNode = s->_segMan->lookupNode(argv[1]);
	Node *newNode = s->_segMan->lookupNode(argv[2]);

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif

	if (!newNode) {
		error("New 'node' %04x:%04x is not a node", PRINT_REG(argv[2]));
		return NULL_REG;
	}

	if (argc != 3 && argc != 4) {
		error("kAddAfter: Haven't got 3 or 4 arguments, aborting");
		return NULL_REG;
	}

	if (argc == 4)
		newNode->key = argv[3];

	if (firstNode) { // We're really appending after
		const reg_t oldNext = firstNode->succ;

		newNode->pred = argv[1];
		firstNode->succ = argv[2];
		newNode->succ = oldNext;

		if (oldNext.isNull())  // Appended after last node?
			// Set new node as last list node
			list->last = argv[2];
		else
			s->_segMan->lookupNode(oldNext)->pred = argv[2];

	} else {
		addToFront(s, argv[0], argv[2]); // Set as initial list node
	}

	return s->r_acc;
}

reg_t kAddBefore(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);
	Node *firstNode = s->_segMan->lookupNode(argv[1]);
	Node *newNode = s->_segMan->lookupNode(argv[2]);

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif

	if (!newNode) {
		error("New 'node' %04x:%04x is not a node", PRINT_REG(argv[2]));
		return NULL_REG;
	}

	if (argc != 3 && argc != 4) {
		error("kAddBefore: Haven't got 3 or 4 arguments, aborting");
		return NULL_REG;
	}

	if (argc == 4)
		newNode->key = argv[3];

	if (firstNode) { // We're really appending before
		const reg_t oldPred = firstNode->pred;

		newNode->succ = argv[1];
		firstNode->pred = argv[2];
		newNode->pred = oldPred;

		if (oldPred.isNull())  // Appended before first node?
			// Set new node as first list node
			list->first = argv[2];
		else
			s->_segMan->lookupNode(oldPred)->succ = argv[2];

	} else {
		addToFront(s, argv[0], argv[2]); // Set as initial list node
	}

	return s->r_acc;
}

reg_t kFindKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos;
	reg_t key = argv[1];
	reg_t list_pos = argv[0];

	debugC(kDebugLevelNodes, "Looking for key %04x:%04x in list %04x:%04x", PRINT_REG(key), PRINT_REG(list_pos));

#ifdef CHECK_LISTS
	checkListPointer(s->_segMan, argv[0]);
#endif

	node_pos = s->_segMan->lookupList(list_pos)->first;

	debugC(kDebugLevelNodes, "First node at %04x:%04x", PRINT_REG(node_pos));

	while (!node_pos.isNull()) {
		Node *n = s->_segMan->lookupNode(node_pos);
		if (n->key == key) {
			debugC(kDebugLevelNodes, " Found key at %04x:%04x", PRINT_REG(node_pos));
			return node_pos;
		}

		node_pos = n->succ;
		debugC(kDebugLevelNodes, "NextNode at %04x:%04x", PRINT_REG(node_pos));
	}

	debugC(kDebugLevelNodes, "Looking for key without success");
	return NULL_REG;
}

reg_t kDeleteKey(EngineState *s, int argc, reg_t *argv) {
	reg_t node_pos = kFindKey(s, 2, argv);
	List *list = s->_segMan->lookupList(argv[0]);

	if (node_pos.isNull())
		return NULL_REG; // Signal failure

	Node *n = s->_segMan->lookupNode(node_pos);

#ifdef ENABLE_SCI32
	for (int i = 1; i <= list->numRecursions; ++i) {
		if (list->nextNodes[i] == node_pos) {
			list->nextNodes[i] = n->succ;
		}
	}
#endif

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

	if (st1->order.getSegment() < st2->order.getSegment() ||
		(st1->order.getSegment() == st2->order.getSegment() &&
		st1->order.getOffset() < st2->order.getOffset()))
		return -1;

	if (st1->order.getSegment() > st2->order.getSegment() ||
		(st1->order.getSegment() == st2->order.getSegment() &&
		st1->order.getOffset() > st2->order.getOffset()))
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
		addToEnd(s, output_data, lNode);
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
		// Happens in Torin when examining Di's locket in chapter 3
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

	// Update the virtual file selected in the character import screen of QFG4.
	// For the SCI0-SCI1.1 version of this, check kDrawControl().
	if (g_sci->inQfGImportRoom() && !strcmp(s->_segMan->getObjectName(curObject), "SelectorDText"))
		s->_chosenQfGImportItem = listIndex;

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
	const reg_t listReg = argv[0];
	List *list = s->_segMan->lookupList(listReg);

	Node *curNode = s->_segMan->lookupNode(list->first);
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	++list->numRecursions;

	if (list->numRecursions >= ARRAYSIZE(list->nextNodes)) {
		error("Too much recursion in kListEachElementDo");
	}

	while (curNode) {
		// We get the next node here as the current node might be deleted by the
		// invoke. In the case that the next node is also deleted, kDeleteKey
		// needs to be able to adjust the location of the next node, which is
		// why it is stored on the list instead of on the stack
		list->nextNodes[list->numRecursions] = curNode->succ;
		reg_t curObject = curNode->value;

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

			// Check if the call above leads to a game restore, in which case
			// the segment manager will be reset, and the original list will
			// be invalidated
			if (s->abortScriptProcessing == kAbortLoadGame)
				return s->r_acc;
		}

		curNode = s->_segMan->lookupNode(list->nextNodes[list->numRecursions]);
	}

	if (s->_segMan->isValidAddr(listReg, SEG_TYPE_LISTS)) {
		--list->numRecursions;
	}

	return s->r_acc;
}

reg_t kListFirstTrue(EngineState *s, int argc, reg_t *argv) {
	const reg_t listReg = argv[0];
	List *list = s->_segMan->lookupList(listReg);

	Node *curNode = s->_segMan->lookupNode(list->first);
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	s->r_acc = NULL_REG;

	++list->numRecursions;

	if (list->numRecursions >= ARRAYSIZE(list->nextNodes)) {
		error("Too much recursion in kListFirstTrue");
	}

	while (curNode) {
		// We get the next node here as the current node might be deleted by the
		// invoke. In the case that the next node is also deleted, kDeleteKey
		// needs to be able to adjust the location of the next node, which is
		// why it is stored on the list instead of on the stack
		list->nextNodes[list->numRecursions] = curNode->succ;
		reg_t curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// If it's a variable selector, check its value.
			// Example: script 64893 in Torin, MenuHandler::isHilited checks
			// all children for variable selector 0x03ba (bHilited).
			if (!readSelector(s->_segMan, curObject, slc).isNull()) {
				s->r_acc = curObject;
				break;
			}
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);

			// Check if the call above leads to a game restore, in which case
			// the segment manager will be reset, and the original list will
			// be invalidated
			if (s->abortScriptProcessing == kAbortLoadGame)
				return s->r_acc;

			// Check if the result is true
			if (!s->r_acc.isNull()) {
				s->r_acc = curObject;
				break;
			}
		}

		curNode = s->_segMan->lookupNode(list->nextNodes[list->numRecursions]);
	}

	if (s->_segMan->isValidAddr(listReg, SEG_TYPE_LISTS)) {
		--list->numRecursions;
	}

	return s->r_acc;
}

reg_t kListAllTrue(EngineState *s, int argc, reg_t *argv) {
	const reg_t listReg = argv[0];
	List *list = s->_segMan->lookupList(listReg);

	Node *curNode = s->_segMan->lookupNode(list->first);
	reg_t curObject;
	Selector slc = argv[1].toUint16();

	ObjVarRef address;

	s->r_acc = TRUE_REG;

	++list->numRecursions;

	if (list->numRecursions >= ARRAYSIZE(list->nextNodes)) {
		error("Too much recursion in kListAllTrue");
	}

	while (curNode) {
		// We get the next node here as the current node might be deleted by the
		// invoke. In the case that the next node is also deleted, kDeleteKey
		// needs to be able to adjust the location of the next node, which is
		// why it is stored on the list instead of on the stack
		list->nextNodes[list->numRecursions] = curNode->succ;
		curObject = curNode->value;

		// First, check if the target selector is a variable
		if (lookupSelector(s->_segMan, curObject, slc, &address, NULL) == kSelectorVariable) {
			// If it's a variable selector, check its value
			s->r_acc = readSelector(s->_segMan, curObject, slc);
		} else {
			invokeSelector(s, curObject, slc, argc, argv, argc - 2, argv + 2);

			// Check if the call above leads to a game restore, in which case
			// the segment manager will be reset, and the original list will
			// be invalidated
			if (s->abortScriptProcessing == kAbortLoadGame)
				return s->r_acc;
		}

		// Check if the result isn't true
		if (s->r_acc.isNull())
			break;

		curNode = s->_segMan->lookupNode(list->nextNodes[list->numRecursions]);
	}

	if (s->_segMan->isValidAddr(listReg, SEG_TYPE_LISTS)) {
		--list->numRecursions;
	}

	return s->r_acc;
}

reg_t kListSort(EngineState *s, int argc, reg_t *argv) {
	List *list = s->_segMan->lookupList(argv[0]);
	const int16 selector = argv[1].toSint16();
	const bool isDescending = argc > 2 ? (bool)argv[2].toUint16() : false;

	reg_t firstNode = list->first;
	for (reg_t node = firstNode; node != NULL_REG; node = s->_segMan->lookupNode(firstNode)->succ) {

		reg_t a;
		if (selector == -1) {
			a = s->_segMan->lookupNode(node)->value;
		} else {
			a = readSelector(s->_segMan, s->_segMan->lookupNode(node)->value, selector);
		}

		firstNode = node;
		for (reg_t newNode = s->_segMan->lookupNode(node)->succ; newNode != NULL_REG; newNode = s->_segMan->lookupNode(newNode)->succ) {
			reg_t b;
			if (selector == -1) {
				b = s->_segMan->lookupNode(newNode)->value;
			} else {
				b = readSelector(s->_segMan, s->_segMan->lookupNode(newNode)->value, selector);
			}

			if ((!isDescending && b < a) || (isDescending && a < b)) {
				firstNode = newNode;
				a = b;
			}
		}

		if (firstNode != node) {
			reg_t buf[4] = { argv[0], s->_segMan->lookupNode(firstNode)->key };
			kDeleteKey(s, 2, buf);

			buf[1] = node;
			buf[2] = firstNode;
			buf[3] = s->_segMan->lookupNode(firstNode)->value;
			kAddBefore(s, 4, buf);
		}
	}

	return s->r_acc;
}

reg_t kList(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kMoveToFront(EngineState *s, int argc, reg_t *argv) {
	error("Unimplemented function kMoveToFront called");
	return s->r_acc;
}

reg_t kMoveToEnd(EngineState *s, int argc, reg_t *argv) {
	error("Unimplemented function kMoveToEnd called");
	return s->r_acc;
}

reg_t kArray(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kArrayNew(EngineState *s, int argc, reg_t *argv) {
	uint16 size = argv[0].toUint16();
	SciArrayType type = (SciArrayType)argv[1].toUint16();

	if (type == kArrayTypeString) {
		++size;
	}

	reg_t arrayHandle;
	s->_segMan->allocateArray(type, size, &arrayHandle);
	return arrayHandle;
}

reg_t kArrayGetSize(EngineState *s, int argc, reg_t *argv) {
	const SciArray &array = *s->_segMan->lookupArray(argv[0]);
	return make_reg(0, array.size());
}

reg_t kArrayGetElement(EngineState *s, int argc, reg_t *argv) {
	if (getSciVersion() == SCI_VERSION_2_1_LATE) {
		return kStringGetChar(s, argc, argv);
	}

	SciArray &array = *s->_segMan->lookupArray(argv[0]);
	return array.getAsID(argv[1].toUint16());
}

reg_t kArraySetElements(EngineState *s, int argc, reg_t *argv) {
	SciArray &array = *s->_segMan->lookupArray(argv[0]);
	array.setElements(argv[1].toUint16(), argc - 2, argv + 2);
	return argv[0];
}

reg_t kArrayFree(EngineState *s, int argc, reg_t *argv) {
	if (getSciVersion() == SCI_VERSION_2_1_LATE && !s->_segMan->isValidAddr(argv[0], SEG_TYPE_ARRAY)) {
		return s->r_acc;
	}

	s->_segMan->freeArray(argv[0]);
	return s->r_acc;
}

reg_t kArrayFill(EngineState *s, int argc, reg_t *argv) {
	SciArray &array = *s->_segMan->lookupArray(argv[0]);
	array.fill(argv[1].toUint16(), argv[2].toUint16(), argv[3]);
	return argv[0];
}

reg_t kArrayCopy(EngineState *s, int argc, reg_t *argv) {
	SciArray &target = *s->_segMan->lookupArray(argv[0]);
	const uint16 targetIndex = argv[1].toUint16();
	const uint16 sourceIndex = argv[3].toUint16();
	const int16 count = argv[4].toSint16();

	if (!s->_segMan->isArray(argv[2])) {
		// String copies may be made from static script data
		SciArray source;
		source.setType(kArrayTypeString);
		source.fromString(s->_segMan->getString(argv[2]));
		target.copy(source, sourceIndex, targetIndex, count);
	} else {
		target.copy(*s->_segMan->lookupArray(argv[2]), sourceIndex, targetIndex, count);
	}

	return argv[0];
}

reg_t kArrayDuplicate(EngineState *s, int argc, reg_t *argv) {
	reg_t targetHandle;

	// String duplicates may be made from static script data
	if (!s->_segMan->isArray(argv[0])) {
		const Common::String source = s->_segMan->getString(argv[0]);
		SciArray &target = *s->_segMan->allocateArray(kArrayTypeString, source.size(), &targetHandle);
		target.fromString(source);
	} else {
		SciArray &source = *s->_segMan->lookupArray(argv[0]);
		SciArray &target = *s->_segMan->allocateArray(source.getType(), source.size(), &targetHandle);
		target = source;
	}

	return targetHandle;
}

reg_t kArrayGetData(EngineState *s, int argc, reg_t *argv) {
	if (s->_segMan->isObject(argv[0])) {
		return readSelector(s->_segMan, argv[0], SELECTOR(data));
	}

	return argv[0];
}

reg_t kArrayByteCopy(EngineState *s, int argc, reg_t *argv) {
	SciArray &target = *s->_segMan->lookupArray(argv[0]);
	const uint16 targetOffset = argv[1].toUint16();
	const SciArray &source = *s->_segMan->lookupArray(argv[2]);
	const uint16 sourceOffset = argv[3].toUint16();
	const uint16 count = argv[4].toUint16();

	target.byteCopy(source, sourceOffset, targetOffset, count);
	return argv[0];
}
#endif

} // End of namespace Sci
