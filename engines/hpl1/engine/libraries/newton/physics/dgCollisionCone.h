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

#if !defined(AFX_DGCOLLISIONCONE_H__AS235640FER__INCLUDED_)
#define AFX_DGCOLLISIONCONE_H__AS235640FER__INCLUDED_

#include "dgCollisionConvex.h"

#define DG_CONE_SEGMENTS 8


class dgCollisionCone: public dgCollisionConvex  
{
	public:
	dgCollisionCone (dgMemoryAllocator* const allocator, dgUnsigned32 signature, dgFloat32 radius, dgFloat32 height, const dgMatrix& matrix);
	dgCollisionCone(dgWorld* const world, dgDeserialize deserialization, void* const userData);
	virtual ~dgCollisionCone();

	private:
	void Init (dgFloat32 radius, dgFloat32 height);
	virtual dgVector SupportVertex (const dgVector& dir) const;
	virtual dgVector SupportVertexSimd (const dgVector& dir) const;

	virtual dgInt32 CalculatePlaneIntersection (const dgVector& normal, const dgVector& origin, dgVector* const contactsOut) const;
	virtual dgInt32 CalculatePlaneIntersectionSimd (const dgVector& normal, const dgVector& point, dgVector* const contactsOut) const;

	virtual dgFloat32 CalculateMassProperties (dgVector& inertia, dgVector& crossInertia, dgVector& centerOfMass) const;
//	virtual void DebugCollision (const dgBody& myBody, DebugCollisionMeshCallback callback) const;
	virtual void DebugCollision (const dgMatrix& matrix, OnDebugCollisionMeshCallback callback, void* const userData) const;
	

//	virtual dgVector GetLocalCG () const;
	virtual dgInt32 CalculateSignature () const;
	virtual void SetCollisionBBox (const dgVector& p0, const dgVector& p1);
	virtual void GetCollisionInfo(dgCollisionInfo* info) const;
	virtual void Serialize(dgSerialize callback, void* const userData) const;

	dgFloat32 m_height;
	dgFloat32 m_radius;
	dgFloat32 m_sinAngle;

//	dgFloat32 m_delCosTetha;
//	dgFloat32 m_delSinTetha;
//	dgFloat32 m_tethaStep;
//	dgFloat32 m_tethaStepInv;

	dgFloat32 m_amp;

	dgVector m_vertex[DG_CONE_SEGMENTS + 1];
	static dgInt32 m_shapeRefCount;
	static dgConvexSimplexEdge m_edgeArray[];

	friend class dgWorld;
};

#endif 

