/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	 * Rotate the ray using a quaternion - rotates both origin and direction
	 */
	void rotate(const Quaternion &rot);

	/**
	 * Rotate the ray direction only using a quaternion - origin stays fixed
	 */
	void rotateDirection(const Quaternion &rot);

	/**
	 * Translate the ray by a vector
	 */
	void translate(const Vector3d &v);

	/**
	 * Test the intersection of the ray with an Axis Aligned Bounding Box
	 */
	bool intersectAABB(const AABB &aabb) const;

	/**
	 * Test and return the intersection of the ray with a triangle defned by 3 verticies
	 *
	 * @param v0 first triangle vertex
	 * @param v1 second triangle vertex
	 * @param v2 third triangle vertex
	 * @param loc If return is true, set to the intersection point
	 * @param dist If return is true, set to distance along the ray (relative to direction)
	 */
	bool intersectTriangle(const Vector3d &v0, const Vector3d &v1, const Vector3d &v2, Vector3d &loc, float &dist) const;

private:
	Vector3d _origin;
	Vector3d _direction;
};

} // end of namespace Math

#endif
