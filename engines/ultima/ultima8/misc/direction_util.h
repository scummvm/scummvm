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

#ifndef ULTIMA8_MISC_DIRECTIONUTIL_H
#define ULTIMA8_MISC_DIRECTIONUTIL_H

#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/ultima8.h"
#include "common/math.h"

namespace Ultima {
namespace Ultima8 {

/*
 * Tables to map a Direction to x/y deltas
 */


inline int Direction_XFactor(Direction dir) {
	static const int _x_fact[] = {  0, +1, +1, +1,  0, -1, -1, -1 };
	//static const int _x_fact16[] = {  0, +1, +2, +2, +2, +2, +2, +1, 0, -1, -2, -2, -2, -2, -2, -1 };
	// TODO: AnimPrimitiveProcess uses the below table.. what's the other table for?
	// (same for y)
	static const int _x_fact16[] = {  0, +1, +1, +2, +1, +2, +1, +1, 0, -1, -1, -2, -1, -2, -1, -1 };

	if (GAME_IS_U8)
		return _x_fact[(int)dir / 2];
	else
		return _x_fact16[(int)dir];
}

inline int Direction_YFactor(Direction dir) {
	static const int _y_fact[] = { -1, -1,  0, +1, +1, +1,  0, -1 };
	//static const int _y_fact16[] = { -2, -2, -2, -1,  0, +1, +2, +2, +2, +2, +2, +1, 0, -1, -2, -2 };
	static const int _y_fact16[] = { -1, -2, -1, -1, 0, +1, +1, +2, +1, +2, +1, +1, 0, -1, -1, -2 };

	if (GAME_IS_U8)
		return _y_fact[(int)dir / 2];
	else
		return _y_fact16[(int)dir];
}

//! Convert a direction to hundreths of degrees (rotated by 90 degrees)
inline int32 Direction_ToCentidegrees(Direction dir) {
	return static_cast<int>(dir) * 2250;
}

//! Convert from centidegrees to a direction.
inline Direction Direction_FromCentidegrees(int32 cds) {
	return static_cast<Direction>(((cds + 1125) / 2250) % 16);
}


/**
 *  Return the direction for a given slope (0-7).
 *  NOTE:  Assumes cartesian coords, NOT screen coords. (which have y
 *    growing downwards).
 *
 *  NOTE: The returned direction is rotated 45 degrees clockwise! This is
 *  how U8 things should be.
 */
inline Direction Direction_Get(int deltay, int deltax, DirectionMode dirmode) {
	if (deltax == 0)
		return deltay > 0 ? dir_northwest : dir_southeast;

	if (dirmode == dirmode_8dirs) {
		int dydx = (1024 * deltay) / deltax; // Figure 1024*tan.
		if (dydx >= 0)
			if (deltax > 0) // Top-right
				return dydx <= 424 ? dir_northeast : dydx <= 2472 ? dir_north
					   : dir_northwest;
			else			// Bottom-left.
				return dydx <= 424 ? dir_southwest : dydx <= 2472 ? dir_south
					   : dir_southeast;
		else if (deltax > 0) // Bottom-right.
			return dydx >= -424 ? dir_northeast : dydx >= -2472 ? dir_east
				   : dir_southeast;
		else			// Top-left
			return dydx >= -424 ? dir_southwest : dydx >= -2472 ? dir_west
				   : dir_northwest;
	} else {
		double angle = Common::rad2deg(atan2(deltay, deltax));
		if (angle < -168.75)      return dir_southwest;
		else if (angle < -146.25) return dir_ssw;
		else if (angle < -123.75) return dir_south;
		else if (angle < -101.25) return dir_sse;
		else if (angle < -78.75)  return dir_southeast;
		else if (angle < -56.25)  return dir_ese;
		else if (angle < -33.75)  return dir_east;
		else if (angle < -11.25)  return dir_ene;
		else if (angle < 11.25)   return dir_northeast;
		else if (angle < 33.75)   return dir_nne;
		else if (angle < 56.25)   return dir_north;
		else if (angle < 78.75)   return dir_nnw;
		else if (angle < 101.25)  return dir_northwest;
		else if (angle < 123.75)  return dir_wnw;
		else if (angle < 146.25)  return dir_west;
		else if (angle < 168.75)  return dir_wsw;
		return dir_southwest;
	}
}

// Note that for WorldDir, Y goes down, so a positive Y points south.
inline Direction Direction_GetWorldDir(int deltay, int deltax, DirectionMode dirmode) {
	if (deltax == 0) {
		if (deltay == 0) return dir_northeast; // for better compatibility with U8
		return deltay > 0 ? dir_south : dir_north;
	}

	if (dirmode == dirmode_8dirs) {
		int dydx = (1024 * deltay) / deltax;

		if (dydx >= 0)
			if (deltax > 0) // south-east
				return dydx <= 424 ? dir_east : dydx <= 2472 ? dir_southeast : dir_south;
			else			// north-west
				return dydx <= 424 ? dir_west : dydx <= 2472 ? dir_northwest : dir_north;
		else if (deltax > 0) // north-east
			return dydx >= -424 ? dir_east : dydx >= -2472 ? dir_northeast : dir_north;
		else			// south-west
			return dydx >= -424 ? dir_west : dydx >= -2472 ? dir_southwest : dir_south;
	} else {
		double angle = Common::rad2deg(atan2(deltay, deltax));
		if (angle < -168.75)      return dir_west;
		else if (angle < -146.25) return dir_wnw;
		else if (angle < -123.75) return dir_northwest;
		else if (angle < -101.25) return dir_nnw;
		else if (angle < -78.75)  return dir_north;
		else if (angle < -56.25)  return dir_nne;
		else if (angle < -33.75)  return dir_northeast;
		else if (angle < -11.25)  return dir_ene;
		else if (angle < 11.25)   return dir_east;
		else if (angle < 33.75)   return dir_ese;
		else if (angle < 56.25)   return dir_southeast;
		else if (angle < 78.75)   return dir_sse;
		else if (angle < 101.25)  return dir_south;
		else if (angle < 123.75)  return dir_ssw;
		else if (angle < 146.25)  return dir_southwest;
		else if (angle < 168.75)  return dir_wsw;
		return dir_west;
	}
}



inline Direction Direction_GetWorldDirInRange(int deltay, int deltax, DirectionMode dirmode, Direction mindir, Direction maxdir) {
	int ndirs = (dirmode == dirmode_8dirs ? 8 : 16);
	Direction dir = Direction_GetWorldDir(deltay, deltax, dirmode);

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
			return mindir;
		} else {
			return maxdir;
		}
	}

	return dir;
}

inline Direction Direction_Invert(Direction dir) {
	assert(dir != dir_current);
	switch (dir) {
		case dir_north:		return dir_south;
		case dir_nne:		return dir_ssw;
		case dir_northeast:	return dir_southwest;
		case dir_ene:		return dir_wsw;
		case dir_east:		return dir_west;
		case dir_ese:		return dir_wnw;
		case dir_southeast:	return dir_northwest;
		case dir_sse:		return dir_nnw;
		case dir_south:		return dir_north;
		case dir_ssw:		return dir_nne;
		case dir_southwest:	return dir_northeast;
		case dir_wsw:		return dir_ene;
		case dir_west:		return dir_east;
		case dir_wnw:		return dir_ese;
		case dir_northwest:	return dir_southeast;
		case dir_nnw:		return dir_sse;
		default:			return dir_north;
	}
}

//! Return the direction one left (aka counter-clockwise) of the input
inline Direction Direction_OneLeft(Direction dir, DirectionMode mode) {
	if (mode == dirmode_8dirs)
		return static_cast<Direction>((static_cast<int>(dir) + 14) % 16);
	else
		return static_cast<Direction>((static_cast<int>(dir) + 15) % 16);
}

//! Return the direction one right (aka clockwise) of the input
inline Direction Direction_OneRight(Direction dir, DirectionMode mode) {
	if (mode == dirmode_8dirs)
		return static_cast<Direction>((static_cast<int>(dir) + 2) % 16);
	else
		return static_cast<Direction>((static_cast<int>(dir) + 1) % 16);
}

inline Direction Direction_TurnByDelta(Direction dir, int delta, DirectionMode mode) {
	if (delta > 0) {
		for (int i = 0; i < delta; i++)
			dir = Direction_OneRight(dir, mode);
	} else if (delta < 0) {
		for (int i = 0; i < -delta; i++)
			dir = Direction_OneLeft(dir, mode);
	}
	return dir;
}

//! Get a turn delta (-1 for left, +1 for right) to turn the fastest
//! way from one direction to another
inline int Direction_GetShorterTurnDelta(Direction from, Direction to) {
	if ((from - to + 16) % 16 < 8)
		return -1;
	return 1;
}

inline uint32 Direction_ToUsecodeDir(Direction dir) {
	if (GAME_IS_U8) {
		return static_cast<int32>(dir / 2);
	} else {
		return static_cast<int32>(dir);
	}
}

inline Direction Direction_FromUsecodeDir(uint32 dir) {
	if (GAME_IS_U8) {
		return static_cast<Direction>(dir * 2);
	} else {
		return static_cast<Direction>(dir);
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
