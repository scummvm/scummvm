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
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/util/LZ77.h"

namespace QDEngine {

#define BITS_LEN    4

CLZ77::CLZ77() {
}
CLZ77::~CLZ77() {
}

long CLZ77::LZComp(unsigned char *s1, unsigned char *s2, long maxlen) {
	long i;
	for (i = 0; i < maxlen; i++)
		if (s1[i] != s2[i])
			return i;
	return maxlen;
}
unsigned char *CLZ77::FindLZ(unsigned char *source, unsigned char *s, long slen, long border, long mlen, long &len) {
	long maxlen = 0;
	long limit = slen - (s - source);
	unsigned char *maxp = s - 1;
	unsigned char *p;
	len = 0;
	for (p = s - 1; p >= source; p--) {
		len = LZComp(p, s, limit);
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

long CLZ77::GetMaxEncoded(long len) {
	return len + sizeof(unsigned long);
}
long CLZ77::GetMaxDecoded(unsigned char *source) {
	return ((unsigned long *)source)[0];
}
void CLZ77::Encode(unsigned char *target, long &tlen, const unsigned char *source, long slen) {
	long len, block;
	long shift, border;
	unsigned char *s, *t, *p;
	unsigned char *flag;
	unsigned short *ptmp;

	((unsigned long *)target)[0] = slen;    // save source size
	target += sizeof(unsigned long);
	tlen = sizeof(unsigned long);

	block = 0;              // block - bit in single flag byte
	shift = 16;             // shift offset to most significant bits
	border = 1;             // offset can`t be more then border
	flag = target;          // flag for every 8 entities
	tlen++;                 // byte for first flag
	*flag = 0;
	s = (unsigned char *)source;
	t = target + 1;
	for (s = (unsigned char *)source; s - source < slen;) {
		if (shift > BITS_LEN)
			while (s - source >= border) {
				if (shift <= BITS_LEN) break;
				border = border << 1;
				shift--;
			}
		p = FindLZ((unsigned char *)source, s, slen, border, (1 << shift), len);
		if (len <= 2) len = 1;
		if (len <= 1) {
			*t++ = *s++;
			tlen++;
		} else {
			ptmp = (unsigned short *)t;
			*ptmp = (unsigned short)(((s - p - 1) << shift) + len);

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
long CLZ77::Decode(unsigned char *target, long &tlen, const unsigned char *source, long slen) {
	long i;
	long block, len;
	long shift, border;
	unsigned char *s, *t, *p;
	unsigned char *flag;
	unsigned short *ptmp;

	tlen = ((unsigned long *)source)[0];
	source += sizeof(unsigned long);            // read/remove target size
	slen -= sizeof(unsigned long);

	t = target;
	flag = (unsigned char *)source;
	block = 0;              // block - bit in single flag byte
	shift = 16;             // shift offset to most significant bits
	border = 1;             // offset can`t be more then border
	for (s = (unsigned char *)source + 1; (s < source + slen) && (t - target < tlen);) {
		if (shift > BITS_LEN)
			while (t - target >= border) {
				if (shift <= BITS_LEN) break;
				border = border << 1;
				shift--;
			}
		if (flag[0] & (1 << block)) {
			ptmp = (unsigned short *)s;
			len = ((1 << shift) - 1)&ptmp[0];
			p = t - (ptmp[0] >> shift) - 1;
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
	return (s - source) + sizeof(unsigned long);
}

} // namespace QDEngine
