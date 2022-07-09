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

#if !defined(AFX_DGCOLLISIONCONVEXHULL_H__AS235640FER__INCLUDED_)
#define AFX_DGCOLLISIONCONVEXHULL_H__AS235640FER__INCLUDED_

#include "dgCollisionConvex.h"


class dgCollisionConvexHull: public dgCollisionConvex  
{
	public:
	dgCollisionConvexHull(dgMemoryAllocator* const allocator, dgUnsigned32 signature, dgInt32 count, dgInt32 strideInBytes, dgFloat32 tolerance, const dgFloat32* vertexArray, const dgMatrix& matrix);
	dgCollisionConvexHull(dgWorld* const world, dgDeserialize deserialization, void* const userData);
	virtual ~dgCollisionConvexHull();

	dgInt32 GetFaceIndices (dgInt32 index, dgInt32* indices) const;
	private:
	
	bool Create (dgInt32 count, dgInt32 strideInBytes, const dgFloat32* const vertexArray, dgFloat32 tolerance);

	bool RemoveCoplanarEdge (dgPolyhedra& convex, const dgBigVector* const hullVertexArray) const;	
	dgBigVector FaceNormal (const dgEdge *face, const dgBigVector* const pool) const;
	bool CheckConvex (dgPolyhedra& polyhedra, const dgBigVector* hullVertexArray) const;

	virtual dgInt32 CalculateSignature () const;
	virtual void SetCollisionBBox (const dgVector& p0, const dgVector& p1);
	virtual void DebugCollision (const dgMatrix& matrix, OnDebugCollisionMeshCallback callback, void* const userData) const;
	virtual void GetCollisionInfo(dgCollisionInfo* info) const;
	virtual void Serialize(dgSerialize callback, void* const userData) const;
	virtual bool OOBBTest (const dgMatrix& matrix, const dgCollisionConvex* const shape, void* const cacheOrder) const; 

	virtual void SetBreakImpulse(dgFloat32 force);
	virtual dgFloat32 GetBreakImpulse() const;

	dgFloat32 m_destructionImpulse;
	dgInt32 m_faceCount;
	dgInt32 m_boundPlanesCount;
	dgConvexSimplexEdge** m_faceArray;

	friend class dgWorld;
	friend class dgCollisionConvex;
};

#endif 

