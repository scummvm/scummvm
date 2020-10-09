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

#ifndef MATH_VECTOR2D_H
#define MATH_VECTOR2D_H

#include "math/vector.h"
#include "math/vector3d.h"

namespace Math {

template<>
class Matrix<2, 1> : public MatrixType<2, 1> {
public:
	Matrix();
	Matrix(float x, float y);
	Matrix(const MatrixBase<2, 1> &vec);
	Matrix(const float *data);

	inline float getX() const { return getValue(0); }
	inline float getY() const { return getValue(1); }
	inline void setX(float x) { setValue(0, x); }
	inline void setY(float y) { setValue(1, y); }

	void rotateAround(const Vector(2) &point, const Angle &angle);
	Angle getAngle() const;

	Vector3d toVector3d() const;
};

typedef Matrix<2, 1> Vector2d;

}

#endif
