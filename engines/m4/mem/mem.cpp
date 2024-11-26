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
#include "m4/mem/mem.h"
#include "m4/mem/memman.h"
#include "m4/core/errors.h"
#include "m4/vars.h"

namespace M4 {

#define MAX_REQUESTS 255

void mem_stash_init(int16 num_types) {
	if (num_types > _MEMTYPE_LIMIT)
		error_show(FL, 'MSIF', "num_types (%d) _MEMTYPE_LIMIT (%d)", num_types, _MEMTYPE_LIMIT);

	for (int i = 0; i < _MEMTYPE_LIMIT; i++) {
		_G(memBlock)[i] = nullptr;
		_G(sizeMem)[i] = 0;
		_G(requests)[i] = 0;
	}
}

void mem_stash_shutdown(void) {
	for (int i = 0; i < _MEMTYPE_LIMIT; i++) {
		if (_G(memBlock)[i]) {
			mem_free(_G(memBlock)[i]);
			_G(memBlock)[i] = nullptr;
		}
	}
}

bool mem_register_stash_type(int32 *memType, int32 blockSize, int32 maxNumRequests, const Common::String &name) {
	int32 i = 0;
	bool found = false;

	while ((i < _MEMTYPE_LIMIT) && (_G(sizeMem)[i] > 0) && (!found)) {
		i++;
	}
	if (i == _MEMTYPE_LIMIT)
		error_show(FL, 'MSIF', "stash: %s", name.c_str());

	// Found a slot
	if (found || (i < _MEMTYPE_LIMIT)) {
		_G(sizeMem)[i] = blockSize;
		*memType = i;

		if (maxNumRequests > MAX_REQUESTS)
			maxNumRequests = MAX_REQUESTS;

		_G(requests)[i] = maxNumRequests;

		_G(memBlock)[i] = mem_alloc((blockSize + sizeof(uintptr)) * maxNumRequests, name.c_str());
		memset(_G(memBlock)[i], 0, (blockSize + sizeof(uintptr)) * maxNumRequests);

		return true;
	}

	error_show(FL, 'MSIF', "stash: %s", name.c_str());
	return false;
}

void mem_free_to_stash(void *mem, int32 memType) {
	// _G(memBlock)[memType] is block associated with memType
	int8 *b_ptr = (int8 *)_G(memBlock)[memType];
	int32 index = ((intptr)mem - (intptr)_G(memBlock)[memType]) / (_G(sizeMem)[memType] + sizeof(uintptr));

	if (index < 0 || index > _G(requests)[memType])
		error_show(FL, 'MSGF');

	b_ptr += index * (_G(sizeMem)[memType] + sizeof(uintptr));
	*(uintptr *)b_ptr = 0;
}

void *mem_get_from_stash(int32 memType, const Common::String &name) {
	int i;
	int8 *b_ptr = (int8 *)_G(memBlock)[memType];

	for (i = 0; i < _G(requests)[memType]; i++) {
		if (!*(uintptr *)b_ptr) {
			*(uintptr *)b_ptr = 1;
			void *result = (void *)(b_ptr + sizeof(uintptr));
			Common::fill((byte *)result, (byte *)result + _G(sizeMem)[memType], 0);
			return result;

		} else {
			b_ptr += _G(sizeMem)[memType] + sizeof(uintptr);
		}
	}

	error_show(FL, 'OOS!', "stash full %s", name.c_str());
	return 0;
}

char *mem_strdup(const char *str) {
	char *new_str = nullptr;

	if (!str) {
		new_str = (char *)mem_alloc(1, "string");
		new_str[0] = '\0';
		return new_str;
	}

	new_str = (char *)mem_alloc(strlen(str) + 1, "string");
	if (!new_str)
		return nullptr;

	Common::strcpy_s(new_str, 256, str);
	return new_str;
}


} // namespace M4
