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

#include "tesoro/core/collision.h"

namespace Tesoro {

namespace {

/** A point along the movement as an exact fraction, 0 at the start and 1 at the end. The denominator is positive. */
struct Fraction {
	int64 numerator, denominator;
};

Fraction makeFraction(int64 numerator, int64 denominator) {
	return denominator < 0 ? Fraction{-numerator, -denominator} : Fraction{numerator, denominator};
}

bool lessThan(const Fraction &a, const Fraction &b) {
	return a.numerator * b.denominator < b.numerator * a.denominator;
}

} // End of anonymous namespace

int32 sweepRects(int32 ax1, int32 ay1, int32 ax2, int32 ay2, int32 bx1, int32 by1, int32 bx2, int32 by2, int32 dx, int32 dy) {
	// If they already overlap, return 0 immediately.
	if (ax1 <= bx2 && bx1 <= ax2 && ay2 <= by1 && by2 <= ay1)
		return 0;

	// On each axis the two overlap during a window of time. The rectangles touch when the windows of both axes are open
	// together, so the answer is the later of the two openings, as long as it comes before either window closes and
	// within the movement (0 to 1).
	const int32 from[2] = {ax1, ay2}, to[2] = {ax2, ay1};
	const int32 otherFrom[2] = {bx1, by2}, otherTo[2] = {bx2, by1};
	const int32 move[2] = {dx, dy};

	// mark when the window opens for each axis and when it closes
	Fraction opens = {0, 1}, closes = {1, 1};
	for (int axis = 0; axis < 2; ++axis) {
		if (move[axis] == 0) {
			// If theres no movement in this axis, then it either overlaps the whole time or never does.
			if (from[axis] > otherTo[axis] || otherFrom[axis] > to[axis])
				return -1;
			continue;
		}

		// The leading edge reaches the other rectangle, then the trailing edge leaves it.
		const bool forward = move[axis] > 0;
		const Fraction start = makeFraction(forward ? otherFrom[axis] - to[axis] : otherTo[axis] - from[axis], move[axis]);
		const Fraction end = makeFraction(forward ? otherTo[axis] - from[axis] : otherFrom[axis] - to[axis], move[axis]);
		if (lessThan(opens, start))
			opens = start;
		if (lessThan(end, closes))
			closes = end;
	}

	if (lessThan(closes, opens))
		return -1;

	// The original's fixed point: 16 bits of fraction, truncated.
	return (int32)((opens.numerator * 0x10000) / opens.denominator);
}

} // End of namespace Tesoro
