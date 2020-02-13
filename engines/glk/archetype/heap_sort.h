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

#ifndef ARCHETYPE_HEAP_SORT
#define ARCHETYPE_HEAP_SORT

#include "glk/archetype/array.h"
#include "glk/archetype/string.h"

namespace Glk {
namespace Archetype {

typedef XArrayType HeapType;
typedef void *Element;

extern HeapType H;

// Public methods
extern void heap_sort_init();
extern bool pop_heap(Element &e);
extern void drop_on_heap(Element e);
extern void drop_str_on_heap(const String &s);
extern void reinit_heap();


} // End of namespace Archetype
} // End of namespace Glk

#endif
