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

#include "mads/madsv2/core/heap.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {

int heap_create(Heap *heap, int module_id, long heap_size, const char *heap_name) {
	int error_flag = true;

	heap->module = (byte)module_id;

	heap->base = (char *)mem_get_name(heap_size, heap_name);
	if (heap->base == NULL) {
		error_report(ERROR_HEAP_REQUEST_FAILED, ERROR, module_id, heap_size, mem_get_avail());
		goto done;
	}

	heap->destruct = true;
	heap->marker = heap->base;
	heap->size = heap_size;
	heap->base_size = heap_size;

	error_flag = false;

done:
	return error_flag;
}

void heap_declare(Heap *heap, int module_id, char *buffer, long heap_size) {
	heap->destruct = false;
	heap->module = (byte)module_id;

	heap->base = heap->marker = buffer;
	heap->base_size = heap->size = heap_size;
}

void heap_destroy(Heap *heap) {
	if (heap->destruct) {
		mem_free(heap->base);
	}

	heap->base = NULL;
	heap->size = 0;
	heap->base_size = 0;
}

void *heap_get(Heap *heap, long size) {
	char *result = NULL;

	if (size > heap->size) {
		error_report(ERROR_NO_MORE_HEAP, ERROR, heap->module, size, heap->size);
		goto done;
	}

	result = heap->marker;
	heap->marker += size;
	heap->size -= size;

done:
	return (void *)result;
}

} // namespace MADSV2
} // namespace MADS
