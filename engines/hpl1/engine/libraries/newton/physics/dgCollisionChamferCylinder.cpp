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

#include "dgCollisionChamferCylinder.h"
#include "dgBody.h"
#include "dgContact.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgInt32 dgCollisionChamferCylinder::m_shapeRefCount = 0;
dgVector dgCollisionChamferCylinder::m_shapesDirs[DG_MAX_CHAMFERCYLINDER_DIR_COUNT];
dgConvexSimplexEdge dgCollisionChamferCylinder::m_edgeArray[(4 * DG_CHAMFERCYLINDER_SLICES + 2) * DG_CHAMFERCYLINDER_BRAKES];

dgCollisionChamferCylinder::dgCollisionChamferCylinder(
    dgMemoryAllocator *allocator, dgUnsigned32 signature, dgFloat32 radius,
    dgFloat32 height, const dgMatrix &matrix) : dgCollisionConvex(allocator, signature, matrix, m_chamferCylinderCollision) {
	Init(radius, height);
}

dgCollisionChamferCylinder::dgCollisionChamferCylinder(dgWorld *const world,
        dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	dgVector size;
	deserialization(userData, &size, sizeof(dgVector));
	Init(size.m_x, size.m_y);
}

dgCollisionChamferCylinder::~dgCollisionChamferCylinder() {
	m_shapeRefCount--;
	NEWTON_ASSERT(m_shapeRefCount >= 0);

	dgCollisionConvex::m_simplex = NULL;
	dgCollisionConvex::m_vertex = NULL;
}

void dgCollisionChamferCylinder::Init(dgFloat32 radius, dgFloat32 height) {
	//  dgInt32 i;
	//  dgInt32 j;
	//  dgInt32 index;
	//  dgInt32 index0;
	//  dgFloat32 sliceStep;
	//  dgFloat32 sliceAngle;
	//  dgFloat32 breakStep;
	//  dgFloat32 breakAngle;
	//  dgEdge *edge;

	m_rtti |= dgCollisionChamferCylinder_RTTI;
	m_radius = dgAbsf(radius);
	m_height = dgAbsf(height * dgFloat32(0.5f));
	m_radius = GetMax(dgFloat32(0.001f), m_radius - m_height);

	m_silhuette[0] = dgVector(m_height, m_radius, dgFloat32(0.0f),
	                          dgFloat32(0.0f));
	m_silhuette[1] = dgVector(m_height, -m_radius, dgFloat32(0.0f),
	                          dgFloat32(0.0f));
	m_silhuette[2] = dgVector(-m_height, -m_radius, dgFloat32(0.0f),
	                          dgFloat32(0.0f));
	m_silhuette[3] = dgVector(-m_height, m_radius, dgFloat32(0.0f),
	                          dgFloat32(0.0f));

	//  m_tethaStep = GetDiscretedAngleStep (m_radius);
	//  m_tethaStepInv = dgFloat32 (1.0f) / m_tethaStep;
	//  m_delCosTetha = dgCos (m_tethaStep);
	//  m_delSinTetha = dgSin (m_tethaStep);

	dgFloat32 sliceAngle = dgFloat32(0.0f);
	// dgFloat32 breakAngle = dgFloat32 (0.0f);
	dgFloat32 sliceStep = dgPI / DG_CHAMFERCYLINDER_SLICES;
	dgFloat32 breakStep = dgPI2 / DG_CHAMFERCYLINDER_BRAKES;

	dgMatrix rot(dgPitchMatrix(breakStep));
	dgInt32 k = 0;
	for (dgInt32 j = 0; j <= DG_CHAMFERCYLINDER_SLICES; j++) {
		dgVector p0(-m_height * dgCos(sliceAngle), dgFloat32(0.0f),
		            m_radius + m_height * dgSin(sliceAngle), dgFloat32(1.0f));
		sliceAngle += sliceStep;
		for (dgInt32 i = 0; i < DG_CHAMFERCYLINDER_BRAKES; i++) {
			m_vertex[k] = p0;
			k++;
			p0 = rot.UnrotateVector(p0);
		}
	}

	m_edgeCount = (4 * DG_CHAMFERCYLINDER_SLICES + 2) * DG_CHAMFERCYLINDER_BRAKES;
	m_vertexCount = DG_CHAMFERCYLINDER_BRAKES * (DG_CHAMFERCYLINDER_SLICES + 1);
	dgCollisionConvex::m_vertex = m_vertex;

	if (!m_shapeRefCount) {
		dgPolyhedra polyhedra(m_allocator);
		dgInt32 wireframe[DG_CHAMFERCYLINDER_SLICES + 10];

		for (dgInt32 i = 0; i < DG_MAX_CHAMFERCYLINDER_DIR_COUNT; i++) {
			dgMatrix matrix(
			    dgPitchMatrix(
			        dgFloat32(dgPI2 * i) / DG_MAX_CHAMFERCYLINDER_DIR_COUNT));
			m_shapesDirs[i] = matrix.RotateVector(
			                      dgVector(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
			                               dgFloat32(0.0f)));
		}

		dgInt32 index = 0;
		for (dgInt32 j = 0; j < DG_CHAMFERCYLINDER_SLICES; j++) {
			dgInt32 index0 = index + DG_CHAMFERCYLINDER_BRAKES - 1;
			for (dgInt32 i = 0; i < DG_CHAMFERCYLINDER_BRAKES; i++) {
				wireframe[0] = index;
				wireframe[1] = index0;
				wireframe[2] = index0 + DG_CHAMFERCYLINDER_BRAKES;
				wireframe[3] = index + DG_CHAMFERCYLINDER_BRAKES;

				index0 = index;
				index++;
				polyhedra.AddFace(4, wireframe);
			}
		}

		for (dgInt32 i = 0; i < DG_CHAMFERCYLINDER_BRAKES; i++) {
			wireframe[i] = i;
		}
		polyhedra.AddFace(DG_CHAMFERCYLINDER_BRAKES, wireframe);

		for (dgInt32 i = 0; i < DG_CHAMFERCYLINDER_BRAKES; i++) {
			wireframe[i] = DG_CHAMFERCYLINDER_BRAKES * (DG_CHAMFERCYLINDER_SLICES + 1) - i - 1;
		}
		polyhedra.AddFace(DG_CHAMFERCYLINDER_BRAKES, wireframe);
		polyhedra.EndFace();

		NEWTON_ASSERT(SanityCheck(polyhedra));

		dgUnsigned64 i = 0;
		dgPolyhedra::Iterator iter(polyhedra);
		for (iter.Begin(); iter; iter++) {
			dgEdge *const edge = &(*iter);
			edge->m_userData = i;
			i++;
		}

		for (iter.Begin(); iter; iter++) {
			dgEdge *const edge = &(*iter);

			dgConvexSimplexEdge *const ptr = &m_edgeArray[edge->m_userData];
			ptr->m_vertex = edge->m_incidentVertex;
			ptr->m_next = &m_edgeArray[edge->m_next->m_userData];
			ptr->m_prev = &m_edgeArray[edge->m_prev->m_userData];
			ptr->m_twin = &m_edgeArray[edge->m_twin->m_userData];
		}
	}

	m_shapeRefCount++;
	dgCollisionConvex::m_simplex = m_edgeArray;

	SetVolumeAndCG();
	//  CalculateDistanceTravel();
}

void dgCollisionChamferCylinder::DebugCollision(const dgMatrix &matrixPtr,
        OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt32 i;
	dgInt32 j;
	dgInt32 index;
	dgInt32 index0;
	dgInt32 brakes;
	dgInt32 slices;
	dgFloat32 sliceStep;
	dgFloat32 sliceAngle;
	dgFloat32 breakStep;

	slices = 12;
	brakes = 24;
	sliceAngle = dgFloat32(0.0f);
	sliceStep = dgPI / slices;
	breakStep = dgPI2 / brakes;

	dgTriplex pool[24 * (12 + 1)];

	dgMatrix rot(dgPitchMatrix(breakStep));
	index = 0;
	for (j = 0; j <= slices; j++) {
		dgVector p0(-m_height * dgCos(sliceAngle), dgFloat32(0.0f),
		            m_radius + m_height * dgSin(sliceAngle), dgFloat32(0.0f));
		sliceAngle += sliceStep;
		for (i = 0; i < brakes; i++) {
			pool[index].m_x = p0.m_x;
			pool[index].m_y = p0.m_y;
			pool[index].m_z = p0.m_z;
			index++;
			p0 = rot.UnrotateVector(p0);
		}
	}

	//  const dgMatrix &matrix = myBody.GetCollisionMatrix();
	dgMatrix matrix(GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex(&pool[0].m_x, sizeof(dgTriplex), &pool[0].m_x,
	                        sizeof(dgTriplex), 24 * (12 + 1));

	dgTriplex face[32];

	index = 0;
	for (j = 0; j < slices; j++) {
		index0 = index + brakes - 1;
		for (i = 0; i < brakes; i++) {
			face[0] = pool[index];
			face[1] = pool[index0];
			face[2] = pool[index0 + brakes];
			face[3] = pool[index + brakes];
			index0 = index;
			index++;
			callback(userData, 4, &face[0].m_x, 0);
		}
	}

	for (i = 0; i < brakes; i++) {
		face[i] = pool[i];
	}
	callback(userData, 24, &face[0].m_x, 0);

	for (i = 0; i < brakes; i++) {
		face[i] = pool[brakes * (slices + 1) - i - 1];
	}
	callback(userData, 24, &face[0].m_x, 0);
}

dgInt32 dgCollisionChamferCylinder::CalculateSignature() const {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_chamferCylinderCollision;
	buffer[1] = dgCollision::Quantize(m_radius);
	buffer[2] = dgCollision::Quantize(m_height);
	memcpy(&buffer[3], &m_offset, sizeof(dgMatrix));
	return dgInt32(dgCollision::MakeCRC(buffer, sizeof(buffer)));
}

void dgCollisionChamferCylinder::SetCollisionBBox(const dgVector &p0__,
        const dgVector &p1__) {
	NEWTON_ASSERT(0);
}

/*
 DG_INLINE_FUNTION dgVector dgCollisionChamferCylinder::QuatizedSupportVertexSimd (const dgVector& dir) const
 {
 NEWTON_ASSERT (dgAbsf(dir % dir - dgFloat32 (1.0f)) < dgFloat32 (1.0e-3f));

 dgVector sideDir (dgFloat32 (0.0f), dir.m_y, dir.m_z, dgFloat32 (0.0f));
 sideDir = sideDir.Scale (m_radius * dgRsqrt (sideDir % sideDir + dgFloat32 (1.0e-14f)));
 return sideDir + dir.Scale (m_height);
 }

 DG_INLINE_FUNTION dgVector dgCollisionChamferCylinder::QuatizedSupportVertex (const dgVector& dir) const
 {
 NEWTON_ASSERT (dgAbsf(dir % dir - dgFloat32 (1.0f)) < dgFloat32 (1.0e-3f));

 dgVector sideDir (dgFloat32 (0.0f), dir.m_y, dir.m_z, dgFloat32 (0.0f));
 sideDir = sideDir.Scale (m_radius * dgRsqrt (sideDir % sideDir + dgFloat32 (1.0e-14f)));
 return sideDir + dir.Scale (m_height);
 }
 */

dgVector dgCollisionChamferCylinder::SupportVertexSimd(
    const dgVector &dir) const {
	return SupportVertex(dir);
}

dgVector dgCollisionChamferCylinder::SupportVertex(const dgVector &dir) const {
	NEWTON_ASSERT(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	if (dgAbsf(dir.m_x) > dgFloat32(0.9998f)) {
		dgFloat32 x0;
		x0 = (dir.m_x >= dgFloat32(0.0f)) ? m_height : -m_height;
		return dgVector(x0, dgFloat32(0.0f), m_radius, dgFloat32(0.0f));
	}

	NEWTON_ASSERT(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	dgVector sideDir(dgFloat32(0.0f), dir.m_y, dir.m_z, dgFloat32(0.0f));
	sideDir = sideDir.Scale(
	              m_radius * dgRsqrt(sideDir % sideDir + dgFloat32(1.0e-18f)));
	return sideDir + dir.Scale(m_height);
}

dgFloat32 dgCollisionChamferCylinder::RayCast(const dgVector &q0,
        const dgVector &q1, dgContactPoint &contactOut,
        OnRayPrecastAction preFilter, const dgBody *const body,
        void *const userData) const {
	//  dgFloat32 t;
	//  dgFloat32 y;
	//  dgFloat32 z;
	//  dgFloat32 t1;

	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	//  dgFloat32 t = dgFloat32 (1.2f);
	if (q0.m_x > m_height) {
		if (q1.m_x < m_height) {
			dgFloat32 t1 = (m_height - q0.m_x) / (q1.m_x - q0.m_x);
			dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
			dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
			if ((y * y + z * z) < m_radius * m_radius) {
				contactOut.m_normal = dgVector(dgFloat32(1.0f), dgFloat32(0.0f),
				                               dgFloat32(0.0f), dgFloat32(0.0f));
				contactOut.m_userId = SetUserDataID();
				return t1;
			}
		}
	}

	if (q0.m_x < -m_height) {
		if (q1.m_x > -m_height) {
			dgFloat32 t1 = (-m_height - q0.m_x) / (q1.m_x - q0.m_x);
			dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
			dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
			if ((y * y + z * z) < m_radius * m_radius) {
				contactOut.m_normal = dgVector(dgFloat32(-1.0f), dgFloat32(0.0f),
				                               dgFloat32(0.0f), dgFloat32(0.0f));
				contactOut.m_userId = SetUserDataID();
				return t1;
			}
		}
	}

	return dgCollisionConvex::RayCast(q0, q1, contactOut, NULL, NULL, NULL);
}

dgFloat32 dgCollisionChamferCylinder::RayCastSimd(const dgVector &q0,
        const dgVector &q1, dgContactPoint &contactOut,
        OnRayPrecastAction preFilter, const dgBody *const body,
        void *const userData) const {
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	//  dgFloat32 t = dgFloat32 (1.2f);
	if (q0.m_x > m_height) {
		if (q1.m_x < m_height) {
			dgFloat32 t1 = (m_height - q0.m_x) / (q1.m_x - q0.m_x);
			dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
			dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
			if ((y * y + z * z) < m_radius * m_radius) {
				contactOut.m_normal = dgVector(dgFloat32(1.0f), dgFloat32(0.0f),
				                               dgFloat32(0.0f), dgFloat32(0.0f));
				contactOut.m_userId = SetUserDataID();
				return t1;
			}
		}
	}

	if (q0.m_x < -m_height) {
		if (q1.m_x > -m_height) {
			dgFloat32 t1 = (-m_height - q0.m_x) / (q1.m_x - q0.m_x);
			dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
			dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
			if ((y * y + z * z) < m_radius * m_radius) {
				contactOut.m_normal = dgVector(dgFloat32(-1.0f), dgFloat32(0.0f),
				                               dgFloat32(0.0f), dgFloat32(0.0f));
				contactOut.m_userId = SetUserDataID();
				return t1;
			}
		}
	}
	return dgCollisionConvex::RayCastSimd(q0, q1, contactOut, NULL, NULL, NULL);
}

dgInt32 dgCollisionChamferCylinder::CalculatePlaneIntersectionSimd(
    const dgVector &normal, const dgVector &origin,
    dgVector *const contactsOut) const {
	return dgCollisionChamferCylinder::CalculatePlaneIntersection(normal, origin,
	        contactsOut);
}

dgInt32 dgCollisionChamferCylinder::CalculatePlaneIntersection(
    const dgVector &normal, const dgVector &origin,
    dgVector *const contactsOut) const {
	dgInt32 count;
	if (dgAbsf(normal.m_x) < dgFloat32(0.999f)) {
		dgFloat32 magInv =
		    dgRsqrt(normal.m_y * normal.m_y + normal.m_z * normal.m_z);
		dgFloat32 cosAng = normal.m_y * magInv;
		dgFloat32 sinAng = normal.m_z * magInv;
		NEWTON_ASSERT(
		    dgAbsf(normal.m_z * cosAng - normal.m_y * sinAng) < dgFloat32(1.0e-4f));
		dgVector normal1(normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng,
		                 dgFloat32(0.0f), dgFloat32(0.0f));
		dgVector origin1(origin.m_x, origin.m_y * cosAng + origin.m_z * sinAng,
		                 origin.m_z * cosAng - origin.m_y * sinAng, dgFloat32(0.0f));
		dgPlane plane(normal1, -(normal1 % origin1));
		count = 0;
		dgVector maxDir(
		    (normal1.m_x > dgFloat32(0.0f)) ? m_silhuette[0].m_x : -m_silhuette[0].m_x,
		    (normal1.m_y > dgFloat32(0.0f)) ? m_silhuette[0].m_y : -m_silhuette[0].m_y, dgFloat32(0.0f),
		    dgFloat32(0.0f));

		dgFloat32 test0 = plane.Evalue(maxDir);
		dgFloat32 test1 = plane.Evalue(maxDir.Scale(dgFloat32(-1.0f)));
		if ((test0 * test1) > dgFloat32(0.0f)) {
			test0 = plane.m_w + plane.m_y * m_radius;
			if (dgAbsf(test0) < m_height) {
				contactsOut[count] = normal1.Scale(-test0);
				contactsOut[count].m_y += m_radius;
				count++;
			} else {
				test0 = plane.m_w - plane.m_y * m_radius;
				if (dgAbsf(test0) < m_height) {
					contactsOut[count] = normal1.Scale(-test0);
					contactsOut[count].m_y -= m_radius;
					count++;
				}
			}

		} else {
			dgVector dp(m_silhuette[1] - m_silhuette[0]);
			dgFloat32 den = normal1 % dp;
			NEWTON_ASSERT(dgAbsf(den) > dgFloat32(0.0f));
			test0 = -plane.Evalue(m_silhuette[0]) / den;
			if ((test0 <= dgFloat32(1.0)) && (test0 >= dgFloat32(0.0f))) {
				contactsOut[count] = m_silhuette[0] + dp.Scale(test0);
				count++;
			}
			if (count < 2) {
				test0 = plane.m_w - plane.m_y * m_radius;
				if (dgAbsf(test0) < m_height) {
					dgFloat32 r = -m_radius;
					dgFloat32 d = plane.m_w + r * plane.m_y;

					dgFloat32 a = plane.m_x * plane.m_x + plane.m_y * plane.m_y;
					dgFloat32 b = dgFloat32(2.0f) * plane.m_y * d;
					dgFloat32 c = d * d - m_height * m_height * plane.m_x * plane.m_x;
					dgFloat32 desc = b * b - dgFloat32(4.0f) * a * c;
					if (desc > dgFloat32(0.0f)) {
						NEWTON_ASSERT(dgAbsf(a) > dgFloat32(0.0f));
						desc = dgSqrt(desc);
						a = -dgFloat32(0.5f) * b / a;
						dgFloat32 y0 = a + desc;
						dgFloat32 y1 = a - desc;
						if (y0 > dgFloat32(0.0f)) {
							y0 = y1;
						}
						NEWTON_ASSERT(y0 < dgFloat32(0.0f));
						NEWTON_ASSERT(dgAbsf(plane.m_x) > dgFloat32(0.0f));
						dgFloat32 x = -(plane.m_y * y0 + d) / plane.m_x;
						contactsOut[count] = dgVector(x, y0 + r, dgFloat32(0.0f),
						                              dgFloat32(0.0f));
						count++;
					}
				}
			}

			if (count < 2) {
				dgVector dpp(m_silhuette[3] - m_silhuette[2]);
				den = normal1 % dpp;
				NEWTON_ASSERT(dgAbsf(den) > dgFloat32(0.0f));
				test0 = -plane.Evalue(m_silhuette[2]) / den;
				if ((test0 <= dgFloat32(1.0)) && (test0 >= dgFloat32(0.0f))) {
					contactsOut[count] = m_silhuette[2] + dpp.Scale(test0);
					count++;
				}
			}

			if (count < 2) {
				test0 = plane.m_w + plane.m_y * m_radius;
				if (dgAbsf(test0) < m_height) {

					dgFloat32 r = m_radius;
					dgFloat32 d = plane.m_w + r * plane.m_y;

					dgFloat32 a = plane.m_x * plane.m_x + plane.m_y * plane.m_y;
					dgFloat32 b = dgFloat32(2.0f) * plane.m_y * d;
					dgFloat32 c = d * d - m_height * m_height * plane.m_x * plane.m_x;
					dgFloat32 desc = b * b - dgFloat32(4.0f) * a * c;
					if (desc > dgFloat32(0.0f)) {
						NEWTON_ASSERT(dgAbsf(a) > dgFloat32(0.0f));
						desc = dgSqrt(desc);
						a = -dgFloat32(0.5f) * b / a;
						dgFloat32 y0 = a + desc;
						dgFloat32 y1 = a - desc;
						if (y0 < dgFloat32(0.0f)) {
							y0 = y1;
						}
						NEWTON_ASSERT(y0 > dgFloat32(0.0f));
						NEWTON_ASSERT(dgAbsf(plane.m_x) > dgFloat32(0.0f));
						dgFloat32 x = -(plane.m_y * y0 + d) / plane.m_x;
						contactsOut[count] = dgVector(x, y0 + r, dgFloat32(0.0f),
						                              dgFloat32(0.0f));
						count++;
					}
				}
			}
		}

		for (dgInt32 i = 0; i < count; i++) {
			dgFloat32 y = contactsOut[i].m_y;
			dgFloat32 z = contactsOut[i].m_z;
			contactsOut[i].m_y = y * cosAng - z * sinAng;
			contactsOut[i].m_z = z * cosAng + y * sinAng;
		}

	} else {
		count = dgCollisionConvex::CalculatePlaneIntersection(normal, origin,
		        contactsOut);
	}
	return count;
}

void dgCollisionChamferCylinder::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_chamferCylinder.m_r = m_radius + m_height;
	info->m_chamferCylinder.m_height = m_height * dgFloat32(2.0f);
	info->m_offsetMatrix = GetOffsetMatrix();
	//  strcpy (info->m_collisionType, "chamferCylinder");
	info->m_collisionType = m_collsionId;
}

void dgCollisionChamferCylinder::Serialize(dgSerialize callback,
        void *const userData) const {
	dgVector size(m_radius + m_height, m_height * dgFloat32(2.0f),
	              dgFloat32(0.0f), dgFloat32(0.0f));

	SerializeLow(callback, userData);
	callback(userData, &size, sizeof(dgVector));
}
