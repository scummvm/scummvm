/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRAPHICS_MATRIX4_H
#define GRAPHICS_MATRIX4_H

#include "graphics/matrix3.h"

namespace Graphics {

// matrix 4 is a rotation matrix + position
class Matrix4 {
public:
	Matrix3 _rot;
	Vector3d _pos;

	Matrix4();

	Matrix4& operator =(const Matrix4& s) {
		_pos = s._pos;
		_rot = s._rot;

		return *this;
	}

	Matrix4& operator *=(const Matrix4& s) {
		Vector3d v;

		v = s._pos;
		_rot.transform(&v);
		_pos += v;
		_rot *= s._rot;

		return *this;
	}

	void translate(float x, float y, float z);

private:
};

} // end of namespace Graphics

#endif
