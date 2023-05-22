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

#include "common/memory.h"
#include "common/util.h"

namespace Common {

void memset8(uint64 *dst, uint64 val, size_t count) {
	if (!count)
		return;

	int n = (count + 7) >> 3;
	switch (count % 8) {
	default:
	case 0:	do {
	       		*dst++ = val; // fall through
	case 7:		*dst++ = val; // fall through
	case 6:		*dst++ = val; // fall through
	case 5:		*dst++ = val; // fall through
	case 4:		*dst++ = val; // fall through
	case 3:		*dst++ = val; // fall through
	case 2:		*dst++ = val; // fall through
	case 1:		*dst++ = val;
	       	} while (--n > 0);
	}
}

void memset4(uint32 *dst, uint32 val, size_t count) {
	if (!IS_ALIGNED(dst, 8) && count) {
		*dst++ = val;
		count -= 1;
	}

	memset8((uint64 *)dst, val | ((uint64)val << 32), count >> 1);

	if (count & 1) {
		dst[count & ~1] = val;
	}
}

void memset2(uint16 *dst, uint16 val, size_t count) {
	if (!IS_ALIGNED(dst, 4) && count) {
		*dst++ = val;
		count -= 1;
	}

	memset4((uint32 *)dst, val | ((uint32)val << 16), count >> 1);

	if (count & 1) {
		dst[count & ~1] = val;
	}
}

} // End of namespace Common
