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

#ifndef QDENGINE_XLIBS_UTIL_XMATH_SAFEMATH_H
#define QDENGINE_XLIBS_UTIL_XMATH_SAFEMATH_H

#include "qdengine/util/xmath/fastMath.h"


namespace QDEngine {

inline float cycle(float f, float size) {
	return fmodFast(fmodFast(f, size) + size, size);
}

inline float getDist(float v0, float v1, float size) {
	float d = fmodFast(v0 - v1, size);
	float ad = (float)fabs(d);
	float dd = size - ad;
	if (ad <= dd) return d;
	return d < 0 ? d + size : d - size;
}

inline float uncycle(float f1, float f0, float size) {
	return f0 + getDist(f1, f0, size);
}

inline float getDeltaAngle(float to, float from) {
	return getDist(to, from, 2 * M_PI);
}

inline float cycleAngle(float a) {
	return cycle(a, 2 * M_PI);
}


inline bool isEq(float x, float y, float eps = FLT_EPS) {
	return fabsf(x - y) < eps;
}

inline bool isLess(float x, float y, float eps = FLT_EPS) {
	return x < y + eps;
}

inline bool isGreater(float x, float y, float eps = FLT_EPS) {
	return x > y - eps;
}

} // namespace QDEngine

#endif // QDENGINE_XLIBS_UTIL_XMATH_SAFEMATH_H
