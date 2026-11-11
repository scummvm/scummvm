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

#ifndef TESORO_CORE_OUTLINE_H
#define TESORO_CORE_OUTLINE_H

#include "common/scummsys.h"

namespace Tesoro {

/**
 * Outlines used for pixel-accurate collision between two objects that move.
 * Ppoints with x and y in the low and high half of a word. The first chain, points 0 to split,
 * runs left to right; the second chain, split + 1 to count - 1, comes back right to left,
 * so together they close the shape.
 */
struct Outline {
	uint16 count = 0;
	uint16 split = 0;
	const uint32 *points = nullptr;

	// The original also reads the words next to the array when a walk runs off an end (shapes whose first chain is only
	// their first point); what it finds there depends on memory it does not own, so those reads give 0 here.
	int x(int i) const { return i >= 0 && i < (int)count ? (int16)(points[i] & 0xffff) : 0; }
	int y(int i) const { return i >= 0 && i < (int)count ? (int16)(points[i] >> 16) : 0; }

	/** Whether (px, py) is inside the outline or on it, counting the closing edge from the last point to the first. */
	bool contains(int px, int py) const;

	/**
	 * Cuts this outline to the columns cx1 to cx2 into out, whose points need room for count + 4 entries. bx1 and bx2 are the
	 * columns the outline covers: a side that already lies inside the cut is copied unchanged.
	 */
	void clipColumns(int bx1, int bx2, int cx1, int cx2, Outline &out, uint32 *outPoints) const;

	/**
	 * Whether the two outlines touch, found by walking one chain of each side by side and watching whether they ever
	 * cross. The test is not symmetric: the game asks it both ways round.
	 */
	bool overlaps(const Outline &other) const;
};

} // End of namespace Tesoro

#endif
