/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef MATH_RECT2D_H
#define MATH_RECT2D_H

#include "math/vector2d.h"
#include "math/line2d.h"

namespace Math {

class Segment2d;

class Rect2d {
public:
	Rect2d();
	Rect2d(const Vector2d &topLeft, const Vector2d &bottomRight);
	Rect2d(const Vector2d &topLeft, const Vector2d &topRight,
		   const Vector2d &bottomLeft, const Vector2d &bottomRight);

	void rotateAround(const Vector2d &point, const Angle &angle);
	void rotateAroundCenter(const Angle &angle);
	void moveCenterTo(const Vector2d &pos);
	void scale(float amount);
	void translate(const Vector2d &vec);
	bool intersectsRect(const Rect2d &rect) const;
	bool intersectsCircle(const Vector2d &center, float radius) const;
	bool containsPoint(const Vector2d &point) const;

	Vector2d getCenter() const;
	Vector2d getTopLeft() const;
	Vector2d getTopRight() const;
	Vector2d getBottomLeft() const;
	Vector2d getBottomRight() const;
	float getWidth() const;
	float getHeight() const;
	Vector2d getIntersection(const Vector2d &start, const Vector2d &direction, Segment2d *edge) const;

// private:
	Vector2d _topLeft;
	Vector2d _topRight;
	Vector2d _bottomLeft;
	Vector2d _bottomRight;
};

}

#endif
