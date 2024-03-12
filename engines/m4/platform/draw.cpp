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

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "m4/platform/draw.h"

namespace M4 {

void RLE8Decode(const uint8 *inBuff, uint8 *outBuff, uint32 pitch) {
	byte val, count;
	int line = 0, numY = 0;
	byte *destP = outBuff;

	for (;;) {
		count = *inBuff++;

		if (count) {
			// Basic run length
			val = *inBuff++;
			Common::fill(destP, destP + count, val);
			destP += count;

		} else {
			count = *inBuff++;

			if (count >= 3) {
				// Block of uncompressed pixels to copy
				Common::copy(inBuff, inBuff + count, destP);
				inBuff += count;
				destP += count;

			} else if (!(count & 3)) {
				// End of Line code
				++line;
				destP = outBuff + line * pitch;

			} else if (!(count & 2)) {
				break;

			} else {
				// Move down by X, Y amount
				destP += *inBuff++;		// x amount
				numY = *inBuff++;		// y amount
				line += numY;
				destP += numY * pitch;
			}
		}
	}
}

uint8 *SkipRLE_Lines(uint32 linesToSkip, uint8 *rleData) {
	while (linesToSkip > 0) {
		if (*rleData) {
			// Simple RLE sequence, so skip over count and value
			rleData += 2;

		} else if (rleData[1] >= 3) {
			rleData += 2 + rleData[2];
		} else {
			rleData += 2;
			--linesToSkip;
		}
	}

	return rleData;
}

size_t RLE8Decode_Size(byte *src, int pitch) {
	size_t total = 0, line = 0, y;
	byte count;

	for (;;) {
		count = *src++;

		if (count) {
			total += count;
			++src;
		} else {
			count = *src++;

			if (count >= 3) {
				// Block of uncompressed pixels to copy
				total += count;
				src += count;

			} else if (!(count & 3)) {
				// End of Line code
				++line;
				total = line * pitch;

			} else if (!(count & 2)) {
				break;

			} else {
				// Move down by X, Y amount
				total += *src++;	// x amount
				y = *src++;			// y amount
				line += y;
				total += y * pitch;
			}
		}
	}

	return total;
}

void RLE_Draw(Buffer *src, Buffer *dest, int32 x, int32 y) {
	const byte *srcP = src->data;
	byte *destData = dest->data + y * dest->w + x;
	byte *destP = destData;
	int destWidth = dest->w;
	byte count, val;
	int line = 0;

	assert(x >= 0 && y >= 0 && x < dest->w && y < dest->h);

	for (;;) {
		count = *srcP++;

		if (count) {
			// Basic run length
			val = *srcP++;

			// 0 pixels are transparent, and are skipped. Otherwise, draw pixels
			if (val != 0)
				Common::fill(destP, destP + count, val);
			destP += count;

		} else {
			count = *srcP++;

			if (count >= 3) {
				// Block of uncompressed pixels to copy
				for (; count > 0; --count, ++destP) {
					val = *srcP++;
					if (val != 0)
						*destP = val;
				}

			} else if (!(count & 3)) {
				// End of line code
				++line;
				destP = destData + line * destWidth;

			} else {
				// Stop drawing image. Seems weird that it doesn't handle the X/Y offset
				// form for count & 2, but the original explicitly doesn't implement it
				break;
			}
		}
	}

	assert(destP <= (dest->data + dest->h * dest->stride));
}

} // namespace M4
