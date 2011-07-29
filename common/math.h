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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on eos' math code

#ifndef COMMON_MATH_H
#define COMMON_MATH_H

#include "common/scummsys.h"

#ifndef M_SQRT1_2
	#define M_SQRT1_2 0.70710678118654752440 /* 1/sqrt(2) */
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#ifndef FLT_MIN
	#define FLT_MIN 1E-37
#endif

#ifndef FLT_MAX
	#define FLT_MAX 1E+37
#endif

namespace Common {

/** A complex number. */
struct Complex {
	float re, im;
};

#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
inline int intLog2(uint32 v) {
	// This is a slightly optimized implementation of log2 for natural numbers
	// targeting gcc. It also saves some binary size over our fallback
	// implementation, since it does not need any table.
	if (v == 0)
		return -1;
	else
		// This is really "sizeof(unsigned int) * CHAR_BIT - 1" but using 8
		// instead of CHAR_BIT is sane enough and it saves us from including
		// limits.h
		return (sizeof(unsigned int) * 8 - 1) - __builtin_clz(v);
}
#else
// See http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
static const char LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
	-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
	LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};

inline int intLog2(uint32 v) {
	register uint32 t, tt;

	if ((tt = v >> 16))
		return (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		return (t =  v >> 8) ?  8 + LogTable256[t] : LogTable256[v];
}
#endif

inline float rad2deg(float rad) {
	return rad * 180.0 / M_PI;
}

inline float deg2rad(float deg) {
	return deg * M_PI / 180.0;
}

} // End of namespace Common

#endif // COMMON_MATH_H
