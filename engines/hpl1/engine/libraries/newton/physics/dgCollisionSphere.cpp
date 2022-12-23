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

#include "dgCollisionSphere.h"
#include "dgBody.h"
#include "dgContact.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define EDGE_COUNT 96

dgInt32 dgCollisionSphere::m_shapeRefCount = 0;
dgVector dgCollisionSphere::m_unitSphere[DG_SPHERE_VERTEX_COUNT];
dgConvexSimplexEdge dgCollisionSphere::m_edgeArray[EDGE_COUNT];

dgCollisionSphere::dgCollisionSphere(dgMemoryAllocator *const allocator,
                                     dgUnsigned32 signature, dgFloat32 radii, const dgMatrix &offsetMatrix) : dgCollisionConvex(allocator, signature, offsetMatrix, m_sphereCollision) {
	Init(radii, allocator);
}

dgCollisionSphere::dgCollisionSphere(dgWorld *const world,
                                     dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	dgVector size;
	deserialization(userData, &size, sizeof(dgVector));
	Init(size.m_x, world->GetAllocator());
}

dgCollisionSphere::~dgCollisionSphere() {
	m_shapeRefCount--;
	NEWTON_ASSERT(m_shapeRefCount >= 0);

	dgCollisionConvex::m_simplex = NULL;
	dgCollisionConvex::m_vertex = NULL;
}

void dgCollisionSphere::Init(dgFloat32 radius, dgMemoryAllocator *allocator) {
	m_rtti |= dgCollisionSphere_RTTI;
	m_radius = radius;

	m_edgeCount = EDGE_COUNT;
	m_vertexCount = DG_SPHERE_VERTEX_COUNT;
	dgCollisionConvex::m_vertex = m_vertex;

	if (!m_shapeRefCount) {

		dgInt32 indexList[256];
		dgVector tmpVectex[256];

		dgVector p0(dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		            dgFloat32(0.0f));
		dgVector p1(-dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		            dgFloat32(0.0f));
		dgVector p2(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
		            dgFloat32(0.0f));
		dgVector p3(dgFloat32(0.0f), -dgFloat32(1.0f), dgFloat32(0.0f),
		            dgFloat32(0.0f));
		dgVector p4(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f),
		            dgFloat32(0.0f));
		dgVector p5(dgFloat32(0.0f), dgFloat32(0.0f), -dgFloat32(1.0f),
		            dgFloat32(0.0f));

		dgInt32 i = 1;
		dgInt32 count = 0;
		TesselateTriangle(i, p4, p0, p2, count, tmpVectex);
		TesselateTriangle(i, p4, p2, p1, count, tmpVectex);
		TesselateTriangle(i, p4, p1, p3, count, tmpVectex);
		TesselateTriangle(i, p4, p3, p0, count, tmpVectex);
		TesselateTriangle(i, p5, p2, p0, count, tmpVectex);
		TesselateTriangle(i, p5, p1, p2, count, tmpVectex);
		TesselateTriangle(i, p5, p3, p1, count, tmpVectex);
		TesselateTriangle(i, p5, p0, p3, count, tmpVectex);

		//NEWTON_ASSERT (count == EDGE_COUNT);
		dgInt32 vertexCount = dgVertexListToIndexList(&tmpVectex[0].m_x,
		                      sizeof(dgVector), 3 * sizeof(dgFloat32), 0, count, indexList, 0.001f);

		NEWTON_ASSERT(vertexCount == DG_SPHERE_VERTEX_COUNT);
		for (dgInt32 j = 0; j < vertexCount; j++) {
			m_unitSphere[j] = tmpVectex[j];
		}
		dgPolyhedra polyhedra(m_allocator);

		polyhedra.BeginFace();
		for (dgInt32 j = 0; j < count; j += 3) {
#ifdef _DEBUG
			dgEdge *const edge = polyhedra.AddFace(indexList[j], indexList[j + 1],
			                                       indexList[j + 2]);
			NEWTON_ASSERT(edge);
#else
			polyhedra.AddFace(indexList[j], indexList[j + 1], indexList[j + 2]);
#endif
		}
		polyhedra.EndFace();

		dgUnsigned64 i1 = 0;
		dgPolyhedra::Iterator iter(polyhedra);
		for (iter.Begin(); iter; iter++) {
			dgEdge *const edge = &(*iter);
			edge->m_userData = i1;
			i1++;
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

	for (dgInt32 i = 0; i < DG_SPHERE_VERTEX_COUNT; i++) {
		m_vertex[i] = m_unitSphere[i].Scale(m_radius);
	}

	m_shapeRefCount++;
	dgCollisionConvex::m_simplex = m_edgeArray;

	SetVolumeAndCG();

	dgVector inertia;
	dgVector centerOfMass;
	dgVector crossInertia;
	m_volume.m_w = CalculateMassProperties(inertia, crossInertia, centerOfMass);
}

dgVector dgCollisionSphere::SupportVertexSimd(const dgVector &dir) const {
	NEWTON_ASSERT(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));
	//  return SupportVertex (dir);
	return dir.Scale(m_radius);
}

dgVector dgCollisionSphere::SupportVertex(const dgVector &dir) const {
	NEWTON_ASSERT(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));
	return dir.Scale(m_radius);
}

void dgCollisionSphere::TesselateTriangle(dgInt32 level, const dgVector &p0,
        const dgVector &p1, const dgVector &p2, dgInt32 &count,
        dgVector *ouput) const {
	if (level) {
		NEWTON_ASSERT(dgAbsf(p0 % p0 - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));
		NEWTON_ASSERT(dgAbsf(p1 % p1 - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));
		NEWTON_ASSERT(dgAbsf(p2 % p2 - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));
		dgVector p01(p0 + p1);
		dgVector p12(p1 + p2);
		dgVector p20(p2 + p0);

		p01 = p01.Scale(dgFloat32(1.0f) / dgSqrt(p01 % p01));
		p12 = p12.Scale(dgFloat32(1.0f) / dgSqrt(p12 % p12));
		p20 = p20.Scale(dgFloat32(1.0f) / dgSqrt(p20 % p20));

		NEWTON_ASSERT(dgAbsf(p01 % p01 - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));
		NEWTON_ASSERT(dgAbsf(p12 % p12 - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));
		NEWTON_ASSERT(dgAbsf(p20 % p20 - dgFloat32(1.0f)) < dgFloat32(1.0e-4f));

		TesselateTriangle(level - 1, p0, p01, p20, count, ouput);
		TesselateTriangle(level - 1, p1, p12, p01, count, ouput);
		TesselateTriangle(level - 1, p2, p20, p12, count, ouput);
		TesselateTriangle(level - 1, p01, p12, p20, count, ouput);

	} else {
		ouput[count++] = p0;
		ouput[count++] = p1;
		ouput[count++] = p2;
	}
}

void dgCollisionSphere::SetCollisionBBox(const dgVector &p0__,
        const dgVector &p1__) {
	NEWTON_ASSERT(0);
}

dgInt32 dgCollisionSphere::CalculateSignature() const {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_sphereCollision;
	buffer[1] = Quantize(m_radius);
	memcpy(&buffer[2], &m_offset, sizeof(dgMatrix));
	return dgInt32(MakeCRC(buffer, sizeof(buffer)));
}

void dgCollisionSphere::CalcAABB(const dgMatrix &matrix, dgVector &p0,
                                 dgVector &p1) const {
	dgFloat32 radius = m_radius + DG_MAX_COLLISION_PADDING;
	p0.m_x = matrix[3][0] - radius;
	p1.m_x = matrix[3][0] + radius;

	p0.m_y = matrix[3][1] - radius;
	p1.m_y = matrix[3][1] + radius;

	p0.m_z = matrix[3][2] - radius;
	p1.m_z = matrix[3][2] + radius;

	p0.m_w = dgFloat32(1.0f);
	p1.m_w = dgFloat32(1.0f);
}

dgInt32 dgCollisionSphere::CalculatePlaneIntersection(const dgVector &normal,
        const dgVector &point, dgVector *const contactsOut) const {
	NEWTON_ASSERT((normal % normal) > dgFloat32(0.999f));
	//  contactsOut[0] = point;
	contactsOut[0] = normal.Scale(normal % point);
	return 1;
}

dgInt32 dgCollisionSphere::CalculatePlaneIntersectionSimd(
    const dgVector &normal, const dgVector &point,
    dgVector *const contactsOut) const {
#ifdef DG_BUILD_SIMD_CODE

	NEWTON_ASSERT((normal % normal) > dgFloat32(0.999f));
	//  contactsOut[0] = point;
	contactsOut[0] = normal.Scale(normal % point);
	return 1;

#else
	return 0;
#endif
}

void dgCollisionSphere::DebugCollision(const dgMatrix &matrixPtr,
                                       OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt32 i;
	dgInt32 count;
	dgTriplex pool[1024 * 2];
	dgVector tmpVectex[1024 * 2];

	dgVector p0(dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	            dgFloat32(0.0f));
	dgVector p1(-dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	            dgFloat32(0.0f));
	dgVector p2(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
	            dgFloat32(0.0f));
	dgVector p3(dgFloat32(0.0f), -dgFloat32(1.0f), dgFloat32(0.0f),
	            dgFloat32(0.0f));
	dgVector p4(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f),
	            dgFloat32(0.0f));
	dgVector p5(dgFloat32(0.0f), dgFloat32(0.0f), -dgFloat32(1.0f),
	            dgFloat32(0.0f));

	i = 3;
	count = 0;
	TesselateTriangle(i, p4, p0, p2, count, tmpVectex);
	TesselateTriangle(i, p4, p2, p1, count, tmpVectex);
	TesselateTriangle(i, p4, p1, p3, count, tmpVectex);
	TesselateTriangle(i, p4, p3, p0, count, tmpVectex);
	TesselateTriangle(i, p5, p2, p0, count, tmpVectex);
	TesselateTriangle(i, p5, p1, p2, count, tmpVectex);
	TesselateTriangle(i, p5, p3, p1, count, tmpVectex);
	TesselateTriangle(i, p5, p0, p3, count, tmpVectex);

	for (i = 0; i < count; i++) {
		tmpVectex[i] = tmpVectex[i].Scale(m_radius);
	}

	//  const dgMatrix &matrix = myBody.GetCollisionMatrix();
	dgMatrix matrix(GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex(&pool[0].m_x, sizeof(dgTriplex), &tmpVectex[0].m_x,
	                        sizeof(dgVector), count);
	for (i = 0; i < count; i += 3) {
		callback(userData, 3, &pool[i].m_x, 0);
	}
}

dgFloat32 dgCollisionPoint::GetVolume() const {
	NEWTON_ASSERT(0);
	return dgFloat32(0.0f);
}

void dgCollisionPoint::CalculateInertia(dgVector &inertia,
                                        dgVector &origin) const {
	NEWTON_ASSERT(0);
	//  matrix = dgGetIdentityMatrix();
	inertia.m_x = dgFloat32(0.0f);
	inertia.m_y = dgFloat32(0.0f);
	inertia.m_z = dgFloat32(0.0f);

	origin.m_x = dgFloat32(0.0f);
	origin.m_y = dgFloat32(0.0f);
	origin.m_z = dgFloat32(0.0f);
}

dgVector dgCollisionPoint::SupportVertex(const dgVector &dir) const {
	return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                dgFloat32(0.0f));
}

dgVector dgCollisionPoint::SupportVertexSimd(const dgVector &dir) const {
	NEWTON_ASSERT(0);
	return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                dgFloat32(0.0f));
}

dgFloat32 dgCollisionSphere::RayCast(const dgVector &p0, const dgVector &p1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const {
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgVector dp(p1 - p0);
	dgFloat32 a = dp % dp;
	dgFloat32 b = dgFloat32(2.0f) * (p0 % dp);
	dgFloat32 c = (p0 % p0) - m_radius * m_radius;

	dgFloat32 t = dgFloat32(1.2f);
	dgFloat32 desc = b * b - dgFloat32(4.0f) * a * c;
	if (desc > dgFloat32(0.0f)) {
		desc = dgSqrt(desc);
		a = dgFloat32(1.0f) / (dgFloat32(2.0f) * a);
		dgFloat32 t1 = GetMin((-b + desc) * a, (-b - desc) * a);
		if (t1 < dgFloat32(0.0f)) {
			t1 = dgFloat32(1.2f);
		}
		if (t1 < dgFloat32(1.0f)) {
			t = t1;
			dgVector contact(p0 + dp.Scale(t));
			contactOut.m_normal = contact.Scale(dgRsqrt(contact % contact));
			contactOut.m_userId = SetUserDataID();
		}
	}
	return t;
}

dgFloat32 dgCollisionSphere::RayCastSimd(const dgVector &p0, const dgVector &p1,
        dgContactPoint &contactOut, OnRayPrecastAction preFilter,
        const dgBody *const body, void *const userData) const {
	return RayCast(p0, p1, contactOut, preFilter, body, userData);
}

dgFloat32 dgCollisionSphere::CalculateMassProperties(dgVector &inertia,
        dgVector &crossInertia, dgVector &centerOfMass) const {
	dgFloat32 volume;
	dgFloat32 inerta;

	// volume = dgCollisionConvex::CalculateMassProperties (inertia, crossInertia, centerOfMass);

	centerOfMass = GetOffsetMatrix().m_posit;
	volume = dgFloat32(4.0f * 3.141592f / 3.0f) * m_radius * m_radius * m_radius;
	inerta = dgFloat32(2.0f / 5.0f) * m_radius * m_radius * volume;

	crossInertia.m_x = -volume * centerOfMass.m_y * centerOfMass.m_z;
	crossInertia.m_y = -volume * centerOfMass.m_z * centerOfMass.m_x;
	crossInertia.m_z = -volume * centerOfMass.m_x * centerOfMass.m_y;

	dgVector central(centerOfMass.CompProduct(centerOfMass));
	inertia.m_x = inerta + volume * (central.m_y + central.m_z);
	inertia.m_y = inerta + volume * (central.m_z + central.m_x);
	inertia.m_z = inerta + volume * (central.m_x + central.m_y);

	centerOfMass = centerOfMass.Scale(volume);
	return volume;
}

void dgCollisionSphere::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_sphere.m_r0 = m_radius;
	info->m_sphere.m_r1 = m_radius;
	info->m_sphere.m_r2 = m_radius;
	info->m_offsetMatrix = GetOffsetMatrix();
	//  strcpy (info->m_collisionType, "sphere");
	info->m_collisionType = m_collsionId;
}

void dgCollisionSphere::Serialize(dgSerialize callback,
                                  void *const userData) const {
	dgVector size(m_radius, m_radius, m_radius, dgFloat32(0.0f));

	SerializeLow(callback, userData);
	callback(userData, &size, sizeof(dgVector));
}
