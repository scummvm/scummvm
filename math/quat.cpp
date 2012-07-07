/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

// Quaternion-math borrowed from plib http://plib.sourceforge.net/index.html
// Which is covered by LGPL2
// And has this additional copyright note:
/*
 Quaternion routines are Copyright (C) 1999
 Kevin B. Thompson <kevinbthompson@yahoo.com>
 Modified by Sylvan W. Clebsch <sylvan@stanford.edu>
 Largely rewritten by "Negative0" <negative0@earthlink.net>
 */

#include "common/streamdebug.h"

#include "math/quat.h"

namespace Math {

Quaternion Quaternion::slerpQuat(const Quaternion& to, const float t) {
	Quaternion dst;
	float co, scale0, scale1;
	bool flip = false ;

	/* SWC - Interpolate between to quaternions */

	co = this->dotProduct(to);

	if (co < 0.0f) {
		co = -co;
		flip = true;
	}

	if ( co < 1.0f - (float) 1e-6 )	{
		float o = (float) acos ( co );
		float so = 1.0f / (float) sin ( o );

		scale0 = (float) sin ( (1.0f - t) * o ) * so;
		scale1 = (float) sin ( t * o ) * so;
	} else {
		scale0 = 1.0f - t;
		scale1 = t;
	}

	if (flip) {
		scale1 = -scale1 ;
	}

	dst.x() = scale0 * this->x() + scale1 * to.x() ;
	dst.y() = scale0 * this->y() + scale1 * to.y() ;
	dst.z() = scale0 * this->z() + scale1 * to.z() ;
	dst.w() = scale0 * this->w() + scale1 * to.w() ;
	
	return dst;
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
		1.0f-(two_yy+two_zz),	two_xy-two_wz,			two_xz+two_wy,			0.0f,
		two_xy+two_wz,			1.0f-(two_xx+two_zz),	two_yz-two_wx,			0.0f,
		two_xz-two_wy,			two_yz+two_wx,			1.0f-(two_xx+two_yy),	0.0f,
		0.0f,					0.0f,					0.0f,					1.0f
	};
	dst.setData(newMat);
}

Matrix4 Quaternion::toMatrix() const {
	Matrix4 dst;
	toMatrix(dst);
	return dst;
}

	Quaternion Quaternion::fromEuler(const Angle &yaw, const Angle &pitch, const Angle &roll) {
		float cr, cp, cy, sr, sp, sy, cpcy, spsy;

		cy = (yaw / 2).getCosine();
		cp = (pitch / 2).getCosine();
		cr = (roll / 2).getCosine();

		sy = (yaw / 2).getSine();
		sp = (pitch / 2).getSine();
		sr = (roll / 2).getSine();

		cpcy = cp * cy;
		spsy = sp * sy;

		return Quaternion(
			cr * sp * cy + sr * cp * sy,
			cr * cp * sy - sr * sp * cy,
			sr * cpcy - cr * spsy,
			cr * cpcy + sr * spsy);
	}
}
