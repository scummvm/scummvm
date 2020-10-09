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

}
