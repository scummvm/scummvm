/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef COMMON_RECT_H
#define COMMON_RECT_H

#include "common/scummsys.h"
#include "common/util.h"

namespace Common {

/*! 	@brief simple class for handling both 2D position and size

	This small class is an helper for position and size values.
*/
struct Point {
	int16 x;	//!< The horizontal part of the point
	int16 y;	//!< The vertical part of the point

	Point() : x(0), y(0) {};
	Point(const Point & p) : x(p.x), y(p.y) {};
	explicit Point(int16 x1, int16 y1) : x(x1), y(y1) {};
	Point & operator=(const Point & p) { x = p.x; y = p.y; return *this; };
	bool operator==(const Point & p) const { return x == p.x && y == p.y; };
	bool operator!=(const Point & p) const { return x != p.x || y != p.y; };
};

/*! 	@brief simple class for handling a rectangular zone.

	This small class is an helper for rectangles.
	It is mostly used by the blitter class.
*/
struct Rect {
	int16 top, left;		//!< The point at the top left of the rectangle (part of the rect).
	int16 bottom, right;	//!< The point at the bottom right of the rectangle (not part of the rect).

	Rect() : top(0), left(0), bottom(0), right(0) {}
	Rect(int16 x, int16 y) : top(0), left(0), bottom(x), right(y) {}
	Rect(int16 x1, int16 y1, int16 x2, int16 y2) : top(y1), left(x1), bottom(y2), right(x2) {}
	int16 width() const { return right - left; }
	int16 height() const { return bottom - top; }

	/*!	@brief check if given position is inside this rectangle
		
		@param x the horizontal position to check
		@param y the vertical position to check	
		
		@return true if the given position is inside this rectangle, false otherwise
	*/
	bool contains(int16 x, int16 y) const {
		return (left <= x) && (x < right) && (top <= y) && (y < bottom);
	}

	/*!	@brief check if given point is inside this rectangle
		
		@param p the point to check
		
		@return true if the given point is inside this rectangle, false otherwise
	*/
	bool contains(const Point & p) const {
		return (left <= p.x) && (p.x < right) && (top <= p.y) && (p.y < bottom);
	}

	/*!	@brief check if given rectangle intersects with this rectangle
		
		@param r the rectangle to check
		
		@return true if the given rectangle is inside the rectangle, false otherwise
	*/
	bool intersects(const Rect & r) const {
		return (left < r.right) && (r.left < right) && (top < r.bottom) && (r.top < bottom);
	}

	/*!	@brief extend this rectangle so that it contains r
		
		@param r the rectangle to extend by
	*/
	void extend(const Rect & r) {
		left = MIN(left, r.left);
		right = MAX(right, r.right);
		top = MIN(top, r.top);
		bottom = MAX(bottom, r.bottom);
	}
	
	void grow(int16 offset) {
		top -= offset;
		left -= offset;
		bottom += offset;
		right += offset;
	}
};

}	// End of namespace Common

#endif
