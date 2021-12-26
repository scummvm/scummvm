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

#include "common/streamdebug.h"
#include "math/vector3d.h"
#include "math/vector4d.h"

namespace Math {

Vector4d::Matrix() :
	MatrixType<4, 1>() {
}

Vector4d::Matrix(float lx, float ly, float lz, float lw) :
	MatrixType<4, 1>() {
	x() = lx;
	y() = ly;
	z() = lz;
	w() = lw;
}

Vector4d::Matrix(const MatrixBase<4, 1> &vec) :
	MatrixType<4, 1>(vec) {
}

Vector4d::Matrix(const float *data) :
	MatrixType<4, 1>(data) {
}

void Vector4d::set(float lx, float ly, float lz, float lw) {
	x() = lx; y() = ly; z() = lz; w() = lw;
}

Vector3d Vector4d::getXYZ() const {
	Vector3d v(value(0), value(1), value(2));
	return v;
}

} // end of namespace Math
