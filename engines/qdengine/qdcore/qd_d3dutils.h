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

#ifndef QDENGINE_QDCORE_QD_ENGINE_D3DUTILS_H
#define QDENGINE_QDCORE_QD_ENGINE_D3DUTILS_H

#include "qdengine/xmath.h"

namespace QDEngine {

typedef float VALUE3D;
typedef struct _MATRIX3D {
	VALUE3D m[4][4];

	_MATRIX3D() { }
	_MATRIX3D(VALUE3D _m00, VALUE3D _m01, VALUE3D _m02, VALUE3D _m03,
	          VALUE3D _m10, VALUE3D _m11, VALUE3D _m12, VALUE3D _m13,
	          VALUE3D _m20, VALUE3D _m21, VALUE3D _m22, VALUE3D _m23,
	          VALUE3D _m30, VALUE3D _m31, VALUE3D _m32, VALUE3D _m33
	         ) {
		m[0][0] = _m00;
		m[0][1] = _m01;
		m[0][2] = _m02;
		m[0][3] = _m03;
		m[1][0] = _m10;
		m[1][1] = _m11;
		m[1][2] = _m12;
		m[1][3] = _m13;
		m[2][0] = _m20;
		m[2][1] = _m21;
		m[2][2] = _m22;
		m[2][3] = _m23;
		m[3][0] = _m30;
		m[3][1] = _m31;
		m[3][2] = _m32;
		m[3][3] = _m33;
	}

	VALUE3D &operator()(int iRow, int iColumn) {
		return m[iRow][iColumn];
	}
	const VALUE3D &operator()(int iRow, int iColumn) const {
		return m[iRow][iColumn];
	}
} MATRIX3D, *LPDMATRIX3D;

/*
**-----------------------------------------------------------------------------
** Function Prototypes
**-----------------------------------------------------------------------------
*/

// generic simple matrix routines
MATRIX3D ZeroMatrix();
MATRIX3D IdentityMatrix();

MATRIX3D ProjectionMatrix(const float near_plane, const float far_plane, const float fov);
MATRIX3D ViewMatrixByDir(const Vect3f &from,
                         const Vect3f &view_dir,
                         const Vect3f &world_up,
                         const Vect3f &cam_up);
MATRIX3D ViewMatrix(const Vect3f &from, const Vect3f &at,
                    const Vect3f &world_up,
                    const Vect3f &cam_up);

MATRIX3D RotateXMatrix(const float rads);
MATRIX3D RotateYMatrix(const float rads);
MATRIX3D RotateZMatrix(const float rads);
MATRIX3D TranslateMatrix(const float dx, const float dy, const float dz);
MATRIX3D TranslateMatrix(const Vect3f &v);
MATRIX3D ScaleMatrix(const float size);
MATRIX3D ScaleMatrix(const float a, const float b, const float c);
MATRIX3D ScaleMatrix(const Vect3f &v);

MATRIX3D MatrixMult(const MATRIX3D &a, const MATRIX3D &b);
MATRIX3D MatrixInverse(const MATRIX3D &m);
MATRIX3D MatrixTranspose(const MATRIX3D &m);

Vect3f TransformVector(const Vect3f &v, const MATRIX3D &m);
Vect3f TransformNormal(const Vect3f &v, const MATRIX3D &m);

/*
**-----------------------------------------------------------------------------
**  End of File
**-----------------------------------------------------------------------------
*/

} // namespace QDEngine

#endif // QDENGINE_QDCORE_QD_ENGINE_D3DUTILS_H
