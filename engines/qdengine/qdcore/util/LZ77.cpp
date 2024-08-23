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

// Implementation of pure LZ77 looseless compression.
// Modification of this algorithm was used for NTFS compression.
//
// Implemented by Arkadi Kagan.
//
#include "common/endian.h"
#include "common/util.h"

#include "qdengine/qdcore/util/LZ77.h"

namespace QDEngine {

#define BITS_LEN    4

CLZ77::CLZ77() {
}
CLZ77::~CLZ77() {
}

int32 CLZ77::lzComp(const byte *s1, const byte *s2, int32 maxlen) {
	int32 i;
	for (i = 0; i < maxlen; i++)
		if (s1[i] != s2[i])
			return i;
	return maxlen;
}
const byte *CLZ77::findLZ(const byte *source, const byte *s, int32 slen, int32 border, int32 mlen, int32 &len) {
	int32 maxlen = 0;
	int32 limit = slen - (s - source);
	const byte *maxp = s - 1;
	const byte *p;
	len = 0;
	for (p = s - 1; p >= source; p--) {
		len = lzComp(p, s, limit);
		if (len > maxlen) {
			maxp = p;
			maxlen = len;
		}
		if (s - p >= border - 1) break;
		if (len >= mlen - 1) break;
	}
	len = MIN(maxlen, mlen - 1);
	return maxp;
}

int32 CLZ77::getMaxEncoded(int32 len) {
	return len + sizeof(uint32);
}
int32 CLZ77::getMaxDecoded(byte *source) {
	return ((uint32 *)source)[0];
}
void CLZ77::encode(byte *target, int32 &tlen, const byte *source, int32 slen) {
	int32 len, block;
	int32 shift, border;
	const byte *s, *p;
	byte *t;
	byte *flag;
	uint16 *ptmp;

	((uint32 *)target)[0] = slen;    // save source size
	target += sizeof(uint32);
	tlen = sizeof(uint32);

	block = 0;              // block - bit in single flag byte
	shift = 16;             // shift offset to most significant bits
	border = 1;             // offset can`t be more then border
	flag = target;          // flag for every 8 entities
	tlen++;                 // byte for first flag
	*flag = 0;
	s = (const byte *)source;
	t = target + 1;
	for (s = (const byte *)source; s - source < slen;) {
		if (shift > BITS_LEN)
			while (s - source >= border) {
				if (shift <= BITS_LEN) break;
				border = border << 1;
				shift--;
			}
		p = findLZ((const byte *)source, s, slen, border, (1 << shift), len);
		if (len <= 2) len = 1;
		if (len <= 1) {
			*t++ = *s++;
			tlen++;
		} else {
			ptmp = (uint16 *)t;
			*ptmp = (uint16)(((s - p - 1) << shift) + len);

			*flag |= 1 << block;
			t += 2;
			tlen += 2;
			s += len;
		}
		if (++block >= 8) {
			flag = t++;
			*flag = 0;
			block = 0;
			tlen++;
		}
		/*      if (tlen >= slen)
		        {
		            tlen = 0;
		            return;
		        }*/
//		OnStep();
	}
}
int32 CLZ77::decode(byte *target, int32 &tlen, const byte *source, int32 slen) {
	uint32 i;
	uint32 block, len;
	uint32 shift, border;
	const byte *s;
	byte *t, *p;
	const byte *flag;
	uint16 tmp;

	tlen = READ_LE_UINT32(source);
	source += sizeof(uint32);            // read/remove target size
	slen -= sizeof(uint32);

	t = target;
	flag = (const byte *)source;
	block = 0;              // block - bit in single flag byte
	shift = 16;             // shift offset to most significant bits
	border = 1;             // offset can`t be more then border
	for (s = (const byte *)source + 1; (s < source + slen) && (t - target < tlen);) {
		if (shift > BITS_LEN)
			while (t - target >= (int)border) {
				if (shift <= BITS_LEN) break;
				border = border << 1;
				shift--;
			}
		if (flag[0] & (1 << block)) {
			tmp = READ_LE_UINT16(s);
			len = ((1 << shift) - 1) & tmp;
			p = t - (tmp >> shift) - 1;
			for (i = 0; i < len; i++)
				t[i] = p[i];
			t += len;
			s += 2;
		} else {
			*t++ = *s++;
			len = 1;
		}
		if (++block >= 8) {
			flag = s++;
			block = 0;
		}
//		OnStep();
	}
	return (s - source) + sizeof(uint32);
}

} // namespace QDEngine
