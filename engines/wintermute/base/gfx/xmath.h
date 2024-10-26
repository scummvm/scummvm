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
 * Code originated from Wine sources.
 * Copyright (C) 2008 David Adam
 * Copyright (C) 2008 Luis Busquets
 * Copyright (C) 2008 Jérôme Gardou
 * Copyright (C) 2008 Philip Nilsson
 * Copyright (C) 2008 Henri Verbeet
 */

#ifndef WINTERMUTE_XMATH_H
#define WINTERMUTE_XMATH_H

#include "engines/wintermute/math/rect32.h"

namespace Wintermute {

#if defined(SCUMMVM_USE_PRAGMA_PACK)
#pragma pack(4)
#endif

struct DXVector2 {
	float           _x;
	float           _y;

	DXVector2() {}
};

struct DXVector3 {
	float           _x;
	float           _y;
	float           _z;

	DXVector3() {}
	DXVector3(const float *pf);
	DXVector3(float fx, float fy, float fz);

	operator float* ();
	operator const float* () const;

	DXVector3 &operator += (const DXVector3 &);
	DXVector3 &operator -= (const DXVector3 &);
	DXVector3 &operator *= (float);
	DXVector3 &operator /= (float);

	DXVector3 operator + () const;
	DXVector3 operator - () const;

	DXVector3 operator + (const DXVector3 &) const;
	DXVector3 operator - (const DXVector3 &) const;
	DXVector3 operator * (float) const;
	DXVector3 operator / (float) const;

	bool operator == (const DXVector3 &) const;
	bool operator != (const DXVector3 &) const;
};

struct DXVector4 {
	float           _x;
	float           _y;
	float           _z;
	float           _w;

	DXVector4() {}
	DXVector4(const float *pf);
	DXVector4(float fx, float fy, float fz, float fw);

	operator float* ();
	operator const float* () const;
};

struct DXQuaternion {
	float           _x;
	float           _y;
	float           _z;
	float           _w;

	DXQuaternion() {}
	DXQuaternion(const float *pf);
	DXQuaternion(float fx, float fy, float fz, float fw);

	operator float* ();
	operator const float* () const;
};

struct DXPlane {
	float           _a;
	float           _b;
	float           _c;
	float           _d;

	DXPlane() {}
};

struct DXMatrix {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		} matrix;
		float _m[4][4];
		float _m4x4[16];
	};
	
	DXMatrix() {}
	DXMatrix(const float *pf);

	operator float* ();
	operator const float* () const;

	DXMatrix operator * (const DXMatrix &) const;
};

struct DXViewport {
	uint32       _x;
	uint32       _y;
	uint32       _width;
	uint32       _height;
	float        _minZ;
	float        _maxZ;
};

#if defined(SCUMMVM_USE_PRAGMA_PACK)
#pragma pack()
#endif

DXQuaternion *DXQuaternionRotationMatrix(DXQuaternion *out,const DXMatrix *m);
DXMatrix *DXMatrixPerspectiveFovLH(DXMatrix *pout,float fovy, float aspect, float zn, float zf);
DXMatrix *DXMatrixPerspectiveFovRH(DXMatrix *pout, float fovy, float aspect, float zn, float zf);
DXMatrix *DXMatrixLookAtLH(DXMatrix *out, const DXVector3 *eye, const DXVector3 *at, const DXVector3 *up);
DXMatrix *DXMatrixLookAtRH(DXMatrix *out, const DXVector3 *eye, const DXVector3 *at, const DXVector3 *up);
DXMatrix *DXMatrixInverse(DXMatrix *pout, float *pdeterminant, const DXMatrix *pm);
DXPlane *DXPlaneFromPointNormal(DXPlane *pout, const DXVector3 *pvpoint, const DXVector3 *pvnormal);
DXPlane *DXPlaneFromPoints(DXPlane *pout, const DXVector3 *pv1, const DXVector3 *pv2, const DXVector3 *pv3);
DXVector3 *DXPlaneIntersectLine(DXVector3 *pout, const DXPlane *pp, const DXVector3 *pv1, const DXVector3 *pv2);
DXVector3 *DXVec3Normalize(DXVector3 *pout, const DXVector3 *pv);
DXMatrix *DXMatrixTranslation(DXMatrix *pout, float x, float y, float z);
DXMatrix *DXMatrixScaling(DXMatrix *pout, float sx, float sy, float sz);
DXMatrix *DXMatrixRotationY(DXMatrix *pout, float angle);
DXMatrix *DXMatrixRotationZ(DXMatrix *pout, float angle);
DXMatrix *DXMatrixRotationYawPitchRoll(DXMatrix *out, float yaw, float pitch, float roll);
DXMatrix *DXMatrixRotationQuaternion(DXMatrix *pout, const DXQuaternion *pq);
DXQuaternion *DXQuaternionSlerp(DXQuaternion *out, const DXQuaternion *q1, const DXQuaternion *q2, float t);
DXVector4 *DXVec3Transform(DXVector4 *pout, const DXVector3 *pv, const DXMatrix *pm);
DXVector3 *DXVec3TransformCoord(DXVector3 *pout, const DXVector3 *pv, const DXMatrix *pm);
DXVector3 *DXVec3TransformNormal(DXVector3 *pout, const DXVector3 *pv, const DXMatrix *pm);
DXMatrix *DXMatrixMultiply(DXMatrix *pout, const DXMatrix *pm1, const DXMatrix *pm2);
DXVector3 *DXVec3Project(DXVector3 *pout, const DXVector3 *pv, const DXViewport *pviewport,
                         const DXMatrix *pprojection, const DXMatrix *pview, const DXMatrix *pworld);
DXMatrix *DXMatrixTranspose(DXMatrix *pout, const DXMatrix *pm);

static inline DXMatrix *DXMatrixIdentity(DXMatrix *pout) {
	(*pout)._m[0][1] = 0.0f;
	(*pout)._m[0][2] = 0.0f;
	(*pout)._m[0][3] = 0.0f;
	(*pout)._m[1][0] = 0.0f;
	(*pout)._m[1][2] = 0.0f;
	(*pout)._m[1][3] = 0.0f;
	(*pout)._m[2][0] = 0.0f;
	(*pout)._m[2][1] = 0.0f;
	(*pout)._m[2][3] = 0.0f;
	(*pout)._m[3][0] = 0.0f;
	(*pout)._m[3][1] = 0.0f;
	(*pout)._m[3][2] = 0.0f;
	(*pout)._m[0][0] = 1.0f;
	(*pout)._m[1][1] = 1.0f;
	(*pout)._m[2][2] = 1.0f;
	(*pout)._m[3][3] = 1.0f;
	return pout;
}

static inline DXVector3 *DXVec3Lerp(DXVector3 *pout, const DXVector3 *pv1, const DXVector3 *pv2, float s) {
	pout->_x = (1-s) * (pv1->_x) + s * (pv2->_x);
	pout->_y = (1-s) * (pv1->_y) + s * (pv2->_y);
	pout->_z = (1-s) * (pv1->_z) + s * (pv2->_z);
	return pout;
}

static inline float DXQuaternionDot(const DXQuaternion *pq1, const DXQuaternion *pq2) {
	return (pq1->_x) * (pq2->_x) + (pq1->_y) * (pq2->_y) + (pq1->_z) * (pq2->_z) + (pq1->_w) * (pq2->_w);
}

static inline DXVector3 *DXVec3Cross(DXVector3 *pout, const DXVector3 *pv1, const DXVector3 *pv2) {
	DXVector3 temp;

	temp._x = (pv1->_y) * (pv2->_z) - (pv1->_z) * (pv2->_y);
	temp._y = (pv1->_z) * (pv2->_x) - (pv1->_x) * (pv2->_z);
	temp._z = (pv1->_x) * (pv2->_y) - (pv1->_y) * (pv2->_x);
	*pout = temp;
	return pout;
}

static inline float DXVec3Dot(const DXVector3 *pv1, const DXVector3 *pv2) {
	return (pv1->_x) * (pv2->_x) + (pv1->_y) * (pv2->_y) + (pv1->_z) * (pv2->_z);
}

static inline DXVector3 *DXVec3Subtract(DXVector3 *pout, const DXVector3 *pv1, const DXVector3 *pv2) {
	pout->_x = pv1->_x - pv2->_x;
	pout->_y = pv1->_y - pv2->_y;
	pout->_z = pv1->_z - pv2->_z;
	return pout;
}

static inline float DXVec3Length(const DXVector3 *pv) {
	return sqrtf(pv->_x * pv->_x + pv->_y * pv->_y + pv->_z * pv->_z);
}

} // End of namespace Wintermute

#endif
