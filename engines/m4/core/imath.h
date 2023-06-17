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

#ifndef M4_CORE_IMATH_H
#define M4_CORE_IMATH_H

#include "m4/m4_types.h"

namespace M4 {

#define FRAC16(a,b) ((a<<16) + (b))

#define Atan2F16(x1, x2) ArcTan(x1, x2)
#define MulSF16(x1, x2) FixedMul(x1, x2)
#define DivSF16(x1, x2) FixedDiv(x1, x2)
#define RecipUF16(x) FixedDiv(1, x)
#define SquareSF16(x) FixedMul(x, x)

frac16 FixedMul(frac16 Multiplicand, frac16 Multiplier);
frac16 FixedDiv(frac16 Dividend, frac16 Divisor);

unsigned long sqrtul(unsigned long v);

int32 imath_max(int32 a, int32 b);
int32 imath_min(int32 a, int32 b);
int32 imath_abs(int32 a);

void imath_seed(int32 seednum);
uint32 imath_random();
int32 imath_ranged_rand(int32 a, int32 b);
frac16 imath_ranged_rand16(frac16 a, frac16 b);

frac16 dist2d(int32 x1, int32 y1, int32 x2, int32 y2);
frac16 SqrtF16(frac16 n);
//frac16 ArcTan(frac16 x, frac16 y);

uint16 HighWord(uint32 n);
uint16 LowWord(uint32 n);

uint32 convert_intel32(uint32 a);
uint16 convert_intel16(uint16 a);

//
//	I N L I N E S
//

inline static short abs(short a) {
	return (a >= 0) ? a : -a;
}

inline static long abs(long a) {
	return (a >= 0) ? a : -a;
}

} // namespace M4

#endif
