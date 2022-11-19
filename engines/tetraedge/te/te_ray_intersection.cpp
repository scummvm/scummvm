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

#include "tetraedge/te/te_ray_intersection.h"

namespace Tetraedge {

namespace TeRayIntersection {

TePickMesh *getMesh(const TeVector3f32 &param_1, const TeVector3f32 &param_2, const Common::Array<TePickMesh *> &pickMeshes,
			float param_4, float param_5, TeVector3f32 *param_6) {
	error("TODO: implement TeRayIntersection::getMesh");
}

int intersect(const TeVector3f32 &v1, const TeVector3f32 &v2, const TeVector3f32 &v3,
			  const TeVector3f32 &v4, const TeVector3f32 &v5, TeVector3f32 &vout, float &fout) {
	const TeVector3f32 v4_v3 = (v4 - v3);
	const TeVector3f32 v5_v3 = (v5 - v3);
	TeVector3f32 v = v4_v3 ^ v5_v3;

	if (v == TeVector3f32(0.0f, 0.0f, 0.0f))
		return -1;

	int result = -1;
	float f1 = v.dotProduct(v1 - v3);
	float f2 = v.dotProduct(v2);
	if (fabs(f2) > 1e-9) {
		f2 = -f1 / f2;
		fout = f2;
		if (f2 >= 0.0) {
			vout = v1 + (v2 * f2);
			float dot1 = v4_v3.dotProduct(v4_v3);
			float dot2 = v4_v3.dotProduct(v5_v3);
			float dot3 = v5_v3.dotProduct(v5_v3);
			const TeVector3f32 vout_v3 = vout - v3;
			float dots1 = (dot2 * dot2) - (dot1 * dot3);
			float dot4 = vout_v3.dotProduct(v4_v3);
			float dot5 = vout_v3.dotProduct(v5_v3);
			float dots2 = ((dot2 * dot5) - (dot3 * dot4)) / dots1;
			if (dots2 >= 0.0) {
				result = 0;
				if (dots2 <= 1.0) {
					float dots3 = (dot2 * dot4 - dot1 * dot5) / dots1;
					if (dots3 >= 0 && dots2 + dots3 <= 1.0)
						result = 1;
				}
			}
		}
	}
	return result;
}


}



} // end namespace Tetraedge
