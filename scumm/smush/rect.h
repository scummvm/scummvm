/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#ifndef SMUSH_RECT_H
#define SMUSH_RECT_H

#include "config.h"

/*! 	@brief simple class for handling both 2D position and size

	This small class is an helper for position and size values.
*/
class Point {
	friend class Rect;
private:
	int32 _x;	//!< The horizontal part of the point
	int32 _y;	//!< The vertical part of the point
public:
	Point() : _x(0), _y(0) {};
	Point(const Point & p) : _x(p._x), _y(p._y) {};
	explicit Point(int32 x, int32 y) : _x(x), _y(y) {};
	Point & operator=(const Point & p) { _x = p._x; _y = p._y; return *this; };
	bool operator==(const Point & p) const { return _x == p._x && _y == p._y; };
	const int32 & getX() const { return _x; };
	const int32 & getY() const { return _y; };
	int32 & getX() { return _x; };
	int32 & getY() { return _y; };
	Point operator+(const Point & p) const { return Point(_x + p._x, _y+p._y); };
	Point operator-(const Point & p) const { return Point(_x - p._x, _y-p._y); };
	Point & operator+=(const Point & p) { _x += p._x; _y += p._y; return *this; };
	Point & operator-=(const Point & p) { _x -= p._x; _y -= p._y; return *this; };
	bool isOrigin() const { return _x == 0 && _y == 0; };
	void set(int32 x, int32 y) { _x = x; _y = y; }
};

/*! 	@brief simple class for handling a rectangular zone.

	This small class is an helper for rectangles.
	It is mostly used by the blitter class.
*/
class Rect {
private:
	Point _topLeft;		//!< The point at the top left of the rectangle
	Point _bottomRight;	//!< The point at the bottom right of the rectangle
public:
	Rect() : _topLeft(0, 0), _bottomRight(0,0) {}
	Rect(int32 x, int32 y) : _topLeft(0, 0), _bottomRight(x, y) {}
	Rect(int32 x1, int32 y1, int32 x2, int32 y2) : _topLeft(x1, y1), _bottomRight(x2, y2) {}
	Point size() const { return (_bottomRight - _topLeft); };
	int32 width() const { return size()._x; }
	int32 height() const { return size()._y; }
	int32 left() const { return _topLeft._x; }
	int32 right() const { return _bottomRight._x; }
	int32 top() const { return _topLeft._y; }
	int32 bottom() const { return _bottomRight._y; }
	const Point & topLeft() const { return _topLeft; }
	const Point & bottomRight() const { return _bottomRight; }

	/*!	@brief check if given position is inside the rectangle
		
		@param x the horizontal position to check
		@param y the vertical position to check	
		
		@return true if the given position is inside the rectangle, false otherwise
	*/
	bool isInside(int32 x, int32 y) const {
		return (_topLeft._x <= x) && (_bottomRight._x > x) && (_topLeft._y <= y) && (_bottomRight._y > y);
	}
	/*!	@brief check if given point is inside the rectangle
		
		@param p the point to check
		
		@return true if the given point is inside the rectangle, false otherwise
	*/
	bool isInside(const Point & p) const {
		return (_topLeft._x <= p._x) && (_bottomRight._x > p._x) && (_topLeft._y <= p._y) && (_bottomRight._y > p._y);
	}

	bool clip(Rect & r) const;
};

#endif
