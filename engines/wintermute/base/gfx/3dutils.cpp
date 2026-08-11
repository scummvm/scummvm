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
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/gfx/3dutils.h"
#include "common/scummsys.h"

#ifdef ENABLE_WME3D
#include "common/util.h"
#endif

namespace Wintermute {

#ifdef ENABLE_WME3D

bool C3DUtils::intersectTriangle(const DXVector3 &orig, const DXVector3 &dir,
	                         DXVector3 &v0, DXVector3 &v1, DXVector3 &v2,
	                         float *t, float *u, float *v) {
	// Find vectors for two edges sharing vert0
	DXVector3 edge1 = v1 - v0;
	DXVector3 edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	DXVector3 pvec;
	DXVec3Cross(&pvec, &dir, &edge2);

	// If determinant is near zero, ray lies in plane of triangle
	float det = DXVec3Dot(&edge1, &pvec);
	if (det < 0.0001f)
		return false;

	// Calculate distance from vert0 to ray origin
	DXVector3 tvec = orig - v0;

	// Calculate U parameter and test bounds
	*u = DXVec3Dot(&tvec, &pvec);
	if (*u < 0.0f || *u > det)
		return false;

	// Prepare to test V parameter
	DXVector3 qvec;
	DXVec3Cross(&qvec, &tvec, &edge1);

	// Calculate V parameter and test bounds
	*v = DXVec3Dot(&dir, &qvec);
	if (*v < 0.0f || *u + *v > det)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	*t = DXVec3Dot(&edge2, &qvec);

	float fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	DXVector3 intersection;
	DXVector3 dest = orig + dir;
	DXPlane plane;
	DXPlaneFromPoints(&plane, &v0, &v1, &v2);
	DXPlaneIntersectLine(&intersection, &plane, &orig, &dest);
	*t = intersection._x;
	*u = intersection._y;
	*v = intersection._z;

	return true;
}

DXMatrix *C3DUtils::matrixSetTranslation(DXMatrix *mat, DXVector3 *vec) {
	mat->matrix._41 = vec->_x;
	mat->matrix._42 = vec->_y;
	mat->matrix._43 = vec->_z;

	return mat;
}

DXMatrix *C3DUtils::matrixSetRotation(DXMatrix *mat, DXVector3 *vec) {
	double cr = cos(vec->_x);
	double sr = sin(vec->_x);
	double cp = cos(vec->_y);
	double sp = sin(vec->_y);
	double cy = cos(vec->_z);
	double sy = sin(vec->_z);

	mat->matrix._11 = (float)(cp * cy);
	mat->matrix._12 = (float)(cp * sy);
	mat->matrix._13 = (float)(-sp);

	double srsp = sr * sp;
	double crsp = cr * sp;

	mat->matrix._21 = (float)(srsp * cy - cr * sy);
	mat->matrix._22 = (float)(srsp * sy + cr * cy);
	mat->matrix._23 = (float)(sr * cp);

	mat->matrix._31 = (float)(crsp * cy + sr * sy);
	mat->matrix._32 = (float)(crsp * sy - sr * cy);
	mat->matrix._33 = (float)(cr * cp);

	return mat;
}


bool C3DUtils::pickGetIntersect(DXVector3 lineStart, DXVector3 lineEnd,
                                DXVector3 v0, DXVector3 v1, DXVector3 v2,
	                        DXVector3 *intersection, float *distance) {
	// compute plane's normal
	DXVector3 vertex;
	DXVector3 normal;

	DXVector3 edge1 = v1 - v0;
	DXVector3 edge2 = v2 - v1;

	DXVec3Cross(&normal, &edge1, &edge2);
	DXVec3Normalize(&normal, &normal);

	vertex = v0;

	DXVector3 direction, l1;
	float lineLength, distFromPlane, percentage;

	direction._x = lineEnd._x - lineStart._x; // calculate the lines direction vector
	direction._y = lineEnd._y - lineStart._y;
	direction._z = lineEnd._z - lineStart._z;

	lineLength = DXVec3Dot(&direction, &normal); // This gives us the line length (the blue dot L3 + L4 in figure d)

	if (fabsf(lineLength) < 0.00001f)
		return false;

	l1._x = vertex._x - lineStart._x; // calculate vector L1 (the PINK line in figure d)
	l1._y = vertex._y - lineStart._y;
	l1._z = vertex._z - lineStart._z;

	distFromPlane = DXVec3Dot(&l1, &normal); // gives the distance from the plane (ORANGE Line L3 in figure d)
	percentage = distFromPlane / lineLength; // How far from Linestart , intersection is as a percentage of 0 to 1
	if (percentage < 0.0)
		return false;
	else if (percentage > 1.0)
		return false;

	*distance = percentage; //record the distance from beginning of ray (0.0 -1.0)

	intersection->_x = lineStart._x + direction._x * percentage; // add the percentage of the line to line start
	intersection->_y = lineStart._y + direction._y * percentage;
	intersection->_z = lineStart._z + direction._z * percentage;

	return true;
}

void C3DUtils::decomposeMatrixSimple(const DXMatrix *mat, DXVector3 *transVec, DXVector3 *scaleVec, DXQuaternion *rotQ) {
	*transVec = DXVector3(mat->matrix._41, mat->matrix._42, mat->matrix._43);
	*scaleVec = DXVector3(sqrtf(mat->matrix._11 * mat->matrix._11 + mat->matrix._21 * mat->matrix._21 + mat->matrix._31 * mat->matrix._31),
	                      sqrtf(mat->matrix._12 * mat->matrix._12 + mat->matrix._22 * mat->matrix._22 + mat->matrix._32 * mat->matrix._32),
	                      sqrtf(mat->matrix._13 * mat->matrix._13 + mat->matrix._23 * mat->matrix._23 + mat->matrix._33 * mat->matrix._33));

	DXQuaternion q;
	DXQuaternionRotationMatrix(&q, mat);

	*rotQ = q;
}

#endif

} // End of namespace Wintermute
