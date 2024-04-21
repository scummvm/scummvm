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

#include "bagel/boflib/vector.h"

namespace Bagel {

#define UNIT_Vector_LENGTH 1

CVector::CVector() {
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

CVector::CVector(const Vector &stVector) {
	x = stVector.x;
	y = stVector.y;
	z = stVector.z;
}

CVector::CVector(double xx, double yy, double zz) {
	x = xx;
	y = yy;
	z = zz;
}

double CVector::Length() {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	return sqrt(x * x + y * y);
}

double CVector::AngleBetween(const Vector &vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vTmp(vector);

	// Get the angle by getting the arc-cosine of the cosine of the
	// angle between the 2 vectors.
	double fCos = this->DotProduct(vTmp) / (this->Length() * vTmp.Length());

	if (fCos > 1.0) {
		fCos = 1.0;
	} else if (fCos < -1.0) {
		fCos = -1.0;
	}

	double angle = acos(fCos);
	return angle;
}

double CVector::DotProduct(const Vector &vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	return (this->x * vector.x) + (this->y * vector.y);
}

void CVector::Rotate(double angle) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	// Get the sine and cosine of the angle
	double co = cos(angle);
	double si = sin(angle);

	double xx = this->x * co - this->y * si;
	double yy = this->y * co + this->x * si;

	this->x = xx;
	this->y = yy;
}

double CVector::RealAngle(const Vector &vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vTmp = *this;
	double angle = vTmp.AngleBetween(vector);

	if (angle != (double)0.0) {
		vTmp.Rotate(angle);

		// Determine if the angle is greater then 180 degrees
		if (((int)(vTmp.AngleBetween(vector) * 1000) == 0)) {
			angle = 2 * PI - angle;
		}
	}

	return angle;
}

CVector CVector::operator+(Vector vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vSum(this->x + vector.x, this->y + vector.y, this->z + vector.z);

	return vSum;
}

CVector CVector::operator+(double offset) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vSum(this->x + offset, this->y + offset, this->z + offset);

	return vSum;
}

CVector CVector::operator-(Vector vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vDif(this->x - vector.x, this->y - vector.y, this->z - vector.z);

	return vDif;
}

CVector CVector::operator-(double offset) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vDif(this->x - offset, this->y - offset, this->z - offset);

	return vDif;
}

void CVector::operator+=(Vector vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
}

void CVector::operator-=(Vector vector) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
}

CVector CVector::operator*(double scalar) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vProduct(this->x * scalar, this->y * scalar, this->z * scalar);

	return vProduct;
}

CVector CVector::operator/(double scalar) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	// Can't divide by 0
	Assert(scalar != (double)0.0);

	CVector vDividend;

	if (scalar != (double)0.0) {
		vDividend.x = this->x / scalar;
		vDividend.y = this->y / scalar;
		vDividend.z = this->z / scalar;
	}

	return vDividend;
}

void CVector::operator*=(double scalar) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

void CVector::operator/=(double scalar) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	// can't divide by 0
	Assert(scalar != (double)0.0);

	if (scalar != (double)0.0) {
		this->x /= scalar;
		this->y /= scalar;
		this->z /= scalar;
	}
}

bool CVector::operator==(Vector v) {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	bool bReturn = false;

	if ((this->x == v.x) && (this->y == v.y))
		bReturn = true;

	return bReturn;
}

} // namespace Bagel
