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

////////////////////////////////////////////////////////////////////////////////
//	XMath 1.0
//	Vector - Matrix library for 3D visualisation and
//	physics modeling.
//
//	(C) 1999 KDV Games
//	Author: Alexandre Kotliar
//
//
////////////////////////////////////////////////////////////////////////////////
#include "qdengine/xmath.h"


namespace QDEngine {


///////////////////////////////////////////////////////////////////////////////
//
//	Constants
//
///////////////////////////////////////////////////////////////////////////////

const Vect3f Vect3f::ZERO(0, 0, 0);

///////////////////////////////////////////////////////////////////////////////
//
//  class Vect2f
//
///////////////////////////////////////////////////////////////////////////////

float Vect2f::angle(const Vect2f &other) const {
	return acosf(dot(other) * invSqrtFast(norm2() * other.norm2() * 1.0000001f));
}

///////////////////////////////////////////////////////////////////////////////
//
//  class Vect3f
//
///////////////////////////////////////////////////////////////////////////////

float Vect3f::angle(const Vect3f &other) const {
	return acosf(dot(other) * invSqrtFast(norm2() * other.norm2() * 1.0000001f));
}


///////////////////////////////////////////////////////////////////////////////
//
//  class Mat3f
//
///////////////////////////////////////////////////////////////////////////////

Mat3f &Mat3f::set(const Vect3f &x_from, const Vect3f &y_from, const Vect3f &z_from,
                  const Vect3f &x_to, const Vect3f &y_to, const Vect3f &z_to) {
	xx = dot(x_from, x_to);
	xy = dot(y_from, x_to);
	xz = dot(z_from, x_to);

	yx = dot(x_from, y_to);
	yy = dot(y_from, y_to);
	yz = dot(z_from, y_to);

	zx = dot(x_from, z_to);
	zy = dot(y_from, z_to);
	zz = dot(z_from, z_to);
	return *this;
}

int Mat3f::invert(const Mat3f &M) {
	float D, oneOverDet;

	if (fabs(D = M.det()) < 1.0e-12) return 1; // not invertible
	oneOverDet = 1 / D;

	xx = (M.yy * M.zz - M.yz * M.zy) * oneOverDet;
	xy = (M.xz * M.zy - M.xy * M.zz) * oneOverDet;
	xz = (M.xy * M.yz - M.xz * M.yy) * oneOverDet;
	yx = (M.yz * M.zx - M.yx * M.zz) * oneOverDet;
	yy = (M.xx * M.zz - M.xz * M.zx) * oneOverDet;
	yz = (M.xz * M.yx - M.xx * M.yz) * oneOverDet;
	zx = (M.yx * M.zy - M.yy * M.zx) * oneOverDet;
	zy = (M.xy * M.zx - M.xx * M.zy) * oneOverDet;
	zz = (M.xx * M.yy - M.xy * M.yx) * oneOverDet;
	return 0;
}



int Mat3f::invert() {
	float D, oneOverDet;
	float oxy, oyz, ozx, oyx, ozy, oxz, oxx, oyy;

	if (fabs(D = det()) < 1.0e-12) return 1; // not invertible
	oneOverDet = 1 / D;

	oxx = xx;
	oyy = yy;
	oxy = xy;
	oyx = yx;
	oyz = yz;
	ozy = zy;
	ozx = zx;
	oxz = xz;

	xy = (oxz * ozy - zz * oxy) * oneOverDet;
	yz = (oxz * oyx - xx * oyz) * oneOverDet;
	zx = (oyx * ozy - yy * ozx) * oneOverDet;
	yx = (oyz * ozx - oyx * zz) * oneOverDet;
	zy = (oxy * ozx - ozy * xx) * oneOverDet;
	xz = (oxy * oyz - oxz * yy) * oneOverDet;
	xx = (oyy * zz - oyz * ozy) * oneOverDet;
	yy = (oxx * zz - oxz * ozx) * oneOverDet;
	zz = (oxx * oyy - oxy * oyx) * oneOverDet;
	return 0;
}


Mat3f &Mat3f::mult(const Mat3f &M, const Mat3f &N) {
	xx = M.xx * N.xx + M.xy * N.yx + M.xz * N.zx;
	xy = M.xx * N.xy + M.xy * N.yy + M.xz * N.zy;
	xz = M.xx * N.xz + M.xy * N.yz + M.xz * N.zz;
	yx = M.yx * N.xx + M.yy * N.yx + M.yz * N.zx;
	yy = M.yx * N.xy + M.yy * N.yy + M.yz * N.zy;
	yz = M.yx * N.xz + M.yy * N.yz + M.yz * N.zz;
	zx = M.zx * N.xx + M.zy * N.yx + M.zz * N.zx;
	zy = M.zx * N.xy + M.zy * N.yy + M.zz * N.zy;
	zz = M.zx * N.xz + M.zy * N.yz + M.zz * N.zz;

	return *this;
}


Mat3f &Mat3f::premult(const Mat3f &M) {
	float oxy, oyz, ozx, oyx, ozy, oxz;

	oxy = xy;
	oyx = yx;
	oyz = yz;
	ozy = zy;
	ozx = zx;
	oxz = xz;

	xy = M.xx * oxy + M.xy * yy  + M.xz * ozy;
	xz = M.xx * oxz + M.xy * oyz + M.xz * zz;
	yx = M.yx * xx  + M.yy * oyx + M.yz * ozx;
	yz = M.yx * oxz + M.yy * oyz + M.yz * zz;
	zx = M.zx * xx  + M.zy * oyx + M.zz * ozx;
	zy = M.zx * oxy + M.zy * yy  + M.zz * ozy;

	xx = M.xx * xx  + M.xy * oyx + M.xz * ozx;
	yy = M.yx * oxy + M.yy * yy  + M.yz * ozy;
	zz = M.zx * oxz + M.zy * oyz + M.zz * zz;

	return *this;
}


Mat3f &Mat3f::postmult(const Mat3f &M) {
	float oxy, oyz, ozx, oyx, ozy, oxz;

	oxy = xy;
	oyx = yx;
	oyz = yz;
	ozy = zy;
	ozx = zx;
	oxz = xz;

	xy = xx *  M.xy + oxy * M.yy + oxz * M.zy;
	xz = xx *  M.xz + oxy * M.yz + oxz * M.zz;
	yx = oyx * M.xx + yy  * M.yx + oyz * M.zx;
	yz = oyx * M.xz + yy  * M.yz + oyz * M.zz;
	zx = ozx * M.xx + ozy * M.yx + zz  * M.zx;
	zy = ozx * M.xy + ozy * M.yy + zz  * M.zy;

	xx = xx  * M.xx + oxy * M.yx + oxz * M.zx;
	yy = oyx * M.xy + yy  * M.yy + oyz * M.zy;
	zz = ozx * M.xz + ozy * M.yz + zz  * M.zz;

	return *this;
}

void Mat3f::makeRotationZ() {
	float cos = 0.0f;
	float sin;
	if (yy > FLT_EPS) {
		float tan = xy / yy;
		cos = invSqrtFast(tan * tan + 1.0f);
		sin = cos * tan;
	} else
		sin = xy > 0.0f ? 1.0f : -1.0f;
	xx = cos;
	xy = -sin;
	xz = 0.0f;
	yx = sin;
	yy = cos;
	yz = 0.0f;
	zx = 0.0f;
	zy = 0.0f;
	zz = 1.0f;
}
} // namespace QDEngine
