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
