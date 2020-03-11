/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "bladerunner/decompress_lcw.h"

#include "common/util.h"

namespace BladeRunner {

uint32 decompress_lcw(uint8 *inBuf, uint32 inLen, uint8 *outBuf, uint32 outLen) {
	int version = 1;
	int count, i, color, pos, relpos;

	uint8 *src = inBuf;
	uint8 *dst = outBuf;
	uint8 *outEnd = dst + outLen;

	if (src[0] == 0) {
		version = 2;
		++src;
	}

	while (src < inBuf + inLen && dst < outEnd && src[0] != 0x80) {
		int out_remain = (int)(outEnd - dst);

		if (src[0] == 0xff) {     // 0b11111111
			count = src[1] | (src[2] << 8);
			pos   = src[3] | (src[4] << 8);
			src += 5;
			count = MIN(count, out_remain);

			if (version == 1) {
				for (i = 0; i < count; ++i)
					dst[i] = outBuf[i + pos];
			} else {
				for (i = 0; i < count; ++i)
					dst[i] = *(dst + i - pos);
			}
		} else if (src[0] == 0xfe) { // 0b11111110
			count = src[1] | (src[2] << 8);
			color = src[3];
			src += 4;
			count = MIN(count, out_remain);

			memset(dst, color, count);
		} else if (src[0] >= 0xc0) { // 0b11??????
			count = (src[0] & 0x3f) + 3;
			pos   = src[1] | (src[2] << 8);
			src += 3;
			count = MIN(count, out_remain);

			if (version == 1) {
				for (i = 0; i < count; ++i)
					dst[i] = outBuf[i + pos];
			} else {
				for (i = 0; i < count; ++i)
					dst[i] = *(dst + i - pos);
			}
		} else if (src[0] >= 0x80) { // 0b10??????
			count = src[0] & 0x3f;
			++src;
			count = MIN(count, out_remain);

			memcpy(dst, src, count);
			src += count;
		} else {                    // 0b0???????
			count  = ((src[0] & 0x70) >> 4) + 3;
			relpos = ((src[0] & 0x0f) << 8) | src[1];
			src += 2;
			count = MIN(count, out_remain);

			for (i = 0; i < count; ++i) {
				dst[i] = *(dst + i - relpos);
			}
		}

		dst += count;
	}

	return uint32(dst - outBuf);
}

uint32 decompress_lcw_output_size(uint8 *inBuf, uint32 inLen) {
	int count;
	uint8 *src     = inBuf;
	uint32 outsize = 0;

	if (src[0] == 0)
		++src;

	while (src[0] != 0x80 && src < inBuf + inLen) {
		if (src[0] == 0xff) {        // 0b11111111
			count = src[1] | (src[2] << 8);
			src += 5;
		} else if (src[0] == 0xfe) { // 0b11111110
			count = src[1] | (src[2] << 8);
			src += 4;
		} else if (src[0] >= 0xc0) { // 0b11??????
			count = (src[0] & 0x3f) + 3;
			src += 3;
		} else if (src[0] & 0x80) {  // 0b10??????
			count = src[0] & 0x3f;
			src += count + 1;
		} else {                    // 0b0???????
			count  = ((src[0] & 0x70) >> 4) + 3;
			src += 2;
		}

		outsize += count;
	}

	return outsize;
}

} // End of namespace BladeRunner
