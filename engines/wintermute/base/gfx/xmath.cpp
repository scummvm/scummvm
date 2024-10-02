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

#include "engines/wintermute/base/gfx/xmath.h"

namespace Wintermute {

DXQuaternion *DXQuaternionRotationMatrix(DXQuaternion *out, const DXMatrix *m) {
	float s, trace;

	trace = m->_m[0][0] + m->_m[1][1] + m->_m[2][2] + 1.0f;
	if (trace > 1.0f) {
		s = 2.0f * sqrtf(trace);
		out->_x = (m->_m[1][2] - m->_m[2][1]) / s;
		out->_y = (m->_m[2][0] - m->_m[0][2]) / s;
		out->_z = (m->_m[0][1] - m->_m[1][0]) / s;
		out->_w = 0.25f * s;
	} else {
		int i, maxi = 0;

		for (i = 1; i < 3; i++) {
			if (m->_m[i][i] > m->_m[maxi][maxi])
				maxi = i;
		}

		switch (maxi) {
		case 0:
			s = 2.0f * sqrtf(1.0f + m->_m[0][0] - m->_m[1][1] - m->_m[2][2]);
			out->_x = 0.25f * s;
			out->_y = (m->_m[0][1] + m->_m[1][0]) / s;
			out->_z = (m->_m[0][2] + m->_m[2][0]) / s;
			out->_w = (m->_m[1][2] - m->_m[2][1]) / s;
			break;

		case 1:
			s = 2.0f * sqrtf(1.0f + m->_m[1][1] - m->_m[0][0] - m->_m[2][2]);
			out->_x = (m->_m[0][1] + m->_m[1][0]) / s;
			out->_y = 0.25f * s;
			out->_z = (m->_m[1][2] + m->_m[2][1]) / s;
			out->_w = (m->_m[2][0] - m->_m[0][2]) / s;
			break;

		case 2:
			s = 2.0f * sqrtf(1.0f + m->_m[2][2] - m->_m[0][0] - m->_m[1][1]);
			out->_x = (m->_m[0][2] + m->_m[2][0]) / s;
			out->_y = (m->_m[1][2] + m->_m[2][1]) / s;
			out->_z = 0.25f * s;
			out->_w = (m->_m[0][1] - m->_m[1][0]) / s;
			break;
		}
	}

	return out;
}

DXMatrix *DXMatrixPerspectiveFovLH(DXMatrix *pout,float fovy, float aspect, float zn, float zf) {
	DXMatrixIdentity(pout);
	pout->_m[0][0] = 1.0f / (aspect * tanf(fovy/2.0f));
	pout->_m[1][1] = 1.0f / tanf(fovy/2.0f);
	pout->_m[2][2] = zf / (zf - zn);
	pout->_m[2][3] = 1.0f;
	pout->_m[3][2] = (zf * zn) / (zn - zf);
	pout->_m[3][3] = 0.0f;
	return pout;
}

DXMatrix *DXMatrixPerspectiveFovRH(DXMatrix *pout, float fovy, float aspect, float zn, float zf) {
	DXMatrixIdentity(pout);
	pout->_m[0][0] = 1.0f / (aspect * tanf(fovy / 2.0f));
	pout->_m[1][1] = 1.0f / tanf(fovy / 2.0f);
	pout->_m[2][2] = zf / (zn - zf);
	pout->_m[2][3] = -1.0f;
	pout->_m[3][2] = (zf * zn) / (zn - zf);
	pout->_m[3][3] = 0.0f;
	return pout;
}

DXMatrix *DXMatrixInverse(DXMatrix *pout, float *pdeterminant, const DXMatrix *pm) {
	float det, t[3], v[16];
	int i, j;

	t[0] = pm->_m[2][2] * pm->_m[3][3] - pm->_m[2][3] * pm->_m[3][2];
	t[1] = pm->_m[1][2] * pm->_m[3][3] - pm->_m[1][3] * pm->_m[3][2];
	t[2] = pm->_m[1][2] * pm->_m[2][3] - pm->_m[1][3] * pm->_m[2][2];
	v[0] = pm->_m[1][1] * t[0] - pm->_m[2][1] * t[1] + pm->_m[3][1] * t[2];
	v[4] = -pm->_m[1][0] * t[0] + pm->_m[2][0] * t[1] - pm->_m[3][0] * t[2];

	t[0] = pm->_m[1][0] * pm->_m[2][1] - pm->_m[2][0] * pm->_m[1][1];
	t[1] = pm->_m[1][0] * pm->_m[3][1] - pm->_m[3][0] * pm->_m[1][1];
	t[2] = pm->_m[2][0] * pm->_m[3][1] - pm->_m[3][0] * pm->_m[2][1];
	v[8] = pm->_m[3][3] * t[0] - pm->_m[2][3] * t[1] + pm->_m[1][3] * t[2];
	v[12] = -pm->_m[3][2] * t[0] + pm->_m[2][2] * t[1] - pm->_m[1][2] * t[2];

	det = pm->_m[0][0] * v[0] + pm->_m[0][1] * v[4] + pm->_m[0][2] * v[8] + pm->_m[0][3] * v[12];
	if (det == 0.0f)
		return nullptr;
	if (pdeterminant)
		*pdeterminant = det;

	t[0] = pm->_m[2][2] * pm->_m[3][3] - pm->_m[2][3] * pm->_m[3][2];
	t[1] = pm->_m[0][2] * pm->_m[3][3] - pm->_m[0][3] * pm->_m[3][2];
	t[2] = pm->_m[0][2] * pm->_m[2][3] - pm->_m[0][3] * pm->_m[2][2];
	v[1] = -pm->_m[0][1] * t[0] + pm->_m[2][1] * t[1] - pm->_m[3][1] * t[2];
	v[5] = pm->_m[0][0] * t[0] - pm->_m[2][0] * t[1] + pm->_m[3][0] * t[2];

	t[0] = pm->_m[0][0] * pm->_m[2][1] - pm->_m[2][0] * pm->_m[0][1];
	t[1] = pm->_m[3][0] * pm->_m[0][1] - pm->_m[0][0] * pm->_m[3][1];
	t[2] = pm->_m[2][0] * pm->_m[3][1] - pm->_m[3][0] * pm->_m[2][1];
	v[9] = -pm->_m[3][3] * t[0] - pm->_m[2][3] * t[1]- pm->_m[0][3] * t[2];
	v[13] = pm->_m[3][2] * t[0] + pm->_m[2][2] * t[1] + pm->_m[0][2] * t[2];

	t[0] = pm->_m[1][2] * pm->_m[3][3] - pm->_m[1][3] * pm->_m[3][2];
	t[1] = pm->_m[0][2] * pm->_m[3][3] - pm->_m[0][3] * pm->_m[3][2];
	t[2] = pm->_m[0][2] * pm->_m[1][3] - pm->_m[0][3] * pm->_m[1][2];
	v[2] = pm->_m[0][1] * t[0] - pm->_m[1][1] * t[1] + pm->_m[3][1] * t[2];
	v[6] = -pm->_m[0][0] * t[0] + pm->_m[1][0] * t[1] - pm->_m[3][0] * t[2];

	t[0] = pm->_m[0][0] * pm->_m[1][1] - pm->_m[1][0] * pm->_m[0][1];
	t[1] = pm->_m[3][0] * pm->_m[0][1] - pm->_m[0][0] * pm->_m[3][1];
	t[2] = pm->_m[1][0] * pm->_m[3][1] - pm->_m[3][0] * pm->_m[1][1];
	v[10] = pm->_m[3][3] * t[0] + pm->_m[1][3] * t[1] + pm->_m[0][3] * t[2];

	t[0] = pm->_m[1][2] * pm->_m[2][3] - pm->_m[1][3] * pm->_m[2][2];
	t[1] = pm->_m[0][2] * pm->_m[2][3] - pm->_m[0][3] * pm->_m[2][2];
	t[2] = pm->_m[0][2] * pm->_m[1][3] - pm->_m[0][3] * pm->_m[1][2];
	v[3] = -pm->_m[0][1] * t[0] + pm->_m[1][1] * t[1] - pm->_m[2][1] * t[2];
	v[7] = pm->_m[0][0] * t[0] - pm->_m[1][0] * t[1] + pm->_m[2][0] * t[2];

	v[11] = -pm->_m[0][0] * (pm->_m[1][1] * pm->_m[2][3] - pm->_m[1][3] * pm->_m[2][1]) +
	         pm->_m[1][0] * (pm->_m[0][1] * pm->_m[2][3] - pm->_m[0][3] * pm->_m[2][1]) -
	         pm->_m[2][0] * (pm->_m[0][1] * pm->_m[1][3] - pm->_m[0][3] * pm->_m[1][1]);

	v[15] = pm->_m[0][0] * (pm->_m[1][1] * pm->_m[2][2] - pm->_m[1][2] * pm->_m[2][1]) -
	        pm->_m[1][0] * (pm->_m[0][1] * pm->_m[2][2] - pm->_m[0][2] * pm->_m[2][1]) +
	        pm->_m[2][0] * (pm->_m[0][1] * pm->_m[1][2] - pm->_m[0][2] * pm->_m[1][1]);

	det = 1.0f / det;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++)
			pout->_m[i][j] = v[4 * i + j] * det;

	return pout;
}

DXPlane *DXPlaneFromPointNormal(DXPlane *pout, const DXVector3 *pvpoint, const DXVector3 *pvnormal) {
	pout->_a = pvnormal->_x;
	pout->_b = pvnormal->_y;
	pout->_c = pvnormal->_z;
	pout->_d = -DXVec3Dot(pvpoint, pvnormal);
	return pout;
}

DXPlane *D3DXPlaneFromPoints(DXPlane *pout, const DXVector3 *pv1, const DXVector3 *pv2, const DXVector3 *pv3) {
	DXVector3 edge1, edge2, normal, Nnormal;

	edge1._x = 0.0f; edge1._y = 0.0f; edge1._z = 0.0f;
	edge2._x = 0.0f; edge2._y = 0.0f; edge2._z = 0.0f;
	DXVec3Subtract(&edge1, pv2, pv1);
	DXVec3Subtract(&edge2, pv3, pv1);
	DXVec3Cross(&normal, &edge1, &edge2);
	DXVec3Normalize(&Nnormal, &normal);
	DXPlaneFromPointNormal(pout, pv1, &Nnormal);
	return pout;
}

DXVector3 *DXPlaneIntersectLine(DXVector3 *pout, const DXPlane *pp, const DXVector3 *pv1, const DXVector3 *pv2) {
	DXVector3 direction, normal;
	float dot, temp;

	normal._x = pp->_a;
	normal._y = pp->_b;
	normal._z = pp->_c;
	direction._x = pv2->_x - pv1->_x;
	direction._y = pv2->_y - pv1->_y;
	direction._z = pv2->_z - pv1->_z;
	dot = DXVec3Dot(&normal, &direction);
	if (!dot)
		return nullptr;
	temp = (pp->_d + DXVec3Dot(&normal, pv1) ) / dot;
	pout->_x = pv1->_x - temp * direction._x;
	pout->_y = pv1->_y - temp * direction._y;
	pout->_z = pv1->_z - temp * direction._z;
	return pout;
}

DXVector3 *DXVec3Normalize(DXVector3 *pout, const DXVector3 *pv) {
	float norm;

	norm = DXVec3Length(pv);
	if (!norm) {
		pout->_x = 0.0f;
		pout->_y = 0.0f;
		pout->_z = 0.0f;
	} else {
		pout->_x = pv->_x / norm;
		pout->_y = pv->_y / norm;
		pout->_z = pv->_z / norm;
	}

	return pout;
}

DXMatrix *DXMatrixTranslation(DXMatrix *pout, float x, float y, float z) {
	DXMatrixIdentity(pout);
	pout->_m[3][0] = x;
	pout->_m[3][1] = y;
	pout->_m[3][2] = z;
	return pout;
}

DXMatrix *D3DXMatrixScaling(DXMatrix *pout, float sx, float sy, float sz) {
	DXMatrixIdentity(pout);
	pout->_m[0][0] = sx;
	pout->_m[1][1] = sy;
	pout->_m[2][2] = sz;
	return pout;
}

DXMatrix *D3DXMatrixRotationZ(DXMatrix *pout, float angle) {
	DXMatrixIdentity(pout);
	pout->_m[0][0] = cosf(angle);
	pout->_m[1][1] = cosf(angle);
	pout->_m[0][1] = sinf(angle);
	pout->_m[1][0] = -sinf(angle);
	return pout;
}

DXMatrix *DXMatrixRotationYawPitchRoll(DXMatrix *out, float yaw, float pitch, float roll) {
	float sroll, croll, spitch, cpitch, syaw, cyaw;

	sroll = sinf(roll);
	croll = cosf(roll);
	spitch = sinf(pitch);
	cpitch = cosf(pitch);
	syaw = sinf(yaw);
	cyaw = cosf(yaw);

	out->_m[0][0] = sroll * spitch * syaw + croll * cyaw;
	out->_m[0][1] = sroll * cpitch;
	out->_m[0][2] = sroll * spitch * cyaw - croll * syaw;
	out->_m[0][3] = 0.0f;
	out->_m[1][0] = croll * spitch * syaw - sroll * cyaw;
	out->_m[1][1] = croll * cpitch;
	out->_m[1][2] = croll * spitch * cyaw + sroll * syaw;
	out->_m[1][3] = 0.0f;
	out->_m[2][0] = cpitch * syaw;
	out->_m[2][1] = -spitch;
	out->_m[2][2] = cpitch * cyaw;
	out->_m[2][3] = 0.0f;
	out->_m[3][0] = 0.0f;
	out->_m[3][1] = 0.0f;
	out->_m[3][2] = 0.0f;
	out->_m[3][3] = 1.0f;

	return out;
}

DXMatrix *DXMatrixRotationQuaternion(DXMatrix *pout, const DXQuaternion *pq) {
	DXMatrixIdentity(pout);
	pout->_m[0][0] = 1.0f - 2.0f * (pq->_y * pq->_y + pq->_z * pq->_z);
	pout->_m[0][1] = 2.0f * (pq->_x *pq->_y + pq->_z * pq->_w);
	pout->_m[0][2] = 2.0f * (pq->_x * pq->_z - pq->_y * pq->_w);
	pout->_m[1][0] = 2.0f * (pq->_x * pq->_y - pq->_z * pq->_w);
	pout->_m[1][1] = 1.0f - 2.0f * (pq->_x * pq->_x + pq->_z * pq->_z);
	pout->_m[1][2] = 2.0f * (pq->_y *pq->_z + pq->_x *pq->_w);
	pout->_m[2][0] = 2.0f * (pq->_x * pq->_z + pq->_y * pq->_w);
	pout->_m[2][1] = 2.0f * (pq->_y *pq->_z - pq->_x *pq->_w);
	pout->_m[2][2] = 1.0f - 2.0f * (pq->_x * pq->_x + pq->_y * pq->_y);
	return pout;
}

DXQuaternion *DXQuaternionSlerp(DXQuaternion *out, const DXQuaternion *q1, const DXQuaternion *q2, float t) {
	float dot, temp;

	temp = 1.0f - t;
	dot = DXQuaternionDot(q1, q2);
	if (dot < 0.0f) {
		t = -t;
		dot = -dot;
	}

	if (1.0f - dot > 0.001f) {
		float theta = acosf(dot);
		temp = sinf(theta * temp) / sinf(theta);
		t = sinf(theta * t) / sinf(theta);
	}

	out->_x = temp * q1->_x + t * q2->_x;
	out->_y = temp * q1->_y + t * q2->_y;
	out->_z = temp * q1->_z + t * q2->_z;
	out->_w = temp * q1->_w + t * q2->_w;

	return out;
}

float D3DXQuaternionDot(const DXVector4 *pq1, const DXVector4 *pq2) {
	return (pq1->_x) * (pq2->_x) + (pq1->_y) * (pq2->_y) + (pq1->_z) * (pq2->_z) + (pq1->_w) * (pq2->_w);
}

DXVector4 *D3DXVec3Transform(DXVector4 *pout, const DXVector3 *pv, const DXMatrix *pm) {
	DXVector4 out;

	out._x = pm->_m[0][0] * pv->_x + pm->_m[1][0] * pv->_y + pm->_m[2][0] * pv->_z + pm->_m[3][0];
	out._y = pm->_m[0][1] * pv->_x + pm->_m[1][1] * pv->_y + pm->_m[2][1] * pv->_z + pm->_m[3][1];
	out._z = pm->_m[0][2] * pv->_x + pm->_m[1][2] * pv->_y + pm->_m[2][2] * pv->_z + pm->_m[3][2];
	out._w = pm->_m[0][3] * pv->_x + pm->_m[1][3] * pv->_y + pm->_m[2][3] * pv->_z + pm->_m[3][3];
	*pout = out;
	return pout;
}

DXVector3 *DXVec3TransformCoord(DXVector3 *pout, const DXVector3 *pv, const DXMatrix *pm) {
	DXVector3 out;

	float norm = pm->_m[0][3] * pv->_x + pm->_m[1][3] * pv->_y + pm->_m[2][3] *pv->_z + pm->_m[3][3];

	out._x = (pm->_m[0][0] * pv->_x + pm->_m[1][0] * pv->_y + pm->_m[2][0] * pv->_z + pm->_m[3][0]) / norm;
	out._y = (pm->_m[0][1] * pv->_x + pm->_m[1][1] * pv->_y + pm->_m[2][1] * pv->_z + pm->_m[3][1]) / norm;
	out._z = (pm->_m[0][2] * pv->_x + pm->_m[1][2] * pv->_y + pm->_m[2][2] * pv->_z + pm->_m[3][2]) / norm;

	*pout = out;

	return pout;
}

DXVector3 *DXVec3TransformNormal(DXVector3 *pout, const DXVector3 *pv, const DXMatrix *pm) {
	const DXVector3 v = *pv;

	pout->_x = pm->_m[0][0] * v._x + pm->_m[1][0] * v._y + pm->_m[2][0] * v._z;
	pout->_y = pm->_m[0][1] * v._x + pm->_m[1][1] * v._y + pm->_m[2][1] * v._z;
	pout->_z = pm->_m[0][2] * v._x + pm->_m[1][2] * v._y + pm->_m[2][2] * v._z;
	return pout;
}

DXMatrix *DXMatrixMultiply(DXMatrix *pout, const DXMatrix *pm1, const DXMatrix *pm2) {
	DXMatrix out;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			out._m[i][j] = pm1->_m[i][0] * pm2->_m[0][j] + pm1->_m[i][1] * pm2->_m[1][j] + pm1->_m[i][2] * pm2->_m[2][j] + pm1->_m[i][3] * pm2->_m[3][j];
		}
	}

	*pout = out;
	return pout;
}

DXVector3 *DXVec3Project(DXVector3 *pout, const DXVector3 *pv, const Rect32 *pviewport,
                         const DXMatrix *pprojection, const DXMatrix *pview, const DXMatrix *pworld) {
	DXMatrix m;

	DXMatrixIdentity(&m);
	if (pworld)
		DXMatrixMultiply(&m, &m, pworld);
	if (pview)
		DXMatrixMultiply(&m, &m, pview);
	if (pprojection)
		DXMatrixMultiply(&m, &m, pprojection);

	DXVec3TransformCoord(pout, pv, &m);

	if (pviewport) {
		pout->_x = pviewport->left + (1.0f + pout->_x) * pviewport->width() / 2.0f;
		pout->_y = pviewport->top  + (1.0f - pout->_y) * pviewport->height() / 2.0f;
		pout->_z = (1.0f + pout->_z) / 2.0f;
	}
	return pout;
}

float D3DXVec3Length(const DXVector3 *pv) {
	return sqrtf(pv->_x * pv->_x + pv->_y * pv->_y + pv->_z * pv->_z);
}

DXMatrix *DXMatrixLookAtLH(DXMatrix *out, const DXVector3 *eye, const DXVector3 *at, const DXVector3 *up) {
	DXVector3 right, upn, vec;

	DXVec3Subtract(&vec, at, eye);
	DXVec3Normalize(&vec, &vec);
	DXVec3Cross(&right, up, &vec);
	DXVec3Cross(&upn, &vec, &right);
	DXVec3Normalize(&right, &right);
	DXVec3Normalize(&upn, &upn);
	out->_m[0][0] = right._x;
	out->_m[1][0] = right._y;
	out->_m[2][0] = right._z;
	out->_m[3][0] = -DXVec3Dot(&right, eye);
	out->_m[0][1] = upn._x;
	out->_m[1][1] = upn._y;
	out->_m[2][1] = upn._z;
	out->_m[3][1] = -DXVec3Dot(&upn, eye);
	out->_m[0][2] = vec._x;
	out->_m[1][2] = vec._y;
	out->_m[2][2] = vec._z;
	out->_m[3][2] = -DXVec3Dot(&vec, eye);
	out->_m[0][3] = 0.0f;
	out->_m[1][3] = 0.0f;
	out->_m[2][3] = 0.0f;
	out->_m[3][3] = 1.0f;

	return out;
}


DXMatrix *DXMatrixLookAtRH(DXMatrix *out, const DXVector3 *eye, const DXVector3 *at, const DXVector3 *up) {
	DXVector3 right, upn, vec;

	DXVec3Subtract(&vec, at, eye);
	DXVec3Normalize(&vec, &vec);
	DXVec3Cross(&right, up, &vec);
	DXVec3Cross(&upn, &vec, &right);
	DXVec3Normalize(&right, &right);
	DXVec3Normalize(&upn, &upn);
	out->_m[0][0] = -right._x;
	out->_m[1][0] = -right._y;
	out->_m[2][0] = -right._z;
	out->_m[3][0] = DXVec3Dot(&right, eye);
	out->_m[0][1] = upn._x;
	out->_m[1][1] = upn._y;
	out->_m[2][1] = upn._z;
	out->_m[3][1] = -DXVec3Dot(&upn, eye);
	out->_m[0][2] = -vec._x;
	out->_m[1][2] = -vec._y;
	out->_m[2][2] = -vec._z;
	out->_m[3][2] = DXVec3Dot(&vec, eye);
	out->_m[0][3] = 0.0f;
	out->_m[1][3] = 0.0f;
	out->_m[2][3] = 0.0f;
	out->_m[3][3] = 1.0f;

	return out;
}

} // End of namespace Wintermute
