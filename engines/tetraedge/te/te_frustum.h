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

#ifndef TETRAEDGE_TE_TE_FRUSTUM_H
#define TETRAEDGE_TE_TE_FRUSTUM_H

#include "tetraedge/te/te_camera.h"
#include "tetraedge/te/te_vector3f32.h"
#include "tetraedge/te/te_matrix4x4.h"

namespace Tetraedge {

class TeFrustum {
public:
	TeFrustum();

	void computeNormal(unsigned int val);
	void extractPlanAdd(const TeMatrix4x4 &matrix, uint param_2, uint param_3);
	void extractPlanSub(const TeMatrix4x4 &matrix, uint param_2, uint param_3);
	bool pointIsIn(const TeVector3f32 &pt);
	bool sphereIsIn(const TeVector3f32 &vec, float f);
	bool triangleIsIn(const TeVector3f32 *vertexes);
	void update(TeCamera *camera);

private:
	float planeLen(int num) const;
	float _m[24];

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_FRUSTUM_H
