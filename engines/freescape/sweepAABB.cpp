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

#include "freescape/freescape.h"

namespace Freescape {

/*
This code is inspired by the Luis Eduard Reis implementation:
https://luisreis.net/blog/aabb_collision_handling/
*/

float lineToPlane(Math::Vector3d const &p, Math::Vector3d const &u,  Math::Vector3d const &v, Math::Vector3d const &n) {
	float NdotU = n.dotProduct(u);
	if (NdotU == 0)
		return INFINITY;

	return n.dotProduct(v - p) / NdotU;
}

bool between(float x, float a, float b) {
	return x >= a && x <= b;
}

float sweepAABB(Math::AABB const &a, Math::AABB const &b, Math::Vector3d const &direction, Math::Vector3d &normal) {
	Math::Vector3d m = b.getMin() - a.getMax();
	Math::Vector3d mh = a.getSize() + b.getSize();

	float h = 1.0;
	float s = 0.0;
	Math::Vector3d zero;

	// X min
	s = lineToPlane(zero, direction, m, Math::Vector3d(-1, 0, 0));
	if (s >= 0 && direction.x() > 0 && s < h && between(s * direction.y(), m.y(), m.y() + mh.y()) && between(s * direction.z(), m.z(), m.z() + mh.z())) {
		h = s;
		normal = Math::Vector3d(-1, 0, 0);
	}

	// X max
	m.x() = m.x() + mh.x();
	s = lineToPlane(zero, direction, m, Math::Vector3d(1, 0, 0));
	if (s >= 0 && direction.x() < 0 && s < h && between(s * direction.y(), m.y(), m.y() + mh.y()) && between(s * direction.z(), m.z(), m.z() + mh.z())) {
		h = s;
		normal = Math::Vector3d(1, 0, 0);
	}

	// Y min
	m.x() = m.x() - mh.x();
	s = lineToPlane(zero, direction, m, Math::Vector3d(0, -1, 0));
	if (s >= 0 && direction.y() > 0 && s < h && between(s * direction.x(), m.x(), m.x() + mh.x()) && between(s * direction.z(), m.z(), m.z() + mh.z())) {
		h = s;
		normal = Math::Vector3d(0, -1, 0);
	}

	// Y max
	m.y() = m.y() + mh.y();
	s = lineToPlane(zero, direction, m, Math::Vector3d(0, 1, 0));
	if (s >= 0 && direction.y() < 0 && s < h && between(s * direction.x(), m.x(), m.x() + mh.x()) && between(s * direction.z(), m.z(), m.z() + mh.z())) {
		h = s;
		normal = Math::Vector3d(0, 1, 0);
	}


	// Z min
	m.y() = m.y() - mh.y();
	s = lineToPlane(zero, direction, m, Math::Vector3d(0, 0, -1));
	if (s >= 0 && direction.z() > 0 && s < h && between(s * direction.x(), m.x(), m.x() + mh.x()) && between(s * direction.y(), m.y(), m.y() + mh.y())) {
		h = s;
		normal = Math::Vector3d(0, 0, -1);
	}

	// Z max
	m.z() = m.z() + mh.z();
	s = lineToPlane(zero, direction, m, Math::Vector3d(0, 0, 1));
	if (s >= 0 && direction.z() < 0 && s < h && between(s * direction.x(), m.x(), m.x() + mh.x()) && between(s * direction.y(), m.y(), m.y() + mh.y())) {
		h = s;
		normal = Math::Vector3d(0, 0, 1);
	}

	return h;
}

}
