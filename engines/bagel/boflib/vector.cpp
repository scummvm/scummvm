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

#define UNIT_VECTOR_LENGTH 1

CVector::CVector() {
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

CVector::CVector(const VECTOR &stVector) {
	x = stVector.x;
	y = stVector.y;
	z = stVector.z;
}

CVector::CVector(double xx, double yy, double zz) {
	x = xx;
	y = yy;
	z = zz;
}

#if BOF_DEBUG
CVector::~CVector() {
	Assert(IsValidObject(this));
}
#endif

void CVector::Unitize() {
	// Make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	double w;

	// can't unitize an empty vector (avoid division by zero errors)
	//
	if ((this->x != 0.0) || (this->y != 0.0)) {

		w = UNIT_VECTOR_LENGTH / (this->x * this->x + this->y * this->y);

		*this *= sqrt(w);
	}
}

void CVector::Normalize() {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	double length;

	length = Length();

	Assert(length != 0);

	if (length != 0) {
		this->x /= length;
		this->y /= length;
	}
}

void CVector::SetVector(double xx, double yy, double zz) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	x = xx;
	y = yy;
	z = zz;
}

double CVector::Length() {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	return sqrt(x * x + y * y);
}

double CVector::AngleBetween(const VECTOR &vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vTmp(vector);
	double fCos, angle;

	// get the angle by getting the arc-cosine of the cosine of the
	// angle between the 2 vectors.
	//
	fCos = this->DotProduct(vTmp) / (this->Length() * vTmp.Length());

	if (fCos > 1.0) {
		fCos = 1.0;
	} else if (fCos < -1.0) {
		fCos = -1.0;
	}

	angle = acos(fCos);

	// angle must be positive and less than 180 degrees
	// otherwise something is wrong
	//
	// Assert((angle >= (double)0.0) && (angle <= PI));

	return angle;
}

double CVector::DotProduct(const VECTOR &vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	return (this->x * vector.x) + (this->y * vector.y);
}

CVector CVector::CrossProduct(const VECTOR &vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	//
	// TODO: This function needs to be finished
	//

	return vector;
}

void CVector::Reflect(const VECTOR &vMirror) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vTmp(vMirror);
	double angle, length;

	// Unitize the vectors (scale the vector so it's length is 1 pixel)
	//
	length = this->Length();

	this->Unitize();
	vTmp.Unitize();

	angle = this->AngleBetween(vTmp);

	// The vector reflection:  R = 2 * N * cos(angle) - L
	*this = (vTmp * cos(angle) * 2 - *this) * length;
}

void CVector::Rotate(double angle) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	double co, si, xx, yy;

	// get the sine and cosine of the angle
	//
	co = cos(angle);
	si = sin(angle);

	xx = this->x * co - this->y * si;
	yy = this->y * co + this->x * si;

	this->x = xx;
	this->y = yy;
}

double CVector::RealAngle(const VECTOR &vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vTmp;
	double angle;

	vTmp = *this;
	angle = vTmp.AngleBetween(vector);

	if (angle != (double)0.0) {

		vTmp.Rotate(angle);

		// determine if the angle is greater then 180 degrees
		//
		if (((int)(vTmp.AngleBetween(vector) * 1000) == 0)) {
			angle = 2 * PI - angle;
		}
	}

	return angle;
}

double CVector::Angle() {
	// make sure this object has not been destructed
	Assert(IsValidObject(this));

	CVector vTmp(0, -1);
	double fAngle;

	fAngle = vTmp.RealAngle(*this);

	if (fAngle < Deg2Rad(0)) {
		fAngle += Deg2Rad(360);
	}

	Assert((fAngle >= Deg2Rad(0)) && fAngle < Deg2Rad(360));

	return fAngle;
}

CVector CVector::operator+(VECTOR vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vSum(this->x + vector.x, this->y + vector.y, this->z + vector.z);

	return vSum;
}

CVector CVector::operator+(double offset) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vSum(this->x + offset, this->y + offset, this->z + offset);

	return vSum;
}

CVector CVector::operator-(VECTOR vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vDif(this->x - vector.x, this->y - vector.y, this->z - vector.z);

	return vDif;
}

CVector CVector::operator-(double offset) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vDif(this->x - offset, this->y - offset, this->z - offset);

	return vDif;
}

void CVector::operator+=(VECTOR vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	this->x += vector.x;
	this->y += vector.y;
	this->z += vector.z;
}

void CVector::operator-=(VECTOR vector) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	this->x -= vector.x;
	this->y -= vector.y;
	this->z -= vector.z;
}

CVector CVector::operator*(double scalar) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	CVector vProduct(this->x * scalar, this->y * scalar, this->z * scalar);

	return vProduct;
}

CVector CVector::operator/(double scalar) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	// can't divide by 0
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
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}

void CVector::operator/=(double scalar) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	// can't divide by 0
	Assert(scalar != (double)0.0);

	if (scalar != (double)0.0) {
		this->x /= scalar;
		this->y /= scalar;
		this->z /= scalar;
	}
}

bool CVector::operator==(VECTOR v) {
	// make sure this object is not used after it is destructed
	Assert(IsValidObject(this));

	bool bReturn;

	bReturn = false;

	if ((this->x == v.x) && (this->y == v.y))
		bReturn = true;

	return bReturn;
}

double DistanceBetweenPoints(VECTOR v1, VECTOR v2) {
	CVector vTmp(v1.x - v2.x, v1.y - v2.y, 0);

	return vTmp.Length();
}

} // namespace Bagel
