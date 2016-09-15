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

#ifndef MATH_RAY_H
#define MATH_RAY_H

#include "math/aabb.h"
#include "math/matrix4.h"
#include "math/quat.h"
#include "math/vector3d.h"

namespace Math {

/**
 * A three dimensional half-line
 */
class Ray {
public:
	Ray();
	Ray(const Vector3d &origin, const Vector3d &direction);

	Vector3d& getOrigin() { return _origin; }
	Vector3d getOrigin() const { return _origin; }
	Vector3d& getDirection() { return _direction; }
	Vector3d getDirection() const { return _direction; }

	/**
	 * Apply a transformation to the ray
	 */
	void transform(const Matrix4 &matrix);

	/**
	 * Rotate the ray using a quaternion
	 */
	void rotate(const Quaternion &rot);

	/**
	 * Translate the ray by a vector
	 */
	void translate(const Vector3d &v);

	/**
	 * Test the intersection of the ray with an Axis Aligned Bounding Box
	 */
	bool intersectAABB(const AABB &aabb) const;

private:
	Vector3d _origin;
	Vector3d _direction;
};

} // end of namespace Math

#endif
