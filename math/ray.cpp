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

void Ray::rotateDirection(const Quaternion &rot) {
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

// Algorithm adapted from https://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
bool Ray::intersectTriangle(const Vector3d &v0, const Vector3d &v1,
		const Vector3d &v2, Vector3d &vout, float &fout) const {
	const Vector3d e1 = v1 - v0;
	const Vector3d e2 = v2 - v0;
	const Vector3d h = Vector3d::crossProduct(_direction, e2);

	float a = e1.dotProduct(h);
	if (fabs(a) < 1e-6f)
		return false;

	float f = 1.0f / a;
	const Vector3d s = _origin - v0;
	float u = f * s.dotProduct(h);
	if (u < 0.0f || u > 1.0f)
		return false;

	const Vector3d q = Vector3d::crossProduct(s, e1);
	float v = f * _direction.dotProduct(q);

	if (v < 0.0f || u + v > 1.0f)
		return false;

	float t = f * e2.dotProduct(q);

	if (t < 1e-6f)
		return false;

	fout = t;
	vout = _origin + t * _direction;

	return true;
}

}
