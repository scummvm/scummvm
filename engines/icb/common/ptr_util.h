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

#ifndef ICB_PTR_UTIL_H
#define ICB_PTR_UTIL_H

#include "engines/icb/common/px_common.h"

#include "common/array.h"

namespace ICB {

// map pointers to 32-bit references (bottom 32-bits of address)
struct PointerReference {
	uint32 ref;
	uint8 *ptr;
};

extern Common::Array<PointerReference> *g_ptrArray;

namespace MemoryUtil {

uint32 encodePtr(uint8 *ptr);
uint8 *resolvePtr(uint32 ptrRef);
void clearAllPtrs();
}

#endif // PTR_UTIL_H

} // End of namespace ICB
