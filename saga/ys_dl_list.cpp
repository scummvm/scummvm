/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "saga/saga.h"
#include "saga/yslib.h"

namespace Saga {

YS_DL_LIST *ys_dll_create() {
	YS_DL_LIST *new_list;

	new_list = (YS_DL_LIST *)malloc(sizeof *new_list);

	if (new_list != NULL) {
		new_list->next = new_list;
		new_list->prev = new_list;

		// Sentinel is marked by self-referential node data. 
		// No other link is permitted to do this
		new_list->data = new_list;
	}

	return new_list;
}

void ys_dll_destroy(YS_DL_LIST *list) {
	YS_DL_NODE *walk_p;
	YS_DL_NODE *temp_p;

	for (walk_p = list->next; walk_p != list; walk_p = temp_p) {
		temp_p = walk_p->next;
		free(walk_p->data);
		free(walk_p);
	}

	free(list);

	return;
}

void *ys_dll_get_data(YS_DL_NODE *node) {
	return node->data;
}

YS_DL_NODE *ys_dll_head(YS_DL_LIST *list) {
	return (list->next != list) ? list->next : NULL;
}

YS_DL_NODE *ys_dll_tail(YS_DL_LIST *list) {
	return (list->prev != list) ? list->prev : NULL;
}

YS_DL_NODE *ys_dll_next(YS_DL_NODE *node) {
	return (node->next != (YS_DL_LIST *) node->next->data) ? node->next : NULL;
}

YS_DL_NODE *ys_dll_prev(YS_DL_NODE *node) {
	return (node->prev != (YS_DL_LIST *) node->prev->data) ? node->prev : NULL;
}

YS_DL_NODE *ys_dll_add_head(YS_DL_LIST *list, void *data, size_t size) {
	YS_DL_NODE *new_node;
	void *new_data;

	new_node = (YS_DL_NODE *)malloc(sizeof *new_node);

	if (new_node) {
		new_data = malloc(size);

		if (new_data) {
			memcpy(new_data, data, size);
			new_node->data = new_data;
			new_node->prev = list;
			new_node->next = list->next;
			new_node->next->prev = new_node;
			list->next = new_node;
		}
	}
	return new_node;
}

YS_DL_NODE *ys_dll_add_tail(YS_DL_LIST *list, void *data, size_t size) {
	YS_DL_NODE *new_node;
	void *new_data;

	new_node = (YS_DL_NODE *)malloc(sizeof *new_node);

	if (new_node != NULL) {
		new_data = malloc(size);

		if (new_data != NULL) {
			memcpy(new_data, data, size);
			new_node->data = new_data;
			new_node->next = list;
			new_node->prev = list->prev;
			new_node->prev->next = new_node;
			list->prev = new_node;
		}
	}
	return new_node;
}

YS_DL_NODE *ys_dll_insert(YS_DL_NODE *list, void *data, size_t size, YS_COMPARE_FUNC *compare) {
	YS_DL_NODE *walk_p;
	YS_DL_NODE *new_node;
	int result;

	for (walk_p = list->next; walk_p != list; walk_p = walk_p->next) {
		result = compare(data, walk_p->data);
		if (result < 0) {
			new_node = ys_dll_preinsert(walk_p, data, size);
			return new_node;
		}
	}

	new_node = ys_dll_add_tail(list, data, size);
	return new_node;
}

int ys_dll_delete(YS_DL_NODE *node) {
	if (node == NULL) {
		return YS_E_FAILURE;
	}

	node->next->prev = node->prev;
	node->prev->next = node->next;

	free(node->data);
	free(node);

	return YS_E_SUCCESS;
}

void ys_dll_delete_all(YS_DL_LIST *list) {
	YS_DL_NODE *walk_p;
	YS_DL_NODE *temp_p;

	for (walk_p = list->next; walk_p != list; walk_p = temp_p) {
		temp_p = walk_p->next;
		free(walk_p->data);
		free(walk_p);
	}

	list->next = list;
	list->prev = list;
}

YS_DL_NODE *ys_dll_replace(YS_DL_NODE *node, void *data, size_t size) {
	void *new_data;

	if ((node == NULL) || (data == NULL) || (!size)) {
		return NULL;
	}

	new_data = malloc(size);

	if (new_data == NULL) {
		return NULL;
	}

	free(node->data);
	node->data = new_data;
	memcpy(new_data, data, size);

	return node;
}

int ys_dll_reorder_up(YS_DL_NODE *list, YS_DL_NODE *olink, YS_COMPARE_FUNC *compare) {
	YS_DL_NODE *walk_p;
	int result;
	int reorder = 0;

	for (walk_p = olink->prev; walk_p != list; walk_p = walk_p->prev) {
		result = compare(walk_p->data, olink->data);
		if (result <= 0) {
			reorder = 1;
			break;
		}
	}

	if (reorder) {
		// Unlink original link
		olink->next->prev = olink->prev;
		olink->prev->next = olink->next;
		// Reinsert after walk link
		olink->prev = walk_p;
		olink->next = walk_p->next;
		olink->next->prev = olink;
		walk_p->next = olink;
	}
	return YS_E_SUCCESS;
}

int ys_dll_reorder_down(YS_DL_NODE *list, YS_DL_NODE *olink, YS_COMPARE_FUNC *compare) {
	YS_DL_NODE *walk_p;
	int result;
	int reorder = 0;

	for (walk_p = olink->next; walk_p != list; walk_p = walk_p->next) {
		result = compare(walk_p->data, olink->data);
		if (result >= 0) {
			reorder = 1;
			break;
		}
	}

	if (reorder) {
		// Unlink original link
		olink->next->prev = olink->prev;
		olink->prev->next = olink->next;
		//Reinsert before walk link
		olink->next = walk_p;
		olink->prev = walk_p->prev;
		olink->prev->next = olink;
		walk_p->prev = olink;
	}
	return YS_E_SUCCESS;
}

} // End of namespace Saga
