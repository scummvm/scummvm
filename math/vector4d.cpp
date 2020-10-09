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

} // end of namespace Math
