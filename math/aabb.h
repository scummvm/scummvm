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

#ifndef MATH_AABB_H
#define MATH_AABB_H

#include "math/vector3d.h"
#include "math/matrix4.h"

namespace Math {

// Axis-aligned bounding box
class AABB {
public:
	AABB();
	AABB(const Math::Vector3d &min, const Math::Vector3d &max);

	void reset();
	void expand(const Math::Vector3d &v);
	void transform(const Math::Matrix4 &matrix);
	Math::Vector3d getMin() const { return _min; }
	Math::Vector3d getMax() const { return _max; }
	bool isValid() const { return _valid; }

private:
	Math::Vector3d _min, _max;
	bool _valid;
};

} // end of namespace Math

#endif
