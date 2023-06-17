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

#include "m4/core/imath.h"

namespace M4 {

unsigned long sqrtul(unsigned long v) {
	unsigned long r = 0, s;

#define STEP(k) s = r + (1L << k * 2); r >>= 1; \
		if (s <= v) { v -= s; r |= (1L << k * 2); }

	STEP(15);
	STEP(14);
	STEP(13);
	STEP(12);
	STEP(11);
	STEP(10);
	STEP(9);
	STEP(8);
	STEP(7);
	STEP(6);
	STEP(5);
	STEP(4);
	STEP(3);
	STEP(2);
	STEP(1);
	STEP(0);

	return r;
#undef STEP
}

int32 imath_min(int32 a, int32 b) {
	return ((a < b) ? a : b);
}

int32 imath_max(int32 a, int32 b) {
	return ((a > b) ? a : b);
}

int32 imath_abs(int32 a) {
	return ((a >= 0) ? a : -a);
}

static int32 seed;

void imath_seed(int32 seednum) {
	seed = seednum;
}

uint32 imath_random() {
	return(seed = (25173 * seed + 13849) & 0xffff);
}

int32 imath_ranged_rand(int32 a, int32 b) {
	int32 result;
	result = (a + (((1 + imath_abs(b - a)) * imath_random()) >> 16));
	return result;
}

frac16 imath_ranged_rand16(frac16 a, frac16 b) {
	frac16 result;
	result = ((a + MulSF16(1 + imath_abs(b - a), imath_random())));
	return result;
}

frac16 dist2d(int32 x1, int32 y1, int32 x2, int32 y2) {
	if ((x2 -= x1) < 0) x2 = -x2;
	if ((y2 -= y1) < 0) y2 = -y2;
	return (x2 + y2 - (((x2 > y2) ? y2 : x2) >> 1));
}

#define STEP(k) s = r + (1L << k * 2); r >>= 1; \
				if (s <= v) { v -= s; r |= (1L << k * 2); }

frac16 SqrtF16(frac16 n) {
	ulong r = 0, s, v;
	v = (ulong)n;

	STEP(15);
	STEP(14);
	STEP(13);
	STEP(12);
	STEP(11);
	STEP(10);
	STEP(9);
	STEP(8);
	STEP(7);
	STEP(6);
	STEP(5);
	STEP(4);
	STEP(3);
	STEP(2);
	STEP(1);
	STEP(0);
	return (frac16)r;
}

} // namespace M4
