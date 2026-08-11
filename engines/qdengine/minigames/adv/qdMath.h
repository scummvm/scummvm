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

#ifndef QDENGINE_MINIGAMES_ADV_QDMATH_H
#define QDENGINE_MINIGAMES_ADV_QDMATH_H

#include "qdengine/xmath.h"

namespace QDEngine {

#define SQRT2 1.41421356f
#define SQRT3 1.73205081f

inline float dist(const mgVect2f& v1, const mgVect2f& v2) {
	return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

inline float abs(const mgVect2f& v) {
	return sqrt(v.x * v.x + v.y * v.y);
}

inline void norm(mgVect2f& v) {
	float mod = abs(v);
	if (mod < FLT_EPS) {
		v = mgVect2f(0, 1);
		return;
	}
	v.x /= mod;
	v.y /= mod;
}

template<class T, class T1, class T2>
inline T clamp(const T& x, const T1& xmin, const T2& xmax) {
	if (x < xmin) return xmin;
	if (x > xmax) return xmax;
	return x;
}

template<class T>
inline T abs(const T& x) {
	if (x < 0) return -x;
	return x;
}

} // namespace QDEngine

#endif // QDENGINE_MINIGAMES_ADV_QDMATH_H
