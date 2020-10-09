/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */

#include "common/scummsys.h"

#include "graphics/tinygl/zmath.h"

namespace TinyGL {

// Inversion of a 4x4 matrix.
// It's not just unrolling, this is a different implementation that directly
// uses the formula whereas the previous one is using another method (which is generic and thus, slower) 
int MatrixInverse(float *m) {
	double inv[16];

	inv[0] = m[5]  * m[10] * m[15] - 
		m[5]  * m[11] * m[14] - 
		m[9]  * m[6]  * m[15] + 
		m[9]  * m[7]  * m[14] +
		m[13] * m[6]  * m[11] - 
		m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] + 
		m[4]  * m[11] * m[14] + 
		m[8]  * m[6]  * m[15] - 
		m[8]  * m[7]  * m[14] - 
		m[12] * m[6]  * m[11] + 
		m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9] * m[15] - 
		m[4]  * m[11] * m[13] - 
		m[8]  * m[5] * m[15] + 
		m[8]  * m[7] * m[13] + 
		m[12] * m[5] * m[11] - 
		m[12] * m[7] * m[9];

	inv[12] = -m[4]  * m[9] * m[14] + 
		m[4]  * m[10] * m[13] +
		m[8]  * m[5] * m[14] - 
		m[8]  * m[6] * m[13] - 
		m[12] * m[5] * m[10] + 
		m[12] * m[6] * m[9];

	inv[1] = -m[1]  * m[10] * m[15] + 
		m[1]  * m[11] * m[14] + 
		m[9]  * m[2] * m[15] - 
		m[9]  * m[3] * m[14] - 
		m[13] * m[2] * m[11] + 
		m[13] * m[3] * m[10];

	inv[5] = m[0]  * m[10] * m[15] - 
		m[0]  * m[11] * m[14] - 
		m[8]  * m[2] * m[15] + 
		m[8]  * m[3] * m[14] + 
		m[12] * m[2] * m[11] - 
		m[12] * m[3] * m[10];

	inv[9] = -m[0]  * m[9] * m[15] + 
		m[0]  * m[11] * m[13] + 
		m[8]  * m[1] * m[15] - 
		m[8]  * m[3] * m[13] - 
		m[12] * m[1] * m[11] + 
		m[12] * m[3] * m[9];

	inv[13] = m[0]  * m[9] * m[14] - 
		m[0]  * m[10] * m[13] - 
		m[8]  * m[1] * m[14] + 
		m[8]  * m[2] * m[13] + 
		m[12] * m[1] * m[10] - 
		m[12] * m[2] * m[9];

	inv[2] = m[1]  * m[6] * m[15] - 
		m[1]  * m[7] * m[14] - 
		m[5]  * m[2] * m[15] + 
		m[5]  * m[3] * m[14] + 
		m[13] * m[2] * m[7] - 
		m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] + 
		m[0]  * m[7] * m[14] + 
		m[4]  * m[2] * m[15] - 
		m[4]  * m[3] * m[14] - 
		m[12] * m[2] * m[7] + 
		m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] - 
		m[0]  * m[7] * m[13] - 
		m[4]  * m[1] * m[15] + 
		m[4]  * m[3] * m[13] + 
		m[12] * m[1] * m[7] - 
		m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] + 
		m[0]  * m[6] * m[13] + 
		m[4]  * m[1] * m[14] - 
		m[4]  * m[2] * m[13] - 
		m[12] * m[1] * m[6] + 
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] + 
		m[1] * m[7] * m[10] + 
		m[5] * m[2] * m[11] - 
		m[5] * m[3] * m[10] - 
		m[9] * m[2] * m[7] + 
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] - 
		m[0] * m[7] * m[10] - 
		m[4] * m[2] * m[11] + 
		m[4] * m[3] * m[10] + 
		m[8] * m[2] * m[7] - 
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] + 
		m[0] * m[7] * m[9] + 
		m[4] * m[1] * m[11] - 
		m[4] * m[3] * m[9] - 
		m[8] * m[1] * m[7] + 
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] - 
		m[0] * m[6] * m[9] - 
		m[4] * m[1] * m[10] + 
		m[4] * m[2] * m[9] + 
		m[8] * m[1] * m[6] - 
		m[8] * m[2] * m[5];

	double det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (int i = 0; i < 16; i++) {
		m[i] = inv[i] * det;
	}
	return true;
}

void Vector3::normalize() {
	float n = sqrt(X * X + Y * Y + Z * Z);
	if (n != 0) {
		X /= n;
		Y /= n;
		Z /= n;
	}
}

Vector4::Vector4(const Vector3 &vec, float w) {
	X = vec.X;
	Y = vec.Y;
	Z = vec.Z;
	W = w;
}

void Matrix4::identity() {
	memset(_m, 0, sizeof(_m));
	_m[0][0] = 1.0f;
	_m[1][1] = 1.0f;
	_m[2][2] = 1.0f;
	_m[3][3] = 1.0f;
}

Matrix4 Matrix4::transpose() const {
	Matrix4 a;

	a._m[0][0] = this->_m[0][0];
	a._m[0][1] = this->_m[1][0];
	a._m[0][2] = this->_m[2][0];
	a._m[0][3] = this->_m[3][0];

	a._m[1][0] = this->_m[0][1];
	a._m[1][1] = this->_m[1][1];
	a._m[1][2] = this->_m[2][1];
	a._m[1][3] = this->_m[3][1];

	a._m[2][0] = this->_m[0][2];
	a._m[2][1] = this->_m[1][2];
	a._m[2][2] = this->_m[2][2];
	a._m[2][3] = this->_m[3][2];

	a._m[3][0] = this->_m[0][3];
	a._m[3][1] = this->_m[1][3];
	a._m[3][2] = this->_m[2][3];
	a._m[3][3] = this->_m[3][3];

	return a;
}

void Matrix4::transpose() {
	Matrix4 tmp = *this;
	this->_m[0][0] = tmp._m[0][0];
	this->_m[0][1] = tmp._m[1][0];
	this->_m[0][2] = tmp._m[2][0];
	this->_m[0][3] = tmp._m[3][0];

	this->_m[1][0] = tmp._m[0][1];
	this->_m[1][1] = tmp._m[1][1];
	this->_m[1][2] = tmp._m[2][1];
	this->_m[1][3] = tmp._m[3][1];

	this->_m[2][0] = tmp._m[0][2];
	this->_m[2][1] = tmp._m[1][2];
	this->_m[2][2] = tmp._m[2][2];
	this->_m[2][3] = tmp._m[3][2];

	this->_m[3][0] = tmp._m[0][3];
	this->_m[3][1] = tmp._m[1][3];
	this->_m[3][2] = tmp._m[2][3];
	this->_m[3][3] = tmp._m[3][3];
}

Matrix4 Matrix4::inverseOrtho() const {
	Matrix4 a;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			a._m[i][j] = this->_m[j][i];
		}
	}
	a._m[3][0] = 0.0f;
	a._m[3][1] = 0.0f;
	a._m[3][2] = 0.0f;
	a._m[3][3] = 1.0f;

	for (int i = 0; i < 3; i++) {
		float s = 0;
		for (int j = 0; j < 3; j++) {
			s -= this->_m[j][i] * this->_m[j][3];
		}
		a._m[i][3] = s;
	}

	return a;
}

Matrix4 Matrix4::inverse() const {
	Matrix4 result = *this;
	MatrixInverse((float *)result._m);

	return result;
}

void Matrix4::rotation(float t, int u) {
	float s, c;
	int v, w;

	identity();

	if ((v = u + 1) > 2)
		v = 0;
	if ((w = v + 1) > 2)
		w = 0;
	s = sin(t);
	c = cos(t);
	_m[v][v] = c;
	_m[v][w] = -s;
	_m[w][v] = s;
	_m[w][w] = c;
}

bool Matrix4::isIdentity() const {
	//NOTE: This might need to be implemented in a fault-tolerant way.
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (i == j) {
				if (_m[i][j] != 1.0) {
					return false;
				}
			} else if (_m[i][j] != 0.0) {
				return false;
			}
		}
	}
	return true;
}

void Matrix4::invert() {
	MatrixInverse((float *)this->_m);
}

Matrix4 Matrix4::frustum(float left, float right, float bottom, float top, float nearp, float farp) {
	float x, y, A, B, C, D;

	x = (float)((2.0 * nearp) / (right - left));
	y = (float)((2.0 * nearp) / (top - bottom));
	A = (right + left) / (right - left);
	B = (top + bottom) / (top - bottom);
	C = -(farp + nearp) / (farp - nearp);
	D = (float)(-(2.0 * farp * nearp) / (farp - nearp));

	Matrix4 m;

	m._m[0][0] = x; m._m[0][1] = 0; m._m[0][2] = A; m._m[0][3] = 0;
	m._m[1][0] = 0; m._m[1][1] = y; m._m[1][2] = B; m._m[1][3] = 0;
	m._m[2][0] = 0; m._m[2][1] = 0; m._m[2][2] = C; m._m[2][3] = D;
	m._m[3][0] = 0; m._m[3][1] = 0; m._m[3][2] = -1; m._m[3][3] = 0;

	return m;
}

void Matrix4::translate(float x, float y, float z) {
	_m[0][3] += _m[0][0] * x + _m[0][1] * y + _m[0][2] * z;
	_m[1][3] += _m[1][0] * x + _m[1][1] * y + _m[1][2] * z;
	_m[2][3] += _m[2][0] * x + _m[2][1] * y + _m[2][2] * z;
	_m[3][3] += _m[3][0] * x + _m[3][1] * y + _m[3][2] * z;
}

void Matrix4::scale(float x, float y, float z) {
	_m[0][0] *= x; _m[0][1] *= y; _m[0][2] *= z;
	_m[1][0] *= x; _m[1][1] *= y; _m[1][2] *= z;
	_m[2][0] *= x; _m[2][1] *= y; _m[2][2] *= z;
	_m[3][0] *= x; _m[3][1] *= y; _m[3][2] *= z;
}

} // end of namespace TinyGL
