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
 * $URL$
 * $Id$
 *
 */

#ifndef COMMON_SHAPE_H
#define COMMON_SHAPE_H

#include "common/scummsys.h"
#include "common/util.h"

namespace Common {

class Rect;

/*!		@brief simple class for handling both 2D position and size

	This small class is an helper for position and size values.
*/
struct Point {
	int16 x;	//!< The horizontal part of the point
	int16 y;	//!< The vertical part of the point

	Point() : x(0), y(0) {}
	Point(const Point &p) : x(p.x), y(p.y) {}
	explicit Point(int16 x1, int16 y1) : x(x1), y(y1) {}
	Point & operator=(const Point & p) { x = p.x; y = p.y; return *this; };
	bool operator==(const Point & p) const { return x == p.x && y == p.y; };
	bool operator!=(const Point & p) const { return x != p.x || y != p.y; };

	/**
	 * Return the square of the distance between this point and the point p.
	 *
	 * @param p		the other point
	 * @return the distance between this and p
	 */
	uint sqrDist(const Point & p) const {
		int diffx = ABS(p.x - x);
		if (diffx >= 0x1000)
			return 0xFFFFFF;

		int diffy = ABS(p.y - y);
		if (diffy >= 0x1000)
			return 0xFFFFFF;

		return uint(diffx*diffx + diffy*diffy);
	}
};

/*!		@brief simple interface that provides common methods for 2D shapes

*/
struct Shape {

	/*!	@brief check if given position is inside this shape

		@param x the horizontal position to check
		@param y the vertical position to check

		@return true if the given position is inside this shape, false otherwise
	*/
	virtual bool contains(int16 x, int16 y) const = 0;

	/*!	@brief check if given point is inside this shape

		@param p the point to check

		@return true if the given point is inside this shape, false otherwise
	*/
	virtual bool contains(const Point &p) const = 0;

	virtual void moveTo(int16 x, int16 y) = 0;

	virtual void moveTo(const Point &p) = 0;

	virtual void translate(int16 dx, int16 dy) = 0;

	virtual Rect getBoundingRect() const = 0;

};

} // end of namespace Common

#endif
