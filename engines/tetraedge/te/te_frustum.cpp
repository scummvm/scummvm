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

#include "tetraedge/te/te_frustum.h"

namespace Tetraedge {

TeFrustum::TeFrustum() {
}

void TeFrustum::computeNormal(unsigned int val) {
	error("TODO: implement TeFrustum::computeNormal");
}

void TeFrustum::extractPlanAdd(const TeMatrix4x4 &matrix, uint dest, uint col) {
	_m[dest * 4 + 0] = matrix(0, 3) - matrix(0, col);
	_m[dest * 4 + 1] = matrix(1, 3) - matrix(1, col);
	_m[dest * 4 + 2] = matrix(2, 3) - matrix(2, col);
	_m[dest * 4 + 3] = matrix(3, 3) - matrix(3, col);
}

void TeFrustum::extractPlanSub(const TeMatrix4x4 &matrix, uint dest, uint col) {
	_m[dest * 4 + 0] = matrix(0, 3) + matrix(0, col);
	_m[dest * 4 + 1] = matrix(1, 3) + matrix(1, col);
	_m[dest * 4 + 2] = matrix(2, 3) + matrix(2, col);
	_m[dest * 4 + 3] = matrix(3, 3) + matrix(3, col);
}

bool TeFrustum::pointIsIn(const TeVector3f32 &pt) {
	error("TODO: Implement TeFrustum::pointIsIn");
}

float TeFrustum::planeLen(int num) const {
	assert(num >= 0 && num < 6);
	const float *p = _m + num * 4;
	float result = (float)sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
	if (result == 0)
		result = 1e-08f;
	return result;
}

bool TeFrustum::sphereIsIn(const TeVector3f32 &vec, float f) {
	error("TODO: Implement TeFrustum::sphereIsIn");
}

bool TeFrustum::triangleIsIn(const TeVector3f32 *verts) {
	for (unsigned int p = 0; p < 6; p++) {
		bool inside = true;
		for (unsigned int v = 0; v < 3; v++) {
			float *pm = _m + p * 4;
			if(pm[0] * verts[v].x() + pm[1] * verts[v].y() +
				pm[2] * verts[v].z() + pm[3] >= 0.0)
					inside = false;
		}
		if (!inside)
			return false;
	}
	
	return true;
}

void TeFrustum::update(TeCamera *camera) {
	const TeMatrix4x4 camMatrix = camera->worldTransformationMatrix();
	for (unsigned int plane = 0; plane < 6; plane++) {
		if (plane % 2)
			extractPlanAdd(camMatrix, plane, plane / 2);
		else
			extractPlanSub(camMatrix, plane, plane / 2);
		float len = planeLen(plane);
		float *p = _m + plane * 4;
		for (int i = 0; i < 4; i++)
			p[i] /= len;
	}
}


} // end namespace Tetraedge
