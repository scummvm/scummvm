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

#ifndef M4_MEM_MEM_H
#define M4_MEM_MEM_H

#include "common/str.h"

namespace M4 {

/**
 * Initialize the memory manager.
 */
void mem_stash_init(int16 num_types);

/**
 * Purges the memory manager
 */
void mem_stash_shutdown();

bool mem_register_stash_type(int32 *memType, int32 size, int32 numRequests, const Common::String &name);

/**
 * To free a memory block whose size has been previously registered.
 * @param myMem		The pointer to be freed
 */
void mem_free_to_stash(void *myMem, int32 memType);

/**
 * Deliver a memory block whose size has been previously registered.
 */
void *mem_get_from_stash(int32 memType, const Common::String &name);

char *mem_strdup(const char *str);

} // namespace M4

#endif
