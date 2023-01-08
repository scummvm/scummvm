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

#ifndef TETRAEDGE_TE_TE_RAY_INTERSECTION_H
#define TETRAEDGE_TE_TE_RAY_INTERSECTION_H

#include "common/array.h"
#include "tetraedge/te/te_vector3f32.h"

namespace Tetraedge {

class TePickMesh;

namespace TeRayIntersection {

TePickMesh *getMesh(const TeVector3f32 &param_1, const TeVector3f32 &param_2, const Common::Array<TePickMesh *> &pickMeshes,
			float param_4, float param_5, TeVector3f32 *param_6);

// Replaced with Math::Ray::intersectTriangle
//int intersect(const TeVector3f32 &rayPos, const TeVector3f32 &rayDir, const TeVector3f32 &v1,
//				const TeVector3f32 &v2, const TeVector3f32 &v3, TeVector3f32 &vout, float &fout);

} // end namespace TeRayIntersection

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_RAY_INTERSECTION_H
