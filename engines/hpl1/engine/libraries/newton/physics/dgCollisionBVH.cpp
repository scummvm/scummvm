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

#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"

//#include "dgContact.h"
#include "dgCollisionBVH.h"

dgCollisionBVH::dgCollisionBVH(dgMemoryAllocator *const allocator) : dgCollisionMesh(allocator, m_boundingBoxHierachy), dgAABBPolygonSoup() {
	m_rtti |= dgCollisionBVH_RTTI;
	m_builder = NULL;
	m_userRayCastCallback = NULL;
}

dgCollisionBVH::dgCollisionBVH(dgWorld *const world,
                               dgDeserialize deserialization, void *const userData) : dgCollisionMesh(world, deserialization, userData), dgAABBPolygonSoup() {
	m_rtti |= dgCollisionBVH_RTTI;
	m_builder = NULL;
	;
	m_userRayCastCallback = NULL;

	dgAABBPolygonSoup::Deserialize(deserialization, userData);

	dgVector p0;
	dgVector p1;
	GetAABB(p0, p1);
	SetCollisionBBox(p0, p1);
}

dgCollisionBVH::~dgCollisionBVH(void) {
}

void dgCollisionBVH::Serialize(dgSerialize callback, void *const userData) const {
	SerializeLow(callback, userData);
	dgAABBPolygonSoup::Serialize((dgSerialize)callback, userData);
}

void dgCollisionBVH::BeginBuild() {
	m_builder = new (m_allocator) dgPolygonSoupDatabaseBuilder(m_allocator);
	m_builder->Begin();
}

void dgCollisionBVH::AddFace(dgInt32 vertexCount,
                             const dgFloat32 *const vertexPtr, dgInt32 strideInBytes,
                             dgInt32 faceAttribute) {
	dgInt32 faceArray;
	dgInt32 indexList[256];

	faceArray = vertexCount;
	NEWTON_ASSERT(vertexCount < dgInt32(sizeof(indexList) / sizeof(indexList[0])));
	for (dgInt32 i = 0; i < vertexCount; i++) {
		indexList[i] = i;
	}
	m_builder->AddMesh(vertexPtr, vertexCount, strideInBytes, 1, &faceArray,
	                   indexList, &faceAttribute, dgGetIdentityMatrix());
}

void dgCollisionBVH::SetCollisionRayCastCallback(
    dgCollisionBVHUserRayCastCallback rayCastCallback) {
	m_userRayCastCallback = rayCastCallback;
}

void dgCollisionBVH::EndBuild(dgInt32 optimize) {
	dgVector p0;
	dgVector p1;

	bool state = optimize ? true : false;

	m_builder->End(state);
	Create(*m_builder, state);

	GetAABB(p0, p1);
	SetCollisionBBox(p0, p1);

	delete m_builder;
	m_builder = NULL;
}

void dgCollisionBVH::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollision::GetCollisionInfo(info);

	info->m_offsetMatrix = GetOffsetMatrix();
	info->m_collisionType = m_collsionId;

	dgGetVertexListIndexList data;
	data.m_indexList = NULL;
	data.m_userDataList = NULL;
	data.m_maxIndexCount = 1000000000;
	data.m_triangleCount = 0;
	dgVector p0(-1.0e10f, -1.0e10f, -1.0e10f, 1.0f);
	dgVector p1(1.0e10f, 1.0e10f, 1.0e10f, 1.0f);
	ForAllSectors(p0, p1, GetTriangleCount, &data);

	info->m_bvhCollision.m_vertexCount = GetVertexCount();
	info->m_bvhCollision.m_indexCount = data.m_triangleCount * 3;
}

dgIntersectStatus dgCollisionBVH::CollectVertexListIndexList(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgGetVertexListIndexList &data = (*(dgGetVertexListIndexList *)context);

	if ((data.m_triangleCount + indexCount - 2) * 3 > data.m_maxIndexCount) {
		return t_StopSearh;
	}

	dgInt32 k = data.m_triangleCount;
	dgInt32 j = data.m_triangleCount * 3;
	dgInt32 index0 = indexArray[0];
	dgInt32 index1 = indexArray[1];
	dgInt32 atribute = indexArray[-1];
	for (dgInt32 i = 2; i < indexCount; i++) {
		dgInt32 index2 = indexArray[i];
		data.m_indexList[j + 0] = index0;
		data.m_indexList[j + 1] = index1;
		data.m_indexList[j + 2] = index2;
		data.m_userDataList[k] = atribute;
		index1 = index2;
		k++;
		j += 3;
	}

	NEWTON_ASSERT(j <= data.m_maxIndexCount);
	data.m_triangleCount = k;
	NEWTON_ASSERT((data.m_triangleCount * 3) <= data.m_maxIndexCount);

	return t_ContinueSearh;
}

dgIntersectStatus dgCollisionBVH::GetTriangleCount(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgGetVertexListIndexList &data = (*(dgGetVertexListIndexList *)context);

	//  if ((data.m_triangleCount + indexCount - 2) * 3 >= data.m_maxIndexCount) {
	if ((data.m_triangleCount + indexCount - 2) * 3 > data.m_maxIndexCount) {
		return t_StopSearh;
	}

	data.m_triangleCount += (indexCount - 2);
	NEWTON_ASSERT((data.m_triangleCount * 3) <= data.m_maxIndexCount);
	return t_ContinueSearh;
}

void dgCollisionBVH::GetVertexListIndexList(const dgVector &p0,
        const dgVector &p1, dgGetVertexListIndexList &data) const {
	ForAllSectors(p0, p1, CollectVertexListIndexList, &data);

	data.m_veterxArray = GetLocalVertexPool();
	data.m_vertexCount = GetVertexCount();
	data.m_vertexStrideInBytes = GetStrideInBytes();
}

dgFloat32 dgCollisionBVH::RayHitSimd(void *context,
                                     const dgFloat32 *const polygon, dgInt32 strideInBytes,
                                     const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgBVHRay &me = *((dgBVHRay *)context);
	dgVector normal(
	    &polygon[indexArray[indexCount] * (strideInBytes / sizeof(dgFloat32))]);
	dgFloat32 t = me.PolygonIntersectSimd(normal, polygon, strideInBytes,
	                                      indexArray, indexCount);
	if (t < dgFloat32(1.0f)) {
		if (t <= (me.m_t * dgFloat32(1.0001f))) {
			if ((t * dgFloat32(1.0001f)) >= me.m_t) {
				dgFloat32 dist0;
				dgFloat32 dist1;
				dist0 = me.m_diff % normal;
				dist1 = me.m_diff % me.m_normal;
				if (dist0 < dist1) {
					me.m_t = t;
					me.m_normal = normal;
					me.m_id = me.m_me->GetTagId(indexArray);
				} else {
					t = me.m_t;
				}
			} else {
				me.m_t = t;
				me.m_normal = normal;
				me.m_id = me.m_me->GetTagId(indexArray);
			}
		}
	}
	return t;
}

dgFloat32 dgCollisionBVH::RayHit(void *context, const dgFloat32 *const polygon,
                                 dgInt32 strideInBytes, const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgBVHRay &me = *((dgBVHRay *)context);
	dgVector normal(
	    &polygon[indexArray[indexCount] * (strideInBytes / sizeof(dgFloat32))]);
	dgFloat32 t = me.PolygonIntersect(normal, polygon, strideInBytes, indexArray,
	                                  indexCount);
	if (t < dgFloat32(1.0f)) {
		//      if (t <= me.m_t) {
		if (t <= (me.m_t * dgFloat32(1.0001f))) {
			if ((t * dgFloat32(1.0001f)) >= me.m_t) {
				dgFloat32 dist0;
				dgFloat32 dist1;
				dist0 = me.m_diff % normal;
				dist1 = me.m_diff % me.m_normal;
				if (dist0 < dist1) {
					me.m_t = t;
					me.m_normal = normal;
					me.m_id = me.m_me->GetTagId(indexArray);
				} else {
					t = me.m_t;
				}
			} else {
				me.m_t = t;
				me.m_normal = normal;
				me.m_id = me.m_me->GetTagId(indexArray);
			}
		}
	}
	return t;
}

dgFloat32 dgCollisionBVH::RayHitUserSimd(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgFloat32 t = dgFloat32(1.2f);
	dgBVHRay &me = *((dgBVHRay *)context);
	dgVector normal(
	    &polygon[indexArray[indexCount] * (strideInBytes / sizeof(dgFloat32))]);
	t = me.PolygonIntersectSimd(normal, polygon, strideInBytes, indexArray,
	                            indexCount);
	if (t < dgFloat32(1.0f)) {
		if (t < me.m_t) {
			me.m_t = t;
			me.m_normal = normal;
			me.m_id = me.m_me->GetTagId(indexArray);
		}
		normal = me.m_matrix.RotateVectorSimd(normal);
		t = me.m_me->GetDebugRayCastCallback()(me.m_myBody, me.m_me, t, &normal[0],
		                                       dgInt32(me.m_me->GetTagId(indexArray)), me.m_userData);
	}
	return t;
}

dgFloat32 dgCollisionBVH::RayHitUser(void *context,
                                     const dgFloat32 *const polygon, dgInt32 strideInBytes,
                                     const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgFloat32 t = dgFloat32(1.2f);
	dgBVHRay &me = *((dgBVHRay *)context);
	dgVector normal(
	    &polygon[indexArray[indexCount] * (strideInBytes / sizeof(dgFloat32))]);
	t = me.PolygonIntersect(normal, polygon, strideInBytes, indexArray,
	                        indexCount);
	if (t < dgFloat32(1.0f)) {
		if (t < me.m_t) {
			me.m_t = t;
			me.m_normal = normal;
			me.m_id = me.m_me->GetTagId(indexArray);
		}
		normal = me.m_matrix.RotateVector(normal);
		t = me.m_me->GetDebugRayCastCallback()(me.m_myBody, me.m_me, t, &normal[0],
		                                       dgInt32(me.m_me->GetTagId(indexArray)), me.m_userData);
	}
	return t;
}

dgFloat32 dgCollisionBVH::RayCastSimd(const dgVector &localP0,
                                      const dgVector &localP1, dgContactPoint &contactOut,
                                      OnRayPrecastAction preFilter, const dgBody *const body,
                                      void *const userData) const {
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgFloat32 param = dgFloat32(1.2f);
	dgBVHRay ray(localP0, localP1);
	ray.m_t = 2.0f;
	ray.m_me = this;
	ray.m_userData = userData;
	if (!m_userRayCastCallback) {
		ForAllSectorsRayHitSimd(ray, RayHitSimd, &ray);
		if (ray.m_t <= 1.0f) {
			param = ray.m_t;
			contactOut.m_normal = ray.m_normal.Scale(
			                          dgRsqrt((ray.m_normal % ray.m_normal) + 1.0e-8f));
			contactOut.m_userId = ray.m_id;
		}
	} else {
		if (body) {
			ray.m_matrix = body->m_collisionWorldMatrix;
		}

		ForAllSectorsRayHitSimd(ray, RayHitUserSimd, &ray);
		if (ray.m_t <= 1.0f) {
			param = ray.m_t;
			contactOut.m_normal = ray.m_normal.Scale(
			                          dgRsqrt((ray.m_normal % ray.m_normal) + 1.0e-8f));
			contactOut.m_userId = ray.m_id;
		}
	}

	return param;
}

dgFloat32 dgCollisionBVH::RayCast(const dgVector &localP0,
                                  const dgVector &localP1, dgContactPoint &contactOut,
                                  OnRayPrecastAction preFilter, const dgBody *const body,
                                  void *const userData) const {
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgFloat32 param = dgFloat32(1.2f);

	dgBVHRay ray(localP0, localP1);
	ray.m_t = 2.0f;
	ray.m_me = this;
	ray.m_userData = userData;
	if (!m_userRayCastCallback) {
		ForAllSectorsRayHit(ray, RayHit, &ray);
		if (ray.m_t <= 1.0f) {
			param = ray.m_t;
			contactOut.m_normal = ray.m_normal.Scale(
			                          dgRsqrt((ray.m_normal % ray.m_normal) + 1.0e-8f));
			contactOut.m_userId = ray.m_id;
		}
	} else {
		if (body) {
			ray.m_matrix = body->m_collisionWorldMatrix;
		}

		ForAllSectorsRayHit(ray, RayHitUser, &ray);
		if (ray.m_t <= 1.0f) {
			param = ray.m_t;
			contactOut.m_normal = ray.m_normal.Scale(
			                          dgRsqrt((ray.m_normal % ray.m_normal) + 1.0e-8f));
			contactOut.m_userId = ray.m_id;
		}
	}
	return param;
}

dgIntersectStatus dgCollisionBVH::GetPolygon(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgPolygonMeshDesc &data = (*(dgPolygonMeshDesc *)context);
	if (data.m_faceCount >= DG_MAX_COLLIDING_FACES) {
		NEWTON_ASSERT(0);
		return t_StopSearh;
	}

	if ((data.m_globalIndexCount + indexCount) >= DG_MAX_COLLIDING_VERTEX) {
		NEWTON_ASSERT(0);
		return t_StopSearh;
	}

	if (data.m_me->GetDebugCollisionCallback()) {
		dgTriplex triplex[128];
		dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));
		const dgMatrix &matrix = data.m_polySoupBody->GetCollisionMatrix();
		for (dgInt32 i = 0; i < indexCount; i++) {
			dgVector p(&polygon[indexArray[i] * stride]);
			p = matrix.TransformVector(p);
			triplex[i].m_x = p.m_x;
			triplex[i].m_y = p.m_y;
			triplex[i].m_z = p.m_z;
		}
		data.m_me->GetDebugCollisionCallback()(data.m_polySoupBody, data.m_objBody,
		                                       indexArray[-1], indexCount, &triplex[0].m_x, sizeof(dgTriplex));
	}

	NEWTON_ASSERT(data.m_vertex == polygon);

	data.m_userAttribute[data.m_faceCount] = indexArray[-1];
	data.m_faceIndexCount[data.m_faceCount] = indexCount;
	data.m_faceNormalIndex[data.m_faceCount] = indexArray[indexCount];
	data.m_faceMaxSize[data.m_faceCount] = dgFloat32(
	        indexArray[(indexCount << 1) + 1]);
	data.m_faceCount++;

	dgInt32 j = data.m_globalIndexCount;
	for (dgInt32 i = 0; i < indexCount; i++) {
		data.m_faceVertexIndex[j] = indexArray[i];
		data.m_faceAdjencentEdgeNormal[j] = indexArray[i + indexCount + 1];
		j++;
	}
	data.m_globalIndexCount = j;

	return t_ContinueSearh;
}

void dgCollisionBVH::GetCollidingFacesSimd(dgPolygonMeshDesc *const data) const {
	data->m_faceCount = 0;

	data->m_me = this;
	data->m_vertex = GetLocalVertexPool();
	data->m_vertexStrideInBytes = GetStrideInBytes();

	data->m_globalIndexCount = 0;
	data->m_faceMaxSize = data->m_globalFaceMaxSize;
	data->m_userAttribute = data->m_globalUserAttribute;
	data->m_faceIndexCount = data->m_globalFaceIndexCount;
	data->m_faceVertexIndex = data->m_globalFaceVertexIndex;
	data->m_faceNormalIndex = data->m_globalFaceNormalIndex;
	data->m_faceAdjencentEdgeNormal = data->m_globalAdjencentEdgeNormal;
	ForAllSectorsSimd(data->m_boxP0, data->m_boxP1, GetPolygon, data);
}

void dgCollisionBVH::GetCollidingFaces(dgPolygonMeshDesc *const data) const {
	data->m_faceCount = 0;

	data->m_me = this;
	data->m_vertex = GetLocalVertexPool();
	data->m_vertexStrideInBytes = GetStrideInBytes();

	data->m_globalIndexCount = 0;
	data->m_faceMaxSize = data->m_globalFaceMaxSize;
	data->m_userAttribute = data->m_globalUserAttribute;
	data->m_faceIndexCount = data->m_globalFaceIndexCount;
	data->m_faceVertexIndex = data->m_globalFaceVertexIndex;
	data->m_faceNormalIndex = data->m_globalFaceNormalIndex;
	data->m_faceAdjencentEdgeNormal = data->m_globalAdjencentEdgeNormal;
	ForAllSectors(data->m_boxP0, data->m_boxP1, GetPolygon, data);
}

dgVector dgCollisionBVH::SupportVertex(const dgVector &dir) const {
	return ForAllSectorsSupportVectex(dir);
}

struct dgCollisionBVHShowPolyContext {
	dgMatrix m_matrix;
	void *m_userData;
	OnDebugCollisionMeshCallback m_callback;
};

dgIntersectStatus dgCollisionBVH::ShowDebugPolygon(void *context,
        const dgFloat32 *const polygon, dgInt32 strideInBytes,
        const dgInt32 *const indexArray, dgInt32 indexCount) {
	dgTriplex triplex[128];
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));

	dgCollisionBVHShowPolyContext &data =
	    *(dgCollisionBVHShowPolyContext *)context;
	for (dgInt32 i = 0; i < indexCount; i++) {
		dgVector p(&polygon[indexArray[i] * stride]);
		p = data.m_matrix.TransformVector(p);
		triplex[i].m_x = p.m_x;
		triplex[i].m_y = p.m_y;
		triplex[i].m_z = p.m_z;
	}
	data.m_callback(data.m_userData, indexCount, &triplex[0].m_x, indexArray[-1]);

	return t_ContinueSearh;
}

void dgCollisionBVH::DebugCollision(const dgMatrix &matrixPtr,
                                    OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgCollisionBVHShowPolyContext context;

	context.m_matrix = matrixPtr;
	context.m_userData = userData;
	;
	context.m_callback = callback;

	dgVector p0(dgFloat32(-1.0e20f), dgFloat32(-1.0e20f), dgFloat32(-1.0e20f),
	            dgFloat32(0.0f));
	dgVector p1(dgFloat32(1.0e20f), dgFloat32(1.0e20f), dgFloat32(1.0e20f),
	            dgFloat32(0.0f));
	ForAllSectors(p0, p1, ShowDebugPolygon, &context);
}
