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

#include "math/aabb.h"

namespace Math {

AABB::AABB() : _valid(false) {
}

AABB::AABB(const Math::Vector3d &min, const Math::Vector3d &max) : _valid(true), _min(min), _max(max) {
}

void AABB::reset() {
	_valid = false;
}

void AABB::expand(const Math::Vector3d &v) {
	if (!_valid) {
		_min = v;
		_max = v;
		_valid = true;
	} else {
		if (v.x() < _min.x())
			_min.x() = v.x();
		if (v.y() < _min.y())
			_min.y() = v.y();
		if (v.z() < _min.z())
			_min.z() = v.z();

		if (v.x() > _max.x())
			_max.x() = v.x();
		if (v.y() > _max.y())
			_max.y() = v.y();
		if (v.z() > _max.z())
			_max.z() = v.z();
	}
}

void AABB::transform(const Math::Matrix4 &matrix) {
	Math::Vector3d min = _min;
	Math::Vector3d max = _max;

	reset();

	Math::Vector3d verts[8];
	verts[0].set(min.x(), min.y(), min.z());
	verts[1].set(max.x(), min.y(), min.z());
	verts[2].set(min.x(), max.y(), min.z());
	verts[3].set(min.x(), min.y(), max.z());
	verts[4].set(max.x(), max.y(), min.z());
	verts[5].set(max.x(), min.y(), max.z());
	verts[6].set(min.x(), max.y(), max.z());
	verts[7].set(max.x(), max.y(), max.z());

	for (int i = 0; i < 8; ++i) {
		matrix.transform(&verts[i], true);
		expand(verts[i]);
	}
}

bool AABB::collides(const AABB &aabb) const {
	return (getMax().x() > aabb.getMin().x() &&
			getMin().x() < aabb.getMax().x() &&
			getMax().y() > aabb.getMin().y() &&
			getMin().y() < aabb.getMax().y() &&
			getMax().z() > aabb.getMin().z() &&
			getMin().z() < aabb.getMax().z());
}

Math::Vector3d AABB::distance(const Math::Vector3d &point) const {
	double dx = MAX(getMin().x() - point.x(), point.x() - getMax().x());
	dx = MAX(dx, 0.0);

	double dy = MAX(getMin().y() - point.y(), point.y() - getMax().y());
	dy = MAX(dy, 0.0);

	double dz = MAX(getMin().z() - point.z(), point.z() - getMax().z());
	dz = MAX(dz, 0.0);

	return Math::Vector3d(dx, dy, dz);
}

}
