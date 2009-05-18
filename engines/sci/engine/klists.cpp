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

Node *lookup_node(EngineState *s, reg_t addr) {
	if (!addr.offset && !addr.segment)
		return NULL; // Non-error null

	MemObject *mobj = GET_SEGMENT(*s->seg_manager, addr.segment, MEM_OBJ_NODES);
	if (!mobj) {
		// FIXME: This occurs right at the beginning of SQ4, when walking north from the first screen. It doesn't
		// seem to have any apparent ill-effects, though, so it's been changed to non-fatal, for now
		//sciprintf("%s, L%d: Attempt to use non-node "PREG" as list node\n", __FILE__, __LINE__, PRINT_REG(addr));
		//script_debug_flag = script_error_flag = 1;
		warning("Attempt to use non-node "PREG" as list node", PRINT_REG(addr));
		return NULL;
	}

	NodeTable *nt = (NodeTable *)mobj;

	if (!nt->isValidEntry(addr.offset)) {
		sciprintf("Attempt to use non-node "PREG" as list node\n", PRINT_REG(addr));
		script_debug_flag = script_error_flag = 1;
		return NULL;
	}

	return &(nt->_table[addr.offset]);
}

List *lookup_list(EngineState *s, reg_t addr) {
	MemObject *mobj = GET_SEGMENT(*s->seg_manager, addr.segment, MEM_OBJ_LISTS);

	if (!mobj) {
		sciprintf("Attempt to use non-list "PREG" as list\n", PRINT_REG(addr));
		script_debug_flag = script_error_flag = 1;
		return NULL;
	}

	ListTable *lt = (ListTable *)mobj;

	if (!lt->isValidEntry(addr.offset)) {
		sciprintf("Attempt to use non-list "PREG" as list\n", PRINT_REG(addr));
		script_debug_flag = script_error_flag = 1;
		return NULL;
	}

	return &(lt->_table[addr.offset]);
}

#ifdef DISABLE_VALIDATIONS

#define sane_nodep(a, b) 1
#define sane_listp(a, b) 1

#else

static int sane_nodep(EngineState *s, reg_t addr) {
	int have_prev = 0;
	reg_t prev = addr;

	do {
		Node *node = lookup_node(s, addr);

		if (!node)
			return 0;

		if ((have_prev) && !REG_EQ(node->pred, prev))
			return 0;

		prev = addr;
		addr = node->succ;
		have_prev = 1;
	} while (!IS_NULL_REG(addr));

	return 1;
}

int sane_listp(EngineState *s, reg_t addr) {
	List *l = lookup_list(s, addr);
	int empties = 0;

	if (IS_NULL_REG(l->first))
		++empties;
	if (IS_NULL_REG(l->last))
		++empties;

	// Either none or both must be set
	if (empties == 1)
		return 0;

	if (!empties) {
		Node *node_a, *node_z;

		node_a = lookup_node(s, l->first);
		node_z = lookup_node(s, l->last);

		if (!node_a || !node_z)
			return 0;

		if (!IS_NULL_REG(node_a->pred))
			return 0;

		if (!IS_NULL_REG(node_z->succ))
			return 0;

		return sane_nodep(s, l->first);
	}

	return 1; // Empty list is fine
}
#endif

reg_t kNewList(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t listbase;
	List *l;
	l = s->seg_manager->alloc_List(&listbase);
	l->first = l->last = NULL_REG;
	SCIkdebug(SCIkNODES, "New listbase at "PREG"\n", PRINT_REG(listbase));

	return listbase; // Return list base address
}

reg_t kDisposeList(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	List *l = lookup_list(s, argv[0]);

	if (!l) {
		// FIXME: This should be an error, but it's turned to a warning for now
		warning("Attempt to dispose non-list at "PREG"!\n", PRINT_REG(argv[0]));
		return NULL_REG;
	}

	if (!sane_listp(s, argv[0]))
		error("List at "PREG" is not sane anymore!\n", PRINT_REG(argv[0]));

/*	if (!IS_NULL_REG(l->first)) {
		reg_t n_addr = l->first;

		while (!IS_NULL_REG(n_addr)) { // Free all nodes
			Node *n = lookup_node(s, n_addr);
			s->seg_manager->free_Node(n_addr);
			n_addr = n->succ;
		}
	}

	s->seg_manager->free_list(argv[0]);
*/
	return s->r_acc;
}

reg_t _k_new_node(EngineState *s, reg_t value, reg_t key) {
	reg_t nodebase;
	Node *n = s->seg_manager->alloc_Node(&nodebase);

	if (!n) {
		KERNEL_OOPS("Out of memory while creating a node");
		return NULL_REG;
	}

	n->pred = n->succ = NULL_REG;
	n->key = key;
	n->value = value;

	return nodebase;
}

reg_t kNewNode(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	s->r_acc = _k_new_node(s, argv[0], argv[1]);

	SCIkdebug(SCIkNODES, "New nodebase at "PREG"\n", PRINT_REG(s->r_acc));

	return s->r_acc;
}

reg_t kFirstNode(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	if (IS_NULL_REG(argv[0]))
		return NULL_REG;
	List *l = lookup_list(s, argv[0]);

	if (l && !sane_listp(s, argv[0]))
		error("List at "PREG" is not sane anymore!\n", PRINT_REG(argv[0]));

	if (l)
		return l->first;
	else
		return NULL_REG;
}

reg_t kLastNode(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	List *l = lookup_list(s, argv[0]);

	if (l && !sane_listp(s, argv[0]))
		error("List at "PREG" is not sane anymore!\n", PRINT_REG(argv[0]));

	if (l)
		return l->last;
	else
		return NULL_REG;
}

reg_t kEmptyList(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	List *l = lookup_list(s, argv[0]);

	if (!l || !sane_listp(s, argv[0]))
		error("List at "PREG" is invalid or not sane anymore!\n", PRINT_REG(argv[0]));

	return make_reg(0, ((l) ? IS_NULL_REG(l->first) : 0));
}

void _k_add_to_front(EngineState *s, reg_t listbase, reg_t nodebase) {
	List *l = lookup_list(s, listbase);
	Node *new_n = lookup_node(s, nodebase);

	SCIkdebug(SCIkNODES, "Adding node "PREG" to end of list "PREG"\n", PRINT_REG(nodebase), PRINT_REG(listbase));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!new_n)
		warning("Attempt to add non-node ("PREG") to list at "PREG"\n", PRINT_REG(nodebase), PRINT_REG(listbase));
	if (!l || !sane_listp(s, listbase))
		error("List at "PREG" is not sane anymore!\n", PRINT_REG(listbase));

	new_n->succ = l->first;
	new_n->pred = NULL_REG;
	// Set node to be the first and last node if it's the only node of the list
	if (IS_NULL_REG(l->first))
		l->last = nodebase;
	else {
		Node *old_n = lookup_node(s, l->first);
		old_n->pred = nodebase;
	}
	l->first = nodebase;
}

void _k_add_to_end(EngineState *s, reg_t listbase, reg_t nodebase) {
	List *l = lookup_list(s, listbase);
	Node *new_n = lookup_node(s, nodebase);

	SCIkdebug(SCIkNODES, "Adding node "PREG" to end of list "PREG"\n", PRINT_REG(nodebase), PRINT_REG(listbase));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!new_n)
		warning("Attempt to add non-node ("PREG") to list at "PREG"\n", PRINT_REG(nodebase), PRINT_REG(listbase));
	if (!l || !sane_listp(s, listbase))
		error("List at "PREG" is not sane anymore!\n", PRINT_REG(listbase));

	new_n->succ = NULL_REG;
	new_n->pred = l->last;
	// Set node to be the first and last node if it's the only node of the list
	if (IS_NULL_REG(l->last))
		l->first = nodebase;
	else {
		Node *old_n = lookup_node(s, l->last);
		old_n->succ = nodebase;
	}
	l->last = nodebase;
}

reg_t kNextNode(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	Node *n = lookup_node(s, argv[0]);
	if (!sane_nodep(s, argv[0])) {
		error("List node at "PREG" is not sane anymore!\n", PRINT_REG(argv[0]));
		script_error_flag = script_debug_flag = 0;
		return NULL_REG;
	}

	return n->succ;
}

reg_t kPrevNode(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	Node *n = lookup_node(s, argv[0]);
	if (!sane_nodep(s, argv[0]))
		error("List node at "PREG" is not sane anymore!\n", PRINT_REG(argv[0]));

	return n->pred;
}

reg_t kNodeValue(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	Node *n = lookup_node(s, argv[0]);
	if (!sane_nodep(s, argv[0])) {
		error("List node at "PREG" is not sane", PRINT_REG(argv[0]));
		script_debug_flag = script_error_flag = 0;
		return NULL_REG;
	}

	return n->value;
}

reg_t kAddToFront(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	_k_add_to_front(s, argv[0], argv[1]);
	return s->r_acc;
}

reg_t kAddAfter(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	List *l =lookup_list(s, argv[0]);
	Node *firstnode = IS_NULL_REG(argv[1]) ? NULL : lookup_node(s, argv[1]);
	Node *newnode = lookup_node(s, argv[2]);

	if (!l || !sane_listp(s, argv[0]))
		error("List at "PREG" is not sane anymore", PRINT_REG(argv[0]));

	// FIXME: This should be an error, but it's turned to a warning for now
	if (!newnode) {
		warning("New 'node' "PREG" is not a node", PRINT_REG(argv[2]));
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

		if (IS_NULL_REG(oldnext))  // Appended after last node?
			// Set new node as last list node
			l->last = argv[2];
		else
			lookup_node(s, oldnext)->pred = argv[2];

		return s->r_acc;
	} else { // !firstnode
		// Prepare call to AddToFront...
		argv[1] = argv[0];
		return kAddToFront(s, funct_nr, 2, argv + 1); // Set as initial list node
	}
}

reg_t kAddToEnd(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	_k_add_to_end(s, argv[0], argv[1]);
	return s->r_acc;
}

reg_t kFindKey(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t node_pos;
	reg_t key = argv[1];
	reg_t list_pos = argv[0];

	SCIkdebug(SCIkNODES, "Looking for key "PREG" in list "PREG"\n", PRINT_REG(key), PRINT_REG(list_pos));

	if (!sane_listp(s, list_pos))
		error("List at "PREG" is not sane anymore", PRINT_REG(list_pos));

	node_pos = lookup_list(s, list_pos)->first;

	SCIkdebug(SCIkNODES, "First node at "PREG"\n", PRINT_REG(node_pos));

	while (!IS_NULL_REG(node_pos)) {
		Node *n = lookup_node(s, node_pos);
		if (REG_EQ(n->key, key)) {
			SCIkdebug(SCIkNODES, " Found key at "PREG"\n", PRINT_REG(node_pos));
			return node_pos;
		}

		node_pos = n->succ;
		SCIkdebug(SCIkNODES, "NextNode at "PREG"\n", PRINT_REG(node_pos));
	}

	SCIkdebug(SCIkNODES, "Looking for key without success\n");
	return NULL_REG;
}

reg_t kDeleteKey(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t node_pos = kFindKey(s, funct_nr, 2, argv);
	Node *n;
	List *l = lookup_list(s, argv[0]);

	if (IS_NULL_REG(node_pos))
		return NULL_REG; // Signal falure

	n = lookup_node(s, node_pos);
	if (REG_EQ(l->first, node_pos))
		l->first = n->succ;
	if (REG_EQ(l->last, node_pos))
		l->last = n->pred;

	if (!IS_NULL_REG(n->pred))
		lookup_node(s, n->pred)->succ = n->succ;
	if (!IS_NULL_REG(n->succ))
		lookup_node(s, n->succ)->pred = n->pred;

	//s->seg_manager->free_Node(node_pos);

	return make_reg(0, 1); // Signal success
}

struct sort_temp_t {
	reg_t key, value;
	reg_t order;
};

int sort_temp_cmp(const void *p1, const void *p2) {
	sort_temp_t *st1 = (sort_temp_t *)p1;
	sort_temp_t *st2 = (sort_temp_t *)p2;

	if (st1->order.segment < st1->order.segment || (st1->order.segment == st1->order.segment && st1->order.offset < st2->order.offset))
		return -1;

	if (st1->order.segment > st2->order.segment || (st1->order.segment == st2->order.segment && st1->order.offset > st2->order.offset))
		return 1;

	return 0;
}

reg_t kSort(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	reg_t source = argv[0];
	reg_t dest = argv[1];
	reg_t order_func = argv[2];

	int input_size = GET_SEL32SV(source, size);
	int i;

	sort_temp_t *temp_array = (sort_temp_t *)malloc(sizeof(sort_temp_t) * input_size);

	reg_t input_data = GET_SEL32(source, elements);
	reg_t output_data = GET_SEL32(dest, elements);

	List *list;
	Node *node;

	if (!input_size)
		return s->r_acc;

	if (IS_NULL_REG(output_data)) {
		list = s->seg_manager->alloc_List(&output_data);
		list->first = list->last = NULL_REG;
		PUT_SEL32(dest, elements, output_data);
	}

	PUT_SEL32V(dest, size, input_size);

	list = lookup_list(s, input_data);
	node = lookup_node(s, list->first);

	i = 0;
	while (node) {
		invoke_selector(INV_SEL(order_func, doit, 0), 1, node->value);
		temp_array[i].key = node->key;
		temp_array[i].value = node->value;
		temp_array[i].order = s->r_acc;
		i++;
		node = lookup_node(s, node->succ);
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
