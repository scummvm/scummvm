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

#ifndef __dgIntersections__
#define __dgIntersections__

#include "dgStdafx.h"
#include "dgDebug.h"
#include "dgVector.h"


class dgPlane;
class dgObject;
class dgPolyhedra;

DG_MSC_VECTOR_ALIGMENT
class dgFastRayTest {
public:
	dgFastRayTest(const dgVector &l0, const dgVector &l1);

	dgInt32 BoxTest(const dgVector &minBox, const dgVector &maxBox) const;
	dgInt32 BoxTestSimd(const dgVector &minBox, const dgVector &maxBox) const;

	dgFloat32 PolygonIntersect(const dgVector &normal, const dgFloat32 *const polygon, dgInt32 strideInBytes, const dgInt32 *const indexArray, dgInt32 indexCount) const;
	dgFloat32 PolygonIntersectSimd(const dgVector &normal, const dgFloat32 *const polygon, dgInt32 strideInBytes, const dgInt32 *const indexArray, dgInt32 indexCount) const;

	void Reset(dgFloat32 t) {
		m_dpInv = m_dpBaseInv.Scale(dgFloat32(1.0f) / t);
	}

	dgVector m_p0;
	dgVector m_p1;
	dgVector m_diff;
	dgVector m_dpInv;
	dgVector m_dpBaseInv;
	dgVector m_minT;
	dgVector m_maxT;

	dgVector m_ray_xxxx;
	dgVector m_ray_yyyy;
	dgVector m_ray_zzzz;
	dgVector m_zero;
	dgInt32 m_isParallel[4];

	dgFloat32 m_dirError;
	dgFloat32 m_magRayTest;
} DG_GCC_VECTOR_ALIGMENT;



enum dgIntersectStatus {
	t_StopSearh,
	t_ContinueSearh
};

typedef dgIntersectStatus(*dgAABBIntersectCallback)(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount);

typedef dgFloat32(*dgRayIntersectCallback)(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount);


class dgBeamHitStruct {
public:
	dgVector m_Origin;
	dgVector m_NormalOut;
	dgObject *m_HitObjectOut;
	dgFloat32 m_ParametricIntersctionOut;
};


bool dgApi dgRayBoxClip(dgVector &ray_p0, dgVector &ray_p1, const dgVector &boxP0, const dgVector &boxP1);

/*
dgFloat32 dgRayPolygonIntersect (const dgVector& ray_p0,
                                 const dgVector& ray_p1,
                                 const dgVector& normal,
                                 const dgFloat32* const polygon,
                                 dgInt32 strideInBytes,
                                 const dgInt32* const indexArray,
                                 dgInt32 indexCount);

dgFloat32 dgRayPolygonIntersectSimd (const dgVector& ray_p0,
                                    const dgVector& ray_p1,
                                    const dgVector& normal,
                                    const dgFloat32* const polygon,
                                    dgInt32 strideInBytes,
                                    const dgInt32* const indexArray,
                                    dgInt32 indexCount);

*/

dgVector dgApi dgPointToRayDistance(const dgVector &point, const dgVector &ray_p0, const dgVector &ray_p1);

void dgApi dgRayToRayDistance(const dgVector &ray_p0,
                              const dgVector &ray_p1,
                              const dgVector &ray_q0,
                              const dgVector &ray_q1,
                              dgVector &p0Out,
                              dgVector &p1Out);

dgVector    dgPointToTriangleDistance(const dgVector &point, const dgVector &p0, const dgVector &p1, const dgVector &p2);
dgBigVector dgPointToTriangleDistance(const dgBigVector &point, const dgBigVector &p0, const dgBigVector &p1, const dgBigVector &p2);

bool  dgApi dgPointToPolygonDistance(const dgVector &point, const dgFloat32 *const polygon,  dgInt32 strideInInBytes,
                                     const dgInt32 *const indexArray, dgInt32 indexCount, dgFloat32 bailOutDistance, dgVector &pointOut);


/*
dgInt32 dgApi dgConvexPolyhedraToPlaneIntersection (
                            const dgPolyhedra& convexPolyhedra,
                            const dgFloat32 polyhedraVertexArray[],
                            int polyhedraVertexStrideInBytes,
                            const dgPlane& plane,
                            dgVector* intersectionOut,
                            dgInt32 maxSize);


dgInt32 dgApi ConvecxPolygonToConvexPolygonIntersection (
                            dgInt32 polygonIndexCount_1,
                            const dgInt32 polygonIndexArray_1[],
                           dgInt32 convexPolygonStrideInBytes_1,
                            const dgFloat32 convexPolygonVertex_1[],
                            dgInt32 polygonIndexCount_2,
                            const dgInt32 polygonIndexArray_2[],
                           dgInt32 convexPolygonStrideInBytes_2,
                            const dgFloat32 convexPolygonVertex_2[],
                            dgVector* intersectionOut,
                            dgInt32 maxSize);


dgInt32 dgApi dgConvexIntersection (const dgVector convexPolyA[], dgInt32 countA,
                                                                 const dgVector convexPolyB[], dgInt32 countB,
                                                                 dgVector output[]);

*/

DG_INLINE dgInt32 dgOverlapTest(const dgVector &p0, const dgVector &p1, const dgVector &q0, const dgVector &q1) {
	return ((p0.m_x < q1.m_x) && (p1.m_x > q0.m_x) && (p0.m_z < q1.m_z) && (p1.m_z > q0.m_z) && (p0.m_y < q1.m_y) && (p1.m_y > q0.m_y));
}

DG_INLINE dgInt32 dgOverlapTestSimd(const dgVector &p0, const dgVector &p1, const dgVector &q0, const dgVector &q1) {
#ifdef DG_BUILD_SIMD_CODE
	simd_type test = simd_and_v(simd_cmplt_v((simd_type &)p0, (simd_type &) q1), simd_cmpgt_v((simd_type &)p1, (simd_type &) q0));
	dgInt32 ret = _mm_movemask_ps(test);
	return ((ret & 0x07) == 0x07);

#else
	return 0;
#endif
}


dgBigVector LineTriangleIntersection(const dgBigVector &l0, const dgBigVector &l1, const dgBigVector &A, const dgBigVector &B, const dgBigVector &C);

#endif
