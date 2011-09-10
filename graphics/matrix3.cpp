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

#include "graphics/matrix3.h"

namespace Graphics {

Matrix3x3::Matrix3x3() :
	Matrix(), Rotation3D(this) {

}

void Matrix3x3::transform(Vector3d *v) const {

	Matrix<3, 1> m;
	m(0, 0) = v->x();
	m(1, 0) = v->y();
	m(2, 0) = v->z();

	m = *this * m;

	v->set(m(0, 0), m(1, 0), m(2, 0));
}

#define DEGTORAD(a) (a * LOCAL_PI / 180.0)
#define RADTODEG(a) (a * 180.0 / LOCAL_PI)

float RadianToDegree(float rad) {
	return (float)RADTODEG(rad);
}

float DegreeToRadian(float degrees) {
	return (float)DEGTORAD(degrees);
}

} // end of namespace Graphics
