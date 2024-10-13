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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/math/math_util.h"
#include "common/scummsys.h"

#ifdef ENABLE_WME3D
#include "common/util.h"
#endif

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
float MathUtil::round(float val) {
	float result = floor(val);
	if (val - result >= 0.5f) {
		result += 1.0;
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////
float MathUtil::roundUp(float val) {
	float result = floor(val);
	if (val - result > 0) {
		result += 1.0;
	}
	return result;
}

#ifdef ENABLE_WME3D

bool intersectTriangle(const Math::Vector3d &orig, const Math::Vector3d &dir,
                       const Math::Vector3d &v0, const Math::Vector3d &v1, const Math::Vector3d &v2,
					   float &t, float &u, float &v) {
	// Find vectors for two edges sharing vert0
	Math::Vector3d edge1 = v1 - v0;
	Math::Vector3d edge2 = v2 - v0;

	// Begin calculating determinant - also used to calculate U parameter
	Math::Vector3d pVec;
	pVec = Math::Vector3d::crossProduct(dir, edge2);

	// If determinant is near zero, ray lies in plane of triangle
	float det = Math::Vector3d::dotProduct(edge1, pVec);
	if (ABS(det) < 0.0001f)
		return false;

	// Calculate distance from vert0 to ray origin
	Math::Vector3d tvec = orig - v0;

	// Calculate U parameter and test bounds
	u = Math::Vector3d::dotProduct(tvec, pVec) / det;
	if (u < 0.0f || u > 1.0f)
		return false;

	// Prepare to test V parameter
	Math::Vector3d qvec;
	qvec = Math::Vector3d::crossProduct(tvec, edge1);

	// Calculate V parameter and test bounds
	v = Math::Vector3d::dotProduct(dir, qvec) / det;
	if (v < 0.0f || u + v > 1.0f)
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	t = Math::Vector3d::dotProduct(edge2, qvec) / det;

	Math::Vector3d intersection;
	intersection = orig + dir * t;

	t = intersection.x();
	u = intersection.y();
	v = intersection.z();

	return true;
}

bool pickGetIntersect(const Math::Vector3d &lineStart, const Math::Vector3d &lineEnd,
								   const Math::Vector3d &v0, const Math::Vector3d &v1, const Math::Vector3d &v2,
								   Math::Vector3d &intersection, float &distance) {
	// compute plane's normal
	Math::Vector3d vertex;
	Math::Vector3d normal;

	Math::Vector3d edge1 = v1 - v0;
	Math::Vector3d edge2 = v2 - v1;

	normal = Math::Vector3d::crossProduct(edge1, edge2);
	normal.normalize();

	vertex = v0;

	Math::Vector3d direction, l1;
	float lineLength, distFromPlane, percentage;

	direction.x() = lineEnd.x() - lineStart.x(); // calculate the lines direction vector
	direction.y() = lineEnd.y() - lineStart.y();
	direction.z() = lineEnd.z() - lineStart.z();

	lineLength = Math::Vector3d::dotProduct(direction, normal); // This gives us the line length (the blue dot L3 + L4 in figure d)

	if (ABS(lineLength) < 0.0001f)
		return false;

	l1.x() = vertex.x() - lineStart.x(); // calculate vector L1 (the PINK line in figure d)
	l1.y() = vertex.y() - lineStart.y();
	l1.z() = vertex.z() - lineStart.z();

	distFromPlane = Math::Vector3d::dotProduct(l1, normal); // gives the distance from the plane (ORANGE Line L3 in figure d)
	percentage = distFromPlane / lineLength; // How far from Linestart , intersection is as a percentage of 0 to 1
	if (percentage < 0.0)
		return false;
	else if (percentage > 1.0)
		return false;

	distance = percentage; //record the distance from beginning of ray (0.0 -1.0)

	intersection.x() = lineStart.x() + direction.x() * percentage; // add the percentage of the line to line start
	intersection.y() = lineStart.y() + direction.y() * percentage;
	intersection.z() = lineStart.z() + direction.z() * percentage;

	return true;
}

void decomposeMatrixSimple(const DXMatrix *mat, DXVector3 *transVec, DXVector3 *scaleVec, DXQuaternion *rotQ) {
	*transVec = DXVector3(mat->matrix._41, mat->matrix._42, mat->matrix._43);
	*scaleVec = DXVector3(sqrtf(mat->matrix._11 * mat->matrix._11 + mat->matrix._21 * mat->matrix._21 + mat->matrix._31 * mat->matrix._31),
			sqrtf(mat->matrix._12 * mat->matrix._12 + mat->matrix._22 * mat->matrix._22 + mat->matrix._32 * mat->matrix._32),
			sqrtf(mat->matrix._13 * mat->matrix._13 + mat->matrix._23 * mat->matrix._23 + mat->matrix._33 * mat->matrix._33));

	DXQuaternion q;
	DXQuaternionRotationMatrix(&q, mat);

	*rotQ = q;
}

DXMatrix *matrixSetTranslation(DXMatrix *mat, DXVector3 *vec) {
	mat->matrix._41 = vec->_x;
	mat->matrix._42 = vec->_y;
	mat->matrix._43 = vec->_z;

	return mat;
}

DXMatrix *matrixSetRotation(DXMatrix *mat, DXVector3 *vec) {
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

#endif

} // End of namespace Wintermute
