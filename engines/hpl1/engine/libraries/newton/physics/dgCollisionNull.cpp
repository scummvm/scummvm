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

#include "dgCollisionNull.h"
#include "dgBody.h"
#include "dgContact.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgCollisionNull::dgCollisionNull(dgMemoryAllocator *const allocator,
								 dgUnsigned32 signature) : dgCollisionConvex(allocator, signature, dgGetIdentityMatrix(),
																			 m_nullCollision) {
	m_rtti |= dgCollisionNull_RTTI;
}

dgCollisionNull::dgCollisionNull(dgWorld *const world,
								 dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	m_rtti |= dgCollisionNull_RTTI;
}

void dgCollisionNull::Serialize(dgSerialize callback,
								void *const userData) const {
	SerializeLow(callback, userData);
}

dgCollisionNull::~dgCollisionNull() {
}

void dgCollisionNull::SetCollisionBBox(const dgVector &p0__,
									   const dgVector &p1__) {
	_ASSERTE(0);
}

void dgCollisionNull::DebugCollision(const dgMatrix &matrixPtr,
									 OnDebugCollisionMeshCallback callback, void *const userData) const {
}

dgInt32 dgCollisionNull::CalculateSignature() const {
	return dgInt32(GetSignature());
}

void dgCollisionNull::CalcAABB(const dgMatrix &matrix, dgVector &p0,
							   dgVector &p1) const {
	p0.m_x = matrix[3][0];
	p1.m_x = matrix[3][0];

	p0.m_y = matrix[3][1];
	p1.m_y = matrix[3][1];

	p0.m_z = matrix[3][2];
	p1.m_z = matrix[3][2];

	p0.m_w = 1.0f;
	p1.m_w = 1.0f;
}

void dgCollisionNull::CalcAABBSimd(const dgMatrix &matrix, dgVector &p0,
								   dgVector &p1) const {
	CalcAABB(matrix, p0, p1);
}

dgVector dgCollisionNull::SupportVertex(const dgVector &dir) const {
	_ASSERTE(0);
	return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					dgFloat32(0.0f));
}

dgVector dgCollisionNull::SupportVertexSimd(const dgVector &dir) const {
	_ASSERTE(0);
	return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					dgFloat32(0.0f));
}

dgFloat32 dgCollisionNull::GetVolume() const {
	return 0.0f;
}

dgFloat32 dgCollisionNull::RayCast(const dgVector &localP0,
								   const dgVector &localP1, dgContactPoint &contactOut,
								   OnRayPrecastAction preFilter, const dgBody *const body,
								   void *const userData) const {
	return dgFloat32(1.2f);
}

dgFloat32 dgCollisionNull::RayCastSimd(const dgVector &localP0,
									   const dgVector &localP1, dgContactPoint &contactOut,
									   OnRayPrecastAction preFilter, const dgBody *const body,
									   void *const userData) const {
	return dgFloat32(1.2f);
}

dgVector dgCollisionNull::CalculateVolumeIntegral(const dgMatrix &matrix__,
												  GetBuoyancyPlane buoyancuPlane__, void *context__) const {
	_ASSERTE(0);
	return dgVector(0.0f, 0.0f, 0.0f, 0.0f);
}

void dgCollisionNull::CalculateInertia(dgVector &inertia,
									   dgVector &origin) const {
	inertia.m_x = dgFloat32(0.0f);
	inertia.m_y = dgFloat32(0.0f);
	inertia.m_z = dgFloat32(0.0f);

	origin.m_x = dgFloat32(0.0f);
	origin.m_y = dgFloat32(0.0f);
	origin.m_z = dgFloat32(0.0f);
}
