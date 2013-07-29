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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_ZVISION_POINT_H
#define COMMON_ZVISION_POINT_H

#include "common/types.h"

namespace ZVision {

/**
 * Simple class for handling both 2D position and size.
 */
template<class T>
class Point {
public:
	T x;	///< The horizontal part of the point
	T y;	///< The vertical part of the point

public:
	Point() : x(0), y(0) {}
	Point(T x1, T y1) : x(x1), y(y1) {}

public:
	bool operator==(const Point &p) const { return x == p.x && y == p.y; }
	bool operator!=(const Point &p) const { return x != p.x || y != p.y; }
	Point operator+(const Point &delta) const {	return Point(x + delta.x, y + delta.y);	}
	Point operator-(const Point &delta) const {	return Point(x - delta.x, y - delta.y);	}

	void operator+=(const Point &delta) {
		x += delta.x;
		y += delta.y;
	}

	void operator-=(const Point &delta) {
		x -= delta.x;
		y -= delta.y;
	}

	/**
	 * Return the square of the distance between this point and the point p.
	 *
	 * @param p		the other point
	 * @return the distance between this and p
	 */
	uint sqrDist(const Point &p) const {
		int diffx = ABS(p.x - x);
		if (diffx >= 0x1000)
			return 0xFFFFFF;

		int diffy = ABS(p.y - y);
		if (diffy >= 0x1000)
			return 0xFFFFFF;

		return uint(diffx * diffx + diffy * diffy);
	}
};

} // End of namespace ZVision

#endif
