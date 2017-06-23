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

#include "bladerunner/matrix.h"

#include "common/scummsys.h"

namespace BladeRunner {

Matrix3x2::Matrix3x2() {
	for (int r = 0; r != 2; ++r)
		for (int c = 0; c != 3; ++c)
			_m[r][c] = (r == c) ? 1.0f : 0.0f;
}

Matrix3x2::Matrix3x2(float d[6]) {
	for (int r = 0; r != 2; ++r)
		for (int c = 0; c != 3; ++c)
			_m[r][c] = d[r*3+c];
}

Matrix3x2::Matrix3x2(
	float m00, float m01, float m02,
	float m10, float m11, float m12) {
	_m[0][0] = m00;
	_m[0][1] = m01;
	_m[0][2] = m02;
	_m[1][0] = m10;
	_m[1][1] = m11;
	_m[1][2] = m12;
}

Matrix4x3::Matrix4x3() {
	for (int r = 0; r != 3; ++r)
		for (int c = 0; c != 4; ++c)
			_m[r][c] = (r == c) ? 1.0f : 0.0f;
}

Matrix4x3::Matrix4x3(float d[12]) {
	for (int r = 0; r != 3; ++r)
		for (int c = 0; c != 4; ++c)
			_m[r][c] = d[r*4+c];
}

Matrix4x3::Matrix4x3(
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23) {
	_m[0][0] = m00;
	_m[0][1] = m01;
	_m[0][2] = m02;
	_m[0][3] = m03;
	_m[1][0] = m10;
	_m[1][1] = m11;
	_m[1][2] = m12;
	_m[1][3] = m13;
	_m[2][0] = m20;
	_m[2][1] = m21;
	_m[2][2] = m22;
	_m[2][3] = m23;
}

Matrix4x3 rotationMatrixX(float angle) {
	float ca = cos(angle);
	float sa = sin(angle);

	return Matrix4x3( 1.0f, 0.0f, 0.0f, 0.0f,
	                  0.0f,   ca,   sa, 0.0f,
	                  0.0f,  -sa,   ca, 0.0f );
}

static inline void swapRows(double *r1, double *r2) {
	for (int c = 0; c != 8; ++c) {
		double t = r1[c];
		r1[c] = r2[c];
		r2[c] = t;
	}
}

static inline void subtractRow(double *r1, double factor, double *r2) {
	for (int c = 0; c != 8; ++c)
		r1[c] -= factor * r2[c];
}

static inline void divideRow(double *r1, double d) {
	for (int c = 0; c != 8; ++c)
		r1[c] /= d;
}

Matrix4x3 invertMatrix(const Matrix4x3 &m) {
	double w[3][8];

	for (int r = 0; r != 3; ++r) {
		for (int c = 0; c != 4; ++c) {
			w[r][c] = m(r, c);
			w[r][c+4] = (r == c) ? 1.0 : 0.0;
		}
	}

	if (w[0][0] == 0.0) {
		if (w[1][0] != 0.0)
			swapRows(w[0], w[1]);
		else
			swapRows(w[0], w[2]);
	}
	divideRow(w[0], w[0][0]);
	subtractRow(w[1], w[1][0], w[0]);
	subtractRow(w[2], w[2][0], w[0]);

	if (w[1][1] == 0.0)
		swapRows(w[1], w[2]);

	divideRow(w[1], w[1][1]);
	subtractRow(w[0], w[0][1], w[1]);
	subtractRow(w[2], w[2][1], w[1]);

	divideRow(w[2], w[2][2]);
	subtractRow(w[0], w[0][2], w[2]);
	subtractRow(w[1], w[1][2], w[2]);

	for (int r = 0; r != 3; ++r) {
		w[r][7] = -w[r][3];
		w[r][3] = 0.0;
	}

	Matrix4x3 result;

	for (int r = 0; r != 3; ++r)
		for (int c = 0; c != 4; ++c)
			result(r, c) = float(w[r][c+4]);

	return result;
}

void Matrix4x3::unknown() {
	Matrix4x3 t;

	// Transpose the 3x3 top left submatrix
	for (int r = 0; r != 3; ++r)
		for (int c = 0; c != 3; ++c)
			t(r, c) = _m[c][r];

	t(0,3) = -(_m[0][3] * _m[0][0] + _m[1][3] * _m[1][0] + _m[2][3] * _m[2][0]);
	t(1,3) = -(_m[0][3] * _m[0][1] + _m[1][3] * _m[1][1] + _m[2][3] * _m[2][1]);
	t(2,3) = -(_m[0][3] * _m[0][2] + _m[1][3] * _m[1][2] + _m[2][3] * _m[2][2]);

	*this = t;
}

} // End of namespace BladeRunner
