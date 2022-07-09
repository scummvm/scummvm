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

#ifndef __DGCOLLIIO_USERMESH__
#define __DGCOLLIIO_USERMESH__

#include "dgCollision.h"
#include "dgCollisionMesh.h"


typedef void (dgApi *OnUserMeshDestroyCallback) (void* const userData);
typedef void (dgApi *OnUserMeshCollideCallback) (dgPolygonMeshDesc& collideData);
typedef dgFloat32 (dgApi *OnUserMeshRayHitCallback) (dgCollisionMeshRayHitDesc& rayHitdata);
typedef void (dgApi *UserMeshCollisionInfo) (void* userData, dgCollisionInfo* infoRecord);
typedef void (dgApi *UserMeshFacesInAABB) (void* userData, const dgFloat32* p0, const dgFloat32* p1,
										   const dgFloat32** vertexArray, dgInt32* vertexCount, dgInt32* vertexStrideInBytes, 
										   const dgInt32* indexList, dgInt32 maxIndexCount, const dgInt32* faceAttribute);

class dgUserMeshCreation
{
	public:
	void* m_userData;
	OnUserMeshCollideCallback m_collideCallback;
	OnUserMeshRayHitCallback m_rayHitCallBack;
	OnUserMeshDestroyCallback m_destroyCallback;
	UserMeshCollisionInfo m_getInfo;
	UserMeshFacesInAABB m_faceInAabb;
};


class dgCollisionUserMesh: public dgCollisionMesh
{
	public:
	dgCollisionUserMesh(dgMemoryAllocator* allocator, const dgVector& boxP0, const dgVector& boxP1, const dgUserMeshCreation& data);
	dgCollisionUserMesh (dgWorld* const world, dgDeserialize deserialization, void* const userData);
	virtual ~dgCollisionUserMesh(void);

	void GetVertexListIndexList (const dgVector& p0, const dgVector& p1, dgGetVertexListIndexList &data) const;

	private:
	void Serialize(dgSerialize callback, void* const userData) const;
	virtual void GetCollisionInfo(dgCollisionInfo* info) const;
	virtual dgFloat32 RayCast (const dgVector& localP0, const dgVector& localP1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const;
	virtual dgFloat32 RayCastSimd (const dgVector& localP0, const dgVector& localP1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const;
	virtual void GetCollidingFaces (dgPolygonMeshDesc* const data) const;
	virtual void GetCollidingFacesSimd (dgPolygonMeshDesc* const data) const;
	void DebugCollision (const dgMatrix& matrixPtr, OnDebugCollisionMeshCallback callback, void* const userData) const;

	void* m_userData;
	UserMeshCollisionInfo m_getInfo;
	UserMeshFacesInAABB m_faceInAabb;
	OnUserMeshRayHitCallback m_rayHitCallBack;
	OnUserMeshCollideCallback m_collideCallback;
	OnUserMeshDestroyCallback m_destroyCallback;
};


#endif
