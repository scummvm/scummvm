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

#ifndef MADS_CORE_IMATH_H
#define MADS_CORE_IMATH_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

/**
 * Distance: fast approximate hypotenuse with exact fallback.
 * Uses an octagonal approximation with a correction term.
 * Guaranteed to be a lower bound (underestimates by at most ~3%).
 * Falls through to exact hypot if the estimate is below thresh.
 */
extern int imath_distance(int side_a, int side_b, int threshold);

/**
 * Exact integer hypotenuse via sum of squares.
 */
extern int imath_hypot(int side_a, int side_b);

/**
 * Integer square root (truncating).
 * @return		Returns 0 for negative input.
 */
extern int imath_isqrt(long square);

extern void imath_circular_arc(word *buffer, int radius);

extern int imath_random(int from, int unto);

} // namespace MADSV2
} // namespace MADS

#endif
