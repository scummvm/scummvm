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

#include "tetraedge/tetraedge.h"
#include "common/str-array.h"
#include "math/matrix4.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_quaternion.h"

namespace Tetraedge {

bool TeVector3f32::parse(const Common::String &val) {
	const Common::StringArray parts = TetraedgeEngine::splitString(val, ',');
	if (parts.size() != 3)
		return false;
	x() = atof(parts[0].c_str());
	y() = atof(parts[1].c_str());
	z() = atof(parts[2].c_str());
	return true;
}

void TeVector3f32::rotate(const TeQuaternion &rot) {
	const TeMatrix4x4 matrix = rot.toTeMatrix();
	*this = (matrix * *this);
}

TeVector3f32 operator^(const TeVector3f32 &left, const TeVector3f32 &right) {
	TeVector3f32 retval;
	float rx = right.x();
	float ry = right.y();
	float rz = right.z();
	float lx = left.x();
	float ly = left.y();
	float lz = left.z();
	retval.x() = ly * rz - lz * ry;
	retval.y() = lz * rx - lx * rz;
	retval.z() = lx * ry - ly * rx;
	return retval;
}



} // end namespace Tetraedge
