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

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "common/scummsys.h"

#ifndef FLT_MIN
	#define FLT_MIN 1E-37f
#endif

#ifndef FLT_MAX
	#define FLT_MAX 1E+37f
#endif

namespace Math {

/** A complex number. */
struct Complex {
	float re, im;
};

/* Math::epsilon is a constant with a small value which is used for comparing
 * floating point numbers.
 *
 * The value is based on the previous hard-coded numbers in
 * Line2d.cpp. Smaller numbers could be used unless they are
 * smaller than the float granularity.
 */
static const float epsilon = 0.0001f;

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

inline float square(float x) {
	return x * x;
}

}

#endif
