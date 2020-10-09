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

#ifndef GRAPHICS_TINYGL_ZMATH_H
#define GRAPHICS_TINYGL_ZMATH_H

namespace TinyGL {

#define X _v[0]
#define Y _v[1]
#define Z _v[2]
#define W _v[3]

// Matrix & Vertex
class Vector3 {
public:
	Vector3() { }
	Vector3(float x, float y, float z) {
		X = x;
		Y = y;
		Z = z;
	}

	void normalize();

	float getLength() const { return sqrt(X * X + Y * Y + Z * Z); }

	bool operator==(const Vector3 &other) const {
		return X == other.X && Y == other.Y && Z == other.Z;
	}

	bool operator!=(const Vector3 &other) const {
		return X != other.X || Y != other.Y || Z != other.Z;
	}

	Vector3 operator-() const {
		return Vector3(-X, -Y, -Z);
	}

	Vector3 operator*(float factor) const {
		return Vector3(X * factor, Y * factor, Z * factor);
	}

	Vector3 operator+(const Vector3 &other) const {
		return Vector3(X + other.X, Y + other.Y, Z + other.Z);
	}

	Vector3 operator-(const Vector3 &other) const {
		return Vector3(X - other.X, Y - other.Y, Z - other.Z);
	}

	Vector3 &operator*=(float factor) {
		X *= factor;
		Y *= factor;
		Z *= factor;
		return *this;
	}

	Vector3 &operator+=(float value) {
		X += value;
		Y += value;
		Z += value;
		return *this;
	}

	Vector3 &operator-=(float value) {
		X -= value;
		Y -= value;
		Z -= value;
		return *this;
	}

	float _v[3];
};

class Vector4 {
public:
	Vector4() { }
	Vector4(const Vector3 &vec, float w);

	Vector4(float x, float y, float z, float w) {
		X = x;
		Y = y;
		Z = z;
		W = w;
	}

	bool operator==(const Vector4 &other) const {
		return X == other.X && Y == other.Y && Z == other.Z && W == other.W;
	}

	bool operator!=(const Vector4 &other) const {
		return X != other.X || Y != other.Y || Z != other.Z || W != other.W;
	}

	Vector4 operator-() const {
		return Vector4(-X, -Y, -Z, -W);
	}

	Vector4 operator*(float factor) const {
		return Vector4(X * factor, Y * factor, Z * factor,W * factor);
	}

	Vector4 operator+(const Vector4 &other) const {
		return Vector4(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
	}

	Vector4 operator-(const Vector4 &other) const {
		return Vector4(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
	}

	Vector4 &operator*=(float factor) {
		X *= factor;
		Y *= factor;
		Z *= factor;
		W *= factor;
		return *this;
	}

	Vector4 &operator+=(float value) {
		X += value;
		Y += value;
		Z += value;
		W += value;
		return *this;
	}

	Vector4 &operator-=(float value) {
		X -= value;
		Y -= value;
		Z -= value;
		W -= value;
		return *this;
	}

	float _v[4];
};

class Matrix4 {
public:
	Matrix4() { }

	bool isIdentity() const;

	inline Matrix4 operator+(const Matrix4 &b) const {
		Matrix4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result._m[i][j] = _m[i][j] + b._m[i][j];
			}
		}
		return result;
	}

	inline Matrix4 operator-(const Matrix4 &b) const {
		Matrix4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result._m[i][j] = _m[i][j] - b._m[i][j];
			}
		}
		return result;
	}

	inline Matrix4 operator*(const Matrix4 &b) const {
		Matrix4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				float s = 0.0;
				for (int k = 0; k < 4; k++)
					s += _m[i][k] * b._m[k][j];
				result._m[i][j] = s;
			}
		}
		return result;
	}

	inline Matrix4 &operator*=(const Matrix4 &b) {
		Matrix4 a = *this;
		float s;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				s = 0.0;
				for (int k = 0; k < 4; k++)
					s += a._m[i][k] * b._m[k][j];
				this->_m[i][j] = s;
			}
		}
		return *this;
	}

	void scale(float x, float y, float z);
	void translate(float x, float y, float z);
	void identity();
	void rotation(float t, int);

	void invert();
	void transpose();

	Matrix4 transpose() const;
	Matrix4 inverseOrtho() const;
	Matrix4 inverse() const;

	static Matrix4 frustum(float left, float right, float bottom, float top, float nearp, float farp);

	inline void transform(const Vector3 &vector, Vector3 &out) const {
		out.X = vector.X * _m[0][0] + vector.Y * _m[0][1] + vector.Z * _m[0][2] + _m[0][3];
		out.Y = vector.X * _m[1][0] + vector.Y * _m[1][1] + vector.Z * _m[1][2] + _m[1][3];
		out.Z = vector.X * _m[2][0] + vector.Y * _m[2][1] + vector.Z * _m[2][2] + _m[2][3];
	}

	// Transform the vector as if this were a 3x3 matrix.
	inline void transform3x3(const Vector3 &vector, Vector3 &out) const {
		out.X = vector.X * _m[0][0] + vector.Y * _m[0][1] + vector.Z * _m[0][2];
		out.Y = vector.X * _m[1][0] + vector.Y * _m[1][1] + vector.Z * _m[1][2];
		out.Z = vector.X * _m[2][0] + vector.Y * _m[2][1] + vector.Z * _m[2][2];
	}

	// Transform the vector as if this were a 3x3 matrix.
	inline void transform3x3(const Vector4 &vector, Vector3 &out) const {
		out.X = vector.X * _m[0][0] + vector.Y * _m[0][1] + vector.Z * _m[0][2];
		out.Y = vector.X * _m[1][0] + vector.Y * _m[1][1] + vector.Z * _m[1][2];
		out.Z = vector.X * _m[2][0] + vector.Y * _m[2][1] + vector.Z * _m[2][2];
	}

	// Transform the vector as if this were a 3x4 matrix.
	inline void transform3x4(const Vector4 &vector, Vector4 &out) const {
		out.X = vector.X * _m[0][0] + vector.Y * _m[0][1] + vector.Z * _m[0][2] + _m[0][3];
		out.Y = vector.X * _m[1][0] + vector.Y * _m[1][1] + vector.Z * _m[1][2] + _m[1][3];
		out.Z = vector.X * _m[2][0] + vector.Y * _m[2][1] + vector.Z * _m[2][2] + _m[2][3];
		out.W = vector.X * _m[3][0] + vector.Y * _m[3][1] + vector.Z * _m[3][2] + _m[3][3];
	}

	inline void transform(const Vector4 &vector, Vector4 &out) const {
		out.X = vector.X * _m[0][0] + vector.Y * _m[0][1] + vector.Z * _m[0][2] + vector.W * _m[0][3];
		out.Y = vector.X * _m[1][0] + vector.Y * _m[1][1] + vector.Z * _m[1][2] + vector.W * _m[1][3];
		out.Z = vector.X * _m[2][0] + vector.Y * _m[2][1] + vector.Z * _m[2][2] + vector.W * _m[2][3];
		out.W = vector.X * _m[3][0] + vector.Y * _m[3][1] + vector.Z * _m[3][2] + vector.W * _m[3][3];
	}

	float _m[4][4];
};

} // end of namespace TinyGL

#endif
