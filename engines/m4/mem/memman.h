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

#ifndef M4_MEM_MEMMAN_H
#define M4_MEM_MEMMAN_H

#include "common/algorithm.h"
#include "m4/m4_types.h"

namespace M4 {

#define _MEMTYPE_LIMIT 33

inline Handle mem_alloc(size_t size, const char *) {
	byte *ptr = (byte *)malloc(size);
	Common::fill(ptr, ptr + size, 0);
	return (Handle)ptr;
}

inline void mem_free(Handle ptr) {
	free(ptr);
}

inline Handle mem_realloc(Handle src, size_t new_size, const char *) {
	return (Handle)realloc(src, new_size);
}

} // namespace M4

#endif
