/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"

#include "common/array.h"

namespace ICB {

namespace MemoryUtil {

const int PTR_ARRAY_MAX(1024);

// map pointers to 32-bit references (bottom 32-bits of address)
struct PointerReference {
	uint32 ref;
	uint8 *ptr;
};

Common::Array<PointerReference> ptrArray;

uint32 encodePtr(uint8 *ptr) {
	PointerReference ptrRef;

	ptrdiff_t diff = ptr - (uint8 *)0;
	ptrRef.ref = (uint32)(diff & 0xFFFFFFFF);
	ptrRef.ptr = ptr;

	// find free slot
	for (Common::Array<PointerReference>::iterator it = ptrArray.begin(); it < ptrArray.end(); it++) {
		if (it->ref == 0) {
			*it = ptrRef; // store
			return ptrRef.ref;
		}
	}

	// append
	ptrArray.push_back(ptrRef);

	if (ptrArray.size() >= (uint)PTR_ARRAY_MAX) {
		error("MemoryUtil::encodePtr(): too many pointers (MAX = %u)\n", PTR_ARRAY_MAX);
	}

	return ptrRef.ref;
}

uint8 *resolvePtr(uint32 ref) {
	if (ref == 0)
		return NULL;

	// do a linear search
	for (Common::Array<PointerReference>::iterator it = ptrArray.begin(); it < ptrArray.end(); it++) {
		if (it->ref == ref) {
			uint8 *ptr = it->ptr;

			// purge
			it->ref = 0;
			it->ptr = 0;

			return ptr;
		}
	}

	error("MemoryUtil::resolvePtr(%08x) COULD NOT RESOLVE POINTER!\n", ref);

	return NULL;
}

void clearAllPtrs(void) { ptrArray.clear(); }
}

} // End of namespace ICB
