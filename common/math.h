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

// Based on eos' math code

#ifndef COMMON_MATH_H
#define COMMON_MATH_H

#include "common/scummsys.h"
#ifdef _MSC_VER
// HACK:
// intrin.h on MSVC includes setjmp.h, which will fail compiling due to our
// forbidden symbol colde. Since we also can not assure that defining
// FORBIDDEN_SYMBOL_EXCEPTION_setjmp and FORBIDDEN_SYMBOL_EXCEPTION_longjmp
// will actually allow the symbols, since forbidden.h might be included
// earlier already we need to undefine them here...
#undef setjmp
#undef longjmp
#include <intrin.h>
// ...and redefine them here so no code can actually use it.
// This could be resolved by including intrin.h on MSVC in scummsys.h before
// the forbidden.h include. This might make sense, in case we use MSVC
// extensions like _BitScanReverse in more places. But for now this hack should
// be ok...
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#undef setjmp
#define setjmp(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#undef longjmp
#define longjmp(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif
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

#if GCC_ATLEAST(3, 4)
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
#elif defined(_MSC_VER)
inline int intLog2(uint32 v) {
	unsigned long result = 0;
	unsigned char nonZero = _BitScanReverse(&result, v);
	// _BitScanReverse stores the position of the MSB set in case its result
	// is non zero, thus we can just return it as is.
	return nonZero ? result : -1;
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
	uint32 t, tt;

	if ((tt = v >> 16))
		return (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		return (t =  v >> 8) ?  8 + LogTable256[t] : LogTable256[v];
}
#endif

// Round a number towards zero
// Input and Output type can be different
template<class InputT, class OutputT>
inline OutputT trunc(InputT x) {
    return (x > 0) ? floor(x) : ceil(x);
}

// Round a number towards zero
// Input and Output type are the same
template<class T>
inline T trunc(T x) {
	return trunc<T,T>(x);
}

// Convert radians to degrees
// Input and Output type can be different
// Upconvert everything to floats
template<class InputT, class OutputT>
inline OutputT rad2deg(InputT rad) {
	return (OutputT)( (float)rad * (float)57.2957795130823); // 180.0/M_PI = 57.2957795130823
}

// Handle the case differently when the input type is double
template<class OutputT>
inline OutputT rad2deg(double rad) {
	return (OutputT)( rad * 57.2957795130823);
}

// Convert radians to degrees
// Input and Output type are the same
template<class T>
inline T rad2deg(T rad) {
	return rad2deg<T,T>(rad);
}

// Convert degrees to radians
// Input and Output type can be different
// Upconvert everything to floats
template<class InputT, class OutputT>
inline OutputT deg2rad(InputT deg) {
	return (OutputT)( (float)deg * (float)0.0174532925199433); // M_PI/180.0 = 0.0174532925199433
}

// Handle the case differently when the input type is double
template<class OutputT>
inline OutputT deg2rad(double deg) {
	return (OutputT)( deg * 0.0174532925199433);
}

// Convert degrees to radians
// Input and Output type are the same
template<class T>
inline T deg2rad(T deg) {
	return deg2rad<T,T>(deg);
}

template<class T>
inline T hypotenuse(T xv, T yv) {
	return (T)sqrt((double)(xv * xv + yv * yv));
}

} // End of namespace Common

#endif // COMMON_MATH_H
