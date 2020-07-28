/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_MISC_DIRECTION_H
#define ULTIMA8_MISC_DIRECTION_H

namespace Ultima {
namespace Ultima8 {

/*
 *  Directions:
 */
enum Direction {
	dir_north = 0,
	dir_northeast = 1,
	dir_east = 2,
	dir_southeast = 3,
	dir_south = 4,
	dir_southwest = 5,
	dir_west = 6,
	dir_northwest = 7,
	dir_current = 16
};

/*
 * Tables to map a Direction to x/y deltas
 */
static const int x_fact[] = {  0, +1, +1, +1,  0, -1, -1, -1 };
static const int y_fact[] = { -1, -1,  0, +1, +1, +1,  0, -1 };

static const int x_fact16[] = {  0, +1, +2, +2, +2, +2, +2, +1, 0, -1, -2, -2, -2, -2, -2, -1 };
static const int y_fact16[] = { -2, -2, -2, -1,  0, +1, +2, +2, +2, +2, +2, +1, 0, -1, -2, -2 };

/*
 *  Return the direction for a given slope (0-7).
 *  NOTE:  Assumes cartesian coords, NOT screen coords. (which have y
 *      growing downwards).
 *
 *  NOTE: The returned direction is rotated 45 degrees clockwise! This is
 *  how U8 things should be.
 */

inline Direction Get_direction(int deltay, int deltax) {
	if (deltax == 0)
		return deltay > 0 ? dir_northwest : dir_southeast;
	int dydx = (1024 * deltay) / deltax; // Figure 1024*tan.
	if (dydx >= 0)
		if (deltax > 0) // Top-right
			return dydx <= 424 ? dir_northeast : dydx <= 2472 ? dir_north
			       : dir_northwest;
		else            // Bottom-left.
			return dydx <= 424 ? dir_southwest : dydx <= 2472 ? dir_south
			       : dir_southeast;
	else if (deltax > 0) // Bottom-right.
		return dydx >= -424 ? dir_northeast : dydx >= -2472 ? dir_east
		       : dir_southeast;
	else            // Top-left
		return dydx >= -424 ? dir_southwest : dydx >= -2472 ? dir_west
		       : dir_northwest;
}


inline Direction Get_WorldDirection(int deltay, int deltax) {
	if (deltax == 0) {
		if (deltay == 0) return dir_northeast; // for better compatibility with U8
		return deltay > 0 ? dir_south : dir_north;
	}
	int dydx = (1024 * deltay) / deltax;

	if (dydx >= 0)
		if (deltax > 0) // south-east
			return dydx <= 424 ? dir_east : dydx <= 2472 ? dir_southeast : dir_south;
		else            // north-west
			return dydx <= 424 ? dir_west : dydx <= 2472 ? dir_northwest : dir_north;
	else if (deltax > 0) // north-east
		return dydx >= -424 ? dir_east : dydx >= -2472 ? dir_northeast : dir_north;
	else            // south-west
		return dydx >= -424 ? dir_west : dydx >= -2472 ? dir_southwest : dir_south;
}

inline Direction Get_WorldDirectionClosestInRange(int deltay, int deltax, uint16 ndirs, uint16 mindir, uint16 maxdir) {
	// TODO: Implement proper 16 directions here.
	uint32 dir = static_cast<uint32>(Get_WorldDirection(deltay, deltax));
	if (ndirs == 16) {
		dir *= 2;
	}

	if ((dir < mindir) || (dir > maxdir)) {
		int32 dmin1 = dir - mindir;
		int32 dmin2 = mindir - dir;
		if (dmin1 < 0) {
			dmin1 = dmin1 + ndirs;
		}
		if (dmin2 < 0) {
			dmin2 = dmin2 + ndirs;
		}
		int32 dist_to_min = MIN(dmin1, dmin2);

		int dmax1 = dir - maxdir;
		int dmax2 = maxdir - dir;
		if (dmax1 < 0) {
			dmax1 = dmax1 + ndirs;
		}
		if (dmax2 < 0) {
			dmax2 = dmax2 + ndirs;
		}
		int32 dist_to_max = MIN(dmax1, dmax2);

		if (dist_to_min < dist_to_max) {
			return static_cast<Direction>(mindir);
		} else {
			return static_cast<Direction>(maxdir);
		}
	}

	return static_cast<Direction>(dir);
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
