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

#include "dgCollisionCylinder.h"
#include "dgBody.h"
#include "dgContact.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgInt32 dgCollisionCylinder::m_shapeRefCount = 0;
dgConvexSimplexEdge dgCollisionCylinder::m_edgeArray[DG_CYLINDER_SEGMENTS * 2 * 3];

dgCollisionCylinder::dgCollisionCylinder(dgMemoryAllocator *allocator,
										 dgUnsigned32 signature, dgFloat32 radius, dgFloat32 height,
										 const dgMatrix &matrix) : dgCollisionConvex(allocator, signature, matrix, m_cylinderCollision) {
	Init(radius, height);
}

dgCollisionCylinder::dgCollisionCylinder(dgWorld *const world,
										 dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	dgVector size;
	deserialization(userData, &size, sizeof(dgVector));
	Init(size.m_x, size.m_y);
}

void dgCollisionCylinder::Init(dgFloat32 radius, dgFloat32 height) {
	//	dgInt32 i;
	//	dgInt32 j;
	//	dgEdge *edge;
	//	dgFloat32 y;
	//	dgFloat32 z;
	//	dgFloat32 angle;

	m_rtti |= dgCollisionCylinder_RTTI;
	m_radius = dgAbsf(radius);
	m_height[0] = dgAbsf(height * dgFloat32(0.5f));
	m_height[1] = -m_height[0];

	dgFloat32 angle = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < DG_CYLINDER_SEGMENTS; i++) {
		dgFloat32 z = dgSin(angle) * m_radius;
		dgFloat32 y = dgCos(angle) * m_radius;
		m_vertex[i] = dgVector(-m_height[0], y, z, dgFloat32(1.0f));
		m_vertex[i + DG_CYLINDER_SEGMENTS] = dgVector(m_height[0], y, z,
													  dgFloat32(1.0f));
		angle += dgPI2 / DG_CYLINDER_SEGMENTS;
	}

	m_edgeCount = DG_CYLINDER_SEGMENTS * 6;
	m_vertexCount = DG_CYLINDER_SEGMENTS * 2;
	dgCollisionConvex::m_vertex = m_vertex;

	if (!m_shapeRefCount) {
		dgPolyhedra polyhedra(m_allocator);
		dgInt32 wireframe[DG_CYLINDER_SEGMENTS];

		dgInt32 j = DG_CYLINDER_SEGMENTS - 1;
		polyhedra.BeginFace();
		for (dgInt32 i = 0; i < DG_CYLINDER_SEGMENTS; i++) {
			wireframe[0] = j;
			wireframe[1] = i;
			wireframe[2] = i + DG_CYLINDER_SEGMENTS;
			wireframe[3] = j + DG_CYLINDER_SEGMENTS;
			j = i;
			polyhedra.AddFace(4, wireframe);
		}

		for (dgInt32 i = 0; i < DG_CYLINDER_SEGMENTS; i++) {
			wireframe[i] = DG_CYLINDER_SEGMENTS - 1 - i;
		}
		polyhedra.AddFace(DG_CYLINDER_SEGMENTS, wireframe);

		for (dgInt32 i = 0; i < DG_CYLINDER_SEGMENTS; i++) {
			wireframe[i] = i + DG_CYLINDER_SEGMENTS;
		}
		polyhedra.AddFace(DG_CYLINDER_SEGMENTS, wireframe);
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

dgCollisionCylinder::~dgCollisionCylinder() {
	m_shapeRefCount--;
	_ASSERTE(m_shapeRefCount >= 0);

	dgCollisionConvex::m_simplex = NULL;
	dgCollisionConvex::m_vertex = NULL;
}

dgInt32 dgCollisionCylinder::CalculateSignature() const {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_cylinderCollision;
	buffer[1] = dgCollision::Quantize(m_radius);
	buffer[2] = dgCollision::Quantize(m_height[0]);
	memcpy(&buffer[3], &m_offset, sizeof(dgMatrix));
	return dgInt32(dgCollision::MakeCRC(buffer, sizeof(buffer)));
}

void dgCollisionCylinder::SetCollisionBBox(const dgVector &p0__,
										   const dgVector &p1__) {
	_ASSERTE(0);
}

void dgCollisionCylinder::DebugCollision(const dgMatrix &matrixPtr,
										 OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt32 i;
	dgInt32 j;
	dgFloat32 y;
	dgFloat32 z;
	dgFloat32 angle;
	dgTriplex pool[24 * 2];

	angle = dgFloat32(0.0f);
	for (i = 0; i < 24; i++) {
		z = dgSin(angle) * m_radius;
		y = dgCos(angle) * m_radius;
		pool[i].m_x = -m_height[0];
		pool[i].m_y = y;
		pool[i].m_z = z;
		pool[i + 24].m_x = m_height[0];
		pool[i + 24].m_y = y;
		pool[i + 24].m_z = z;
		angle += dgPI2 / dgFloat32(24.0f);
	}

	dgMatrix matrix(GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex(&pool[0].m_x, sizeof(dgTriplex), &pool[0].m_x,
							sizeof(dgTriplex), 24 * 2);

	dgTriplex face[24];

	j = 24 - 1;
	for (i = 0; i < 24; i++) {
		face[0] = pool[j];
		face[1] = pool[i];
		face[2] = pool[i + 24];
		face[3] = pool[j + 24];
		j = i;
		callback(userData, 4, &face[0].m_x, 0);
	}

	for (i = 0; i < 24; i++) {
		face[i] = pool[24 - 1 - i];
	}
	callback(userData, 24, &face[0].m_x, 0);

	for (i = 0; i < 24; i++) {
		face[i] = pool[i + 24];
	}
	callback(userData, 24, &face[0].m_x, 0);
}

dgVector dgCollisionCylinder::SupportVertexSimd(const dgVector &dir) const {
	return SupportVertex(dir);
}

dgVector dgCollisionCylinder::SupportVertex(const dgVector &dir) const {
	/*
	 dgInt32 index;
	 dgFloat32 y0;
	 dgFloat32 z0;
	 dgFloat32 y1;
	 dgFloat32 z1;
	 dgFloat32 dist0;
	 dgFloat32 dist1;

	 _ASSERTE (dgAbsf ((dir % dir - dgFloat32 (1.0f))) < dgFloat32 (1.0e-3f));

	 //	sign = dir.m_x > dgFloat32 (0.0f) ? dgFloat32 (1.0f) : -dgFloat32 (1.0f);
	 dgFloatSign *ptr = (dgFloatSign*) &dir;
	 index = -(ptr[0].m_integer.m_iVal >> 31);
	 dgSinCos (m_tethaStep * dgFloor (dgAtan2 (dir.m_y, dir.m_z) * m_tethaStepInv), y0, z0);

	 y0 *= m_radius;
	 z0 *= m_radius;

	 y1 = y0 * m_delCosTetha + z0 * m_delSinTetha;
	 z1 = z0 * m_delCosTetha - y0 * m_delSinTetha;

	 dist0 = dir.m_y * y0 + dir.m_z * z0;
	 dist1 = dir.m_y * y1 + dir.m_z * z1;
	 if (dist1 > dist0) {
	 y0 = y1;
	 z0 = z1;
	 }
	 return dgVector (m_height[index], y0, z0, dgFloat32 (0.0f));
	 */

	dgInt32 index;
	dgFloat32 y0;
	dgFloat32 z0;
	dgFloat32 mag2;
	dgFloatSign const *ptr = (dgFloatSign *)&dir;

	_ASSERTE(dgAbsf((dir % dir - dgFloat32(1.0f))) < dgFloat32(1.0e-3f));

	y0 = m_radius;
	z0 = dgFloat32(0.0f);
	mag2 = dir.m_y * dir.m_y + dir.m_z * dir.m_z;
	if (mag2 > dgFloat32(1.0e-12f)) {
		mag2 = dgRsqrt(mag2);
		y0 = dir.m_y * m_radius * mag2;
		z0 = dir.m_z * m_radius * mag2;
	}
	index = -(ptr[0].m_integer.m_iVal >> 31);

	return dgVector(m_height[index], y0, z0, dgFloat32(0.0f));
}

/*
 dgVector dgCollisionCylinder::ImplicitCylindexSupport (const dgVector& dir) const
 {
 dgFloat32 sign;
 dgFloat32 invMag;

 _ASSERTE ((dir % dir - dgFloat32 (dgFloat32 (1.0f))) < dgFloat32 (1.0e-3f));
 sign = dir.m_x > dgFloat32 (0.0f) ? dgFloat32 (dgFloat32 (1.0f)) : -dgFloat32 (dgFloat32 (1.0f));

 invMag = m_radius * dgRsqrt (dir.m_y * dir.m_y + dir.m_z * dir.m_z + 1.0e-12f) ;
 return dgVector (m_height[0] * sign, invMag * dir.m_y, invMag * dir.m_z, dgFloat32 (0.0f));
 }
 */

dgFloat32 dgCollisionCylinder::CalculateMassProperties(dgVector &inertia,
													   dgVector &crossInertia, dgVector &centerOfMass) const {
	dgFloat32 volume;
	dgFloat32 inertaxx;
	dgFloat32 inertayyzz;

	// volume = dgCollisionConvex::CalculateMassProperties (inertia, crossInertia, centerOfMass);

	centerOfMass = GetOffsetMatrix().m_posit;
	volume = dgFloat32(3.1616f * 2.0f) * m_radius * m_radius * m_height[0];

	inertaxx = dgFloat32(0.5f) * m_radius * m_radius * volume;
	inertayyzz = (dgFloat32(0.25f) * m_radius * m_radius + dgFloat32(1.0f / 3.0f) * m_height[0] * m_height[0]) * volume;

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

dgInt32 dgCollisionCylinder::CalculatePlaneIntersection(const dgVector &normal,
														const dgVector &origin, dgVector *const contactsOut) const {
	dgInt32 count;
	if (dgAbsf(normal.m_x) < dgFloat32(0.999f)) {
		dgFloat32 magInv =
			dgRsqrt(normal.m_y * normal.m_y + normal.m_z * normal.m_z);
		dgFloat32 cosAng = normal.m_y * magInv;
		dgFloat32 sinAng = normal.m_z * magInv;

		_ASSERTE(
			dgAbsf(normal.m_z * cosAng - normal.m_y * sinAng) < dgFloat32(1.0e-4f));
		dgVector normal1(normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng,
						 dgFloat32(0.0f), dgFloat32(0.0f));
		dgVector origin1(origin.m_x, origin.m_y * cosAng + origin.m_z * sinAng,
						 origin.m_z * cosAng - origin.m_y * sinAng, dgFloat32(0.0f));

		count = dgCollisionConvex::CalculatePlaneIntersection(normal1, origin1,
															  contactsOut);
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

dgInt32 dgCollisionCylinder::CalculatePlaneIntersectionSimd(
	const dgVector &normal, const dgVector &origin,
	dgVector *const contactsOut) const {
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
		y = normal.m_y * normal.m_y + normal.m_z * normal.m_z;
		mag2 = simd_load_s(y);
		tmp0 = simd_rsqrt_s(mag2);
		simd_store_s(
			simd_mul_s(simd_mul_s(*(simd_type *)&m_nrh0p5, tmp0), simd_mul_sub_s(*(simd_type *)&m_nrh3p0, simd_mul_s(mag2, tmp0), tmp0)),
			&magInv);

		cosAng = normal.m_y * magInv;
		sinAng = normal.m_z * magInv;

		//		dgMatrix matrix (dgGetIdentityMatrix ());
		//		matrix[1][1] = cosAng;
		//		matrix[1][2] = sinAng;
		//		matrix[2][1] = -sinAng;
		//		matrix[2][2] = cosAng;
		//		dgVector normal1 (matrix.UnrotateVector (normal));
		//		dgVector origin1 (matrix.UnrotateVector (origin));
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
		//		matrix.TransformTriplex (contactsOut, sizeof (dgVector), contactsOut, sizeof (dgVector), count);
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

dgFloat32 dgCollisionCylinder::RayCast(const dgVector &q0, const dgVector &q1,
									   dgContactPoint &contactOut, OnRayPrecastAction preFilter,
									   const dgBody *const body, void *const userData) const {
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgFloat32 t = dgFloat32(1.2f);
	dgVector p0(q0);
	p0.m_x = dgFloat32(0.0f);
	dgFloat32 c = (p0 % p0) - m_radius * m_radius;
	if (c > dgFloat32(0.0f)) {

		dgVector dp(q1 - q0);
		dp.m_x = dgFloat32(0.0f);
		dgFloat32 a = dp % dp;
		dgFloat32 b = dgFloat32(2.0f) * (p0 % dp);

		dgFloat32 desc = b * b - dgFloat32(4.0f) * a * c;
		if (desc > 1.0e-8f) {
			desc = dgSqrt(desc);
			a = dgFloat32(1.0f) / (dgFloat32(2.0f) * a);
			dgFloat32 t1 = GetMin((-b + desc) * a, (-b - desc) * a);
			if ((t1 < dgFloat32(1.0f)) && (t1 >= dgFloat32(0.0f))) {
				dgVector dq(q1 - q0);
				dgVector contact(q0 + dq.Scale(t1));
				if (contact.m_x > m_height[0]) {
					if (q1.m_x < m_height[0]) {
						t1 = (m_height[0] - q0.m_x) / (q1.m_x - q0.m_x);
						dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
						dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
						if ((y * y + z * z - m_radius * m_radius) < dgFloat32(0.0f)) {
							t = t1;
							contactOut.m_normal = dgVector(dgFloat32(dgFloat32(1.0f)),
														   dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
							contactOut.m_userId = SetUserDataID();
						}
					}
				} else if (contact.m_x < -m_height[0]) {
					if (q1.m_x > -m_height[0]) {
						t1 = (-m_height[0] - q0.m_x) / (q1.m_x - q0.m_x);
						dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
						dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
						if ((y * y + z * z - m_radius * m_radius) < dgFloat32(0.0f)) {
							t = t1;
							contactOut.m_normal = dgVector(-dgFloat32(dgFloat32(1.0f)),
														   dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
							contactOut.m_userId = SetUserDataID();
						}
					}
				} else if (t1 >= dgFloat32(0.0f)) {
					t = t1;
					dgVector n(contact);
					n.m_x = dgFloat32(0.0f);
					contactOut.m_normal = n.Scale(dgRsqrt(n % n));
					contactOut.m_userId = SetUserDataID();
				}
			}
		}
	} else {
		if (q0.m_x > m_height[0]) {
			if (q1.m_x < m_height[0]) {
				dgFloat32 t1 = (m_height[0] - q0.m_x) / (q1.m_x - q0.m_x);
				dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
				dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
				if ((y * y + z * z - m_radius * m_radius) < dgFloat32(0.0f)) {
					t = t1;
					contactOut.m_normal = dgVector(dgFloat32(dgFloat32(1.0f)),
												   dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
					contactOut.m_userId = SetUserDataID();
				}
			}
		} else if (q0.m_x < -m_height[0]) {
			if (q1.m_x > -m_height[0]) {
				dgFloat32 t1 = (-m_height[0] - q0.m_x) / (q1.m_x - q0.m_x);
				dgFloat32 y = q0.m_y + (q1.m_y - q0.m_y) * t1;
				dgFloat32 z = q0.m_z + (q1.m_z - q0.m_z) * t1;
				if ((y * y + z * z - m_radius * m_radius) < dgFloat32(0.0f)) {
					t = t1;
					contactOut.m_normal = dgVector(-dgFloat32(dgFloat32(1.0f)),
												   dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
					contactOut.m_userId = SetUserDataID();
				}
			}
		}
	}
	return t;
}

dgFloat32 dgCollisionCylinder::RayCastSimd(const dgVector &q0,
										   const dgVector &q1, dgContactPoint &contactOut,
										   OnRayPrecastAction preFilter, const dgBody *const body,
										   void *const userData) const {
	return RayCast(q0, q1, contactOut, preFilter, body, userData);
}

void dgCollisionCylinder::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_cylinder.m_r0 = m_radius;
	info->m_cylinder.m_r1 = m_radius;
	info->m_cylinder.m_height = m_height[0] * dgFloat32(2.0f);
	info->m_offsetMatrix = GetOffsetMatrix();
	//	strcpy (info->m_collisionType, "cylinder");
	info->m_collisionType = m_collsionId;
}

void dgCollisionCylinder::Serialize(dgSerialize callback,
									void *const userData) const {
	dgVector size(dgAbsf(m_radius), m_height[0] * dgFloat32(2.0f),
				  dgFloat32(0.0f), dgFloat32(0.0f));
	SerializeLow(callback, userData);
	callback(userData, &size, sizeof(dgVector));
}
