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

#include "math/plane.h"

namespace Math {

Plane::Plane() : _d(0.0f) {
}

Plane::Plane(const Math::Vector3d &normal, float d) : _normal(normal), _d(d) {
}

float Plane::getSignedDistance(const Math::Vector3d &p) const {
	return _normal.x() * p.x() + _normal.y() * p.y() + _normal.z() * p.z() + _d;
}

void Plane::normalize() {
	float mag = _normal.getMagnitude();
	if (mag > 0.0f) {
		_normal /= mag;
		_d /= mag;
	}
}

}
