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

#include "math/frustum.h"

namespace Math {

Frustum::Frustum() {
}

void Frustum::setup(const Math::Matrix4 &matrix) {
	// Based on "Fast Extraction of Viewing Frustum Planes from the
	// World-View-Projection matrix" by Gil Gribb and Klaus Hartmann.
	// http://www.cs.otago.ac.nz/postgrads/alexis/planeExtraction.pdf

	_planes[0]._normal.x() = matrix.getValue(3, 0) + matrix.getValue(0, 0);
	_planes[0]._normal.y() = matrix.getValue(3, 1) + matrix.getValue(0, 1);
	_planes[0]._normal.z() = matrix.getValue(3, 2) + matrix.getValue(0, 2);
	_planes[0]._d = matrix.getValue(3, 3) + matrix.getValue(0, 3);

	_planes[1]._normal.x() = matrix.getValue(3, 0) - matrix.getValue(0, 0);
	_planes[1]._normal.y() = matrix.getValue(3, 1) - matrix.getValue(0, 1);
	_planes[1]._normal.z() = matrix.getValue(3, 2) - matrix.getValue(0, 2);
	_planes[1]._d = matrix.getValue(3, 3) - matrix.getValue(0, 3);

	_planes[2]._normal.x() = matrix.getValue(3, 0) - matrix.getValue(1, 0);
	_planes[2]._normal.y() = matrix.getValue(3, 1) - matrix.getValue(1, 1);
	_planes[2]._normal.z() = matrix.getValue(3, 2) - matrix.getValue(1, 2);
	_planes[2]._d = matrix.getValue(3, 3) - matrix.getValue(1, 3);

	_planes[3]._normal.x() = matrix.getValue(3, 0) + matrix.getValue(1, 0);
	_planes[3]._normal.y() = matrix.getValue(3, 1) + matrix.getValue(1, 1);
	_planes[3]._normal.z() = matrix.getValue(3, 2) + matrix.getValue(1, 2);
	_planes[3]._d = matrix.getValue(3, 3) + matrix.getValue(1, 3);

	_planes[4]._normal.x() = matrix.getValue(3, 0) + matrix.getValue(2, 0);
	_planes[4]._normal.y() = matrix.getValue(3, 1) + matrix.getValue(2, 1);
	_planes[4]._normal.z() = matrix.getValue(3, 2) + matrix.getValue(2, 2);
	_planes[4]._d = matrix.getValue(3, 3) + matrix.getValue(2, 3);

	_planes[5]._normal.x() = matrix.getValue(3, 0) - matrix.getValue(2, 0);
	_planes[5]._normal.y() = matrix.getValue(3, 1) - matrix.getValue(2, 1);
	_planes[5]._normal.z() = matrix.getValue(3, 2) - matrix.getValue(2, 2);
	_planes[5]._d = matrix.getValue(3, 3) - matrix.getValue(2, 3);
	
	for (int i = 0; i < 6; ++i) {
		_planes[i].normalize();
	}
}

bool Frustum::isInside(const Math::AABB &aabb) const {
	Math::Vector3d min = aabb.getMin();
	Math::Vector3d max = aabb.getMax();
	
	for (int i = 0; i < 6; ++i) {
		const Plane &plane = _planes[i];
		Math::Vector3d positive = min;

		if (plane._normal.x() >= 0.0f)
			positive.x() = max.x();
		if (plane._normal.y() >= 0.0f)
			positive.y() = max.y();
		if (plane._normal.z() >= 0.0f)
			positive.z() = max.z();

		float dist = _planes[i].getSignedDistance(positive);
		if (dist < 0.0f)
			return false;
	}

	return true;
}

}
