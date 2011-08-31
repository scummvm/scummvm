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

#ifndef GRAPHICS_VECTOR2D_H
#define GRAPHICS_VECTOR2D_H

#include "graphics/vector3d.h"

namespace Graphics {

class Vector2d {
public:
	Vector2d();
	Vector2d(float x, float y);
	Vector2d(const Vector2d &vec);

	inline float getX() const { return _x; }
	inline float getY() const { return _y; }
	void setX(float x);
	void setY(float y);

	Vector2d &operator=(const Vector2d &vec);
	Vector2d &operator/=(float s);

	void normalize();
	Vector2d getNormalized() const;

	void rotateAround(const Vector2d &point, float angle);
	float getAngle() const;
	float getMagnitude() const;
	float getDistanceTo(const Vector2d &point) const;

	Vector3d toVector3d() const;

private:
	float _x;
	float _y;
};

inline Vector2d operator-(const Vector2d& v1, const Vector2d& v2) {
	Vector2d result(v1.getX() - v2.getX(), v1.getY() - v2.getY());
	return result;
}

inline Vector2d operator+(const Vector2d &v1, const Vector2d &v2) {
	Vector2d result(v1.getX() + v2.getX(), v1.getY() + v2.getY());
	return result;
}

inline Vector2d operator*(const Vector2d &v1, float factor) {
	Vector2d result(v1.getX() * factor, v1.getY() * factor);
	return result;
}

inline Vector2d operator/(const Vector2d &v1, float factor) {
	Vector2d result(v1.getX() / factor, v1.getY() / factor);
	return result;
}

inline Vector2d operator-(const Vector2d &v) {
	return Vector2d(-v.getX(), -v.getY());
}

}

#endif
