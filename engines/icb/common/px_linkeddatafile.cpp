/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_string.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_clu_api.h"
#include "engines/icb/common/px_linkeddatafile.h"

namespace ICB {

// get NUMBER given HASH (does the search) - hashs must be ordered for this binary search to succeed.
uint32 LinkedDataObject::Fetch_item_number_by_hash(LinkedDataFile *file, const uint32 hash) {
	int32 top, bottom;
	int32 i;
	uint32 current;

	if (!FROM_LE_32(file->number_of_items))
		return PX_LINKED_DATA_FILE_ERROR;

	top = FROM_LE_32(file->number_of_items) - 1;
	i = top >> 1;
	bottom = 0;
	for (;;) {
		current = FROM_LE_32(file->list[i].name_hash_value);
		if (hash == current)
			return i;
		else if (top == bottom)
			return PX_LINKED_DATA_FILE_ERROR;
		else if (hash > current) {
			bottom = i + 1;
			i = (top + bottom) >> 1;
		} else {
			top = i;
			i = (top + bottom) >> 1;
		}
	}
}

} // End of namespace ICB
