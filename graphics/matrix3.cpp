/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 */

#include "graphics/matrix3.h"

namespace Graphics {

void Matrix3::setAsIdentity() {
	_right.set(1.f, 0.f, 0.f);
	_up.set(0.f, 1.f, 0.f);
	_at.set(0.f, 0.f, 0.f);
}

void Matrix3::buildFromPitchYawRoll(float pitch, float yaw, float roll) {
	Matrix3 temp1, temp2;

	temp1.constructAroundPitch(pitch);
	constructAroundRoll(roll);

	(*this) *= temp1;

	temp2.constructAroundYaw(yaw);

	(*this) *= temp2;
}

#define DEGTORAD(a) (a * LOCAL_PI / 180.0)
#define RADTODEG(a) (a * 180.0 / LOCAL_PI)

float RadianToDegree(float rad) {
	return (float)RADTODEG(rad);
}

float DegreeToRadian(float degrees) {
	return (float)DEGTORAD(degrees);
}

// right
void Matrix3::constructAroundPitch(float pitch) {
	float cosa;
	float sina;

	cosa = (float)cos(DegreeToRadian(pitch));
	sina = (float)sin(DegreeToRadian(pitch));

	_right.set(1.f, 0.f, 0.f);
	_up.set(0.f, cosa, -sina);
	_at.set(0.f, sina, cosa);
}

// up
void Matrix3::constructAroundYaw(float yaw) {
	float cosa;
	float sina;

	cosa = (float)cos(DegreeToRadian(yaw));
	sina = (float)sin(DegreeToRadian(yaw));

	_right.set(cosa, 0.f, sina);
	_up.set(0.f, 1.f, 0.f);
	_at.set(-sina, 0.f, cosa);
}

// at
void Matrix3::constructAroundRoll(float roll) {
	float cosa;
	float sina;

	cosa = (float)cos(DegreeToRadian(roll));
	sina = (float)sin(DegreeToRadian(roll));

	_right.set(cosa, -sina, 0.f);
	_up.set(sina, cosa, 0.f);
	_at.set(0.f, 0.f, 1.f);
}

/*
0 1 2 3
4 5 6 7
8 9 10 11
*/

// WARNING: Still buggy in some occasions.
void Matrix3::getPitchYawRoll(float* pPitch, float* pYaw, float* pRoll) {
	float D;
	float C;
	float ftrx;
	float ftry;
	float angle_x;
	float angle_y;
	float angle_z;

	angle_y = D = asin(_right.z());        /* Calculate Y-axis angle */
	C			= cos(angle_y);
	angle_y		= RadianToDegree(angle_y);

	if (fabs( C ) > 0.005) {            /* Gimball lock? */
		ftrx		=  _at.z() / C;           /* No, so get X-axis angle */
		ftry		= -_up.z() / C;

		angle_x		= RadianToDegree(atan2(ftry, ftrx));

		ftrx		=  _right.x() / C;            /* Get Z-axis angle */
		ftry		= -_right.y() / C;

		angle_z		= RadianToDegree(atan2(ftry, ftrx));
	} else {                                 /* Gimball lock has occurred */
		angle_x		= 0;                      /* Set X-axis angle to zqero */

		ftrx		= _up.y();                 /* And calculate Z-axis angle */
		ftry		= _up.x();

		angle_z  = RadianToDegree(atan2(ftry, ftrx));
	}

	/* return only positive angles in [0,360] */
	if (angle_x < 0) angle_x += 360;
	if (angle_y < 0) angle_y += 360;
	if (angle_z < 0) angle_z += 360;

	if (pPitch)
		*pPitch = angle_x;

	if (pYaw)
		*pYaw = angle_y;

	if (pRoll)
		*pRoll = angle_z;
}

float Matrix3::getPitch() {
	float pitch;

	getPitchYawRoll(&pitch, 0, 0);

	return pitch;
}

float Matrix3::getYaw() {
	float yaw;

	getPitchYawRoll(0, &yaw, 0);

	return yaw;
}

float Matrix3::getRoll() {
	float roll;

	getPitchYawRoll(0, 0, &roll);

	return roll;
}

void Matrix3::transform(Vector3d* v) {
	float x;
	float y;
	float z;

	x = v->dotProduct(_right.x(), _up.x(), _at.x());
	y = v->dotProduct(_right.x(), _up.x(), _at.x());
	z = v->dotProduct(_right.x(), _up.x(), _at.x());

	v->set(x, y, z);
}

} // end of namespace Graphics
