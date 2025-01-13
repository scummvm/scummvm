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

#include "got/utils/compression.h"
#include "common/algorithm.h"
#include "common/endian.h"

namespace Got {

void lzss_decompress(const byte *src, byte *dest, size_t destSize) {
	byte *endP = dest + destSize;

	for (;;) {
		byte v = *src++;

		for (int bits = 8; bits > 0; --bits) {
			if (endP == dest)
				return;

			bool bit = (v & 1) != 0;
			v >>= 1;
			if (bit) {
				*dest++ = *src++;
			} else {
				uint16 offset = READ_LE_UINT16(src);
				src += 2;
				int count = (offset >> 12) + 2;
				offset &= 0xfff;

				Common::copy(dest - offset, dest - offset + count, dest);
				dest += count;
			}
		}
	}
}

void rle_decompress(const byte *src, byte *dest, size_t destSize) {
	byte *endP = dest + destSize;

	for (;;) {
		byte val = *src++;

		if ((val & 0x80) != 0) {
			byte rep = *src++;
			val &= 0x7f;
			Common::fill(dest, dest + val, rep);
			dest += val;
		} else if (val > 0) {
			Common::copy(src, src + val, dest);
			src += val;
			dest += val;
		} else {
			break;
		}
	}

	assert(dest == endP);
}

} // namespace Got
