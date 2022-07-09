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

#include "dgCollision.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


// dgInitRtti(dgCollision);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
dgCollision::dgCollision(dgMemoryAllocator *const allocator,
						 dgUnsigned32 signature, const dgMatrix &matrix, dgCollisionID id)
//	:dgRef()
{
	m_rtti = 0;
	m_refCount = 1;
	m_userDataID = 0;
	m_collsionId = id;
	m_signature = signature;
	m_allocator = allocator;
	//	m_maxDistanceTravel = dgFloat32 (0.0f);
	//	m_omegaDistanceBound = dgFloat32 (0.0f);
	SetOffsetMatrix(matrix);
}

dgCollision::dgCollision(dgWorld *const world, dgDeserialize deserialization,
						 void *const userData) {
	dgInt32 signature[4];
	deserialization(userData, &signature, sizeof(signature));
	deserialization(userData, &m_offset, sizeof(dgMatrix));

	m_rtti = 0;
	m_refCount = 1;
	m_signature = dgUnsigned32(signature[0]);
	m_userDataID = dgUnsigned32(signature[2]);
	m_allocator = world->GetAllocator();
	m_collsionId = dgCollisionID(signature[1]);
}

dgCollision::~dgCollision() {
}

dgUnsigned32 dgCollision::Quantize(dgFloat32 value) {
	return dgUnsigned32(value * 1024.0f);
}

dgUnsigned32 dgCollision::MakeCRC(void *buffer, int size) {
	dgUnsigned32 crc;

	crc = dgCRC(buffer, size);
	return crc;
}

void dgCollision::SetOffsetMatrix(const dgMatrix &matrix) {
	m_offset = matrix;
	m_offset[0][3] = dgFloat32(0.0f);
	m_offset[1][3] = dgFloat32(0.0f);
	m_offset[2][3] = dgFloat32(0.0f);
	m_offset[3][3] = dgFloat32(1.0f);

#ifdef _DEBUG
	dgFloat32 det;
	det = (m_offset.m_front * m_offset.m_up) % m_offset.m_right;
	_ASSERTE(det > dgFloat32(0.999f));
	_ASSERTE(det < dgFloat32(1.001f));
#endif
}

void *dgCollision::GetUserData() const {
	return NULL;
}

void dgCollision::SetUserData(void *const userData) {
}

void dgCollision::GetCollisionInfo(dgCollisionInfo *info) const {
	//	memset (info, 0, sizeof (dgCollisionInfo));
	info->m_offsetMatrix = dgGetIdentityMatrix();
	info->m_collisionType = m_collsionId;
	info->m_refCount = GetRefCount();
	info->m_userDadaID = dgInt32(SetUserDataID());
}

void dgCollision::SerializeLow(dgSerialize callback, void *const userData) const {
	dgInt32 signature[4];
	signature[0] = dgInt32(GetSignature());
	signature[1] = GetCollisionPrimityType();
	signature[2] = dgInt32(SetUserDataID());
	signature[3] = 0;

	callback(userData, &signature, sizeof(signature));
	callback(userData, &m_offset, sizeof(dgMatrix));
}

/*
 void dgCollision::Serialize(dgSerialize callback, void* const userData) const
 {
 _ASSERTE (0);
 }
 */
