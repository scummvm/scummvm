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

#if !defined(AFX_DGCONVEXCOLLISION_H__57E159CE_6B6F_42DE_891C_1F6C38EB9D29__INCLUDED_)
#define AFX_DGCONVEXCOLLISION_H__57E159CE_6B6F_42DE_891C_1F6C38EB9D29__INCLUDED_


#include "dgCollision.h"


class dgConvexSimplexEdge
{
	public:
	dgInt32 m_vertex;
	dgConvexSimplexEdge *m_twin;
	dgConvexSimplexEdge *m_next;
	dgConvexSimplexEdge *m_prev;
};


DG_MSC_VECTOR_ALIGMENT
class dgCollisionConvex: public dgCollision
{
	bool RayHitBox (const dgVector& localP0, const dgVector& localP1) const; 		
	dgInt32 RayCastClosestFace (dgVector* tetrahedrum, const dgVector& origin, dgFloat32& pointDist) const;


	public:
	virtual void CalcAABB (const dgMatrix &matrix, dgVector& p0, dgVector& p1) const;
	virtual void CalcAABBSimd (const dgMatrix &matrix, dgVector& p0, dgVector& p1) const;
	virtual bool OOBBTest (const dgMatrix& matrix, const dgCollisionConvex* const shape, void* const cacheOrder) const; 
	
	virtual dgFloat32 RayCast (const dgVector& localP0, const dgVector& localP1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const;
	virtual dgFloat32 RayCastSimd (const dgVector& localP0, const dgVector& localP1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const;
	virtual dgVector SupportVertex (const dgVector& dir) const;
	virtual dgVector SupportVertexSimd (const dgVector& dir) const;

	virtual dgInt32 CalculatePlaneIntersection (const dgVector& normal, const dgVector& point, dgVector* const contactsOut) const;
	virtual dgInt32 CalculatePlaneIntersectionSimd (const dgVector& normal, const dgVector& point, dgVector* const contactsOut) const;
	dgInt32 GetVertexCount() const { return m_vertexCount;}
	virtual bool IsTriggerVolume() const;

	private:
	dgInt32 SimplifyClipPolygon (dgInt32 count, const dgVector& normal, dgVector* const polygon) const;

	virtual dgVector CalculateVolumeIntegral (const dgMatrix& globalMatrix, GetBuoyancyPlane bouyancyPlane, void* const context) const;
	static void CalculateInertia (void *userData, int vertexCount, const dgFloat32* FaceArray, int faceId);
	virtual void CalculateInertia (dgVector& inertia, dgVector& origin) const;

	virtual dgFloat32 GetVolume () const;

	virtual dgFloat32 GetBoxMinRadius () const; 
	virtual dgFloat32 GetBoxMaxRadius () const;


	protected:
	virtual void* GetUserData () const;
	virtual void SetUserData (void* const userData);

	dgCollisionConvex (dgMemoryAllocator* const allocator, dgUnsigned32 signature, const dgMatrix& matrix, dgCollisionID id);
	dgCollisionConvex (dgWorld* const world, dgDeserialize deserialization, void* const userData);
	~dgCollisionConvex ();

	
	virtual void SetAsTriggerVolume(bool mode);
	virtual void SerializeLow(dgSerialize callback, void* const userData) const;

	dgVector CalculateVolumeIntegral (const dgPlane& plane) const; 
	dgFloat32 GetDiscretedAngleStep (dgFloat32 radius) const;
	dgConvexSimplexEdge *GetSupportEdge (const dgVector& dir) const;
	
	void SetVolumeAndCG ();
	bool SanityCheck (dgPolyhedra& hull) const;
	virtual void DebugCollision (const dgMatrix& matrix, OnDebugCollisionMeshCallback callback, void* const userData) const;
	virtual dgFloat32 CalculateMassProperties (dgVector& inertia, dgVector& crossInertia, dgVector& centerOfMass) const;

	bool SanityCheck(dgInt32 count, const dgVector& normal, dgVector* const contactsOut) const;
	dgInt32 RectifyConvexSlice (dgInt32 count, const dgVector& normal, dgVector* const contactsOut) const;


	dgVector m_volume;
	dgVector m_boxSize;
	dgVector m_boxOrigin;
	dgVector m_size_x;
	dgVector m_size_y;
	dgVector m_size_z;
	dgConvexSimplexEdge *m_supportVertexStarCuadrant[8];
	
	void* m_userData;
	dgVector* m_vertex;
	dgConvexSimplexEdge *m_simplex;
	
	dgFloat32 m_boxMinRadius;
	dgFloat32 m_boxMaxRadius;
	dgFloat32 m_simplexVolume;
	dgUnsigned16 m_edgeCount;
	dgUnsigned16 m_vertexCount;
	dgUnsigned32 m_isTriggerVolume : 1;
	
	static dgVector m_zero;
	static dgVector m_negOne;
	static dgVector m_indexStep;
	static dgVector m_aabb_padd;
	static dgVector m_index_0123;
	static dgVector m_index_4567;
	static dgVector m_multiResDir[8];
	static dgVector m_multiResDir_sse[6];
	
	static dgVector m_nrh0p5;
	static dgVector m_nrh3p0;
	static dgVector m_huge;
	static dgVector m_negativeTiny;

	static dgVector m_signMask;
	static dgVector m_triplexMask;
	static dgTriplex m_hullDirs[14]; 

	static dgInt32 m_iniliazised;
	static dgInt32 m_rayCastSimplex[4][4];
	
	friend class dgWorld;
	friend class dgMinkowskiConv;
	friend class dgCollisionCompound;
	friend class dgBroadPhaseCollision;
	friend class dgCollisionConvexModifier;
}DG_GCC_VECTOR_ALIGMENT;



#endif //AFX_DGCONVEXCOLLISION_H__57E159CE_6B6F_42DE_891C_1F6C38EB9D29__INCLUDED_


