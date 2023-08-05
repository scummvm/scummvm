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

bool imath_rand_bool(int max) {
	return imath_ranged_rand(1, max) == 1;
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

#define DIV_128_PI	0x28be61

frac16 ArcTan(frac16 x, frac16 y) {
	double	floatX, floatY, result;
	frac16	fracResult;

	floatX = (float)(x >> 16) + (float)((float)(x & 0xffff) / (float)65536);
	floatY = (float)(y >> 16) + (float)((float)(y & 0xffff) / (float)65536);
	result = atan2(floatY, floatX);
	fracResult = (((int32)(floor(result))) << 16) + (int32)(floor((result - floor(result)) * 65536));
	fracResult = MulSF16(fracResult, DIV_128_PI);
	if (fracResult < 0) fracResult += 0x1000000;
	return fracResult;
}

uint16 HighWord(uint32 n) {
	return (uint16)(n >> 16);
}

uint16 LowWord(uint32 n) {
	return (uint16)(n & 0xffff);
}

uint32 convert_intel32(uint32 a) {
	return FROM_LE_32(a);
}

uint16 convert_intel16(uint16 a) {
	return FROM_LE_16(a);
}

frac16 FixedMul(frac16 a, frac16 b) {
	float ta = a;
	float tb = b;
	return (frac16)((ta * tb) / 65536.0);
}

frac16 FixedDiv(frac16 a, frac16 b) {
	float ta = a;
	float tb = b;
	return (frac16)((ta / tb) * 65536.0);
}

frac16 FixedMulDiv(frac16 a, frac16 b, frac16 c) {
	float ta = a;
	float tb = b;
	float tc = c;
	return (frac16)((ta * tb) / tc);
}

} // namespace M4
