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
private:
	int _x;	//!< The horizontal part of the point
	int _y;	//!< The vertical part of the point
public:
	Point() : _x(0), _y(0) {};
	Point(const Point & p) : _x(p._x), _y(p._y) {};
	explicit Point(int x, int y) : _x(x), _y(y) {};
	Point & operator=(const Point & p) { _x = p._x; _y = p._y; return *this; };
	bool operator==(const Point & p) const { return _x == p._x && _y == p._y; };
	const int & getX() const { return _x; };
	const int & getY() const { return _y; };
	int & getX() { return _x; };
	int & getY() { return _y; };
	Point operator+(const Point & p) const { return Point(_x + p._x, _y+p._y); };
	Point operator-(const Point & p) const { return Point(_x - p._x, _y-p._y); };
	Point & operator+=(const Point & p) { _x += p._x; _y += p._y; return *this; };
	Point & operator-=(const Point & p) { _x -= p._x; _y -= p._y; return *this; };
	bool isOrigin() const { return _x == 0 && _y == 0; };
	void set(int x, int y) { _x = x; _y = y; }
};

/*! 	@brief simple class for handling a rectangular zone.

	This small class is an helper for rectangles.
	It is mostly used by the blitter class.
*/
class Rect {
private:
	Point _topLeft;		//!< The point at the top left of the rectangle
	Point _bottomRight;	//!< The point at the bottom right of the rectangle
protected:
	void check();
public:
	Rect();
	Rect(int x, int y);
	explicit Rect(const Point & size);
	Rect(int x1, int y1, int x2, int y2);
	Rect(const Point & topleft, const Point & bottomright);
	Rect(const Rect & r);
	Rect & operator=(const Rect & r);
	bool operator==(const Rect & r) const;
	Point size() const { return (_bottomRight - _topLeft); };
	int width() const { return size().getX(); }
	int height() const { return size().getY(); }
	int left() const { return _topLeft.getX(); }
	int right() const { return _bottomRight.getX(); }
	int top() const { return _topLeft.getY(); }
	int bottom() const { return _bottomRight.getY(); }
	const Point & topLeft() const { return _topLeft; }
	const Point & bottomRight() const { return _bottomRight; }

	/*!	@brief check if given position is inside the rectangle
		
		@param x the horizontal position to check
		@param y the vertical position to check	
		
		@return true if the given position is inside the rectangle, false otherwise
	*/
	bool isInside(int x, int y) const;
	/*!	@brief check if given point is inside the rectangle
		
		@param p the point to check
		
		@return true if the given point is inside the rectangle, false otherwise
	*/
	bool isInside(const Point & p) const;
	bool clip(Rect & r) const;
};

#endif
