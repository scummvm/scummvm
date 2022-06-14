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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_VECTOR3_H
#define HPL_VECTOR3_H

#include "Vector2.h"
#include <math.h>

namespace hpl {

template<class T>
class cVector3 {
public:
	T x, y, z;
#if 0
		union{
			struct {
				T x,y,z;
			};
			T v[3];
		};
#endif
	//////////////////////////////////////////
	// Constructors
	/////////////////////////////////////////
	cVector3() {
		x = 0;
		y = 0;
		z = 0;
	}
	cVector3(T aVal) {
		x = aVal;
		y = aVal;
		z = aVal;
	}
	cVector3(T aX, T aY, T aZ) {
		x = aX;
		y = aY;
		z = aZ;
	}

	cVector3(cVector3<T> const &aVec) {
		x = aVec.x;
		y = aVec.y;
		z = aVec.z;
	}

	cVector3(cVector2<T> const &aVec) {
		x = aVec.x;
		y = aVec.y;
		z = 0;
	}

	//////////////////////////////////////////
	// Copy
	/////////////////////////////////////////

	inline cVector3<T> &operator=(const cVector3<T> &aVec) {
		x = aVec.x;
		y = aVec.y;
		z = aVec.z;
		return *this;
	}

	inline cVector3<T> &operator=(const cVector2<T> &aVec) {
		x = aVec.x;
		y = aVec.y;
		return *this;
	}

	inline cVector3<T> &operator=(const T aVal) {
		x = aVal;
		y = aVal;
		z = aVal;
		return *this;
	}

	//////////////////////////////////////////
	// Boolean
	/////////////////////////////////////////

	inline bool operator==(const cVector3<T> &aVec) const {
		if (x == aVec.x && y == aVec.y && z == aVec.z)
			return true;
		else
			return false;
	}

	inline bool operator!=(const cVector3<T> &aVec) const {
		if (x == aVec.x && y == aVec.y && z == aVec.z)
			return false;
		else
			return true;
	}

	inline bool operator<(const cVector3<T> &aVec) const {
		if (x != aVec.x)
			return x < aVec.x;
		if (y != aVec.y)
			return y < aVec.y;
		return z < aVec.z;
	}

	inline bool operator>(const cVector3<T> &aVec) const {
		if (x != aVec.x)
			return x > aVec.x;
		if (y != aVec.y)
			return y > aVec.y;
		return z > aVec.z;
	}

	//////////////////////////////////////////
	// Vector3 Arithmetic
	/////////////////////////////////////////

	inline cVector3<T> operator+(const cVector3<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x + aVec.x;
		vec.y = y + aVec.y;
		vec.z = z + aVec.z;
		return vec;
	}

	inline cVector3<T> operator-(const cVector3<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x - aVec.x;
		vec.y = y - aVec.y;
		vec.z = z - aVec.z;
		return vec;
	}

	inline cVector3<T> operator*(const cVector3<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x * aVec.x;
		vec.y = y * aVec.y;
		vec.z = z * aVec.z;
		return vec;
	}

	inline cVector3<T> operator/(const cVector3<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x / aVec.x;
		vec.y = y / aVec.y;
		vec.z = z / aVec.z;
		return vec;
	}

	inline cVector3<T> &operator-=(const cVector3<T> &aVec) {
		x -= aVec.x;
		y -= aVec.y;
		z -= aVec.z;
		return *this;
	}

	inline cVector3<T> &operator+=(const cVector3<T> &aVec) {
		x += aVec.x;
		y += aVec.y;
		z += aVec.z;
		return *this;
	}

	inline cVector3<T> &operator*=(const cVector3<T> &aVec) {
		x *= aVec.x;
		y *= aVec.y;
		z *= aVec.z;
		return *this;
	}

	inline cVector3<T> &operator/=(const cVector3<T> &aVec) {
		x /= aVec.x;
		y /= aVec.y;
		z /= aVec.z;
		return *this;
	}

	//////////////////////////////////////////
	// Vector2 Arithmetic
	/////////////////////////////////////////

	inline cVector3<T> operator+(const cVector2<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x + aVec.x;
		vec.y = y + aVec.y;
		vec.z = z;
		return vec;
	}

	inline cVector3<T> operator-(const cVector2<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x - aVec.x;
		vec.y = y - aVec.y;
		vec.z = z;
		return vec;
	}

	inline cVector3<T> operator*(const cVector2<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x * aVec.x;
		vec.y = y * aVec.y;
		vec.z = z;
		return vec;
	}

	inline cVector3<T> operator/(const cVector2<T> &aVec) const {
		cVector3<T> vec;
		vec.x = x / aVec.x;
		vec.y = y / aVec.y;
		vec.z = z;
		return vec;
	}

	inline cVector3<T> &operator-=(const cVector2<T> &aVec) {
		x -= aVec.x;
		y -= aVec.y;
		return *this;
	}

	inline cVector3<T> &operator+=(const cVector2<T> &aVec) {
		x += aVec.x;
		y += aVec.y;
		return *this;
	}

	inline cVector3<T> &operator*=(const cVector2<T> &aVec) {
		x *= aVec.x;
		y *= aVec.y;
		return *this;
	}

	inline cVector3<T> &operator/=(const cVector2<T> &aVec) {
		x /= aVec.x;
		y /= aVec.y;
		return *this;
	}

	//////////////////////////////////////////
	// Single Float Arithmetic
	/////////////////////////////////////////

	inline cVector3<T> operator/(const T &aVal) const {
		cVector3<T> vec;
		vec.x = x / aVal;
		vec.y = y / aVal;
		vec.z = z / aVal;
		return vec;
	}

	inline cVector3<T> operator*(const T &aVal) const {
		cVector3<T> vec;
		vec.x = x * aVal;
		vec.y = y * aVal;
		vec.z = z * aVal;
		return vec;
	}

	inline cVector3<T> operator+(const T &aVal) const {
		cVector3<T> vec;
		vec.x = x + aVal;
		vec.y = y + aVal;
		vec.z = z + aVal;
		return vec;
	}

	cVector3<T> operator-(const T &aVal) const {
		cVector3<T> vec;
		vec.x = x - aVal;
		vec.y = y - aVal;
		vec.z = z - aVal;
		return vec;
	}

	//////////////////////////////////////////
	// Methods
	/////////////////////////////////////////

	inline void FromVec(const T *apVec) {
		x = apVec[0];
		y = apVec[1];
		z = apVec[2];
	}

	inline T Length() {
		return sqrt(x * x + y * y + z * z);
	}

	inline T SqrLength() {
		return x * x + y * y + z * z;
	}

	/**
	 * Only use this on double or float vectors
	 * \return Length of Vector
	 */
	T Normalise() {
		T length = sqrt(x * x + y * y + z * z);

		// Will also work for zero-sized vectors, but will change nothing
		if (length > 1e-08) {
			T InvLength = 1.0f / length;
			x *= InvLength;
			y *= InvLength;
			z *= InvLength;
		}

		return length;
	}

	//////////////////////////////////////////
	// Printing
	/////////////////////////////////////////

	tString ToString() const {
		char buf[512];
		sprintf(buf, "%f : %f : %f", x, y, z);
		tString str = buf;
		return str;
	}

	tString ToFileString() const {
		char buf[512];
		sprintf(buf, "%g %g %g", x, y, z);
		tString str = buf;
		return str;
	}
};

};     // namespace hpl
#endif // HPL_VECTOR3_H
