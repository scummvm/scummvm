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


#include "common/endian.h"

namespace Scumm {

void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch) {
	byte val, code;
	int32 length;
	int h = height, lineSize;

	dst += top * pitch;
	for (h = 0; h < height; h++) {
		lineSize = READ_LE_UINT16(src);
		src += 2;
		dst += left;
		while (lineSize > 0) {
			code = *src++;
			lineSize--;
			length = (code >> 1) + 1;
			if (code & 1) {
				val = *src++;
				lineSize--;
				if (val)
					memset(dst, val, length);
				dst += length;
			} else {
				lineSize -= length;
				while (length--) {
					val = *src++;
					if (val)
						*dst = val;
					dst++;
				}
			}
		}
		dst += pitch - left - width;
	}
}

} // End of namespace Scumm
