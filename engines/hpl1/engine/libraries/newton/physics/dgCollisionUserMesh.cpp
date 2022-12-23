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

#include "dgCollisionUserMesh.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


dgCollisionUserMesh::dgCollisionUserMesh(dgMemoryAllocator *allocator,
        const dgVector &boxP0, const dgVector &boxP1,
        const dgUserMeshCreation &data) : dgCollisionMesh(allocator, m_userMeshCollision) {
	m_rtti |= dgCollisionUserMesh_RTTI;

	m_userData = data.m_userData;
	m_getInfo = data.m_getInfo;
	m_faceInAabb = data.m_faceInAabb;
	m_rayHitCallBack = data.m_rayHitCallBack;
	m_collideCallback = data.m_collideCallback;
	m_destroyCallback = data.m_destroyCallback;

	SetCollisionBBox(boxP0, boxP1);
}

dgCollisionUserMesh::dgCollisionUserMesh(dgWorld *const world,
        dgDeserialize deserialization, void *const userData) : dgCollisionMesh(world, deserialization, userData) {
	NEWTON_ASSERT(0);
	m_rtti |= dgCollisionUserMesh_RTTI;

	/*
	 dgAABBPolygonSoup::Deserialize (deserialization, userData);

	 dgVector p0;
	 dgVector p1;
	 GetAABB (p0, p1);
	 SetCollisionBBox(p0, p1);
	 */
}

dgCollisionUserMesh::~dgCollisionUserMesh(void) {
	if (m_destroyCallback) {
		m_destroyCallback(m_userData);
	}
}

void dgCollisionUserMesh::Serialize(dgSerialize callback,
                                    void *const userData) const {
	NEWTON_ASSERT(0);
	/*
	 SerializeLow(callback, userData);
	 dgAABBPolygonSoup::Serialize ((dgSerialize) callback, userData);
	 */
}

void dgCollisionUserMesh::GetVertexListIndexList(const dgVector &p0,
        const dgVector &p1, dgGetVertexListIndexList &data) const {
	if (m_faceInAabb) {
		return m_faceInAabb(m_userData, &p0[0], &p1[0],
		                    (const dgFloat32 * const *)&data.m_veterxArray, &data.m_vertexCount,
		                    &data.m_vertexStrideInBytes, data.m_indexList, data.m_maxIndexCount,
		                    data.m_userDataList);

	} else {
		data.m_triangleCount = 0;
	}
}

void dgCollisionUserMesh::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollision::GetCollisionInfo(info);
	info->m_offsetMatrix = GetOffsetMatrix();
	if (m_getInfo) {
		m_getInfo(m_userData, info);
	}
}

dgFloat32 dgCollisionUserMesh::RayCastSimd(const dgVector &localP0,
        const dgVector &localP1, dgContactPoint &contactOut,
        OnRayPrecastAction preFilter, const dgBody *const body,
        void *const userData) const {
	return RayCast(localP0, localP1, contactOut, preFilter, body, userData);
}

dgFloat32 dgCollisionUserMesh::RayCast(const dgVector &localP0,
                                       const dgVector &localP1, dgContactPoint &contactOut,
                                       OnRayPrecastAction preFilter, const dgBody *const body,
                                       void *const userData) const {
	dgFloat32 t;
	dgFloat32 param;
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	param = dgFloat32(1.2f);
	if (m_rayHitCallBack) {
		dgCollisionMeshRayHitDesc data;
		data.m_localP0 = localP0;
		data.m_localP1 = localP1;
		data.m_userData = m_userData;
		data.m_altenateUserData = userData;
		if (body) {
			data.m_matrix = body->m_collisionWorldMatrix;
		}

		t = m_rayHitCallBack(data);
		if ((t < dgFloat32(1.0f)) && (t > dgFloat32(0.0f))) {
			param = t;
			contactOut.m_normal = data.m_normal;
			contactOut.m_userId = data.m_userId;
		}
	}
	return param;
}

void dgCollisionUserMesh::GetCollidingFacesSimd(
    dgPolygonMeshDesc *const data) const {
	GetCollidingFaces(data);
}

void dgCollisionUserMesh::GetCollidingFaces(dgPolygonMeshDesc *const data) const {
	data->m_faceCount = 0;

	if (m_collideCallback) {
		data->m_me = this;
		data->m_userData = m_userData;
		m_collideCallback(*data);
	}
}

void dgCollisionUserMesh::DebugCollision(const dgMatrix &matrixPtr,
        OnDebugCollisionMeshCallback callback, void *const userData) const {
	/*
	 dgCollisionUserMeshShowPolyContext context;

	 context.m_matrix = matrixPtr;
	 context.m_userData = userData;;
	 context.m_callback = callback;

	 dgVector p0 (dgFloat32 (-1.0e20f), dgFloat32 (-1.0e20f), dgFloat32 (-1.0e20f), dgFloat32 (0.0f));
	 dgVector p1 (dgFloat32 ( 1.0e20f), dgFloat32 ( 1.0e20f), dgFloat32 ( 1.0e20f), dgFloat32 (0.0f));
	 ForAllSectors (p0, p1, ShowDebugPolygon, &context);
	 */
}
