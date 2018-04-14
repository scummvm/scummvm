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

#ifndef MATH_VECTOR4D_H
#define MATH_VECTOR4D_H

#include "common/scummsys.h"
#include "common/endian.h"

#include "math/vector.h"
#include "math/angle.h"

namespace Math {

typedef Matrix<4, 1> Vector4d;

template<>
class Matrix<4, 1> : public MatrixType<4, 1> {
public:
	float& x() { return value(0); }
	float x() const { return value(0); }
	float& y() { return value(1); }
	float y() const { return value(1); }
	float& z() { return value(2); }
	float z() const { return value(2); }
	float& w() { return value(3); }
	float w() const { return value(3); }

	Matrix();
	Matrix(float lx, float ly, float lz, float lw);
	Matrix(const MatrixBase<4, 1> &m);
	Matrix(const float *data);

	void set(float lx, float ly, float lz, float lw);

};

} // end of namespace Math

#endif
