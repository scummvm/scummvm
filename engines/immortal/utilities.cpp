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

#include "immortal/utilities.h"

namespace Immortal {

/* 
 *
 * -----             -----
 * ----- General Use -----
 * -----             -----
 *
 */

void Utilities::delay(int j) {             // Delay is measured in jiffies, which are 56.17ms
	g_system->delayMillis(j * 56);
}

void Utilities::delay4(int j) {            // Named in source quarterClock for some reason, 1/4 jiffies are 14.04ms
	g_system->delayMillis(j * 14);
}

void Utilities::delay8(int j) {            // 1/8 jiffies are 7.02ms
	g_system->delayMillis(j * 7);
}

bool Utilities::inside(uint8 dist, uint8 cenX, uint8 cenY, uint8 pointX, uint8 pointY) {
	// you can't be within 0 distance of something
	if (dist == 0) {
		return false;
	}

	// we want the negative distance because this is a rectangle all the way around a point
	uint8 negDist = ((dist ^ 0xFF) + 1) + 1;

	// First is the X, so we get delta X from the points
	uint8 dX = cenX - pointX;
	if (dX < 0x80) {
		// Our point is beyond the other point
		if (dX >= dist) {
			// And it is further than point + distance, so it's not useable
			return false;
		}

	} else if (dX >= negDist) {
		// If the negative delta X is *greater* than the negative distance, that means we're not far *enough* in the X
		return false;
	}

	// Exact same system here but with the Y positions instead
	uint8 dY = cenY - pointY;
	if (dY < 0x80) {
		if (dY >= dist) {
			return false;
		}

	} else if (dY >= negDist) {
		return false;
	}

	// If all conditions are met, we are within the distance of the point
	return true;
}

bool Utilities::insideRect(uint8 rectX, uint8 rectY, uint8 w, uint8 h, uint8 pointX, uint8 pointY) {
	/* Very simple comapred to inside, we simply check
	 * first if width and height are >0, to make sure
	 * the rectangle has a size, and then we see if
	 * the point is between the point X,Y and the
	 * point X,Y + the width,height of the rect.
	 * This is done by grabbing the delta X,Y and
	 * making sure it is not negative.
	 */

	// The source specifically checks only for w *and* h being 0, so you could give it a rect with a width or height or 0, just not both
	if ((w == 0) && (h == 0)) {
		return false;
	}

	uint8 dX = pointX - rectX;
	uint8 dY = pointY - rectY;

	if ((dX < 0x80) && (dX < w)) {
		if ((dY < 0x80) && (dY < h)) {
			return true;
		}
	}

	return false;
}

}; // namespace Immortal
