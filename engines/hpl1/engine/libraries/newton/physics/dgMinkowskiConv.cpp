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

#include "dgMinkowskiConv.h"
#include "dgCollisionBox.h"
#include "dgCollisionCapsule.h"
#include "dgCollisionConvex.h"
#include "dgCollisionEllipse.h"
#include "dgCollisionMesh.h"
#include "dgCollisionSphere.h"
#include "dgWorld.h"
#include "dgWorldDynamicUpdate.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#define DG_MINK_MAX_FACES 64
#define DG_MINK_MAX_FACES_SIZE (DG_MINK_MAX_FACES + 16)
#define DG_MINK_MAX_POINTS 64
#define DG_MINK_MAX_POINTS_SIZE (DG_MINK_MAX_POINTS + 16)
#define DG_MAX_SIMPLEX_FACE (DG_MINK_MAX_POINTS * 4)
#define DG_HEAP_EDGE_COUNT 256
#define DG_ROBUST_PLANE_CLIP dgFloat32(1.0f / 256.0f)
#define DG_DISTANCE_TOLERANCE dgFloat32(1.0e-3f)
#define DG_DISTANCE_TOLERANCE_ZERO dgFloat32(1.0e-24f)
#define DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION 32
#define DG_UPDATE_SEPARATING_PLANE_DISTANCE_TOLERANCE1 (DG_DISTANCE_TOLERANCE * dgFloat32(1.0e-1f))
#define DG_UPDATE_SEPARATING_PLANE_DISTANCE_TOLERANCE2 (DG_DISTANCE_TOLERANCE * dgFloat32(1.0e-3f))
#define DG_MIN_VERTEX_ERROR (dgFloat32(1.0e-3f))
#define DG_MIN_VERTEX_ERROR_2 (DG_MIN_VERTEX_ERROR * DG_MIN_VERTEX_ERROR)
#define DG_FALLBACK_SEPARATING_PLANE_ITERATIONS 32
#define DG_FALLBACK_SEPARATING_DIST_TOLERANCE dgFloat32(1.0e-6f)
#define DG_CALCULATE_SEPARATING_PLANE_ERROR (DG_ROBUST_PLANE_CLIP * dgFloat32(2.0f))
#define DG_CALCULATE_SEPARATING_PLANE_ERROR1 (DG_ROBUST_PLANE_CLIP * dgFloat32(0.5f))
#define DG_GETADJACENTINDEX_ACTIVE(x) ((!m_simplex[x->m_adjancentFace[1]].m_isActive) ? 1 : ((!m_simplex[x->m_adjancentFace[2]].m_isActive) ? 2 : 0))
#define DG_GETADJACENTINDEX_VERTEX(x, v) (((x->m_vertex[1] == v) ? 1 : 0) | ((x->m_vertex[2] == v) ? 2 : 0))
#define DG_RSQRT_SIMD_S(x, y)                                                                                                              \
	{                                                                                                                                      \
		simd_type tmp0 = simd_rsqrt_s(x);                                                                                                  \
		y = simd_mul_s(simd_mul_s(dgContactSolver::m_nrh0p5, tmp0), simd_mul_sub_v(dgContactSolver::m_nrh3p0, simd_mul_s(x, tmp0), tmp0)); \
	}

DG_MSC_VECTOR_ALIGMENT
class dgContactSolver {

	enum dgMinkReturnCode {
		dgMinkError,
		dgMinkDisjoint,
		dgMinkIntersecting
	};

	DG_MSC_VECTOR_ALIGMENT
	class dgMinkFace : public dgPlane {
	public:
		dgInt16 m_vertex[4];
		dgInt16 m_adjancentFace[3];
		dgInt8 m_inHeap;
		dgInt8 m_isActive;
	} DG_GCC_VECTOR_ALIGMENT;

	class dgMinkFacePurge {
	public:
		dgMinkFacePurge *m_next;
	};

	DG_MSC_VECTOR_ALIGMENT
	struct dgPerimenterEdge {
		const dgVector *m_vertex;
		dgPerimenterEdge *m_next;
		dgPerimenterEdge *m_prev;
	} DG_GCC_VECTOR_ALIGMENT;

	class dgClosestFace : public dgDownHeap<dgMinkFace *, dgFloat32> {
	public:
		inline dgClosestFace(void *ptr, dgInt32 sizeInBytes) : dgDownHeap<dgMinkFace *, dgFloat32>(ptr, sizeInBytes) {
		}
	};

	inline dgFloat32 GetShapeClipSize(dgCollision *const collision) const {
		return GetMax(
		           collision->GetBoxMaxRadius() * dgFloat32(4.0f) + dgFloat32(1.0f),
		           dgFloat32(32.0f));
		//      return GetMax (collision->GetBoxMaxRadius() * dgFloat32 (4.0f) + dgFloat32 (1.0f), dgFloat32 (10000.0f));
	}

	inline bool CheckTetraHedronVolume() const {
		dgVector e0(m_hullVertex[1] - m_hullVertex[0]);
		dgVector e1(m_hullVertex[2] - m_hullVertex[0]);
		dgVector e2(m_hullVertex[3] - m_hullVertex[0]);

		dgFloat32 volume = (e1 * e0) % e2;
		return (volume >= dgFloat32(0.0f));
	}

	inline bool CheckTetraHedronVolumeLarge() const {
		dgBigVector e0(m_hullVertexLarge[1] - m_hullVertexLarge[0]);
		dgBigVector e1(m_hullVertexLarge[2] - m_hullVertexLarge[0]);
		dgBigVector e2(m_hullVertexLarge[3] - m_hullVertexLarge[0]);

		dgFloat64 volume = (e1 * e0) % e2;
		return (volume >= dgFloat32(0.0f));
	}

	void CalcSupportVertexSimd(const dgVector &dir, dgInt32 entry) {
#ifdef DG_BUILD_SIMD_CODE
		NEWTON_ASSERT((dir % dir) > dgFloat32(0.999f));
		dgVector p(m_referenceCollision->SupportVertexSimd(dir));
		dgVector dir1(
		    m_matrix.UnrotateVectorSimd(
		        simd_mul_v((simd_type &)dir, m_negativeOne)));
		dgVector q(
		    m_matrix.TransformVectorSimd(
		        m_floatingcollision->SupportVertexSimd(dir1)));
		(simd_type &)m_hullVertex[entry] = simd_sub_v((simd_type &)p, (simd_type &)q);
		(simd_type &)m_averVertex[entry] = simd_add_v((simd_type &)p, (simd_type &)q);

#else
#endif
	}

	void CalcSupportVertex(const dgVector &dir, dgInt32 entry) {
		NEWTON_ASSERT((dir % dir) > dgFloat32(0.999f));
		dgVector p(m_referenceCollision->SupportVertex(dir));
		dgVector dir1(m_matrix.UnrotateVector(dir.Scale(dgFloat32(-1.0f))));
		dgVector q(
		    m_matrix.TransformVector(m_floatingcollision->SupportVertex(dir1)));

		m_hullVertex[entry] = p - q;
		m_averVertex[entry] = p + q;
	}

	void CalcSupportVertexLarge(const dgVector &dir, dgInt32 entry) {
		NEWTON_ASSERT((dir % dir) > dgFloat32(0.999f));
		dgVector p0(m_referenceCollision->SupportVertex(dir));
		dgVector dir1(m_matrix.UnrotateVector(dir.Scale(dgFloat32(-1.0f))));
		dgVector q0(
		    m_matrix.TransformVector(m_floatingcollision->SupportVertex(dir1)));

		dgBigVector p(p0);
		dgBigVector q(q0);

		m_hullVertexLarge[entry] = p - q;
		m_averVertexLarge[entry] = p + q;
	}

	/*
	 bool CheckNormal (dgPerimenterEdge* const polygon, const dgVector& shapeNormal) const
	 {
	 dgPerimenterEdge* ptr;

	 ptr = polygon;
	 do {
	 NEWTON_ASSERT (ptr->m_prev->m_next == ptr);
	 NEWTON_ASSERT (ptr->m_next->m_prev == ptr);
	 ptr = ptr->m_next;
	 } while (ptr != polygon);


	 dgVector p0  (*polygon->m_vertex) ;
	 dgVector e0 (*polygon->m_next->m_vertex - p0);

	 dgVector normal (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	 ptr = polygon->m_next->m_next;
	 do {
	 dgVector e1 (*ptr->m_vertex - p0);
	 normal += e0 * e1;
	 e0 = e1;
	 ptr = ptr->m_next;

	 } while (ptr != polygon);

	 dgFloat32 mag2;
	 mag2 = normal % normal;
	 if (mag2 > dgFloat32 (1.0e-18f)) {
	 normal = normal.Scale (dgFloat32 (1.0f) / dgSqrt (mag2));
	 } else {
	 normal = shapeNormal;
	 }
	 //return (normal % shapeNormal) > dgFloat32 (0.9f);
	 return (normal % shapeNormal) > dgFloat32 (0.5f);
	 }
	 */

	dgInt32 CalculateClosestPoints() {
		dgMinkFace *face;
		// dgMinkReturnCode code;

		dgVector contactA(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                  dgFloat32(0.0f));
		dgVector contactB(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                  dgFloat32(0.0f));
		dgMinkReturnCode code = CalcSeparatingPlane(face);
		if (code == dgMinkDisjoint) {
			NEWTON_ASSERT(face);
			// dgInt32 i0 = face->m_vertex[0];
			// dgInt32 i1 = face->m_vertex[1];
			// dgInt32 i2 = face->m_vertex[2];

			dgVector hullVertex[3];
			dgVector averVertex[3];

			for (dgInt32 i = 0; i < 3; i++) {
				hullVertex[i] = m_hullVertex[face->m_vertex[i]];
				averVertex[i] = m_averVertex[face->m_vertex[i]];
			}
			for (dgInt32 i = 0; i < 3; i++) {
				m_hullVertex[i] = hullVertex[i];
				m_averVertex[i] = averVertex[i];
			}

			m_vertexIndex = 3;
			dgVector dir0(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			              dgFloat32(0.0f));
			dgVector origin(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			                dgFloat32(0.0f));
			dgVector v(ReduceTriangle(origin));
			for (dgInt32 i = 0; (i < 32) && (m_vertexIndex < 4); i++) {
				//              dist = v % v + dgFloat32 (1.0e-12f);
				dgFloat32 dist = v % v;
				dgVector dir(dir0);
				if (dist > dgFloat32(1.0e-12f)) {
					NEWTON_ASSERT(dist > DG_DISTANCE_TOLERANCE_ZERO);

					dir = v.Scale(-dgRsqrt(dist));
					dist = dir0 % dir;
					if (dist < dgFloat32(0.9995f)) {
						CalcSupportVertex(dir, m_vertexIndex);
						const dgVector &w = m_hullVertex[m_vertexIndex];
						dgVector wv(w - v);
						dist = dir % wv;
					} else {
						dist = dgFloat32(0.0f);
					}
				} else {
					dist = dgFloat32(0.0f);
				}

				dir0 = dir;

				//              NEWTON_ASSERT (0); // !!! this can be take outside the loop
				//              if (dist < dgFloat32 (5.0e-4f)) {
				if (dist < (DG_DISTANCE_TOLERANCE * dgFloat32(0.5f))) {
					switch (m_vertexIndex) {
					case 1: {
						contactA = m_hullVertex[0] + m_averVertex[0];
						contactB = m_averVertex[0] - m_hullVertex[0];
						break;
					}

					case 2: {
						dgVector lp0(m_hullVertex[0] + m_averVertex[0]);
						dgVector lp1(m_hullVertex[1] + m_averVertex[1]);
						dgVector lr0(m_averVertex[0] - m_hullVertex[0]);
						dgVector lr1(m_averVertex[1] - m_hullVertex[1]);

						dgVector dp(m_hullVertex[1] - m_hullVertex[0]);

						dgFloat32 alpha1 = -(m_hullVertex[0] % dp) / (dp % dp + dgFloat32(1.0e-24f));
						dgFloat32 alpha0 = dgFloat32(1.0f) - alpha1;
						NEWTON_ASSERT(alpha1 >= dgFloat32(0.0f));
						NEWTON_ASSERT(alpha1 <= dgFloat32(1.0f));

						contactA = lp0.Scale(alpha0) + lp1.Scale(alpha1);
						contactB = lr0.Scale(alpha0) + lr1.Scale(alpha1);
						break;
					}

					case 3: {
						// dgFloat32 b1;
						// dgFloat32 b2;
						// dgFloat32 a11;
						// dgFloat32 a22;
						// dgFloat32 a21;
						// dgFloat32 den;

						dgVector lp0(m_hullVertex[0] + m_averVertex[0]);
						dgVector lp1(m_hullVertex[1] + m_averVertex[1]);
						dgVector lp2(m_hullVertex[2] + m_averVertex[2]);
						dgVector lr0(m_averVertex[0] - m_hullVertex[0]);
						dgVector lr1(m_averVertex[1] - m_hullVertex[1]);
						dgVector lr2(m_averVertex[2] - m_hullVertex[2]);

						const dgVector &p0 = m_hullVertex[0];
						const dgVector &p1 = m_hullVertex[1];
						const dgVector &p2 = m_hullVertex[2];

						dgVector p10(p1 - p0);
						dgVector p20(p2 - p0);

						dgFloat32 a11 = p10 % p10;
						dgFloat32 a22 = p20 % p20;
						dgFloat32 a21 = p10 % p20;
						dgFloat32 b1 = -(p10 % p0);
						dgFloat32 b2 = -(p20 % p0);
						dgFloat32 den = a11 * a22 - a21 * a21;

						dgFloat32 alpha0;
						dgFloat32 alpha1;
						dgFloat32 alpha2;
						if (den > dgFloat32(1.0e-7f)) {
							NEWTON_ASSERT(den > dgFloat32(0.0f));
							alpha1 = b1 * a22 - a21 * b2;
							alpha2 = a11 * b2 - b1 * a21;
							alpha0 = den - alpha1 - alpha2;

							den = dgFloat32(1.0f) / den;

							alpha0 *= den;
							alpha1 *= den;
							alpha2 *= den;
						} else {
							alpha0 = dgFloat32(0.33f);
							alpha1 = dgFloat32(0.33f);
							alpha2 = dgFloat32(0.33f);
						}

						NEWTON_ASSERT(alpha0 >= dgFloat32(-2.0e-2f));
						NEWTON_ASSERT(alpha1 >= dgFloat32(-2.0e-2f));
						NEWTON_ASSERT(alpha2 >= dgFloat32(-2.0e-2f));
						NEWTON_ASSERT(alpha0 <= dgFloat32(1.0f + 2.0e-2f));
						NEWTON_ASSERT(alpha1 <= dgFloat32(1.0f + 2.0e-2f));
						NEWTON_ASSERT(alpha2 <= dgFloat32(1.0f + 2.0e-2f));

						contactA = lp0.Scale(alpha0) + lp1.Scale(alpha1) + lp2.Scale(alpha2);
						contactB = lr0.Scale(alpha0) + lr1.Scale(alpha1) + lr2.Scale(alpha2);
						break;
					}
					}

					//                  contactA = m_referenceBody->m_collisionWorldMatrix.TransformVector (contactA.Scale (dgFloat32 (0.5f)));
					//                  contactB = m_referenceBody->m_collisionWorldMatrix.TransformVector (contactB.Scale (dgFloat32 (0.5f)));
					//                  dir = m_referenceBody->m_collisionWorldMatrix.RotateVector(dir);
					contactA = m_proxy->m_referenceMatrix.TransformVector(
					               contactA.Scale(dgFloat32(0.5f)));
					contactB = m_proxy->m_referenceMatrix.TransformVector(
					               contactB.Scale(dgFloat32(0.5f)));
					dir = m_proxy->m_referenceMatrix.RotateVector(dir);

					dgContactPoint *contact = m_proxy->m_contacts;
					contact[0].m_point.m_x = contactA.m_x;
					contact[0].m_point.m_y = contactA.m_y;
					contact[0].m_point.m_z = contactA.m_z;
					contact[1].m_point.m_x = contactB.m_x;
					contact[1].m_point.m_y = contactB.m_y;
					contact[1].m_point.m_z = contactB.m_z;

					contact[0].m_normal.m_x = dir.m_x;
					contact[0].m_normal.m_y = dir.m_y;
					contact[0].m_normal.m_z = dir.m_z;

					contact[1].m_normal.m_x = -dir.m_x;
					contact[1].m_normal.m_y = -dir.m_y;
					contact[1].m_normal.m_z = -dir.m_z;
					return 1;
				}

				m_vertexIndex++;
				switch (m_vertexIndex) {
				case 1: {
					NEWTON_ASSERT(0);
					break;
				}

				case 2: {
					v = ReduceLine(origin);
					break;
				}

				case 3: {
					v = ReduceTriangle(origin);
					break;
				}

				case 4: {
					v = ReduceTetrahedrum(origin);
					break;
				}
				}
			}
		}

		//NEWTON_ASSERT (i < CLOSE_DISTANCE_MAX_ITERATION);
		NEWTON_ASSERT(code != dgMinkError);
		return 0;
	}

	dgInt32 CalculateConvexShapeIntersectionLine(const dgMatrix &matrix,
	        const dgVector &shapeNormal, dgUnsigned32 id, dgFloat32 penetration,
	        dgInt32 shape1VertexCount, dgVector *const shape1,
	        dgInt32 shape2VertexCount, dgVector *const shape2,
	        dgContactPoint *const contactOut) {
		//      dgInt32 i0;
		//      dgInt32 i1;
		//      dgInt32 count;
		//      dgFloat32 den;
		//      dgFloat32 test0;
		//      dgFloat32 test1;
		//      dgVector* ptr;
		//      dgVector* output;

		dgInt32 count = 0;
		dgVector *output = (dgVector *)&m_hullVertex[shape1VertexCount + shape2VertexCount + 1];

		NEWTON_ASSERT(shape1VertexCount >= 3);
		NEWTON_ASSERT(shape2VertexCount <= 2);

		dgVector *ptr = NULL;
		// face line intersection
		if (shape2VertexCount == 2) {
			ptr = (dgVector *)&shape2[0];
			dgInt32 i0 = shape1VertexCount - 1;
			for (dgInt32 i1 = 0; i1 < shape1VertexCount; i1++) {
				dgVector n(shapeNormal * (shape1[i1] - shape1[i0]));
				NEWTON_ASSERT((n % n) > dgFloat32(0.0f));
				dgPlane plane(n, -(n % shape1[i0]));

				dgFloat32 test0 = plane.Evalue(ptr[0]);
				dgFloat32 test1 = plane.Evalue(ptr[1]);
				//      ForceFloatConsistency ();
				if (test0 >= dgFloat32(0.0f)) {
					if (test1 >= dgFloat32(0.0f)) {
						output[count + 0] = ptr[0];
						output[count + 1] = ptr[1];
						count += 2;
					} else {
						dgVector dp(ptr[1] - ptr[0]);
						dgFloat32 den = plane % dp;
						if (dgAbsf(den) < 1.0e-10f) {
							den = 1.0e-10f;
						}
						output[count + 0] = ptr[0];
						output[count + 1] = ptr[0] - dp.Scale(test0 / den);
						count += 2;
					}
				} else if (test1 >= dgFloat32(0.0f)) {
					dgVector dp(ptr[1] - ptr[0]);
					dgFloat32 den = plane % dp;
					if (dgAbsf(den) < 1.0e-10f) {
						den = 1.0e-10f;
					}
					output[count] = ptr[0] - dp.Scale(test0 / den);
					count++;
					output[count] = ptr[1];
					count++;
				} else {
					return 0;
				}

				shape2VertexCount = count;
				ptr = output;
				output = &output[count];
				count = 0;
				i0 = i1;
			}

		} else {
			shape2VertexCount = 0;
		}

		dgVector normal = matrix.RotateVector(shapeNormal);
		for (dgInt32 i0 = 0; i0 < shape2VertexCount; i0++) {
			contactOut[i0].m_point = matrix.TransformVector(ptr[i0]);
			contactOut[i0].m_normal = normal;
			contactOut[i0].m_penetration = penetration;
			contactOut[i0].m_userId = id;
		}

		return shape2VertexCount;
	}

	dgPerimenterEdge *ReduceContacts(dgPerimenterEdge *poly,
	                                 dgInt32 maxCount) const {
		// dgPerimenterEdge* ptr;
		// dgPerimenterEdge* ptr0;
		dgInt32 buffer[256];
		for (uint i = 0; i < ARRAYSIZE(buffer); i++) buffer[i] = 0;
		dgUpHeap<dgPerimenterEdge *, dgFloat32> heap(buffer, sizeof(buffer));

		dgInt32 restart = 1;
		while (restart) {
			restart = 0;
			dgPerimenterEdge *ptr0 = poly;
			poly = poly->m_next;
			if (poly->m_next != poly) {
				heap.Flush();
				dgPerimenterEdge *ptr = poly;
				do {
					dgFloat32 dist2;
					dgVector error(*ptr->m_next->m_vertex - *ptr->m_vertex);
					dist2 = error % error;
					if (dist2 < DG_MIN_VERTEX_ERROR_2) {
						ptr0->m_next = ptr->m_next;
						if (ptr == poly) {
							poly = ptr0;
							restart = 1;
							break;
						}
						ptr = ptr0;
					} else {
						heap.Push(ptr, dist2);
						ptr0 = ptr;
					}

					ptr = ptr->m_next;
				} while (ptr != poly);
			}
		}

		if (heap.GetCount()) {
			if (maxCount > 8) {
				maxCount = 8;
			}
			while (heap.GetCount() > maxCount) {
				// dgFloat32 dist2;
				dgPerimenterEdge *ptr = heap[0];
				heap.Pop();
				for (dgInt32 i = 0; i < heap.GetCount(); i++) {
					if (heap[i] == ptr->m_next) {
						heap.Remove(i);
						break;
					}
				}

				ptr->m_next = ptr->m_next->m_next;
				dgVector error(*ptr->m_next->m_vertex - *ptr->m_vertex);
				dgFloat32 dist2 = error % error;
				heap.Push(ptr, dist2);
			}
			poly = heap[0];
		}

		return poly;
	}

	dgInt32 CalculateContactAlternateMethod(dgMinkFace *const face,
	                                        dgInt32 contacID, dgContactPoint *const contactOut, dgInt32 maxContacts) {
		dgInt32 count = 0;
		// Get the contact form the last face
		const dgPlane &plane = *face;
		dgFloat32 penetration = plane.m_w - m_penetrationPadding;
		dgFloat32 dist = (plane % m_averVertex[face->m_vertex[0]]) * dgFloat32(0.5f);
		const dgPlane clipPlane(plane.Scale(dgFloat32(-1.0f)), dist);

		dgVector point1(clipPlane.Scale(-clipPlane.m_w));
		dgVector *const shape1 = m_hullVertex;

		dgVector p1(
		    m_referenceCollision->SupportVertex(clipPlane.Scale(dgFloat32(-1.0f))));
		p1 += clipPlane.Scale(DG_ROBUST_PLANE_CLIP);
		dgInt32 count1 = m_referenceCollision->CalculatePlaneIntersection(clipPlane,
		                 p1, shape1);
		dgVector err(clipPlane.Scale(clipPlane % (point1 - p1)));
		for (dgInt32 i = 0; i < count1; i++) {
			shape1[i] += err;
		}

		//      NEWTON_ASSERT (penetration <= dgFloat32 (2.0e-1f));
		dist = GetMax(-(penetration + DG_IMPULSIVE_CONTACT_PENETRATION),
		              dgFloat32(0.0f));
		if (count1) {

			dgVector *const shape2 = &m_hullVertex[count1];
			const dgPlane clipPlane2(m_matrix.UntransformPlane(clipPlane));

			dgVector point2(clipPlane2.Scale(-clipPlane2.m_w));
			dgVector p2(
			    m_floatingcollision->SupportVertex(
			        clipPlane2.Scale(dgFloat32(-1.0f))));
			p2 += clipPlane2.Scale(DG_ROBUST_PLANE_CLIP);
			dgInt32 count2 = m_floatingcollision->CalculatePlaneIntersection(
			                     clipPlane2, p2, shape2);
			dgVector err2(clipPlane2.Scale(clipPlane2 % (point2 - p2)));
			for (dgInt32 i = 0; i < count2; i++) {
				shape2[i] += err2;
			}

			if (count2) {
				NEWTON_ASSERT(count1);
				NEWTON_ASSERT(count2);

				if (count1 == 1) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;

					count = 1;
					contactOut[0].m_point = matrix1.TransformVector(shape1[0]);
					contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
					contactOut[0].m_userId = contacID;
					contactOut[0].m_penetration = dist;
				} else if (count2 == 1) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					const dgMatrix &matrix2 = m_proxy->m_floatingMatrix;

					count = 1;
					contactOut[0].m_point = matrix2.TransformVector(shape2[0]);
					contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
					contactOut[0].m_userId = contacID;
					contactOut[0].m_penetration = dist;

				} else if ((count1 == 2) && (count2 == 2)) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					dgVector pp0(shape1[0]);
					dgVector pp1(shape1[1]);
					dgVector qq0(m_matrix.TransformVector(shape2[0]));
					dgVector qq1(m_matrix.TransformVector(shape2[1]));
					dgVector p10(pp1 - pp0);
					dgVector q10(qq1 - qq0);
					p10 = p10.Scale(
					          dgFloat32(1.0f) / dgSqrt(p10 % p10 + dgFloat32(1.0e-8f)));
					q10 = q10.Scale(
					          dgFloat32(1.0f) / dgSqrt(q10 % q10 + dgFloat32(1.0e-8f)));
					dgFloat32 dot = q10 % p10;
					if (dgAbsf(dot) > dgFloat32(0.998f)) {
						// dgFloat32 pl0;
						// dgFloat32 pl1;
						// dgFloat32 ql0;
						// dgFloat32 ql1;
						dgFloat32 pl0 = pp0 % p10;
						dgFloat32 pl1 = pp1 % p10;
						dgFloat32 ql0 = qq0 % p10;
						dgFloat32 ql1 = qq1 % p10;
						if (pl0 > pl1) {
							Swap(pl0, pl1);
							Swap(pp0, pp1);
							p10 = p10.Scale(dgFloat32(-1.0f));
						}
						if (ql0 > ql1) {
							Swap(ql0, ql1);
						}
						if (!((ql0 > pl1) && (ql1 < pl0))) {
							// dgFloat32 clip0;
							// dgFloat32 clip1;
							dgFloat32 clip0 = (ql0 > pl0) ? ql0 : pl0;
							dgFloat32 clip1 = (ql1 < pl1) ? ql1 : pl1;

							count = 2;
							contactOut[0].m_point = pp0 + p10.Scale(clip0 - pl0);
							contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
							contactOut[0].m_userId = contacID;
							contactOut[0].m_penetration = dist;

							contactOut[1].m_point = pp0 + p10.Scale(clip1 - pl0);
							contactOut[1].m_normal = matrix1.RotateVector(clipPlane);
							contactOut[1].m_userId = contacID;
							contactOut[1].m_penetration = dist;
						}

					} else {
						dgVector c0;
						dgVector c1;
						count = 1;
						dgRayToRayDistance(pp0, pp1, qq0, qq1, c0, c1);
						contactOut[0].m_point = (c0 + c1).Scale(dgFloat32(0.5f));
						contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
						contactOut[0].m_userId = contacID;
						contactOut[0].m_penetration = dist;
					}
					for (dgInt32 i = 0; i < count; i++) {
						contactOut[i].m_point = matrix1.TransformVector(
						                            contactOut[i].m_point);
					}

				} else {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					m_matrix.TransformTriplex(&shape2[0].m_x, sizeof(dgVector),
					                          &shape2[0].m_x, sizeof(dgVector), count2);
					count = CalculateConvexShapeIntersection(matrix1, clipPlane,
					        dgUnsigned32(contacID), dist, count1, shape1, count2, shape2,
					        contactOut, maxContacts);
				}
			}
		}

		return count;
	}

	dgInt32 CalculateConvexShapeIntersectionSimd(const dgMatrix &matrix,
	        const dgVector &shapeNormal, dgUnsigned32 id, dgFloat32 penetration,
	        dgInt32 shape1VertexCount, dgVector *const shape1,
	        dgInt32 shape2VertexCount, dgVector *const shape2,
	        dgContactPoint *const contactOut, dgInt32 maxContacts) const {
#ifdef DG_BUILD_SIMD_CODE

		dgInt32 count = 0;
		if (shape2VertexCount <= 2) {
			count = CalculateConvexShapeIntersectionLine(matrix, shapeNormal, id,
			        penetration, shape1VertexCount, shape1, shape2VertexCount, shape2,
			        contactOut);
			if (count > maxContacts) {
				count = maxContacts;
			}

		} else if (shape1VertexCount <= 2) {
			count = CalculateConvexShapeIntersectionLine(matrix, shapeNormal, id,
			        penetration, shape2VertexCount, shape2, shape1VertexCount, shape1,
			        contactOut);
			if (count > maxContacts) {
				count = maxContacts;
			}

		} else {
			//          dgInt32 i0;
			//          dgInt32 edgeIndex;
			//          dgVector* ptr;
			//          dgVector* output;
			//          dgPerimenterEdge *poly;
			dgPerimenterEdge *edgeClipped[2];
			dgPerimenterEdge subdivision[128];

			NEWTON_ASSERT(shape1VertexCount >= 3);
			NEWTON_ASSERT(shape2VertexCount >= 3);
			dgVector *output = (dgVector *)&m_hullVertex[shape1VertexCount + shape2VertexCount];

			//          ptr = NULL;
			NEWTON_ASSERT(
			    (shape1VertexCount + shape2VertexCount) < dgInt32(sizeof(subdivision) / (2 * sizeof(subdivision[0]))));
			for (dgInt32 i0 = 0; i0 < shape2VertexCount; i0++) {
				subdivision[i0].m_vertex = &shape2[i0];
				subdivision[i0].m_prev = &subdivision[i0 - 1];
				subdivision[i0].m_next = &subdivision[i0 + 1];
			}
			// i0 --;
			// subdivision[0].m_prev = &subdivision[i0];
			// subdivision[i0].m_next = &subdivision[0];
			subdivision[0].m_prev = &subdivision[shape2VertexCount - 1];
			subdivision[shape2VertexCount - 1].m_next = &subdivision[0];

			dgPerimenterEdge *poly = &subdivision[0];
			//NEWTON_ASSERT (CheckNormal (poly, shapeNormal));

#ifdef _DEBUG
			{
				dgVector p0;
				dgVector p1;
				m_referenceCollision->CalcAABB(dgGetIdentityMatrix(), p0, p1);
				p0 -= dgVector(0.5f, 0.5f, 0.5f, 0.0f);
				p1 += dgVector(0.5f, 0.5f, 0.5f, 0.0f);
				for (dgInt32 i = 0; i < shape1VertexCount; i++) {
					dgVector p(shape1[i]);
					NEWTON_ASSERT(p.m_x >= p0.m_x);
					NEWTON_ASSERT(p.m_y >= p0.m_y);
					NEWTON_ASSERT(p.m_z >= p0.m_z);
					NEWTON_ASSERT(p.m_x <= p1.m_x);
					NEWTON_ASSERT(p.m_y <= p1.m_y);
					NEWTON_ASSERT(p.m_z <= p1.m_z);
				}
			}
#endif

			//          simd_type zero;
			//          simd_type neg_one;
			//          simd_type tol_pos_1e_24;
			//          simd_type tol_neg_1e_24;
			//          simd_type signMask;
			//          t0 = 0x7fffffff;
			//          signMask = simd_set1((dgFloat32&) t0 );

			simd_type zero = simd_set1(dgFloat32(0.0f));
			simd_type neg_one = simd_set1(dgFloat32(-1.0f));
			simd_type tol_pos_1e_24 = simd_set1(dgFloat32(1.0e-24f));
			simd_type tol_neg_1e_24 = simd_set1(dgFloat32(-1.0e-24f));

			edgeClipped[0] = NULL;
			edgeClipped[1] = NULL;

			dgInt32 i0 = shape1VertexCount - 1;
			dgInt32 edgeIndex = shape2VertexCount;
			for (dgInt32 i1 = 0; i1 < shape1VertexCount; i1++) {
				// dgInt32 isInside;
				// dgFloat32 test0;
				// dgFloat32 test1;
				// dgPerimenterEdge *tmp;

				dgVector n(
				    shapeNormal.CrossProductSimd(
				        simd_sub_v((simd_type &)shape1[i1], (simd_type &)shape1[i0])));
				dgPlane plane(n, -n.DotProductSimd(shape1[i0]));
				i0 = i1;

				count = 0;
				dgPerimenterEdge *tmp = poly;
				dgInt32 isInside = 0;
				dgFloat32 test0 = plane.Evalue(*tmp->m_vertex);
				do {
					dgFloat32 test1 = plane.Evalue(*tmp->m_next->m_vertex);
					if (test0 >= dgFloat32(0.0f)) {
						isInside |= 1;
						if (test1 < dgFloat32(0.0f)) {
							// dgFloat32 den;
							// simd_type dp;
							// simd_type den;
							// simd_type test;
							const dgVector &p0 = *tmp->m_vertex;
							const dgVector &p1 = *tmp->m_next->m_vertex;

							//                          dgVector dp (p1 - p0);
							simd_type dp = simd_sub_v((simd_type &)p1, (simd_type &)p0);

							//                          den = plane % dp;
							simd_type den = simd_mul_v((simd_type &)plane, dp);
							den = simd_add_v(den, simd_move_hl_v(den, den));
							den =
							    simd_add_s(den, simd_permut_v(den, den, PURMUT_MASK(3, 3, 3, 1)));
							//                          if (dgAbsf(den) < dgFloat32 (1.0e-24f)) {
							//                              den = dgFloat32 (1.0e-24f) * (den > dgFloat32 (0.0f)) ? dgFloat32 (1.0f) : dgFloat32 (-1.0f);
							//                          }
							simd_type test = simd_cmpge_s(den, zero);
							den =
							    simd_or_v(simd_and_v(simd_max_s(den, tol_pos_1e_24), test), simd_andnot_v(simd_min_s(den, tol_neg_1e_24), test));

							//                          den = test0 / den;
							den = simd_div_s(simd_set1(test0), den);

							//                          if (den >= dgFloat32 (0.0f)) {
							//                              den = dgFloat32 (0.0f);
							//                          } else if (den <= -1.0f) {
							//                              den = dgFloat32 (-1.0f);
							//                          }
							den = simd_max_v(neg_one, simd_min_v(den, zero));
							//                          output[0] = p0 - dp.Scale (den);
							(simd_type &)output[0] =
							    simd_mul_sub_v((simd_type &)p0, dp, simd_permut_v(den, den, PURMUT_MASK(0, 0, 0, 0)));

							edgeClipped[0] = tmp;
							count++;
						}
					} else if (test1 >= dgFloat32(0.0f)) {
						// simd_type dp;
						// simd_type den;
						// simd_type test;

						isInside |= 1;
						const dgVector &p0 = *tmp->m_vertex;
						const dgVector &p1 = *tmp->m_next->m_vertex;

						// dgVector dp (p1 - p0);
						simd_type dp = simd_sub_v((simd_type &)p1, (simd_type &)p0);

						// den = plane % dp;
						simd_type den = simd_mul_v((simd_type &)plane, dp);
						den = simd_add_v(den, simd_move_hl_v(den, den));
						den =
						    simd_add_s(den, simd_permut_v(den, den, PURMUT_MASK(3, 3, 3, 1)));
						// if (dgAbsf(den) < dgFloat32 (1.0e-24f)) {
						//  den = dgFloat32 (1.0e-24f) * (den > dgFloat32 (0.0f)) ? dgFloat32 (1.0f) : dgFloat32 (-1.0f);
						// }
						simd_type test = simd_cmpge_s(den, zero);
						den =
						    simd_or_v(simd_and_v(simd_max_s(den, tol_pos_1e_24), test), simd_andnot_v(simd_min_s(den, tol_neg_1e_24), test));

						// den = test0 / den;
						den = simd_div_s(simd_set1(test0), den);

						// if (den >= dgFloat32 (0.0f)) {
						//  den = dgFloat32 (0.0f);
						// } else if (den <= -1.0f) {
						//  den = dgFloat32 (-1.0f);
						// }
						den = simd_max_v(neg_one, simd_min_v(den, zero));

						// output[1] = p0 - dp.Scale (den);
						(simd_type &)output[1] =
						    simd_mul_sub_v((simd_type &)p0, dp, simd_permut_v(den, den, PURMUT_MASK(0, 0, 0, 0)));

						edgeClipped[1] = tmp;
						count++;
					}
					test0 = test1;
					tmp = tmp->m_next;
				} while (tmp != poly && (count < 2));

				if (!isInside) {
					return 0;
				}

				if (count == 2) {
					// dgPerimenterEdge *newEdge;
					dgPerimenterEdge *const newEdge = &subdivision[edgeIndex];
					newEdge->m_next = edgeClipped[1];
					newEdge->m_prev = edgeClipped[0];
					edgeClipped[0]->m_next = newEdge;
					edgeClipped[1]->m_prev = newEdge;

					newEdge->m_vertex = &output[0];
					edgeClipped[1]->m_vertex = &output[1];
					poly = newEdge;

					output += 2;
					edgeIndex++;
					NEWTON_ASSERT(edgeIndex < dgInt32(sizeof(subdivision) / sizeof(subdivision[0])));
					//NEWTON_ASSERT (CheckNormal (poly, shapeNormal));
				}
			}

			NEWTON_ASSERT(poly);

#ifdef _DEBUG
			{
				dgVector p0;
				dgVector p1;
				m_referenceCollision->CalcAABB(dgGetIdentityMatrix(), p0, p1);
				p0 -= dgVector(0.5f, 0.5f, 0.5f, 0.0f);
				p1 += dgVector(0.5f, 0.5f, 0.5f, 0.0f);
				dgPerimenterEdge *tmp;
				tmp = poly;
				do {
					dgVector p(*tmp->m_vertex);
					NEWTON_ASSERT(p.m_x >= p0.m_x);
					NEWTON_ASSERT(p.m_y >= p0.m_y);
					NEWTON_ASSERT(p.m_z >= p0.m_z);
					NEWTON_ASSERT(p.m_x <= p1.m_x);
					NEWTON_ASSERT(p.m_y <= p1.m_y);
					NEWTON_ASSERT(p.m_z <= p1.m_z);
					tmp = tmp->m_next;
				} while (tmp != poly);
			}
#endif

			NEWTON_ASSERT(poly);
			poly = ReduceContacts(poly, maxContacts);

			// if (count > 0) {
			// dgPerimenterEdge *intersection;
			count = 0;
			dgPerimenterEdge *intersection = poly;
			dgVector normal = matrix.RotateVector(shapeNormal);
			do {
				contactOut[count].m_point = matrix.TransformVectorSimd(
				                                *intersection->m_vertex);
				contactOut[count].m_normal = normal;
				contactOut[count].m_penetration = penetration;
				contactOut[count].m_userId = id;
				count++;
				intersection = intersection->m_next;
			} while (intersection != poly);
			//}
		}
		return count;

#else
		return 0;
#endif
	}

	dgInt32 CalculateConvexShapeIntersection(const dgMatrix &matrix,
	        const dgVector &shapeNormal, dgUnsigned32 id, dgFloat32 penetration,
	        dgInt32 shape1VertexCount, dgVector *const shape1,
	        dgInt32 shape2VertexCount, dgVector *const shape2,
	        dgContactPoint *const contactOut, dgInt32 maxContacts) {

		dgInt32 count = 0;
		if (shape2VertexCount <= 2) {
			count = CalculateConvexShapeIntersectionLine(matrix, shapeNormal, id,
			        penetration, shape1VertexCount, shape1, shape2VertexCount, shape2,
			        contactOut);

			if (count > maxContacts) {
				count = maxContacts;
			}
		} else if (shape1VertexCount <= 2) {
			count = CalculateConvexShapeIntersectionLine(matrix, shapeNormal, id,
			        penetration, shape2VertexCount, shape2, shape1VertexCount, shape1,
			        contactOut);
			if (count > maxContacts) {
				count = maxContacts;
			}

		} else {
			dgPerimenterEdge *edgeClipped[2];
			dgPerimenterEdge subdivision[128];

			NEWTON_ASSERT(shape1VertexCount >= 3);
			NEWTON_ASSERT(shape2VertexCount >= 3);
			dgVector *output = (dgVector *)&m_hullVertex[shape1VertexCount + shape2VertexCount];

			// ptr = NULL;
			NEWTON_ASSERT(
			    (shape1VertexCount + shape2VertexCount) < dgInt32(sizeof(subdivision) / (2 * sizeof(subdivision[0]))));
			for (dgInt32 i0 = 0; i0 < shape2VertexCount; i0++) {
				subdivision[i0].m_vertex = &shape2[i0];
				subdivision[i0].m_prev = &subdivision[i0 - 1];
				subdivision[i0].m_next = &subdivision[i0 + 1];
			}
			// i0 --;
			// subdivision[0].m_prev = &subdivision[i0];
			// subdivision[i0].m_next = &subdivision[0];
			subdivision[0].m_prev = &subdivision[shape2VertexCount - 1];
			subdivision[shape2VertexCount - 1].m_next = &subdivision[0];

			dgPerimenterEdge *poly = &subdivision[0];
			edgeClipped[0] = NULL;
			edgeClipped[1] = NULL;
			dgInt32 i0 = shape1VertexCount - 1;
			dgInt32 edgeIndex = shape2VertexCount;
			for (dgInt32 i1 = 0; i1 < shape1VertexCount; i1++) {
				dgVector n(shapeNormal * (shape1[i1] - shape1[i0]));
				dgPlane plane(n, -(n % shape1[i0]));
				i0 = i1;

				count = 0;
				dgPerimenterEdge *tmp = poly;
				dgInt32 isInside = 0;
				dgFloat32 test0 = plane.Evalue(*tmp->m_vertex);
				do {
					dgFloat32 test1 = plane.Evalue(*tmp->m_next->m_vertex);
					if (test0 >= dgFloat32(0.0f)) {
						isInside |= 1;
						if (test1 < dgFloat32(0.0f)) {
							const dgVector &p0 = *tmp->m_vertex;
							const dgVector &p1 = *tmp->m_next->m_vertex;
							dgVector dp(p1 - p0);
							dgFloat32 den = plane % dp;
							if (dgAbsf(den) < dgFloat32(1.0e-24f)) {
								den = dgFloat32(1.0e-24f) * ((den > dgFloat32(0.0f)) ? dgFloat32(1.0f) : dgFloat32(-1.0f));
							}

							den = test0 / den;
							if (den >= dgFloat32(0.0f)) {
								den = dgFloat32(0.0f);
							} else if (den <= -1.0f) {
								den = dgFloat32(-1.0f);
							}
							output[0] = p0 - dp.Scale(den);
							edgeClipped[0] = tmp;
							count++;
						}
					} else if (test1 >= dgFloat32(0.0f)) {
						const dgVector &p0 = *tmp->m_vertex;
						const dgVector &p1 = *tmp->m_next->m_vertex;
						isInside |= 1;
						dgVector dp(p1 - p0);
						dgFloat32 den = plane % dp;
						if (dgAbsf(den) < dgFloat32(1.0e-24f)) {
							den =
							    dgFloat32(1.0e-24f) * ((den > dgFloat32(0.0f)) ? dgFloat32(1.0f) : dgFloat32(-1.0f));
						}
						den = test0 / den;
						if (den >= dgFloat32(0.0f)) {
							den = dgFloat32(0.0f);
						} else if (den <= -1.0f) {
							den = dgFloat32(-1.0f);
						}
						output[1] = p0 - dp.Scale(den);
						edgeClipped[1] = tmp;
						count++;
					}
					test0 = test1;
					tmp = tmp->m_next;
				} while (tmp != poly && (count < 2));

				if (!isInside) {
					return 0;
				}

				if (count == 2) {
					dgPerimenterEdge *const newEdge = &subdivision[edgeIndex];
					newEdge->m_next = edgeClipped[1];
					newEdge->m_prev = edgeClipped[0];
					edgeClipped[0]->m_next = newEdge;
					edgeClipped[1]->m_prev = newEdge;

					newEdge->m_vertex = &output[0];
					edgeClipped[1]->m_vertex = &output[1];
					poly = newEdge;

					output += 2;
					edgeIndex++;
					NEWTON_ASSERT(edgeIndex < dgInt32(sizeof(subdivision) / sizeof(subdivision[0])));
					//NEWTON_ASSERT (CheckNormal (poly, shapeNormal));
				}
			}

			NEWTON_ASSERT(poly);

#ifdef _DEBUG
			{
				dgVector p0;
				dgVector p1;
				m_referenceCollision->CalcAABB(dgGetIdentityMatrix(), p0, p1);
				p0 -= dgVector(2.5f, 2.5f, 2.5f, 0.0f);
				p1 += dgVector(2.5f, 2.5f, 2.5f, 0.0f);
				dgPerimenterEdge *tmp;
				tmp = poly;
				do {
					dgVector p(*tmp->m_vertex);
					NEWTON_ASSERT(p.m_x >= p0.m_x);
					NEWTON_ASSERT(p.m_y >= p0.m_y);
					NEWTON_ASSERT(p.m_z >= p0.m_z);
					NEWTON_ASSERT(p.m_x <= p1.m_x);
					NEWTON_ASSERT(p.m_y <= p1.m_y);
					NEWTON_ASSERT(p.m_z <= p1.m_z);
					tmp = tmp->m_next;
				} while (tmp != poly);
			}
#endif

			NEWTON_ASSERT(poly);
			poly = ReduceContacts(poly, maxContacts);

			// dgPerimenterEdge *intersection;
			count = 0;
			dgPerimenterEdge *intersection = poly;
			dgVector normal = matrix.RotateVector(shapeNormal);
			do {
				contactOut[count].m_point = matrix.TransformVector(
				                                *intersection->m_vertex);
				contactOut[count].m_normal = normal;
				contactOut[count].m_penetration = penetration;
				contactOut[count].m_userId = id;
				count++;
				intersection = intersection->m_next;
			} while (intersection != poly);
		}

		return count;
	}

	dgInt32 CalculateContactsSimd(dgMinkFace *const face, dgInt32 contacID,
	                              dgContactPoint *const contactOut, dgInt32 maxContacts) {
#ifdef DG_BUILD_SIMD_CODE

		dgInt32 count = 0;
		// Get the contact form the last face
		const dgPlane &plane = *face;
		dgFloat32 penetration = plane.m_w - m_penetrationPadding;
		dgFloat32 dist = (plane % m_averVertex[face->m_vertex[0]]) * dgFloat32(0.5f);
		const dgPlane clipPlane(plane.Scale(dgFloat32(-1.0f)), dist);

		dgVector point1(clipPlane.Scale(-clipPlane.m_w));
		dgVector *const shape1 = m_hullVertex;
		dgInt32 count1 = m_referenceCollision->CalculatePlaneIntersectionSimd(
		                     clipPlane, point1, shape1);
		if (!count1) {
			dgVector p1(
			    m_referenceCollision->SupportVertexSimd(
			        clipPlane.Scale(dgFloat32(-1.0f))));
			p1 += clipPlane.Scale(DG_ROBUST_PLANE_CLIP);
			count1 = m_referenceCollision->CalculatePlaneIntersectionSimd(clipPlane,
			         p1, shape1);
			dgVector err(clipPlane.Scale(clipPlane % (point1 - p1)));
			for (dgInt32 i = 0; i < count1; i++) {
				shape1[i] += err;
			}
		}

		NEWTON_ASSERT(penetration <= dgFloat32(2.0e-1f));
		dist = GetMax(-(penetration + DG_IMPULSIVE_CONTACT_PENETRATION),
		              dgFloat32(0.0f));
		if (count1) {
			dgVector *const shape2 = &m_hullVertex[count1];
			const dgPlane clipPlane2(m_matrix.UntransformPlane(clipPlane));

			dgVector point2(clipPlane2.Scale(-clipPlane2.m_w));
			dgInt32 count2 = m_floatingcollision->CalculatePlaneIntersectionSimd(
			                     clipPlane2, point2, shape2);
			if (!count2) {
				dgVector p2(
				    m_floatingcollision->SupportVertexSimd(
				        clipPlane2.Scale(dgFloat32(-1.0f))));
				p2 += clipPlane2.Scale(DG_ROBUST_PLANE_CLIP);
				count2 = m_floatingcollision->CalculatePlaneIntersectionSimd(clipPlane2,
				         p2, shape2);
				dgVector err(clipPlane2.Scale(clipPlane2 % (point2 - p2)));
				for (dgInt32 i = 0; i < count2; i++) {
					shape2[i] += err;
				}
			}

			if (count2) {

				NEWTON_ASSERT(count1);
				NEWTON_ASSERT(count2);
				if (count1 == 1) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;

					count = 1;
					contactOut[0].m_point = matrix1.TransformVectorSimd(shape1[0]);
					contactOut[0].m_normal = matrix1.RotateVectorSimd(clipPlane);
					contactOut[0].m_userId = contacID;
					contactOut[0].m_penetration = dist;
				} else if (count2 == 1) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					// const dgMatrix& matrix2 = m_proxy->m_floatingMatrix;

					count = 1;
					// contactOut[0].m_point = matrix2.TransformVectorSimd (shape2[0]);
					contactOut[0].m_point = matrix1.TransformVectorSimd(
					                            m_matrix.TransformVectorSimd(shape2[0]));
					contactOut[0].m_normal = matrix1.RotateVectorSimd(clipPlane);
					contactOut[0].m_userId = contacID;
					contactOut[0].m_penetration = dist;

				} else if ((count1 == 2) && (count2 == 2)) {

					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					dgVector p0(shape1[0]);
					dgVector p1(shape1[1]);
					dgVector q0(m_matrix.TransformVectorSimd(shape2[0]));
					dgVector q1(m_matrix.TransformVectorSimd(shape2[1]));
					dgVector p10(p1 - p0);
					dgVector q10(q1 - q0);
					p10 = p10.Scale(
					          dgFloat32(1.0f) / dgSqrt(p10 % p10 + dgFloat32(1.0e-8f)));
					q10 = q10.Scale(
					          dgFloat32(1.0f) / dgSqrt(q10 % q10 + dgFloat32(1.0e-8f)));
					dgFloat32 dot = q10 % p10;
					if (dgAbsf(dot) > dgFloat32(0.998f)) {
						dgFloat32 pl0 = p0 % p10;
						dgFloat32 pl1 = p1 % p10;
						dgFloat32 ql0 = q0 % p10;
						dgFloat32 ql1 = q1 % p10;
						if (pl0 > pl1) {
							Swap(pl0, pl1);
							Swap(p0, p1);
							p10 = p10.Scale(dgFloat32(-1.0f));
						}
						if (ql0 > ql1) {
							Swap(ql0, ql1);
						}
						if (!((ql0 > pl1) && (ql1 < pl0))) {
							dgFloat32 clip0 = (ql0 > pl0) ? ql0 : pl0;
							dgFloat32 clip1 = (ql1 < pl1) ? ql1 : pl1;

							count = 2;
							contactOut[0].m_point = p0 + p10.Scale(clip0 - pl0);
							contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
							contactOut[0].m_userId = contacID;
							contactOut[0].m_penetration = dist;

							contactOut[1].m_point = p0 + p10.Scale(clip1 - pl0);
							contactOut[1].m_normal = matrix1.RotateVector(clipPlane);
							contactOut[1].m_userId = contacID;
							contactOut[1].m_penetration = dist;
						}

					} else {
						dgVector c0;
						dgVector c1;
						count = 1;
						dgRayToRayDistance(p0, p1, q0, q1, c0, c1);
						contactOut[0].m_point = (c0 + c1).Scale(dgFloat32(0.5f));
						contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
						contactOut[0].m_userId = contacID;
						contactOut[0].m_penetration = dist;
					}
					if (count > maxContacts) {
						count = maxContacts;
					}
					for (dgInt32 i = 0; i < count; i++) {
						contactOut[i].m_point = matrix1.TransformVectorSimd(
						                            contactOut[i].m_point);
					}

				} else {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					m_matrix.TransformVectorsSimd(shape2, shape2, count2);
					count = CalculateConvexShapeIntersectionSimd(matrix1, clipPlane,
					        dgUnsigned32(contacID), dist, count1, shape1, count2, shape2,
					        contactOut, maxContacts);
					if (!count) {
						count = CalculateContactAlternateMethod(face, contacID, contactOut,
						                                        maxContacts);
					}
				}

				dgInt32 edgeContactFlag =
				    (m_floatingcollision->IsEdgeIntersection() | m_referenceCollision->IsEdgeIntersection()) ? 1 : 0;
				for (dgInt32 i = 0; i < count; i++) {
					contactOut[i].m_isEdgeContact = edgeContactFlag;
				}
			}
		}

		return count;
#else
		return 0;
#endif
	}

	dgInt32 CalculateContacts(dgMinkFace *const face, dgInt32 contacID,
	                          dgContactPoint *const contactOut, dgInt32 maxContacts) {
		dgInt32 count = 0;
		// Get the contact form the last face
		const dgPlane &plane = *face;
		dgFloat32 penetration = plane.m_w - m_penetrationPadding;
		dgFloat32 dist = (plane % m_averVertex[face->m_vertex[0]]) * dgFloat32(0.5f);
		const dgPlane clipPlane(plane.Scale(dgFloat32(-1.0f)), dist);

		dgVector point1(clipPlane.Scale(-clipPlane.m_w));
		dgVector *const shape1 = m_hullVertex;
		dgInt32 count1 = m_referenceCollision->CalculatePlaneIntersection(clipPlane,
		                 point1, shape1);
		if (!count1) {
			dgVector p1(
			    m_referenceCollision->SupportVertex(
			        clipPlane.Scale(dgFloat32(-1.0f))));
			p1 += clipPlane.Scale(DG_ROBUST_PLANE_CLIP);
			count1 = m_referenceCollision->CalculatePlaneIntersection(clipPlane, p1,
			         shape1);
			dgVector err(clipPlane.Scale(clipPlane % (point1 - p1)));
			for (dgInt32 i = 0; i < count1; i++) {
				shape1[i] += err;
			}
		}

		NEWTON_ASSERT(penetration <= dgFloat32(2.0e-1f));
		dist = GetMax(-(penetration + DG_IMPULSIVE_CONTACT_PENETRATION),
		              dgFloat32(0.0f));
		if (count1) {
			dgVector *const shape2 = &m_hullVertex[count1];
			const dgPlane clipPlane2(m_matrix.UntransformPlane(clipPlane));

			dgVector point2(clipPlane2.Scale(-clipPlane2.m_w));
			dgInt32 count2 = m_floatingcollision->CalculatePlaneIntersection(
			                     clipPlane2, point2, shape2);
			if (!count2) {
				dgVector p2(
				    m_floatingcollision->SupportVertex(
				        clipPlane2.Scale(dgFloat32(-1.0f))));
				p2 += clipPlane2.Scale(DG_ROBUST_PLANE_CLIP);
				count2 = m_floatingcollision->CalculatePlaneIntersection(clipPlane2, p2,
				         shape2);
				dgVector err(clipPlane2.Scale(clipPlane2 % (point2 - p2)));
				for (dgInt32 i = 0; i < count2; i++) {
					shape2[i] += err;
				}
			}

			if (count2) {
				NEWTON_ASSERT(count1);
				NEWTON_ASSERT(count2);

				if (count1 == 1) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;

					count = 1;
					contactOut[0].m_point = matrix1.TransformVector(shape1[0]);
					contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
					contactOut[0].m_userId = contacID;
					contactOut[0].m_penetration = dist;
				} else if (count2 == 1) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					count = 1;
					contactOut[0].m_point = matrix1.TransformVector(
					                            m_matrix.TransformVector(shape2[0]));
					contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
					contactOut[0].m_userId = contacID;
					contactOut[0].m_penetration = dist;

				} else if ((count1 == 2) && (count2 == 2)) {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					dgVector p0(shape1[0]);
					dgVector p1(shape1[1]);
					dgVector q0(m_matrix.TransformVector(shape2[0]));
					dgVector q1(m_matrix.TransformVector(shape2[1]));
					dgVector p10(p1 - p0);
					dgVector q10(q1 - q0);
					p10 = p10.Scale(
					          dgFloat32(1.0f) / dgSqrt(p10 % p10 + dgFloat32(1.0e-8f)));
					q10 = q10.Scale(
					          dgFloat32(1.0f) / dgSqrt(q10 % q10 + dgFloat32(1.0e-8f)));
					dgFloat32 dot = q10 % p10;
					if (dgAbsf(dot) > dgFloat32(0.998f)) {
						dgFloat32 pl0 = p0 % p10;
						dgFloat32 pl1 = p1 % p10;
						dgFloat32 ql0 = q0 % p10;
						dgFloat32 ql1 = q1 % p10;
						if (pl0 > pl1) {
							Swap(pl0, pl1);
							Swap(p0, p1);
							p10 = p10.Scale(dgFloat32(-1.0f));
						}
						if (ql0 > ql1) {
							Swap(ql0, ql1);
						}
						if (!((ql0 > pl1) && (ql1 < pl0))) {
							dgFloat32 clip0 = (ql0 > pl0) ? ql0 : pl0;
							dgFloat32 clip1 = (ql1 < pl1) ? ql1 : pl1;

							count = 2;
							contactOut[0].m_point = p0 + p10.Scale(clip0 - pl0);
							contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
							contactOut[0].m_userId = contacID;
							contactOut[0].m_penetration = dist;

							contactOut[1].m_point = p0 + p10.Scale(clip1 - pl0);
							contactOut[1].m_normal = matrix1.RotateVector(clipPlane);
							contactOut[1].m_userId = contacID;
							contactOut[1].m_penetration = dist;
						}

					} else {
						dgVector c0;
						dgVector c1;
						count = 1;
						dgRayToRayDistance(p0, p1, q0, q1, c0, c1);
						contactOut[0].m_point = (c0 + c1).Scale(dgFloat32(0.5f));
						contactOut[0].m_normal = matrix1.RotateVector(clipPlane);
						contactOut[0].m_userId = contacID;
						contactOut[0].m_penetration = dist;
					}
					if (count > maxContacts) {
						count = maxContacts;
					}
					for (dgInt32 i = 0; i < count; i++) {
						contactOut[i].m_point = matrix1.TransformVector(
						                            contactOut[i].m_point);
					}

				} else {
					const dgMatrix &matrix1 = m_proxy->m_referenceMatrix;
					m_matrix.TransformTriplex(&shape2[0].m_x, sizeof(dgVector),
					                          &shape2[0].m_x, sizeof(dgVector), count2);
					count = CalculateConvexShapeIntersection(matrix1, clipPlane,
					        dgUnsigned32(contacID), dist, count1, shape1, count2, shape2,
					        contactOut, maxContacts);
					if (!count) {
						count = CalculateContactAlternateMethod(face, contacID, contactOut,
						                                        maxContacts);
					}
				}

				dgInt32 edgeContactFlag =
				    (m_floatingcollision->IsEdgeIntersection() || m_referenceCollision->IsEdgeIntersection()) ? 1 : 0;
				for (dgInt32 i = 0; i < count; i++) {
					contactOut[i].m_isEdgeContact = edgeContactFlag;
				}
			}
		}
		return count;
	}

	inline dgInt32 CalculateContactsContinuesSimd(dgInt32 contacID,
	        dgContactPoint *const contactOut, dgInt32 maxContacts,
	        const dgVector *diffPoins, const dgVector *averPoins, dgFloat32 timestep) {
#ifdef DG_BUILD_SIMD_CODE
		//      dgInt32 i;
		// dgInt32 count;
		// dgMinkFace *face;
		simd_type invMag;

		//      m_planePurge = NULL;
		//      m_facePlaneIndex = 0;
		dgMinkFace *const face = &m_simplex[0];

		//      dgVector step (veloc.Scale (timestep + DG_ROBUST_PLANE_CLIP * dgRsqrt(veloc % veloc)));
		DG_RSQRT_SIMD_S(simd_set1(m_localRelVeloc.DotProductSimd(m_localRelVeloc)),
		                invMag)
		invMag =
		    simd_mul_add_s(simd_set1(timestep), simd_set1(DG_ROBUST_PLANE_CLIP), invMag);
		dgVector step(
		    simd_mul_v((simd_type &)m_localRelVeloc, simd_permut_v(invMag, invMag, PURMUT_MASK(0, 0, 0, 0))));

		for (dgInt32 i = 0; i < 3; i++) {
			//          m_hullVertex[i] = diffPoins[i] - step;
			(simd_type &)m_hullVertex[i] =
			    simd_sub_v((simd_type &)diffPoins[i], (simd_type &)step);

			//          m_averVertex[i] = averPoins[i] + step;
			(simd_type &)m_averVertex[i] =
			    simd_add_v((simd_type &)averPoins[i], (simd_type &)step);
		}

		CalcFacePlaneSimd(face);
		dgPlane &facePlane = *face;
		if ((facePlane % m_localRelVeloc) > dgFloat32(0.0f)) {
			facePlane = facePlane.Scale(dgFloat32(-1.0f));
		}

		dgVector minkFloatingPosit(m_matrix.m_posit);
		(simd_type &)m_matrix.m_posit =
		    simd_add_v((simd_type &)m_matrix.m_posit, (simd_type &)step);
		dgInt32 count = CalculateContactsSimd(face, contacID, contactOut,
		                                      maxContacts);
		NEWTON_ASSERT(count <= maxContacts);

		m_matrix.m_posit = minkFloatingPosit;

		return count;

#else
		return 0;
#endif
	}

	inline dgInt32 CalculateContactsContinues(dgInt32 contacID,
	        dgContactPoint *const contactOut, dgInt32 maxContacts,
	        const dgVector *diffPoins, const dgVector *averPoins, dgFloat32 timestep) {
		// dgInt32 count;
		// dgMinkFace *face;

		//      m_planePurge = NULL;
		//      m_facePlaneIndex = 0;
		dgMinkFace *const face = &m_simplex[0];

		dgVector step(
		    m_localRelVeloc.Scale(
		        timestep + DG_ROBUST_PLANE_CLIP * dgRsqrt(m_localRelVeloc % m_localRelVeloc)));
		for (dgInt32 i = 0; i < 3; i++) {
			m_hullVertex[i] = diffPoins[i] - step;
			m_averVertex[i] = averPoins[i] + step;
		}
		CalcFacePlane(face);
		dgPlane &facePlane = *face;
		//      if ((m_facePlane[face->m_face] % veloc) > dgFloat32 (0.0f)) {
		if ((facePlane % m_localRelVeloc) > dgFloat32(0.0f)) {
			//          NEWTON_ASSERT (0);
			facePlane = facePlane.Scale(dgFloat32(-1.0f));
		}

		dgVector minkFloatingPosit(m_matrix.m_posit);
		m_matrix.m_posit += step;
		dgInt32 count = CalculateContacts(face, contacID, contactOut, maxContacts);
		NEWTON_ASSERT(count <= maxContacts);

		m_matrix.m_posit = minkFloatingPosit;
		return count;
	}

	dgVector ReduceLine(const dgVector &origin) {
		const dgVector &p0 = m_hullVertex[0];
		const dgVector &p1 = m_hullVertex[1];
		dgVector dp(p1 - p0);

		dgVector v;
		dgFloat32 alpha0 = ((origin - p0) % dp) / (dp % dp + dgFloat32(1.0e-24f));
		if (alpha0 > dgFloat32(1.0f)) {
			v = p1;
			m_vertexIndex = 1;
			m_hullVertex[0] = m_hullVertex[1];
			m_averVertex[0] = m_averVertex[1];

		} else if (alpha0 < dgFloat32(0.0f)) {
			v = p0;
			m_vertexIndex = 1;
		} else {
			v = p0 + dp.Scale(alpha0);
		}
		return v;
	}

	dgBigVector ReduceLineLarge(const dgBigVector &origin) {
		dgFloat64 alpha0;
		dgBigVector v;
		const dgBigVector &p0 = m_hullVertex[0];
		const dgBigVector &p1 = m_hullVertex[1];
		dgBigVector dp(p1 - p0);

		alpha0 = ((origin - p0) % dp) / (dp % dp + dgFloat64(1.0e-24f));
		if (alpha0 > dgFloat64(1.0f)) {
			v = p1;
			m_vertexIndex = 1;
			m_hullVertexLarge[0] = m_hullVertexLarge[1];
			m_averVertexLarge[0] = m_averVertexLarge[1];

		} else if (alpha0 < dgFloat64(0.0f)) {
			v = p0;
			m_vertexIndex = 1;
		} else {
			v = p0 + dp.Scale(alpha0);
		}
		return v;
	}

	dgVector ReduceTriangle(const dgVector &origin) {
		const dgVector &p0 = m_hullVertex[0];
		const dgVector &p1 = m_hullVertex[1];
		const dgVector &p2 = m_hullVertex[2];
		const dgVector p(origin);

		dgVector p10(p1 - p0);
		dgVector p20(p2 - p0);
		dgVector p_p0(p - p0);

		dgFloat32 alpha1 = p10 % p_p0;
		dgFloat32 alpha2 = p20 % p_p0;
		if ((alpha1 <= dgFloat32(0.0f)) && (alpha2 <= dgFloat32(0.0f))) {
			m_vertexIndex = 1;
			return p0;
		}

		dgVector p_p1(p - p1);
		dgFloat32 alpha3 = p10 % p_p1;
		dgFloat32 alpha4 = p20 % p_p1;
		if ((alpha3 >= dgFloat32(0.0f)) && (alpha4 <= alpha3)) {
			m_vertexIndex = 1;
			m_hullVertex[0] = p1;
			m_averVertex[0] = m_averVertex[1];
			return p1;
		}

		dgFloat32 vc = alpha1 * alpha4 - alpha3 * alpha2;
		if ((vc <= dgFloat32(0.0f)) && (alpha1 >= dgFloat32(0.0f)) && (alpha3 <= dgFloat32(0.0f))) {
			dgFloat32 t = alpha1 / (alpha1 - alpha3);
			NEWTON_ASSERT(t >= dgFloat32(0.0f));
			NEWTON_ASSERT(t <= dgFloat32(1.0f));
			m_vertexIndex = 2;
			return p0.Scale(dgFloat32(1.0f) - t) + p1.Scale(t);
		}

		dgVector p_p2(p - p2);
		dgFloat32 alpha5 = p10 % p_p2;
		dgFloat32 alpha6 = p20 % p_p2;
		if ((alpha6 >= dgFloat32(0.0f)) && (alpha5 <= alpha6)) {
			m_vertexIndex = 1;
			m_hullVertex[0] = p2;
			m_averVertex[0] = m_averVertex[2];
			return p2;
		}

		dgFloat32 vb = alpha5 * alpha2 - alpha1 * alpha6;
		if ((vb <= dgFloat32(0.0f)) && (alpha2 >= dgFloat32(0.0f)) && (alpha6 <= dgFloat32(0.0f))) {
			dgFloat32 t = alpha2 / (alpha2 - alpha6);
			NEWTON_ASSERT(t >= dgFloat32(0.0f));
			NEWTON_ASSERT(t <= dgFloat32(1.0f));
			m_vertexIndex = 2;
			m_hullVertex[1] = p2;
			m_averVertex[1] = m_averVertex[2];
			return p0.Scale(dgFloat32(1.0f) - t) + p2.Scale(t);
		}

		dgFloat32 va = alpha3 * alpha6 - alpha5 * alpha4;
		if ((va <= dgFloat32(0.0f)) && ((alpha4 - alpha3) >= dgFloat32(0.0f)) && ((alpha5 - alpha6) >= dgFloat32(0.0f))) {
			dgFloat32 t = (alpha4 - alpha3) / ((alpha4 - alpha3) + (alpha5 - alpha6));
			NEWTON_ASSERT(t >= dgFloat32(0.0f));
			NEWTON_ASSERT(t <= dgFloat32(1.0f));
			// return b + (c - b).Scale (t);
			// return dgVector (dgFloat32 (0.0f), dgFloat32 (1.0f) - t, t);
			m_vertexIndex = 2;
			m_hullVertex[0] = p2;
			m_averVertex[0] = m_averVertex[2];
			return p1.Scale(dgFloat32(1.0f) - t) + p2.Scale(t);
		}

		dgFloat32 den = dgFloat32(1.0f) / (va + vb + vc);
		dgFloat32 t = vb * den;
		dgFloat32 s = vc * den;

		NEWTON_ASSERT(t >= dgFloat32(0.0f));
		NEWTON_ASSERT(s >= dgFloat32(0.0f));
		NEWTON_ASSERT(t <= dgFloat32(1.0f));
		NEWTON_ASSERT(s <= dgFloat32(1.0f));
		m_vertexIndex = 3;
		return p0 + p10.Scale(t) + p20.Scale(s);
	}

	dgBigVector ReduceTriangleLarge(const dgBigVector &origin) {
		const dgBigVector &p0 = m_hullVertexLarge[0];
		const dgBigVector &p1 = m_hullVertexLarge[1];
		const dgBigVector &p2 = m_hullVertexLarge[2];
		const dgBigVector p(origin);

		dgBigVector p10(p1 - p0);
		dgBigVector p20(p2 - p0);
		dgBigVector p_p0(p - p0);

		dgFloat64 alpha1 = p10 % p_p0;
		dgFloat64 alpha2 = p20 % p_p0;
		if ((alpha1 <= dgFloat64(0.0f)) && (alpha2 <= dgFloat64(0.0f))) {
			m_vertexIndex = 1;
			return p0;
		}

		dgBigVector p_p1(p - p1);
		dgFloat64 alpha3 = p10 % p_p1;
		dgFloat64 alpha4 = p20 % p_p1;
		if ((alpha3 >= dgFloat64(0.0f)) && (alpha4 <= alpha3)) {
			m_vertexIndex = 1;
			m_hullVertexLarge[0] = p1;
			m_averVertexLarge[0] = m_averVertexLarge[1];
			return p1;
		}

		dgFloat64 vc = alpha1 * alpha4 - alpha3 * alpha2;
		if ((vc <= dgFloat64(0.0f)) && (alpha1 >= dgFloat64(0.0f)) && (alpha3 <= dgFloat64(0.0f))) {
			dgFloat64 t = alpha1 / (alpha1 - alpha3);
			NEWTON_ASSERT(t >= dgFloat64(0.0f));
			NEWTON_ASSERT(t <= dgFloat64(1.0f));
			m_vertexIndex = 2;
			return p0.Scale(dgFloat64(1.0f) - t) + p1.Scale(t);
		}

		dgBigVector p_p2(p - p2);
		dgFloat64 alpha5 = p10 % p_p2;
		dgFloat64 alpha6 = p20 % p_p2;
		if ((alpha6 >= dgFloat64(0.0f)) && (alpha5 <= alpha6)) {
			m_vertexIndex = 1;
			m_hullVertexLarge[0] = p2;
			m_averVertexLarge[0] = m_averVertexLarge[2];
			return p2;
		}

		dgFloat64 vb = alpha5 * alpha2 - alpha1 * alpha6;
		if ((vb <= dgFloat64(0.0f)) && (alpha2 >= dgFloat64(0.0f)) && (alpha6 <= dgFloat64(0.0f))) {
			dgFloat64 t = alpha2 / (alpha2 - alpha6);
			NEWTON_ASSERT(t >= dgFloat64(0.0f));
			NEWTON_ASSERT(t <= dgFloat64(1.0f));
			m_vertexIndex = 2;
			m_hullVertexLarge[1] = p2;
			m_averVertexLarge[1] = m_averVertexLarge[2];
			return p0.Scale(dgFloat64(1.0f) - t) + p2.Scale(t);
		}

		dgFloat64 va = alpha3 * alpha6 - alpha5 * alpha4;
		if ((va <= dgFloat64(0.0f)) && ((alpha4 - alpha3) >= dgFloat64(0.0f)) && ((alpha5 - alpha6) >= dgFloat64(0.0f))) {
			dgFloat64 t = (alpha4 - alpha3) / ((alpha4 - alpha3) + (alpha5 - alpha6));
			NEWTON_ASSERT(t >= dgFloat64(0.0f));
			NEWTON_ASSERT(t <= dgFloat64(1.0f));
			// return b + (c - b).Scale (t);
			// return dgBigVector (dgFloat64 (0.0f), dgFloat64 (1.0f) - t, t);
			m_vertexIndex = 2;
			m_hullVertexLarge[0] = p2;
			m_averVertexLarge[0] = m_averVertexLarge[2];
			return p1.Scale(dgFloat64(1.0f) - t) + p2.Scale(t);
		}

		dgFloat64 den = dgFloat64(1.0f) / (va + vb + vc);
		dgFloat64 t = vb * den;
		dgFloat64 s = vc * den;

		NEWTON_ASSERT(t >= dgFloat64(0.0f));
		NEWTON_ASSERT(s >= dgFloat64(0.0f));
		NEWTON_ASSERT(t <= dgFloat64(1.0f));
		NEWTON_ASSERT(s <= dgFloat64(1.0f));
		m_vertexIndex = 3;
		return p0 + p10.Scale(t) + p20.Scale(s);
	}

	dgVector ReduceTetrahedrum(const dgVector &origin) {
		dgInt32 index0 = -1;
		dgInt32 index1 = -1;
		dgInt32 index2 = -1;
		dgVector p(origin);
		dgFloat32 minDist = dgFloat32(1.0e20f);
		for (dgInt32 i = 0; i < 4; i++) {
			dgInt32 i0 = m_faceIndex[i][0];
			dgInt32 i1 = m_faceIndex[i][1];
			dgInt32 i2 = m_faceIndex[i][2];
			const dgVector &p0 = m_hullVertex[i0];
			const dgVector &p1 = m_hullVertex[i1];
			const dgVector &p2 = m_hullVertex[i2];

			dgVector p10(p1 - p0);
			dgVector p20(p2 - p0);
			const dgVector p_p0(origin - p0);
			dgFloat32 volume = p_p0 % (p10 * p20);
			if (volume < dgFloat32(0.0f)) {
				dgVector q(dgPointToTriangleDistance(origin, p0, p1, p2));
				dgVector qDist(q - origin);

				dgFloat32 dist = qDist % qDist;
				if (dist < minDist) {
					p = q;
					index0 = i0;
					index1 = i1;
					index2 = i2;
					minDist = dist;
				}
			}
		}

		if (index0 != -1) {
			dgVector tmpSum[2];
			dgVector tmpDiff[2];

			tmpDiff[0] = m_hullVertex[index1];
			tmpDiff[1] = m_hullVertex[index2];
			tmpSum[0] = m_averVertex[index1];
			tmpSum[1] = m_averVertex[index2];

			m_hullVertex[0] = m_hullVertex[index0];
			m_hullVertex[1] = tmpDiff[0];
			m_hullVertex[2] = tmpDiff[1];
			m_averVertex[0] = m_averVertex[index0];
			m_averVertex[1] = tmpSum[0];
			m_averVertex[2] = tmpSum[1];
			return ReduceTriangle(origin);
		}
		return p;
	}

	dgBigVector ReduceTetrahedrumLarge(const dgBigVector &origin) {
		//      dgInt32 i;
		//      dgInt32 i0;
		//      dgInt32 i1;
		//      dgInt32 i2;
		//      dgInt32 index0;
		//      dgInt32 index1;
		//      dgInt32 index2;
		//      dgFloat64 dist;
		//      dgFloat64 volume;
		//      dgFloat64 minDist;
		//      dgBigVector tmpSum[2];
		//      dgBigVector tmpDiff[2];

		dgInt32 index0 = -1;
		dgInt32 index1 = -1;
		dgInt32 index2 = -1;
		dgBigVector p(origin);
		dgFloat64 minDist = dgFloat32(1.0e20f);
		for (dgInt32 i = 0; i < 4; i++) {
			dgInt32 i0 = m_faceIndex[i][0];
			dgInt32 i1 = m_faceIndex[i][1];
			dgInt32 i2 = m_faceIndex[i][2];
			const dgBigVector &p0 = m_hullVertexLarge[i0];
			const dgBigVector &p1 = m_hullVertexLarge[i1];
			const dgBigVector &p2 = m_hullVertexLarge[i2];

			dgBigVector p10(p1 - p0);
			dgBigVector p20(p2 - p0);
			dgBigVector p_p0(origin - p0);
			dgFloat64 volume = p_p0 % (p10 * p20);
			if (volume < dgFloat64(0.0f)) {
				dgBigVector q(dgPointToTriangleDistance(origin, p0, p1, p2));
				dgBigVector qDist(q - origin);

				dgFloat64 dist = qDist % qDist;
				if (dist < minDist) {
					p = q;
					index0 = i0;
					index1 = i1;
					index2 = i2;
					minDist = dist;
				}
			}
		}

		if (index0 != -1) {
			dgBigVector tmpSum[2];
			dgBigVector tmpDiff[2];
			tmpDiff[0] = m_hullVertexLarge[index1];
			tmpDiff[1] = m_hullVertexLarge[index2];
			tmpSum[0] = m_averVertexLarge[index1];
			tmpSum[1] = m_averVertexLarge[index2];

			m_hullVertexLarge[0] = m_hullVertexLarge[index0];
			m_hullVertexLarge[1] = tmpDiff[0];
			m_hullVertexLarge[2] = tmpDiff[1];
			m_averVertexLarge[0] = m_averVertexLarge[index0];
			m_averVertexLarge[1] = tmpSum[0];
			m_averVertexLarge[2] = tmpSum[1];
			return ReduceTriangleLarge(origin);
		}
		return p;
	}

	dgMinkReturnCode UpdateSeparatingPlaneFallbackSolution(dgMinkFace *&plane,
	        const dgVector &origin) {
		//      dgInt32 cicling;
		//      dgFloat32 minDist;
		//      dgMinkReturnCode code;

		dgInt32 cicling = -1;
		dgFloat32 minDist = dgFloat32(1.0e20f);
		dgMinkReturnCode code = dgMinkError;

		dgVector v(ReduceTetrahedrum(origin) - origin);
		dgVector dir0(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		              dgFloat32(0.0f));
		for (dgInt32 i = 0;
		        (i < DG_FALLBACK_SEPARATING_PLANE_ITERATIONS) && (m_vertexIndex < 4);
		        i++) {
			dgFloat32 dist = v % v;
			if (dist < dgFloat32(1.0e-9f)) {
				switch (m_vertexIndex) {
				case 1: {
					// dgInt32 i;
					// dgInt32 best;
					// dgFloat32 maxErr;

					dgInt32 best = 0;
					dgFloat32 maxErr = dgFloat32(0.0f);
					dgInt32 j = 0;
					for (; j < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); j++) {
						// dgFloat32 error2;
						CalcSupportVertex(m_dir[j], 1);
						dgVector e(m_hullVertex[1] - m_hullVertex[0]);
						dgFloat32 error2 = e % e;
						if (error2 > dgFloat32(1.0e-4f)) {
							break;
						}
						if (error2 > maxErr) {
							best = j;
							maxErr = error2;
						}
					}

					if (j == dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
						NEWTON_ASSERT(maxErr > dgFloat32(0.0f));
						CalcSupportVertex(m_dir[best], 1);
					}
					m_vertexIndex = 2;
				}
				// FIXME: fallthrough intended?
				// fallthrough

				case 2: {
					dgInt32 best = 0;
					dgFloat32 maxErr = dgFloat32(0.0f);
					dgVector e0(m_hullVertex[1] - m_hullVertex[0]);
					dgInt32 j = 0;
					for (; j < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); j++) {
						CalcSupportVertex(m_dir[j], 2);
						dgVector e1(m_hullVertex[2] - m_hullVertex[0]);
						dgVector n(e0 * e1);
						dgFloat32 error2 = n % n;
						if (error2 > dgFloat32(1.0e-4f)) {
							break;
						}
						if (error2 > maxErr) {
							best = j;
							maxErr = error2;
						}
					}

					if (j == dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
						NEWTON_ASSERT(maxErr > dgFloat32(0.0f));
						CalcSupportVertex(m_dir[best], 2);
					}
					m_vertexIndex = 3;
				}
				// FIXME: fallthrough intended?
				// fallthrough

				default: {
					const dgVector &p0 = m_hullVertex[0];
					const dgVector &p1 = m_hullVertex[1];
					const dgVector &p2 = m_hullVertex[2];
					dgVector normal((p1 - p0) * (p2 - p0));
					dgFloat32 mag2 = normal % normal;
					NEWTON_ASSERT(mag2 > dgFloat32(1.0e-10f));
					normal = normal.Scale(dgRsqrt(mag2));
					CalcSupportVertex(normal, 3);
					CalcSupportVertex(normal.Scale(dgFloat32(-1.0f)), 4);
					if (dgAbsf((m_hullVertex[4] - p0) % normal) > dgAbsf((m_hullVertex[3] - p0) % normal)) {
						m_hullVertex[3] = m_hullVertex[4];
						m_averVertex[3] = m_averVertex[4];
					}
					m_vertexIndex = 4;

					if (!CheckTetraHedronVolume()) {
						Swap(m_hullVertex[2], m_hullVertex[1]);
						Swap(m_averVertex[2], m_averVertex[1]);
						NEWTON_ASSERT(CheckTetraHedronVolume());
					}
				}
				}
				return dgMinkIntersecting;
			}

			if (dist < minDist) {
				minDist = dist;
				cicling = -1;
			}

			NEWTON_ASSERT(dist > dgFloat32(1.0e-24f));
			dgVector dir(v.Scale(-dgRsqrt(dist)));
			dist = dir0 % dir;
			if (dist < dgFloat32(0.9995f)) {
				CalcSupportVertex(dir, m_vertexIndex);
				dgVector w(m_hullVertex[m_vertexIndex] - origin);
				dgVector wv(w - v);
				dist = dir % wv;
			} else {
				dist = dgFloat32(0.0f);
			}

			cicling++;
			if (cicling > 4) {
				dist = dgFloat32(0.0f);
			}

			dir0 = dir;
			if (dist < dgFloat32(5.0e-4f)) {
				dgMatrix rotMatrix;
				dgPlane separatingPlane(dir.Scale(dgFloat32(-1.0f)), origin % dir);
				switch (m_vertexIndex) {
				case 1: {
					dgFloat32 minDistance = dgFloat32(1.0e10f);
					rotMatrix = dgMatrix(dir);
					NEWTON_ASSERT(rotMatrix.m_front.m_w == dgFloat32(0.0f));
					NEWTON_ASSERT(rotMatrix.m_up.m_w == dgFloat32(0.0f));
					NEWTON_ASSERT(rotMatrix.m_right.m_w == dgFloat32(0.0f));

					dgInt32 keepSeaching = 1;
					dgVector dir1(dgFloat32(1.0), dgFloat32(0.0f), dgFloat32(0.0f),
					              dgFloat32(0.0f));
					// 2.0 degree rotation
					dgVector yawRoll(dgFloat32(0.99939083f), dgFloat32(0.0f),
					                 dgFloat32(0.034899497f), dgFloat32(0.0f));
					// 45 degree rotation
					dgVector yawPitch(dgFloat32(0.0f), dgFloat32(0.70710678f),
					                  dgFloat32(0.70710678f), dgFloat32(0.0f));
					//                      do {
					for (dgInt32 j = 0; keepSeaching && (j < 180); j++) {
						dgFloat32 val;
						dgVector tmp(yawRoll.m_x * dir1.m_x - yawRoll.m_z * dir1.m_z,
						             dgFloat32(0.0f),
						             yawRoll.m_z * dir1.m_x + yawRoll.m_x * dir1.m_z,
						             dgFloat32(0.0f));

						val = tmp % tmp;
						if (dgAbsf(val - dgFloat32(1.0f)) > dgFloat32(1.0e-4f)) {
							tmp = tmp.Scale(dgFloat32(1.0f) / dgSqrt(val));
						}

						dir1 = tmp;
						dgVector dir2(dir1);
						for (dgInt32 k = 0; k < 8; k++) {
							dgVector tmpV(dir2.m_x,
							              dir2.m_y * yawPitch.m_y - dir2.m_z * yawPitch.m_z,
							              dir2.m_y * yawPitch.m_z + dir2.m_z * yawPitch.m_y,
							              dgFloat32(0.0f));

							NEWTON_ASSERT(
							    dgAbsf((tmpV % tmpV) - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));

							dir2 = tmpV;
							tmpV = rotMatrix.RotateVector(dir2);
							CalcSupportVertex(tmpV, 2);
							dgVector err0(m_hullVertex[2] - m_hullVertex[0]);
							val = err0 % err0;
							if (val > DG_FALLBACK_SEPARATING_DIST_TOLERANCE) {
								val = separatingPlane.Evalue(m_hullVertex[2]);
								NEWTON_ASSERT(val > dgFloat32(0.0f));
								if (val < minDistance) {
									keepSeaching = 0;
									minDistance = val;
									m_hullVertex[1] = m_hullVertex[2];
									m_averVertex[1] = m_averVertex[2];
								}
							}
						}
						//                      } while (keepSeaching);
					}

					if (keepSeaching) {
						return dgMinkDisjoint;
					}
				}
				// FIXME: fallthrough intended?
				// fallthrough

				case 2: {
					rotMatrix.m_front = dir;
					rotMatrix.m_up = m_hullVertex[1] - m_hullVertex[0];
					dgFloat32 mag2 = rotMatrix.m_up % rotMatrix.m_up;
					NEWTON_ASSERT(mag2 > dgFloat32(1.0e-24f));
					rotMatrix.m_up = rotMatrix.m_up.Scale(dgRsqrt(mag2));
					rotMatrix.m_right = rotMatrix.m_front * rotMatrix.m_up;

					rotMatrix.m_front.m_w = dgFloat32(0.0f);
					rotMatrix.m_up.m_w = dgFloat32(0.0f);
					rotMatrix.m_right.m_w = dgFloat32(0.0f);

					dgFloat32 val = dgFloat32(0.0f);
					// 2.0 degree rotation
					dgVector rot(dgFloat32(0.99939083f), dgFloat32(0.0f),
					             dgFloat32(0.034899497f), dgFloat32(0.0f));
					dgVector dir1(dgFloat32(1.0), dgFloat32(0.0f), dgFloat32(0.0f),
					              dgFloat32(0.0f));
					do {
						dgVector tmp(rot.m_x * dir1.m_x - rot.m_z * dir1.m_z,
						             dgFloat32(0.0f), rot.m_z * dir1.m_x + rot.m_x * dir1.m_z,
						             dgFloat32(0.0f));

						dir1 = tmp;
						tmp = rotMatrix.RotateVector(dir1);
						tmp = tmp.Scale(dgRsqrt(tmp % tmp));
						CalcSupportVertex(tmp, 2);
						dgVector err0(m_hullVertex[2] - m_hullVertex[0]);
						val = err0 % err0;
						if (val > DG_FALLBACK_SEPARATING_DIST_TOLERANCE) {
							dgVector err1(m_hullVertex[2] - m_hullVertex[1]);
							val = err1 % err1;
						}
					} while (val < DG_FALLBACK_SEPARATING_DIST_TOLERANCE);
#ifdef _DEBUG
					dgFloat32 test = (m_hullVertex[0] - m_hullVertex[2]) % dir;
					NEWTON_ASSERT(test >= dgFloat32(-2.0e-1f));
#endif
					dir1 = dgVector(dgFloat32(1.0), dgFloat32(0.0f), dgFloat32(0.0f),
					                dgFloat32(0.0f));

					do {
						dgVector tmp(rot.m_x * dir1.m_x + rot.m_z * dir1.m_z,
						             dgFloat32(0.0f), rot.m_x * dir1.m_z - rot.m_z * dir1.m_x,
						             dgFloat32(0.0f));

						dir1 = tmp;
						tmp = rotMatrix.RotateVector(dir1);
						tmp = tmp.Scale(dgRsqrt(tmp % tmp));
						CalcSupportVertex(tmp, 3);
						dgVector err0(m_hullVertex[3] - m_hullVertex[0]);
						val = err0 % err0;
						if (val > DG_FALLBACK_SEPARATING_DIST_TOLERANCE) {
							dgVector err1(m_hullVertex[3] - m_hullVertex[1]);
							val = err1 % err1;
						}
					} while (val < DG_FALLBACK_SEPARATING_DIST_TOLERANCE);
#ifdef _DEBUG
					dgFloat32 test1 = (m_hullVertex[0] - m_hullVertex[3]) % dir;
					NEWTON_ASSERT(test1 >= dgFloat32(-2.0e-1f));
#endif

					dgFloat32 dist2 = separatingPlane.Evalue(m_hullVertex[2]);
					dgFloat32 dist3 = separatingPlane.Evalue(m_hullVertex[3]);
					NEWTON_ASSERT(dist2 > dgFloat32(0.0f));
					NEWTON_ASSERT(dist3 > dgFloat32(0.0f));
					if (dist3 < dist2) {
						m_hullVertex[2] = m_hullVertex[3];
						m_averVertex[2] = m_averVertex[3];
						dist2 = dist3;
					}
				}
				// FIXME: fallthrough intended?
				// fallthrough

				case 3: {
					CalcSupportVertex(separatingPlane, 3);
				}
					// FIXME: fallthrough intended?
					// fallthrough
				}

				m_vertexIndex = 4;
				plane = &m_simplex[0];
				if (!CheckTetraHedronVolume()) {
					Swap(m_hullVertex[2], m_hullVertex[1]);
					Swap(m_averVertex[2], m_averVertex[1]);
					NEWTON_ASSERT(CheckTetraHedronVolume());
				}

				return dgMinkDisjoint;
			}

			m_vertexIndex++;
			switch (m_vertexIndex) {
			case 1: {
				NEWTON_ASSERT(0);
				break;
			}

			case 2: {
				v = ReduceLine(origin) - origin;
				break;
			}

			case 3: {
				v = ReduceTriangle(origin) - origin;
				break;
			}

			case 4: {
				v = ReduceTetrahedrum(origin) - origin;
				break;
			}
			}
		}

		if (m_vertexIndex == 4) {
			dgFloat32 minDistance;
			if (!CheckTetraHedronVolume()) {
				Swap(m_hullVertex[2], m_hullVertex[1]);
				Swap(m_averVertex[2], m_averVertex[1]);
				NEWTON_ASSERT(CheckTetraHedronVolume());
			}

			minDistance = dgFloat32(1.0e20f);
			for (dgInt32 i = 0; i < 4; i++) {
				// dgFloat32 dist;
				dgInt32 i0 = m_faceIndex[i][0];
				dgInt32 i1 = m_faceIndex[i][1];
				dgInt32 i2 = m_faceIndex[i][2];

				NEWTON_ASSERT(i0 == m_simplex[i].m_vertex[0]);
				NEWTON_ASSERT(i1 == m_simplex[i].m_vertex[1]);
				NEWTON_ASSERT(i2 == m_simplex[i].m_vertex[2]);

				const dgVector &p0 = m_hullVertex[i0];
				const dgVector &p1 = m_hullVertex[i1];
				const dgVector &p2 = m_hullVertex[i2];
				dgVector e0(p1 - p0);
				dgVector e1(p2 - p0);
				dgVector n(e0 * e1);

				dgFloat32 dist = n % n;
				NEWTON_ASSERT(dist > dgFloat32(1.0e-20f));
				if (dist > DG_DISTANCE_TOLERANCE_ZERO) {
					n = n.Scale(dgRsqrt(dist));
					dist = dgAbsf(n % (origin - p0));
					if (dist < minDistance) {
						minDistance = dist;
						plane = &m_simplex[i];
					}
				}
			}
			NEWTON_ASSERT(plane);
			code = dgMinkIntersecting;
		}
#ifdef _DEBUG
		if (m_vertexIndex < 4) {
			NEWTON_ASSERT(0);
#ifdef _MSC_VER
			dgTrace(("too many iterations  in: %s\n", __FUNCDNAME__));
#else
			dgTrace(("too many iterations  in: %s\n", __PRETTY_FUNCTION__));
#endif
		}
#endif

		return code;
	}

	dgMinkReturnCode UpdateSeparatingPlaneFallbackSolutionLarge(
	    dgMinkFace *&plane, const dgBigVector &origin) {
		//      dgInt32 cicling;
		//      dgFloat64 minDist;
		//      dgMinkReturnCode code;

		dgInt32 cicling = -1;
		dgFloat64 minDist = dgFloat64(1.0e20f);
		dgMinkReturnCode code = dgMinkError;

		dgBigVector v(ReduceTetrahedrumLarge(origin) - origin);
		dgBigVector dir0(dgFloat64(0.0f), dgFloat64(0.0f), dgFloat64(0.0f),
		                 dgFloat64(0.0f));

		for (dgInt32 i = 0;
		        (i < DG_FALLBACK_SEPARATING_PLANE_ITERATIONS) && (m_vertexIndex < 4);
		        i++) {
			dgFloat64 dist = v % v;
			if (dist < dgFloat64(1.0e-9f)) {
				switch (m_vertexIndex) {
				case 1: {
					dgInt32 best = 0;
					dgFloat64 maxErr = dgFloat64(0.0f);
					dgInt32 j = 0;
					for (; j < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); j++) {
						dgFloat64 error2;
						CalcSupportVertexLarge(m_dir[j], 1);
						dgBigVector e(m_hullVertexLarge[1] - m_hullVertexLarge[0]);
						error2 = e % e;
						if (error2 > dgFloat64(1.0e-4f)) {
							break;
						}
						if (error2 > maxErr) {
							best = j;
							maxErr = error2;
						}
					}

					if (j == dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
						NEWTON_ASSERT(maxErr > dgFloat64(0.0f));
						CalcSupportVertexLarge(m_dir[best], 1);
					}
					m_vertexIndex = 2;
				}
				// FIXME: fallthrough intended?
				// fallthrough

				case 2: {
					dgInt32 best = 0;
					dgFloat64 maxErr = dgFloat64(0.0f);
					dgBigVector e0(m_hullVertexLarge[1] - m_hullVertexLarge[0]);
					dgInt32 j = 0;
					for (; j < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); j++) {
						dgFloat64 error2;
						CalcSupportVertexLarge(m_dir[j], 2);
						dgBigVector e1(m_hullVertexLarge[2] - m_hullVertexLarge[0]);
						dgBigVector n(e0 * e1);
						error2 = n % n;
						if (error2 > dgFloat64(1.0e-4f)) {
							break;
						}
						if (error2 > maxErr) {
							best = j;
							maxErr = error2;
						}
					}

					if (j == dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
						NEWTON_ASSERT(maxErr > dgFloat64(0.0f));
						CalcSupportVertexLarge(m_dir[best], 2);
					}
					m_vertexIndex = 3;
				}
				// FIXME: fallthrough intended?
				// fallthrough

				default: {
					const dgBigVector &p0 = m_hullVertexLarge[0];
					const dgBigVector &p1 = m_hullVertexLarge[1];
					const dgBigVector &p2 = m_hullVertexLarge[2];
					dgBigVector normal((p1 - p0) * (p2 - p0));
					dgFloat64 mag2 = normal % normal;
					NEWTON_ASSERT(mag2 > dgFloat64(1.0e-10f));
					normal = normal.Scale(dgFloat64(1.0f) / sqrt(mag2));
					dgVector dir(dgFloat32(normal.m_x), dgFloat32(normal.m_y),
					             dgFloat32(normal.m_z), dgFloat32(0.0f));
					CalcSupportVertexLarge(dir, 3);
					CalcSupportVertexLarge(dir.Scale(dgFloat32(-1.0f)), 4);
					if (fabs((m_hullVertexLarge[4] - p0) % normal) > fabs((m_hullVertexLarge[3] - p0) % normal)) {
						m_hullVertexLarge[3] = m_hullVertexLarge[4];
						m_averVertexLarge[3] = m_averVertexLarge[4];
					}
					m_vertexIndex = 4;

					if (!CheckTetraHedronVolumeLarge()) {
						Swap(m_hullVertexLarge[2], m_hullVertexLarge[1]);
						Swap(m_averVertexLarge[2], m_averVertexLarge[1]);
						NEWTON_ASSERT(CheckTetraHedronVolumeLarge());
					}
				}
				}
				return dgMinkIntersecting;
			}

			if (dist < minDist) {
				minDist = dist;
				cicling = -1;
			}

			NEWTON_ASSERT(dist > dgFloat64(1.0e-24f));
			dgBigVector dir(v.Scale(-dgFloat64(1.0f) / sqrt(dist)));
			dist = dir0 % dir;
			if (dist < dgFloat64(0.9995f)) {
				dgVector dir1(dgFloat32(dir.m_x), dgFloat32(dir.m_y),
				              dgFloat32(dir.m_z), dgFloat32(0.0f));
				CalcSupportVertexLarge(dir1, m_vertexIndex);
				dgBigVector w(m_hullVertexLarge[m_vertexIndex] - origin);
				dgBigVector wv(w - v);
				dist = dir % wv;
			} else {
				dist = dgFloat64(0.0f);
			}

			cicling++;
			if (cicling > 4) {
				dist = dgFloat64(0.0f);
			}

			dir0 = dir;
			if (dist < dgFloat64(5.0e-4f)) {
				dgMatrix rotMatrix;
				//              dgBigPlane separatingPlane (dir.Scale (dgFloat64 (-1.0f)), origin % dir);
				dgVector dir32(dgFloat32(dir.m_x), dgFloat32(dir.m_y),
				               dgFloat32(dir.m_z), dgFloat32(0.0f));
				dgPlane separatingPlane(dir32.Scale(dgFloat32(-1.0f)),
				                        dgFloat32(origin % dir));

				switch (m_vertexIndex) {
				case 1: {
					dgFloat64 minDistance = dgFloat64(1.0e10f);
					rotMatrix = dgMatrix(dir32);
					NEWTON_ASSERT(rotMatrix.m_front.m_w == dgFloat64(0.0f));
					NEWTON_ASSERT(rotMatrix.m_up.m_w == dgFloat64(0.0f));
					NEWTON_ASSERT(rotMatrix.m_right.m_w == dgFloat64(0.0f));

					dgInt32 keepSeaching = 1;
					dgVector dir1(dgFloat32(1.0), dgFloat32(0.0f), dgFloat32(0.0f),
					              dgFloat64(0.0f));
					// 2.0 degree rotation
					dgVector yawRoll(dgFloat32(0.99939083f), dgFloat32(0.0f),
					                 dgFloat32(0.034899497f), dgFloat32(0.0f));
					// 45 degree rotation
					dgVector yawPitch(dgFloat32(0.0f), dgFloat64(0.70710678f),
					                  dgFloat32(0.70710678f), dgFloat32(0.0f));
					for (dgInt32 j = 0; keepSeaching && (j < 180); j++) {
						//                      do {
						dgVector tmp(yawRoll.m_x * dir1.m_x - yawRoll.m_z * dir1.m_z,
						             dgFloat64(0.0f),
						             yawRoll.m_z * dir1.m_x + yawRoll.m_x * dir1.m_z,
						             dgFloat64(0.0f));

						dgFloat32 val1 = tmp % tmp;
						if (dgAbsf(val1 - dgFloat32(1.0f)) > dgFloat32(1.0e-4f)) {
							tmp = tmp.Scale(dgFloat32(1.0f) / dgSqrt(dgFloat32(val1)));
						}

						dir1 = tmp;
						dgVector dir2(dir1);
						for (dgInt32 k = 0; k < 8; k++) {
							dgVector tmp2(dir2.m_x,
							              dir2.m_y * yawPitch.m_y - dir2.m_z * yawPitch.m_z,
							              dir2.m_y * yawPitch.m_z + dir2.m_z * yawPitch.m_y,
							              dgFloat64(0.0f));

							//                              NEWTON_ASSERT (dgAbsf ((tmp2 % tmp2) - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));

							dir2 = tmp2;
							tmp2 = rotMatrix.RotateVector(dir2);
							CalcSupportVertexLarge(tmp2, 2);
							dgBigVector err0(m_hullVertex[2] - m_hullVertex[0]);
							dgFloat64 val = err0 % err0;
							if (val > DG_FALLBACK_SEPARATING_DIST_TOLERANCE) {
								//                                  val = separatingPlane.Evalue(m_hullVertex[2]);
								val = separatingPlane.m_x * m_hullVertexLarge[2].m_x + separatingPlane.m_y * m_hullVertexLarge[2].m_y + separatingPlane.m_z * m_hullVertexLarge[2].m_z + separatingPlane.m_w;
								NEWTON_ASSERT(val > dgFloat64(0.0f));
								if (val < minDistance) {
									keepSeaching = 0;
									minDistance = val;
									m_hullVertexLarge[1] = m_hullVertexLarge[2];
									m_averVertexLarge[1] = m_averVertexLarge[2];
								}
							}
						}
						//                      } while (keepSeaching);
					}
					if (keepSeaching) {
						return dgMinkDisjoint;
					}
				}
				// FIXME: fallthrough intended?
				// fallthrough

				case 2: {
					rotMatrix.m_front = dir32;
					// rotMatrix.m_up = m_hullVertexLarge[1] - m_hullVertexLarge[0];
					// mag2 = rotMatrix.m_up % rotMatrix.m_up;
					dgBigVector up(m_hullVertexLarge[1] - m_hullVertexLarge[0]);
					dgFloat64 mag2 = up % up;
					NEWTON_ASSERT(mag2 > dgFloat64(1.0e-24f));
					// rotMatrix.m_up = rotMatrix.m_up.Scale(dgRsqrt (mag2));
					up = up.Scale(dgFloat64(1.0f) / sqrt(mag2));
					rotMatrix.m_up = dgVector(dgFloat32(up.m_x), dgFloat32(up.m_y),
					                          dgFloat32(up.m_z), dgFloat32(0.0f));
					rotMatrix.m_right = rotMatrix.m_front * rotMatrix.m_up;

					// rotMatrix.m_front.m_w = dgFloat64 (0.0f);
					// rotMatrix.m_up.m_w    = dgFloat64 (0.0f);
					rotMatrix.m_right.m_w = dgFloat64(0.0f);

					dgFloat64 val = dgFloat64(0.0f);
					// 2.0 degree rotation
					dgVector rot(dgFloat32(0.99939083f), dgFloat32(0.0f),
					             dgFloat32(0.034899497f), dgFloat32(0.0f));
					dgVector dir1(dgFloat64(1.0), dgFloat64(0.0f), dgFloat64(0.0f),
					              dgFloat64(0.0f));
					do {
						dgVector tmp(rot.m_x * dir1.m_x - rot.m_z * dir1.m_z,
						             dgFloat64(0.0f), rot.m_z * dir1.m_x + rot.m_x * dir1.m_z,
						             dgFloat64(0.0f));

						dir1 = tmp;
						tmp = rotMatrix.RotateVector(dir1);
						tmp = tmp.Scale(dgRsqrt(tmp % tmp));
						CalcSupportVertexLarge(tmp, 2);
						dgBigVector err0(m_hullVertexLarge[2] - m_hullVertexLarge[0]);
						val = err0 % err0;
						if (val > DG_FALLBACK_SEPARATING_DIST_TOLERANCE) {
							dgBigVector err1(m_hullVertexLarge[2] - m_hullVertexLarge[1]);
							val = err1 % err1;
						}
					} while (val < DG_FALLBACK_SEPARATING_DIST_TOLERANCE);
					NEWTON_ASSERT(
					    ((m_hullVertexLarge[0] - m_hullVertexLarge[2]) % dir) >= dgFloat64(-1.0e-1f));
					dir1 = dgVector(dgFloat64(1.0), dgFloat64(0.0f), dgFloat64(0.0f),
					                dgFloat64(0.0f));

					do {
						dgVector tmp(rot.m_x * dir1.m_x + rot.m_z * dir1.m_z,
						             dgFloat64(0.0f), rot.m_x * dir1.m_z - rot.m_z * dir1.m_x,
						             dgFloat64(0.0f));

						dir1 = tmp;
						tmp = rotMatrix.RotateVector(dir1);
						tmp = tmp.Scale(dgRsqrt(tmp % tmp));
						CalcSupportVertexLarge(tmp, 3);
						dgBigVector err0(m_hullVertexLarge[3] - m_hullVertexLarge[0]);
						val = err0 % err0;
						if (val > DG_FALLBACK_SEPARATING_DIST_TOLERANCE) {
							dgBigVector err1(m_hullVertexLarge[3] - m_hullVertexLarge[1]);
							val = err1 % err1;
						}
					} while (val < DG_FALLBACK_SEPARATING_DIST_TOLERANCE);
					NEWTON_ASSERT(
					    ((m_hullVertexLarge[0] - m_hullVertexLarge[3]) % dir) >= dgFloat64(-1.0e-1f));

					//                      dist2 = separatingPlane.Evalue(m_hullVertexLarge[2]);
					//                      dist3 = separatingPlane.Evalue(m_hullVertexLarge[3]);
					dgFloat64 dist2 = separatingPlane.m_x * m_hullVertexLarge[2].m_x + separatingPlane.m_y * m_hullVertexLarge[2].m_y + separatingPlane.m_z * m_hullVertexLarge[2].m_z + separatingPlane.m_w;
					dgFloat64 dist3 = separatingPlane.m_x * m_hullVertexLarge[3].m_x + separatingPlane.m_y * m_hullVertexLarge[3].m_y + separatingPlane.m_z * m_hullVertexLarge[3].m_z + separatingPlane.m_w;

					NEWTON_ASSERT(dist2 > dgFloat64(0.0f));
					NEWTON_ASSERT(dist3 > dgFloat64(0.0f));
					if (dist3 < dist2) {
						m_hullVertexLarge[2] = m_hullVertexLarge[3];
						m_averVertexLarge[2] = m_averVertexLarge[3];
						dist2 = dist3;
					}
				}
				// FIXME: fallthrough intended?
				// fallthrough

				case 3: {
					CalcSupportVertexLarge(separatingPlane, 3);
				}
					// FIXME: fallthrough intended?
					// fallthrough
				}

				m_vertexIndex = 4;
				plane = &m_simplex[0];
				if (!CheckTetraHedronVolumeLarge()) {
					Swap(m_hullVertexLarge[2], m_hullVertexLarge[1]);
					Swap(m_averVertexLarge[2], m_averVertexLarge[1]);
					NEWTON_ASSERT(CheckTetraHedronVolumeLarge());
				}

				return dgMinkDisjoint;
			}

			m_vertexIndex++;
			switch (m_vertexIndex) {
			case 1: {
				NEWTON_ASSERT(0);
				break;
			}

			case 2: {
				v = ReduceLineLarge(origin) - origin;
				break;
			}

			case 3: {
				v = ReduceTriangleLarge(origin) - origin;
				break;
			}

			case 4: {
				v = ReduceTetrahedrumLarge(origin) - origin;
				break;
			}
			}
		}

		if (m_vertexIndex == 4) {
			if (!CheckTetraHedronVolumeLarge()) {
				Swap(m_hullVertexLarge[2], m_hullVertexLarge[1]);
				Swap(m_averVertexLarge[2], m_averVertexLarge[1]);
				NEWTON_ASSERT(CheckTetraHedronVolumeLarge());
			}

			dgFloat64 minDistance = dgFloat64(1.0e20f);
			for (dgInt32 i = 0; i < 4; i++) {
				dgInt32 i0 = m_faceIndex[i][0];
				dgInt32 i1 = m_faceIndex[i][1];
				dgInt32 i2 = m_faceIndex[i][2];

				NEWTON_ASSERT(i0 == m_simplex[i].m_vertex[0]);
				NEWTON_ASSERT(i1 == m_simplex[i].m_vertex[1]);
				NEWTON_ASSERT(i2 == m_simplex[i].m_vertex[2]);

				const dgBigVector &p0 = m_hullVertexLarge[i0];
				const dgBigVector &p1 = m_hullVertexLarge[i1];
				const dgBigVector &p2 = m_hullVertexLarge[i2];
				dgBigVector e0(p1 - p0);
				dgBigVector e1(p2 - p0);
				dgBigVector n(e0 * e1);

				dgFloat64 dist = n % n;
				NEWTON_ASSERT(dist > dgFloat64(1.0e-20f));
				if (dist > DG_DISTANCE_TOLERANCE_ZERO) {
					n = n.Scale(dgFloat32(1.0f) / sqrt(dist));
					dist = fabs(n % (origin - p0));
					if (dist < minDistance) {
						minDistance = dist;
						plane = &m_simplex[i];
					}
				}
			}
			NEWTON_ASSERT(plane);
			code = dgMinkIntersecting;
		}
#ifdef _DEBUG
		if (m_vertexIndex < 4) {
			NEWTON_ASSERT(0);
#ifdef _MSC_VER
			dgTrace(("too many iterations  in: %s\n", __FUNCDNAME__));
#else
			dgTrace(("too many iterations  in: %s\n", __PRETTY_FUNCTION__));
#endif
		}
#endif
		return code;
	}

	dgMinkReturnCode UpdateSeparatingPlaneSimd(dgMinkFace *&plane,
	        const dgVector &origin) {
#ifdef DG_BUILD_SIMD_CODE
		dgVector diff[4];
		dgVector aveg[4];

		plane = NULL;
		dgMinkFace *face = &m_simplex[0];
		dgMinkReturnCode code = dgMinkIntersecting;

		dgInt32 ciclingCount = -1;
		dgMinkFace *lastDescendFace = NULL;
		dgFloat32 minDist = dgFloat32(1.0e20f);

		dgInt32 j = 0;
		for (; face && (j < DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION); j++) {
			face = NULL;

			//          maxDist = dgFloat32 (0.0f);
			//          for (dgInt32 i = 0; i < 4; i ++) {
			//              dgInt32 i0 = m_faceIndex[i][0];
			//              dgInt32 i1 = m_faceIndex[i][1];
			//              dgInt32 i2 = m_faceIndex[i][2];
			//
			//              NEWTON_ASSERT (i0 == m_simplex[i].m_vertex[0]);
			//              NEWTON_ASSERT (i1 == m_simplex[i].m_vertex[1]);
			//              NEWTON_ASSERT (i2 == m_simplex[i].m_vertex[2]);
			//
			//              const dgVector& p0 = m_hullVertex[i0];
			//              const dgVector& p1 = m_hullVertex[i1];
			//              const dgVector& p2 = m_hullVertex[i2];
			//              dgVector e0 (p1 - p0);
			//              dgVector e1 (p2 - p0);
			//              dgVector n (e0 * e1);
			//
			//              dgFloat32 dist = n % n;
			//              if (dist > DG_DISTANCE_TOLERANCE_ZERO) {
			//                  n = n.Scale (dgRsqrt (dist));
			//                  dist = n % (origin - p0);
			//
			//                  // find the plane farther away from the origin
			//                  if (dist > maxDist) {
			//                      maxDist = dist;
			//                      normal = n;
			//                      face = &m_simplex[i];
			//                  }
			//              }
			//          }

			/*
			 simd_type tmp0 = simd_sub_v (((simd_type *)m_hullVertex)[1], ((simd_type *)m_hullVertex)[0]);
			 simd_type tmp1 = simd_sub_v (((simd_type *)m_hullVertex)[2], ((simd_type *)m_hullVertex)[0]);
			 simd_type tmp2 = simd_sub_v (((simd_type *)m_hullVertex)[3], ((simd_type *)m_hullVertex)[0]);
			 simd_type tmp4 = simd_sub_v (((simd_type *)m_hullVertex)[3], ((simd_type *)m_hullVertex)[2]);
			 simd_type tmp5 = simd_sub_v (((simd_type *)m_hullVertex)[2], ((simd_type *)m_hullVertex)[1]);

			 simd_type Ax = simd_permut_v (simd_permut_v (tmp0, tmp2, PURMUT_MASK (0, 0, 0, 0)),
			 simd_permut_v (tmp1, tmp4, PURMUT_MASK (0, 0, 0, 0)), PURMUT_MASK (2, 0, 2, 0));

			 simd_type Ay = simd_permut_v (simd_permut_v (tmp0, tmp2, PURMUT_MASK (1, 1, 1, 1)),
			 simd_permut_v (tmp1, tmp4, PURMUT_MASK (1, 1, 1, 1)), PURMUT_MASK (2, 0, 2, 0));

			 simd_type Az = simd_permut_v (simd_permut_v (tmp0, tmp2, PURMUT_MASK (2, 2, 2, 2)),
			 simd_permut_v (tmp1, tmp4, PURMUT_MASK (2, 2, 2, 2)), PURMUT_MASK (2, 0, 2, 0));


			 simd_type Bx = simd_permut_v (simd_permut_v (tmp1, tmp0, PURMUT_MASK (0, 0, 0, 0)),
			 simd_permut_v (tmp2, tmp5, PURMUT_MASK (0, 0, 0, 0)), PURMUT_MASK (2, 0, 2, 0));

			 simd_type By = simd_permut_v (simd_permut_v (tmp1, tmp0, PURMUT_MASK (1, 1, 1, 1)),
			 simd_permut_v (tmp2, tmp5, PURMUT_MASK (1, 1, 1, 1)), PURMUT_MASK (2, 0, 2, 0));

			 simd_type Bz = simd_permut_v (simd_permut_v (tmp1, tmp0, PURMUT_MASK (2, 2, 2, 2)),
			 simd_permut_v (tmp2, tmp5, PURMUT_MASK (2, 2, 2, 2)), PURMUT_MASK (2, 0, 2, 0));

			 simd_type nx = simd_mul_sub_v (simd_mul_v(Ay, Bz), Az, By);
			 simd_type ny = simd_mul_sub_v (simd_mul_v(Az, Bx), Ax, Bz);
			 simd_type nz = simd_mul_sub_v (simd_mul_v(Ax, By), Ay, Bx);
			 simd_type dist2 = simd_mul_add_v (simd_mul_add_v (simd_mul_v(nx, nx), ny, ny), nz, nz);
			 simd_type mask = simd_cmpgt_v (dist2, m_zeroTolerenace);
			 dist2 = simd_max_v(dist2, m_zeroTolerenace);
			 tmp0 = simd_rsqrt_v(dist2);
			 dist2 =  simd_mul_v (simd_mul_v(m_nrh0p5, tmp0), simd_mul_sub_v (m_nrh3p0, simd_mul_v (dist2, tmp0), tmp0));
			 nx = simd_mul_v (nx, dist2);
			 ny = simd_mul_v (ny, dist2);
			 nz = simd_mul_v (nz, dist2);

			 tmp4 = simd_sub_v (*((simd_type *)&origin), ((simd_type *)m_hullVertex)[0]);
			 tmp5 = simd_sub_v (*((simd_type *)&origin), ((simd_type *)m_hullVertex)[3]);
			 tmp0 = simd_permut_v (tmp4, tmp5, PURMUT_MASK (0, 0, 0, 0));
			 tmp1 = simd_permut_v (tmp4, tmp5, PURMUT_MASK (1, 1, 1, 1));
			 tmp2 = simd_permut_v (tmp4, tmp5, PURMUT_MASK (2, 2, 2, 2));
			 tmp0 = simd_permut_v (tmp0, tmp0, PURMUT_MASK (3, 0, 0, 0));
			 tmp1 = simd_permut_v (tmp1, tmp1, PURMUT_MASK (3, 0, 0, 0));
			 tmp2 = simd_permut_v (tmp2, tmp2, PURMUT_MASK (3, 0, 0, 0));

			 dist2 = simd_mul_add_v (simd_mul_add_v (simd_mul_v(nx, tmp0), ny, tmp1), nz, tmp2);
			 dist2 = simd_or_v (simd_and_v(dist2, mask), simd_andnot_v (m_negativeOne, mask));
			 tmp2 = simd_permut_v (dist2, dist2, PURMUT_MASK (3, 2, 3, 2));
			 mask = simd_cmpgt_v (dist2, tmp2);
			 dist2 = simd_or_v (simd_and_v(dist2, mask), simd_andnot_v (tmp2, mask));
			 tmp0 = simd_or_v (simd_and_v(m_index_yx, mask), simd_andnot_v (m_index_wz, mask));
			 tmp2 = simd_permut_v (dist2, dist2, PURMUT_MASK (3, 2, 1, 1));
			 mask = simd_cmpgt_v (dist2, tmp2);
			 dist2 = simd_or_v (simd_and_v(dist2, mask), simd_andnot_v (tmp2, mask));
			 tmp0 = simd_or_v (simd_and_v(tmp0, mask), simd_andnot_v (simd_permut_v (tmp0, tmp0, PURMUT_MASK (3, 2, 1, 1)), mask));
			 mask = simd_cmpgt_v (dist2, m_zero);
			 tmp0 = simd_or_v (simd_and_v(tmp0, mask), simd_andnot_v (m_negIndex, mask));
			 dgInt32 faceIndex = simd_store_is (tmp0);
			 */

			//          {0, 1, 2, 3},
			//          {1, 0, 3, 2},
			//          {0, 2, 3, 1},
			//          {2, 1, 3, 0},
			simd_type p0_ = ((simd_type *)m_hullVertex)[0];
			simd_type p1_ = ((simd_type *)m_hullVertex)[1];
			simd_type p2_ = ((simd_type *)m_hullVertex)[2];
			simd_type p3_ = ((simd_type *)m_hullVertex)[3];

			simd_type e10_ = simd_sub_v(p1_, p0_);
			simd_type e20_ = simd_sub_v(p2_, p0_);
			simd_type e30_ = simd_sub_v(p3_, p0_);
			simd_type e12_ = simd_sub_v(p1_, p2_);
			simd_type e32_ = simd_sub_v(p3_, p2_);

			simd_type tmp0_ = simd_pack_lo_v(e10_, e30_);
			simd_type tmp1_ = simd_pack_lo_v(e20_, e12_);
			simd_type e10_x = simd_move_lh_v(tmp0_, tmp1_);
			simd_type e10_y = simd_move_hl_v(tmp1_, tmp0_);
			tmp0_ = simd_pack_hi_v(e10_, e30_);
			tmp1_ = simd_pack_hi_v(e20_, e12_);
			simd_type e10_z = simd_move_lh_v(tmp0_, tmp1_);

			tmp0_ = simd_pack_lo_v(e20_, e10_);
			tmp1_ = simd_pack_lo_v(e30_, e32_);
			simd_type e20_x = simd_move_lh_v(tmp0_, tmp1_);
			simd_type e20_y = simd_move_hl_v(tmp1_, tmp0_);
			tmp0_ = simd_pack_hi_v(e20_, e10_);
			tmp1_ = simd_pack_hi_v(e30_, e32_);
			simd_type e20_z = simd_move_lh_v(tmp0_, tmp1_);

			simd_type nx_ = simd_mul_sub_v(simd_mul_v(e10_y, e20_z), e10_z, e20_y);
			simd_type ny_ = simd_mul_sub_v(simd_mul_v(e10_z, e20_x), e10_x, e20_z);
			simd_type nz_ = simd_mul_sub_v(simd_mul_v(e10_x, e20_y), e10_y, e20_x);

			simd_type dist2_ =
			    simd_mul_add_v(simd_mul_add_v(simd_mul_v(nx_, nx_), ny_, ny_), nz_, nz_);
			simd_type mask_ = simd_cmpgt_v(dist2_, m_zeroTolerenace);
			dist2_ = simd_max_v(dist2_, m_zeroTolerenace);
			tmp0_ = simd_rsqrt_v(dist2_);
			dist2_ =
			    simd_mul_v(simd_mul_v(m_nrh0p5, tmp0_), simd_mul_sub_v(m_nrh3p0, simd_mul_v(dist2_, tmp0_), tmp0_));

			nx_ = simd_mul_v(nx_, dist2_);
			ny_ = simd_mul_v(ny_, dist2_);
			nz_ = simd_mul_v(nz_, dist2_);

			simd_type origin_P0 = simd_sub_v(*((simd_type *)&origin), p0_);
			simd_type origin_P3 = simd_sub_v(*((simd_type *)&origin), p3_);
			simd_type origin_P0_xxxx =
			    simd_permut_v(origin_P0, origin_P3, PURMUT_MASK(0, 0, 0, 0));
			simd_type origin_P0_yyyy =
			    simd_permut_v(origin_P0, origin_P3, PURMUT_MASK(1, 1, 1, 1));
			simd_type origin_P0_zzzz =
			    simd_permut_v(origin_P0, origin_P3, PURMUT_MASK(2, 2, 2, 2));

			dist2_ =
			    simd_mul_add_v(simd_mul_add_v(simd_mul_v(nx_, origin_P0_xxxx), ny_, origin_P0_yyyy), nz_, origin_P0_zzzz);
			dist2_ =
			    simd_or_v(simd_and_v(dist2_, mask_), simd_andnot_v(m_negativeOne, mask_));

			tmp1_ = simd_permut_v(dist2_, dist2_, PURMUT_MASK(3, 2, 3, 2));
			mask_ = simd_cmpgt_v(dist2_, tmp1_);
			dist2_ =
			    simd_or_v(simd_and_v(dist2_, mask_), simd_andnot_v(tmp1_, mask_));
			tmp0_ =
			    simd_or_v(simd_and_v(m_index_yx, mask_), simd_andnot_v(m_index_wz, mask_));

			tmp1_ = simd_permut_v(dist2_, dist2_, PURMUT_MASK(3, 2, 1, 1));
			mask_ = simd_cmpgt_v(dist2_, tmp1_);
			dist2_ =
			    simd_or_v(simd_and_v(dist2_, mask_), simd_andnot_v(tmp1_, mask_));
			tmp0_ =
			    simd_or_v(simd_and_v(tmp0_, mask_), simd_andnot_v(simd_permut_v(tmp0_, tmp0_, PURMUT_MASK(3, 2, 1, 1)), mask_));

			mask_ = simd_cmpgt_v(dist2_, m_zero);
			tmp0_ =
			    simd_or_v(simd_and_v(tmp0_, mask_), simd_andnot_v(m_negIndex, mask_));
			dgInt32 faceIndex = simd_store_is(tmp0_);

			if (faceIndex >= 0) {
				dgVector transposedNormals[3];
				face = &m_simplex[faceIndex];
				simd_store_v(nx_, &transposedNormals[0][0]);
				simd_store_v(ny_, &transposedNormals[1][0]);
				simd_store_v(nz_, &transposedNormals[2][0]);
				dgVector normal(transposedNormals[0][faceIndex],
				                transposedNormals[1][faceIndex], transposedNormals[2][faceIndex],
				                dgFloat32(0.0f));

				// i0 = face->m_vertex;
				dgInt32 index = face->m_vertex[0];
				CalcSupportVertexSimd(normal, 4);
				dgFloat32 dist = normal % (m_hullVertex[4] - m_hullVertex[index]);

				// if we are doing too many passes it means that it is a skew shape
				// increasing the tolerance help to resolve the problem
				if (dist < DG_UPDATE_SEPARATING_PLANE_DISTANCE_TOLERANCE1) {
					plane = face;
					code = dgMinkDisjoint;
					break;
				}

				if (dist < minDist) {
					minDist = dist;
					lastDescendFace = face;
					ciclingCount = -1;
					for (dgInt32 k = 0; k < 4; k++) {
						diff[k] = m_hullVertex[k];
						aveg[k] = m_averVertex[k];
					}
				}

				ciclingCount++;
				if (ciclingCount > 4) {
					for (dgInt32 k = 0; k < 4; k++) {
						m_hullVertex[k] = diff[k];
						m_averVertex[k] = aveg[k];
					}
					code = dgMinkDisjoint;
					plane = lastDescendFace;
					break;
				}

				if (dist < DG_DISTANCE_TOLERANCE) {
					dgInt32 i = 0;
					for (; i < 4; i++) {
						dgVector error(m_hullVertex[i] - m_hullVertex[4]);
						if ((error % error) < (DG_DISTANCE_TOLERANCE * DG_DISTANCE_TOLERANCE)) {
							plane = face;
							// code = dgMinkDisjoint;
							code = UpdateSeparatingPlaneFallbackSolution(plane, origin);
							NEWTON_ASSERT(
							    (code == dgMinkDisjoint) || ((code == dgMinkIntersecting) && (m_vertexIndex == 4)));
							break;
						}
					}
					if (i < 4) {
						break;
					}
				}

				dgInt32 i0 = face->m_vertex[0];
				dgInt32 i1 = face->m_vertex[1];
				dgInt32 i2 = m_faceIndex[face - m_simplex][3];
				NEWTON_ASSERT(i2 != face->m_vertex[0]);
				NEWTON_ASSERT(i2 != face->m_vertex[1]);
				NEWTON_ASSERT(i2 != face->m_vertex[2]);
				Swap(m_hullVertex[i0], m_hullVertex[i1]);
				Swap(m_averVertex[i0], m_averVertex[i1]);
				m_hullVertex[i2] = m_hullVertex[4];
				m_averVertex[i2] = m_averVertex[4];
				if (!CheckTetraHedronVolume()) {
					Swap(m_hullVertex[1], m_hullVertex[2]);
					Swap(m_averVertex[1], m_averVertex[2]);
					NEWTON_ASSERT(CheckTetraHedronVolume());
				}
			}
		}

		if (j >= DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION) {
			NEWTON_ASSERT(CheckTetraHedronVolume());
			code = UpdateSeparatingPlaneFallbackSolution(plane, origin);
		}
		return code;

#else
		return dgMinkIntersecting;
#endif
	}

	dgMinkReturnCode UpdateSeparatingPlane(dgMinkFace *&plane,
	                                       const dgVector &origin) {
		dgVector diff[4];
		dgVector aveg[4];

		plane = NULL;
		dgMinkFace *face = &m_simplex[0];
		dgMinkFace *lastDescendFace = NULL;
		dgMinkReturnCode code = dgMinkIntersecting;

		// this loop can calculate the closest point to the origin usually in 4 to 5 passes,
		dgInt32 j = 0;
		dgInt32 ciclingCount = -1;
		dgFloat32 minDist = dgFloat32(1.0e20f);
		for (; face && (j < DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION); j++) {
			face = NULL;
			dgVector normal;
			// initialize distance to zero (very important)
			dgFloat32 maxDist = dgFloat32(0.0f);
			for (dgInt32 i = 0; i < 4; i++) {
				dgInt32 i0 = m_faceIndex[i][0];
				dgInt32 i1 = m_faceIndex[i][1];
				dgInt32 i2 = m_faceIndex[i][2];

				NEWTON_ASSERT(i0 == m_simplex[i].m_vertex[0]);
				NEWTON_ASSERT(i1 == m_simplex[i].m_vertex[1]);
				NEWTON_ASSERT(i2 == m_simplex[i].m_vertex[2]);

				const dgVector &p0 = m_hullVertex[i0];
				const dgVector &p1 = m_hullVertex[i1];
				const dgVector &p2 = m_hullVertex[i2];
				dgVector e0(p1 - p0);
				dgVector e1(p2 - p0);
				dgVector n(e0 * e1);

				dgFloat32 dist = n % n;
				if (dist > DG_DISTANCE_TOLERANCE_ZERO) {
					n = n.Scale(dgRsqrt(dist));
					dist = n % (origin - p0);

					// find the plane farther away from the origin
					if (dist > maxDist) {
						maxDist = dist;
						normal = n;
						face = &m_simplex[i];
					}
				}
			}

			// if we do not have a face at this point it means that the mink shape of the tow convexity face have a very
			// skew ratios on floating point accuracy is not enough to guarantee convexity of the shape
			if (face) {
				dgInt32 index = face->m_vertex[0];
				CalcSupportVertex(normal, 4);
				dgFloat32 dist = normal % (m_hullVertex[4] - m_hullVertex[index]);

				// if we are doing too many passes it means that it is a skew shape with big and small floats
				// significant bits may be lost in dist calculation, increasing the tolerance help to resolve the problem
				if (dist < DG_UPDATE_SEPARATING_PLANE_DISTANCE_TOLERANCE1) {
					plane = face;
					code = dgMinkDisjoint;
					break;
				}

				if (dist < minDist) {
					minDist = dist;
					lastDescendFace = face;
					ciclingCount = -1;
					for (dgInt32 k = 0; k < 4; k++) {
						diff[k] = m_hullVertex[k];
						aveg[k] = m_averVertex[k];
					}
				}

				ciclingCount++;
				if (ciclingCount > 4) {
					for (dgInt32 k = 0; k < 4; k++) {
						m_hullVertex[k] = diff[k];
						m_averVertex[k] = aveg[k];
					}
					code = dgMinkDisjoint;
					plane = lastDescendFace;
					break;
				}

				if (dist < DG_DISTANCE_TOLERANCE) {
					dgInt32 i = 0;
					for (; i < 4; i++) {
						dgVector error(m_hullVertex[i] - m_hullVertex[4]);
						if ((error % error) < (DG_DISTANCE_TOLERANCE * DG_DISTANCE_TOLERANCE)) {
							plane = face;
							// code = dgMinkDisjoint;
							code = UpdateSeparatingPlaneFallbackSolution(plane, origin);
							NEWTON_ASSERT(
							    (code == dgMinkDisjoint) || ((code == dgMinkIntersecting) && (m_vertexIndex == 4)));
							break;
						}
					}
					if (i < 4) {
						break;
					}
				}

				dgInt32 i0 = face->m_vertex[0];
				dgInt32 i1 = face->m_vertex[1];
				dgInt32 i2 = m_faceIndex[face - m_simplex][3];
				NEWTON_ASSERT(i2 != face->m_vertex[0]);
				NEWTON_ASSERT(i2 != face->m_vertex[1]);
				NEWTON_ASSERT(i2 != face->m_vertex[2]);
				Swap(m_hullVertex[i0], m_hullVertex[i1]);
				Swap(m_averVertex[i0], m_averVertex[i1]);
				m_hullVertex[i2] = m_hullVertex[4];
				m_averVertex[i2] = m_averVertex[4];
				if (!CheckTetraHedronVolume()) {
					Swap(m_hullVertex[1], m_hullVertex[2]);
					Swap(m_averVertex[1], m_averVertex[2]);
					NEWTON_ASSERT(CheckTetraHedronVolume());
				}
			}
		}

		if (j >= DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION) {
			NEWTON_ASSERT(CheckTetraHedronVolume());
			code = UpdateSeparatingPlaneFallbackSolution(plane, origin);
		}
		return code;
	}

	dgMinkReturnCode UpdateSeparatingPlaneLarge(dgMinkFace *&plane,
	        const dgBigVector &origin) {
		dgBigVector diff[4];
		dgBigVector aveg[4];

		dgBigVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                   dgFloat32(0.0f));

		plane = NULL;
		dgMinkFace *face = &m_simplex[0];
		dgMinkReturnCode code = dgMinkIntersecting;

		dgInt32 ciclingCount = -1;
		dgMinkFace *lastDescendFace = NULL;
		dgFloat64 minDist = dgFloat32(1.0e20f);

		// this loop can calculate the closest point to the origin usually in 4 to 5 passes,
		dgInt32 j = 0;
		for (; face && (j < DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION); j++) {
			face = NULL;
			// initialize distance to zero (very important)
			dgFloat64 maxDist = dgFloat32(0.0f);

			for (dgInt32 i = 0; i < 4; i++) {
				dgInt32 i0 = m_faceIndex[i][0];
				dgInt32 i1 = m_faceIndex[i][1];
				dgInt32 i2 = m_faceIndex[i][2];

				NEWTON_ASSERT(i0 == m_simplex[i].m_vertex[0]);
				NEWTON_ASSERT(i1 == m_simplex[i].m_vertex[1]);
				NEWTON_ASSERT(i2 == m_simplex[i].m_vertex[2]);

				const dgBigVector &p0 = m_hullVertexLarge[i0];
				const dgBigVector &p1 = m_hullVertexLarge[i1];
				const dgBigVector &p2 = m_hullVertexLarge[i2];

				dgBigVector e0(p1 - p0);
				dgBigVector e1(p2 - p0);
				dgBigVector n(e0 * e1);

				dgFloat64 dist = n % n;
				if (dist > DG_DISTANCE_TOLERANCE_ZERO) {
					n = n.Scale(dgFloat64(1.0f) / sqrt(dist));
					dist = n % (origin - p0);

					// find the plane farther away from the origin
					if (dist > maxDist) {
						maxDist = dist;
						normal = n;
						face = &m_simplex[i];
					}
				}
			}

			// if we do not have a face at this point it means that the mink shape of the tow convexity face have a very
			// skew ratios on floating point accuracy is not enough to guarantee convexity of the shape
			if (face) {
				dgInt32 index = face->m_vertex[0];
				dgVector dir(dgFloat32(normal.m_x), dgFloat32(normal.m_y),
				             dgFloat32(normal.m_z), 0.0f);
				CalcSupportVertexLarge(dir, 4);

				dgFloat64 dist = normal % (m_hullVertexLarge[4] - m_hullVertexLarge[index]);

				// if we are doing too many passes it means that it is a skew shape with big and small floats
				// significant bits may be lost in dist calculation, increasing the tolerance help to resolve the problem
				if (dist < DG_UPDATE_SEPARATING_PLANE_DISTANCE_TOLERANCE1) {
					plane = face;
					code = dgMinkDisjoint;
					break;
				}

				if (dist < minDist) {
					minDist = dist;
					lastDescendFace = face;
					ciclingCount = -1;
					for (dgInt32 k = 0; k < 4; k++) {
						diff[k] = m_hullVertexLarge[k];
						aveg[k] = m_averVertexLarge[k];
					}
				}

				ciclingCount++;
				if (ciclingCount > 4) {
					for (dgInt32 k = 0; k < 4; k++) {
						m_hullVertexLarge[k] = diff[k];
						m_averVertexLarge[k] = aveg[k];
					}
					code = dgMinkDisjoint;
					plane = lastDescendFace;
					break;
				}

				if (dist < DG_DISTANCE_TOLERANCE) {
					dgInt32 i = 0;
					for (; i < 4; i++) {
						dgBigVector error(m_hullVertexLarge[i] - m_hullVertexLarge[4]);
						if ((error % error) < (DG_DISTANCE_TOLERANCE * DG_DISTANCE_TOLERANCE)) {
							plane = face;
							// code = dgMinkDisjoint;
							code = UpdateSeparatingPlaneFallbackSolutionLarge(plane, origin);
							NEWTON_ASSERT(
							    (code == dgMinkDisjoint) || ((code == dgMinkIntersecting) && (m_vertexIndex == 4)));
							break;
						}
					}
					if (i < 4) {
						break;
					}
				}

				dgInt32 i0 = face->m_vertex[0];
				dgInt32 i1 = face->m_vertex[1];
				dgInt32 i2 = m_faceIndex[face - m_simplex][3];
				NEWTON_ASSERT(i2 != face->m_vertex[0]);
				NEWTON_ASSERT(i2 != face->m_vertex[1]);
				NEWTON_ASSERT(i2 != face->m_vertex[2]);
				Swap(m_hullVertexLarge[i0], m_hullVertexLarge[i1]);
				Swap(m_averVertexLarge[i0], m_averVertexLarge[i1]);
				m_hullVertexLarge[i2] = m_hullVertexLarge[4];
				m_averVertexLarge[i2] = m_averVertexLarge[4];
				if (!CheckTetraHedronVolumeLarge()) {
					Swap(m_hullVertexLarge[1], m_hullVertexLarge[2]);
					Swap(m_averVertexLarge[1], m_averVertexLarge[2]);
					NEWTON_ASSERT(CheckTetraHedronVolumeLarge());
				}
			}
		}

		if (j >= DG_UPDATE_SEPARATING_PLANE_MAX_ITERATION) {
			NEWTON_ASSERT(CheckTetraHedronVolumeLarge());
			code = UpdateSeparatingPlaneFallbackSolutionLarge(plane, origin);
		}
		return code;
	}

	dgMinkReturnCode CalcSeparatingPlaneSimd(
	    dgMinkFace *&plane,
	    const dgVector &origin = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
	                                      dgFloat32(0.0f), dgFloat32(1.0f))) {
#ifdef DG_BUILD_SIMD_CODE

		dgInt32 best;
		dgFloat32 maxErr;
		dgFloat32 error2;
		dgVector e1;
		dgVector e2;
		dgVector e3;
		dgVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                dgFloat32(0.0f));

		CalcSupportVertexSimd(m_dir[0], 0);

		dgInt32 i = 1;
		for (; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertexSimd(m_dir[i], 1);
			e1 = m_hullVertex[1] - m_hullVertex[0];
			error2 = e1 % e1;
			if (error2 > DG_CALCULATE_SEPARATING_PLANE_ERROR) {
				break;
			}
		}

		for (i++; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertexSimd(m_dir[i], 2);
			e2 = m_hullVertex[2] - m_hullVertex[0];
			normal = e1 * e2;
			error2 = normal % normal;
			if (error2 > DG_CALCULATE_SEPARATING_PLANE_ERROR1) {
				break;
			}
		}

		error2 = dgFloat32(0.0f);
		for (i++; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertexSimd(m_dir[i], 3);

			e3 = m_hullVertex[3] - m_hullVertex[0];
			error2 = normal % e3;
			if (dgAbsf(error2) > DG_CALCULATE_SEPARATING_PLANE_ERROR1) {
				break;
			}
		}

		if (i >= dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertexSimd(m_dir[i], 1);
				e1 = m_hullVertex[1] - m_hullVertex[0];
				error2 = e1 % e1;
				if (error2 > maxErr) {
					best = i;
					maxErr = error2;
				}
			}
			CalcSupportVertexSimd(m_dir[best], 1);
			e1 = m_hullVertex[1] - m_hullVertex[0];

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertexSimd(m_dir[i], 2);
				e2 = m_hullVertex[2] - m_hullVertex[0];
				normal = e1 * e2;
				error2 = normal % normal;
				if (error2 > maxErr) {
					best = i;
					maxErr = error2;
				}
			}

			CalcSupportVertexSimd(m_dir[best], 2);
			e2 = m_hullVertex[2] - m_hullVertex[0];
			normal = e1 * e2;

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertexSimd(m_dir[i], 3);

				e3 = m_hullVertex[3] - m_hullVertex[0];
				error2 = normal % e3;
				if (dgAbsf(error2) > dgAbsf(maxErr)) {
					best = i;
					maxErr = error2;
				}
			}
			error2 = maxErr;
			CalcSupportVertexSimd(m_dir[best], 3);
		}

		m_vertexIndex = 4;
		if (error2 > dgFloat32(0.0f)) {
			Swap(m_hullVertex[1], m_hullVertex[2]);
			Swap(m_averVertex[1], m_averVertex[2]);
		}

		NEWTON_ASSERT(CheckTetraHedronVolume());

		NEWTON_ASSERT((((dgUnsigned64)&m_simplex[0]) & 0x0f) == 0);
		NEWTON_ASSERT((((dgUnsigned64)&m_simplex[1]) & 0x0f) == 0);

		// face 0
		m_simplex[0].m_vertex[0] = 0;
		m_simplex[0].m_vertex[1] = 1;
		m_simplex[0].m_vertex[2] = 2;
		m_simplex[0].m_vertex[3] = 0;
		m_simplex[0].m_adjancentFace[0] = 1;
		m_simplex[0].m_adjancentFace[1] = 3;
		m_simplex[0].m_adjancentFace[2] = 2;

		// face 1
		m_simplex[1].m_vertex[0] = 1;
		m_simplex[1].m_vertex[1] = 0;
		m_simplex[1].m_vertex[2] = 3;
		m_simplex[1].m_vertex[3] = 1;
		m_simplex[1].m_adjancentFace[0] = 0;
		m_simplex[1].m_adjancentFace[1] = 2;
		m_simplex[1].m_adjancentFace[2] = 3;

		// face 2
		m_simplex[2].m_vertex[0] = 0;
		m_simplex[2].m_vertex[1] = 2;
		m_simplex[2].m_vertex[2] = 3;
		m_simplex[2].m_vertex[3] = 0;
		m_simplex[2].m_adjancentFace[0] = 0;
		m_simplex[2].m_adjancentFace[1] = 3;
		m_simplex[2].m_adjancentFace[2] = 1;

		// face 3
		m_simplex[3].m_vertex[0] = 2;
		m_simplex[3].m_vertex[1] = 1;
		m_simplex[3].m_vertex[2] = 3;
		m_simplex[3].m_vertex[3] = 2;
		m_simplex[3].m_adjancentFace[0] = 0;
		m_simplex[3].m_adjancentFace[1] = 1;
		m_simplex[3].m_adjancentFace[2] = 2;

		return UpdateSeparatingPlaneSimd(plane, origin);

#else
		return dgMinkIntersecting;
#endif
	}

	dgMinkReturnCode CalcSeparatingPlane(
	    dgMinkFace *&plane,
	    const dgVector &origin = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
	                                      dgFloat32(0.0f), dgFloat32(1.0f))) {
		dgInt32 best;
		dgFloat32 maxErr;
		dgFloat32 error2;
		dgVector e1;
		dgVector e2;
		dgVector e3;
		dgVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                dgFloat32(0.0f));

		CalcSupportVertex(m_dir[0], 0);
		dgInt32 i = 1;
		for (; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertex(m_dir[i], 1);
			e1 = m_hullVertex[1] - m_hullVertex[0];
			error2 = e1 % e1;
			if (error2 > DG_CALCULATE_SEPARATING_PLANE_ERROR) {
				break;
			}
		}

		for (i++; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertex(m_dir[i], 2);
			e2 = m_hullVertex[2] - m_hullVertex[0];
			normal = e1 * e2;
			error2 = normal % normal;
			if (error2 > DG_CALCULATE_SEPARATING_PLANE_ERROR1) {
				break;
			}
		}

		error2 = dgFloat32(0.0f);
		for (i++; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertex(m_dir[i], 3);
			e3 = m_hullVertex[3] - m_hullVertex[0];
			error2 = normal % e3;
			if (dgAbsf(error2) > DG_CALCULATE_SEPARATING_PLANE_ERROR1) {
				break;
			}
		}

		if (i >= dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertex(m_dir[i], 1);
				e1 = m_hullVertex[1] - m_hullVertex[0];
				error2 = e1 % e1;
				if (error2 > maxErr) {
					best = i;
					maxErr = error2;
				}
			}
			CalcSupportVertex(m_dir[best], 1);
			e1 = m_hullVertex[1] - m_hullVertex[0];

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertex(m_dir[i], 2);
				e2 = m_hullVertex[2] - m_hullVertex[0];
				normal = e1 * e2;
				error2 = normal % normal;
				if (error2 > maxErr) {
					best = i;
					maxErr = error2;
				}
			}

			CalcSupportVertex(m_dir[best], 2);
			e2 = m_hullVertex[2] - m_hullVertex[0];
			normal = e1 * e2;

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertex(m_dir[i], 3);

				e3 = m_hullVertex[3] - m_hullVertex[0];
				error2 = normal % e3;
				if (dgAbsf(error2) > dgAbsf(maxErr)) {
					best = i;
					maxErr = error2;
				}
			}
			error2 = maxErr;
			CalcSupportVertex(m_dir[best], 3);
		}

		m_vertexIndex = 4;
		if (error2 > dgFloat32(0.0f)) {
			Swap(m_hullVertex[1], m_hullVertex[2]);
			Swap(m_averVertex[1], m_averVertex[2]);
		}
		NEWTON_ASSERT(CheckTetraHedronVolume());

		NEWTON_ASSERT((((dgUnsigned64)&m_simplex[0]) & 0x0f) == 0);
		NEWTON_ASSERT((((dgUnsigned64)&m_simplex[1]) & 0x0f) == 0);

		// face 0
		m_simplex[0].m_vertex[0] = 0;
		m_simplex[0].m_vertex[1] = 1;
		m_simplex[0].m_vertex[2] = 2;
		m_simplex[0].m_vertex[3] = 0;
		m_simplex[0].m_adjancentFace[0] = 1;
		m_simplex[0].m_adjancentFace[1] = 3;
		m_simplex[0].m_adjancentFace[2] = 2;

		// face 1
		m_simplex[1].m_vertex[0] = 1;
		m_simplex[1].m_vertex[1] = 0;
		m_simplex[1].m_vertex[2] = 3;
		m_simplex[1].m_vertex[3] = 1;
		m_simplex[1].m_adjancentFace[0] = 0;
		m_simplex[1].m_adjancentFace[1] = 2;
		m_simplex[1].m_adjancentFace[2] = 3;

		// face 2
		m_simplex[2].m_vertex[0] = 0;
		m_simplex[2].m_vertex[1] = 2;
		m_simplex[2].m_vertex[2] = 3;
		m_simplex[2].m_vertex[3] = 0;
		m_simplex[2].m_adjancentFace[0] = 0;
		m_simplex[2].m_adjancentFace[1] = 3;
		m_simplex[2].m_adjancentFace[2] = 1;

		// face 3
		m_simplex[3].m_vertex[0] = 2;
		m_simplex[3].m_vertex[1] = 1;
		m_simplex[3].m_vertex[2] = 3;
		m_simplex[3].m_vertex[3] = 2;
		m_simplex[3].m_adjancentFace[0] = 0;
		m_simplex[3].m_adjancentFace[1] = 1;
		m_simplex[3].m_adjancentFace[2] = 2;

		return UpdateSeparatingPlane(plane, origin);
	}

	dgMinkReturnCode CalcSeparatingPlaneLarge(
	    dgMinkFace *&plane,
	    const dgBigVector &origin = dgBigVector(dgFloat32(0.0f), dgFloat32(0.0f),
	                                dgFloat32(0.0f), dgFloat32(1.0f))) {
		dgFloat64 error2;
		dgBigVector e1;
		dgBigVector e2;
		dgBigVector e3;
		dgBigVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                   dgFloat32(0.0f));

		CalcSupportVertexLarge(m_dir[0], 0);
		dgInt32 i = 1;
		for (; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			dgFloat64 error3;
			CalcSupportVertexLarge(m_dir[i], 1);
			e1 = m_hullVertexLarge[1] - m_hullVertexLarge[0];
			error3 = e1 % e1;
			if (error3 > DG_CALCULATE_SEPARATING_PLANE_ERROR) {
				break;
			}
		}

		for (i++; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			dgFloat64 error3;
			CalcSupportVertexLarge(m_dir[i], 2);
			e2 = m_hullVertexLarge[2] - m_hullVertexLarge[0];
			normal = e1 * e2;
			error3 = normal % normal;
			if (error3 > DG_CALCULATE_SEPARATING_PLANE_ERROR1) {
				break;
			}
		}

		error2 = dgFloat32(0.0f);
		for (i++; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
			CalcSupportVertexLarge(m_dir[i], 3);
			e3 = m_hullVertexLarge[3] - m_hullVertexLarge[0];
			error2 = normal % e3;
			if (fabs(error2) > DG_CALCULATE_SEPARATING_PLANE_ERROR1) {
				break;
			}
		}

		if (i >= dgInt32(sizeof(m_dir) / sizeof(m_dir[0]))) {
			dgInt32 best;
			dgFloat64 maxErr;

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertexLarge(m_dir[i], 1);
				e1 = m_hullVertexLarge[1] - m_hullVertexLarge[0];
				error2 = e1 % e1;
				if (error2 > maxErr) {
					best = i;
					maxErr = error2;
				}
			}
			CalcSupportVertexLarge(m_dir[best], 1);
			e1 = m_hullVertexLarge[1] - m_hullVertexLarge[0];

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertexLarge(m_dir[i], 2);
				dgBigVector e4(m_hullVertexLarge[2] - m_hullVertexLarge[0]);
				normal = e1 * e4;
				error2 = normal % normal;
				if (error2 > maxErr) {
					best = i;
					maxErr = error2;
				}
			}

			CalcSupportVertexLarge(m_dir[best], 2);
			dgBigVector e4(m_hullVertexLarge[2] - m_hullVertexLarge[0]);
			normal = e1 * e4;

			best = 0;
			maxErr = dgFloat32(0.0f);
			for (i = 1; i < dgInt32(sizeof(m_dir) / sizeof(m_dir[0])); i++) {
				CalcSupportVertexLarge(m_dir[i], 3);

				dgBigVector e5(m_hullVertexLarge[3] - m_hullVertexLarge[0]);
				error2 = normal % e5;
				if (fabs(error2) > fabs(maxErr)) {
					best = i;
					maxErr = error2;
				}
			}
			error2 = maxErr;
			CalcSupportVertexLarge(m_dir[best], 3);
		}

		m_vertexIndex = 4;
		if (error2 > dgFloat32(0.0f)) {
			Swap(m_hullVertexLarge[1], m_hullVertexLarge[2]);
			Swap(m_averVertexLarge[1], m_averVertexLarge[2]);
		}
		NEWTON_ASSERT(CheckTetraHedronVolumeLarge());

		NEWTON_ASSERT((((dgUnsigned64)&m_simplex[0]) & 0x0f) == 0);
		NEWTON_ASSERT((((dgUnsigned64)&m_simplex[1]) & 0x0f) == 0);

		// face 0
		m_simplex[0].m_vertex[0] = 0;
		m_simplex[0].m_vertex[1] = 1;
		m_simplex[0].m_vertex[2] = 2;
		m_simplex[0].m_vertex[3] = 0;
		m_simplex[0].m_adjancentFace[0] = 1;
		m_simplex[0].m_adjancentFace[1] = 3;
		m_simplex[0].m_adjancentFace[2] = 2;

		// face 1
		m_simplex[1].m_vertex[0] = 1;
		m_simplex[1].m_vertex[1] = 0;
		m_simplex[1].m_vertex[2] = 3;
		m_simplex[1].m_vertex[3] = 1;
		m_simplex[1].m_adjancentFace[0] = 0;
		m_simplex[1].m_adjancentFace[1] = 2;
		m_simplex[1].m_adjancentFace[2] = 3;

		// face 2
		m_simplex[2].m_vertex[0] = 0;
		m_simplex[2].m_vertex[1] = 2;
		m_simplex[2].m_vertex[2] = 3;
		m_simplex[2].m_vertex[3] = 0;
		m_simplex[2].m_adjancentFace[0] = 0;
		m_simplex[2].m_adjancentFace[1] = 3;
		m_simplex[2].m_adjancentFace[2] = 1;

		// face 3
		m_simplex[3].m_vertex[0] = 2;
		m_simplex[3].m_vertex[1] = 1;
		m_simplex[3].m_vertex[2] = 3;
		m_simplex[3].m_vertex[3] = 2;
		m_simplex[3].m_adjancentFace[0] = 0;
		m_simplex[3].m_adjancentFace[1] = 1;
		m_simplex[3].m_adjancentFace[2] = 2;

		return UpdateSeparatingPlaneLarge(plane, origin);
	}

	/*
	 inline bool CalcFacePlaneSimd (dgMinkFace *face)
	 {
	 #ifdef DG_BUILD_SIMD_CODE
	 dgInt32 i0;
	 dgInt32 i1;
	 dgInt32 i2;
	 simd_type e0;
	 simd_type e1;
	 simd_type n;
	 simd_type w;
	 simd_type dist2;
	 simd_type mask;
	 simd_type tmp0;

	 i0 = face->m_vertex[0];
	 i1 = face->m_vertex[1];
	 i2 = face->m_vertex[2];
	 dgPlane &plane = *face;

	 e0 = simd_sub_v ((*(simd_type*)&m_hullVertex[i1]), (*(simd_type*)&m_hullVertex[i0]));
	 e1 = simd_sub_v ((*(simd_type*)&m_hullVertex[i2]), (*(simd_type*)&m_hullVertex[i0]));
	 n = simd_mul_sub_v (simd_mul_v (simd_permut_v (e0, e0, PURMUT_MASK (3,0,2,1)), simd_permut_v (e1, e1, PURMUT_MASK (3,1,0,2))),
	 simd_permut_v (e0, e0, PURMUT_MASK (3,1,0,2)), simd_permut_v (e1, e1, PURMUT_MASK (3,0,2,1)));
	 e0 = simd_mul_v (n, n);
	 dist2 = simd_add_s(simd_add_v (e0, simd_move_hl_v (e0, e0)), simd_permut_v (e0, e0, PURMUT_MASK (3,3,3,1)));

	 mask = simd_cmpgt_s (dist2, m_zeroTolerenace);

	 e0 = simd_mul_v ((*(simd_type*)&m_hullVertex[i0]), n);
	 w = simd_and_v (mask, simd_mul_s (m_negativeOne, simd_add_s(simd_add_v (e0, simd_move_hl_v (e0, e0)), simd_permut_v (e0, e0, PURMUT_MASK (3,3,3,1)))));

	 tmp0 = simd_rsqrt_s(simd_max_s(dist2, m_zeroTolerenace));
	 dist2 =  simd_mul_s (simd_mul_s(m_nrh0p5, tmp0), simd_mul_sub_s (m_nrh3p0, simd_mul_s (dist2, tmp0), tmp0));

	 i0 = simd_store_is (simd_and_v(mask, m_negativeOne));

	 n = simd_permut_v (n, simd_permut_v (n, w, PURMUT_MASK (0,0,1,2)), PURMUT_MASK (2,0,1,0) );
	 simd_store_v(simd_mul_v (n, simd_permut_v (dist2, dist2, PURMUT_MASK (0,0,0,0))), (float*)&plane.m_x);

	 face->m_isActive = 1;
	 return i0 ? true : false;


	 #else
	 return false;
	 #endif
	 }
	 */
	inline bool CalcFacePlaneSimd(dgMinkFace *face) {
		return CalcFacePlane(face);
	}

	inline bool CalcFacePlane(dgMinkFace *const face) {
		dgInt32 i0 = face->m_vertex[0];
		dgInt32 i1 = face->m_vertex[1];
		dgInt32 i2 = face->m_vertex[2];

		dgPlane &plane = *face;
		plane = dgPlane(m_hullVertex[i0], m_hullVertex[i1], m_hullVertex[i2]);

		bool ret = false;
		dgFloat32 mag2 = plane % plane;

		//      if (mag2 > DG_DISTANCE_TOLERANCE_ZERO) {
		if (mag2 > dgFloat32(1.0e-12f)) {
			ret = true;
			plane = plane.Scale(dgRsqrt(mag2));
		} else {
			//NEWTON_ASSERT (0);
			plane.m_w = dgFloat32(0.0f);
		}

		face->m_isActive = 1;
		return ret;
	}

	inline bool CalcFacePlaneLarge(dgMinkFace *const face) {
		dgInt32 i0 = face->m_vertex[0];
		dgInt32 i1 = face->m_vertex[1];
		dgInt32 i2 = face->m_vertex[2];

		//      dgBigPlane &plane = *face;
		dgBigPlane plane(m_hullVertexLarge[i0], m_hullVertexLarge[i1],
		                 m_hullVertexLarge[i2]);

		bool ret = false;
		dgFloat64 mag2 = plane % plane;
		//      if (mag2 > DG_DISTANCE_TOLERANCE_ZERO) {
		if (mag2 > dgFloat32(1.0e-12f)) {
			ret = true;
			plane = plane.Scale(dgFloat64(1.0f) / sqrt(mag2));
		} else {
			//NEWTON_ASSERT (0);
			plane.m_w = dgFloat64(0.0f);
		}

		face->m_x = dgFloat32(plane.m_x);
		face->m_y = dgFloat32(plane.m_y);
		face->m_z = dgFloat32(plane.m_z);
		face->m_w = dgFloat32(plane.m_w);
		face->m_isActive = 1;
		return ret;
	}

	inline dgMinkFace *NewFace() {
		dgMinkFace *face;
		if (m_facePurge) {
			face = (dgMinkFace *)m_facePurge;
			m_facePurge = m_facePurge->m_next;
		} else {
			face = &m_simplex[m_planeIndex];
			m_planeIndex++;
			NEWTON_ASSERT(m_planeIndex < dgInt32(sizeof(m_simplex) / sizeof(m_simplex[0])));
		}
		return face;
	}

	struct SilhouetteFaceCap {
		dgMinkFace *m_face;
		dgInt16 *m_faceCopling;
	};

	dgMinkFace *CalculateClipPlaneSimd() {
#ifdef DG_BUILD_SIMD_CODE
		dgInt32 i;
		dgInt32 i0;
		dgInt32 i1;
		dgInt32 i2;
		dgInt32 stack;
		dgInt32 cicling;
		dgInt32 deadCount;
		dgInt32 adjacentIndex;
		dgInt32 prevEdgeIndex;
		dgInt32 silhouetteCapCount;
		dgInt32 lastSilhouetteVertex;

		dgFloat32 dist;
		dgFloat32 minValue;
		dgFloat32 penetration;
		dgFloat32 ciclingMem[4];
		dgMinkFace *face;
		dgMinkFace *adjacent;
		dgMinkFace *prevFace;
		dgMinkFace *firstFace;
		dgMinkFace *silhouette;
		dgMinkFace *lastSilhouette;
		dgMinkFace *closestFace;
		dgMinkFacePurge *nextPurge;
		dgMinkFace *stackPool[128];
		dgMinkFace *deadFaces[128];
		SilhouetteFaceCap sillueteCap[128];
		dgVector diff[3];
		dgVector aver[3];
		dgInt8 buffer[DG_HEAP_EDGE_COUNT * (sizeof(dgFloat32) + sizeof(dgMinkFace *))];
		dgClosestFace heapSort(buffer, sizeof(buffer));

		m_planeIndex = 4;
		closestFace = NULL;
		m_facePurge = NULL;
		penetration = dgFloat32(0.0f);

		NEWTON_ASSERT(m_vertexIndex == 4);
		for (i = 0; i < 4; i++) {
			face = &m_simplex[i];
			face->m_inHeap = 0;
			face->m_isActive = 1;
			if (CalcFacePlaneSimd(face)) {
				face->m_inHeap = 1;
				heapSort.Push(face, face->m_w);
			}
		}

		cicling = 0;
		ciclingMem[0] = dgFloat32(1.0e10f);
		ciclingMem[1] = dgFloat32(1.0e10f);
		ciclingMem[2] = dgFloat32(1.0e10f);
		ciclingMem[3] = dgFloat32(1.0e10f);

		minValue = dgFloat32(1.0e10f);
		dgPlane bestPlane(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                  dgFloat32(0.0f));
		diff[0] = bestPlane;
		diff[1] = bestPlane;
		diff[2] = bestPlane;
		aver[0] = bestPlane;
		aver[1] = bestPlane;
		aver[2] = bestPlane;

		while (heapSort.GetCount()) {
			face = heapSort[0];
			face->m_inHeap = 0;
			heapSort.Pop();

			if (face->m_isActive) {
				const dgPlane &plane = *face;

				CalcSupportVertexSimd(plane, m_vertexIndex);
				const dgVector &p = m_hullVertex[m_vertexIndex];
				dist = plane.Evalue(p);
				m_vertexIndex++;

				if (m_vertexIndex > 16) {
					if (dist < minValue) {
						if (dist >= dgFloat32(0.0f)) {
							minValue = dist;
							bestPlane = plane;

							i = face->m_vertex[0];
							diff[0] = m_hullVertex[i];
							aver[0] = m_averVertex[i];

							i = face->m_vertex[1];
							diff[1] = m_hullVertex[i];
							aver[1] = m_averVertex[i];

							i = face->m_vertex[2];
							diff[2] = m_hullVertex[i];
							aver[2] = m_averVertex[i];
						}
					}
				}

				ciclingMem[cicling] = dist;
				cicling = (cicling + 1) & 3;
				for (i = 0; i < 4; i++) {
					if (dgAbsf(dist - ciclingMem[i]) > dgFloat32(1.0e-6f)) {
						break;
					}
				}
				if (i == 4) {
					dist = dgFloat32(0.0f);
				}

				if ((m_vertexIndex > DG_MINK_MAX_POINTS) || (m_planeIndex > DG_MINK_MAX_FACES) || (heapSort.GetCount() > (DG_HEAP_EDGE_COUNT - 24))) {

					//                      dgTrace (("Max face count overflow, breaking with last best face\n"));

					dgPlane &plane = *face;
					plane = bestPlane;

					i = face->m_vertex[0];
					face->m_vertex[0] = 0;
					m_hullVertex[i] = diff[0];
					m_averVertex[i] = aver[0];

					i = face->m_vertex[1];
					face->m_vertex[1] = 1;
					m_hullVertex[i] = diff[1];
					m_averVertex[i] = aver[1];

					i = face->m_vertex[2];
					face->m_vertex[2] = 2;
					m_hullVertex[i] = diff[2];
					m_averVertex[i] = aver[2];
					dist = dgFloat32(0.0f);
				}

				if (dist < (dgFloat32(DG_IMPULSIVE_CONTACT_PENETRATION) / dgFloat32(16.0f))) {
					NEWTON_ASSERT(m_planeIndex <= DG_MINK_MAX_FACES_SIZE);
					NEWTON_ASSERT(heapSort.GetCount() <= DG_HEAP_EDGE_COUNT);
					closestFace = face;
					break;
				} else if (dist > dgFloat32(0.0f)) {
					NEWTON_ASSERT(face->m_inHeap == 0);

					stack = 0;
					deadCount = 1;
					silhouette = NULL;
					deadFaces[0] = face;
					closestFace = face;
					face->m_isActive = 0;
					for (i = 0; i < 3; i++) {
						adjacent = &m_simplex[face->m_adjancentFace[i]];
						NEWTON_ASSERT(adjacent->m_isActive);
						dist = adjacent->Evalue(p);
						if (dist > dgFloat32(0.0f)) {
							adjacent->m_isActive = 0;
							stackPool[stack] = adjacent;
							deadFaces[deadCount] = adjacent;
							stack++;
							deadCount++;
						} else {
							silhouette = adjacent;
						}
					}
					while (stack) {
						stack--;
						face = stackPool[stack];
						for (i = 0; i < 3; i++) {
							adjacent = &m_simplex[face->m_adjancentFace[i]];
							if (adjacent->m_isActive) {
								dist = adjacent->Evalue(p);
								if (dist > dgFloat32(0.0f)) {
									adjacent->m_isActive = 0;
									stackPool[stack] = adjacent;
									deadFaces[deadCount] = adjacent;
									stack++;
									deadCount++;
									NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(stackPool[0])));
									NEWTON_ASSERT(deadCount < dgInt32(sizeof(deadFaces) / sizeof(deadFaces[0])));

								} else {
									silhouette = adjacent;
								}
							}
						}
					}

					if (!silhouette) {
						closestFace = face;
						break;
					}

					// build silhouette
					NEWTON_ASSERT(silhouette);
					NEWTON_ASSERT(silhouette->m_isActive);

					i2 = (m_vertexIndex - 1);
					lastSilhouette = silhouette;
					//                  NEWTON_ASSERT ( (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
					//                      (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
					//                      (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 1) ||
					//                      (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
					//                      (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
					//                      (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 2));
					NEWTON_ASSERT(
					    (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[1]) && (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[2]) && (silhouette->m_adjancentFace[1] != silhouette->m_adjancentFace[2]));

					adjacentIndex = DG_GETADJACENTINDEX_ACTIVE(silhouette);
					face = NewFace();
					i0 = silhouette->m_vertex[adjacentIndex];
					i1 = silhouette->m_vertex[adjacentIndex + 1];

					face->m_vertex[0] = dgInt16(i1);
					face->m_vertex[1] = dgInt16(i0);
					face->m_vertex[2] = dgInt16(i2);
					face->m_vertex[3] = face->m_vertex[0];
					face->m_adjancentFace[0] = dgInt16(silhouette - m_simplex);
					face->m_inHeap = 0;
					face->m_isActive = 1;

					sillueteCap[0].m_face = face;
					sillueteCap[0].m_faceCopling =
					    &silhouette->m_adjancentFace[adjacentIndex];
					silhouetteCapCount = 1;
					NEWTON_ASSERT(
					    silhouetteCapCount < dgInt32(sizeof(sillueteCap) / sizeof(sillueteCap[0])));
					do {
						silhouette = &m_simplex[silhouette->m_adjancentFace[adjacentIndex]];
						adjacentIndex = (DG_GETADJACENTINDEX_VERTEX(silhouette, i0));
					} while (!silhouette->m_isActive);

					prevFace = face;
					firstFace = face;
					lastSilhouetteVertex = i0;
					prevEdgeIndex = dgInt32(face - m_simplex);
					do {
						//                      NEWTON_ASSERT ( (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
						//                          (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
						//                          (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 1) ||
						//                          (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
						//                          (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
						//                          (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 2));
						NEWTON_ASSERT((silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[1]) && (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[2]) && (silhouette->m_adjancentFace[1] != silhouette->m_adjancentFace[2]));

						adjacentIndex = adjacentIndex ? adjacentIndex - 1 : 2;

						face = NewFace();
						i0 = silhouette->m_vertex[adjacentIndex];
						i1 = silhouette->m_vertex[adjacentIndex + 1];

						face->m_vertex[0] = dgInt16(i1);
						face->m_vertex[1] = dgInt16(i0);
						face->m_vertex[2] = dgInt16(i2);
						face->m_vertex[3] = face->m_vertex[0];
						face->m_adjancentFace[0] = dgInt16(silhouette - m_simplex);
						face->m_adjancentFace[2] = dgInt16(prevEdgeIndex);
						face->m_inHeap = 0;
						face->m_isActive = 1;

						prevEdgeIndex = dgInt32(face - m_simplex);
						prevFace->m_adjancentFace[1] = dgInt16(prevEdgeIndex);
						prevFace = face;

						sillueteCap[silhouetteCapCount].m_face = face;
						sillueteCap[silhouetteCapCount].m_faceCopling =
						    &silhouette->m_adjancentFace[adjacentIndex];
						silhouetteCapCount++;
						NEWTON_ASSERT(
						    silhouetteCapCount < dgInt32(sizeof(sillueteCap) / sizeof(sillueteCap[0])));

						do {
							silhouette =
							    &m_simplex[silhouette->m_adjancentFace[adjacentIndex]];
							adjacentIndex = (DG_GETADJACENTINDEX_VERTEX(silhouette, i0));
						} while (!silhouette->m_isActive);

					} while ((silhouette != lastSilhouette) || (silhouette->m_vertex[adjacentIndex ? adjacentIndex - 1 : 2] != lastSilhouetteVertex));
					firstFace->m_adjancentFace[2] = dgInt16(prevEdgeIndex);
					prevFace->m_adjancentFace[1] = dgInt16(firstFace - m_simplex);

					for (i = 0; i < deadCount; i++) {
						if (!deadFaces[i]->m_inHeap) {
							nextPurge = (dgMinkFacePurge *)deadFaces[i];
							nextPurge->m_next = m_facePurge;
							m_facePurge = nextPurge;
						}
					}

					while (heapSort.GetCount() && (!heapSort[0]->m_isActive)) {
						face = heapSort[0];
						heapSort.Pop();
						nextPurge = (dgMinkFacePurge *)face;
						nextPurge->m_next = m_facePurge;
						m_facePurge = nextPurge;
					}

					for (i = 0; i < silhouetteCapCount; i++) {
						face = sillueteCap[i].m_face;
						*sillueteCap[i].m_faceCopling = dgInt16(face - m_simplex);

						if (CalcFacePlaneSimd(face)) {
							face->m_inHeap = 1;
							heapSort.Push(face, face->m_w);
						}
					}
				}
			} else {
				NEWTON_ASSERT(0);
				nextPurge = (dgMinkFacePurge *)face;
				nextPurge->m_next = m_facePurge;
				m_facePurge = nextPurge;
			}
		}
		return closestFace;
#else
		return NULL;
#endif
	}

	dgMinkFace *CalculateClipPlane() {
		dgInt32 i;
		dgInt32 i0;
		dgInt32 i1;
		dgInt32 i2;
		dgInt32 stack;
		dgInt32 cicling;
		dgInt32 deadCount;
		dgInt32 adjacentIndex;
		dgInt32 prevEdgeIndex;
		dgInt32 silhouetteCapCount;
		dgInt32 lastSilhouetteVertex;

		dgFloat32 dist;
		dgFloat32 minValue;
		dgFloat32 ciclingMem[4];
		dgMinkFace *face;
		dgMinkFace *adjacent;
		dgMinkFace *prevFace;
		dgMinkFace *firstFace;
		dgMinkFace *silhouette;
		dgMinkFace *lastSilhouette;
		dgMinkFace *closestFace;
		dgMinkFacePurge *nextPurge;
		dgMinkFace *stackPool[128];
		dgMinkFace *deadFaces[128];
		SilhouetteFaceCap sillueteCap[128];
		dgVector diff[3];
		dgVector aver[3];
		dgInt8 buffer[DG_HEAP_EDGE_COUNT * (sizeof(dgFloat32) + sizeof(dgMinkFace *))];
		dgClosestFace heapSort(buffer, sizeof(buffer));

		m_planeIndex = 4;
		closestFace = NULL;
		m_facePurge = NULL;

		NEWTON_ASSERT(m_vertexIndex == 4);
		for (i = 0; i < 4; i++) {
			face = &m_simplex[i];
			face->m_inHeap = 0;
			face->m_isActive = 1;
			if (CalcFacePlane(face)) {
				face->m_inHeap = 1;
				heapSort.Push(face, face->m_w);
			}
		}

		cicling = 0;
		ciclingMem[0] = dgFloat32(1.0e10f);
		ciclingMem[1] = dgFloat32(1.0e10f);
		ciclingMem[2] = dgFloat32(1.0e10f);
		ciclingMem[3] = dgFloat32(1.0e10f);

		minValue = dgFloat32(1.0e10f);
		dgPlane bestPlane(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                  dgFloat32(0.0f));
		diff[0] = bestPlane;
		diff[1] = bestPlane;
		diff[2] = bestPlane;
		aver[0] = bestPlane;
		aver[1] = bestPlane;
		aver[2] = bestPlane;
		while (heapSort.GetCount()) {
			face = heapSort[0];
			face->m_inHeap = 0;
			heapSort.Pop();

			if (face->m_isActive) {
				const dgPlane &plane = *face;

				CalcSupportVertex(plane, m_vertexIndex);
				const dgVector &p = m_hullVertex[m_vertexIndex];
				dist = plane.Evalue(p);
				m_vertexIndex++;

				if (m_vertexIndex > 16) {
					if (dist < minValue) {
						if (dist >= dgFloat32(0.0f)) {
							minValue = dist;
							bestPlane = plane;

							i = face->m_vertex[0];
							diff[0] = m_hullVertex[i];
							aver[0] = m_averVertex[i];

							i = face->m_vertex[1];
							diff[1] = m_hullVertex[i];
							aver[1] = m_averVertex[i];

							i = face->m_vertex[2];
							diff[2] = m_hullVertex[i];
							aver[2] = m_averVertex[i];
						}
					}
				}

				ciclingMem[cicling] = dist;
				cicling = (cicling + 1) & 3;
				for (i = 0; i < 4; i++) {
					if (dgAbsf(dist - ciclingMem[i]) > dgFloat32(1.0e-6f)) {
						break;
					}
				}
				if (i == 4) {
					dist = dgFloat32(0.0f);
				}

				if ((m_vertexIndex > DG_MINK_MAX_POINTS) || (m_planeIndex > DG_MINK_MAX_FACES) || (heapSort.GetCount() > (DG_HEAP_EDGE_COUNT - 24))) {

					//                  dgTrace (("Max face count overflow, breaking with last best face\n"));

					dgPlane &planeF = *face;
					planeF = bestPlane;

					i = face->m_vertex[0];
					face->m_vertex[0] = 0;
					m_hullVertex[i] = diff[0];
					m_averVertex[i] = aver[0];

					i = face->m_vertex[1];
					face->m_vertex[1] = 1;
					m_hullVertex[i] = diff[1];
					m_averVertex[i] = aver[1];

					i = face->m_vertex[2];
					face->m_vertex[2] = 2;
					m_hullVertex[i] = diff[2];
					m_averVertex[i] = aver[2];
					dist = dgFloat32(0.0f);
				}

				if (dist < (dgFloat32(DG_IMPULSIVE_CONTACT_PENETRATION) / dgFloat32(16.0f))) {
					NEWTON_ASSERT(m_planeIndex <= DG_MINK_MAX_FACES_SIZE);
					NEWTON_ASSERT(heapSort.GetCount() <= DG_HEAP_EDGE_COUNT);
					closestFace = face;
					break;
				} else if (dist > dgFloat32(0.0f)) {
					NEWTON_ASSERT(face->m_inHeap == 0);

					stack = 0;
					deadCount = 1;
					silhouette = NULL;
					deadFaces[0] = face;
					closestFace = face;
					face->m_isActive = 0;
					for (i = 0; i < 3; i++) {
						adjacent = &m_simplex[face->m_adjancentFace[i]];
						NEWTON_ASSERT(adjacent->m_isActive);
						dist = adjacent->Evalue(p);
						if (dist > dgFloat32(0.0f)) {
							adjacent->m_isActive = 0;
							stackPool[stack] = adjacent;
							deadFaces[deadCount] = adjacent;
							stack++;
							deadCount++;
						} else {
							silhouette = adjacent;
						}
					}
					while (stack) {
						stack--;
						face = stackPool[stack];
						for (i = 0; i < 3; i++) {
							adjacent = &m_simplex[face->m_adjancentFace[i]];
							if (adjacent->m_isActive) {
								dist = adjacent->Evalue(p);
								if (dist > dgFloat32(0.0f)) {
									adjacent->m_isActive = 0;
									stackPool[stack] = adjacent;
									deadFaces[deadCount] = adjacent;
									stack++;
									deadCount++;
									NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(stackPool[0])));
									NEWTON_ASSERT(
									    deadCount < dgInt32(sizeof(deadFaces) / sizeof(deadFaces[0])));

								} else {
									silhouette = adjacent;
								}
							}
						}
					}

					if (!silhouette) {
						closestFace = face;
						break;
					}
					// build silhouette
					NEWTON_ASSERT(silhouette);
					NEWTON_ASSERT(silhouette->m_isActive);

					i2 = (m_vertexIndex - 1);
					lastSilhouette = silhouette;
					//                  NEWTON_ASSERT ( (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 1) ||
					//                             (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 2));
					NEWTON_ASSERT(
					    (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[1]) && (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[2]) && (silhouette->m_adjancentFace[1] != silhouette->m_adjancentFace[2]));

					adjacentIndex = DG_GETADJACENTINDEX_ACTIVE(silhouette);
					face = NewFace();
					i0 = silhouette->m_vertex[adjacentIndex];
					i1 = silhouette->m_vertex[adjacentIndex + 1];

					face->m_vertex[0] = dgInt16(i1);
					face->m_vertex[1] = dgInt16(i0);
					face->m_vertex[2] = dgInt16(i2);
					face->m_vertex[3] = face->m_vertex[0];
					face->m_adjancentFace[0] = dgInt16(silhouette - m_simplex);
					face->m_inHeap = 0;
					face->m_isActive = 1;

					sillueteCap[0].m_face = face;
					sillueteCap[0].m_faceCopling =
					    &silhouette->m_adjancentFace[adjacentIndex];
					silhouetteCapCount = 1;
					NEWTON_ASSERT(silhouetteCapCount < dgInt32(sizeof(sillueteCap) / sizeof(sillueteCap[0])));
					do {
						silhouette = &m_simplex[silhouette->m_adjancentFace[adjacentIndex]];
						adjacentIndex = (DG_GETADJACENTINDEX_VERTEX(silhouette, i0));
					} while (!silhouette->m_isActive);

					prevFace = face;
					firstFace = face;
					lastSilhouetteVertex = i0;
					prevEdgeIndex = dgInt32(face - m_simplex);
					do {
						//                      NEWTON_ASSERT ( (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 1) ||
						//                                  (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 2));
						NEWTON_ASSERT(
						    (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[1]) && (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[2]) && (silhouette->m_adjancentFace[1] != silhouette->m_adjancentFace[2]));

						adjacentIndex = adjacentIndex ? adjacentIndex - 1 : 2;

						face = NewFace();
						i0 = silhouette->m_vertex[adjacentIndex];
						i1 = silhouette->m_vertex[adjacentIndex + 1];

						face->m_vertex[0] = dgInt16(i1);
						face->m_vertex[1] = dgInt16(i0);
						face->m_vertex[2] = dgInt16(i2);
						face->m_vertex[3] = face->m_vertex[0];
						face->m_adjancentFace[0] = dgInt16(silhouette - m_simplex);
						face->m_adjancentFace[2] = dgInt16(prevEdgeIndex);
						face->m_inHeap = 0;
						face->m_isActive = 1;

						prevEdgeIndex = dgInt32(face - m_simplex);
						prevFace->m_adjancentFace[1] = dgInt16(prevEdgeIndex);
						prevFace = face;

						sillueteCap[silhouetteCapCount].m_face = face;
						sillueteCap[silhouetteCapCount].m_faceCopling =
						    &silhouette->m_adjancentFace[adjacentIndex];
						silhouetteCapCount++;
						NEWTON_ASSERT(silhouetteCapCount < dgInt32(sizeof(sillueteCap) / sizeof(sillueteCap[0])));

						do {
							silhouette =
							    &m_simplex[silhouette->m_adjancentFace[adjacentIndex]];
							adjacentIndex = (DG_GETADJACENTINDEX_VERTEX(silhouette, i0));
						} while (!silhouette->m_isActive);

					} while ((silhouette != lastSilhouette) || (silhouette->m_vertex[adjacentIndex ? adjacentIndex - 1 : 2] != lastSilhouetteVertex));
					firstFace->m_adjancentFace[2] = dgInt16(prevEdgeIndex);
					prevFace->m_adjancentFace[1] = dgInt16(firstFace - m_simplex);

					for (i = 0; i < deadCount; i++) {
						if (!deadFaces[i]->m_inHeap) {
							nextPurge = (dgMinkFacePurge *)deadFaces[i];
							nextPurge->m_next = m_facePurge;
							m_facePurge = nextPurge;
						}
					}

					while (heapSort.GetCount() && (!heapSort[0]->m_isActive)) {
						face = heapSort[0];
						heapSort.Pop();
						nextPurge = (dgMinkFacePurge *)face;
						nextPurge->m_next = m_facePurge;
						m_facePurge = nextPurge;
					}

					for (i = 0; i < silhouetteCapCount; i++) {
						face = sillueteCap[i].m_face;
						*sillueteCap[i].m_faceCopling = dgInt16(face - m_simplex);

						if (CalcFacePlane(face)) {
							face->m_inHeap = 1;
							heapSort.Push(face, face->m_w);
						}
					}
				}
			} else {
				NEWTON_ASSERT(0);
				nextPurge = (dgMinkFacePurge *)face;
				nextPurge->m_next = m_facePurge;
				m_facePurge = nextPurge;
			}
		}
		return closestFace;
	}

	dgMinkFace *CalculateClipPlaneLarge() {
		dgInt32 i;
		dgInt32 i0;
		dgInt32 i1;
		dgInt32 i2;
		dgInt32 stack;
		dgInt32 cicling;
		dgInt32 deadCount;
		dgInt32 adjacentIndex;
		dgInt32 prevEdgeIndex;
		dgInt32 silhouetteCapCount;
		dgInt32 lastSilhouetteVertex;

		dgFloat64 dist;
		dgFloat64 minValue;
		dgFloat64 ciclingMem[4];
		dgMinkFace *face;
		dgMinkFace *adjacent;
		dgMinkFace *prevFace;
		dgMinkFace *firstFace;
		dgMinkFace *silhouette;
		dgMinkFace *lastSilhouette;
		dgMinkFace *closestFace;
		dgMinkFacePurge *nextPurge;
		dgMinkFace *stackPool[128];
		dgMinkFace *deadFaces[128];
		SilhouetteFaceCap sillueteCap[128];
		dgBigVector diff[3];
		dgBigVector aver[3];
		dgInt8 buffer[DG_HEAP_EDGE_COUNT * (sizeof(dgFloat32) + sizeof(dgMinkFace *))];
		dgClosestFace heapSort(buffer, sizeof(buffer));

		m_planeIndex = 4;
		closestFace = NULL;
		m_facePurge = NULL;

		NEWTON_ASSERT(m_vertexIndex == 4);
		for (i = 0; i < 4; i++) {
			face = &m_simplex[i];
			face->m_inHeap = 0;
			face->m_isActive = 1;
			if (CalcFacePlaneLarge(face)) {
				face->m_inHeap = 1;
				heapSort.Push(face, face->m_w);
			}
		}

		cicling = 0;
		ciclingMem[0] = dgFloat32(1.0e10f);
		ciclingMem[1] = dgFloat32(1.0e10f);
		ciclingMem[2] = dgFloat32(1.0e10f);
		ciclingMem[3] = dgFloat32(1.0e10f);

		minValue = dgFloat32(1.0e10f);
		dgPlane bestPlane(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		                  dgFloat32(0.0f));
		diff[0] = dgBigVector(dgFloat64(0.0f), dgFloat64(0.0f), dgFloat64(0.0f),
		                      dgFloat64(0.0f));
		diff[1] = diff[0];
		diff[2] = diff[0];
		aver[0] = diff[0];
		aver[1] = diff[0];
		aver[2] = diff[0];

		face = NULL;
		while (heapSort.GetCount()) {
			face = heapSort[0];
			face->m_inHeap = 0;
			heapSort.Pop();

			if (face->m_isActive) {

				const dgPlane &plane = *face;

				CalcSupportVertexLarge(plane, m_vertexIndex);
				dgVector p(dgFloat32(m_hullVertexLarge[m_vertexIndex].m_x),
				           dgFloat32(m_hullVertexLarge[m_vertexIndex].m_y),
				           dgFloat32(m_hullVertexLarge[m_vertexIndex].m_z), dgFloat64(0.0f));
				dist = plane.Evalue(p);
				m_vertexIndex++;

				if (m_vertexIndex > 16) {
					if (dist < minValue) {
						if (dist >= dgFloat64(0.0f)) {
							minValue = dist;
							bestPlane = plane;

							i = face->m_vertex[0];
							diff[0] = m_hullVertexLarge[i];
							aver[0] = m_averVertexLarge[i];

							i = face->m_vertex[1];
							diff[1] = m_hullVertexLarge[i];
							aver[1] = m_averVertexLarge[i];

							i = face->m_vertex[2];
							diff[2] = m_hullVertexLarge[i];
							aver[2] = m_averVertexLarge[i];
						}
					}
				}

				ciclingMem[cicling] = dist;
				cicling = (cicling + 1) & 3;
				for (i = 0; i < 4; i++) {
					if (fabs(dist - ciclingMem[i]) > dgFloat64(1.0e-6f)) {
						break;
					}
				}
				if (i == 4) {
					dist = dgFloat64(0.0f);
				}

				if ((m_vertexIndex > DG_MINK_MAX_POINTS) || (m_planeIndex > DG_MINK_MAX_FACES) || (heapSort.GetCount() > (DG_HEAP_EDGE_COUNT - 24))) {

					//                  dgTrace (("Max face count overflow, breaking with last best face\n"));

					dgPlane &planeF = *face;
					planeF = bestPlane;

					i = face->m_vertex[0];
					face->m_vertex[0] = 0;
					m_hullVertexLarge[i] = diff[0];
					m_averVertexLarge[i] = aver[0];

					i = face->m_vertex[1];
					face->m_vertex[1] = 1;
					m_hullVertexLarge[i] = diff[1];
					m_averVertexLarge[i] = aver[1];

					i = face->m_vertex[2];
					face->m_vertex[2] = 2;
					m_hullVertexLarge[i] = diff[2];
					m_averVertexLarge[i] = aver[2];
					dist = dgFloat32(0.0f);
				}

				if (dist < (dgFloat32(DG_IMPULSIVE_CONTACT_PENETRATION) / dgFloat32(16.0f))) {
					NEWTON_ASSERT(m_planeIndex <= DG_MINK_MAX_FACES_SIZE);
					NEWTON_ASSERT(heapSort.GetCount() <= DG_HEAP_EDGE_COUNT);
					closestFace = face;
					break;
				} else if (dist > dgFloat32(0.0f)) {
					NEWTON_ASSERT(face->m_inHeap == 0);

					stack = 0;
					deadCount = 1;
					silhouette = NULL;
					deadFaces[0] = face;
					closestFace = face;
					face->m_isActive = 0;
					for (i = 0; i < 3; i++) {
						adjacent = &m_simplex[face->m_adjancentFace[i]];
						NEWTON_ASSERT(adjacent->m_isActive);
						dist = adjacent->Evalue(p);
						if (dist > dgFloat64(0.0f)) {
							adjacent->m_isActive = 0;
							stackPool[stack] = adjacent;
							deadFaces[deadCount] = adjacent;
							stack++;
							deadCount++;
						} else {
							silhouette = adjacent;
						}
					}

					while (stack) {
						stack--;
						face = stackPool[stack];
						for (i = 0; i < 3; i++) {
							adjacent = &m_simplex[face->m_adjancentFace[i]];
							if (adjacent->m_isActive) {
								dist = adjacent->Evalue(p);
								if (dist > dgFloat64(0.0f)) {
									adjacent->m_isActive = 0;
									stackPool[stack] = adjacent;
									deadFaces[deadCount] = adjacent;
									stack++;
									deadCount++;
									NEWTON_ASSERT(stack < dgInt32(sizeof(stackPool) / sizeof(stackPool[0])));
									NEWTON_ASSERT(deadCount < dgInt32(sizeof(deadFaces) / sizeof(deadFaces[0])));

								} else {
									silhouette = adjacent;
								}
							}
						}
					}

					if (!silhouette) {
						closestFace = face;
						break;
					}
					// build silhouette
					NEWTON_ASSERT(silhouette);
					NEWTON_ASSERT(silhouette->m_isActive);

					i2 = (m_vertexIndex - 1);
					lastSilhouette = silhouette;
					//                  NEWTON_ASSERT ( (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 1) ||
					//                             (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
					//                              (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 2));
					NEWTON_ASSERT(
					    (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[1]) && (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[2]) && (silhouette->m_adjancentFace[1] != silhouette->m_adjancentFace[2]));

					adjacentIndex = DG_GETADJACENTINDEX_ACTIVE(silhouette);
					face = NewFace();
					i0 = silhouette->m_vertex[adjacentIndex];
					i1 = silhouette->m_vertex[adjacentIndex + 1];

					face->m_vertex[0] = dgInt16(i1);
					face->m_vertex[1] = dgInt16(i0);
					face->m_vertex[2] = dgInt16(i2);
					face->m_vertex[3] = face->m_vertex[0];
					face->m_adjancentFace[0] = dgInt16(silhouette - m_simplex);
					face->m_inHeap = 0;
					face->m_isActive = 1;

					sillueteCap[0].m_face = face;
					sillueteCap[0].m_faceCopling =
					    &silhouette->m_adjancentFace[adjacentIndex];
					silhouetteCapCount = 1;
					NEWTON_ASSERT(
					    silhouetteCapCount < dgInt32(sizeof(sillueteCap) / sizeof(sillueteCap[0])));
					do {
						silhouette = &m_simplex[silhouette->m_adjancentFace[adjacentIndex]];
						adjacentIndex = (DG_GETADJACENTINDEX_VERTEX(silhouette, i0));
					} while (!silhouette->m_isActive);

					prevFace = face;
					firstFace = face;
					lastSilhouetteVertex = i0;
					prevEdgeIndex = dgInt32(face - m_simplex);
					do {
						//                      NEWTON_ASSERT ( (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 1) ||
						//                                  (((!m_simplex[silhouette->m_adjancentFace[0]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[1]].m_isActive) +
						//                                  (!m_simplex[silhouette->m_adjancentFace[2]].m_isActive)) == 2));
						NEWTON_ASSERT(
						    (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[1]) && (silhouette->m_adjancentFace[0] != silhouette->m_adjancentFace[2]) && (silhouette->m_adjancentFace[1] != silhouette->m_adjancentFace[2]));

						adjacentIndex = adjacentIndex ? adjacentIndex - 1 : 2;

						face = NewFace();
						i0 = silhouette->m_vertex[adjacentIndex];
						i1 = silhouette->m_vertex[adjacentIndex + 1];

						face->m_vertex[0] = dgInt16(i1);
						face->m_vertex[1] = dgInt16(i0);
						face->m_vertex[2] = dgInt16(i2);
						face->m_vertex[3] = face->m_vertex[0];
						face->m_adjancentFace[0] = dgInt16(silhouette - m_simplex);
						face->m_adjancentFace[2] = dgInt16(prevEdgeIndex);
						face->m_inHeap = 0;
						face->m_isActive = 1;

						prevEdgeIndex = dgInt32(face - m_simplex);
						prevFace->m_adjancentFace[1] = dgInt16(prevEdgeIndex);
						prevFace = face;

						sillueteCap[silhouetteCapCount].m_face = face;
						sillueteCap[silhouetteCapCount].m_faceCopling =
						    &silhouette->m_adjancentFace[adjacentIndex];
						silhouetteCapCount++;
						NEWTON_ASSERT(
						    silhouetteCapCount < dgInt32(sizeof(sillueteCap) / sizeof(sillueteCap[0])));

						do {
							silhouette =
							    &m_simplex[silhouette->m_adjancentFace[adjacentIndex]];
							adjacentIndex = (DG_GETADJACENTINDEX_VERTEX(silhouette, i0));
						} while (!silhouette->m_isActive);

					} while ((silhouette != lastSilhouette) || (silhouette->m_vertex[adjacentIndex ? adjacentIndex - 1 : 2] != lastSilhouetteVertex));
					firstFace->m_adjancentFace[2] = dgInt16(prevEdgeIndex);
					prevFace->m_adjancentFace[1] = dgInt16(firstFace - m_simplex);

					for (i = 0; i < deadCount; i++) {
						if (!deadFaces[i]->m_inHeap) {
							nextPurge = (dgMinkFacePurge *)deadFaces[i];
							nextPurge->m_next = m_facePurge;
							m_facePurge = nextPurge;
						}
					}

					while (heapSort.GetCount() && (!heapSort[0]->m_isActive)) {
						face = heapSort[0];
						heapSort.Pop();
						nextPurge = (dgMinkFacePurge *)face;
						nextPurge->m_next = m_facePurge;
						m_facePurge = nextPurge;
					}

					for (i = 0; i < silhouetteCapCount; i++) {
						face = sillueteCap[i].m_face;
						*sillueteCap[i].m_faceCopling = dgInt16(face - m_simplex);

						if (CalcFacePlaneLarge(face)) {
							face->m_inHeap = 1;
							heapSort.Push(face, face->m_w);
						}
					}
				}

			} else {
				NEWTON_ASSERT(0);
				nextPurge = (dgMinkFacePurge *)face;
				nextPurge->m_next = m_facePurge;
				m_facePurge = nextPurge;
			}
		}

		NEWTON_ASSERT(face);
		i = face->m_vertex[0];
		//      m_hullVertex[i] = dgVector (dgFloat32 (m_hullVertexLarge[i].m_x), dgFloat32 (m_hullVertexLarge[i].m_y), dgFloat32 (m_hullVertexLarge[i].m_z), dgFloat32 (0.0f));
		m_averVertex[i] = dgVector(dgFloat32(m_averVertexLarge[i].m_x),
		                           dgFloat32(m_averVertexLarge[i].m_y),
		                           dgFloat32(m_averVertexLarge[i].m_z), dgFloat32(0.0f));
		;
		return closestFace;
	}

public:
	inline dgContactSolver(dgCollisionParamProxy &proxy, dgCollision *polygon) : m_matrix(*proxy.m_localMatrixInv)
		//       m_simplexLarge ((dgMinkFaceLarge*) m_simplex),
		//       m_hullVertexLarge ((dgBigVector*) m_hullVertex),
		//       m_averVertexLarge ((dgBigVector*) m_averVertex)
	{
		void *hullVertexLarge = m_hullVertex;
		void *averVertexLarge = m_averVertex;
		m_hullVertexLarge = (dgBigVector *)hullVertexLarge;
		m_averVertexLarge = (dgBigVector *)averVertexLarge;

		NEWTON_ASSERT((m_matrix.m_front % m_matrix.m_front) > dgFloat32(0.9999f));
		NEWTON_ASSERT((m_matrix.m_up % m_matrix.m_up) > dgFloat32(0.9999f));
		NEWTON_ASSERT((m_matrix.m_right % m_matrix.m_right) > dgFloat32(0.9999f));
		NEWTON_ASSERT(
		    ((m_matrix.m_front * m_matrix.m_up) % m_matrix.m_right) < dgFloat32(1.0001f));

		m_lastFaceCode = dgMinkError;

		m_proxy = &proxy;
		m_floatingBody = proxy.m_floatingBody;
		// m_floatingcollision = (dgCollisionConvex*) proxy.m_floatingCollision;
		m_floatingcollision = (dgCollisionConvex *)polygon;
		m_referenceBody = proxy.m_referenceBody;
		m_referenceCollision = (dgCollisionConvex *)proxy.m_referenceCollision;
		m_penetrationPadding = proxy.m_penetrationPadding;
	}

	inline dgContactSolver(dgCollisionParamProxy &proxy) : m_matrix(*proxy.m_localMatrixInv)
		//       m_simplexLarge ((dgMinkFaceLarge*) m_simplex),
		//       m_hullVertexLarge ((dgBigVector*) m_hullVertex),
		//       m_averVertexLarge ((dgBigVector*) m_averVertex)
	{
		void *hullVertexLarge = m_hullVertex;
		void *averVertexLarge = m_averVertex;
		m_hullVertexLarge = (dgBigVector *)hullVertexLarge;
		m_averVertexLarge = (dgBigVector *)averVertexLarge;

		NEWTON_ASSERT((m_matrix.m_front % m_matrix.m_front) > dgFloat32(0.9995f));
		NEWTON_ASSERT((m_matrix.m_up % m_matrix.m_up) > dgFloat32(0.9995f));
		NEWTON_ASSERT((m_matrix.m_right % m_matrix.m_right) > dgFloat32(0.9995f));
		NEWTON_ASSERT(
		    ((m_matrix.m_front * m_matrix.m_up) % m_matrix.m_right) < dgFloat32(1.0001f));

		m_lastFaceCode = dgMinkError;

		m_proxy = &proxy;
		m_floatingBody = proxy.m_floatingBody;
		m_referenceBody = proxy.m_referenceBody;
		m_penetrationPadding = proxy.m_penetrationPadding;
		m_floatingcollision = (dgCollisionConvex *)proxy.m_floatingCollision;
		m_referenceCollision = (dgCollisionConvex *)proxy.m_referenceCollision;
	}

	dgContactSolver &operator=(const dgContactSolver &me);

	dgInt32 HullHullContactsSimd(dgInt32 contactID) {
#ifdef DG_BUILD_SIMD_CODE
		dgInt32 i0;
		dgInt32 count;
		dgMinkFace *face;
		dgMinkReturnCode code;
		dgContactPoint *contactOut;

		count = 0;
		m_proxy->m_inTriggerVolume = 0;
		code = CalcSeparatingPlaneSimd(face);
		switch (code) {
		case dgMinkIntersecting: {
			if (m_proxy->m_isTriggerVolume) {
				m_proxy->m_inTriggerVolume = 1;
			} else {
				face = CalculateClipPlaneSimd();
				if (face) {
					count = CalculateContactsSimd(face, contactID, m_proxy->m_contacts,
					                              m_proxy->m_maxContacts);
					NEWTON_ASSERT(count <= m_proxy->m_maxContacts);
				}
			}
			break;
		}

		case dgMinkDisjoint: {
			NEWTON_ASSERT(face);
			if (CalcFacePlaneSimd(face)) {
				//                  NEWTON_ASSERT (face->m_w >= dgFloat32 (0.0f));
				NEWTON_ASSERT((*face) % (*face) > dgFloat32(0.0f));
				if (face->m_w < m_penetrationPadding) {
					dgVector step(*face);
					step = step.Scale(-(face->m_w + DG_IMPULSIVE_CONTACT_PENETRATION));

					i0 = face->m_vertex[0];
					m_hullVertex[i0] -= step;
					m_averVertex[i0] += step;

					m_matrix.m_posit += step;
					dgVector stepWorld(m_proxy->m_referenceMatrix.RotateVectorSimd(step));
					m_proxy->m_floatingMatrix.m_posit += stepWorld;

					count = CalculateContactsSimd(face, contactID, m_proxy->m_contacts,
					                              m_proxy->m_maxContacts);
					NEWTON_ASSERT(count < m_proxy->m_maxContacts);
					stepWorld = stepWorld.Scale(dgFloat32(0.5f));

					if (m_proxy->m_isTriggerVolume) {
						m_proxy->m_inTriggerVolume = 1;
						count = 0;
					}

					contactOut = m_proxy->m_contacts;
					for (i0 = 0; i0 < count; i0++) {
						// contactOut[i0].m_point -= stepWorld ;
						(simd_type &)contactOut[i0].m_point =
						    simd_sub_v((simd_type &)contactOut[i0].m_point, (simd_type &)stepWorld);
					}
					return count;
				}
			}
		}
		case dgMinkError:
		default:
			;
		}
		return count;

#else
		return 0;
#endif
	}

	dgInt32 HullHullContacts(dgInt32 contactID) {
		dgInt32 i0;
		dgInt32 count;
		dgMinkFace *face;
		dgMinkReturnCode code;
		dgContactPoint *contactOut;

		count = 0;
		m_proxy->m_inTriggerVolume = 0;
		code = CalcSeparatingPlane(face);
		switch (code) {
		case dgMinkIntersecting: {
			if (m_proxy->m_isTriggerVolume) {
				m_proxy->m_inTriggerVolume = 1;
			} else {
				face = CalculateClipPlane();
				if (face) {
					count = CalculateContacts(face, contactID, m_proxy->m_contacts,
					                          m_proxy->m_maxContacts);
					NEWTON_ASSERT(count <= m_proxy->m_maxContacts);
				}
			}
			break;
		}

		case dgMinkDisjoint: {
			NEWTON_ASSERT(face);

			if (CalcFacePlane(face)) {
				//NEWTON_ASSERT (face->m_w >= dgFloat32 (0.0f));
				NEWTON_ASSERT(face->m_w >= dgFloat32(-1.0e-2f));
				NEWTON_ASSERT((*face) % (*face) > dgFloat32(0.0f));
				if (face->m_w < m_penetrationPadding) {
					dgVector step(*face);
					step = step.Scale(-(face->m_w + DG_IMPULSIVE_CONTACT_PENETRATION));

					i0 = face->m_vertex[0];
					m_hullVertex[i0] -= step;
					m_averVertex[i0] += step;

					m_matrix.m_posit += step;
					dgVector stepWorld(m_proxy->m_referenceMatrix.RotateVector(step));
					m_proxy->m_floatingMatrix.m_posit += stepWorld;

					count = CalculateContacts(face, contactID, m_proxy->m_contacts,
					                          m_proxy->m_maxContacts);
					stepWorld = stepWorld.Scale(dgFloat32(0.5f));

					if (m_proxy->m_isTriggerVolume) {
						m_proxy->m_inTriggerVolume = 1;
						count = 0;
					}

					contactOut = m_proxy->m_contacts;
					for (i0 = 0; i0 < count; i0++) {
						contactOut[i0].m_point -= stepWorld;
					}
					return count;
				}
			}
		}
		case dgMinkError:
		default:
			;
		}
		return count;
	}

	dgInt32 HullHullContactsLarge(dgInt32 contactID) {
		//      dgInt32 i0;
		dgInt32 count;
		dgMinkFace *face;
		dgMinkReturnCode code;
		dgContactPoint *contactOut;

		count = 0;
		m_proxy->m_inTriggerVolume = 0;
		code = CalcSeparatingPlaneLarge(face);

		switch (code) {
		case dgMinkIntersecting: {
			if (m_proxy->m_isTriggerVolume) {
				m_proxy->m_inTriggerVolume = 1;
			} else {
				face = CalculateClipPlaneLarge();
				if (face) {
					count = CalculateContacts(face, contactID, m_proxy->m_contacts,
					                          m_proxy->m_maxContacts);
				}
			}
			break;
		}

		case dgMinkDisjoint: {
			if (CalcFacePlaneLarge(face)) {

				//NEWTON_ASSERT (face->m_w >= dgFloat32 (0.0f));
				NEWTON_ASSERT(face->m_w >= dgFloat32(-1.0e-1f));
				NEWTON_ASSERT((*face) % (*face) > dgFloat32(0.0f));
				if (face->m_w < m_penetrationPadding) {
					dgInt32 i0;
					dgVector step(*face);
					step = step.Scale(-(face->m_w + DG_IMPULSIVE_CONTACT_PENETRATION));

					i0 = face->m_vertex[0];
					m_hullVertex[i0] = dgVector(dgFloat32(m_hullVertexLarge[i0].m_x),
					                            dgFloat32(m_hullVertexLarge[i0].m_y),
					                            dgFloat32(m_hullVertexLarge[i0].m_z), dgFloat32(0.0f));
					m_averVertex[i0] = dgVector(dgFloat32(m_averVertexLarge[i0].m_x),
					                            dgFloat32(m_averVertexLarge[i0].m_y),
					                            dgFloat32(m_averVertexLarge[i0].m_z), dgFloat32(0.0f));
					m_hullVertex[i0] -= step;
					m_averVertex[i0] += step;

					m_matrix.m_posit += step;
					dgVector stepWorld(m_proxy->m_referenceMatrix.RotateVector(step));
					m_proxy->m_floatingMatrix.m_posit += stepWorld;

					count = CalculateContacts(face, contactID, m_proxy->m_contacts,
					                          m_proxy->m_maxContacts);
					stepWorld = stepWorld.Scale(dgFloat32(0.5f));

					if (m_proxy->m_isTriggerVolume) {
						m_proxy->m_inTriggerVolume = 1;
						count = 0;
					}

					contactOut = m_proxy->m_contacts;
					for (i0 = 0; i0 < count; i0++) {
						contactOut[i0].m_point -= stepWorld;
					}
					return count;
				}
			}
		}
		case dgMinkError:
		default:
			;
		}
		return count;
	}

	dgInt32 HullHullContinueContactsSimd(dgFloat32 &timeStep,
	                                     dgContactPoint *const contactOut, dgInt32 contactID, dgInt32 maxContacts,
	                                     dgInt32 conditionalContactCalculationAtOrigin) {
#ifdef DG_BUILD_SIMD_CODE
		dgInt32 count;
		dgMinkFace *face;
		dgMinkReturnCode code;

		count = 0;
		code = CalcSeparatingPlaneSimd(face);

		m_lastFaceCode = code;
		if (code == dgMinkIntersecting) {

			if (m_proxy->m_isTriggerVolume) {
				timeStep = dgFloat32(0.0f);
				m_proxy->m_inTriggerVolume = 1;
				count = 0;
			} else {
				face = CalculateClipPlaneSimd();
				if (face) {
					if (conditionalContactCalculationAtOrigin) {
						dgFloat32 projVeloc;
						const dgPlane &plane = *face;
						projVeloc = plane.DotProductSimd(m_localRelVeloc);
						if (projVeloc >= dgFloat32(0.0f)) {
							return 0;
						}
					}
					if (maxContacts) {
						count = CalculateContactsSimd(face, contactID, contactOut,
						                              maxContacts);
						NEWTON_ASSERT(count <= maxContacts);
					}

					timeStep = dgFloat32(0.0f);
				}
			}
		} else if (code == dgMinkDisjoint) {
			dgInt32 i0;
			dgInt32 i1;
			dgInt32 i2;
			dgFloat32 t0;
			dgFloat32 projVeloc;
			dgFloat32 timeOfImpact;
			dgVector saveHull[3];
			dgVector saveAver[3];

			NEWTON_ASSERT(face);
			t0 = dgFloat32(0.0f);
			i0 = face->m_vertex[0];
			i1 = face->m_vertex[1];
			i2 = face->m_vertex[2];
			dgVector plane(
			    (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]));
			NEWTON_ASSERT(plane % plane > dgFloat32(0.0f));
			//          NEWTON_ASSERT (dgAbsf (plane % vRel) > dgFloat32 (0.0f));
			projVeloc = plane.DotProductSimd(m_localRelVeloc);
			if (projVeloc >= dgFloat32(-1.0e-24f)) {
				code = UpdateSeparatingPlaneSimd(
				           face,
				           dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				                    dgFloat32(0.0f)));
				if (code != dgMinkDisjoint) {
					return 0;
				}
				NEWTON_ASSERT(code == dgMinkDisjoint);

				i0 = face->m_vertex[0];
				i1 = face->m_vertex[1];
				i2 = face->m_vertex[2];
				plane = (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]);
				NEWTON_ASSERT(plane % plane > dgFloat32(0.0f));
				projVeloc = plane.DotProductSimd(m_localRelVeloc);
				if (projVeloc > dgFloat32(-1.0e-24f)) {
					//NEWTON_ASSERT ((plane % m_localRelVeloc) < dgFloat32 (0.0f));
					return 0;
				}
			}

			//          timeOfImpact = - plane.m_w / (plane % vRel + dgFloat32 (1.0e-24f));
			//          timeOfImpact = (plane % m_hullVertex[i0]) / (plane % m_localRelVeloc + dgFloat32 (1.0e-24f));
			timeOfImpact = (plane.DotProductSimd(m_hullVertex[i0])) / (plane.DotProductSimd(m_localRelVeloc) + dgFloat32(1.0e-24f));
			if (timeOfImpact > 0.0f) {
				saveHull[0] = m_hullVertex[i0];
				saveHull[1] = m_hullVertex[i1];
				saveHull[2] = m_hullVertex[i2];
				saveAver[0] = m_averVertex[i0];
				saveAver[1] = m_averVertex[i1];
				saveAver[2] = m_averVertex[i2];
				dgVector p1(m_localRelVeloc.Scale(timeOfImpact));

				for (dgInt32 maxPasses = 0;
				        (maxPasses < 32) && (timeOfImpact < timeStep) && (timeOfImpact > t0);
				        maxPasses++) {
					dgMinkFace *tmpFaceface;
					t0 = timeOfImpact;
					code = UpdateSeparatingPlaneSimd(tmpFaceface, p1);
					NEWTON_ASSERT(code != dgMinkError);
					if (code == dgMinkDisjoint) {
						dgFloat32 den;

						NEWTON_ASSERT(tmpFaceface);

						face = tmpFaceface;
						i0 = face->m_vertex[0];
						i1 = face->m_vertex[1];
						i2 = face->m_vertex[2];
						// dgPlane plane (m_hullVertex[i0], m_hullVertex[i1], m_hullVertex[i2]);
						dgVector plane(
						    (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]));
						NEWTON_ASSERT(plane % plane > dgFloat32(0.0f));

						//                      den = plane % m_localRelVeloc ;
						den = plane.DotProductSimd(m_localRelVeloc);
						if (den >= dgFloat32(-1.0e-24f)) {
							code = UpdateSeparatingPlaneSimd(tmpFaceface, p1);
							NEWTON_ASSERT(code == dgMinkDisjoint);

							i0 = face->m_vertex[0];
							i1 = face->m_vertex[1];
							i2 = face->m_vertex[2];
							// dgPlane plane (m_hullVertex[i0], m_hullVertex[i1], m_hullVertex[i2]);
							dgVector plane(
							    (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]));
							NEWTON_ASSERT(plane % plane > dgFloat32(0.0f));
							//                          den = plane % m_localRelVeloc;
							den = plane.DotProductSimd(m_localRelVeloc);
							if (den > dgFloat32(-1.0e-24f)) {
								return 0;
							}
						}

						saveHull[0] = m_hullVertex[i0];
						saveHull[1] = m_hullVertex[i1];
						saveHull[2] = m_hullVertex[i2];
						saveAver[0] = m_averVertex[i0];
						saveAver[1] = m_averVertex[i1];
						saveAver[2] = m_averVertex[i2];
						if (den < dgFloat32(-1.0e-24f)) {
							//                          timeOfImpact = (plane % m_hullVertex[i0]) / den;
							timeOfImpact = (plane.DotProductSimd(m_hullVertex[i0])) / den;
							if (timeOfImpact < 0.0f) {
								return 0;
							}
							NEWTON_ASSERT(timeOfImpact >= 0.0f);
							//NEWTON_ASSERT (timeOfImpact >= dgFloat32 (-1.0f));
							p1 = m_localRelVeloc.Scale(timeOfImpact);
						}
					}
				}

				if ((timeOfImpact <= timeStep) && (timeOfImpact >= dgFloat32(0.0f))) {

					if (maxContacts) {
						count = CalculateContactsContinuesSimd(contactID, contactOut,
						                                       maxContacts, saveHull, saveAver, timeOfImpact);
						NEWTON_ASSERT(count <= maxContacts);
					}

					if (m_proxy->m_isTriggerVolume) {
						m_proxy->m_inTriggerVolume = 1;
						count = 0;
					}

					timeStep = timeOfImpact;
					if (count) {
						// dgVector step (vRel.Scale (- timeOfImpact * 0.5f));
						// dgVector step (refVeloc.Scale (timeOfImpact));
						dgVector step(m_referenceBodyVeloc.Scale(timeOfImpact));
						for (i0 = 0; i0 < count; i0++) {
							//                          contactOut[i0].m_point += step;
							(simd_type &)contactOut[i0].m_point =
							    simd_add_v((simd_type &)contactOut[i0].m_point, (simd_type &)step);
						}
					}
				}
			}
		}
		return count;

#else
		return 0;
#endif
	}

	dgInt32 HullHullContinueContacts(dgFloat32 &timeStep,
	                                 dgContactPoint *const contactOut, dgInt32 contactID, dgInt32 maxContacts,
	                                 dgInt32 conditionalContactCalculationAtOrigin) {
		dgInt32 count;
		dgMinkFace *face;

		dgMinkReturnCode code;

		count = 0;
		code = CalcSeparatingPlane(face);

		m_lastFaceCode = code;
		if (code == dgMinkIntersecting) {

			if (m_proxy->m_isTriggerVolume) {
				timeStep = dgFloat32(0.0f);
				m_proxy->m_inTriggerVolume = 1;
				count = 0;
			} else {
				face = CalculateClipPlane();
				if (face) {
					if (conditionalContactCalculationAtOrigin) {
						dgFloat32 projVeloc;
						const dgPlane &plane = *face;
						projVeloc = plane % m_localRelVeloc;
						if (projVeloc >= dgFloat32(0.0f)) {
							return 0;
						}
					}
					if (maxContacts) {
						count = CalculateContacts(face, contactID, contactOut, maxContacts);
						NEWTON_ASSERT(count <= maxContacts);
					}
					timeStep = dgFloat32(0.0f);
				}
			}
		} else if (code == dgMinkDisjoint) {
			dgInt32 i0;
			dgInt32 i1;
			dgInt32 i2;
			dgFloat32 t0;
			dgFloat32 projVeloc;
			dgFloat32 timeOfImpact;
			dgVector saveHull[3];
			dgVector saveAver[3];

			NEWTON_ASSERT(face);
			t0 = dgFloat32(0.0f);
			i0 = face->m_vertex[0];
			i1 = face->m_vertex[1];
			i2 = face->m_vertex[2];
			dgVector plane(
			    (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]));
			NEWTON_ASSERT(plane % plane > dgFloat32(0.0f));
			//          NEWTON_ASSERT (dgAbsf (plane % vRel) > dgFloat32 (0.0f));
			projVeloc = plane % m_localRelVeloc;
			if (projVeloc >= dgFloat32(-1.0e-24f)) {
				code = UpdateSeparatingPlane(
				           face,
				           dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				                    dgFloat32(0.0f)));
				if (code != dgMinkDisjoint) {
					return 0;
				}
				NEWTON_ASSERT(code == dgMinkDisjoint);
				i0 = face->m_vertex[0];
				i1 = face->m_vertex[1];
				i2 = face->m_vertex[2];
				plane = (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]);

				NEWTON_ASSERT(plane % plane > dgFloat32(0.0f));
				projVeloc = plane % m_localRelVeloc;
				if (projVeloc >= dgFloat32(-1.0e-24f)) {
					//NEWTON_ASSERT ((plane % m_localRelVeloc) < dgFloat32 (0.0f));
					return 0;
				}
			}

			//          timeOfImpact = - plane.m_w / (plane % vRel + dgFloat32 (1.0e-24f));
			timeOfImpact = (plane % m_hullVertex[i0]) / (plane % m_localRelVeloc + dgFloat32(1.0e-24f));

			if (timeOfImpact > 0.0f) {
				saveHull[0] = m_hullVertex[i0];
				saveHull[1] = m_hullVertex[i1];
				saveHull[2] = m_hullVertex[i2];
				saveAver[0] = m_averVertex[i0];
				saveAver[1] = m_averVertex[i1];
				saveAver[2] = m_averVertex[i2];
				dgVector p1(m_localRelVeloc.Scale(timeOfImpact));

				for (dgInt32 maxPasses = 0;
				        (maxPasses < 32) && (timeOfImpact < timeStep) && (timeOfImpact > t0);
				        maxPasses++) {
					dgMinkFace *tmpFaceface;
					t0 = timeOfImpact;
					code = UpdateSeparatingPlane(tmpFaceface, p1);
					NEWTON_ASSERT(code != dgMinkError);
					if (code == dgMinkDisjoint) {
						dgFloat32 den;

						NEWTON_ASSERT(tmpFaceface);

						face = tmpFaceface;
						i0 = face->m_vertex[0];
						i1 = face->m_vertex[1];
						i2 = face->m_vertex[2];
						// dgPlane planeTemp (m_hullVertex[i0], m_hullVertex[i1], m_hullVertex[i2]);
						dgVector planeTemp(
						    (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]));
						NEWTON_ASSERT(planeTemp % planeTemp > dgFloat32(0.0f));

						den = planeTemp % m_localRelVeloc;
						//                      if (dgAbsf (den) > dgFloat32 (1.0e-24f)) {
						if (den >= dgFloat32(-1.0e-24f)) {
							code = UpdateSeparatingPlane(tmpFaceface, p1);
							NEWTON_ASSERT(code == dgMinkDisjoint);

							i0 = face->m_vertex[0];
							i1 = face->m_vertex[1];
							i2 = face->m_vertex[2];
							// dgPlane planeT (m_hullVertex[i0], m_hullVertex[i1], m_hullVertex[i2]);
							dgVector planeT(
							    (m_hullVertex[i1] - m_hullVertex[i0]) * (m_hullVertex[i2] - m_hullVertex[i0]));
							NEWTON_ASSERT(planeT % planeT > dgFloat32(-1.0e-24f));
							den = planeT % m_localRelVeloc;
							if (den >= dgFloat32(0.0f)) {
								return 0;
							}
						}

						saveHull[0] = m_hullVertex[i0];
						saveHull[1] = m_hullVertex[i1];
						saveHull[2] = m_hullVertex[i2];
						saveAver[0] = m_averVertex[i0];
						saveAver[1] = m_averVertex[i1];
						saveAver[2] = m_averVertex[i2];
						if (den < dgFloat32(-1.0e-24f)) {
							timeOfImpact = (planeTemp % m_hullVertex[i0]) / den;
							if (timeOfImpact < 0.0f) {
								return 0;
							}

							NEWTON_ASSERT(timeOfImpact >= 0.0f);

							//NEWTON_ASSERT (timeOfImpact >= dgFloat32 (-1.0f));
							p1 = m_localRelVeloc.Scale(timeOfImpact);
						}
					}
				}

				if ((timeOfImpact <= timeStep) && (timeOfImpact >= dgFloat32(0.0f))) {

					if (maxContacts) {
						count = CalculateContactsContinues(contactID, contactOut,
						                                   maxContacts, saveHull, saveAver, timeOfImpact);
						NEWTON_ASSERT(count <= maxContacts);
					}

					if (m_proxy->m_isTriggerVolume) {
						m_proxy->m_inTriggerVolume = 1;
						count = 0;
					}

					timeStep = timeOfImpact;
					if (count) {
						// dgVector step (vRel.Scale (- timeOfImpact * 0.5f));
						// dgVector step (refVeloc.Scale (timeOfImpact));
						dgVector step(m_referenceBodyVeloc.Scale(timeOfImpact));
						for (i0 = 0; i0 < count; i0++) {
							contactOut[i0].m_point += step;
						}
					}
				}
			}
		}
		return count;
	}

	void CalculateVelocities(dgFloat32 timestep) {
		dgVector refOmega;
		dgVector floatOmega;

		m_referenceBody->CalculateContinueVelocity(timestep, m_referenceBodyVeloc,
		        refOmega);
		m_floatingBody->CalculateContinueVelocity(timestep, m_floatingBodyVeloc,
		        floatOmega);
		dgVector vRel(m_floatingBodyVeloc - m_referenceBodyVeloc);
		m_localRelVeloc = m_proxy->m_referenceMatrix.UnrotateVector(vRel);
	}

	void CalculateVelocitiesSimd(dgFloat32 timestep) {
#ifdef DG_BUILD_SIMD_CODE
		dgVector refOmega;
		dgVector floatOmega;

		m_referenceBody->CalculateContinueVelocitySimd(timestep,
		        m_referenceBodyVeloc, refOmega);
		m_floatingBody->CalculateContinueVelocitySimd(timestep, m_floatingBodyVeloc,
		        floatOmega);
		dgVector vRel(m_floatingBodyVeloc - m_referenceBodyVeloc);
		//      m_localRelVeloc = m_referenceBody->m_collisionWorldMatrix.UnrotateVectorSimd(vRel);
		m_localRelVeloc = m_proxy->m_referenceMatrix.UnrotateVectorSimd(vRel);
#else
#endif
	}

	dgMatrix m_matrix;
	dgVector m_localRelVeloc;
	dgVector m_floatingBodyVeloc;
	dgVector m_referenceBodyVeloc;
	dgVector m_hullVertex[DG_MINK_MAX_POINTS_SIZE * 2];
	dgVector m_averVertex[DG_MINK_MAX_POINTS_SIZE * 2];
	dgMinkFace m_simplex[DG_MINK_MAX_FACES_SIZE * 2];

	dgInt32 m_planeIndex;
	dgInt32 m_vertexIndex;
	dgFloat32 m_penetrationPadding;
	dgBody *m_floatingBody;
	dgBody *m_referenceBody;
	dgCollisionConvex *m_floatingcollision;
	dgCollisionConvex *m_referenceCollision;
	dgCollisionParamProxy *m_proxy;
	dgMinkFacePurge *m_facePurge;

	//  dgMinkFaceLarge* const m_simplexLarge;
	dgBigVector *m_hullVertexLarge;
	dgBigVector *m_averVertexLarge;

	dgMinkReturnCode m_lastFaceCode;

#ifdef DG_BUILD_SIMD_CODE
	static simd_type m_zero;
	static simd_type m_nrh0p5;
	static simd_type m_nrh3p0;
	static simd_type m_negIndex;
	static simd_type m_index_yx;
	static simd_type m_index_wz;
	static simd_type m_negativeOne;
	static simd_type m_zeroTolerenace;
#endif

	static dgVector m_dir[14];
	static dgInt32 m_faceIndex[][4];

	friend class dgWorld;
} DG_GCC_VECTOR_ALIGMENT;
#ifdef DG_BUILD_SIMD_CODE
simd_type dgContactSolver::m_zero;
simd_type dgContactSolver::m_nrh0p5;
simd_type dgContactSolver::m_nrh3p0;
simd_type dgContactSolver::m_negIndex;
simd_type dgContactSolver::m_index_yx;
simd_type dgContactSolver::m_index_wz;
simd_type dgContactSolver::m_negativeOne;
simd_type dgContactSolver::m_zeroTolerenace;
#endif

dgVector dgContactSolver::m_dir[14];

dgInt32 dgContactSolver::m_faceIndex[][4] = {
	{0, 1, 2, 3},
	{1, 0, 3, 2},
	{0, 2, 3, 1},
	{2, 1, 3, 0},
};

void dgWorld::InitConvexCollision() {
	dgInt32 i;

//	#ifndef __USE_DOUBLE_PRECISION__
#ifdef DG_BUILD_SIMD_CODE
	//      dgInt32* ptr;

	((dgVector &)dgContactSolver::m_zero) = dgVector(dgFloat32(0.0f),
	                                        dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	((dgVector &)dgContactSolver::m_negativeOne) = dgVector(dgFloat32(-1.0f),
	        dgFloat32(-1.0f), dgFloat32(-1.0f), dgFloat32(-1.0f));
	((dgVector &)dgContactSolver::m_zeroTolerenace) = dgVector(
	            DG_DISTANCE_TOLERANCE_ZERO, DG_DISTANCE_TOLERANCE_ZERO,
	            DG_DISTANCE_TOLERANCE_ZERO, DG_DISTANCE_TOLERANCE_ZERO);
	((dgVector &)dgContactSolver::m_nrh0p5) = dgVector(dgFloat32(0.5f),
	        dgFloat32(0.5f), dgFloat32(0.5f), dgFloat32(0.5f));
	((dgVector &)dgContactSolver::m_nrh3p0) = dgVector(dgFloat32(3.0f),
	        dgFloat32(3.0f), dgFloat32(3.0f), dgFloat32(3.0f));

	((dgVector &)dgContactSolver::m_index_yx) = dgVector(dgFloat32(0.0f),
	        dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(1.0f));
	//      ptr = (dgInt32*) &dgContactSolver::m_index_yx ;
	//      ptr[0] = 0;
	//      ptr[1] = 1;
	//      ptr[2] = 0;
	//      ptr[3] = 1;

	((dgVector &)dgContactSolver::m_index_wz) = dgVector(dgFloat32(2.0f),
	        dgFloat32(3.0f), dgFloat32(2.0f), dgFloat32(3.0f));
	//      ptr = (dgInt32*) &dgContactSolver::m_index_wz;
	//      ptr[0] = 2;
	//      ptr[1] = 3;
	//      ptr[2] = 2;
	//      ptr[3] = 3;

	((dgVector &)dgContactSolver::m_negIndex) = dgVector(dgFloat32(-1.0f),
	        dgFloat32(-1.0f), dgFloat32(-1.0f), dgFloat32(-1.0f));
//		ptr = (dgInt32*) &dgContactSolver::m_negIndex;
//		ptr[0] = -1;
//		ptr[1] = -1;
//		ptr[2] = -1;
//		ptr[3] = -1;
#endif

	dgContactSolver::m_dir[0] = dgVector(dgFloat32(1.0f), -dgFloat32(1.0f),
	                                     dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[1] = dgVector(-dgFloat32(1.0f), -dgFloat32(1.0f),
	                                     -dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[2] = dgVector(dgFloat32(1.0f), -dgFloat32(1.0f),
	                                     -dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[3] = dgVector(-dgFloat32(1.0f), dgFloat32(1.0f),
	                                     dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[4] = dgVector(dgFloat32(1.0f), dgFloat32(1.0f),
	                                     -dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[5] = dgVector(-dgFloat32(1.0f), dgFloat32(1.0f),
	                                     -dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[6] = dgVector(-dgFloat32(1.0f), -dgFloat32(1.0f),
	                                     dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[7] = dgVector(dgFloat32(1.0f), dgFloat32(1.0f),
	                                     dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[8] = dgVector(dgFloat32(0.0f), -dgFloat32(1.0f),
	                                     dgFloat32(0.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[9] = dgVector(dgFloat32(0.0f), dgFloat32(1.0f),
	                                     dgFloat32(0.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[10] = dgVector(dgFloat32(1.0f), dgFloat32(0.0f),
	                                      dgFloat32(0.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[11] = dgVector(-dgFloat32(1.0f), dgFloat32(0.0f),
	                                      dgFloat32(0.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[12] = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
	                                      dgFloat32(1.0f), dgFloat32(0.0f));
	dgContactSolver::m_dir[13] = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
	                                      -dgFloat32(1.0f), dgFloat32(0.0f));

	for (i = 0;
	        i < dgInt32(
	            sizeof(dgContactSolver::m_dir) / sizeof(dgContactSolver::m_dir[0]));
	        i++) {
		dgVector dir(dgContactSolver::m_dir[i]);
		dgContactSolver::m_dir[i] = dir.Scale(dgFloat32(1.0f) / dgSqrt(dir % dir));
	}

	dgCollisionConvex::m_multiResDir[0] = dgVector(dgFloat32(0.577350f),
	                                      dgFloat32(0.577350f), dgFloat32(0.577350f), dgFloat32(0.0f));
	dgCollisionConvex::m_multiResDir[1] = dgVector(dgFloat32(-0.577350f),
	                                      dgFloat32(0.577350f), dgFloat32(0.577350f), dgFloat32(0.0f));
	dgCollisionConvex::m_multiResDir[2] = dgVector(dgFloat32(0.577350f),
	                                      dgFloat32(-0.577350f), dgFloat32(0.577350f), dgFloat32(0.0f));
	dgCollisionConvex::m_multiResDir[3] = dgVector(dgFloat32(-0.577350f),
	                                      dgFloat32(-0.577350f), dgFloat32(0.577350f), dgFloat32(0.0f));

	dgCollisionConvex::m_multiResDir[4] =
	    dgCollisionConvex::m_multiResDir[0].Scale(dgFloat32(-1.0f));
	dgCollisionConvex::m_multiResDir[5] =
	    dgCollisionConvex::m_multiResDir[1].Scale(dgFloat32(-1.0f));
	dgCollisionConvex::m_multiResDir[6] =
	    dgCollisionConvex::m_multiResDir[2].Scale(dgFloat32(-1.0f));
	dgCollisionConvex::m_multiResDir[7] =
	    dgCollisionConvex::m_multiResDir[3].Scale(dgFloat32(-1.0f));

	dgCollisionConvex::m_multiResDir_sse[0] = dgVector(
	            dgCollisionConvex::m_multiResDir[0].m_x,
	            dgCollisionConvex::m_multiResDir[1].m_x,
	            dgCollisionConvex::m_multiResDir[2].m_x,
	            dgCollisionConvex::m_multiResDir[3].m_x);
	dgCollisionConvex::m_multiResDir_sse[1] = dgVector(
	            dgCollisionConvex::m_multiResDir[0].m_y,
	            dgCollisionConvex::m_multiResDir[1].m_y,
	            dgCollisionConvex::m_multiResDir[2].m_y,
	            dgCollisionConvex::m_multiResDir[3].m_y);
	dgCollisionConvex::m_multiResDir_sse[2] = dgVector(
	            dgCollisionConvex::m_multiResDir[0].m_z,
	            dgCollisionConvex::m_multiResDir[1].m_z,
	            dgCollisionConvex::m_multiResDir[2].m_z,
	            dgCollisionConvex::m_multiResDir[3].m_z);
	dgCollisionConvex::m_multiResDir_sse[3] = dgVector(
	            dgCollisionConvex::m_multiResDir[4].m_x,
	            dgCollisionConvex::m_multiResDir[5].m_x,
	            dgCollisionConvex::m_multiResDir[6].m_x,
	            dgCollisionConvex::m_multiResDir[7].m_x);
	dgCollisionConvex::m_multiResDir_sse[4] = dgVector(
	            dgCollisionConvex::m_multiResDir[4].m_y,
	            dgCollisionConvex::m_multiResDir[5].m_y,
	            dgCollisionConvex::m_multiResDir[6].m_y,
	            dgCollisionConvex::m_multiResDir[7].m_y);
	dgCollisionConvex::m_multiResDir_sse[5] = dgVector(
	            dgCollisionConvex::m_multiResDir[4].m_z,
	            dgCollisionConvex::m_multiResDir[5].m_z,
	            dgCollisionConvex::m_multiResDir[6].m_z,
	            dgCollisionConvex::m_multiResDir[7].m_z);

#ifdef DG_BUILD_SIMD_CODE

	//  dgInt32* ptr;
	//  ptr = (dgInt32*) &dgCollisionConvex::m_signMask.m_x;
	//  ptr[0] = 0x7fffffff;
	//  ptr[1] = 0x7fffffff;
	//  ptr[2] = 0x7fffffff;
	//  ptr[3] = 0x7fffffff;

	dgFloatSign tmp;
	tmp.m_integer.m_iVal = 0x7fffffff;
	dgCollisionConvex::m_signMask.m_x = tmp.m_fVal;
	dgCollisionConvex::m_signMask.m_y = tmp.m_fVal;
	dgCollisionConvex::m_signMask.m_z = tmp.m_fVal;
	dgCollisionConvex::m_signMask.m_w = tmp.m_fVal;

	//  ptr = (dgInt32*) &dgCollisionConvex::m_triplexMask.m_x;
	//  ptr[0] = 0xffffffff;
	//  ptr[1] = 0xffffffff;
	//  ptr[2] = 0xffffffff;
	//  ptr[3] = 0x0;

	tmp.m_integer.m_iVal = 0xffffffff;
	dgCollisionConvex::m_triplexMask.m_x = tmp.m_fVal;
	dgCollisionConvex::m_triplexMask.m_y = tmp.m_fVal;
	dgCollisionConvex::m_triplexMask.m_z = tmp.m_fVal;
	dgCollisionConvex::m_triplexMask.m_w = 0.0f;

#endif

#ifdef _DEBUG
	for (i = 0;
	        i < dgInt32(
	            sizeof(dgContactSolver::m_dir) / sizeof(dgContactSolver::m_dir[0]));
	        i++) {
		NEWTON_ASSERT(
		    dgContactSolver::m_dir[i] % dgContactSolver::m_dir[i] > dgFloat32(0.9999f));
		for (dgInt32 j = i + 1;
		        j < dgInt32(
		            sizeof(dgContactSolver::m_dir) / sizeof(dgContactSolver::m_dir[0]));
		        j++) {
			NEWTON_ASSERT(
			    dgContactSolver::m_dir[i] % dgContactSolver::m_dir[j] < dgFloat32(0.9999f));
		}
	}
#endif
}

dgInt32 dgWorld::SphereSphereCollision(const dgVector &sph0, dgFloat32 radius0,
                                       const dgVector &sph1, dgFloat32 radius1, dgCollisionParamProxy &proxy) const {
	dgFloat32 dist;
	dgFloat32 mag2;

	dgVector dir(sph1 - sph0);
	dgContactPoint *const contactOut = proxy.m_contacts;

	proxy.m_inTriggerVolume = 0;

	mag2 = dir % dir;
	if (mag2 < dgFloat32(1.0e-4f)) {
		// if the two spheres are located at the exact same origin just move then apart in any direction
		if (proxy.m_isTriggerVolume) {
			proxy.m_inTriggerVolume = 1;
			return 0;
		}

		contactOut[0].m_normal = dgVector(dgFloat32(0.0f), dgFloat32(1.0f),
		                                  dgFloat32(0.0f), dgFloat32(0.0f));
		contactOut[0].m_point = sph0 + contactOut[0].m_normal.Scale(radius0);
		contactOut[0].m_penetration = dgFloat32(0.01f);
		contactOut[0].m_userId = 0;
		return 1;
	}
	dgFloat32 mag;
	mag = dgSqrt(mag2);

	// get penetration distance
	dist = mag - (radius0 + radius1 + proxy.m_penetrationPadding);
	if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
		return 0;
	}

	dist = (dgAbsf(dist) - DG_IMPULSIVE_CONTACT_PENETRATION);
	if (dist < dgFloat32(0.0f)) {
		dist = dgFloat32(0.0f);
	}

	if (proxy.m_isTriggerVolume) {
		proxy.m_inTriggerVolume = 1;
		return 0;
	}

	contactOut[0].m_normal = dir.Scale(-dgFloat32(1.0f) / mag);
	contactOut[0].m_point = sph0 - contactOut[0].m_normal.Scale(mag * dgFloat32(0.5f));
	contactOut[0].m_penetration = dist;
	contactOut[0].m_userId = 0;
	return 1;
}

dgInt32 dgWorld::CalculateSphereToSphereContacts(
    dgCollisionParamProxy &proxy) const {
	dgFloat32 radius1;
	dgFloat32 radius2;
	const dgCollisionSphere *collSph1;
	const dgCollisionSphere *collSph2;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgCollisionSphere_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionSphere_RTTI));

	collSph1 = (dgCollisionSphere *)proxy.m_referenceCollision;
	collSph2 = (dgCollisionSphere *)proxy.m_floatingCollision;

	radius1 = collSph1->m_radius;
	radius2 = collSph2->m_radius;

	const dgVector &center1 = proxy.m_referenceMatrix.m_posit;
	const dgVector &center2 = proxy.m_floatingMatrix.m_posit;
	return SphereSphereCollision(center1, radius1, center2, radius2, proxy);
}

dgInt32 dgWorld::CalculateCapsuleToSphereContacts(
    dgCollisionParamProxy &proxy) const {
	dgFloat32 sphereRadius;
	dgFloat32 capsuleRadius;
	const dgCollisionSphere *sphere;
	const dgCollisionCapsule *capsule;

	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionSphere_RTTI));
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgCollisionCapsule_RTTI));

	sphere = (dgCollisionSphere *)proxy.m_floatingCollision;
	capsule = (dgCollisionCapsule *)proxy.m_referenceCollision;

	const dgVector &sphereCenter = proxy.m_floatingMatrix.m_posit;

	sphereRadius = sphere->m_radius;
	capsuleRadius = capsule->GetRadius();
	dgVector cylP0(
	    proxy.m_referenceMatrix.TransformVector(
	        dgVector(-capsule->GetHeight(), dgFloat32(0.0f), dgFloat32(0.0f),
	                 dgFloat32(0.0f))));
	dgVector cylP1(
	    proxy.m_referenceMatrix.TransformVector(
	        dgVector(capsule->GetHeight(), dgFloat32(0.0f), dgFloat32(0.0f),
	                 dgFloat32(0.0f))));
	dgVector point(dgPointToRayDistance(sphereCenter, cylP0, cylP1));
	return SphereSphereCollision(point, capsuleRadius, sphereCenter, sphereRadius,
	                             proxy);
}

dgInt32 dgWorld::CalculateCapsuleToCapsuleContacts(
    dgCollisionParamProxy &proxy) const {
	dgInt32 count;
	dgFloat32 dot;
	dgFloat32 mag;
	dgFloat32 mag2;
	dgFloat32 dist;
	dgFloat32 radius1;
	dgFloat32 radius2;
	const dgCollisionCapsule *collSph1;
	const dgCollisionCapsule *collSph2;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgCollisionCapsule_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionCapsule_RTTI));

	proxy.m_inTriggerVolume = 0;
	collSph1 = (dgCollisionCapsule *)proxy.m_referenceCollision;
	collSph2 = (dgCollisionCapsule *)proxy.m_floatingCollision;

	radius1 = collSph1->GetRadius();
	radius2 = collSph2->GetRadius();
	dgContactPoint *const contactOut = proxy.m_contacts;

	dgVector cylP0(-collSph1->GetHeight(), dgFloat32(0.0f), dgFloat32(0.0f),
	               dgFloat32(0.0f));
	dgVector cylP1(collSph1->GetHeight(), dgFloat32(0.0f), dgFloat32(0.0f),
	               dgFloat32(0.0f));
	dgVector cylQ0(-collSph2->GetHeight(), dgFloat32(0.0f), dgFloat32(0.0f),
	               dgFloat32(0.0f));
	dgVector cylQ1(collSph2->GetHeight(), dgFloat32(0.0f), dgFloat32(0.0f),
	               dgFloat32(0.0f));
	cylQ0 = proxy.m_referenceMatrix.UntransformVector(
	            (proxy.m_floatingMatrix.TransformVector(cylQ0)));
	cylQ1 = proxy.m_referenceMatrix.UntransformVector(
	            (proxy.m_floatingMatrix.TransformVector(cylQ1)));
	dgVector p10(cylP1 - cylP0);
	dgVector q10(cylQ1 - cylQ0);

	count = 0;
	p10 = p10.Scale(dgFloat32(1.0f) / dgSqrt(p10 % p10 + dgFloat32(1.0e-8f)));
	q10 = q10.Scale(dgFloat32(1.0f) / dgSqrt(q10 % q10 + dgFloat32(1.0e-8f)));
	dot = q10 % p10;
	if (dgAbsf(dot) > dgFloat32(0.998f)) {
		dgFloat32 pl0;
		dgFloat32 pl1;
		dgFloat32 ql0;
		dgFloat32 ql1;
		dgFloat32 distSign1;
		dgFloat32 distSign2;
		pl0 = cylP0 % p10;
		pl1 = cylP1 % p10;
		ql0 = cylQ0 % p10;
		ql1 = cylQ1 % p10;

		distSign1 = -1.0f;

		if (pl0 > pl1) {
			distSign1 = 1.0f;
			Swap(pl0, pl1);
			Swap(cylP0, cylP1);
			p10 = p10.Scale(dgFloat32(-1.0f));
		}

		distSign2 = -1.0f;
		if (ql0 > ql1) {
			distSign2 = 1.0f;
			Swap(ql0, ql1);
		}

		NEWTON_ASSERT(pl0 <= pl1);
		NEWTON_ASSERT(ql0 <= ql1);
		if (ql0 >= pl1) {
			dgVector center1(
			    proxy.m_referenceMatrix.TransformVector(
			        dgVector(-collSph1->GetHeight() * distSign1, dgFloat32(0.0f),
			                 dgFloat32(0.0f), dgFloat32(0.0f))));
			dgVector center2(
			    proxy.m_floatingMatrix.TransformVector(
			        dgVector(collSph2->GetHeight() * distSign2, dgFloat32(0.0f),
			                 dgFloat32(0.0f), dgFloat32(0.0f))));
			return SphereSphereCollision(center1, radius1, center2, radius2, proxy);

		} else if (ql1 <= pl0) {
			dgVector center1(
			    proxy.m_referenceMatrix.TransformVector(
			        dgVector(collSph1->GetHeight() * distSign1, dgFloat32(0.0f),
			                 dgFloat32(0.0f), dgFloat32(0.0f))));
			dgVector center2(
			    proxy.m_floatingMatrix.TransformVector(
			        dgVector(-collSph2->GetHeight() * distSign2, dgFloat32(0.0f),
			                 dgFloat32(0.0f), dgFloat32(0.0f))));
			return SphereSphereCollision(center1, radius1, center2, radius2, proxy);
		}

		dgFloat32 clip0;
		dgFloat32 clip1;
		clip0 = (ql0 > pl0) ? ql0 : pl0;
		clip1 = (ql1 < pl1) ? ql1 : pl1;

		count = 2;
		dgVector projectedQ(cylP0 + p10.Scale((cylQ0 - cylP0) % p10));

		dgVector dir(cylQ0 - projectedQ);
		mag2 = dir % dir;
		if (mag2 < dgFloat32(1.0e-4f)) {
			return 0;
		}

		mag = dgSqrt(mag2);
		dist = mag - (radius1 + radius1 + proxy.m_penetrationPadding);
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}

		dist = (dgAbsf(dist) - DG_IMPULSIVE_CONTACT_PENETRATION);
		if (dist < dgFloat32(0.0f)) {
			dist = dgFloat32(0.0f);
		}

		if (proxy.m_isTriggerVolume) {
			proxy.m_inTriggerVolume = 1;
			return 0;
		}

		mag = -dgFloat32(1.0f) / mag;
		contactOut[0].m_normal = dir.Scale(mag);
		contactOut[0].m_point = cylP0 + p10.Scale(clip0 - pl0) - contactOut[0].m_normal.Scale(mag * dgFloat32(0.5f));
		contactOut[0].m_userId = 0;
		contactOut[0].m_penetration = dist;

		contactOut[1].m_normal = dir.Scale(mag);
		contactOut[1].m_point = cylP0 + p10.Scale(clip1 - pl0) - contactOut[1].m_normal.Scale(mag * dgFloat32(0.5f));
		contactOut[1].m_userId = 0;
		contactOut[1].m_penetration = dist;

	} else {
		dgVector center1;
		dgVector center2;
		dgRayToRayDistance(cylP0, cylP1, cylQ0, cylQ1, center1, center2);
		count = SphereSphereCollision(center1, radius1, center2, radius2, proxy);
	}

	for (dgInt32 i = 0; i < count; i++) {
		NEWTON_ASSERT(
		    dgAbsf((contactOut[i].m_normal % contactOut[i].m_normal) - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));
		contactOut[i].m_normal = proxy.m_referenceMatrix.RotateVector(
		                             contactOut[i].m_normal);
		contactOut[i].m_point = proxy.m_referenceMatrix.TransformVector(
		                            contactOut[i].m_point);
	}
	return count;
}

dgInt32 dgWorld::CalculateBoxToSphereContacts(
    dgCollisionParamProxy &proxy) const {
	dgInt32 code;
	dgInt32 codeX;
	dgInt32 codeY;
	dgInt32 codeZ;
	dgFloat32 dist;
	dgFloat32 radius;
	dgContactPoint *contactOut;
	const dgCollisionBox *collBox;
	const dgCollisionSphere *collSph;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgCollisionBox_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionSphere_RTTI));

	const dgMatrix &boxMatrix = proxy.m_referenceMatrix;
	const dgMatrix &sphMatrix = proxy.m_floatingMatrix;

	collBox = (dgCollisionBox *)proxy.m_referenceCollision;
	collSph = (dgCollisionSphere *)proxy.m_floatingCollision;

	radius = collSph->m_radius + proxy.m_penetrationPadding;
	dgVector size(collBox->m_size[0]);
	dgVector center(boxMatrix.UntransformVector(sphMatrix.m_posit));

	codeX = (center.m_x < -size.m_x) + (center.m_x > size.m_x) * 2;
	codeY = (center.m_y < -size.m_y) + (center.m_y > size.m_y) * 2;
	codeZ = (center.m_z < -size.m_z) + (center.m_z > size.m_z) * 2;
	code = codeZ * 9 + codeY * 3 + codeX;

	if (!code) {
		return CalculateHullToHullContacts(proxy);
	}

	dist = dgFloat32(0.0f);
	dgVector point(center);
	dgVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                dgFloat32(0.0f));

	switch (code) {
	case 2 * 9 + 1 * 3 + 2: {
		size.m_y *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 1 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		size.m_y *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 1 * 3 + 2: {
		size.m_y *= dgFloat32(-1.0f);
		size.m_z *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 1 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		size.m_y *= dgFloat32(-1.0f);
		size.m_z *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 2 * 3 + 2: {
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 2 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 2 * 3 + 2: {
		size.m_z *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 2 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		size.m_z *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 0 * 3 + 2: {
		normal.m_x = size.m_x - center.m_x;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 0 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 0 * 3 + 2: {
		size.m_z *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 0 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		size.m_z *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 0 * 9 + 2 * 3 + 2: {
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 0 * 9 + 2 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 0 * 9 + 1 * 3 + 2: {
		size.m_y *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 0 * 9 + 1 * 3 + 1: {
		size.m_x *= dgFloat32(-1.0f);
		size.m_y *= dgFloat32(-1.0f);
		normal.m_x = size.m_x - center.m_x;
		normal.m_y = size.m_y - center.m_y;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 2 * 3 + 0: {
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 2 * 3 + 0: {
		size.m_z *= dgFloat32(-1.0f);
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 2 * 9 + 1 * 3 + 0: {
		size.m_y *= dgFloat32(-1.0f);
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 1 * 9 + 1 * 3 + 0: {
		size.m_y *= dgFloat32(-1.0f);
		size.m_z *= dgFloat32(-1.0f);
		normal.m_y = size.m_y - center.m_y;
		normal.m_z = size.m_z - center.m_z;
		normal = normal.Scale(dgRsqrt(normal % normal));
		dist = normal % (size - center) - radius;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		point += normal.Scale(
		             radius + (dist - proxy.m_penetrationPadding) * dgFloat32(0.5f));
		break;
	}

	case 0 * 9 + 0 * 3 + 1: {
		dist = -((center.m_x + radius) + size.m_x);
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		normal.m_x = 1.0f;
		point.m_x = -size.m_x - (dist + proxy.m_penetrationPadding) * dgFloat32(0.5f);
		break;
	}

	case 0 * 9 + 1 * 3 + 0: {
		dist = -((center.m_y + radius) + size.m_y);
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		normal.m_y = dgFloat32(1.0f);
		point.m_y = -size.m_y - (dist + proxy.m_penetrationPadding) * dgFloat32(0.5f);
		break;
	}

	case 1 * 9 + 0 * 3 + 0: {
		dist = -((center.m_z + radius) + size.m_z);
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		normal.m_z = dgFloat32(1.0f);
		point.m_z = -size.m_z - (dist + proxy.m_penetrationPadding) * dgFloat32(0.5f);
		break;
	}

	case 0 * 9 + 0 * 3 + 2: {
		dist = (center.m_x - radius) - size.m_x;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		normal.m_x = dgFloat32(-1.0f);
		point.m_x = size.m_x + (dist + proxy.m_penetrationPadding) * dgFloat32(0.5f);
		break;
	}

	case 0 * 9 + 2 * 3 + 0: {
		dist = (center.m_y - radius) - size.m_y;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		normal.m_y = dgFloat32(-1.0f);
		point.m_y = size.m_y + (dist + proxy.m_penetrationPadding) * dgFloat32(0.5f);
		break;
	}

	case 2 * 9 + 0 * 3 + 0: {
		dist = (center.m_z - radius) - size.m_z;
		if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
			return 0;
		}
		normal.m_z = dgFloat32(-1.0f);
		point.m_z = size.m_z + (dist + proxy.m_penetrationPadding) * dgFloat32(0.5f);
		break;
	}

	default: {
		return 0;
	}
	}

	if (proxy.m_isTriggerVolume) {
		proxy.m_inTriggerVolume = 1;
		return 0;
	}

	dist = (dgAbsf(dist) - DG_IMPULSIVE_CONTACT_PENETRATION);
	if (dist < dgFloat32(0.0f)) {
		dist = dgFloat32(0.0f);
	}

	contactOut = proxy.m_contacts;
	contactOut[0].m_point = boxMatrix.TransformVector(point);
	contactOut[0].m_normal = boxMatrix.RotateVector(normal);
	contactOut[0].m_penetration = dist;
	contactOut[0].m_userId = 0;
	return 1;
}

/*
 dgInt32 dgWorld::CalculateBoxToBoxContacts (
 dgBody* box1,
 dgBody* box2,
 dgContactPoint* const contactOut) const
 {
 NEWTON_ASSERT (0);
 return 0;

 dgInt32 i;
 dgInt32 k;
 dgInt32 count1;
 dgInt32 count2;
 dgFloat32 d1;
 dgFloat32 d2;
 dgFloat32 min;
 dgFloat32 dist;
 dgFloat32 test;
 dgFloat32 minDist;
 dgPlane plane;
 dgVector shape1[16];
 dgVector shape2[16];
 dgCollisionBox* collision1;
 dgCollisionBox* collision2;



 NEWTON_ASSERT (box1->m_collision->IsType (m_boxType));
 NEWTON_ASSERT (box2->m_collision->IsType (m_boxType));

 const dgMatrix& matrix1 = box1->m_collisionWorldMatrix;
 const dgMatrix& matrix2 = box2->m_collisionWorldMatrix;

 collision1 = (dgCollisionBox*) box1->m_collision;
 collision2 = (dgCollisionBox*) box2->m_collision;

 const dgVector& size1 = collision1->m_size;
 const dgVector& size2 = collision2->m_size;

 minDist = dgFloat32 (-1.0e10f);

 dgMatrix mat12 (matrix1 * matrix2.Inverse ());
 for (i = 0; i < 3; i ++) {
 min = dgAbsf (mat12[0][i]) * size1[0] + dgAbsf (mat12[1][i]) * size1[1] + dgAbsf (mat12[2][i]) * size1[2];
 dist = dgAbsf (mat12[3][i]) - size2[i] - min;
 if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
 return 0;
 }
 if (dist > minDist) {
 minDist = dist;
 plane[0] = dgFloat32 (0.0f);
 plane[1] = dgFloat32 (0.0f);
 plane[2] = dgFloat32 (0.0f);
 plane[3] = - (size2[i] + dist * dgFloat32 (0.5f));

 plane[i] = dgFloat32 (1.0f);
 test = plane[3] + mat12[3][i] + min;
 if (test < dgFloat32 (0.0f)) {
 plane[i] = dgFloat32 (-1.0f);
 }
 plane = matrix2.TransformPlane (plane);
 }
 }

 // dgMatrix mat21 (matrix2 * matrix1.Inverse ());
 dgMatrix mat21 (mat12.Inverse ());
 for (i = 0; i < 3; i ++) {
 min = dgAbsf (mat21[0][i]) * size2[0] + dgAbsf (mat21[1][i]) * size2[1] + dgAbsf (mat21[2][i]) * size2[2];
 dist = dgAbsf (mat21[3][i]) - size1[i] - min;
 if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
 return 0;
 }
 if (dist > minDist) {
 minDist = dist;
 plane[0] = dgFloat32 (0.0f);
 plane[1] = dgFloat32 (0.0f);
 plane[2] = dgFloat32 (0.0f);
 plane[3] = - (size1[i] + dist * dgFloat32 (0.5f));

 plane[i] = dgFloat32 (1.0f);

 test = plane[3] + mat21[3][i] + min;
 if (test < dgFloat32 (0.0f)) {
 plane[i] = dgFloat32 (-1.0f);
 }
 plane = matrix1.TransformPlane (plane).Scale (dgFloat32 (-1.0f));
 }
 }

 for (k = 0; k < 3; k ++) {
 for (i = 0; i < 3; i ++) {
 dgVector normal (matrix1[k] * matrix2[i]);
 test = (normal % normal) ;
 if (test > dgFloat32(1.0e-6f)) {
 normal = normal.Scale (dgRsqrt (test));
 d2 = size2[0] * dgAbsf (matrix2[0] % normal) + size2[1] * dgAbsf (matrix2[1] % normal) + size2[2] * dgAbsf (matrix2[2] % normal);
 d1 = size1[0] * dgAbsf (matrix1[0] % normal) + size1[1] * dgAbsf (matrix1[1] % normal) + size1[2] * dgAbsf (matrix1[2] % normal);

 dgVector q (matrix2[3] - normal.Scale (d2));
 dgVector p (matrix1[3] + normal.Scale (d1));
 dist = (q - p) % normal;
 if (dist > (-DG_RESTING_CONTACT_PENETRATION)) {
 return 0;
 }

 dgVector q1 (matrix2[3] + normal.Scale (d2));
 dgVector p1 (matrix1[3] - normal.Scale (d1));
 test = (p1 - q1) % normal;
 if (test > (-DG_RESTING_CONTACT_PENETRATION)) {
 return 0;
 }

 if (test > dist) {
 dist = test;
 p = p1;
 q = q1;
 }

 if (dist > minDist) {
 minDist = dist;
 plane[0] = normal[0];
 plane[1] = normal[1];
 plane[2] = normal[2];
 plane[3] = - dgFloat32 (0.5f) * ((q + p) % normal);

 test = plane.Evalue (matrix1[3]);
 if (test < dgFloat32 (0.0f)) {
 plane.m_x *= dgFloat32 (-1.0f);
 plane.m_y *= dgFloat32 (-1.0f);
 plane.m_z *= dgFloat32 (-1.0f);
 plane.m_w *= dgFloat32 (-1.0f);
 }
 }
 }
 }
 }

 dgPlane plane1 (matrix1.UntransformPlane (plane));
 count1 = collision1->CalculatePlaneIntersection (plane1, shape1);
 if (!count1) {
 dgVector p1 (collision1->SupportVertex (plane1.Scale (dgFloat32 (-1.0f))));
 dgPlane plane (plane1, - (plane1 % p1) - DG_ROBUST_PLANE_CLIP);
 count1 = collision1->CalculatePlaneIntersection (plane, shape1);
 if (count1) {
 dgVector err (plane1.Scale (plane1.Evalue (shape1[0])));
 for (i = 0; i < count1; i ++) {
 shape1[i] -= err;
 }
 }
 }
 if (count1 == 0) {
 return 0;
 }

 dgPlane plane2 (matrix2.UntransformPlane (plane));
 count2 = collision2->CalculatePlaneIntersection (plane2, shape2);
 if (!count2) {
 dgVector p2 (collision2->SupportVertex (plane2));
 dgPlane plane (plane2, DG_ROBUST_PLANE_CLIP - (plane2 % p2));
 count2 = collision2->CalculatePlaneIntersection (plane, shape2);
 if (count2) {
 dgVector err (plane2.Scale (plane2.Evalue (shape2[0])));
 for (i = 0; i < count2; i ++) {
 shape2[i] -= err;
 }
 }
 }

 if (count2 == 0) {
 return 0;
 }

 NEWTON_ASSERT (count1 <= 6);
 NEWTON_ASSERT (count2 <= 6);
 matrix1.TransformTriplex (shape1, sizeof (dgVector), shape1, sizeof (dgVector), count1);
 matrix2.TransformTriplex (shape2, sizeof (dgVector), shape2, sizeof (dgVector), count2);

 minDist = (dgAbsf (minDist) - DG_IMPULSIVE_CONTACT_PENETRATION);
 if (minDist < dgFloat32 (0.0f)) {
 minDist = dgFloat32 (0.0f);
 }
 k = dgContactSolver::CalculateConvexShapeIntersection (plane, 0, minDist, count1, shape1, count2, shape2, contactOut);
 return k;

 }
 */

dgInt32 dgWorld::FilterPolygonEdgeContacts(dgInt32 count,
        dgContactPoint *const contact) const {
	if (count > 1) {
		dgInt32 faceCount = 0;
		dgInt32 j = count - 1;
		while (faceCount <= j) {
			while ((faceCount <= j) && !contact[faceCount].m_isEdgeContact) {
				faceCount++;
			}
			while ((faceCount <= j) && contact[j].m_isEdgeContact) {
				j--;
			}

			if (faceCount < j) {
				Swap(contact[faceCount], contact[j]);
			}
		}

		if (faceCount < count) {
			for (dgInt32 i = 0; i < faceCount; i++) {
				NEWTON_ASSERT(
				    (contact[i].m_isEdgeContact == 0) || (contact[i].m_isEdgeContact == 1));
				for (dgInt32 k = faceCount; k < count; k++) {
					dgFloat32 dist;
					dgVector distVector(contact[i].m_point - contact[k].m_point);
					dist = distVector % distVector;
					//                  if (dist < dgFloat32 (0.04f)) {
					if (dist < dgFloat32(1.e-2f)) {
						count--;
						contact[k] = contact[count];
						k--;
					}
				}
			}
		}

		for (dgInt32 i = 0; i < count - 1; i++) {
			NEWTON_ASSERT(
			    (contact[i].m_isEdgeContact == 0) || (contact[i].m_isEdgeContact == 1));
			for (dgInt32 k = i + 1; k < count; k++) {
				dgFloat32 dist;
				dgVector distVector(contact[i].m_point - contact[k].m_point);
				dist = distVector % distVector;
				//              if (dist < dgFloat32 (0.04f)) {
				//              if ((dist < dgFloat32 (0.001f)) || ((dist < dgFloat32 (0.01f)) && ((contact[i].m_normal % contact[k].m_normal) > dgFloat32 (0.86f)))) {
				if (dist < dgFloat32(1.e-3f)) {
					count--;
					contact[k] = contact[count];
					k--;
				}
			}
		}
	}

	return count;
}

/*
 dgInt32 dgWorld::FlattenContinueContacts (dgInt32 count, dgContactPoint* const contact, dgFloat32 size) const
 {
 dgInt32 tmp;
 dgInt8 mark[256];

 for (dgInt32 i = 0; i < count; i += 4) {
 ((dgInt32*) mark)[i] = 0;
 }

 size *= 2.0f;
 tmp = count;
 for (dgInt32 i = 0; i < tmp; i ++) {
 if (!mark[i]) {
 dgPlane plane (contact[i].m_normal, - (contact[i].m_point % contact[i].m_normal));
 for (dgInt32 j = i + 1; j < count ; j ++) {
 if (!mark[j]) {
 dgFloat32 dist;
 dist = plane.Evalue(contact[j].m_point);
 if (dgAbsf (dist) < dgFloat32 (0.01f)) {
 mark[j] = true;
 }
 }
 }

 dgVector testPoint;
 if (dgAbsf (plane.m_z) > dgFloat32 (0.577f)) {
 testPoint = dgVector (-plane.m_y, plane.m_z, dgFloat32 (0.0f), dgFloat32 (0.0f));
 } else {
 testPoint = dgVector (-plane.m_y, plane.m_x, dgFloat32 (0.0f), dgFloat32 (0.0f));
 }

 dgVector dir0 (plane * testPoint);
 NEWTON_ASSERT (dir0  % dir0  > dgFloat32 (1.0e-8f));
 dir0  = dir0 .Scale (dgRsqrt (dir0 % dir0) * size);
 dgVector dir1 (plane * dir0);

 contact[count] = contact[i];
 contact[count].m_point += dir0;
 count ++;
 contact[count] = contact[i];
 contact[count].m_point -= dir0;
 count ++;

 contact[count] = contact[i];
 contact[count].m_point += dir1;
 count ++;
 contact[count] = contact[i];
 contact[count].m_point -= dir1;
 count ++;
 }
 }

 return count;
 }
 */

// dgInt32 dgWorld::ClosestPoint (
//	dgBody* convexA,
//	dgBody* convexB,
//	dgTriplex& contactA,
//	dgTriplex& contactB,
//	dgTriplex& normalAB) const
dgInt32 dgWorld::ClosestPoint(dgCollisionParamProxy &proxy) const {
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));

	dgMatrix matrix(proxy.m_floatingMatrix * proxy.m_referenceMatrix.Inverse());
	proxy.m_localMatrixInv = &matrix;

	dgContactSolver mink(proxy);
	return mink.CalculateClosestPoints();
}

// *******************************************************************************************************
//
// convex contact calculation
//
// *******************************************************************************************************
dgInt32 dgWorld::CalculateHullToHullContactsSimd(
    dgCollisionParamProxy &proxy) const {
#ifdef DG_BUILD_SIMD_CODE
	dgFloat32 radiusA;
	dgFloat32 radiusB;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));

	dgMatrix matrix(
	    proxy.m_floatingMatrix.MultiplySimd(
	        proxy.m_referenceMatrix.InverseSimd()));
	proxy.m_localMatrixInv = &matrix;
	//  dgContactSolver mink (hull1, hull2, penetrationPadding);
	dgContactSolver mink(proxy);

	radiusA = proxy.m_referenceCollision->GetBoxMaxRadius();
	radiusB = proxy.m_floatingCollision->GetBoxMaxRadius();

	//  return mink.HullHullContactsSimd (0);
	if ((radiusA * dgFloat32(64.0f) < radiusB) || (radiusB * dgFloat32(64.0f) < radiusA)) {
		return mink.HullHullContactsLarge(0);
	} else {
		return mink.HullHullContactsSimd(0);
	}

#else
	return 0;
#endif
}

dgInt32 dgWorld::CalculateHullToHullContacts(dgCollisionParamProxy &proxy) const {
	dgFloat32 radiusA;
	dgFloat32 radiusB;
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));

	dgMatrix matrix(proxy.m_floatingMatrix * proxy.m_referenceMatrix.Inverse());
	proxy.m_localMatrixInv = &matrix;

	//  dgContactSolver mink (hull1, hull2, penetrationPadding);
	dgContactSolver mink(proxy);

	radiusA = proxy.m_referenceCollision->GetBoxMaxRadius();
	radiusB = proxy.m_floatingCollision->GetBoxMaxRadius();

#ifdef __USE_DOUBLE_PRECISION__
	return mink.HullHullContactsLarge(0);
#else
	if ((radiusA * dgFloat64(32.0f) < radiusB) || (radiusB * dgFloat64(32.0f) < radiusA)) {
		return mink.HullHullContactsLarge(0);
	} else {
		return mink.HullHullContacts(0);
	}
#endif
}

dgInt32 dgWorld::CalculateConvexToConvexContactsSimd(
    dgCollisionParamProxy &proxy) const {
#ifdef DG_BUILD_SIMD_CODE
	dgInt32 i;
	dgInt32 count;
	dgCollisionID id1;
	dgCollisionID id2;
	dgCollision *collision1;
	dgCollision *collision2;
	dgContactPoint *contactOut;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));

	count = 0;
	proxy.m_inTriggerVolume = 0;
	collision1 = proxy.m_referenceCollision;
	collision2 = proxy.m_floatingCollision;

	if (!(((dgCollisionConvex *)collision1)->m_vertexCount && ((dgCollisionConvex *)collision2)->m_vertexCount)) {
		return count;
	}

	NEWTON_ASSERT(collision1->GetCollisionPrimityType() != m_nullCollision);
	NEWTON_ASSERT(collision2->GetCollisionPrimityType() != m_nullCollision);

	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	if (proxy.m_continueCollision) {
		dgInt32 maxContaCount;
		dgFloat32 dist;
		dgFloat32 timestep;
		dgFloat32 distTravel;

		dgMatrix matrix(
		    proxy.m_floatingMatrix.MultiplySimd(
		        proxy.m_referenceMatrix.InverseSimd()));
		proxy.m_localMatrixInv = &matrix;
		dgContactSolver mink(proxy);

		timestep = proxy.m_timestep;
		mink.CalculateVelocitiesSimd(proxy.m_timestep);
		maxContaCount = GetMin(proxy.m_maxContacts, 16);
		count = mink.HullHullContinueContactsSimd(proxy.m_timestep,
		        proxy.m_contacts, 0, maxContaCount, proxy.m_unconditionalCast);

		if (count) {
			dist = GetMin(collision1->GetBoxMinRadius(),
			              collision2->GetBoxMinRadius());
			dist *= dist;
			distTravel = (mink.m_localRelVeloc % mink.m_localRelVeloc) * timestep * timestep;
			if (distTravel * dgFloat32(0.25f * 0.25f) > dist) {
				if (proxy.m_referenceBody->m_mass.m_w > dgFloat32(0.0f)) {
					dgGetUserLock();
					proxy.m_referenceBody->m_solverInContinueCollision = true;
					dgReleasedUserLock();
				}
				if (proxy.m_floatingBody->m_mass.m_w > dgFloat32(0.0f)) {
					dgGetUserLock();
					proxy.m_floatingBody->m_solverInContinueCollision = true;
					dgReleasedUserLock();
				}
			}
		}

	} else {
		id1 = collision1->GetCollisionPrimityType();
		id2 = collision2->GetCollisionPrimityType();
		NEWTON_ASSERT(id1 != m_nullCollision);
		NEWTON_ASSERT(id2 != m_nullCollision);

		switch (id1) {
		case m_sphereCollision: {
			switch (id2) {
			case m_sphereCollision: {
				count = CalculateSphereToSphereContacts(proxy);
				break;
			}

			case m_capsuleCollision: {
				dgCollisionParamProxy tmp(proxy.m_threadIndex);
				tmp.m_referenceBody = proxy.m_floatingBody;
				tmp.m_floatingBody = proxy.m_referenceBody;
				tmp.m_referenceCollision = proxy.m_floatingCollision;
				tmp.m_floatingCollision = proxy.m_referenceCollision;
				tmp.m_referenceMatrix = proxy.m_floatingMatrix;
				tmp.m_floatingMatrix = proxy.m_referenceMatrix;
				tmp.m_timestep = proxy.m_timestep;
				tmp.m_penetrationPadding = proxy.m_penetrationPadding;
				tmp.m_contacts = proxy.m_contacts;
				tmp.m_inTriggerVolume = 0;
				tmp.m_isTriggerVolume = proxy.m_isTriggerVolume;

				count = CalculateCapsuleToSphereContacts(tmp);
				for (dgInt32 i = 0; i < count; i++) {
					proxy.m_contacts[i].m_normal = tmp.m_contacts[0].m_normal.Scale(
					                                   dgFloat32(-1.0f));
				}
				proxy.m_inTriggerVolume = tmp.m_inTriggerVolume;
				break;
			}

			case m_boxCollision: {
				dgCollisionParamProxy tmp(proxy.m_threadIndex);

				tmp.m_referenceBody = proxy.m_floatingBody;
				tmp.m_floatingBody = proxy.m_referenceBody;
				tmp.m_referenceCollision = proxy.m_floatingCollision;
				tmp.m_floatingCollision = proxy.m_referenceCollision;
				tmp.m_referenceMatrix = proxy.m_floatingMatrix;
				tmp.m_floatingMatrix = proxy.m_referenceMatrix;
				tmp.m_timestep = proxy.m_timestep;
				tmp.m_penetrationPadding = proxy.m_penetrationPadding;
				tmp.m_contacts = proxy.m_contacts;
				tmp.m_maxContacts = proxy.m_maxContacts;
				tmp.m_inTriggerVolume = 0;
				tmp.m_isTriggerVolume = proxy.m_isTriggerVolume;

				count = CalculateBoxToSphereContacts(tmp);
				if (count) {
					proxy.m_contacts[0].m_normal = tmp.m_contacts[0].m_normal.Scale(
					                                   dgFloat32(-1.0f));
				}
				proxy.m_inTriggerVolume = tmp.m_inTriggerVolume;
				break;
			}

			default: {
				count = CalculateHullToHullContactsSimd(proxy);
				break;
			}
			}

			break;
		}

		case m_capsuleCollision: {
			switch (id2) {
			case m_sphereCollision: {
				count = CalculateCapsuleToSphereContacts(proxy);
				break;
			}

			case m_capsuleCollision: {
				count = CalculateCapsuleToCapsuleContacts(proxy);
				break;
			}

			default: {
				count = CalculateHullToHullContactsSimd(proxy);
				break;
			}
			}
			break;
		}

		case m_boxCollision: {
			switch (id2) {
			case m_sphereCollision: {
				count = CalculateBoxToSphereContacts(proxy);
				break;
			}

			default: {
				count = CalculateHullToHullContactsSimd(proxy);
				break;
			}
			}
			break;
		}

		default: {
			count = CalculateHullToHullContactsSimd(proxy);
			break;
		}
		}

		if (count) {
			proxy.m_timestep = dgFloat32(0.0f);
		}
	}

	contactOut = proxy.m_contacts;
	for (i = 0; i < count; i++) {
		contactOut[i].m_body0 = proxy.m_referenceBody;
		contactOut[i].m_body1 = proxy.m_floatingBody;
		contactOut[i].m_collision0 = collision1;
		contactOut[i].m_collision1 = collision2;
	}
	return count;

#else
	return 0;
#endif
}

dgInt32 dgWorld::CalculateConvexToConvexContacts(
    dgCollisionParamProxy &proxy) const {
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));

	proxy.m_inTriggerVolume = 0;

	dgInt32 count = 0;
	dgCollision *const collision1 = proxy.m_referenceCollision;
	dgCollision *const collision2 = proxy.m_floatingCollision;

	if (!(((dgCollisionConvex *)collision1)->m_vertexCount && ((dgCollisionConvex *)collision2)->m_vertexCount)) {
		return count;
	}

	NEWTON_ASSERT(collision1->GetCollisionPrimityType() != m_nullCollision);
	NEWTON_ASSERT(collision2->GetCollisionPrimityType() != m_nullCollision);

	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	if (proxy.m_continueCollision) {
		dgInt32 maxContaCount;
		dgFloat32 dist;
		dgFloat32 timestep;
		dgFloat32 distTravel;

		dgMatrix matrix(proxy.m_floatingMatrix * proxy.m_referenceMatrix.Inverse());
		proxy.m_localMatrixInv = &matrix;
		dgContactSolver mink(proxy);

		timestep = proxy.m_timestep;
		mink.CalculateVelocities(proxy.m_timestep);

		maxContaCount = GetMin(proxy.m_maxContacts, dgInt32(16));
		count = mink.HullHullContinueContacts(proxy.m_timestep, proxy.m_contacts, 0,
		                                      maxContaCount, proxy.m_unconditionalCast);

		if (count) {
			dist = GetMin(collision1->GetBoxMinRadius(),
			              collision2->GetBoxMinRadius());
			dist *= dist;
			distTravel = (mink.m_localRelVeloc % mink.m_localRelVeloc) * timestep * timestep;
			if (distTravel * dgFloat32(0.25f * 0.25f) > dist) {
				if (proxy.m_referenceBody->m_mass.m_w > dgFloat32(0.0f)) {
					dgGetUserLock();
					proxy.m_referenceBody->m_solverInContinueCollision = true;
					dgReleasedUserLock();
				}
				if (proxy.m_floatingBody->m_mass.m_w > dgFloat32(0.0f)) {
					dgGetUserLock();
					proxy.m_floatingBody->m_solverInContinueCollision = true;
					dgReleasedUserLock();
				}
			}
		}

	} else {
		dgCollisionID id1 = collision1->GetCollisionPrimityType();
		dgCollisionID id2 = collision2->GetCollisionPrimityType();
		NEWTON_ASSERT(id1 != m_nullCollision);
		NEWTON_ASSERT(id2 != m_nullCollision);

		switch (id1) {
		case m_sphereCollision: {
			switch (id2) {
			case m_sphereCollision: {
				count = CalculateSphereToSphereContacts(proxy);
				break;
			}

			case m_capsuleCollision: {
				dgCollisionParamProxy tmp(proxy.m_threadIndex);
				tmp.m_referenceBody = proxy.m_floatingBody;
				tmp.m_floatingBody = proxy.m_referenceBody;
				tmp.m_referenceCollision = proxy.m_floatingCollision;
				tmp.m_floatingCollision = proxy.m_referenceCollision;
				tmp.m_referenceMatrix = proxy.m_floatingMatrix;
				tmp.m_floatingMatrix = proxy.m_referenceMatrix;
				tmp.m_timestep = proxy.m_timestep;
				tmp.m_penetrationPadding = proxy.m_penetrationPadding;
				tmp.m_contacts = proxy.m_contacts;
				tmp.m_inTriggerVolume = 0;
				tmp.m_isTriggerVolume = proxy.m_isTriggerVolume;

				count = CalculateCapsuleToSphereContacts(tmp);
				for (dgInt32 i = 0; i < count; i++) {
					proxy.m_contacts[i].m_normal = tmp.m_contacts[0].m_normal.Scale(
					                                   dgFloat32(-1.0f));
				}
				proxy.m_inTriggerVolume = tmp.m_inTriggerVolume;
				break;
			}

			case m_boxCollision: {
				dgCollisionParamProxy tmp(proxy.m_threadIndex);

				tmp.m_referenceBody = proxy.m_floatingBody;
				tmp.m_floatingBody = proxy.m_referenceBody;
				tmp.m_referenceCollision = proxy.m_floatingCollision;
				tmp.m_floatingCollision = proxy.m_referenceCollision;
				tmp.m_referenceMatrix = proxy.m_floatingMatrix;
				tmp.m_floatingMatrix = proxy.m_referenceMatrix;
				tmp.m_timestep = proxy.m_timestep;
				tmp.m_penetrationPadding = proxy.m_penetrationPadding;
				tmp.m_contacts = proxy.m_contacts;
				tmp.m_inTriggerVolume = 0;
				tmp.m_maxContacts = proxy.m_maxContacts;
				tmp.m_isTriggerVolume = proxy.m_isTriggerVolume;

				count = CalculateBoxToSphereContacts(tmp);
				if (count) {
					proxy.m_contacts[0].m_normal = tmp.m_contacts[0].m_normal.Scale(
					                                   dgFloat32(-1.0f));
				}
				proxy.m_inTriggerVolume = tmp.m_inTriggerVolume;

				break;
			}

			default: {
				count = CalculateHullToHullContacts(proxy);
				break;
			}
			}

			break;
		}

		case m_capsuleCollision: {
			switch (id2) {
			case m_sphereCollision: {
				count = CalculateCapsuleToSphereContacts(proxy);
				break;
			}

			case m_capsuleCollision: {
				count = CalculateCapsuleToCapsuleContacts(proxy);
				break;
			}

			default: {
				count = CalculateHullToHullContacts(proxy);
				break;
			}
			}
			break;
		}

		case m_boxCollision: {
			switch (id2) {
			case m_sphereCollision: {
				count = CalculateBoxToSphereContacts(proxy);
				break;
			}

			// case m_boxCollision:
			//{
			// count = CalculateBoxToBoxContacts (body1, body2, contactOut);
			// break;
			// }

			default: {
				count = CalculateHullToHullContacts(proxy);
				break;
			}
			}
			break;
		}

		default: {
			count = CalculateHullToHullContacts(proxy);
			break;
		}
		}

		if (count) {
			proxy.m_timestep = dgFloat32(0.0f);
		}
	}

	dgContactPoint *const contactOut = proxy.m_contacts;
	for (dgInt32 i = 0; i < count; i++) {
		contactOut[i].m_body0 = proxy.m_referenceBody;
		contactOut[i].m_body1 = proxy.m_floatingBody;
		contactOut[i].m_collision0 = collision1;
		contactOut[i].m_collision1 = collision2;
	}
	return count;
}

// *******************************************************************************************************
//
// non convex contact calculation
//
// *******************************************************************************************************
dgInt32 dgWorld::CalculatePolySoupToSphereContactsDescrete(
    dgCollisionParamProxy &proxy) const {
	dgInt32 count;
	dgInt32 count1;
	dgInt32 thread;
	dgInt32 countleft;
	dgInt32 indexCount;
	dgInt32 reduceContactCountLimit;
	dgFloat32 radius;
	dgCollisionSphere *sphere;
	dgCollisionMesh *polysoup;
	dgCollisionMesh::dgCollisionConvexPolygon *polygon;
	dgVector point;

	count = 0;
	NEWTON_ASSERT(proxy.m_referenceCollision->IsType(dgCollision::dgCollisionSphere_RTTI));
	NEWTON_ASSERT(proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	sphere = (dgCollisionSphere *)proxy.m_referenceCollision;
	polysoup = (dgCollisionMesh *)proxy.m_floatingCollision;

	const dgMatrix &sphMatrix = proxy.m_referenceMatrix;
	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;

	radius = sphere->m_radius + proxy.m_penetrationPadding;
	dgVector center(soupMatrix.UntransformVector(sphMatrix.m_posit));

	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;
	thread = data.m_threadNumber;

	const dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;

	NEWTON_ASSERT(idArray);
	polygon = polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	dgContactPoint *const contactOut = proxy.m_contacts;
	reduceContactCountLimit = 0;
	countleft = proxy.m_maxContacts;

	indexCount = 0;
	NEWTON_ASSERT(data.m_faceCount);
	// strideInBytes = data.m_vertexStrideInBytes;
	for (dgInt32 i = 0; (i < data.m_faceCount) && (countleft > 0); i++) {
		polygon->m_count = data.m_faceIndexCount[i];
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[i];
			polygon->m_adjacentNormalIndex =
			    (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		bool isEdge = false;
		if (polygon->PointToPolygonDistance(center, radius, point, isEdge)) {
			dgVector dp(center - point);
			dgFloat32 dist2 = dp % dp;
			if (dist2 > dgFloat32(0.0f)) {
				dgFloat32 side;
				dgFloat32 dist2Inv;
				NEWTON_ASSERT(dist2 > dgFloat32(0.0f));
				dist2Inv = dgRsqrt(dist2);
				side = dist2 * dist2Inv - radius;
				if (side < (-DG_RESTING_CONTACT_PENETRATION)) {
					dgVector normal(dp.Scale(dist2Inv));

					NEWTON_ASSERT(dgAbsf(normal % normal - 1.0f) < dgFloat32(1.0e-5f));
					contactOut[count].m_point = soupMatrix.TransformVector(
					                                center - normal.Scale(radius + side * dgFloat32(0.5f)));
					contactOut[count].m_normal = soupMatrix.RotateVector(normal);
					contactOut[count].m_userId = idArray[i];
					contactOut[count].m_isEdgeContact = isEdge ? 1 : 0;

					side = (dgAbsf(side) - DG_IMPULSIVE_CONTACT_PENETRATION);
					if (side < dgFloat32(0.0f)) {
						side = dgFloat32(0.0f);
					}
					contactOut[count].m_penetration = side;

					dgVector prevNormal(contactOut[count].m_normal);
					// pass
					count1 = polygon->ClipContacts(1, &contactOut[count], soupMatrix);
					if ((prevNormal % contactOut[count].m_normal) < dgFloat32(0.9999f)) {
						contactOut[count].m_point = soupMatrix.TransformVector(center) - contactOut[count].m_normal.Scale(
						                                radius + side * dgFloat32(0.5f));
					}

					count += count1;
					countleft -= count1;
					reduceContactCountLimit += count;
					if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
						count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2,
						                       dgFloat32(1.0e-2f));
						countleft = proxy.m_maxContacts - count;
						reduceContactCountLimit = 0;
					}
				}
			}
		}
		indexCount += data.m_faceIndexCount[i];
	}

	count = FilterPolygonEdgeContacts(count, contactOut);
	return count;
}

dgInt32 dgWorld::CalculatePolySoupToElipseContactsDescrete(
    dgCollisionParamProxy &proxy) const {
	dgInt32 count;
	dgInt32 count1;
	dgInt32 thread;
	dgInt32 countleft;
	dgInt32 indexCount;
	dgInt32 reduceContactCountLimit;
	dgFloat32 radius;
	dgCollisionEllipse *sphere;
	dgCollisionMesh *polysoup;
	dgCollisionMesh::dgCollisionConvexPolygon *polygon;
	dgVector point;

	count = 0;
	NEWTON_ASSERT(proxy.m_referenceCollision->IsType(dgCollision::dgCollisionEllipse_RTTI));
	NEWTON_ASSERT(proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	sphere = (dgCollisionEllipse *)proxy.m_referenceCollision;
	polysoup = (dgCollisionMesh *)proxy.m_floatingCollision;

	const dgMatrix &sphMatrix = proxy.m_referenceMatrix;
	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;
	const dgMatrix &matrix = *proxy.m_localMatrixInv;
	const dgVector &scale = sphere->m_scale;
	const dgVector &invScale = sphere->m_invScale;

	radius = sphere->m_radius + proxy.m_penetrationPadding;
	//  dgVector center (soupMatrix.UntransformVector (sphMatrix.m_posit));

	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;
	thread = data.m_threadNumber;

	const dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;

	NEWTON_ASSERT(idArray);
	polygon = polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	dgContactPoint *const contactOut = proxy.m_contacts;
	reduceContactCountLimit = 0;
	countleft = proxy.m_maxContacts;

	indexCount = 0;
	NEWTON_ASSERT(data.m_faceCount);
	// strideInBytes = data.m_vertexStrideInBytes;
	for (dgInt32 i = 0; (i < data.m_faceCount) && (countleft > 0); i++) {
		polygon->m_count = data.m_faceIndexCount[i];
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[i];
			polygon->m_adjacentNormalIndex =
			    (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		bool isEdge = false;
		if (polygon->DistanceToOrigen(matrix, invScale, radius, point, isEdge)) {
			dgFloat32 dist2 = point % point;
			if (dist2 > dgFloat32(0.0f)) {
				dgFloat32 side;
				dgFloat32 contactDist;
				dgFloat32 surfaceDist;

				dgVector contact(scale.CompProduct(point));
				dgVector surfaceContact(
				    scale.CompProduct(point.Scale(radius * dgRsqrt(dist2))));
				contactDist = dgSqrt(contact % contact);
				surfaceDist = dgSqrt(surfaceContact % surfaceContact);

				side = contactDist - surfaceDist;
				if (side < (-DG_RESTING_CONTACT_PENETRATION)) {
					dgVector normal(invScale.CompProduct(point.Scale(dgRsqrt(dist2))));
					normal = normal.Scale(-dgRsqrt(normal % normal));

					dgVector midPoint(contact + surfaceContact);
					contactOut[count].m_point = sphMatrix.TransformVector(
					                                midPoint.Scale(dgFloat32(0.5f)));
					contactOut[count].m_normal = sphMatrix.RotateVector(normal);
					contactOut[count].m_userId = idArray[i];
					contactOut[count].m_isEdgeContact = isEdge ? 1 : 0;

					side = (dgAbsf(side) - DG_IMPULSIVE_CONTACT_PENETRATION);
					if (side < dgFloat32(0.0f)) {
						side = dgFloat32(0.0f);
					}
					contactOut[count].m_penetration = side;

					dgVector prevNormal(contactOut[count].m_normal);
					// pass
					count1 = polygon->ClipContacts(1, &contactOut[count], soupMatrix);
					if ((prevNormal % contactOut[count].m_normal) < dgFloat32(0.9999f)) {
						contactOut[count].m_normal = soupMatrix.RotateVector(
						                                 polygon->m_normal);
					}

					count += count1;
					countleft -= count1;
					reduceContactCountLimit += count;
					if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
						count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2,
						                       dgFloat32(1.0e-2f));
						countleft = proxy.m_maxContacts - count;
						reduceContactCountLimit = 0;
					}
				}
			}
		}
		indexCount += data.m_faceIndexCount[i];
	}

	count = FilterPolygonEdgeContacts(count, contactOut);
	return count;
}

/*
dgInt32 dgWorld::CalculatePolySoupToSphereContactsContinue (dgCollisionParamProxy& proxy) const
{
  dgInt32 count;
  dgInt32 count1;
  dgInt32 thread;
  dgInt32 countleft;
  dgInt32 indexCount;
  dgInt32 reduceContactCountLimit;
  dgFloat32 minTime;
  dgFloat32 radius;
  dgInt32* indexArray;
  dgInt32* idArray;
  dgCollisionSphere *sphere;
  dgContactPoint* contactOut;
  dgCollisionMesh *polysoup;
  dgCollisionMesh::dgCollisionConvexPolygon* polygon;

  count = 0;

  NEWTON_ASSERT(proxy.m_referenceCollision->IsType (dgCollision::dgCollisionSphere_RTTI));
  NEWTON_ASSERT(proxy.m_floatingCollision->IsType (dgCollision::dgCollisionMesh_RTTI));

  sphere = (dgCollisionSphere*) proxy.m_referenceCollision;
  polysoup = (dgCollisionMesh *) proxy.m_floatingCollision;

  const dgMatrix& sphMatrix = proxy.m_referenceMatrix;
  const dgMatrix& soupMatrix = proxy.m_floatingMatrix;

  radius = sphere->m_radius + proxy.m_penetrationPadding;
  dgVector center(soupMatrix.UntransformVector(sphMatrix.m_posit));
  dgVector veloc(soupMatrix.UnrotateVector(spheBody->m_veloc));

  const dgPolygonMeshDesc& data = *proxy.m_polyMeshData;
  thread = data.m_threadNumber;

  idArray = (dgInt32*) data.m_userAttribute;
  indexArray = (dgInt32*) data.m_faceVertexIndex;

  NEWTON_ASSERT(idArray);
  polygon = polysoup->m_polygon[thread];
  polygon->m_vertex = data.m_vertex;
  polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

  contactOut = proxy.m_contacts;
  reduceContactCountLimit = 0;
  countleft = proxy.m_maxContacts;

  NEWTON_ASSERT(data.m_faceCount);
  //strideInBytes = data.m_vertexStrideInBytes;

  indexCount = 0;
  minTime = proxy.m_timestep + dgFloat32(1.0e-5f);
  for (dgInt32 i = 0; (i < data.m_faceCount) && (countleft > 0); i++)
  {

    polygon->m_count = data.m_faceIndexCount[i];
    polygon->m_index = &indexArray[indexCount];

    if (data.m_faceNormalIndex)
    {
      polygon->m_normalIndex = data.m_faceNormalIndex[i];
      polygon->m_adjacentNormalIndex =
          (dgInt32*) &data.m_faceAdjencentEdgeNormal[indexCount];
    }
    else
    {
      polygon->m_normalIndex = 0;
      polygon->m_adjacentNormalIndex = NULL;
    }

    dgContactPoint contact;
    dgFloat32 timestep = polygon->MovingPointToPolygonContact(center, veloc,
        radius, contact);
    if (timestep >= dgFloat32(0.0f))
    {
      if (timestep <= minTime)
      {
        minTime = timestep + dgFloat32(1.0e-5f);

        NEWTON_ASSERT (dgAbsf (contact.m_normal % contact.m_normal - 1.0f) < dgFloat32 (1.0e-5f));
        contactOut[count].m_point = soupMatrix.TransformVector (center - contact.m_normal.Scale (radius));
        contactOut[count].m_normal = soupMatrix.RotateVector (contact.m_normal);
        contactOut[count].m_userId = idArray[i];
        contactOut[count].m_penetration = contact.m_penetration;
        contactOut[count].m_isEdgeContact = contact.m_isEdgeContact;
        contactOut[count].m_point.m_w = timestep;
        // pass
        //count1 = polygon->ClipContacts (1, &contactOut[count], soupMatrix);
        count1 = 1;
        count += count1;
        countleft -= count1;
        reduceContactCountLimit += count;
        if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
          count = ReduceContacts (count, contactOut, proxy.m_maxContacts >> 2, dgFloat32 (1.0e-2f));
          countleft = proxy.m_maxContacts - count;
          reduceContactCountLimit = 0;
        }
      }
    }
    //indexArray += data.m_faceIndexCount[i];
    indexCount += data.m_faceIndexCount[i];
  }

  if (count >= 1) {
    minTime = contactOut[0].m_point.m_w;
    for (dgInt32 j = 1; j < count; j ++) {
      minTime = GetMin(minTime, contactOut[j].m_point.m_w);
    }

    minTime += dgFloat32 (1.0e-5f);
    for (dgInt32 j = 0; j < count; j ++) {
      if (contactOut[j].m_point.m_w > minTime) {
        contactOut[j] = contactOut[count - 1];
        count --;
        j --;
      }
    }
    if (count > 1) {
      count = FilterPolygonEdgeContacts (count, contactOut);
    }
  }

  proxy.m_timestep = minTime;
  return count;
}
*/

dgInt32 dgWorld::CalculatePolySoupToSphereContactsContinue(dgCollisionParamProxy &proxy) const {
	dgInt32 count = 0;

	NEWTON_ASSERT(proxy.m_referenceCollision->IsType(dgCollision::dgCollisionSphere_RTTI));
	NEWTON_ASSERT(proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	dgBody *const spheBody = proxy.m_referenceBody;
	//  dgBody* const soupBody = proxy.m_floatingBody;
	dgCollisionSphere *const sphere = (dgCollisionSphere *)proxy.m_referenceCollision;
	dgCollisionMesh *const polysoup = (dgCollisionMesh *)proxy.m_floatingCollision;

	const dgMatrix &sphMatrix = proxy.m_referenceMatrix;
	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;

	dgFloat32 radius = sphere->m_radius + proxy.m_penetrationPadding;
	dgVector center(soupMatrix.UntransformVector(sphMatrix.m_posit));
	dgVector veloc(soupMatrix.UnrotateVector(spheBody->m_veloc));

	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;
	dgInt32 thread = data.m_threadNumber;

	dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;

	NEWTON_ASSERT(idArray);
	dgCollisionMesh::dgCollisionConvexPolygon *const polygon = polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	dgContactPoint *const contactOut = proxy.m_contacts;
	dgInt32 reduceContactCountLimit = 0;
	dgInt32 countleft = proxy.m_maxContacts;

	NEWTON_ASSERT(data.m_faceCount);
	// strideInBytes = data.m_vertexStrideInBytes;

	dgInt32 indexCount = 0;
	dgFloat32 minTime = proxy.m_timestep + dgFloat32(1.0e-5f);
	for (dgInt32 i = 0; (i < data.m_faceCount) && (countleft > 0); i++) {

		polygon->m_count = data.m_faceIndexCount[i];
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[i];
			polygon->m_adjacentNormalIndex = (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		dgContactPoint contact;
		dgFloat32 timestep = polygon->MovingPointToPolygonContact(center, veloc, radius, contact);
		if (timestep >= dgFloat32(0.0f)) {
			if (timestep <= minTime) {
				minTime = timestep + dgFloat32(1.0e-5f);

				NEWTON_ASSERT(dgAbsf(contact.m_normal % contact.m_normal - 1.0f) < dgFloat32(1.0e-5f));
				contactOut[count].m_point = soupMatrix.TransformVector(center - contact.m_normal.Scale(radius));
				contactOut[count].m_normal = soupMatrix.RotateVector(contact.m_normal);
				contactOut[count].m_userId = idArray[i];
				contactOut[count].m_penetration = contact.m_penetration;
				contactOut[count].m_isEdgeContact = contact.m_isEdgeContact;
				contactOut[count].m_point.m_w = timestep;
				// pass
				// dgInt32 count1 = polygon->ClipContacts (1, &contactOut[count], soupMatrix);
				dgInt32 count1 = 1;
				count += count1;
				countleft -= count1;
				reduceContactCountLimit += count;
				if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
					count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2, dgFloat32(1.0e-2f));
					countleft = proxy.m_maxContacts - count;
					reduceContactCountLimit = 0;
				}
			}
		}
		// indexArray += data.m_faceIndexCount[i];
		indexCount += data.m_faceIndexCount[i];
	}

	if (count > 1) {
		count = FilterPolygonEdgeContacts(count, contactOut);
		if (count > 1) {
			minTime = contactOut[0].m_point.m_w;
			for (dgInt32 j = 1; j < count; j++) {
				minTime = GetMin(minTime, contactOut[j].m_point.m_w);
			}

			minTime *= dgFloat32(1.000001f);
			for (dgInt32 j = 0; j < count; j++) {
				if (contactOut[j].m_point.m_w > minTime) {
					contactOut[j] = contactOut[count - 1];
					count--;
					j--;
				}
			}
		}
	}

	proxy.m_timestep = minTime;
	return count;
}

dgInt32 dgWorld::CalculatePolySoupToHullContactsDescreteSimd(dgCollisionParamProxy &proxy) const {

#ifdef DG_BUILD_SIMD_CODE
	dgInt32 count;
	dgInt32 count1;
	dgInt32 faceId;
	dgInt32 thread;
	dgInt32 countleft;
	dgInt32 indexCount;
	dgInt32 reduceContactCountLimit;
	dgCollisionBoundPlaneCache planeCache;

	count = 0;
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	//  hullBody = proxy.m_referenceBody;
	//  soupBody = proxy.m_floatingBody;
	dgCollisionConvex *const collision =
	    (dgCollisionConvex *)proxy.m_referenceCollision;
	dgCollisionMesh *const polysoup =
	    (dgCollisionMesh *)proxy.m_floatingCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(polysoup->IsType(dgCollision::dgCollisionMesh_RTTI));
	//  NEWTON_ASSERT (polysoup == soupBody->m_collision);

	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;
	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;
	thread = data.m_threadNumber;

	dgFloat32 *const faceSize = data.m_faceMaxSize;
	dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;
	NEWTON_ASSERT(data.m_faceCount);
	NEWTON_ASSERT(idArray);

	dgCollisionMesh::dgCollisionConvexPolygon *const polygon =
	    polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	reduceContactCountLimit = 0;
	countleft = proxy.m_maxContacts;
	proxy.m_floatingCollision = polygon;

	indexCount = 0;
	dgContactPoint *const contactOut = proxy.m_contacts;
	dgContactSolver mink(proxy);
	dgFloat32 convexSphapeSize = mink.GetShapeClipSize(collision);

	//  for (j = 0; j < data.m_faceCount; j ++) {
	for (dgInt32 j = 0; (j < data.m_faceCount) && (countleft > 0); j++) {
		polygon->m_count = data.m_faceIndexCount[j];
		polygon->m_index = indexArray;
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[j];
			polygon->m_adjacentNormalIndex =
			    (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		if (polygon->QuickTestSimd(collision, mink.m_matrix)) {
			if ((data.m_faceCount < 8) || collision->OOBBTest(mink.m_matrix, polygon, &planeCache)) {
				//          if (collision->OOBBTest (mink.m_matrix, polygon, &planeCache))  {

				if (faceSize && (faceSize[j] > convexSphapeSize)) {
					polygon->BeamClippingSimd(collision, mink.m_matrix, convexSphapeSize);
				}

				faceId = idArray[j];
				proxy.m_maxContacts = countleft;
				proxy.m_contacts = &contactOut[count];

				count1 = mink.HullHullContactsSimd(faceId);
				if (count1) {
					count1 = polygon->ClipContacts(count1, &contactOut[count],
					                               soupMatrix);

					count += count1;
					countleft -= count1;
					reduceContactCountLimit += count1;
					if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
						count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2,
						                       dgFloat32(1.0e-2f));
						countleft = proxy.m_maxContacts - count;
						reduceContactCountLimit = 0;
					}
				}
			}
		}
		indexCount += data.m_faceIndexCount[j];
	}

	proxy.m_contacts = contactOut;
	count = FilterPolygonEdgeContacts(count, contactOut);

	// restore the pointer
	proxy.m_floatingCollision = polysoup;
	return count;
#else
	return 0;
#endif
}

dgInt32 dgWorld::CalculatePolySoupToHullContactsDescrete(
    dgCollisionParamProxy &proxy) const {
	dgInt32 count;
	dgInt32 count1;
	dgInt32 faceId;
	dgInt32 thread;
	dgInt32 countleft;
	dgInt32 indexCount;
	dgInt32 reduceContactCountLimit;
	dgCollisionBoundPlaneCache planeCache;

	count = 0;
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	dgCollisionConvex *const collision =
	    (dgCollisionConvex *)proxy.m_referenceCollision;
	dgCollisionMesh *const polysoup =
	    (dgCollisionMesh *)proxy.m_floatingCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(polysoup->IsType(dgCollision::dgCollisionMesh_RTTI));

	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;
	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;
	thread = data.m_threadNumber;

	NEWTON_ASSERT(data.m_faceCount);
	dgFloat32 *const faceSize = data.m_faceMaxSize;
	dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;

	NEWTON_ASSERT(idArray);

	dgCollisionMesh::dgCollisionConvexPolygon *const polygon =
	    polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	reduceContactCountLimit = 0;
	countleft = proxy.m_maxContacts;

	indexCount = 0;
	proxy.m_floatingCollision = polygon;
	dgContactPoint *const contactOut = proxy.m_contacts;
	dgContactSolver mink(proxy);
	dgFloat32 convexSphapeSize = mink.GetShapeClipSize(collision);

	for (dgInt32 j = 0; (j < data.m_faceCount) && (countleft > 0); j++) {
		polygon->m_count = data.m_faceIndexCount[j];
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[j];
			polygon->m_adjacentNormalIndex =
			    (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		if (polygon->QuickTest(collision, mink.m_matrix)) {
			if ((data.m_faceCount < 8) || collision->OOBBTest(mink.m_matrix, polygon, &planeCache)) {

				if (faceSize && (faceSize[j] > convexSphapeSize)) {
					polygon->BeamClipping(collision, mink.m_matrix, convexSphapeSize);
				}

				faceId = idArray[j];
				proxy.m_maxContacts = countleft;
				proxy.m_contacts = &contactOut[count];
				count1 = mink.HullHullContacts(faceId);

				if (count1) {
					count1 = polygon->ClipContacts(count1, &contactOut[count],
					                               soupMatrix);
					count += count1;
					countleft -= count1;
					reduceContactCountLimit += count1;
					if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
						count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2,
						                       dgFloat32(1.0e-2f));
						countleft = proxy.m_maxContacts - count;
						reduceContactCountLimit = 0;
					}
				}
			}
		}
		indexCount += data.m_faceIndexCount[j];
	}

	proxy.m_contacts = contactOut;
	count = FilterPolygonEdgeContacts(count, contactOut);

	// restore the pointer
	proxy.m_floatingCollision = polysoup;
	return count;
}

dgInt32 dgWorld::CalculateConvexToNonConvexContactsContinueSimd(
    dgCollisionParamProxy &proxy) const {
#ifdef DG_BUILD_SIMD_CODE

	dgInt32 count;
	dgInt32 thread;
	dgInt32 countleft;
	dgInt32 indexCount;
	dgInt32 reduceContactCountLimit;
	//  dgInt32* idArray;
	//  dgInt32* indexArray;
	dgFloat32 minTime;
	dgFloat32 timestep;
	dgFloat32 oldTimeStep;
	//  dgBody* hullBody;
	//  dgBody* soupBody;
	//  dgContactPoint* contactOut;
	//  dgCollisionConvex* collision;
	//  dgCollisionMesh *polysoup;
	//  dgCollisionMesh::dgCollisionConvexPolygon* polygon;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	//  dgBody* const hullBody = proxy.m_referenceBody;
	//  dgBody* const soupBody = proxy.m_floatingBody;

	dgCollisionConvex *const collision =
	    (dgCollisionConvex *)proxy.m_referenceCollision;
	dgCollisionMesh *const polysoup =
	    (dgCollisionMesh *)proxy.m_floatingCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(polysoup->IsType(dgCollision::dgCollisionMesh_RTTI));
	//  NEWTON_ASSERT (polysoup == soupBody->m_collision);
	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;
	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;

	thread = data.m_threadNumber;
	count = 0;
	dgContactPoint *const contactOut = proxy.m_contacts;
	NEWTON_ASSERT(proxy.m_timestep <= dgFloat32(1.0f));
	NEWTON_ASSERT(proxy.m_timestep >= dgFloat32(0.0f));

	//  timestep = GetMin (proxy.m_timestep, dgFloat32 (1.0f));
	timestep = proxy.m_timestep;
	NEWTON_ASSERT(timestep >= dgFloat32(0.0f));
	NEWTON_ASSERT(timestep <= dgFloat32(1.0f));
	// hack Omega to 0.5
	// hullBody->SetOmega (hullBody->GetOmega().Scale(dgFloat32 (0.9f)));
	NEWTON_ASSERT(data.m_faceCount);

	dgFloat32 *const faceSize = data.m_faceMaxSize;
	dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;

	dgCollisionMesh::dgCollisionConvexPolygon *const polygon =
	    polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	//  countleft = maxContacts;
	reduceContactCountLimit = 0;
	countleft = proxy.m_maxContacts;

	indexCount = 0;
	dgContactSolver mink(proxy, polygon);
	mink.CalculateVelocitiesSimd(timestep);
	minTime = timestep;
	dgFloat32 convexSphapeSize = mink.GetShapeClipSize(collision);

	//  for (dgInt32 j = 0; j < data.m_faceCount; j ++) {
	for (dgInt32 j = 0; (j < data.m_faceCount) && (countleft > 0); j++) {
		polygon->m_count = data.m_faceIndexCount[j];
		//      polygon->m_index = indexArray;
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[j];
			polygon->m_adjacentNormalIndex =
			    (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		if (polygon->QuickTestContinueSimd(collision, mink.m_matrix)) {
			dgInt32 count1;
			dgInt32 faceId;
			dgFloat32 tmpTimestep;

			faceId = idArray[j];
			oldTimeStep = timestep;
			tmpTimestep = timestep;

			if (faceSize && (faceSize[j] > convexSphapeSize)) {
				polygon->BeamClippingSimd(collision, mink.m_matrix, convexSphapeSize);
			}

			count1 = mink.HullHullContinueContactsSimd(tmpTimestep,
			         &contactOut[count], faceId, countleft, proxy.m_unconditionalCast);
			if (count1) {
				count1 = polygon->ClipContacts(count1, &contactOut[count], soupMatrix);
				if (count1) {
					timestep = tmpTimestep;
					for (dgInt32 i = 0; i < count1; i++) {
						contactOut[count + i].m_point.m_w = timestep;
					}

					minTime = GetMin(minTime, timestep);
					if (timestep < oldTimeStep) {
						timestep = oldTimeStep + (timestep - oldTimeStep) * dgFloat32(0.5f);
					}

					count += count1;
					countleft -= count1;
					reduceContactCountLimit += count1;
					if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
						count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2,
						                       dgFloat32(1.0e-2f));
						countleft = proxy.m_maxContacts - count;
						reduceContactCountLimit = 0;
					}
				}
			}
		}

		//      indexArray += data.m_faceIndexCount[j];
		indexCount += data.m_faceIndexCount[j];
	}

	if (count > 1) {
		if (data.m_faceCount > 1) {
			dgFloat32 dt;
			dt = dgFloat32(
			         0.01f) *
			     dgRsqrt(mink.m_localRelVeloc % mink.m_localRelVeloc);
			for (dgInt32 i = 0; i < count; i++) {
				dgFloat32 err;
				err = contactOut[i].m_point.m_w - minTime;
				if (dgAbsf(err) > dt) {
					contactOut[i] = contactOut[count - 1];
					i--;
					count--;
				}
			}
		}
		count = FilterPolygonEdgeContacts(count, contactOut);
	}

	proxy.m_timestep = minTime;
	return count;

#else
	return 0;
#endif
}

dgInt32 dgWorld::CalculateConvexToNonConvexContactsContinue(
    dgCollisionParamProxy &proxy) const {
	dgInt32 count;
	dgInt32 thread;
	dgInt32 countleft;
	dgInt32 indexCount;
	dgInt32 reduceContactCountLimit;
	dgFloat32 minTime;
	dgFloat32 timestep;
	dgFloat32 oldTimeStep;

	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	dgCollisionConvex *const collision =
	    (dgCollisionConvex *)proxy.m_referenceCollision;
	dgCollisionMesh *const polysoup =
	    (dgCollisionMesh *)proxy.m_floatingCollision;
	NEWTON_ASSERT(collision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(polysoup->IsType(dgCollision::dgCollisionMesh_RTTI));

	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;
	const dgPolygonMeshDesc &data = *proxy.m_polyMeshData;
	thread = data.m_threadNumber;

	count = 0;
	dgContactPoint *const contactOut = proxy.m_contacts;
	NEWTON_ASSERT(proxy.m_timestep <= dgFloat32(1.0f));
	NEWTON_ASSERT(proxy.m_timestep >= dgFloat32(0.0f));

	timestep = proxy.m_timestep;
	NEWTON_ASSERT(timestep >= dgFloat32(0.0f));
	NEWTON_ASSERT(timestep <= dgFloat32(1.0f));
	// hack Omega to 0.5
	// hullBody->SetOmega (hullBody->GetOmega().Scale(dgFloat32 (0.9f)));
	NEWTON_ASSERT(data.m_faceCount);

	dgFloat32 *const faceSize = data.m_faceMaxSize;
	dgInt32 *const idArray = (dgInt32 *)data.m_userAttribute;
	dgInt32 *const indexArray = (dgInt32 *)data.m_faceVertexIndex;

	dgCollisionMesh::dgCollisionConvexPolygon *const polygon =
	    polysoup->m_polygon[thread];
	polygon->m_vertex = data.m_vertex;
	polygon->m_stride = dgInt32(data.m_vertexStrideInBytes / sizeof(dgFloat32));

	reduceContactCountLimit = 0;
	countleft = proxy.m_maxContacts;

	indexCount = 0;
	dgContactSolver mink(proxy, polygon);
	minTime = timestep;
	mink.CalculateVelocities(timestep);
	dgFloat32 convexSphapeSize = mink.GetShapeClipSize(collision);

	//  for (dgInt32 j = 0; j < data.m_faceCount; j ++) {
	for (dgInt32 j = 0; (j < data.m_faceCount) && (countleft > 0); j++) {
		polygon->m_count = data.m_faceIndexCount[j];
		//      polygon->m_index = indexArray;
		polygon->m_index = &indexArray[indexCount];

		if (data.m_faceNormalIndex) {
			polygon->m_normalIndex = data.m_faceNormalIndex[j];
			polygon->m_adjacentNormalIndex =
			    (dgInt32 *)&data.m_faceAdjencentEdgeNormal[indexCount];
		} else {
			polygon->m_normalIndex = 0;
			polygon->m_adjacentNormalIndex = NULL;
		}

		if (polygon->QuickTestContinue(collision, mink.m_matrix)) {
			dgInt32 count1;
			dgInt32 faceId;
			dgFloat32 tmpTimestep;

			faceId = idArray[j];
			oldTimeStep = timestep;
			tmpTimestep = timestep;

			if (faceSize && (faceSize[j] > convexSphapeSize)) {
				polygon->BeamClipping(collision, mink.m_matrix, convexSphapeSize);
			}

			count1 = mink.HullHullContinueContacts(tmpTimestep, &contactOut[count],
			                                       faceId, countleft, proxy.m_unconditionalCast);
			if (count1) {
				count1 = polygon->ClipContacts(count1, &contactOut[count], soupMatrix);
				if (count1) {
					timestep = tmpTimestep;
					for (dgInt32 i = 0; i < count1; i++) {
						contactOut[count + i].m_point.m_w = timestep;
					}

					minTime = GetMin(minTime, timestep);
					if (timestep < oldTimeStep) {
						timestep = oldTimeStep + (timestep - oldTimeStep) * dgFloat32(0.5f);
					}

					count += count1;
					countleft -= count1;
					reduceContactCountLimit += count1;
					if ((reduceContactCountLimit > 24) || (countleft <= 0)) {
						count = ReduceContacts(count, contactOut, proxy.m_maxContacts >> 2,
						                       dgFloat32(1.0e-2f));
						countleft = proxy.m_maxContacts - count;
						reduceContactCountLimit = 0;
					}
				}
			}
		}

		//      indexArray += data.m_faceIndexCount[j];
		indexCount += data.m_faceIndexCount[j];
	}

	if (count > 1) {
		if (data.m_faceCount > 1) {
			dgFloat32 dt;
			dt = dgFloat32(
			         0.01f) *
			     dgRsqrt(mink.m_localRelVeloc % mink.m_localRelVeloc);
			for (dgInt32 i = 0; i < count; i++) {
				dgFloat32 err;
				err = contactOut[i].m_point.m_w - minTime;
				if (dgAbsf(err) > dt) {
					contactOut[i] = contactOut[count - 1];
					i--;
					count--;
				}
			}
		}
		count = FilterPolygonEdgeContacts(count, contactOut);
	}

	proxy.m_timestep = minTime;
	return count;
}

dgInt32 dgWorld::CalculateConvexToNonConvexContactsSimd(
    dgCollisionParamProxy &proxy) const {
#ifdef DG_BUILD_SIMD_CODE

	dgPolygonMeshDesc data;
	//  dgCollisionConvex* collision;
	//  dgCollisionMesh* polysoup;
	//  dgContactPoint* contactOut;

	dgInt32 count = 0;
	proxy.m_inTriggerVolume = 0;
	dgCollisionConvex *collision = (dgCollisionConvex *)proxy.m_referenceCollision;
	if (!collision->m_vertexCount || collision->m_isTriggerVolume) {
		return count;
	}
	proxy.m_isTriggerVolume = 0;

	dgBody *hullBody = proxy.m_referenceBody;
	dgBody *soupBody = proxy.m_floatingBody;
	dgCollisionMesh *polysoup = (dgCollisionMesh *)proxy.m_floatingCollision;
	//  NEWTON_ASSERT (proxy.m_referenceCollision == hullBody->m_collision);
	//  NEWTON_ASSERT (proxy.m_floatingCollision == soupBody->m_collision);
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	const dgMatrix &hullMatrix = proxy.m_referenceMatrix;
	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;

	dgMatrix matrix(hullMatrix.MultiplySimd(soupMatrix.InverseSimd()));

	collision->CalcAABBSimd(matrix, data.m_boxP0, data.m_boxP1);

	NEWTON_ASSERT(proxy.m_timestep <= dgFloat32(1.0f));
	NEWTON_ASSERT(proxy.m_timestep >= dgFloat32(0.0f));

	dgInt32 doContinueCollision = 0;
	bool solverInContinueCollision = false;
	if (proxy.m_continueCollision) {
		dgFloat32 dist;
		dgFloat32 mag2;
		dgFloat32 spand;
		dgVector hullOmega;
		dgVector hullVeloc;

		NEWTON_ASSERT(proxy.m_timestep <= dgFloat32(1.0f));
		NEWTON_ASSERT(proxy.m_timestep >= dgFloat32(0.0f));
		// timestep = GetMin (timestep, dgFloat32 (1.0f));
		// timestep = GetMin (proxy.m_timestep, dgFloat32 (1.0f));
		hullBody->CalculateContinueVelocitySimd(proxy.m_timestep, hullVeloc,
		                                        hullOmega);

		NEWTON_ASSERT(
		    (proxy.m_unconditionalCast == 0) || (proxy.m_unconditionalCast == 1));
		if (proxy.m_unconditionalCast) {
			doContinueCollision = 1;
			dgVector step(
			    soupMatrix.UnrotateVectorSimd(hullVeloc.Scale(proxy.m_timestep)));
			for (dgInt32 j = 0; j < 3; j++) {
				if (step[j] > dgFloat32(0.0f)) {
					// data.m_boxP1.m_z += (step.m_z - boxSize.m_z);
					data.m_boxP1[j] += step[j];
				} else {
					// data.m_boxP0.m_z += (step.m_z + boxSize.m_z);
					data.m_boxP0[j] += step[j];
				}
			}

		} else {
			mag2 = hullVeloc % hullVeloc;

			if (mag2 > dgFloat32(0.1f)) {
				spand = collision->GetBoxMinRadius();
				dgFloat32 padding = collision->GetBoxMaxRadius() - spand;
				dgFloat32 maxOmega = (hullOmega % hullOmega) * proxy.m_timestep * proxy.m_timestep;
				padding = (maxOmega > 1.0f) ? padding : padding * dgSqrt(maxOmega);
				dist = dgSqrt(mag2) * proxy.m_timestep + padding;
				if (dist > (dgFloat32(0.25f) * spand)) {
					doContinueCollision = 1;
					solverInContinueCollision = true;

					dgVector step(
					    soupMatrix.UnrotateVectorSimd(hullVeloc.Scale(proxy.m_timestep)));
					step.m_x += (step.m_x > 0.0f) ? padding : -padding;
					step.m_y += (step.m_y > 0.0f) ? padding : -padding;
					step.m_z += (step.m_z > 0.0f) ? padding : -padding;

					dgVector boxSize(
					    (data.m_boxP1 - data.m_boxP0).Scale(dgFloat32(0.25f)));
					for (dgInt32 j = 0; j < 3; j++) {
						if (dgAbsf(step[j]) > boxSize[j]) {
							if (step[j] > dgFloat32(0.0f)) {
								// data.m_boxP1.m_z += (step.m_z - boxSize.m_z);
								data.m_boxP1[j] += step[j];
							} else {
								// data.m_boxP0.m_z += (step.m_z + boxSize.m_z);
								data.m_boxP0[j] += step[j];
							}
						}
					}
				}
			}
		}
	}

	//  thread = GetThreadNumber();
	data.m_vertex = NULL;
	data.m_threadNumber = proxy.m_threadIndex;
	data.m_faceCount = 0;
	data.m_faceIndexCount = 0;
	data.m_vertexStrideInBytes = 0;

	data.m_faceMaxSize = NULL;
	data.m_userAttribute = NULL;
	data.m_faceVertexIndex = NULL;
	data.m_faceNormalIndex = NULL;
	data.m_faceAdjencentEdgeNormal = NULL;
	data.m_userData = polysoup->GetUserData();
	data.m_objBody = hullBody;
	data.m_polySoupBody = soupBody;
	polysoup->GetCollidingFacesSimd(&data);

	if (data.m_faceCount) {
		dgMatrix matrixInv(matrix.InverseSimd());
		proxy.m_polyMeshData = &data;
		proxy.m_localMatrixInv = &matrixInv;

		if (doContinueCollision) {
			switch (collision->GetCollisionPrimityType()) {
			case m_sphereCollision: {
				count = CalculatePolySoupToSphereContactsContinue(proxy);
				break;
			}

			default: {
				count = CalculateConvexToNonConvexContactsContinueSimd(proxy);
				break;
			}
			}

			count = PruneContacts(count, proxy.m_contacts);
			if (count && solverInContinueCollision) {
				dgGetUserLock();
				hullBody->m_solverInContinueCollision = true;
				dgReleasedUserLock();
			}

		} else {
			switch (collision->GetCollisionPrimityType()) {
			case m_sphereCollision: {
				count = CalculatePolySoupToSphereContactsDescrete(proxy);
				break;
			}

			case m_ellipseCollision: {
				count = CalculatePolySoupToElipseContactsDescrete(proxy);
				break;
			}

			default: {
				count = CalculatePolySoupToHullContactsDescreteSimd(proxy);
			}
			}

			count = PruneContacts(count, proxy.m_contacts);
			if (count) {
				proxy.m_timestep = dgFloat32(0.0f);
			}
		}

		dgContactPoint *contactOut = proxy.m_contacts;
		for (dgInt32 i = 0; i < count; i++) {
			NEWTON_ASSERT(
			    (dgAbsf(contactOut[i].m_normal % contactOut[i].m_normal) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f));
			contactOut[i].m_body0 = proxy.m_referenceBody;
			contactOut[i].m_body1 = proxy.m_floatingBody;
			contactOut[i].m_collision0 = proxy.m_referenceCollision;
			contactOut[i].m_collision1 = proxy.m_floatingCollision;
		}
	}
	return count;
#else
	return 0;
#endif
}

dgInt32 dgWorld::CalculateConvexToNonConvexContacts(
    dgCollisionParamProxy &proxy) const {
	/*
	 static int xxx = 0;
	 static int xxxxx = 0;

	 dgVector force (proxy.m_referenceBody->m_accel);
	 if (force.m_y < -400.0f) {

	 //proxy.m_referenceBody->m_alpha = dgVector (0, 0, 0, 0);
	 #if 0

	 xxxxx = 1;
	 dgMatrix matrix;
	 matrix[0][0] = 0.985059f; matrix[0][1] = -0.000000f; matrix[0][2] = -0.172216f; matrix[0][3] = 0.000000f;
	 matrix[1][0] = 0.137235f; matrix[1][1] = 0.604136f; matrix[1][2] = 0.784975f; matrix[1][3] = 0.000000f;
	 matrix[2][0] = 0.104042f; matrix[2][1] = -0.796881f; matrix[2][2] = 0.595110f; matrix[2][3] = 0.000000f;
	 matrix[3][0] = 3.661594f; matrix[3][1] = 1.246089f; matrix[3][2] = -4.354931f; matrix[3][3] = 1.000000f;
	 proxy.m_referenceBody->m_matrix = matrix;

	 matrix[0][0] = 0.985059f; matrix[0][1] = -0.000000f; matrix[0][2] = -0.172216f; matrix[0][3] = 0.000000f;
	 matrix[1][0] = 0.137235f; matrix[1][1] = 0.604136f; matrix[1][2] = 0.784975f; matrix[1][3] = 0.000000f;
	 matrix[2][0] = 0.104042f; matrix[2][1] = -0.796881f; matrix[2][2] = 0.595110f; matrix[2][3] = 0.000000f;
	 matrix[3][0] = 3.661594f; matrix[3][1] = 1.246089f; matrix[3][2] = -4.354931f; matrix[3][3] = 1.000000f;
	 proxy.m_referenceMatrix = matrix;

	 proxy.m_referenceBody->m_rotation = dgQuaternion (0.892231, 0.443231, 0.077406, -0.038453);
	 proxy.m_referenceBody->m_accel = dgVector (96.322258, -502.308380, 543.779602, 0.0f);
	 proxy.m_referenceBody->m_alpha = dgVector  (2.398191, -0.558803, -0.481042, 0.0f);
	 proxy.m_referenceBody->m_globalCentreOfMass = dgVector (3.670152, 1.283851, -4.305888, 1.0f);

	 //proxy.m_referenceBody->m_localCentreOfMass = dgVector (0.0f, 0.0f, 0.0f, 1.0f);
	 #endif
	 }

	 if (!proxy.m_unconditionalCast && xxxxx)
	 xxx *= 1;



	 xxx += proxy.m_unconditionalCast ? 0 : xxxxx;
	 */

	dgPolygonMeshDesc data;
	dgInt32 count = 0;
	proxy.m_inTriggerVolume = 0;
	dgCollisionConvex *collision = (dgCollisionConvex *)proxy.m_referenceCollision;
	if (!collision->m_vertexCount || collision->m_isTriggerVolume) {
		return count;
	}
	proxy.m_isTriggerVolume = 0;

	dgBody *hullBody = proxy.m_referenceBody;
	dgBody *soupBody = proxy.m_floatingBody;
	dgCollisionMesh *polysoup = (dgCollisionMesh *)proxy.m_floatingCollision;
	NEWTON_ASSERT(
	    proxy.m_referenceCollision->IsType(dgCollision::dgConvexCollision_RTTI));
	NEWTON_ASSERT(
	    proxy.m_floatingCollision->IsType(dgCollision::dgCollisionMesh_RTTI));

	const dgMatrix &hullMatrix = proxy.m_referenceMatrix;
	const dgMatrix &soupMatrix = proxy.m_floatingMatrix;
	dgMatrix matrix(hullMatrix * soupMatrix.Inverse());
	collision->CalcAABB(matrix, data.m_boxP0, data.m_boxP1);

	NEWTON_ASSERT(proxy.m_timestep <= dgFloat32(1.0f));
	NEWTON_ASSERT(proxy.m_timestep >= dgFloat32(0.0f));

	dgInt32 doContinueCollision = 0;
	bool solverInContinueCollision = false;
	if (proxy.m_continueCollision) {
		//      dgFloat32 dist;
		//      dgFloat32 mag2;
		//      dgFloat32 spand;
		dgVector hullOmega;
		dgVector hullVeloc;

		NEWTON_ASSERT(proxy.m_timestep <= dgFloat32(1.0f));
		NEWTON_ASSERT(proxy.m_timestep >= dgFloat32(0.0f));
		// timestep = GetMin (timestep, dgFloat32 (1.0f));
		// timestep = GetMin (proxy.m_timestep, dgFloat32 (1.0f));
		hullBody->CalculateContinueVelocity(proxy.m_timestep, hullVeloc, hullOmega);

		NEWTON_ASSERT(
		    (proxy.m_unconditionalCast == 0) || (proxy.m_unconditionalCast == 1));
		if (proxy.m_unconditionalCast) {
			doContinueCollision = 1;
			dgVector step(
			    soupMatrix.UnrotateVector(hullVeloc.Scale(proxy.m_timestep)));
			for (dgInt32 j = 0; j < 3; j++) {
				if (step[j] > dgFloat32(0.0f)) {
					// data.m_boxP1.m_z += (step.m_z - boxSize.m_z);
					data.m_boxP1[j] += step[j];
				} else {
					// data.m_boxP0.m_z += (step.m_z + boxSize.m_z);
					data.m_boxP0[j] += step[j];
				}
			}
		} else {
			dgFloat32 mag2 = hullVeloc % hullVeloc;
			if (mag2 > dgFloat32(0.1f)) {
				dgFloat32 spand = collision->GetBoxMinRadius();
				dgFloat32 padding = collision->GetBoxMaxRadius() - spand;
				dgFloat32 maxOmega = (hullOmega % hullOmega) * proxy.m_timestep * proxy.m_timestep;
				padding = (maxOmega > 1.0f) ? padding : padding * dgSqrt(maxOmega);

				dgFloat32 dist = dgSqrt(mag2) * proxy.m_timestep + padding;
				if (dist > (dgFloat32(0.25f) * spand)) {
					doContinueCollision = 1;
					solverInContinueCollision = true;

					dgVector step(
					    soupMatrix.UnrotateVector(hullVeloc.Scale(proxy.m_timestep)));
					step.m_x += (step.m_x > 0.0f) ? padding : -padding;
					step.m_y += (step.m_y > 0.0f) ? padding : -padding;
					step.m_z += (step.m_z > 0.0f) ? padding : -padding;

					dgVector boxSize(
					    (data.m_boxP1 - data.m_boxP0).Scale(dgFloat32(0.25f)));
					for (dgInt32 j = 0; j < 3; j++) {
						if (dgAbsf(step[j]) > boxSize[j]) {
							if (step[j] > dgFloat32(0.0f)) {
								data.m_boxP1[j] += step[j];
							} else {
								data.m_boxP0[j] += step[j];
							}
						}
					}
				}
			}
		}
	}

	data.m_vertex = NULL;
	data.m_threadNumber = proxy.m_threadIndex;
	data.m_faceCount = 0;
	data.m_faceIndexCount = 0;
	data.m_vertexStrideInBytes = 0;

	data.m_faceMaxSize = NULL;
	data.m_userAttribute = NULL;
	data.m_faceVertexIndex = NULL;
	data.m_faceNormalIndex = NULL;
	data.m_faceAdjencentEdgeNormal = NULL;
	data.m_userData = polysoup->GetUserData();
	data.m_objBody = hullBody;
	data.m_polySoupBody = soupBody;

	polysoup->GetCollidingFaces(&data);

	if (data.m_faceCount) {
		dgMatrix matrixInv(matrix.Inverse());
		proxy.m_polyMeshData = &data;
		proxy.m_localMatrixInv = &matrixInv;

		if (doContinueCollision) {
			switch (collision->GetCollisionPrimityType()) {
			case m_sphereCollision: {
				count = CalculatePolySoupToSphereContactsContinue(proxy);
				break;
			}

			default: {
				count = CalculateConvexToNonConvexContactsContinue(proxy);
				break;
			}
			}

			count = PruneContacts(count, proxy.m_contacts);
			if (count && solverInContinueCollision) {
				dgGetUserLock();
				hullBody->m_solverInContinueCollision = true;
				dgReleasedUserLock();
			}

		} else {

			switch (collision->GetCollisionPrimityType()) {
			case m_sphereCollision: {
				count = CalculatePolySoupToSphereContactsDescrete(proxy);
				break;
			}

			case m_ellipseCollision: {
				count = CalculatePolySoupToElipseContactsDescrete(proxy);
				break;
			}

			default: {
				count = CalculatePolySoupToHullContactsDescrete(proxy);
			}
			}

			count = PruneContacts(count, proxy.m_contacts);
			if (count) {
				proxy.m_timestep = dgFloat32(0.0f);
			}
		}

		dgContactPoint *contactOut = proxy.m_contacts;
		for (dgInt32 i = 0; i < count; i++) {
			NEWTON_ASSERT(
			    (dgAbsf(contactOut[i].m_normal % contactOut[i].m_normal) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f));
			contactOut[i].m_body0 = proxy.m_referenceBody;
			contactOut[i].m_body1 = proxy.m_floatingBody;
			contactOut[i].m_collision0 = proxy.m_referenceCollision;
			contactOut[i].m_collision1 = proxy.m_floatingCollision;
		}
	}
	return count;
}
