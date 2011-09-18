/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#ifndef MATH_LINE2D_H
#define MATH_LINE2D_H

#include "math/vector2d.h"

namespace Math {

class Line2d {
public:
	Line2d();
	Line2d(const Vector2d &direction, const Vector2d &point);

	Line2d getPerpendicular(const Vector2d &point = Vector2d()) const;
	Vector2d getDirection() const;

	bool intersectsLine(const Line2d &line, Vector2d *pos) const;
	bool containsPoint(const Vector2d &point) const;

	float getYatX(float x) const;

private:
	float _a, _b, _c;

};

class Segment2d {
public:
	Segment2d();
	Segment2d(const Vector2d &begin, const Vector2d &end);
	Segment2d(const Segment2d &other);

	Vector2d begin() const;
	Vector2d end() const;
	Vector2d middle() const;
	Line2d getLine() const;
	Line2d getPerpendicular(const Vector2d &point = Vector2d()) const;

	bool containsPoint(const Vector2d &point) const;

	bool intersectsLine(const Line2d &line, Vector2d *pos);
	bool intersectsSegment(const Segment2d &line, Vector2d *pos);

	void operator=(const Segment2d &other);

private:
	Math::Vector2d _begin, _end;

};

}

#endif


