/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/***************************************************************************/
/*                                                                         */
/*  ftutil.c                                                               */
/*    FreeType utility file for memory and list management (body).         */
/*                                                                         */
/***************************************************************************/


#include "engines/ags/lib/freetype-2.1.3/ft213build.h"
#include "engines/ags/lib/freetype-2.1.3/ftdebug.h"
#include "engines/ags/lib/freetype-2.1.3/ftmemory.h"
#include "engines/ags/lib/freetype-2.1.3/ftlist.h"

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_memory

namespace AGS3 {
namespace FreeType213 {


/**** MEMORY  MANAGEMENT ****/

FT_BASE_DEF(FT_Error)
FT_Alloc(FT_Memory memory, FT_Long size, void **P) {
	FT_ASSERT(P != 0);

	if (size > 0) {
		*P = memory->alloc(memory, size);
		if (!*P) {
			FT_ERROR(("FT_Alloc:"));
			FT_ERROR((" Out of memory? (%ld requested)\n", size));

			return FT_Err_Out_Of_Memory;
		}
		FT_MEM_ZERO(*P, size);
	} else
		*P = NULL;

	FT_TRACE7(("FT_Alloc:"));
	FT_TRACE7((" size = %ld, block = 0x%08p, ref = 0x%08p\n", size, *P, P));

	return FT_Err_Ok;
}

FT_BASE_DEF(FT_Error)
FT_Realloc(FT_Memory memory, FT_Long current, FT_Long size, void **P) {
	void *Q;

	FT_ASSERT(P != 0);

	/* if the original pointer is NULL, call FT_Alloc() */
	if (!*P)
		return FT_Alloc(memory, size, P);

	/* if the new block if zero-sized, clear the current one */
	if (size <= 0) {
		FT_Free(memory, P);
		return FT_Err_Ok;
	}

	Q = memory->realloc(memory, current, size, *P);
	if (!Q)
		goto Fail;

	if (size > current)
		FT_MEM_ZERO((char *)Q + current, size - current);

	*P = Q;
	return FT_Err_Ok;

Fail:
	FT_ERROR(("FT_Realloc:"));
	FT_ERROR((" Failed (current %ld, requested %ld)\n", current, size));
	return FT_Err_Out_Of_Memory;
}

FT_BASE_DEF(void)
FT_Free(FT_Memory memory, void **P) {
	FT_TRACE7(("FT_Free:"));
	FT_TRACE7((" Freeing block 0x%08p, ref 0x%08p\n", P, P ? *P : (void *)0));

	if (P && *P) {
		memory->free(memory, *P);
		*P = 0;
	}
}


/**** DOUBLE LINKED LISTS ****/

#undef  FT_COMPONENT
#define FT_COMPONENT  trace_list

FT_EXPORT_DEF(FT_ListNode)
FT_List_Find(FT_List list, void *data) {
	FT_ListNode cur;

	cur = list->head;
	while (cur) {
		if (cur->data == data)
			return cur;

		cur = cur->next;
	}

	return (FT_ListNode)0;
}

FT_EXPORT_DEF(void)
FT_List_Add(FT_List list, FT_ListNode node) {
	FT_ListNode before = list->tail;

	node->next = 0;
	node->prev = before;

	if (before)
		before->next = node;
	else
		list->head = node;

	list->tail = node;
}

FT_EXPORT_DEF(void)
FT_List_Insert(FT_List list, FT_ListNode node) {
	FT_ListNode after = list->head;

	node->next = after;
	node->prev = 0;

	if (!after)
		list->tail = node;
	else
		after->prev = node;

	list->head = node;
}

FT_EXPORT_DEF(void)
FT_List_Remove(FT_List list, FT_ListNode node) {
	FT_ListNode before, after;

	before = node->prev;
	after = node->next;

	if (before)
		before->next = after;
	else
		list->head = after;

	if (after)
		after->prev = before;
	else
		list->tail = before;
}

FT_EXPORT_DEF(void)
FT_List_Up(FT_List list, FT_ListNode node) {
	FT_ListNode before, after;

	before = node->prev;
	after = node->next;

	/* check whether we are already on top of the list */
	if (!before)
		return;

	before->next = after;

	if (after)
		after->prev = before;
	else
		list->tail = before;

	node->prev = 0;
	node->next = list->head;
	list->head->prev = node;
	list->head = node;
}

FT_EXPORT_DEF(FT_Error)
FT_List_Iterate(FT_List list, FT_List_Iterator iterator, void *user) {
	FT_ListNode cur = list->head;
	FT_Error error = FT_Err_Ok;

	while (cur) {
		FT_ListNode next = cur->next;

		error = iterator(cur, user);
		if (error)
			break;

		cur = next;
	}

	return error;
}

FT_EXPORT_DEF(void)
FT_List_Finalize(FT_List list, FT_List_Destructor destroy, FT_Memory memory, void *user) {
	FT_ListNode cur;

	cur = list->head;
	while (cur) {
		FT_ListNode next = cur->next;
		void *data = cur->data;

		if (destroy)
			destroy(memory, data, user);

		FT_FREE(cur);
		cur = next;
	}

	list->head = 0;
	list->tail = 0;
}

} // End of namespace FreeType213
} // End of namespace AGS3
