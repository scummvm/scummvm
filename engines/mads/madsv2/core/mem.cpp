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

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/error.h"

namespace MADS {
namespace MADSV2 {

byte  mem_initialized = false;

byte  mem_max_free_set = false;
byte  umb_max_free_set = false;

byte mem_last_alloc_failed = false;
int  mem_last_alloc_loader = MODULE_MEMORY;
long mem_last_alloc_size = 0;
long mem_last_alloc_avail = 0;
long mem_last_alloc_umb = 0;
long mem_max_free = -1;
long mem_min_free = 100000000;
long umb_max_free = -1;
long umb_min_free = 100000000;

long mem_save_free_area;
long umb_save_free_area;

void (*mem_manager_update)() = NULL;
int mem_manager_active = false;


void *mem_get_name(long size, const char *) {
	byte *memory_block = nullptr;
	if (size > 0) {
		memory_block = (byte *)malloc(size);
		Common::fill(memory_block, memory_block + size, 0);
	}

	return memory_block;
}

void *mem_get(long size) {
	return (mem_get_name(size, "$sys$"));
}

void mem_get_block_name(byte *block, char *block_name) {
	// TODO: See if the block_name is needed. If so, we'll need to simulate the
	// original by allocating extra space to store it
	error("TODO: mem_get_block_name");
}


bool mem_free(void *memory_block) {
	free(memory_block);
	return false;
}

int mem_adjust_impl(void *&target, long size) {
	void *p = realloc(target, size);
	if (p == NULL)
		return -1;
	target = p;
	return 0;
}

void mem_save_free(void) {
	// No implementation
}

void mem_restore_free(void) {
	// No implementation
}

long mem_conv_get_avail() {
	return 999999;
}

long mem_get_avail(void) {
	return 999999;
}

void mem_set_video_mode(int mode) {
	// No implementation
}

} // namespace MADSV2
} // namespace MADS
