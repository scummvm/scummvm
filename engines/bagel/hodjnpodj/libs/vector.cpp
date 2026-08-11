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

#include "bagel/hodjnpodj/libs/vector.h"

namespace Bagel {
namespace HodjNPodj {

#define UNIT_VECTOR_LENGTH 1

CVector::CVector() {
	x = y = z = 0;
}

CVector::CVector(const VECTOR &src) {
	x = src.x;
	y = src.y;
	z = src.z;
}

CVector::CVector(double xx, double yy, double zz) {
	x = xx;
	y = yy;
	z = zz;
}

void CVector::Unitize() {
	double w;

	// avoid division by zero errors
	assert(this->x != 0 || this->y != 0);

	w = UNIT_VECTOR_LENGTH / (this->x * this->x + this->y * this->y);
	*this *= sqrt(w);
}

void CVector::Normalize() {
	double length;

	length = Length();

	assert(length != 0);

	if (length != 0) {
		this->x /= length;
		this->y /= length;
	}
}

void CVector::SetVector(double xx, double yy, double zz) {
	x = xx;
	y = yy;
	z = zz;
}


double CVector::Length() const {
	return sqrt(x * x + y * y);
}


double CVector::AngleBetween(const VECTOR &rhs) {
	CVector vTmp(rhs);
	double fCos, angle;

	// Get the angle by getting the arc-cosine of the cosine of the
	// angle between the 2 vectors.
	fCos = this->DotProduct(vTmp) / (this->Length() * vTmp.Length());

	if (fCos > 1.0) {
		fCos = 1.0;
	} else if (fCos < -1.0) {
		fCos = -1.0;
	}

	angle = acos(fCos);
	return angle;
}


double CVector::DotProduct(const VECTOR &rhs) const {
	return (x * rhs.x) + (y * rhs.y);
}

void CVector::Reflect(const VECTOR &vMirror) {
	CVector vTmp(vMirror);
	double angle, length;

	// Unitize the vectors (scale the vector so it's length is 1 pixel)
	//
	length = this->Length();

	this->Unitize();
	vTmp.Unitize();

	angle = this->AngleBetween(vTmp);

	// the vector reflection:  R = 2 * N * cos(angle) - L
	*this = (vTmp * cos(angle) * 2 - *this) * length;
}

void CVector::Rotate(double angle) {
	double co, si, xx, yy;

	// get the sine and cosine of the angle
	co = cos(angle);
	si = sin(angle);

	xx = this->x * co - this->y * si;
	yy = this->y * co + this->x * si;

	this->x = xx;
	this->y = yy;
}


double CVector::RealAngle(const VECTOR &rhs) {
	CVector vTmp;
	double angle;

	vTmp = *this;
	angle = vTmp.AngleBetween(rhs);

	if (angle != (double)0.0) {

		vTmp.Rotate(angle);

		// determine if the angle is greater then 180 degrees
		//
		if (((int)(vTmp.AngleBetween(rhs) * 1000) == 0)) {
			angle = 2 * PI - angle;
		}
	}

	return angle;
}


CVector CVector::operator +(const VECTOR &rhs) const {
	CVector vSum(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);

	return vSum;
}


CVector CVector::operator +(double offset) const {
	CVector vSum(this->x + offset, this->y + offset, this->z + offset);

	return vSum;
}


CVector CVector::operator -(const VECTOR &rhs) const {
	CVector vDif(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);

	return vDif;
}


CVector CVector::operator -(double offset) const {
	CVector vDif(this->x - offset, this->y - offset, this->z - offset);

	return vDif;
}


void CVector::operator +=(const VECTOR &rhs) {
	this->x += rhs.x;
	this->y += rhs.y;
	this->z += rhs.z;
}


void CVector::operator -=(const VECTOR &rhs) {
	this->x -= rhs.x;
	this->y -= rhs.y;
	this->z -= rhs.z;
}


CVector CVector::operator *(double scalar) const {
	CVector vProduct(this->x * scalar, this->y * scalar, this->z * scalar);

	return vProduct;
}


CVector CVector::operator /(double scalar) const {
	// can't divide by 0
	assert(scalar != (double)0.0);

	CVector vDividend;

	if (scalar != (double)0.0) {
		vDividend.x = this->x / scalar;
		vDividend.y = this->y / scalar;
		vDividend.z = this->z / scalar;
	}

	return vDividend;
}


void CVector::operator *=(double scalar) {
	this->x *= scalar;
	this->y *= scalar;
	this->z *= scalar;
}


void CVector::operator /=(double scalar) {
	// can't divide by 0
	assert(scalar != (double)0.0);

	if (scalar != (double)0.0) {
		this->x /= scalar;
		this->y /= scalar;
		this->z /= scalar;
	}
}

bool CVector::operator ==(const VECTOR &v) const {
	bool bReturn = ((this->x == v.x) && (this->y == v.y));
	return bReturn;
}


double distanceBetweenPoints(const VECTOR &v1, const VECTOR &v2) {
	CVector vTmp(v1.x - v2.x, v1.y - v2.y, 0);

	return vTmp.Length();
}

} // namespace HodjNPodj
} // namespace Bagel
