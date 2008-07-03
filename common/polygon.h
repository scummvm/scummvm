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

#ifndef COMMON_POLYGON_H
#define COMMON_POLYGON_H

#include "common/array.h"
#include "common/rect.h"
#include "common/shape.h"

namespace Common {

struct Polygon : public Shape {

	Array<Point> points;

	Polygon() {}
	Polygon(const Polygon& p) : Shape(), points(p.points), bound(p.bound) {}
	Polygon(Array<Point> p) : points(p) {
		if (p.empty()) return;
		bound = Rect(p[0].x, p[0].y, p[0].x, p[0].y);
		for (uint i = 1; i < p.size(); i++) {
			bound.extend(Rect(p[i].x, p[i].y, p[i].x, p[i].y));
		}
	}
	Polygon(Point *p, int n) {
		for (int i = 0; i < n; i++) {
			addPoint(p[i]);
		}
	}
	virtual ~Polygon() {}

	void addPoint(const Point& p) {
		points.push_back(p);
		bound.extend(Rect(p.x, p.y, p.x, p.y));
	}

	void addPoint(int16 x, int16 y) {
		addPoint(Point(x,y));
	}

	/*!	@brief check if given position is inside this polygon

		@param x the horizontal position to check
		@param y the vertical position to check

		@return true if the given position is inside this polygon, false otherwise
	*/
	virtual bool contains(int16 x, int16 y) const;

	/*!	@brief check if given point is inside this polygon

		@param p the point to check

		@return true if the given point is inside this polygon, false otherwise
	*/
	virtual bool contains(const Point &p) const {
		return contains(p.x, p.y);
	}

	virtual void moveTo(int16 x, int16 y) {
		int16 dx = x - ((bound.right + bound.left) / 2);
		int16 dy = y - ((bound.bottom + bound.top) / 2);
		translate(dx, dy);
	}

	virtual void moveTo(const Point &p) {
		moveTo(p.x, p.y);
	}

	virtual void translate(int16 dx, int16 dy) {
		Array<Point>::iterator it;
		for (it = points.begin(); it != points.end(); it++) {
			it->x += dx;
			it->y += dy;
		}
	}

	virtual Rect getBoundingRect() const {
		return bound;
	}

private:
	Rect bound;
};

} // end of namespace Common

#endif
