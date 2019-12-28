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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_SPACE_H
#define STARTREK_SPACE_H

#include "fixedint.h"

namespace StarTrek {

template<typename T>
struct TPoint {
	T x;
	T y;
	T z;

	TPoint() : x(0), y(0), z(0) {}
	TPoint(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

	int32 getDiagonal() {
		return (int32)sqrt((double)x * x + y * y + z * z);
	}

	TPoint<T> operator+(const TPoint<T> &p) const {
		TPoint<T> p2;
		p2.x = x + p.x;
		p2.y = y + p.y;
		p2.z = z + p.z;
		return p2;
	}
	TPoint<T> operator-(const TPoint<T> &p) const {
		TPoint<T> p2;
		p2.x = x - p.x;
		p2.y = y - p.y;
		p2.z = z - p.z;
		return p2;
	}
	void operator+=(const TPoint &p) {
		x += p.x;
		y += p.y;
		z += p.z;
	}
	void operator-=(const TPoint &p) {
		x -= p.x;
		y -= p.y;
		z -= p.z;
	}
	T &operator[](int i) {
		if (i == 0)
			return x;
		else if (i == 1)
			return y;
		else if (i == 2)
			return z;

		// Unknown Out of Range
		assert(false);
		return x;
	}
	T operator[](int i) const {
		if (i == 0)
			return x;
		else if (i == 1)
			return y;
		else if (i == 2)
			return z;

		// Unknown Out of Range
		assert(false);
		return x;
	}
};

typedef TPoint<int32> Point3;
typedef TPoint<int16> Point3_Short;
typedef TPoint<Fixed14> Point_Fixed14;


template<typename T>
struct TMatrix {
private:
	T m[3];

public:
	TMatrix() {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				m[i][j] = 0;
	}
	TMatrix(const TMatrix<T> &mat) {
		m[0] = mat.m[0];
		m[1] = mat.m[1];
		m[2] = mat.m[2];
	}
	T &operator[](int i) {
		return m[i];
	};
	T operator[](int i) const {
		return m[i];
	};

	TMatrix operator*(const TMatrix &m2) const {
		TMatrix ret;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				ret[i][j] = 0;
				for (int a = 0; a < 3; a++)
					ret[i][j] += m[i][a] * m2[a][j];
			}
		}
		return ret;
	}

	void operator*=(const TMatrix &m2) {
		*this = *this * m2;
	}

	TMatrix<T> invert() const {
		TMatrix<T> ret;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				ret[i][j] = m[j][i];
			}
		}
		return ret;
	}
};

typedef TMatrix<Point_Fixed14> Matrix;

struct Star {
	bool active;
	Point3 pos;
};

// Struct for objects in space.
// TODO: what does this stand for? Maybe rename it.
struct R3 {
	Point3 pos; // 0x0
	Matrix matrix; // 0xc
	int16 field1e; // 0x1e
	int16 field20; // 0x20
	int16 field22; // 0x22
	int16 field24; // 0x24
	Point3_Short speed; // 0x26
	int32 funcPtr1; // 0x2c
	int32 funcPtr2; // 0x30
	int16 field34; // 0x34
	Point3 field36; // 0x36
	Matrix matrix2; // 0x42
	int32 field54; // 0x54 (used for sorting by draw priority?)
	int16 field58; // 0x58
	int16 field5a; // 0x5a
	Bitmap *bitmap; // 0x68 (was: shpFile)
	// 0x6a (was: bitmapOffset)
	double field80; // 0x80
	double field88; // 0x88
	double field90; // 0x90
	double field98; // 0x98
};

// Maximum number of stars visible at once in the starfields
#define NUM_STARS 16

// Maximum number of R3 objects in space at once
#define NUM_SPACE_OBJECTS 0x30

} // End of namespace StarTrek

#endif
