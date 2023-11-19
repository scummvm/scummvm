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

#include "common/debug.h"

#include "toon/tools.h"
#include "toon/toon.h"

namespace Toon {

uint32 decompressLZSS(byte *src, byte *dst, int dstsize) {
	debugC(5, kDebugTools, "decompressLZSS(src, dst, %d)", dstsize);

	byte *srcp = src;
	byte *dstp = dst;
	uint16 bitbuf;
	int32 len, ofs;
	len = 0;
	while (dstsize > 0) {
		bitbuf = 0x100 | *(srcp++);
		while (bitbuf != 1 && dstsize > 0) {
			if (bitbuf & 1) {
				ofs = READ_LE_UINT16(srcp);
				srcp += 2;
				len = ((ofs & 0xF000) >> 12) + 3;
				ofs = ofs | 0xF000;
				dstsize -= len;
				if (dstsize < 0)
					break;
				while (len--) {
					*dstp = *(byte *)(dstp + (signed short)ofs);
					dstp++;
				}
			} else {
				len = 0;
				while ((bitbuf & 2) == 0) {
					len++;
					bitbuf >>= 1;
				}
				len++;
				dstsize -= len;
				if (dstsize < 0)
					break;
				while (len--)
					*(dstp++) = *(srcp++);
			}
			bitbuf >>= 1;
		}
	}

	if (len == -1 && dstsize == 0) {
		return (dstp - dst);
	}

	len += dstsize;
	if (len < 0)
		return 0;

	while (len--)
		*(dstp++) = *(srcp++);

	return (dstp - dst);
}

uint32 decompressSPCN(byte *src, byte *dst, uint32 dstsize) {
	debugC(1, kDebugTools, "decompressSPCN(src, dst, %d)", dstsize);

	byte *srcp = src;
	byte *dstp = dst, *dste = dst + dstsize;
	byte val;
	uint16 len, ofs;
	if (!(*srcp & 0x80)) srcp++;
	while (dstp < dste) {
		val = *(srcp++);
		if (val & 0x80) {
			if (val & 0x40) {
				if (val == 0xFE) {
					len = READ_LE_UINT16(srcp);
					while (len--)
						*(dstp++) = srcp[2];
					srcp += 3;
				} else {
					if (val == 0xFF) {
						len = READ_LE_UINT16(srcp);
						srcp += 2;
					} else {
						len = (val & 0x3F) + 3;
					}
					ofs = READ_LE_UINT16(srcp);
					srcp += 2;
					while (len--) {
						*dstp = *(byte *)(dstp - ofs);
						dstp++;
					}
				}
			} else {
				len = val & 0x3F;
				while (len--)
					*(dstp++) = *(srcp++);
			}
		} else {
			len = (val >> 4) + 3;
			ofs = ((val & 0x0F) << 8) | *(srcp++);
			while (len--) {
				*dstp = *(byte *)(dstp - ofs);
				dstp++;
			}
		}
	}
	return (dstp - dst);
}

} // End of namespace Toon
