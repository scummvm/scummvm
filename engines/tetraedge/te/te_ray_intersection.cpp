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

/*
// This is a version from https://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
int intersect(const TeVector3f32 &p, const TeVector3f32 &d, const TeVector3f32 &v0,
			  const TeVector3f32 &v1, const TeVector3f32 &v2, TeVector3f32 &hitPt, float &hitDist) {
	const TeVector3f32 e1 = v1 - v0;
	const TeVector3f32 e2 = v2 - v0;
	const TeVector3f32 h = TeVector3f32::crossProduct(d, e2);

	if (h == TeVector3f32())
		return -1;

	float a = e1.dotProduct(h);
	if (fabs(a) < 1e-6f)
		return 0;

	float f = 1.0f / a;
	const TeVector3f32 s = p - v0;
	float u = f * s.dotProduct(h);
	if (u < 0.0f || u > 1.0f)
		return 0;

	const TeVector3f32 q = TeVector3f32::crossProduct(s, e1);
	float v = f * d.dotProduct(q);

	if (v < 0.0f || u + v > 1.0f)
		return 0;

	float t = f * e2.dotProduct(q);

	if (t < 1e-6f)
		return 0;

	hitDist = t;
	hitPt = p + t * d;

	return 1;
}*/

/*
int intersect(const TeVector3f32 &rayPos, const TeVector3f32 &rayDir, const TeVector3f32 &v1,
			  const TeVector3f32 &v2, const TeVector3f32 &v3, TeVector3f32 &vout, float &fout) {
	const TeVector3f32 v2_v1 = v2 - v1;
	const TeVector3f32 v3_v1 = v3 - v1;
	const TeVector3f32 v = v2_v1 ^ v3_v1;

	if (v == TeVector3f32(0.0f, 0.0f, 0.0f))
		return -1;

	int result = -1;
	float f1 = v.dotProduct(rayPos - v1);
	float f2 = v.dotProduct(rayDir);
	if (fabs(f2) > 1e-9) {
		f2 = -f1 / f2;
		fout = f2;
		result = 0;
		if (f2 >= 0.0) {
			vout = rayPos + (rayDir * f2);
			float dot1 = v2_v1.dotProduct(v2_v1);
			float dot2 = v2_v1.dotProduct(v3_v1);
			float dot3 = v3_v1.dotProduct(v3_v1);
			const TeVector3f32 vout_v1 = vout - v1;
			float dots1 = dot2 * dot2 - dot1 * dot3;
			float dot4 = vout_v1.dotProduct(v2_v1);
			float dot5 = vout_v1.dotProduct(v3_v1);
			float dots2 = (dot2 * dot5 - dot3 * dot4) / dots1;
			if (dots2 >= 0.0 && dots2 <= 1.0) {
				float dots3 = (dot2 * dot4 - dot1 * dot5) / dots1;
				if (dots3 >= 0.0 && dots2 + dots3 <= 1.0)
					result = 1;
			}
		}
	} else {
		// Sorry about the logic.. this is what the decompiler gave me
		// and I'm not brave enough to figure it out.
		result = (-(uint)(f1 == -0.0) & 1) * 2;
	}
	return result;
}
*/

} // end namespace TeRayIntersection

} // end namespace Tetraedge
