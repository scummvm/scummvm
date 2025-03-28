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
#include "common/textconsole.h"
#include "snatcher/util.h"

namespace Snatcher {
namespace Util {

uint32 decodeSCDData(const uint8 *src, uint8 *dst) {
	const uint8 *dstStart = dst;

	uint8 bits = 0;
	uint8 code = 0;
	uint8 op = 0;
	uint8 len = 0;
	uint16 offs = 0;

	for (bool loop = true; loop; ) {
		if (!bits--) {
			code = *src++;
			bits = 7;
		}

		op = code & 1; 
		code >>= 1;

		if (op == 0) {
			*dst++ = *src++;
			continue;
		}

		op = *src++;

		if (op == 0x1F) {
			loop = false;
			break;
		} else if (op & 0x80) {
			if (op & 0x40) {
				op -= 184;
				while (op--)
					*dst++ = *src++;
				continue;
			} else {
				len = (op >> 4) - 6;
				offs = op & 0x0F;
			}
		} else {
			len = (op & 0x1F) + 3;
			offs = ((op & 0xE0) << 3) | (*src++);
		}

		const uint8 *s2 = dst - offs;
		assert(s2 >= dstStart);
		while (len--)
			*dst++ = *s2++;
	}

	return (dst - dstStart);
}

uint8 toDecimal(uint8 v) {
	return (v / 10) << 4 | (v % 10);
}

uint32 makeBCDTimeStamp(uint32 msecs) {
	uint32 min = toDecimal(msecs / 60000);
	uint32 sec = toDecimal((msecs / 1000) % 60);
	uint32 frm = toDecimal((msecs % 1000) * 1000000 / 13333333);
	// BCD time code mm:ss:ff:md (md = mode: 00 = CD-DA, 01 = CD-ROM mode 1, 02 = CD-ROM mode 2; irrelevant to us)
	return ((min & 0xFF) << 24) | ((sec & 0xFF) << 16) | ((frm & 0xFF) << 8);
}

} // End of namespace Util
} // End of namespace Snatcher
