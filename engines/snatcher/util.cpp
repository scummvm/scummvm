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

uint8 toBCD(uint8 v) {
	return (v / 10) << 4 | (v % 10);
}

uint32 makeBCDTimeStamp(uint32 msecs, BCDResolution res) {
	uint32 hrs = toBCD((msecs / 3600000) % 24);
	uint32 min = toBCD((msecs / 60000) % 60);
	uint32 sec = toBCD((msecs / 1000) % 60);
	uint32 frm = toBCD((msecs % 1000) * 1000000 / 13333333);

	if (res == kBCD_HHMMSS) // BCD time code hh:mm:ss
		return ((hrs & 0xFF) << 24) | ((min & 0xFF) << 16) | ((sec & 0xFF) << 8);

	// BCD time code mm:ss:ff:md (md = mode: 00 = CD-DA, 01 = CD-ROM mode 1, 02 = CD-ROM mode 2; irrelevant to us)
	return ((min & 0xFF) << 24) | ((sec & 0xFF) << 16) | ((frm & 0xFF) << 8);
}

union RNDC {
	RNDC() : b(0) {}
	uint8 a[2];
	uint16 b;
};

RNDC rndC1;
RNDC rndC2;

uint8 rndH = 0;
uint8 rndL = 1;

void rngReset() {
	rndC1.b = 0;
	rndC2.b = 0;
	rndH = FROM_BE_16(0x100) & 0xFF;
	rndL = rndH ^ 1;
}

uint16 rngMakeNumber() {
	rndC1.a[rndH] += rndC1.a[rndL];
	rndC1.b += rndC2.b;
	rndC1.b += SWAP_BYTES_16(rndC2.b);
	if (rndC2.a[rndH] + rndC2.a[rndL] > 0xFF)
		rndC1.b++;
	rndC2.a[rndH] += rndC2.a[rndL];
	if (rndC2.b + 1 > 0xFFFF)
		rndC1.b++;
	rndC2.b++;
	return rndC1.b;
}

uint16 rngGetNumberFromRange(uint16 min, uint16 range) {
	return ((range * rngMakeNumber()) >> 16) + min;
}

} // End of namespace Util
} // End of namespace Snatcher
