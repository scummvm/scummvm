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

#include "scumm/bomp.h"

namespace Scumm {

void smushDecodeRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch) {
	dst += top * pitch;
	do {
		dst += left;
		bompDecodeLine(dst, src + 2, width, false);
		src += READ_LE_UINT16(src) + 2;
		dst += pitch - left;
	} while (--height);
}


/**
 * RA1 codec 1: RLE with transparency on pixel 0.
 * Same BOMP encoding as smushDecodeRLE but pixel value 0 is not written,
 * allowing the background (restored via FTCH) to show through.
 * Matches FFmpeg's old_codec1() with opaque=0.
 */
void smushDecodeRA1Transparent(byte *dst, const byte *src, int left, int top, int width, int height, int pitch) {
	dst += top * pitch;
	do {
		byte *rowDst = dst + left;
		const byte *lineData = src + 2;
		int remaining = width;

		while (remaining > 0) {
			byte code = *lineData++;
			byte num = (code >> 1) + 1;
			if (num > remaining)
				num = remaining;
			if (code & 1) {
				// Fill: repeat single byte
				byte color = *lineData++;
				if (color != 0) {
					memset(rowDst, color, num);
				}
				// If color == 0: skip (transparent)
			} else {
				// Copy: write each byte, skipping 0
				for (int j = 0; j < num; j++) {
					byte c = lineData[j];
					if (c != 0)
						rowDst[j] = c;
				}
				lineData += num;
			}
			rowDst += num;
			remaining -= num;
		}

		src += READ_LE_UINT16(src) + 2;
		dst += pitch;
	} while (--height);
}

} // End of namespace Scumm
