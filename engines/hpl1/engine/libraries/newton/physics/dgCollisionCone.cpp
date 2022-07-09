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

#include "dgCollisionCone.h"
#include "dgBody.h"
#include "dgContact.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgInt32 dgCollisionCone::m_shapeRefCount = 0;
dgConvexSimplexEdge dgCollisionCone::m_edgeArray[DG_CONE_SEGMENTS * 4];

dgCollisionCone::dgCollisionCone(dgMemoryAllocator *allocator,
								 dgUnsigned32 signature, dgFloat32 radius, dgFloat32 height,
								 const dgMatrix &matrix) : dgCollisionConvex(allocator, signature, matrix, m_coneCollision) {
	Init(radius, height);
}

dgCollisionCone::dgCollisionCone(dgWorld *const world,
								 dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	dgVector size;
	deserialization(userData, &size, sizeof(dgVector));
	Init(size.m_x, size.m_y);
}

dgCollisionCone::~dgCollisionCone() {
	m_shapeRefCount--;
	_ASSERTE(m_shapeRefCount >= 0);

	dgCollisionConvex::m_simplex = NULL;
	dgCollisionConvex::m_vertex = NULL;
}

void dgCollisionCone::Init(dgFloat32 radius, dgFloat32 height) {
	m_rtti |= dgCollisionCone_RTTI;
	m_radius = dgAbsf(radius);
	m_sinAngle = m_radius / dgSqrt(height * height + m_radius * m_radius);
	m_height = dgAbsf(height * dgFloat32(0.5f));
	m_amp = dgFloat32(0.5f) * m_radius / m_height;

	dgFloat32 angle = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < DG_CONE_SEGMENTS; i++) {
		dgFloat32 z = dgSin(angle) * m_radius;
		dgFloat32 y = dgCos(angle) * m_radius;
		m_vertex[i] = dgVector(-m_height, y, z, dgFloat32(1.0f));

		angle += dgPI2 / DG_CONE_SEGMENTS;
	}
	m_vertex[DG_CONE_SEGMENTS] = dgVector(m_height, dgFloat32(0.0f),
										  dgFloat32(0.0f), dgFloat32(1.0f));

	m_edgeCount = DG_CONE_SEGMENTS * 4;
	m_vertexCount = DG_CONE_SEGMENTS + 1;
	dgCollisionConvex::m_vertex = m_vertex;

	if (!m_shapeRefCount) {
		dgPolyhedra polyhedra(m_allocator);
		dgInt32 wireframe[DG_CONE_SEGMENTS];

		dgInt32 j = DG_CONE_SEGMENTS - 1;
		polyhedra.BeginFace();
		for (dgInt32 i = 0; i < DG_CONE_SEGMENTS; i++) {
			wireframe[0] = j;
			wireframe[1] = i;
			wireframe[2] = DG_CONE_SEGMENTS;
			j = i;
			polyhedra.AddFace(3, wireframe);
		}

		for (dgInt32 i = 0; i < DG_CONE_SEGMENTS; i++) {
			wireframe[i] = DG_CONE_SEGMENTS - 1 - i;
		}
		polyhedra.AddFace(DG_CONE_SEGMENTS, wireframe);
		polyhedra.EndFace();

		_ASSERTE(SanityCheck(polyhedra));

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
}

dgInt32 dgCollisionCone::CalculateSignature() const {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_cylinderCollision;
	buffer[1] = dgCollision::Quantize(m_radius);
	buffer[2] = dgCollision::Quantize(m_height);
	memcpy(&buffer[3], &m_offset, sizeof(dgMatrix));
	return dgInt32(dgCollision::MakeCRC(buffer, sizeof(buffer)));
}

void dgCollisionCone::DebugCollision(const dgMatrix &matrixPtr,
									 OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt32 i;
	dgInt32 j;
	dgFloat32 y;
	dgFloat32 z;
	dgFloat32 angle;

#define NUMBER_OF_DEBUG_SEGMENTS 24
	dgTriplex pool[NUMBER_OF_DEBUG_SEGMENTS + 1];
	dgTriplex face[NUMBER_OF_DEBUG_SEGMENTS];

	angle = dgFloat32(0.0f);
	for (i = 0; i < NUMBER_OF_DEBUG_SEGMENTS; i++) {
		z = dgSin(angle) * m_radius;
		y = dgCos(angle) * m_radius;
		pool[i].m_x = -m_height;
		pool[i].m_y = y;
		pool[i].m_z = z;
		angle += dgPI2 / dgFloat32(NUMBER_OF_DEBUG_SEGMENTS);
	}

	pool[i].m_x = m_height;
	pool[i].m_y = dgFloat32(0.0f);
	pool[i].m_z = dgFloat32(0.0f);

	//	const dgMatrix &matrix = myBody.GetCollisionMatrix();
	dgMatrix matrix(GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex(&pool[0].m_x, sizeof(dgTriplex), &pool[0].m_x,
							sizeof(dgTriplex), NUMBER_OF_DEBUG_SEGMENTS + 1);

	j = NUMBER_OF_DEBUG_SEGMENTS - 1;
	for (i = 0; i < NUMBER_OF_DEBUG_SEGMENTS; i++) {
		face[0] = pool[j];
		face[1] = pool[i];
		face[2] = pool[NUMBER_OF_DEBUG_SEGMENTS];
		j = i;
		callback(userData, 3, &face[0].m_x, 0);
	}

	for (i = 0; i < NUMBER_OF_DEBUG_SEGMENTS; i++) {
		face[i] = pool[NUMBER_OF_DEBUG_SEGMENTS - 1 - i];
	}
	callback(userData, NUMBER_OF_DEBUG_SEGMENTS, &face[0].m_x, 0);
}

void dgCollisionCone::SetCollisionBBox(const dgVector &p0__,
									   const dgVector &p1__) {
	_ASSERTE(0);
}

dgVector dgCollisionCone::SupportVertexSimd(const dgVector &dir) const {
	return SupportVertex(dir);
}

dgVector dgCollisionCone::SupportVertex(const dgVector &dir) const {
	_ASSERTE(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-3f));

	if (dir.m_x > m_sinAngle) {
		return dgVector(m_height, dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	}

	dgFloat32 y0 = m_radius;
	dgFloat32 z0 = dgFloat32(0.0f);
	dgFloat32 mag2 = dir.m_y * dir.m_y + dir.m_z * dir.m_z;
	if (mag2 > dgFloat32(1.0e-12f)) {
		mag2 = dgRsqrt(mag2);
		y0 = dir.m_y * m_radius * mag2;
		z0 = dir.m_z * m_radius * mag2;
	}
	return dgVector(-m_height, y0, z0, dgFloat32(0.0f));
}

// dgVector dgCollisionCone::GetLocalCG () const
//{
//	return dgVector (-dgFloat32 (0.5f) * m_height, dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
// }

dgFloat32 dgCollisionCone::CalculateMassProperties(dgVector &inertia,
												   dgVector &crossInertia, dgVector &centerOfMass) const {
	dgFloat32 volume;
	dgFloat32 inertaxx;
	dgFloat32 inertayyzz;

	// dgVector centerOfMass1;
	// dgVector inertia1;
	// dgVector crossInertia1;
	// volume = dgCollisionConvex::CalculateMassProperties (inertia1, crossInertia1, centerOfMass1);

	volume = dgFloat32(3.1616f * 2.0f / 3.0f) * m_radius * m_radius * m_height;

	centerOfMass = GetOffsetMatrix().m_posit - GetOffsetMatrix().m_front.Scale(dgFloat32(0.5f) * m_height);

	inertaxx = dgFloat32(3.0f / 10.0f) * m_radius * m_radius * volume;
	inertayyzz = (dgFloat32(3.0f / 20.0f) * m_radius * m_radius + dgFloat32(4.0f / 10.0f) * m_height * m_height) * volume;

	dgMatrix inertiaTensor(dgGetIdentityMatrix());

	inertiaTensor[0][0] = inertaxx;
	inertiaTensor[1][1] = inertayyzz;
	inertiaTensor[2][2] = inertayyzz;

	inertiaTensor = GetOffsetMatrix().Inverse() * inertiaTensor * GetOffsetMatrix();

	crossInertia.m_x = inertiaTensor[1][2] - volume * centerOfMass.m_y * centerOfMass.m_z;
	crossInertia.m_y = inertiaTensor[0][2] - volume * centerOfMass.m_z * centerOfMass.m_x;
	crossInertia.m_z = inertiaTensor[0][1] - volume * centerOfMass.m_x * centerOfMass.m_y;

	dgVector central(centerOfMass.CompProduct(centerOfMass));
	inertia.m_x = inertiaTensor[0][0] + volume * (central.m_y + central.m_z);
	inertia.m_y = inertiaTensor[1][1] + volume * (central.m_z + central.m_x);
	inertia.m_z = inertiaTensor[2][2] + volume * (central.m_x + central.m_y);

	centerOfMass = centerOfMass.Scale(volume);
	return volume;
}

dgInt32 dgCollisionCone::CalculatePlaneIntersection(const dgVector &normal,
													const dgVector &origin, dgVector *const contactsOut) const {
	dgInt32 i;
	dgInt32 count;
	dgFloat32 y;
	dgFloat32 z;
	dgFloat32 cosAng;
	dgFloat32 sinAng;
	dgFloat32 magInv;

	if (dgAbsf(normal.m_x) < dgFloat32(0.999f)) {
		//		magInv = dgRsqrt (normal.m_y * normal.m_y + normal.m_z * normal.m_z);
		//		cosAng = normal.m_y * magInv;
		//		sinAng = normal.m_z * magInv;
		//		dgMatrix matrix (dgGetIdentityMatrix ());
		//		matrix[1][1] = cosAng;
		//		matrix[1][2] = sinAng;
		//		matrix[2][1] = -sinAng;
		//		matrix[2][2] = cosAng;
		//		dgVector normal2 (matrix.UnrotateVector (normal));
		//		dgVector origin2 (matrix.UnrotateVector (origin));
		//		count = dgCollisionConvex::CalculatePlaneIntersection (normal1, origin1, contactsOut);
		//		matrix.TransformTriplex (contactsOut, sizeof (dgVector), contactsOut, sizeof (dgVector), count);

		magInv = dgRsqrt(normal.m_y * normal.m_y + normal.m_z * normal.m_z);
		cosAng = normal.m_y * magInv;
		sinAng = normal.m_z * magInv;
		_ASSERTE(
			dgAbsf(normal.m_z * cosAng - normal.m_y * sinAng) < dgFloat32(1.0e-4f));
		//		dgVector normal1 (normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng,
		//									  normal.m_z * cosAng - normal.m_y * sinAng, dgFloat32 (0.0f));
		dgVector normal1(normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng,
						 dgFloat32(0.0f), dgFloat32(0.0f));
		dgVector origin1(origin.m_x, origin.m_y * cosAng + origin.m_z * sinAng,
						 origin.m_z * cosAng - origin.m_y * sinAng, dgFloat32(0.0f));
		count = dgCollisionConvex::CalculatePlaneIntersection(normal1, origin1,
															  contactsOut);
		for (i = 0; i < count; i++) {
			y = contactsOut[i].m_y;
			z = contactsOut[i].m_z;
			contactsOut[i].m_y = y * cosAng - z * sinAng;
			contactsOut[i].m_z = z * cosAng + y * sinAng;
		}

	} else {
		count = dgCollisionConvex::CalculatePlaneIntersection(normal, origin,
															  contactsOut);
	}

	return count;
}

dgInt32 dgCollisionCone::CalculatePlaneIntersectionSimd(const dgVector &normal,
														const dgVector &origin, dgVector *const contactsOut) const {
#ifdef DG_BUILD_SIMD_CODE
	dgInt32 i;
	dgInt32 count;
	dgFloat32 y;
	dgFloat32 z;
	dgFloat32 cosAng;
	dgFloat32 sinAng;
	dgFloat32 magInv;
	simd_type tmp0;
	simd_type mag2;

	if (dgAbsf(normal.m_x) < dgFloat32(0.999f)) {
		//		magInv = dgRsqrt (normal.m_y * normal.m_y + normal.m_z * normal.m_z);
		//		cosAng = normal.m_y * magInv;
		//		sinAng = normal.m_z * magInv;
		//		dgMatrix matrix (dgGetIdentityMatrix ());
		//		matrix[1][1] = cosAng;
		//		matrix[1][2] = sinAng;
		//		matrix[2][1] = -sinAng;
		//		matrix[2][2] = cosAng;
		//		dgVector normal2 (matrix.UnrotateVector (normal));
		//		dgVector origin2 (matrix.UnrotateVector (origin));
		//		count = dgCollisionConvex::CalculatePlaneIntersection (normal1, origin1, contactsOut);
		//		matrix.TransformTriplex (contactsOut, sizeof (dgVector), contactsOut, sizeof (dgVector), count);

		y = normal.m_y * normal.m_y + normal.m_z * normal.m_z;
		mag2 = simd_load_s(y);
		tmp0 = simd_rsqrt_s(mag2);
		simd_store_s(
			simd_mul_s(simd_mul_s(*(simd_type *)&m_nrh0p5, tmp0), simd_mul_sub_s(*(simd_type *)&m_nrh3p0, simd_mul_s(mag2, tmp0), tmp0)),
			&magInv);

		cosAng = normal.m_y * magInv;
		sinAng = normal.m_z * magInv;
		_ASSERTE(
			dgAbsf(normal.m_z * cosAng - normal.m_y * sinAng) < dgFloat32(1.0e-4f));
		//		dgVector normal1 (normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng,
		//									  normal.m_z * cosAng - normal.m_y * sinAng, dgFloat32 (0.0f));
		dgVector normal1(normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng,
						 dgFloat32(0.0f), dgFloat32(0.0f));
		dgVector origin1(origin.m_x, origin.m_y * cosAng + origin.m_z * sinAng,
						 origin.m_z * cosAng - origin.m_y * sinAng, dgFloat32(0.0f));

		count = dgCollisionConvex::CalculatePlaneIntersectionSimd(normal1, origin1,
																  contactsOut);
		for (i = 0; i < count; i++) {
			y = contactsOut[i].m_y;
			z = contactsOut[i].m_z;
			contactsOut[i].m_y = y * cosAng - z * sinAng;
			contactsOut[i].m_z = z * cosAng + y * sinAng;
		}

	} else {
		count = dgCollisionConvex::CalculatePlaneIntersectionSimd(normal, origin,
																  contactsOut);
	}

	return count;

#else
	return 0;
#endif
}

void dgCollisionCone::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_cone.m_r = m_radius;
	info->m_cone.m_height = m_height * dgFloat32(2.0f);
	info->m_offsetMatrix = GetOffsetMatrix();
	//	strcpy (info->m_collisionType, "cone");
	info->m_collisionType = m_collsionId;
}

void dgCollisionCone::Serialize(dgSerialize callback,
								void *const userData) const {
	dgVector size(m_radius, m_height * dgFloat32(2.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));
	SerializeLow(callback, userData);
	callback(userData, &size, sizeof(dgVector));
}
