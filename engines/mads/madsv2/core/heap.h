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

#ifndef MADS_CORE_HEAP_H
#define MADS_CORE_HEAP_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

typedef struct {
	byte module;		/* Identifier for heap module   */
	byte destruct;		/* Flag if can be destroyed     */
	char *base;			/* Base memory pointer for heap */
	char *marker;		/* Pointer to free area         */
	long base_size;		/* Original size of the heap    */
	long size;			/* Free space remaining         */
} Heap;

int heap_create(Heap *heap, int module_id, long heap_size, const char *heap_name);
void heap_destroy(Heap *heap);
void heap_declare(Heap *heap, int module_id, char *buffer, long heap_size);

void *heap_get(Heap *heap, long size);

} // namespace MADSV2
} // namespace MADS

#endif
