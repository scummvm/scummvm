/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <stdint.h>

#include "dgCollisionConvex.h"
#include "dgBody.h"
#include "dgCollisionConvexHull.h"
#include "dgContact.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#define DG_MAX_EDGE_COUNT 2048
#define DG_MAX_CIRCLE_DISCRETE_STEPS dgFloat32(256.0f)

#define DG_CLIP_MAX_COUNT 512
#define DG_CLIP_MAX_POINT_COUNT 64
#define DG_MAX_VERTEX_CLIP_FACE 16

struct DG_CONVEX_FIXUP_FACE {
	dgInt32 m_vertex;
	DG_CONVEX_FIXUP_FACE *m_next;
};

dgInt32 dgCollisionConvex::m_rayCastSimplex[4][4] = {
	{0, 1, 2, 3},
	{0, 2, 3, 1},
	{2, 1, 3, 0},
	{1, 0, 3, 2},
};

dgTriplex dgCollisionConvex::m_hullDirs[] = {
	{dgFloat32(0.577350f), dgFloat32(-0.577350f), dgFloat32(0.577350f)},
	{dgFloat32(-0.577350f), dgFloat32(-0.577350f), dgFloat32(-0.577350f)},
	{dgFloat32(0.577350f), dgFloat32(-0.577350f), dgFloat32(-0.577350f)},
	{dgFloat32(-0.577350f), dgFloat32(0.577350f), dgFloat32(0.577350f)},
	{dgFloat32(0.577350f), dgFloat32(0.577350f), dgFloat32(-0.577350f)},
	{dgFloat32(-0.577350f), dgFloat32(0.577350f), dgFloat32(-0.577350f)},
	{dgFloat32(-0.577350f), dgFloat32(-0.577350f), dgFloat32(0.577350f)},
	{dgFloat32(0.577350f), dgFloat32(0.577350f), dgFloat32(0.577350f)},
	{dgFloat32(0.000000f), dgFloat32(-1.000000f), dgFloat32(0.000000f)},
	{dgFloat32(0.000000f), dgFloat32(1.000000f), dgFloat32(0.000000f)},
	{dgFloat32(1.000000f), dgFloat32(0.000000f), dgFloat32(0.000000f)},
	{dgFloat32(-1.000000f), dgFloat32(0.000000f), dgFloat32(0.000000f)},
	{dgFloat32(0.000000f), dgFloat32(0.000000f), dgFloat32(1.000000f)},
	{dgFloat32(0.000000f), dgFloat32(0.000000f), dgFloat32(-1.000000f)},
};

dgVector dgCollisionConvex::m_multiResDir[8];
dgVector dgCollisionConvex::m_multiResDir_sse[6];

// dgCollisionConvex::IntVector dgCollisionConvex::m_signMask = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
// dgCollisionConvex::IntVector dgCollisionConvex::m_triplexMask = {0xffffffff, 0xffffffff, 0xffffffff, 0x0};

dgInt32 dgCollisionConvex::m_iniliazised = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
dgCollisionConvex::dgCollisionConvex(dgMemoryAllocator *const allocator,
                                     dgUnsigned32 signature, const dgMatrix &matrix, dgCollisionID id) : dgCollision(allocator, signature, matrix, id), m_volume(dgFloat32(0.0f),
	                                             dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)),
	m_boxSize(
	    dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)),
	m_boxOrigin(
	    dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)) {
	m_rtti |= dgConvexCollision_RTTI;
	if (!m_iniliazised) {
		dgWorld::InitConvexCollision();
		m_iniliazised = 1;
	}

	m_edgeCount = 0;
	m_vertexCount = 0;
	m_vertex = NULL;
	m_simplex = NULL;
	m_userData = NULL;
	m_simplexVolume = dgFloat32(0.0f);
	m_boxMinRadius = dgFloat32(0.0f);
	m_boxMaxRadius = dgFloat32(0.0f);
	m_isTriggerVolume = false;

	for (dgUnsigned32 i = 0;
	        i < sizeof(m_supportVertexStarCuadrant) / sizeof(m_supportVertexStarCuadrant[0]); i++) {
		m_supportVertexStarCuadrant[i] = NULL;
	}
}

dgCollisionConvex::dgCollisionConvex(dgWorld *const world,
                                     dgDeserialize deserialization, void *const userData) : dgCollision(world, deserialization, userData) {
	dgInt32 isTrigger;
	if (!m_iniliazised) {
		dgWorld::InitConvexCollision();
		m_iniliazised = 1;
	}

	m_rtti |= dgConvexCollision_RTTI;
	m_edgeCount = 0;
	m_vertexCount = 0;
	m_vertex = NULL;
	m_simplex = NULL;
	m_userData = NULL;
	m_simplexVolume = dgFloat32(0.0f);
	m_boxMinRadius = dgFloat32(0.0f);
	m_boxMaxRadius = dgFloat32(0.0f);
	m_isTriggerVolume = false;

	deserialization(userData, &isTrigger, sizeof(dgInt32));
	m_isTriggerVolume = dgUnsigned32(isTrigger ? true : false);

	for (dgUnsigned32 i = 0;
	        i < sizeof(m_supportVertexStarCuadrant) / sizeof(m_supportVertexStarCuadrant[0]); i++) {
		m_supportVertexStarCuadrant[i] = NULL;
	}
}

dgCollisionConvex::~dgCollisionConvex() {
	if (m_vertex) {
		m_allocator->Free(m_vertex);
	}

	if (m_simplex) {
		m_allocator->Free(m_simplex);
	}
}

void dgCollisionConvex::SerializeLow(dgSerialize callback,
                                     void *const userData) const {
	dgCollision::SerializeLow(callback, userData);

	dgInt32 isTrigger = m_isTriggerVolume ? 1 : 0;
	callback(userData, &isTrigger, sizeof(dgInt32));
}

void *dgCollisionConvex::GetUserData() const {
	return m_userData;
}

void dgCollisionConvex::SetUserData(void *const userData) {
	m_userData = userData;
}

void dgCollisionConvex::SetVolumeAndCG() {
	dgPolyhedraMassProperties localData;
	dgVector faceVertex[512];
	dgStack<dgInt8> edgeMarks(m_edgeCount);

	memset(&edgeMarks[0], 0, sizeof(dgInt8) * m_edgeCount);
	for (dgInt32 i = 0; i < m_edgeCount; i++) {
		dgConvexSimplexEdge *const face = &m_simplex[i];
		if (!edgeMarks[i]) {
			dgConvexSimplexEdge *edge = face;
			dgInt32 count = 0;
			do {
				NEWTON_ASSERT((edge - m_simplex) >= 0);
				edgeMarks[dgInt32(edge - m_simplex)] = '1';
				faceVertex[count] = m_vertex[edge->m_vertex];
				count++;
				NEWTON_ASSERT(count < dgInt32(sizeof(faceVertex) / sizeof(faceVertex[0])));
				edge = edge->m_next;
			} while (edge != face);

			localData.AddCGFace(count, faceVertex);
		}
	}

	dgVector inertia;
	dgVector crossInertia;
	dgFloat32 scale = localData.MassProperties(m_volume, inertia, crossInertia);
	m_volume = m_volume.Scale(
	               dgFloat32(1.0f) / GetMax(scale, dgFloat32(1.0e-4f)));
	m_volume.m_w = scale;
	m_simplexVolume = m_volume.m_w;

	// set the table for quick calculation of support vertex
	dgInt32 count = sizeof(m_supportVertexStarCuadrant) / sizeof(m_supportVertexStarCuadrant[0]);
	for (dgInt32 i = 0; i < count; i++) {
		m_supportVertexStarCuadrant[i] = GetSupportEdge(m_multiResDir[i]);
	}
	NEWTON_ASSERT(
	    m_supportVertexStarCuadrant[4] == GetSupportEdge(m_multiResDir[0].Scale(-1.0f)));
	NEWTON_ASSERT(
	    m_supportVertexStarCuadrant[5] == GetSupportEdge(m_multiResDir[1].Scale(-1.0f)));
	NEWTON_ASSERT(
	    m_supportVertexStarCuadrant[6] == GetSupportEdge(m_multiResDir[2].Scale(-1.0f)));
	NEWTON_ASSERT(
	    m_supportVertexStarCuadrant[7] == GetSupportEdge(m_multiResDir[3].Scale(-1.0f)));

	// calculate the origin of the bound box of this primitive
	dgVector p0;
	dgVector p1;
	for (dgInt32 i = 0; i < 3; i++) {
		dgVector dir(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		             dgFloat32(0.0f));
		dir[i] = dgFloat32(-1.0f);
		p0[i] = SupportVertex(dir)[i];

		dir[i] = dgFloat32(1.0f);
		p1[i] = SupportVertex(dir)[i];
	}
	p0[3] = dgFloat32(0.0f);
	p1[3] = dgFloat32(0.0f);
	m_boxSize = (p1 - p0).Scale(dgFloat32(0.5f));
	m_boxOrigin = (p1 + p0).Scale(dgFloat32(0.5f));
	m_boxMinRadius = GetMin(m_boxSize.m_x, m_boxSize.m_y, m_boxSize.m_z);
	m_boxMaxRadius = dgSqrt(m_boxSize % m_boxSize);

	m_size_x.m_x = m_boxSize.m_x;
	m_size_x.m_y = m_boxSize.m_x;
	m_size_x.m_z = m_boxSize.m_x;
	m_size_x.m_w = dgFloat32(0.0f);

	m_size_y.m_x = m_boxSize.m_y;
	m_size_y.m_y = m_boxSize.m_y;
	m_size_y.m_z = m_boxSize.m_y;
	m_size_y.m_w = dgFloat32(0.0f);

	m_size_z.m_x = m_boxSize.m_z;
	m_size_z.m_y = m_boxSize.m_z;
	m_size_z.m_z = m_boxSize.m_z;
	m_size_z.m_w = dgFloat32(0.0f);
}

dgFloat32 dgCollisionConvex::GetDiscretedAngleStep(dgFloat32 radius) const {
	//  segments = GetMax (GetMin (dgFloor (radius * DG_MAX_CIRCLE_DISCRETE_STEPS) + 1.0f, 1024.0f), 128.0f);
	dgFloat32 segments = ClampValue(
	                         dgFloor(radius * DG_MAX_CIRCLE_DISCRETE_STEPS), dgFloat32(128.0f),
	                         dgFloat32(1024.0f));
	return dgPI2 / segments;
}

bool dgCollisionConvex::SanityCheck(dgPolyhedra &hull) const {
	dgPolyhedra::Iterator iter(hull);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_incidentFace < 0) {
			return false;
		}
		//      dgBigVector n0 (hull.FaceNormal (edge, &m_vertex[0].m_x, sizeof (dgBigVector)));
		dgEdge *ptr = edge;
		dgVector p0(m_vertex[edge->m_incidentVertex]);
		ptr = ptr->m_next;
		dgVector p1(m_vertex[ptr->m_incidentVertex]);
		dgVector e1(p1 - p0);
		dgVector n0(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		            dgFloat32(0.0f));
		for (ptr = ptr->m_next; ptr != edge; ptr = ptr->m_next) {
			dgVector p2(m_vertex[ptr->m_incidentVertex]);
			dgVector e2(p2 - p0);
			n0 += e1 * e2;
			e1 = e2;
		}

		ptr = edge;
		do {
			dgVector pp0(m_vertex[ptr->m_twin->m_incidentVertex]);
			for (dgEdge *neiborg = ptr->m_twin->m_next->m_next;
			        neiborg != ptr->m_twin; neiborg = neiborg->m_next) {
				dgVector pp1(m_vertex[neiborg->m_incidentVertex]);
				dgVector dpp(pp1 - pp0);
				dgFloat32 project = dpp % n0;
				if (project > dgFloat32(0.0f)) {
					return false;
				}
			}

			ptr = ptr->m_next;
		} while (ptr != edge);
	}

	return true;
}

// void dgCollisionConvex::DebugCollision (const dgBody& myBody, DebugCollisionMeshCallback callback) const
void dgCollisionConvex::DebugCollision(const dgMatrix &matrixPtr,
                                       OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt8 mark[DG_MAX_EDGE_COUNT];
	dgVector tmp[DG_MAX_EDGE_COUNT];
	dgTriplex vertex[DG_MAX_EDGE_COUNT];

	dgMatrix matrix(GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex(&tmp[0].m_x, sizeof(dgVector), &m_vertex[0].m_x,
	                        sizeof(dgVector), m_vertexCount);

	memset(mark, 0, sizeof(mark));
	for (dgInt32 i = 0; i < m_edgeCount; i++) {
		if (!mark[i]) {
			dgConvexSimplexEdge *const face = &m_simplex[i];
			dgConvexSimplexEdge *edge = face;
			dgInt32 count = 0;
			do {
				mark[edge - m_simplex] = '1';
				dgInt32 index = edge->m_vertex;
				vertex[count].m_x = tmp[index].m_x;
				vertex[count].m_y = tmp[index].m_y;
				vertex[count].m_z = tmp[index].m_z;
				count++;
				edge = edge->m_next;
			} while (edge != face);
			callback(userData, count, &vertex[0].m_x, 0);
		}
	}
}

void dgCollisionConvex::CalcAABB(const dgMatrix &matrix, dgVector &p0,
                                 dgVector &p1) const {
	dgVector origin(matrix.TransformVector(m_boxOrigin));
	dgVector size(
	    m_boxSize.m_x * dgAbsf(matrix[0][0]) + m_boxSize.m_y * dgAbsf(matrix[1][0]) + m_boxSize.m_z * dgAbsf(matrix[2][0]) + DG_MAX_COLLISION_PADDING,
	    m_boxSize.m_x * dgAbsf(matrix[0][1]) + m_boxSize.m_y * dgAbsf(matrix[1][1]) + m_boxSize.m_z * dgAbsf(matrix[2][1]) + DG_MAX_COLLISION_PADDING,
	    m_boxSize.m_x * dgAbsf(matrix[0][2]) + m_boxSize.m_y * dgAbsf(matrix[1][2]) + m_boxSize.m_z * dgAbsf(matrix[2][2]) + DG_MAX_COLLISION_PADDING,
	    dgFloat32(0.0f));

	p0 = origin - size;
	p1 = origin + size;

#ifdef DG_DEBUG_AABB
	dgInt32 i;
	dgVector q0;
	dgVector q1;
	dgMatrix trans(matrix.Transpose());
	for (i = 0; i < 3; i++) {
		q0[i] = matrix.m_posit[i] + matrix.RotateVector(SupportVertex(trans[i].Scale(-dgFloat32(1.0f))))[i];
		q1[i] = matrix.m_posit[i] + matrix.RotateVector(SupportVertex(trans[i]))[i];
	}

	dgVector err0(p0 - q0);
	dgVector err1(p1 - q1);
	dgFloat32 err;
	err = GetMax(size.m_x, size.m_y, size.m_z) * 0.5f;
	NEWTON_ASSERT((err0 % err0) < err * err);
	NEWTON_ASSERT((err1 % err1) < err * err);
#endif
}

void dgCollisionConvex::CalcAABBSimd(const dgMatrix &matrix, dgVector &p0,
                                     dgVector &p1) const {
#ifdef DG_BUILD_SIMD_CODE
	dgVector origin(matrix.TransformVectorSimd(m_boxOrigin));

	//  dgVector size (m_boxSize.m_x * dgAbsf(matrix[0][0]) + m_boxSize.m_y * dgAbsf(matrix[1][0]) + m_boxSize.m_z * dgAbsf(matrix[2][0]) + DG_MAX_COLLISION_PADDING,
	//                 m_boxSize.m_x * dgAbsf(matrix[0][1]) + m_boxSize.m_y * dgAbsf(matrix[1][1]) + m_boxSize.m_z * dgAbsf(matrix[2][1]) + DG_MAX_COLLISION_PADDING,
	//                 m_boxSize.m_x * dgAbsf(matrix[0][2]) + m_boxSize.m_y * dgAbsf(matrix[1][2]) + m_boxSize.m_z * dgAbsf(matrix[2][2]) + DG_MAX_COLLISION_PADDING,
	//                 dgFloat32 (0.0f));

	simd_type tmp =
	    simd_mul_add_v(
	        simd_mul_add_v(
	            simd_mul_add_v((simd_type &)m_aabb_padd, (simd_type &)m_size_x, simd_and_v((simd_type &)m_signMask, (simd_type &)matrix[0])),
	            (simd_type &)m_size_y, simd_and_v((simd_type &)m_signMask, (simd_type &)matrix[1])),
	        (simd_type &)m_size_z, simd_and_v((simd_type &)m_signMask, (simd_type &)matrix[2]));

	//  p0 = origin - size;
	//  p1 = origin + size;
	(simd_type &)p0 = simd_sub_v((simd_type &)origin, tmp);
	(simd_type &)p1 = simd_add_v((simd_type &)origin, tmp);

#else

#endif
}

dgConvexSimplexEdge *dgCollisionConvex::GetSupportEdge(
    const dgVector &dir) const {
	NEWTON_ASSERT(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	dgConvexSimplexEdge *edge = &m_simplex[0];
	dgFloat32 side0 = m_vertex[edge->m_vertex] % dir;
	dgConvexSimplexEdge *ptr = edge;
	do {
		dgFloat32 side1 = m_vertex[ptr->m_twin->m_vertex] % dir;
		if (side1 > side0) {
			side0 = side1;
			edge = ptr->m_twin;
			ptr = edge;
		}
		ptr = ptr->m_twin->m_next;
	} while (ptr != edge);

	return edge;
}

void dgCollisionConvex::CalculateInertia(void *userData, int indexCount,
        const dgFloat32 *faceVertex, int faceId) {
	//  dgConvexMassData& localData = *((dgConvexMassData*) userData);
	dgPolyhedraMassProperties &localData =
	    *((dgPolyhedraMassProperties *)userData);
	localData.AddInertiaFace(indexCount, faceVertex);
}

dgFloat32 dgCollisionConvex::CalculateMassProperties(dgVector &inertia,
        dgVector &crossInertia, dgVector &centerOfMass) const {
	dgFloat32 volume;

	//  dgConvexMassData localData;
	dgPolyhedraMassProperties localData;
	DebugCollision(dgGetIdentityMatrix(), CalculateInertia, &localData);
	volume = localData.MassProperties(centerOfMass, inertia, crossInertia);
	/*
	 #ifdef _DEBUG
	 dgFloat32 volume1;
	 dgVector inertia1;
	 dgVector crossInertia1;
	 dgVector centerOfMass1;
	 dgConvexMassData localData1;
	 DebugCollision (dgGetIdentityMatrix(), CalculateInertia, &localData1);
	 volume1 = localData1.MassProperties (centerOfMass1, inertia1, crossInertia1);
	 NEWTON_ASSERT (dgAbsf (volume1 - volume) < dgFloat32 (1.0e-3f));

	 for (dgInt32 i = 0; i < 3; i ++) {
	 NEWTON_ASSERT (dgAbsf (inertia[i] - inertia1[i]) < dgFloat32 (1.0e-3f));
	 NEWTON_ASSERT (dgAbsf (crossInertia[i] - crossInertia1[i]) < dgFloat32 (1.0e-3f));
	 NEWTON_ASSERT (dgAbsf (centerOfMass[i] - centerOfMass1[i]) < dgFloat32 (1.0e-3f));
	 }
	 #endif
	 */
	return volume;
}

void dgCollisionConvex::CalculateInertia(dgVector &inertiaOut,
        dgVector &originOut) const {
	dgFloat32 volume;
	dgFloat32 invVolume;
	dgVector crossInertia;
	dgVector inertia;
	dgVector origin;
#define DG_MIN_SIDE dgFloat32(1.0e-2f)
#define DG_MIN_VOLUME (DG_MIN_SIDE * DG_MIN_SIDE * DG_MIN_SIDE)

	volume = CalculateMassProperties(inertia, crossInertia, origin);

	invVolume = dgFloat32(1.0f) / GetMax(volume, DG_MIN_VOLUME);

	origin = origin.Scale(invVolume);
	dgVector central2(origin.CompProduct(origin));

	inertia = inertia.Scale(invVolume);
	crossInertia = crossInertia.Scale(invVolume);

	inertia.m_x -= (central2.m_y + central2.m_z);
	inertia.m_y -= (central2.m_z + central2.m_x);
	inertia.m_z -= (central2.m_x + central2.m_y);
	crossInertia.m_x += (origin.m_y * origin.m_z);
	crossInertia.m_y += (origin.m_z * origin.m_x);
	crossInertia.m_z += (origin.m_x * origin.m_y);

	originOut.m_x = origin.m_x;
	originOut.m_y = origin.m_y;
	originOut.m_z = origin.m_z;

	inertiaOut.m_x = inertia.m_x;
	inertiaOut.m_y = inertia.m_y;
	inertiaOut.m_z = inertia.m_z;
	if (inertiaOut.m_x < dgFloat32(1.0e-3f))
		inertiaOut.m_x = dgFloat32(1.0e-3f);
	if (inertiaOut.m_y < dgFloat32(1.0e-3f))
		inertiaOut.m_y = dgFloat32(1.0e-3f);
	if (inertiaOut.m_z < dgFloat32(1.0e-3f))
		inertiaOut.m_z = dgFloat32(1.0e-3f);

	NEWTON_ASSERT(inertiaOut[0] > 0.0f);
	NEWTON_ASSERT(inertiaOut[1] > 0.0f);
	NEWTON_ASSERT(inertiaOut[2] > 0.0f);
}

dgFloat32 dgCollisionConvex::GetVolume() const {
#ifdef _DEBUG
	dgFloat32 volume;
	dgVector inertia;
	dgVector centerOfMass;
	dgVector crossInertia;
	volume = CalculateMassProperties(inertia, crossInertia, centerOfMass);
	NEWTON_ASSERT(m_volume.m_w >= dgFloat32(0.7f) * volume);
#endif

	return m_volume.m_w;
}

dgFloat32 dgCollisionConvex::GetBoxMinRadius() const {
	return m_boxMinRadius;
}

dgFloat32 dgCollisionConvex::GetBoxMaxRadius() const {
	return m_boxMaxRadius;
}

dgInt32 dgCollisionConvex::RayCastClosestFace(dgVector *tetrahedrum,
        const dgVector &origin, dgFloat32 &pointDist) const {
//	dgInt32 i;
//	dgInt32 j;
//	dgInt32 i0;
//	dgInt32 i1;
//	dgInt32 i2;
//	dgInt32 i3;
//	dgInt32 face;
//	dgInt32 plane;
//	dgFloat32 dist;
//	dgFloat32 maxDist;
//	dgVector normal;
#define PLANE_MAX_ITERATION 128

	dgInt32 face = 0;
	dgInt32 plane = -1;
	dgFloat32 maxDist = dgFloat32(1.0e10f);

	dgInt32 j = 0;
	for (; (face != -1) && (j < PLANE_MAX_ITERATION); j++) {
		face = -1;

		// initialize distance to zero (very important)
		maxDist = dgFloat32(0.0f);
		dgVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                dgFloat32(0.0f));
		for (dgInt32 i = 0; i < 4; i++) {
			dgInt32 i0 = m_rayCastSimplex[i][0];
			dgInt32 i1 = m_rayCastSimplex[i][1];
			dgInt32 i2 = m_rayCastSimplex[i][2];
			const dgVector &p0 = tetrahedrum[i0];
			const dgVector &p1 = tetrahedrum[i1];
			const dgVector &p2 = tetrahedrum[i2];
			dgVector e0(p1 - p0);
			dgVector e1(p2 - p0);
			dgVector n(e0 * e1);

			dgFloat32 dist = n % n;
			if (dist > dgFloat32(1.0e-24f)) {
				n = n.Scale(dgRsqrt(n % n));
				dist = n % (origin - p0);
				// find the plane farther away from the origin
				if (dist > maxDist) {
					maxDist = dist;
					normal = n;
					face = i;
				}
			}
		}

		if (face != -1) {
			dgInt32 j0 = m_rayCastSimplex[face][0];
			dgVector p(SupportVertex(normal));
			dgFloat32 dist = normal % (p - tetrahedrum[j0]);
			if (dist < dgFloat32(1.0e-6f)) {
				plane = face;
				break;
			}

			dgInt32 j1 = m_rayCastSimplex[face][1];
			dgInt32 j3 = m_rayCastSimplex[face][3];
			tetrahedrum[j3] = p;
			Swap(tetrahedrum[j0], tetrahedrum[j1]);

			dgInt32 i0 = m_rayCastSimplex[0][0];
			dgInt32 i1 = m_rayCastSimplex[0][1];
			dgInt32 i2 = m_rayCastSimplex[0][2];
			dgInt32 i3 = m_rayCastSimplex[0][3];

			dgVector e0(tetrahedrum[i1] - tetrahedrum[i0]);
			dgVector e1(tetrahedrum[i2] - tetrahedrum[i0]);
			dgVector e2(tetrahedrum[i3] - tetrahedrum[i0]);

			dist = (e1 * e0) % e2;
			// return (volume >= dgFloat32 (0.0f));
			if (dist <= dgFloat32(0.0f)) {
				//NEWTON_ASSERT (0);
				Swap(tetrahedrum[1], tetrahedrum[2]);
				//NEWTON_ASSERT (CheckTetraHedronVolume ());
			}
		}
	}

	if (j >= PLANE_MAX_ITERATION) {
		plane = -1;
		maxDist = dgFloat32(1.0e10f);
		if (face != -1) {
			dgInt32 i0 = m_rayCastSimplex[face][0];
			dgInt32 i1 = m_rayCastSimplex[face][1];
			dgInt32 i2 = m_rayCastSimplex[face][2];
			const dgVector &p0 = tetrahedrum[i0];
			const dgVector &p1 = tetrahedrum[i1];
			const dgVector &p2 = tetrahedrum[i2];
			dgVector e0(p1 - p0);
			dgVector e1(p2 - p0);
			dgVector n(e0 * e1);
			dgFloat32 dist = n % n;
			if (dist > dgFloat32(1.0e-24f)) {
				n = n.Scale(-dgRsqrt(n % n));
				dgVector p(SupportVertex(n));
				dist = n % (p - p0);
				if (dist < dgFloat32(1.0e-3f)) {
					plane = face;
				}
			}
		}
	}
	pointDist = maxDist;
	return plane;
}

bool dgCollisionConvex::RayHitBox(const dgVector &localP0,
                                  const dgVector &localP1) const {
	dgFloat32 tMin;
	dgFloat32 tMax;

	tMin = dgFloat32(0.0f);
	tMax = dgFloat32(1.0f);

	dgVector p0(localP0 - m_boxOrigin);
	dgVector p1(localP1 - m_boxOrigin);
	//  dgVector dp (p1 - p0);
	for (int i = 0; i < 3; i++) {
		dgFloat32 t0;
		dgFloat32 t1;
		dgFloat32 den;

		den = p1[i] - p0[i];
		if (dgAbsf(den) < dgFloat32(1.0e-6f)) {
			if (p0[i] < -m_boxSize[i]) {
				return false;
			}
			if (p0[i] > m_boxSize[i]) {
				return false;
			}
		} else {

			den = dgFloat32(1.0f) / den;
			t0 = (-m_boxSize[i] - p0[i]) * den;
			t1 = (m_boxSize[i] - p0[i]) * den;
			if (t0 > t1) {
				dgFloat32 t;
				t = t0;
				t0 = t1;
				t1 = t;
			}

			if (tMin < t0) {
				tMin = t0;
			}

			if (tMax > t1) {
				tMax = t1;
			}

			if (tMin > tMax) {
				return false;
			}
		}
	}
	return true;
}

dgVector dgCollisionConvex::CalculateVolumeIntegral(
    const dgMatrix &globalMatrix, GetBuoyancyPlane buoyancyPlane,
    void *context) const {
	dgFloat32 volume;

	dgVector cg(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	            dgFloat32(0.0f));
	if (buoyancyPlane) {
		dgPlane globalPlane;
		// if (buoyancyPlane (GetUserData(), context, globalMatrix, globalPlane)) {
		if (buoyancyPlane((void *)(intptr_t)SetUserDataID(), context, globalMatrix,
		                  globalPlane)) {
			globalPlane = globalMatrix.UntransformPlane(globalPlane);
			cg = CalculateVolumeIntegral(globalPlane);
		}
	}
	//  dgVector cg  (CalculateVolumeIntegral (plane));

	volume = cg.m_w;
	cg = globalMatrix.TransformVector(cg);
	cg.m_w = volume;

	return cg;
}

// dgVector dgCollisionConvex::GetLocalCG () const
//{
//	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
// }

dgVector dgCollisionConvex::CalculateVolumeIntegral(const dgPlane &plane) const {
	dgInt32 i;
	dgInt32 count;
	dgInt32 index0;
	dgInt32 positive;
	dgInt32 negative;
	dgFloat32 size0;
	dgFloat32 size1;
	dgConvexSimplexEdge *ptr;
	dgConvexSimplexEdge *edge;
	dgConvexSimplexEdge *face;
	dgConvexSimplexEdge *capEdge;
	dgInt8 mark[DG_MAX_EDGE_COUNT];
	dgFloat32 test[DG_MAX_EDGE_COUNT];
	dgVector faceVertex[256];

	dgVector cg(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	            dgFloat32(0.0f));

	positive = 0;
	negative = 0;
	for (i = 0; i < m_vertexCount; i++) {
		test[i] = plane.Evalue(m_vertex[i]);
		if (test[i] > dgFloat32(1.0e-5f)) {
			positive++;
		} else if (test[i] < -dgFloat32(1.0e-5f)) {
			negative++;
		} else {
			test[i] = dgFloat32(0.0f);
		}
	}

	if (positive == m_vertexCount) {
		return cg;
	}

	if (negative == m_vertexCount) {
		//      return m_volume.Scale (m_volume.m_w);
		dgVector volume(m_volume);
		volume.m_w = m_simplexVolume;
		return volume;
	}

	//  dgConvexMassData localData;
	dgPolyhedraMassProperties localData;
	capEdge = NULL;

	//  m_mark ++;
	memset(mark, 0, sizeof(mark));
	for (i = 0; i < m_edgeCount; i++) {
		if (!mark[i]) {
			face = &m_simplex[i];
			edge = face;
			count = 0;
			size0 = test[edge->m_prev->m_vertex];
			do {
				// edge->m_mark = m_mark;
				mark[edge - m_simplex] = '1';
				size1 = test[edge->m_vertex];
				if (size0 <= dgFloat32(0.0f)) {
					faceVertex[count] = m_vertex[edge->m_prev->m_vertex];
					count++;
					if (size1 > dgFloat32(0.0f)) {
						dgVector dp(
						    m_vertex[edge->m_vertex] - m_vertex[edge->m_prev->m_vertex]);
						faceVertex[count] = m_vertex[edge->m_prev->m_vertex] - dp.Scale(size0 / (plane % dp));
						count++;
					}
				} else if (size1 < dgFloat32(0.0f)) {
					dgVector dp(
					    m_vertex[edge->m_vertex] - m_vertex[edge->m_prev->m_vertex]);
					faceVertex[count] = m_vertex[edge->m_prev->m_vertex] - dp.Scale(size0 / (plane % dp));
					count++;
					NEWTON_ASSERT(count < dgInt32(sizeof(faceVertex) / sizeof(faceVertex[0])));
				}

				if (!capEdge) {
					if ((size1 > dgFloat32(0.0f)) && (size0 < dgFloat32(0.0f))) {
						capEdge = edge->m_prev->m_twin;
					}
				}

				size0 = size1;
				edge = edge->m_next;
			} while (edge != face);

			if (count) {
				// dgPlane plane (faceVertex[0], faceVertex[1], faceVertex[2]);
				// plane = plane.Scale (dgRsqrt ((plane % plane) + dgFloat32(1.0e-8f)));
				// localData.VolumeIntegrals(count, plane, faceVertex);
				localData.AddCGFace(count, faceVertex);
			}
		}
	}

	if (capEdge) {
		count = 0;
		edge = capEdge;
		do {
			dgVector dp(m_vertex[edge->m_twin->m_vertex] - m_vertex[edge->m_vertex]);
			faceVertex[count] = m_vertex[edge->m_vertex] - dp.Scale(test[edge->m_vertex] / (plane % dp));
			count++;
			if (count == 127) {
				// something is wrong return zero
				return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				                dgFloat32(0.0f));
			}

			for (ptr = edge->m_next; ptr != edge; ptr = ptr->m_next) {
				index0 = ptr->m_twin->m_vertex;
				if (test[index0] > dgFloat32(0.0f)) {
					index0 = ptr->m_vertex;
					if (test[index0] < dgFloat32(0.0f)) {
						break;
					}
				}
			}
			edge = ptr->m_twin;
		} while (edge != capEdge);
		// localData.VolumeIntegrals(count, plane, faceVertex);
		localData.AddCGFace(count, faceVertex);
	}

	//  cg.m_x = localData.m_T1[0];
	//  cg.m_y = localData.m_T1[1];
	//  cg.m_z = localData.m_T1[2];
	//  cg.m_w = localData.m_T0;

	dgVector inertia;
	dgVector crossInertia;
	size0 = localData.MassProperties(cg, inertia, crossInertia);
	cg = cg.Scale(dgFloat32(1.0f) / GetMax(size0, dgFloat32(1.0e-4f)));
	cg.m_w = size0;
	return cg;
}

dgVector dgCollisionConvex::SupportVertex(const dgVector &direction) const {
	const dgVector dir(direction.m_x, direction.m_y, direction.m_z, dgFloat32(0.0f));
	NEWTON_ASSERT(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	dgInt32 index = 0;
	dgFloat32 side0 = dgFloat32(-1.0e20f);
	for (dgInt32 i = 0; i < 4; i++) {
		dgFloat32 side1 = m_multiResDir[i] % dir;
		if (side1 > side0) {
			side0 = side1;
			index = i;
		}
		side1 *= dgFloat32(-1.0f);
		if (side1 > side0) {
			side0 = side1;
			index = i + 4;
		}
	}

	dgInt16 cache[16];
	memset(cache, -1, sizeof(cache));
	dgConvexSimplexEdge *edge = m_supportVertexStarCuadrant[index];
	index = edge->m_vertex;
	side0 = m_vertex[index] % dir;

	cache[index & (sizeof(cache) / sizeof(cache[0]) - 1)] = dgInt16(index);
	dgConvexSimplexEdge *ptr = edge;
	dgInt32 maxCount = 128;
	do {
		dgInt32 index1 = ptr->m_twin->m_vertex;
		if (cache[index1 & (sizeof(cache) / sizeof(cache[0]) - 1)] != index1) {
			cache[index1 & (sizeof(cache) / sizeof(cache[0]) - 1)] = dgInt16(index1);
			dgFloat32 side1 = m_vertex[index1] % dir;
			if (side1 > side0) {
				index = index1;
				side0 = side1;
				edge = ptr->m_twin;
				ptr = edge;
			}
		}
		ptr = ptr->m_twin->m_next;
		maxCount--;
	} while ((ptr != edge) && maxCount);
	NEWTON_ASSERT(maxCount);

	NEWTON_ASSERT(index != -1);
	return m_vertex[index];
}

dgVector dgCollisionConvex::SupportVertexSimd(const dgVector &direction) const {
#ifdef DG_BUILD_SIMD_CODE
	NEWTON_ASSERT(dgAbsf(direction % direction - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	simd_type dir_x = simd_set1(direction.m_x);
	simd_type dir_y = simd_set1(direction.m_y);
	simd_type dir_z = simd_set1(direction.m_z);
	simd_type dot0 = simd_mul_add_v(simd_mul_add_v(simd_mul_v(dir_x, *(simd_type *)&m_multiResDir_sse[0]),
	                                dir_y, *(simd_type *)&m_multiResDir_sse[1]),
	                                dir_z, *(simd_type *)&m_multiResDir_sse[2]);
	simd_type dot1 = simd_mul_v(dot0, *(simd_type *)&m_negOne);
	simd_type mask = simd_cmpgt_v(dot0, dot1);
	dot0 = simd_max_v(dot0, dot1);
	simd_type entry = simd_or_v(simd_and_v(*(simd_type *)&m_index_0123, mask), simd_andnot_v(*(simd_type *)&m_index_4567, mask));

	dot1 = simd_move_hl_v(dot0, dot0);
	mask = simd_cmpgt_v(dot0, dot1);
	dot0 = simd_max_v(dot0, dot1);
	entry = simd_or_v(simd_and_v(entry, mask), simd_andnot_v(simd_move_hl_v(entry, entry), mask));

	mask = simd_cmpgt_s(dot0, simd_permut_v(dot0, dot0, PURMUT_MASK(3, 2, 1, 1)));

	dgInt32 index = simd_store_is(simd_or_v(simd_and_v(entry, mask), simd_andnot_v(simd_permut_v(entry, entry, PURMUT_MASK(3, 2, 1, 1)), mask)));
	dgConvexSimplexEdge *edge = m_supportVertexStarCuadrant[index];
	index = edge->m_vertex;

	simd_type dir = simd_set(direction.m_x, direction.m_y, direction.m_z, dgFloat32(0.0f));
	NEWTON_ASSERT(m_vertex[edge->m_vertex].m_w == dgFloat32(1.0f));

	simd_type side0 = simd_mul_v(*(simd_type *)&m_vertex[edge->m_vertex], dir);
	side0 = simd_add_s(simd_add_v(side0, simd_move_hl_v(side0, side0)), simd_permut_v(side0, side0, PURMUT_MASK(3, 3, 3, 1)));

	dgInt16 cache[16];
	memset(cache, -1, sizeof(cache));
	cache[index & (sizeof(cache) / sizeof(cache[0]) - 1)] = dgInt16(index);

	dgConvexSimplexEdge *ptr = edge;
	dgInt32 maxCount = 128;
	do {
		dgInt32 index1 = ptr->m_twin->m_vertex;
		if (cache[index1 & (sizeof(cache) / sizeof(cache[0]) - 1)] != index1) {
			cache[index1 & (sizeof(cache) / sizeof(cache[0]) - 1)] = dgInt16(index1);
			NEWTON_ASSERT(m_vertex[index1].m_w == dgFloat32(1.0f));
			simd_type side1 = simd_mul_v(*(simd_type *)&m_vertex[index1], dir);
			side1 = simd_add_s(simd_add_v(side1, simd_move_hl_v(side1, side1)), simd_permut_v(side1, side1, PURMUT_MASK(3, 3, 3, 1)));
			if (simd_store_is(simd_cmpgt_s(side1, side0))) {
				index = index1;
				side0 = side1;
				edge = ptr->m_twin;
				ptr = edge;
			}
		}
		ptr = ptr->m_twin->m_next;
		maxCount--;
	} while ((ptr != edge) && maxCount);
	NEWTON_ASSERT(maxCount);

	NEWTON_ASSERT(index != -1);
	return m_vertex[index];
#else
	return 0;
#endif
}

bool dgCollisionConvex::SanityCheck(dgInt32 count, const dgVector &normal,
                                    dgVector *const contactsOut) const {
	if (count > 1) {
		dgInt32 j = count - 1;
		for (dgInt32 i = 0; i < count; i++) {
			dgVector error(contactsOut[i] - contactsOut[j]);
			//          NEWTON_ASSERT ((error % error) > dgFloat32 (1.0e-20f));
			if ((error % error) <= dgFloat32(1.0e-20f)) {
				return false;
			}
			j = i;
		}

		if (count >= 3) {
			dgVector n(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			           dgFloat32(0.0f));
			dgVector e0(contactsOut[1] - contactsOut[0]);
			for (dgInt32 i = 2; i < count; i++) {
				dgVector e1(contactsOut[i] - contactsOut[0]);
				n += e0 * e1;
				e0 = e1;
			}
			NEWTON_ASSERT((n % n) > dgFloat32(0.0f));
			n = n.Scale(dgFloat32(1.0f) / dgSqrt(n % n));
			dgFloat32 projection;
			projection = n % normal;
			NEWTON_ASSERT(projection > dgFloat32(0.9f));
			if (projection < dgFloat32(0.9f)) {
				return false;
			}

			e0 = contactsOut[count - 1] - contactsOut[count - 2];
			j = count - 1;
			for (dgInt32 i = 0; i < count; i++) {
				dgVector e1(contactsOut[i] - contactsOut[j]);
				dgVector nx(e0 * e1);
				dgFloat32 error = nx % normal;
				NEWTON_ASSERT(error >= dgFloat32(-1.0e-4f));
				if (error < dgFloat32(-1.0e-4f)) {
					return false;
				}
				j = i;
				e0 = e1;
			}
		}
	}
	return true;
}

dgInt32 dgCollisionConvex::SimplifyClipPolygon(dgInt32 count,
        const dgVector &normal, dgVector *const polygon) const {
	dgInt8 mark[DG_MAX_VERTEX_CLIP_FACE * 8];
	dgInt8 buffer[8 * DG_MAX_VERTEX_CLIP_FACE * (sizeof(dgInt32) + sizeof(dgFloat32))];
	for (uint i = 0; i < ARRAYSIZE(buffer); i++) buffer[i] = 0;

	NEWTON_ASSERT(count < dgInt32(sizeof(mark) / sizeof(mark[0])));
	dgUpHeap<dgInt32, dgFloat32> sortHeap(buffer, sizeof(buffer));

	while (count > DG_MAX_VERTEX_CLIP_FACE) {
		sortHeap.Flush();

		dgInt32 i0 = count - 2;
		dgInt32 i1 = count - 1;
		//      dgInt32 i2 = 0;
		for (dgInt32 i2 = 0; i2 < count; i2++) {
			mark[i2] = 0;

			dgVector e0 = polygon[i1] - polygon[i0];
			dgVector e1 = polygon[i2] - polygon[i0];
			dgFloat32 area = dgAbsf(normal % (e0 * e1));

			sortHeap.Push(i1, area);

			i0 = i1;
			i1 = i2;
		}

		dgInt32 removeCount = count - DG_MAX_VERTEX_CLIP_FACE;
		while (sortHeap.GetCount() && removeCount) {
			dgInt32 ii1 = sortHeap[0];
			sortHeap.Pop();

			dgInt32 ii0 = (ii1 - 1) >= 0 ? ii1 - 1 : count - 1;
			dgInt32 ii2 = (ii1 + 1) < count ? ii1 + 1 : 0;

			if (!(mark[ii0] || mark[ii2])) {
				mark[ii1] = 1;
				removeCount--;
			}
		}

		i0 = 0;
		for (dgInt32 ii1 = 0; ii1 < count; ii1++) {
			if (!mark[ii1]) {
				polygon[i0] = polygon[ii1];
				i0++;
			}
		}
		count = i0;
	}

	return count;
}

dgInt32 dgCollisionConvex::RectifyConvexSlice(dgInt32 count,
        const dgVector &normal, dgVector *const contactsOut) const {
	DG_CONVEX_FIXUP_FACE linkFace[DG_CLIP_MAX_POINT_COUNT * 2];

	NEWTON_ASSERT(count > 2);

	DG_CONVEX_FIXUP_FACE *poly = &linkFace[0];
	for (dgInt32 i = 0; i < count; i++) {
		contactsOut[i].m_w = dgFloat32(1.0f);
		linkFace[i].m_vertex = i;
		linkFace[i].m_next = &linkFace[i + 1];
	}
	linkFace[count - 1].m_next = &linkFace[0];

	dgInt32 restart = 1;
	dgInt32 tmpCount = count;
	while (restart && (tmpCount >= 2)) {
		restart = 0;
		DG_CONVEX_FIXUP_FACE *ptr = poly;
		dgInt32 loops = tmpCount;
		do {
			dgInt32 i0 = ptr->m_vertex;
			dgInt32 i1 = ptr->m_next->m_vertex;
			dgVector error(contactsOut[i1] - contactsOut[i0]);
			dgFloat32 dist2 = error % error;
			if (dist2 < dgFloat32(0.003f * 0.003f)) {
				if (ptr->m_next == poly) {
					poly = ptr;
				}
				restart = 1;
				tmpCount--;
				contactsOut[i1].m_w = dgFloat32(0.0f);
				ptr->m_next = ptr->m_next->m_next;
			} else {
				ptr = ptr->m_next;
			}

			loops--;
		} while (loops);
	}

	restart = 1;
	while (restart && (tmpCount >= 3)) {
		restart = 0;
		DG_CONVEX_FIXUP_FACE *ptr = poly;
		dgInt32 loops = tmpCount;
		do {
			dgInt32 i0 = ptr->m_vertex;
			dgInt32 i1 = ptr->m_next->m_vertex;
			dgInt32 i2 = ptr->m_next->m_next->m_vertex;
			dgVector e0(contactsOut[i2] - contactsOut[i1]);
			dgVector e1(contactsOut[i0] - contactsOut[i1]);
			dgVector n(e0 * e1);
			dgFloat32 area = normal % n;
			if (area <= dgFloat32(1.0e-5f)) {
				if (ptr->m_next == poly) {
					poly = ptr;
				}
				restart = 1;
				tmpCount--;
				contactsOut[i1].m_w = dgFloat32(0.0f);
				ptr->m_next = ptr->m_next->m_next;
			} else {
				ptr = ptr->m_next;
			}
			loops--;
		} while (loops);
	}

	if (tmpCount < count) {
		dgInt32 newCount = 0;
		for (; newCount < count; newCount++) {
			if (contactsOut[newCount].m_w == dgFloat32(0.0f)) {
				break;
			}
		}

		for (dgInt32 i = newCount + 1; i < count; i++) {
			if (contactsOut[i].m_w != dgFloat32(0.0f)) {
				contactsOut[newCount] = contactsOut[i];
				newCount++;
			}
		}
		count = newCount;
		NEWTON_ASSERT(tmpCount == count);
	}

	if (count > DG_MAX_VERTEX_CLIP_FACE) {
		count = SimplifyClipPolygon(count, normal, contactsOut);
	}

	NEWTON_ASSERT(SanityCheck(count, normal, contactsOut));
	return count;
}

dgInt32 dgCollisionConvex::CalculatePlaneIntersectionSimd(
    const dgVector &normal, const dgVector &origin,
    dgVector *const contactsOut) const {
#ifdef DG_BUILD_SIMD_CODE
	//  dgInt32 count;
	//  dgFloat32 side0;
	//  dgFloat32 side1;
	//  dgInt32 maxCount;
	//  dgConvexSimplexEdge *ptr;
	//  dgConvexSimplexEdge *ptr1;
	//  dgConvexSimplexEdge* edge;
	//  dgConvexSimplexEdge *firstEdge;
	//  simd_type tmp;
	//  simd_type den;
	//  simd_type deltaP;
	//  simd_type planeSimdD;

	dgConvexSimplexEdge *edge = &m_simplex[0];
	dgPlane plane(normal, -(normal % origin));
	simd_type planeSimdD = *(simd_type *)&plane;

	NEWTON_ASSERT(m_vertex[edge->m_vertex].m_w == dgFloat32(1.0f));
	//  side0 = plane.Evalue(m_vertex[edge->m_vertex]);
	//  simd_type tmp_ = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[edge->m_vertex]);
	//  tmp_ = simd_add_v (tmp_, simd_move_hl_v (tmp_, tmp_));
	//  tmp_ = simd_add_s(tmp_, simd_permut_v (tmp_, tmp_, PURMUT_MASK (3,3,3,1)));
	//  dgFloat32 side0;
	//  dgFloat32 side1;
	//  simd_store_s (tmp_, &side0);

	simd_type side0 =
	    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[edge->m_vertex]);
	side0 = simd_add_v(side0, simd_move_hl_v(side0, side0));
	side0 = simd_add_s(side0, simd_permut_v(side0, side0, PURMUT_MASK(3, 3, 3, 1)));
	simd_type side1 = side0;

	simd_type zero = simd_set1(dgFloat32(0.0f));
	dgConvexSimplexEdge *firstEdge = NULL;
	//  if (side0 > dgFloat32 (0.0f)) {
	if (simd_store_is(simd_cmpgt_s(side0, zero))) {
		dgConvexSimplexEdge *ptr = edge;
		do {
			NEWTON_ASSERT(m_vertex[ptr->m_twin->m_vertex].m_w == dgFloat32(1.0f));
			//          side1 = plane.Evalue (m_vertex[ptr->m_twin->m_vertex]);
			//          simd_type tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr->m_twin->m_vertex]);
			//          tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
			//          tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
			//          simd_store_s (tmp, &side1);

			side1 =
			    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr->m_twin->m_vertex]);
			side1 = simd_add_v(side1, simd_move_hl_v(side1, side1));
			side1 =
			    simd_add_s(side1, simd_permut_v(side1, side1, PURMUT_MASK(3, 3, 3, 1)));
			//          if (side1 < side0) {
			if (simd_store_is(simd_cmplt_s(side1, side0))) {
				//              xxx = simd_store_is (simd_cmplt_s(side1, zero));
				//              if (side1 < dgFloat32 (0.0f)) {
				if (simd_store_is(simd_cmplt_s(side1, zero))) {
					firstEdge = ptr;
					break;
				}

				//              side0 = side1;
				side0 = side1;
				edge = ptr->m_twin;
				ptr = edge;
			}
			ptr = ptr->m_twin->m_next;
		} while (ptr != edge);

		if (!firstEdge) {
			// we may have a local minimal in the convex hull do to a big flat face
			for (dgInt32 i = 0; i < m_edgeCount; i++) {
				ptr = &m_simplex[i];
				//              side0 = plane.Evalue (m_vertex[ptr->m_vertex]);
				//              simd_type tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr->m_vertex]);
				//              tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
				//              tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
				//              simd_store_s (tmp, &side0);

				side0 = simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr->m_vertex]);
				side0 = simd_add_v(side0, simd_move_hl_v(side0, side0));
				side0 =
				    simd_add_s(side0, simd_permut_v(side0, side0, PURMUT_MASK(3, 3, 3, 1)));
				//              simd_store_s (tmp, &side0);

				//              side1 = plane.Evalue (m_vertex[ptr->m_twin->m_vertex]);
				//              tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr->m_twin->m_vertex]);
				//              tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
				//              tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
				//              simd_store_s (tmp, &side1);

				side1 =
				    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr->m_twin->m_vertex]);
				side1 = simd_add_v(side1, simd_move_hl_v(side1, side1));
				side1 =
				    simd_add_s(side1, simd_permut_v(side1, side1, PURMUT_MASK(3, 3, 3, 1)));

				dgInt32 test =
				    simd_store_is(simd_and_v(simd_cmplt_s(side1, zero), simd_cmpgt_s(side0, zero)));
				//              if ((side1 < dgFloat32 (0.0f)) && (side0 > dgFloat32 (0.0f))){
				if (test) {
					firstEdge = ptr;
					break;
				}
			}
		}

		//  } else if (side0 < dgFloat32 (0.0f)) {
	} else if (simd_store_is(simd_cmplt_s(side0, zero))) {
		dgConvexSimplexEdge *ptr = edge;
		do {
			NEWTON_ASSERT(m_vertex[ptr->m_twin->m_vertex].m_w == dgFloat32(1.0f));
			//          side1 = plane.Evalue (m_vertex[ptr->m_twin->m_vertex]);
			//          simd_type tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr->m_twin->m_vertex]);
			//          tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
			//          tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
			//          simd_store_s (tmp, &side1);

			side1 =
			    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr->m_twin->m_vertex]);
			side1 = simd_add_v(side1, simd_move_hl_v(side1, side1));
			side1 =
			    simd_add_s(side1, simd_permut_v(side1, side1, PURMUT_MASK(3, 3, 3, 1)));
			//          if (side1 > side0) {
			if (simd_store_is(simd_cmpgt_s(side1, side0))) {
				side0 = side1;
				//              if (side1 >= dgFloat32 (0.0f)) {
				if (simd_store_is(simd_cmpge_s(side1, zero))) {
					firstEdge = ptr->m_twin;
					break;
				}
				edge = ptr->m_twin;
				ptr = edge;
			}
			ptr = ptr->m_twin->m_next;
		} while (ptr != edge);

#ifdef _DEBUG
		if (!firstEdge) {
			// we may have a local minimal in the convex hull do to a big flat face
			for (dgInt32 i = 0; i < m_edgeCount; i++) {
				ptr = &m_simplex[i];
				//              side0 = plane.Evalue (m_vertex[ptr->m_vertex]);
				//              simd_type tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr->m_vertex]);
				//              tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
				//              tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
				//              simd_store_s (tmp, &side0);

				side0 = simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr->m_vertex]);
				side0 = simd_add_v(side0, simd_move_hl_v(side0, side0));
				side0 =
				    simd_add_s(side0, simd_permut_v(side0, side0, PURMUT_MASK(3, 3, 3, 1)));

				//              side1 = plane.Evalue (m_vertex[ptr->m_twin->m_vertex]);
				//              tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr->m_twin->m_vertex]);
				//              tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
				//              tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
				//              simd_store_s (tmp, &side1);

				side1 =
				    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr->m_twin->m_vertex]);
				side1 = simd_add_v(side1, simd_move_hl_v(side1, side1));
				side1 =
				    simd_add_s(side1, simd_permut_v(side1, side1, PURMUT_MASK(3, 3, 3, 1)));

				//              if ((side1 < dgFloat32 (0.0f)) && (side0 > dgFloat32 (0.0f))){
				dgInt32 test =
				    simd_store_is(simd_and_v(simd_cmplt_s(side1, zero), simd_cmpgt_s(side0, zero)));
				if (test) {
					firstEdge = ptr;
					break;
				}
			}
		}
#endif
	}

	dgInt32 count = 0;
	if (firstEdge) {
		dgInt32 maxCount = 0;
		dgConvexSimplexEdge *ptr = firstEdge;
		do {
			//          if (side0 > dgFloat32 (0.0f)) {
			if (simd_store_is(simd_cmpgt_s(side0, zero))) {
				//              NEWTON_ASSERT (plane.Evalue (m_vertex[ptr->m_vertex]) > dgFloat32 (0.0f));
				//              NEWTON_ASSERT (plane.Evalue (m_vertex[ptr->m_twin->m_vertex]) < dgFloat32 (0.0f));
				//              dgVector dp (m_vertex[ptr->m_twin->m_vertex] - m_vertex[ptr->m_vertex]);

				simd_type deltaP =
				    simd_sub_v(*(simd_type *)&m_vertex[ptr->m_twin->m_vertex], *(simd_type *)&m_vertex[ptr->m_vertex]);

				NEWTON_ASSERT(((dgFloat32 *)&deltaP)[3] == dgFloat32(0.0f));

				//              t = plane % dp;
				simd_type tmp = simd_mul_v(planeSimdD, deltaP);
				tmp =
				    simd_add_s(simd_add_v(tmp, simd_move_hl_v(tmp, tmp)), simd_permut_v(tmp, tmp, PURMUT_MASK(3, 3, 3, 1)));
				NEWTON_ASSERT(((dgFloat32 *)&tmp)[3] <= dgFloat32(0.0f));

				//              NEWTON_ASSERT (t <= 0.0f);
				//              if (t < dgFloat32 (0.0f)) {
				//                  t = side0 / t;
				//              }
				//              NEWTON_ASSERT (t <= dgFloat32 (0.01f));
				//              NEWTON_ASSERT (t >= dgFloat32 (-1.05f));
				//              contactsOut[count] = m_vertex[ptr->m_vertex] - dp.Scale (t);

				tmp = simd_min_s(*(simd_type *)&m_negativeTiny, tmp);
				simd_type den = simd_rcp_s(tmp);
				//              den = simd_mul_s (simd_load_s(side0), simd_mul_sub_s(simd_add_s(den, den), simd_mul_s(den, tmp), den));
				den =
				    simd_mul_s(side0, simd_mul_sub_s(simd_add_s(den, den), simd_mul_s(den, tmp), den));
				den = simd_min_s(simd_max_s(den, *(simd_type *)&m_negOne), zero);
				NEWTON_ASSERT(((dgFloat32 *)&den)[0] <= dgFloat32(0.0f));
				NEWTON_ASSERT(((dgFloat32 *)&den)[0] >= dgFloat32(-1.0f));
				*((simd_type *)&contactsOut[count]) =
				    simd_mul_sub_v(*((simd_type *)&m_vertex[ptr->m_vertex]), deltaP, simd_permut_v(den, den, PURMUT_MASK(3, 0, 0, 0)));

				dgConvexSimplexEdge *ptr1 = ptr->m_next;
				for (; ptr1 != ptr; ptr1 = ptr1->m_next) {
					//                  side0 = plane.Evalue (m_vertex[ptr1->m_twin->m_vertex]);
					NEWTON_ASSERT(m_vertex[ptr1->m_twin->m_vertex].m_w = dgFloat32(1.0f));
					//                  tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr1->m_twin->m_vertex]);
					//                  tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
					//                  tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
					//                  simd_store_s (tmp, &side0);

					side0 =
					    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr1->m_twin->m_vertex]);
					side0 = simd_add_v(side0, simd_move_hl_v(side0, side0));
					side0 =
					    simd_add_s(side0, simd_permut_v(side0, side0, PURMUT_MASK(3, 3, 3, 1)));
					//                  if (side0 >= dgFloat32 (0.0f)) {
					if (simd_store_is(simd_cmpge_s(side0, zero))) {
						break;
					}
				}
				NEWTON_ASSERT(ptr1 != ptr);
				ptr = ptr1->m_twin;
			} else {
				//              contactsOut[count] = m_vertex[ptr->m_vertex];
				//              side0 = plane.Evalue (m_vertex[ptr->m_prev->m_vertex]);
				//              if (side0 > dgFloat32 (1.0e-24f)) {
				//                  ptr1 = ptr;
				//                  do {
				//                      ptr1 = ptr1->m_twin->m_next;
				//                      side0 = plane.Evalue (m_vertex[ptr1->m_twin->m_vertex]);
				//                  } while ((ptr1 != ptr) && (side0 < dgFloat32 (0.0f)));
				//                  NEWTON_ASSERT (ptr1 != ptr);
				//                  do {
				//                      ptr1 = ptr1->m_twin->m_next;
				//                      side0 = plane.Evalue (m_vertex[ptr1->m_twin->m_vertex]);
				//                  } while ((ptr1 != ptr) && (side0 > dgFloat32 (0.0f)));
				//                  NEWTON_ASSERT (side0 <= dgFloat32 (0.0f));
				//                  ptr = ptr1;
				//              }
				contactsOut[count] = m_vertex[ptr->m_vertex];
				dgConvexSimplexEdge *ptr1 = ptr->m_next;
				for (; ptr1 != ptr; ptr1 = ptr1->m_next) {
					//                  side0 = plane.Evalue (m_vertex[ptr1->m_twin->m_vertex]);
					//                  simd_type tmp = simd_mul_v (planeSimdD, *(simd_type*)&m_vertex[ptr1->m_twin->m_vertex]);
					//                  tmp = simd_add_v (tmp, simd_move_hl_v (tmp, tmp));
					//                  tmp = simd_add_s(tmp, simd_permut_v (tmp, tmp, PURMUT_MASK (3,3,3,1)));
					//                  simd_store_s (tmp, &side0);

					side0 =
					    simd_mul_v(planeSimdD, *(simd_type *)&m_vertex[ptr1->m_twin->m_vertex]);
					side0 = simd_add_v(side0, simd_move_hl_v(side0, side0));
					side0 =
					    simd_add_s(side0, simd_permut_v(side0, side0, PURMUT_MASK(3, 3, 3, 1)));
					//                  if (side0 >= dgFloat32 (0.0f)) {
					if (simd_store_is(simd_cmpge_s(side0, zero))) {
						break;
					}
				}
				if (ptr1 == ptr) {
					ptr = ptr1->m_prev->m_twin;
				} else {
					ptr = ptr1->m_twin;
				}
			}

			count++;
			maxCount++;
			if (count >= DG_CLIP_MAX_POINT_COUNT) {
				for (count = 0; count < (DG_CLIP_MAX_POINT_COUNT >> 1); count++) {
					contactsOut[count] = contactsOut[count * 2];
				}
			}

		} while ((ptr != firstEdge) && (maxCount < DG_CLIP_MAX_COUNT));
		NEWTON_ASSERT(
		    maxCount < DG_CLIP_MAX_COUNT);

		if (count > 1) {
			count = RectifyConvexSlice(count, normal, contactsOut);
		}
	}
	return count;

#else
	return 0;
#endif
}

dgInt32 dgCollisionConvex::CalculatePlaneIntersection(const dgVector &normal,
        const dgVector &origin, dgVector *const contactsOut) const {
	dgConvexSimplexEdge *edge = &m_simplex[0];
	dgPlane plane(normal, -(normal % origin));

	dgFloat32 side0 = plane.Evalue(m_vertex[edge->m_vertex]);
	dgFloat32 side1 = side0;
	dgConvexSimplexEdge *firstEdge = NULL;
	if (side0 > dgFloat32(0.0f)) {
		dgConvexSimplexEdge *ptr = edge;
		do {
			NEWTON_ASSERT(m_vertex[ptr->m_twin->m_vertex].m_w == dgFloat32(1.0f));
			side1 = plane.Evalue(m_vertex[ptr->m_twin->m_vertex]);
			if (side1 < side0) {
				if (side1 < dgFloat32(0.0f)) {
					firstEdge = ptr;
					break;
				}

				side0 = side1;
				edge = ptr->m_twin;
				ptr = edge;
			}
			ptr = ptr->m_twin->m_next;
		} while (ptr != edge);

#ifdef _DEBUG
		if (!firstEdge) {
			// we may have a local minimal in the convex hull do to a big flat face
			for (dgInt32 i = 0; i < m_edgeCount; i++) {
				ptr = &m_simplex[i];
				side0 = plane.Evalue(m_vertex[ptr->m_vertex]);
				side1 = plane.Evalue(m_vertex[ptr->m_twin->m_vertex]);
				if ((side1 < dgFloat32(0.0f)) && (side0 > dgFloat32(0.0f))) {
					firstEdge = ptr;
					break;
				}
			}
		}
#endif
	} else if (side0 < dgFloat32(0.0f)) {
		dgConvexSimplexEdge *ptr = edge;
		do {
			NEWTON_ASSERT(m_vertex[ptr->m_twin->m_vertex].m_w == dgFloat32(1.0f));
			side1 = plane.Evalue(m_vertex[ptr->m_twin->m_vertex]);
			if (side1 > side0) {
				if (side1 >= dgFloat32(0.0f)) {
					side0 = side1;
					firstEdge = ptr->m_twin;
					break;
				}

				side0 = side1;
				edge = ptr->m_twin;
				ptr = edge;
			}
			ptr = ptr->m_twin->m_next;
		} while (ptr != edge);
#ifdef _DEBUG
		if (!firstEdge) {
			// we may have a local minimal in the convex hull do to a big flat face
			for (dgInt32 i = 0; i < m_edgeCount; i++) {
				ptr = &m_simplex[i];
				side0 = plane.Evalue(m_vertex[ptr->m_vertex]);
				dgFloat32 dside1 = plane.Evalue(m_vertex[ptr->m_twin->m_vertex]);
				if ((dside1 < dgFloat32(0.0f)) && (side0 > dgFloat32(0.0f))) {
					NEWTON_ASSERT(0);
					firstEdge = ptr;
					break;
				}
			}
		}
#endif
	}

	dgInt32 count = 0;
	if (firstEdge) {
		NEWTON_ASSERT(side0 >= dgFloat32(0.0f));
		NEWTON_ASSERT(
		    (side1 = plane.Evalue(m_vertex[firstEdge->m_vertex])) >= dgFloat32(0.0f));
		NEWTON_ASSERT(
		    (side1 = plane.Evalue(m_vertex[firstEdge->m_twin->m_vertex])) < dgFloat32(0.0f));
		NEWTON_ASSERT(
		    dgAbsf(side0 - plane.Evalue(m_vertex[firstEdge->m_vertex])) < dgFloat32(1.0e-5f));

		dgInt32 maxCount = 0;
		dgConvexSimplexEdge *ptr = firstEdge;
		do {
			if (side0 > dgFloat32(0.0f)) {
				NEWTON_ASSERT(plane.Evalue(m_vertex[ptr->m_vertex]) > dgFloat32(0.0f));
				NEWTON_ASSERT(
				    plane.Evalue(m_vertex[ptr->m_twin->m_vertex]) < dgFloat32(0.0f));

				dgVector dp(m_vertex[ptr->m_twin->m_vertex] - m_vertex[ptr->m_vertex]);
				dgFloat32 t = plane % dp;
				if (t >= dgFloat32(-1.e-24f)) {
					t = dgFloat32(0.0f);
				} else {
					t = side0 / t;
					if (t > dgFloat32(0.0f)) {
						t = dgFloat32(0.0f);
					}
					if (t < dgFloat32(-1.0f)) {
						t = dgFloat32(-1.0f);
					}
				}

				NEWTON_ASSERT(t <= dgFloat32(0.01f));
				NEWTON_ASSERT(t >= dgFloat32(-1.05f));
				contactsOut[count] = m_vertex[ptr->m_vertex] - dp.Scale(t);

				dgConvexSimplexEdge *ptr1 = ptr->m_next;
				for (; ptr1 != ptr; ptr1 = ptr1->m_next) {
					NEWTON_ASSERT(m_vertex[ptr->m_twin->m_vertex].m_w == dgFloat32(1.0f));
					side0 = plane.Evalue(m_vertex[ptr1->m_twin->m_vertex]);
					if (side0 >= dgFloat32(0.0f)) {
						break;
					}
				}
				NEWTON_ASSERT(ptr1 != ptr);
				ptr = ptr1->m_twin;
			} else {
				contactsOut[count] = m_vertex[ptr->m_vertex];
				dgConvexSimplexEdge *ptr1 = ptr->m_next;
				for (; ptr1 != ptr; ptr1 = ptr1->m_next) {
					NEWTON_ASSERT(m_vertex[ptr1->m_twin->m_vertex].m_w == dgFloat32(1.0f));
					side0 = plane.Evalue(m_vertex[ptr1->m_twin->m_vertex]);
					if (side0 >= dgFloat32(0.0f)) {
						break;
					}
				}

				if (ptr1 == ptr) {
					ptr = ptr1->m_prev->m_twin;
				} else {
					ptr = ptr1->m_twin;
				}
			}

			count++;
			maxCount++;
			if (count >= DG_CLIP_MAX_POINT_COUNT) {
				for (count = 0; count < (DG_CLIP_MAX_POINT_COUNT >> 1); count++) {
					contactsOut[count] = contactsOut[count * 2];
				}
			}

		} while ((ptr != firstEdge) && (maxCount < DG_CLIP_MAX_COUNT));
		NEWTON_ASSERT(
		    maxCount < DG_CLIP_MAX_COUNT);

		if (count > 2) {
			count = RectifyConvexSlice(count, normal, contactsOut);
		}
	}
	return count;
}

dgFloat32 dgCollisionConvex::RayCast(const dgVector &localP0,
                                     const dgVector &localP1, dgContactPoint &contactOut,
                                     OnRayPrecastAction preFilter, const dgBody *const body,
                                     void *const userData) const {
#define DG_LEN (0.01f)
#define DG_AREA (DG_LEN * DG_LEN)
#define DG_VOL (DG_AREA * DG_LEN)

	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgFloat32 interset = dgFloat32(1.2f);
	if (RayHitBox(localP0, localP1)) {
		if ((m_collsionId != m_convexHullCollision) || (((const dgCollisionConvexHull *)this)->m_faceCount > 48)) {
			//          dgInt32 i;
			//          dgInt32 i0;
			//          dgInt32 i1;
			//          dgInt32 i2;
			//          dgInt32 face;
			//          dgInt32 outside;
			//          dgInt32 passes;
			//          dgInt32 normalsCount;
			//          dgFloat32 t;
			//          dgFloat32 t0;
			//          dgFloat32 dist2;
			//          dgFloat32 error2;
			//          dgFloat32 maxError2;
			dgVector tetrahedrum[4];

			dgVector bestPoint(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			                   dgFloat32(0.0f));
			dgVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			                dgFloat32(0.0f));
			dgVector step(localP1 - localP0);
			dgFloat32 dist2 = step % step;
			if (dist2 > dgFloat32(1.0e-8f)) {
				dgVector dir(step.Scale(dgRsqrt(dist2)));

				tetrahedrum[0] = SupportVertex(dir.Scale(-1.0f));
				tetrahedrum[1] = SupportVertex(dir);

				dgInt32 i = 0;
				dgInt32 normalsCount = sizeof(m_hullDirs) / sizeof(dgTriplex);

				dgVector e1(tetrahedrum[1] - tetrahedrum[0]);
				dgFloat32 error2 = e1 % e1;
				if (error2 < dgFloat32(1.0e-2f)) {
					dgFloat32 maxError2 = dgFloat32(0.0f);
					for (i = 0; i < normalsCount; i++) {
						tetrahedrum[1] = SupportVertex(dgVector(&m_hullDirs[i].m_x));
						e1 = tetrahedrum[1] - tetrahedrum[0];
						error2 = e1 % e1;
						if (error2 > DG_AREA) {
							break;
						}
						if (error2 > maxError2) {
							maxError2 = error2;
							bestPoint = tetrahedrum[1];
						}
					}
					if (i >= normalsCount) {
						tetrahedrum[1] = bestPoint;
						e1 = tetrahedrum[1] - tetrahedrum[0];
						i = 0;
					}
				}

				dgFloat32 maxError2 = dgFloat32(0.0f);
				for (i++; i < normalsCount; i++) {
					tetrahedrum[2] = SupportVertex(dgVector(&m_hullDirs[i].m_x));
					dgVector e2(tetrahedrum[2] - tetrahedrum[0]);
					normal = e1 * e2;
					error2 = normal % normal;
					if (error2 > DG_AREA) {
						break;
					}
					if (error2 > maxError2) {
						maxError2 = error2;
						bestPoint = tetrahedrum[2];
					}
				}
				if (i >= normalsCount) {
					tetrahedrum[2] = bestPoint;
					dgVector e2(tetrahedrum[2] - tetrahedrum[0]);
					normal = e1 * e2;
					i = 0;
				}

				maxError2 = dgFloat32(0.0f);
				for (i++; i < normalsCount; i++) {
					tetrahedrum[3] = SupportVertex(dgVector(&m_hullDirs[i].m_x));
					dgVector e3(tetrahedrum[3] - tetrahedrum[0]);
					error2 = normal % e3;
					if (dgAbsf(error2) > DG_VOL) {
						break;
					}

					if (error2 > maxError2) {
						maxError2 = error2;
						bestPoint = tetrahedrum[3];
					}
				}

				if (i >= normalsCount) {
					tetrahedrum[3] = bestPoint;
					dgVector e3(tetrahedrum[3] - tetrahedrum[0]);
					error2 = maxError2;
				}

				if (dgAbsf(error2) > dgFloat32(1.0e-12f)) {
					if (error2 < dgFloat32(0.0f)) {
						Swap(tetrahedrum[0], tetrahedrum[1]);
					}

					dgInt32 passes = 0;
					dgFloat32 t0 = dgFloat32(0.0f);
					dgInt32 face = RayCastClosestFace(tetrahedrum, localP0, error2);
					error2 = dgFloat32(1.0e10f);
					for (dgInt32 outside = (face != -1);
					        (passes < 128) && outside && (error2 > dgFloat32(1.0e-5f));
					        outside = (face != -1)) {
						passes++;
						dgInt32 i0 = m_rayCastSimplex[face][0];
						dgInt32 i1 = m_rayCastSimplex[face][1];
						dgInt32 i2 = m_rayCastSimplex[face][2];
						const dgVector &p0 = tetrahedrum[i0];
						const dgVector &p1 = tetrahedrum[i1];
						const dgVector &p2 = tetrahedrum[i2];
						dgVector e0(p1 - p0);
						dgVector ee1(p2 - p0);
						normal = e0 * ee1;

						face = -1;
						error2 = dgFloat32(1.0e10f);
						dgFloat32 t = normal % step;
						if (dgAbsf(t) > dgFloat32(0.0f)) {
							t = (normal % (p0 - localP0)) / t;
							if ((t >= t0) && (t <= dgFloat32(1.0f))) {
								dgVector p(localP0 + step.Scale(t));
								face = RayCastClosestFace(tetrahedrum, p, error2);
								t0 = t;
							}
						}
					}

					if (error2 < dgFloat32(1.0e-4f)) {
						interset = t0;
						contactOut.m_normal = normal.Scale(dgRsqrt(normal % normal));
						contactOut.m_userId = SetUserDataID();
					}
				}
			}

		} else {
			//          dgInt32 hasHit;
			//          dgInt32 faceCount;
			//          dgFloat32 N;
			//          dgFloat32 D;
			//          dgFloat32 t;
			//          dgFloat32 tE;
			//          dgFloat32 tL;

			dgVector hitNormal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			                   dgFloat32(0.0f));

			dgFloat32 tE = dgFloat32(0.0f); // for the maximum entering segment parameter;
			dgFloat32 tL = dgFloat32(1.0f); // for the minimum leaving segment parameter;
			dgVector dS(localP1 - localP0); // is the segment direction vector;

			dgInt32 faceCount = ((const dgCollisionConvexHull *)this)->m_faceCount;
			const dgConvexSimplexEdge *const *faceArray =
			    ((const dgCollisionConvexHull *)this)->m_faceArray;

			dgInt32 hasHit = 0;
			for (dgInt32 i = 0; i < faceCount; i++) {
				//              dgInt32 i0;
				//              dgInt32 i1;
				//              dgInt32 i2;

				const dgConvexSimplexEdge *const face = faceArray[i];

				dgInt32 i0 = face->m_prev->m_vertex;
				dgInt32 i1 = face->m_vertex;
				dgInt32 i2 = face->m_next->m_vertex;
				const dgVector &p0 = m_vertex[i0];
				dgVector normal((m_vertex[i1] - p0) * (m_vertex[i2] - p0));

				// N = - dot product of (P0-Vi) and ni;
				dgFloat32 N = -((localP0 - p0) % normal);
				// D = dot product of dS and ni;
				dgFloat32 D = dS % normal;

				if (dgAbsf(D) < dgFloat32(1.0e-8f)) { //
					// then S is parallel to the face Fi
					if (N < dgFloat32(0.0f)) {
						// then P0 is outside the face Fi
						return dgFloat32(1.2f);
					} else {
						// S cannot enter or leave W across face Fi
						// ignore face Fi and to process the next face;
						continue;
					}
				}

				dgFloat32 t = N / D;
				if (D < dgFloat32(0.0f)) {
					// then segment S is entering W across face Fi
					if (t > tE) {
						tE = t;
						hasHit = 1;
						hitNormal = normal;
					}
					//                  tE = GetMax (tE, t);
					if (tE > tL) {
						// then segment S enters W after leaving
						// FALSE since S cannot intersect W
						return dgFloat32(1.2f);
					}
				} else {
					NEWTON_ASSERT(D >= dgFloat32(0.0f));
					// then segment S is leaving W across face Fi
					tL = GetMin(tL, t);
					if (tL < tE) {
						// then segment S leaves W before entering
						// FALSE since S cannot intersect W
						return dgFloat32(1.2f);
					}
				}
			}

			if (hasHit) {
				contactOut.m_normal = hitNormal.Scale(dgRsqrt(hitNormal % hitNormal));
				contactOut.m_userId = SetUserDataID();
				interset = tE;
			}
		}
	}

	return interset;
}

dgFloat32 dgCollisionConvex::RayCastSimd(const dgVector &localP0,
        const dgVector &localP1, dgContactPoint &contactOut,
        OnRayPrecastAction preFilter, const dgBody *const body,
        void *const userData) const {
	return RayCast(localP0, localP1, contactOut, preFilter, body, userData);
}

bool dgCollisionConvex::IsTriggerVolume() const {
	return m_isTriggerVolume;
}

void dgCollisionConvex::SetAsTriggerVolume(bool mode) {
	m_isTriggerVolume = dgUnsigned32(mode);
}

bool dgCollisionConvex::OOBBTest(const dgMatrix &matrix,
                                 const dgCollisionConvex *const shape, void *const cacheOrder) const {
	for (dgInt32 i = 0; i < 3; i++) {
		dgVector dir(matrix.m_front[i], matrix.m_up[i], matrix.m_right[i],
		             dgFloat32(0.0f));
		dgVector p(matrix.TransformVector(shape->SupportVertex(dir)));
		if (p[i] <= (m_boxOrigin[i] - m_boxSize[i])) {
			return false;
		}
		dgVector q(
		    matrix.TransformVector(
		        shape->SupportVertex(dir.Scale(dgFloat32(-1.0f)))));
		if (q[i] >= (m_boxOrigin[i] + m_boxSize[i])) {
			return false;
		}
	}
	return true;
}
