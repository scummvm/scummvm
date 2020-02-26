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

#include "bladerunner/decompress_lzo.h"

namespace BladeRunner {

static inline uint32 decode_count(const uint8 **pp) {
	uint32 v = 0;
	for (; !**pp; ++(*pp))
		v += 255;

	v += **pp;
	++(*pp);

	return v;
}

static inline void copy(uint8 **dst, const uint8 **src, int count) {
	assert(count > 0);

	uint8 *d = *dst;
	const uint8 *s = *src;

	*dst += count;
	*src += count;

	do { *d++ = *s++; } while (--count);
}

int decompress_lzo1x(const uint8 *in, size_t inLen, uint8 *out, size_t *outLen) {
	uint32 t;
	uint8 *op;
	const uint8 *ip, *m_pos;
	const uint8 * const ip_end = in + inLen;

	*outLen = 0;

	op = out;
	ip = in;

	if (*ip > 17) {
		t = *ip++ - 17;
		if (t < 4)
			goto match_next;
		copy(&op, &ip, t);
		goto first_literal_run;
	}

	for (;;) {
		t = *ip++;
		if (t >= 16)
			goto match;

		if (t == 0)
			t = 15 + decode_count(&ip);
		copy(&op, &ip, t + 3);

first_literal_run:
		t = *ip++;
		if (t >= 16)
			goto match;
		m_pos = op - 0x0801 - (t >> 2) - (*ip++ << 2);
		copy(&op, &m_pos, 3);
		goto match_done;

		for (;;) {
match:
			if (t >= 64) {
				m_pos = op - 1 - ((t >> 2) & 7) - (*ip++ << 3);
				t = (t >> 5) - 1;
				goto copy_match;
			} else if (t >= 32) {
				t &= 31;
				if (t == 0)
					t = 31 + decode_count(&ip);
				m_pos = op - 1 - (ip[0] >> 2) - (ip[1] << 6);
				ip += 2;
			} else if (t >= 16) {
				m_pos = op - ((t & 8) << 11);
				t &= 7;
				if (t == 0)
					t = 7 + decode_count(&ip);
				m_pos -= (ip[0] >> 2) + (ip[1] << 6);
				ip += 2;
				if (m_pos == op)
					goto eof_found;
				m_pos -= 0x4000;
			} else {
				m_pos = op - 1 - (t >> 2) - (*ip++ << 2);
				copy(&op, &m_pos, 2);
				goto match_done;
			}

copy_match:
			copy(&op, &m_pos, t + 2);

match_done:
			t = ip[-2] & 3;
			if (t == 0)
				break;

match_next:
 			assert(t > 0 && t <= 3);
			copy(&op, &ip, t);
			t = *ip++;
		}
	}

eof_found:
	*outLen = op - out;
	return ip != ip_end;
}

} // End of namespace BladeRunner
