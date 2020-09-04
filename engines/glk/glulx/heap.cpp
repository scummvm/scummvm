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

#include "glk/glulx/glulx.h"

namespace Glk {
namespace Glulx {

void Glulx::heap_clear() {
	while (heap_head) {
		heapblock_t *blo = heap_head;
		heap_head = blo->next;
		blo->next = nullptr;
		blo->prev = nullptr;
		glulx_free(blo);
	}
	heap_tail = nullptr;

	if (heap_start) {
		uint res = change_memsize(heap_start, true);
		if (res)
			fatal_error_i("Unable to revert memory size when deactivating heap.",
			              heap_start);
	}

	heap_start = 0;
	alloc_count = 0;
	/* heap_sanity_check(); */
}

int Glulx::heap_is_active() const {
	return (heap_start != 0);
}

uint Glulx::heap_get_start() const {
	return heap_start;
}

uint Glulx::heap_alloc(uint len) {
	heapblock_t *blo, *newblo;

#ifdef FIXED_MEMSIZE
	return 0;
#else /* FIXED_MEMSIZE */

	if (len <= 0)
		fatal_error("Heap allocation length must be positive.");

	blo = heap_head;
	while (blo) {
		if (blo->isfree && blo->len >= len)
			break;

		if (!blo->isfree) {
			blo = blo->next;
			continue;
		}

		if (!blo->next || !blo->next->isfree) {
			blo = blo->next;
			continue;
		}

		/* This is a free block, but the next block in the list is also
		   free, so we "advance" by merging rather than by going to
		   blo->next. */
		newblo = blo->next;
		blo->len += newblo->len;
		if (newblo->next) {
			blo->next = newblo->next;
			newblo->next->prev = blo;
		} else {
			blo->next = nullptr;
			heap_tail = blo;
		}
		newblo->next = nullptr;
		newblo->prev = nullptr;
		glulx_free(newblo);
		newblo = nullptr;
		continue;
	}

	if (!blo) {
		/* No free area is visible on the list. Try extending memory. How
		   much? Double the heap size, or by 256 bytes, or by the memory
		   length requested -- whichever is greatest. */
		uint res;
		uint extension;
		uint oldendmem = endmem;

		extension = 0;
		if (heap_start)
			extension = endmem - heap_start;
		if (extension < len)
			extension = len;
		if (extension < 256)
			extension = 256;
		/* And it must be rounded up to a multiple of 256. */
		extension = (extension + 0xFF) & (~(uint)0xFF);

		res = change_memsize(endmem + extension, true);
		if (res)
			return 0;

		/* If we just started the heap, note that. */
		if (heap_start == 0)
			heap_start = oldendmem;

		if (heap_tail && heap_tail->isfree) {
			/* Append the new space to the last block. */
			blo = heap_tail;
			blo->len += extension;
		} else {
			/* Append the new space to the block list, as a new block. */
			newblo = (heapblock_t *)glulx_malloc(sizeof(heapblock_t));
			if (!newblo)
				fatal_error("Unable to allocate record for heap block.");
			newblo->addr = oldendmem;
			newblo->len = extension;
			newblo->isfree = true;
			newblo->next = nullptr;
			newblo->prev = nullptr;

			if (!heap_tail) {
				heap_head = newblo;
				heap_tail = newblo;
			} else {
				blo = heap_tail;
				heap_tail = newblo;
				blo->next = newblo;
				newblo->prev = blo;
			}

			blo = newblo;
			newblo = nullptr;
		}

		/* and continue forwards, using this new block (blo). */
	}

	/* Something strange happened. */
	if (!blo || !blo->isfree || blo->len < len)
		return 0;

	/* We now have a free block of size len or longer. */

	if (blo->len == len) {
		blo->isfree = false;
	} else {
		newblo = (heapblock_t *)glulx_malloc(sizeof(heapblock_t));
		if (!newblo)
			fatal_error("Unable to allocate record for heap block.");
		newblo->isfree = true;
		newblo->addr = blo->addr + len;
		newblo->len = blo->len - len;
		blo->len = len;
		blo->isfree = false;
		newblo->next = blo->next;
		if (newblo->next)
			newblo->next->prev = newblo;
		newblo->prev = blo;
		blo->next = newblo;
		if (heap_tail == blo)
			heap_tail = newblo;
	}

	alloc_count++;
	/* heap_sanity_check(); */
	return blo->addr;

#endif /* FIXED_MEMSIZE */
}

void Glulx::heap_free(uint addr) {
	heapblock_t *blo;

	for (blo = heap_head; blo; blo = blo->next) {
		if (blo->addr == addr)
			break;
	};
	if (!blo || blo->isfree)
		fatal_error_i("Attempt to free unallocated address from heap.", addr);

	blo->isfree = true;
	alloc_count--;
	if (alloc_count <= 0) {
		heap_clear();
	}

	/* heap_sanity_check(); */
}

int Glulx::heap_get_summary(uint *valcount, uint **summary) {
	uint *arr, len, pos;
	heapblock_t *blo;

	*valcount = 0;
	*summary = nullptr;

	if (heap_start == 0)
		return 0;

	len = 2 + 2 * alloc_count;
	arr = (uint *)glulx_malloc(len * sizeof(uint));
	if (!arr)
		return 1;

	pos = 0;
	arr[pos++] = heap_start;
	arr[pos++] = alloc_count;

	for (blo = heap_head; blo; blo = blo->next) {
		if (blo->isfree)
			continue;
		arr[pos++] = blo->addr;
		arr[pos++] = blo->len;
	}

	if (pos != len)
		fatal_error("Wrong number of active blocks in heap");

	*valcount = len;
	*summary = arr;
	return 0;
}

int Glulx::heap_apply_summary(uint valcount, uint *summary) {
	uint lx, jx, lastend;

	if (heap_start)
		fatal_error("Heap active when heap_apply_summary called");

	if (valcount == 0 || summary == nullptr)
		return 0;
	if (valcount == 2 && summary[0] == 0 && summary[1] == 0)
		return 0;

#ifdef FIXED_MEMSIZE
	return 1;
#else /* FIXED_MEMSIZE */

	lx = 0;
	heap_start = summary[lx++];
	alloc_count = summary[lx++];

	for (jx = lx; jx + 2 < valcount; jx += 2) {
		if (summary[jx] >= summary[jx + 2])
			fatal_error("Heap block summary is out of order.");
	}

	lastend = heap_start;

	while (lx < valcount || lastend < endmem) {
		heapblock_t *blo;

		blo = (heapblock_t *)glulx_malloc(sizeof(heapblock_t));
		if (!blo)
			fatal_error("Unable to allocate record for heap block.");

		if (lx >= valcount) {
			blo->addr = lastend;
			blo->len = endmem - lastend;
			blo->isfree = true;
		} else {
			if (lastend < summary[lx]) {
				blo->addr = lastend;
				blo->len = summary[lx] - lastend;
				blo->isfree = true;
			} else {
				blo->addr = summary[lx++];
				blo->len = summary[lx++];
				blo->isfree = false;
			}
		}

		blo->prev = nullptr;
		blo->next = nullptr;

		if (!heap_head) {
			heap_head = blo;
			heap_tail = blo;
		} else {
			heap_tail->next = blo;
			blo->prev = heap_tail;
			heap_tail = blo;
		}

		lastend = blo->addr + blo->len;
	}

	/* heap_sanity_check(); */

	return 0;
#endif /* FIXED_MEMSIZE */
}

} // End of namespace Glulx
} // End of namespace Glk
