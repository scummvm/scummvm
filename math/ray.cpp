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

#include "math/ray.h"

#include "common/util.h"

namespace Math {

Ray::Ray() {
}

Ray::Ray(const Vector3d &origin, const Vector3d &direction) :
		_origin(origin),
		_direction(direction) {
}

void Ray::transform(const Matrix4 &matrix) {
	matrix.transform(&_origin, true);
	matrix.transform(&_direction, false);
	_direction.normalize();
}

void Ray::rotate(const Quaternion &rot) {
	rot.transform(_origin);
	rot.transform(_direction);
	_direction.normalize();
}

void Ray::translate(const Vector3d &v) {
	_origin += v;
}


bool Ray::intersectAABB(const AABB &aabb) const {
	Vector3d dirFrac;
	dirFrac.x() = 1.0f / _direction.x();
	dirFrac.y() = 1.0f / _direction.y();
	dirFrac.z() = 1.0f / _direction.z();

	float t1 = (aabb.getMin().x() - _origin.x()) * dirFrac.x();
	float t2 = (aabb.getMax().x() - _origin.x()) * dirFrac.x();
	float t3 = (aabb.getMin().y() - _origin.y()) * dirFrac.y();
	float t4 = (aabb.getMax().y() - _origin.y()) * dirFrac.y();
	float t5 = (aabb.getMin().z() - _origin.z()) * dirFrac.z();
	float t6 = (aabb.getMax().z() - _origin.z()) * dirFrac.z();

	float tMin = MAX(MAX(MIN(t1, t2), MIN(t3, t4)), MIN(t5, t6));
	float tMax = MIN(MIN(MAX(t1, t2), MAX(t3, t4)), MAX(t5, t6));

	// If tMax < 0, the ray is intersecting the AABB, but the whole AABB is in the opposite direction
	if (tMax < 0) {
		return false;
	}

	// If tMin > tMax, the ray doesn't intersect the AABB
	if (tMin > tMax) {
		return false;
	}

	return true;
}

}
