/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
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

#ifndef BLADERUNNER_MATRIX_H
#define BLADERUNNER_MATRIX_H

#include "bladerunner/vector.h"

namespace BladeRunner {

class Matrix3x2 {
public:
	float _m[2][3];

	Matrix3x2();
	Matrix3x2(float d[6]);
	Matrix3x2(
		float m00, float m01, float m02,
		float m10, float m11, float m12);

	      float &operator()(int r, int c)       { assert(r >= 0 && r < 2); assert(c >= 0 && c < 3); return _m[r][c]; }
	const float &operator()(int r, int c) const { assert(r >= 0 && r < 2); assert(c >= 0 && c < 3); return _m[r][c]; }
};

inline Matrix3x2 operator*(const Matrix3x2 &a, const Matrix3x2 &b) {
	Matrix3x2 t;

	t(0, 0) = a(0, 0) * b(0, 0) + a(0, 1) * b(1, 0);
	t(0, 1) = a(0, 0) * b(0, 1) + a(0, 1) * b(1, 1);
	t(0, 2) = a(0, 0) * b(0, 2) + a(0, 1) * b(1, 2) + a(0, 2);
	t(1, 0) = a(1, 0) * b(0, 0) + a(1, 1) * b(1, 0);
	t(1, 1) = a(1, 0) * b(0, 1) + a(1, 1) * b(1, 1);
	t(1, 2) = a(1, 0) * b(0, 2) + a(1, 1) * b(1, 2) + a(1, 2);

	return t;
}

inline Matrix3x2 operator+(const Matrix3x2 &a, Vector2 b) {
	Matrix3x2 t(a);

	t(0, 2) += b.x;
	t(1, 2) += b.y;

	return t;
}

inline Vector2 operator*(const Matrix3x2 &a, Vector2 b) {
	Vector2 t;

	t.x = a(0, 0) * b.x + a(0, 1) * b.y + a(0, 2);
	t.y = a(1, 0) * b.x + a(1, 1) * b.y + a(1, 2);

	return t;
}

class Matrix4x3 {
public:
	float _m[3][4];

	Matrix4x3();
	Matrix4x3(float d[12]);
	Matrix4x3(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23);

	      float &operator()(int r, int c)       { assert(r >= 0 && r < 3); assert(c >= 0 && c < 4); return _m[r][c]; }
	const float &operator()(int r, int c) const { assert(r >= 0 && r < 3); assert(c >= 0 && c < 4); return _m[r][c]; }

	void unknown();
};

Matrix4x3 invertMatrix(const Matrix4x3 &m);
Matrix4x3 rotationMatrixX(float angle);

inline Matrix4x3 operator*(const Matrix4x3 &a, const Matrix4x3 &b) {
	Matrix4x3 t;

	for (int i = 0; i !=3; ++i) {
		t(i, 0) = a(i, 0) * b(0, 0) + a(i, 1) * b(1, 0) + a(i, 2) * b(2, 0);
		t(i, 1) = a(i, 0) * b(0, 1) + a(i, 1) * b(1, 1) + a(i, 2) * b(2, 1);
		t(i, 2) = a(i, 0) * b(0, 2) + a(i, 1) * b(1, 2) + a(i, 2) * b(2, 2);
		t(i, 3) = a(i, 0) * b(0, 3) + a(i, 1) * b(1, 3) + a(i, 2) * b(2, 3) + a(i, 3);
	}

	return t;
}

inline Vector3 operator*(const Matrix4x3 &m, const Vector3 &v) {
	Vector3 r;

	r.x = m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z + m(0, 3);
	r.y = m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z + m(1, 3);
	r.z = m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z + m(2, 3);

	return r;
}

} // End of namespace BladeRunner

#endif
