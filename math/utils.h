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

inline float square(float x) {
	return x * x;
}

}

#endif
