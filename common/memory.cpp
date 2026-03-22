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

template<typename T>
static inline void memsetXX(T *dst, T val, size_t count) {
	while(count >= 8) {
		dst[0] = val;
		dst[1] = val;
		dst[2] = val;
		dst[3] = val;
		dst[4] = val;
		dst[5] = val;
		dst[6] = val;
		dst[7] = val;
		dst += 8;
		count -= 8;
	}
	switch(count) {
	case 7:
		*(dst++) = val;
		/* fall through */
	case 6:
		*(dst++) = val;
		/* fall through */
	case 5:
		*(dst++) = val;
		/* fall through */
	case 4:
		*(dst++) = val;
		/* fall through */
	case 3:
		*(dst++) = val;
		/* fall through */
	case 2:
		*(dst++) = val;
		/* fall through */
	case 1:
		*(dst++) = val;
		/* fall through */
	case 0:
		break;
	}
}

void memset64(uint64 *dst, uint64 val, size_t count) {
	memsetXX<uint64>(dst, val, count);
}

void memset32(uint32 *dst, uint32 val, size_t count) {
	memsetXX<uint32>(dst, val, count);
}

void memset16(uint16 *dst, uint16 val, size_t count) {
	memsetXX<uint16>(dst, val, count);
}

} // End of namespace Common
