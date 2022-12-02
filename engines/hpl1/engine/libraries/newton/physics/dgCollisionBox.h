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

#if !defined(AFX_DGCOLLISIONBOX_H__364692C2_5F23_41AE_A167_7A92E2D2DA5F__INCLUDED_)
#define AFX_DGCOLLISIONBOX_H__364692C2_5F23_41AE_A167_7A92E2D2DA5F__INCLUDED_


#include "dgCollisionConvex.h"

class dgCollisionBox;


class dgCollisionBox: public dgCollisionConvex {
public:

	dgCollisionBox(dgMemoryAllocator *const allocator, dgUnsigned32 signature, dgFloat32 size_x, dgFloat32 size_y, dgFloat32 size_z, const dgMatrix &matrix);
	dgCollisionBox(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollisionBox();

protected:
	void Init(dgFloat32 size_x, dgFloat32 size_y, dgFloat32 size_z);
	virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
	virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;

	virtual dgVector SupportVertex(const dgVector &dir) const;
	virtual dgVector SupportVertexSimd(const dgVector &dir) const;

	virtual dgInt32 CalculatePlaneIntersection(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;
	virtual dgInt32 CalculatePlaneIntersectionSimd(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;

	virtual dgInt32 CalculateSignature() const;
	virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);
	virtual dgFloat32 CalculateMassProperties(dgVector &inertia, dgVector &crossInertia, dgVector &centerOfMass) const;

	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void Serialize(dgSerialize callback, void *const userData) const;

	virtual void SetBreakImpulse(dgFloat32 force);
	virtual dgFloat32 GetBreakImpulse() const;


	dgVector m_size[2];
	dgVector m_vertex[8];
	dgVector m_vertex_sse[6];
	dgFloat32 m_destructionImpulse;

	static dgConvexSimplexEdge m_edgeArray[];
	friend class dgWorld;
};

#endif // !defined(AFX_DGCOLLISIONBOX_H__364692C2_5F23_41AE_A167_7A92E2D2DA5F__INCLUDED_)

