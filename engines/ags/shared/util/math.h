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

//=============================================================================
//
// Helper math functions
//
//=============================================================================

#ifndef AGS_SHARED_UTIL_MATH_H
#define AGS_SHARED_UTIL_MATH_H

#include "ags/lib/std/limits.h"

namespace AGS3 {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace AGS {
namespace Shared {

namespace Math {
template <class T>
inline const T &Max(const T &a, const T &b) {
	return a < b ? b : a;
}

template <class T>
inline const T &Min(const T &a, const T &b) {
	return a < b ? a : b;
}

template <class T>
inline const T &Clamp(const T &val, const T &floor, const T &ceil) {
	return Max<T>(floor, Min<T>(val, ceil));
}

template <class T>
inline void ClampLength(T &from, T &length, const T &floor, const T &height) {
	if (from < floor) {
		length -= floor - from;
		from = floor;
	} else if (from >= floor + height) {
		from = floor + height;
		length = 0;
	}

	length = Max<T>(length, 0);
	length = Min<T>(length, height - from);
}

// Get a measure of how value A is greater than value B;
// if A is smaller than or equal to B, returns 0.
template <class T>
inline T Surplus(const T &larger, const T &smaller) {
	return larger > smaller ? larger - smaller : 0;
}

// Tests if the big-type value is in range of the result type;
// returns same value converted if it's in range, or provided replacement if it's not.
template <typename T, typename TBig>
inline T InRangeOrDef(const TBig &val, const T &def) {
	return (val >= std::numeric_limits<T>::min() && val <= std::numeric_limits<T>::max()) ? val : def;
}

inline float RadiansToDegrees(float rads) {
	return rads * (float)(180.0 / M_PI);
}

inline float DegreesToRadians(float deg) {
	return deg * (float)(M_PI / 180.0);
}

} // namespace Math

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
