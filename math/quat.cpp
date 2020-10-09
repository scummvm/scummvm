/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

/*
 * Quaternion-math originally borrowed from plib http://plib.sourceforge.net/index.html
 * This code was originally made available under the LGPLv2 license (or later).
 *
 * Quaternion routines are Copyright (C) 1999
 * Kevin B. Thompson <kevinbthompson@yahoo.com>
 * Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
 * Largely rewritten by "Negative0" <negative0@earthlink.net>
 *
 * This code (and our modifications) is made available here under the GPLv2 (or later).
 *
 * Additional changes written based on the math presented in
 * http://www.swarthmore.edu/NatSci/mzucker1/e27/diebel2006attitude.pdf
 *
 */

#include "common/streamdebug.h"

#include "common/math.h"
#include "math/quat.h"

namespace Math {

Quaternion::Quaternion(const Matrix3 &m) {
	fromMatrix(m);
	normalize();
}

Quaternion::Quaternion(const Matrix4 &m) {
	fromMatrix(m.getRotation());
}

Quaternion::Quaternion(const Vector3d &axis, const Angle &angle) {
	float s = (angle / 2).getSine();
	float c = (angle / 2).getCosine();
	set(axis.x() * s, axis.y() * s, axis.z() * s, c);
}

Quaternion Quaternion::xAxis(const Angle &angle) {
	Quaternion q(Vector3d(1.0f, 0.0f, 0.0f), angle);
	return q;
}

Quaternion Quaternion::yAxis(const Angle &angle) {
	Quaternion q(Vector3d(0.0f, 1.0f, 0.0f), angle);
	return q;
}

Quaternion Quaternion::zAxis(const Angle &angle) {
	Quaternion q(Vector3d(0.0f, 0.0f, 1.0f), angle);
	return q;
}

Quaternion Quaternion::slerpQuat(const Quaternion& to, const float t) const {
	Quaternion dst;
	float scale0, scale1;
	float flip = 1.0f;
	float angle = this->dotProduct(to);

	// Make sure the rotation is the short one
	if (angle < 0.0f) {
		angle = -angle;
		flip = -1.0f;
	}

	// Spherical Interpolation
	// Threshold of 1e-6
	if (angle < 1.0f - (float) 1E-6f) {
		float theta = acosf(angle);
		float invSineTheta = 1.0f / sinf(theta);

		scale0 = sinf((1.0f - t) * theta) * invSineTheta;
		scale1 = (sinf(t * theta) * invSineTheta) * flip;
	// Linear Interpolation
	} else {
		scale0 = 1.0f - t;
		scale1 = t * flip;
	}

	// Apply the interpolation
	dst = (*this * scale0) + (to * scale1);
	return dst;
}

Quaternion& Quaternion::normalize() {
	const float scale = sqrtf(square(x()) + square(y()) + square(z()) + square(w()));

	// Already normalized if the scale is 1.0
	if (scale != 1.0f && scale != 0.0f)
		set(x() / scale, y() / scale, z() / scale, w() / scale);

	return *this;
}

void Quaternion::transform(Vector3d &v) const {
	const Vector3d im = Vector3d(x(), y(), z());
	v += 2.0 * Vector3d::crossProduct(im, Vector3d::crossProduct(im, v) + w() * v);
}

void Quaternion::fromMatrix(const Matrix3 &m) {
	float qx, qy, qz, qw;
	float tr = m.getValue(0, 0) + m.getValue(1, 1) + m.getValue(2, 2);
	float s;

	if (tr > 0.0f) {
		s = sqrtf(tr + 1.0f);
		qw = s * 0.5f;
		s = 0.5f / s;
		qx = (m.getValue(2, 1) - m.getValue(1, 2)) * s;
		qy = (m.getValue(0, 2) - m.getValue(2, 0)) * s;
		qz = (m.getValue(1, 0) - m.getValue(0, 1)) * s;
	} else {
		int h = 0;
		if (m.getValue(1, 1) > m.getValue(0, 0))
			h = 1;
		if (m.getValue(2, 2) > m.getValue(h, h))
			h = 2;

		if (h == 0) {
			s = sqrt(m.getValue(0, 0) - (m.getValue(1,1) + m.getValue(2, 2)) + 1.0f);
			qx = s * 0.5f;
			s = 0.5f / s;
			qy = (m.getValue(0, 1) + m.getValue(1, 0)) * s;
			qz = (m.getValue(2, 0) + m.getValue(0, 2)) * s;
			qw = (m.getValue(2, 1) - m.getValue(1, 2)) * s;
		} else if (h == 1) {
			s = sqrt(m.getValue(1, 1) - (m.getValue(2,2) + m.getValue(0, 0)) + 1.0f);
			qy = s * 0.5f;
			s = 0.5f / s;
			qz = (m.getValue(1, 2) + m.getValue(2, 1)) * s;
			qx = (m.getValue(0, 1) + m.getValue(1, 0)) * s;
			qw = (m.getValue(0, 2) - m.getValue(2, 0)) * s;
		} else {
			s = sqrt(m.getValue(2, 2) - (m.getValue(0,0) + m.getValue(1, 1)) + 1.0f);
			qz = s * 0.5f;
			s = 0.5f / s;
			qx = (m.getValue(2, 0) + m.getValue(0, 2)) * s;
			qy = (m.getValue(1, 2) + m.getValue(2, 1)) * s;
			qw = (m.getValue(1, 0) - m.getValue(0, 1)) * s;
		}
	}
	set(qx, qy, qz, qw);
}

void Quaternion::toMatrix(Matrix4 &dst) const {
	float two_xx = x() * (x() + x());
	float two_xy = x() * (y() + y());
	float two_xz = x() * (z() + z());

	float two_wx = w() * (x() + x());
	float two_wy = w() * (y() + y());
	float two_wz = w() * (z() + z());

	float two_yy = y() * (y() + y());
	float two_yz = y() * (z() + z());

	float two_zz = z() * (z() + z());

	float newMat[16] = {
		1.0f - (two_yy + two_zz),	two_xy - two_wz,		two_xz + two_wy,	  0.0f,
		two_xy + two_wz,		1.0f - (two_xx + two_zz),	two_yz - two_wx,	  0.0f,
		two_xz - two_wy,		two_yz + two_wx,		1.0f - (two_xx + two_yy), 0.0f,
		0.0f,				0.0f,				0.0f,			  1.0f
	};
	dst.setData(newMat);
}

Matrix4 Quaternion::toMatrix() const {
	Matrix4 dst;
	toMatrix(dst);
	return dst;
}

Quaternion Quaternion::inverse() const {
	Quaternion q = *this;
	q.normalize();
	q.x() = -q.x();
	q.y() = -q.y();
	q.z() = -q.z();
	return q;
}

Vector3d Quaternion::directionVector(const int col) const {
	Matrix4 dirMat = toMatrix();
	return Vector3d(dirMat.getValue(0, col), dirMat.getValue(1, col), dirMat.getValue(2, col));
}

Angle Quaternion::getAngleBetween(const Quaternion &to) {
	Quaternion q = this->inverse() * to;
	Angle diff(Common::rad2deg(2 * acos(q.w())));
	return diff;
}

Quaternion Quaternion::fromEuler(const Angle &first, const Angle &second, const Angle &third, EulerOrder order) {
	// First create a matrix with the rotation
	Matrix4 rot(first, second, third, order);

	// Convert this rotation matrix to a Quaternion
	return Quaternion(rot);
}

void Quaternion::getEuler(Angle *first, Angle *second, Angle *third, EulerOrder order) const {
	// Create a matrix from the Quaternion
	Matrix4 rot = toMatrix();

	// Convert the matrix to Euler Angles
	Angle f, s, t;
	rot.getEuler(&f, &s, &t, order);

	// Assign the Angles if we have a reference
	if (first != nullptr)
		*first = f;
	if (second != nullptr)
		*second = s;
	if (third != nullptr)
		*third = t;
}

Quaternion& Quaternion::operator=(const Quaternion& quat) {
	x() = quat.x();
	y() = quat.y();
	z() = quat.z();
	w() = quat.w();

	return *this;
}

Quaternion Quaternion::operator*(const Quaternion &o) const {
	return Quaternion(
		w() * o.x() + x() * o.w() + y() * o.z() - z() * o.y(),
		w() * o.y() - x() * o.z() + y() * o.w() + z() * o.x(),
		w() * o.z() + x() * o.y() - y() * o.x() + z() * o.w(),
		w() * o.w() - x() * o.x() - y() * o.y() - z() * o.z()
	);
}

Quaternion Quaternion::operator*(const float c) const {
	return Quaternion(x() * c, y() * c, z() * c, w() * c);
}

Quaternion& Quaternion::operator*=(const Quaternion &o) {
	*this = *this * o;
	return *this;
}

Quaternion Quaternion::operator+(const Quaternion &o) const {
	return Quaternion(x() + o.x(), y() + o.y(), z() + o.z(), w() + o.w());
}

Quaternion& Quaternion::operator+=(const Quaternion &o) {
	*this = *this + o;
	return *this;
}

bool Quaternion::operator==(const Quaternion &o) const {
	float dw = fabs(w() - o.w());
	float dx = fabs(x() - o.x());
	float dy = fabs(y() - o.y());
	float dz = fabs(z() - o.z());
	// Threshold of equality
	float th = 1E-5f;

	if ((dw < th) && (dx < th) && (dy < th) && (dz < th)) {
		return true;
	}
	return false;
}

bool Quaternion::operator!=(const Quaternion &o) const {
	return !(*this == o);
}

} // End namespace Math
