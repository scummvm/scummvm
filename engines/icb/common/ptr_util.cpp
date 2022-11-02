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

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/ptr_util.h"

#include "common/array.h"

namespace ICB {

Common::Array<PointerReference> *g_ptrArray;

namespace MemoryUtil {

const int32 PTR_ARRAY_MAX(1024);

uint32 encodePtr(uint8 *ptr) {
	PointerReference ptrRef;

	ptrdiff_t diff = ptr - (uint8 *)nullptr;
	ptrRef.ref = (uint32)(diff & 0xFFFFFFFF);
	ptrRef.ptr = ptr;

	// find free slot
	for (Common::Array<PointerReference>::iterator it = g_ptrArray->begin(); it < g_ptrArray->end(); it++) {
		if (it->ref == 0) {
			*it = ptrRef; // store
			return ptrRef.ref;
		}
	}

	// append
	g_ptrArray->push_back(ptrRef);

	if (g_ptrArray->size() >= (uint)PTR_ARRAY_MAX) {
		error("MemoryUtil::encodePtr(): too many pointers (MAX = %u)\n", PTR_ARRAY_MAX);
	}

	return ptrRef.ref;
}

uint8 *resolvePtr(uint32 ref) {
	if (ref == 0)
		return nullptr;

	// do a linear search
	for (Common::Array<PointerReference>::iterator it = g_ptrArray->begin(); it < g_ptrArray->end(); it++) {
		if (it->ref == ref) {
			uint8 *ptr = it->ptr;

			// purge
			it->ref = 0;
			it->ptr = nullptr;

			return ptr;
		}
	}

	error("MemoryUtil::resolvePtr(%08x) COULD NOT RESOLVE POINTER!\n", ref);

	return nullptr;
}

void clearAllPtrs(void) { g_ptrArray->clear(); }
}

} // End of namespace ICB
