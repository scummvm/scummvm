/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#ifndef MATH_MATRIX4_H
#define MATH_MATRIX4_H

#include "math/matrix3.h"

namespace Math {

// matrix 4 is a rotation matrix + position
class Matrix4 : public Matrix<4, 4>, public Rotation3D<Matrix4> {
public:
	Matrix4();

	void transform(Vector3d *v, bool translate) const;

	Vector3d getPosition() const;
	void setPosition(const Vector3d &v);

	void translate(const Vector3d &v);

};

} // end of namespace Math

#endif
