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

#include "common/util.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

int imath_isqrt(long square) {
	if (square <= 0)
		return 0;
	return (int)sqrt((double)square);
}

int imath_hypot(int side_A, int side_B) {
	long sum = (long)side_A * side_A + (long)side_B * side_B;
	return imath_isqrt(sum);
}

int imath_distance(int side_A, int side_B, int thresh) {
	int dx, dy, ds;

	dx = ABS(side_A);
	dy = ABS(side_B);

	// Ensure dx >= dy for the approximation formula.
	if (dx < dy) {
		int tmp = dx; dx = dy; dy = tmp;
	}

	// Base approximation: ds = dx + dy/4
	ds = dx + (dy >> 2);

	// Correction term if 2*dy >= dx: ds += (2*dy - dx) * 3/16
	if (2 * dy > dx) {
		ds += ((2 * dy - dx) * 3) >> 4;
	}

	// Subtract ~3% to ensure result is a lower bound.
	ds -= (ds >> 6);  // ds -= ds/64, approximately 1.5% ...
	// Note: assembly uses shr cx,6 but comment says 3% - see below
	if (ds < thresh)
		return imath_hypot(side_A, side_B);

	return ds;
}

void imath_circular_arc(word *buffer, int radius) {
	long y_value;
	long radius_squared;
	long difference;
	word root;
	int count;

	radius_squared = ((long)radius << 2L);
	radius_squared *= radius_squared;

	for (count = 0; count < radius; count++) {
		y_value = ((long)count << 2L);
		y_value *= y_value;
		difference = radius_squared - y_value;
		root = imath_isqrt(difference);
		root = root >> 1;
		root++;
		root = root >> 1;
		buffer[count] = root;
	}
}

int imath_random(int from, int unto) {
	return g_engine->getRandomNumber(from, unto);
}

} // namespace MADSV2
} // namespace MADS
