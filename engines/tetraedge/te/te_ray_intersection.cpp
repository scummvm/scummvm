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

#include "math/ray.h"

#include "tetraedge/te/te_ray_intersection.h"
#include "tetraedge/te/te_pick_mesh.h"

namespace Tetraedge {

namespace TeRayIntersection {

// Note: the TeRayIntersection::intersect function is
// replaced by Math::Ray::intersectTriangle

TePickMesh *getMesh(const Math::Ray ray, const Common::Array<TePickMesh *> &pickMeshes,
			float maxDist, float minDist, TeVector3f32 *ptOut) {
	TeVector3f32 bestPt;
	TePickMesh *bestMesh = nullptr;

	for (auto *mesh : pickMeshes) {
		if (!mesh->flag())
			continue;
		for (uint i = 0; i < mesh->nTriangles(); i++) {
			TeVector3f32 v1;
			TeVector3f32 v2;
			TeVector3f32 v3;
			mesh->getTriangle(i, v1, v2, v3);
			float intersectDist;
			TeVector3f32 intersectPt;
			if (ray.intersectTriangle(v1, v2, v3, intersectPt, intersectDist) && intersectDist < maxDist && intersectDist >= minDist) {
				bestPt = intersectPt;
				bestMesh = mesh;
			}
		}
	}

	if (ptOut)
		*ptOut = bestPt;
	return bestMesh;
}

} // end namespace TeRayIntersection

} // end namespace Tetraedge
