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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

/*
    BS_Line
    -------
    This class contains only static methods, which have to do with straight line
    segments. There is no real straight line segment class. Calculations will be
    used with polygons, and it is important the process of starting and selecting
    endpoints of lines is dynamic. This would prhobit a polygon from a set
    being formed by fixed line segments

    Autor: Malte Thiesen
*/

#ifndef SWORD25_LINE_H
#define SWORD25_LINE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

// -----------------------------------------------------------------------------

namespace Sword25 {

class BS_Line {
public:
	/**
	 * Determines whether a piont is left of a line
	 * @param a         The start point of a line
	 * @param b         The end point of a line
	 * @param c         The test point
	 * @return          Returns true if the point is to the left of the line.
	 * If the point is to the right of the line or on the line, false is returned.
	 */
	static bool IsVertexLeft(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		return _TriangleArea2(a, b, c) > 0;
	}

	static bool IsVertexLeftOn(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		return _TriangleArea2(a, b, c) >= 0;
	}

	/**
	 * Determines whether a piont is right of a line
	 * @param a         The start point of a line
	 * @param b         The end point of a line
	 * @param c         The test point
	 * @return          Returns true if the point is to the right of the line.
	 * If the point is to the right of the line or on the line, false is returned.
	 */
	static bool IsVertexRight(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		return _TriangleArea2(a, b, c) < 0;
	}

	static bool IsVertexRightOn(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		return _TriangleArea2(a, b, c) <= 0;
	}

	/**
	 * Determines whether a piont is on a line
	 * @param a         The start point of a line
	 * @param b         The end point of a line
	 * @param c         The test point
	 * @return          Returns true if the point is on the line, false otherwise.
	 */
	static bool IsVertexOn(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		return _TriangleArea2(a, b, c) == 0;
	}

	enum VERTEX_CLASSIFICATION {
		LEFT,
		RIGHT,
		ON
	};

	/**
	 * Determines where a point is relative to a line.
	 * @param a         The start point of a line
	 * @param b         The end point of a line
	 * @param c         The test point
	 * @return          LEFT is returned if the point is to the left of the line.
	 * RIGHT is returned if the point is to the right of the line.
	 * ON is returned if the point is on the line.
	 */
	static VERTEX_CLASSIFICATION ClassifyVertexToLine(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		int Area = _TriangleArea2(a, b, c);
		if (Area > 0) return LEFT;
		if (Area < 0) return RIGHT;
		return ON;
	}

	/**
	 * Determines whether two lines intersect
	 * @param a         The start point of the first line
	 * @param b         The end point of the first line
	 * @param c         The start point of the second line
	 * @param d         The end point of the second line
	 * @remark          In cases where a line only touches the other, false is returned (improper intersection)
	 */
	static bool DoesIntersectProperly(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c, const BS_Vertex &d) {
		VERTEX_CLASSIFICATION Class1 = ClassifyVertexToLine(a, b, c);
		VERTEX_CLASSIFICATION Class2 = ClassifyVertexToLine(a, b, d);
		VERTEX_CLASSIFICATION Class3 = ClassifyVertexToLine(c, d, a);
		VERTEX_CLASSIFICATION Class4 = ClassifyVertexToLine(c, d, b);

		if (Class1 == ON || Class2 == ON || Class3 == ON || Class4 == ON) return false;

		return ((Class1 == LEFT) ^(Class2 == LEFT)) && ((Class3 == LEFT) ^(Class4 == LEFT));
	}

	/**
	 * Determines whether a point is on a line segment
	 * @param a         The start point of a line
	 * @param b         The end point of a line
	 * @param c         The test point
	 */
	static bool IsOnLine(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		// The items must all be Collinear, otherwise don't bothering testing the point
		if (_TriangleArea2(a, b, c) != 0) return false;

		// If the line segment is not vertical, check on the x-axis, otherwise the y-axis
		if (a.X != b.X) {
			return ((a.X <= c.X) &&
			        (c.X <= b.X)) ||
			       ((a.X >= c.X) &&
			        (c.X >= b.X));
		} else {
			return ((a.Y <= c.Y) &&
			        (c.Y <= b.Y)) ||
			       ((a.Y >= c.Y) &&
			        (c.Y >= b.Y));
		}
	}

	static bool IsOnLineStrict(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		// The items must all be Collinear, otherwise don't bothering testing the point
		if (_TriangleArea2(a, b, c) != 0) return false;

		// If the line segment is not vertical, check on the x-axis, otherwise the y-axis
		if (a.X != b.X) {
			return ((a.X < c.X) &&
			        (c.X < b.X)) ||
			       ((a.X > c.X) &&
			        (c.X > b.X));
		} else {
			return ((a.Y < c.Y) &&
			        (c.Y < b.Y)) ||
			       ((a.Y > c.Y) &&
			        (c.Y > b.Y));
		}
	}

private:
	/**
	 * Return double the size of the triangle defined by the three passed points.
	 *
	 * The result is positive if the points are arrange counterclockwise,
	 * and negative if they are arranged counter-clockwise.
	 */
	static int _TriangleArea2(const BS_Vertex &a, const BS_Vertex &b, const BS_Vertex &c) {
		return a.X * b.Y - a.Y * b.X +
		       a.Y * c.X - a.X * c.Y +
		       b.X * c.Y - c.X * b.Y;
	}
};

} // End of namespace Sword25

#endif
