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

#include "common/streamdebug.h"
#include "math/vector3d.h"

namespace Math {

Vector3d::Matrix() :
	MatrixType<3, 1>() {

}

Vector3d::Matrix(float lx, float ly, float lz) :
	MatrixType<3, 1>() {
	x() = lx;
	y() = ly;
	z() = lz;
}

Vector3d::Matrix(const MatrixBase<3, 1> &vec) :
	MatrixType<3, 1>(vec) {
}

Vector3d::Matrix(const float *data) :
	MatrixType<3, 1>(data) {
}

void Vector3d::set(float lx, float ly, float lz) {
	x() = lx; y() = ly; z() = lz;
}

// Get the angle a vector is around the unit circle
// (ignores z-component)
Angle Vector3d::unitCircleAngle() const {
	return Angle::arcTangent2(y(), x());
}

} // end of namespace Math
