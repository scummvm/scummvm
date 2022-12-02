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

#if !defined(AFX_DGCOLLISIONCONVEX_MODIFIER_H__4969D514_69A9_4D96_82B2_E5B0EA3D413F__INCLUDED_)
#define AFX_DGCOLLISIONCONVEX_MODIFIER_H__4969D514_69A9_4D96_82B2_E5B0EA3D413F__INCLUDED_





#include "dgCollisionConvex.h"


class dgCollisionConvexModifier: public dgCollisionConvex {
public:
	dgCollisionConvexModifier(dgCollisionConvex *convexChild, dgWorld *world);
	dgCollisionConvexModifier(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollisionConvexModifier();

	virtual dgMatrix ModifierGetMatrix() const;
	virtual void ModifierSetMatrix(const dgMatrix &matrix);

protected:
	virtual dgFloat32 GetVolume() const;
	virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const;

	virtual dgVector SupportVertex(const dgVector &dir) const;
	virtual dgVector SupportVertexSimd(const dgVector &dir) const;

	virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
//	virtual void DebugCollision (const dgBody& myBody, DebugCollisionMeshCallback callback) const;
	virtual void DebugCollision(const dgMatrix &matrix, OnDebugCollisionMeshCallback callback, void *const userData) const;
	virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
	virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;

	virtual dgVector CalculateVolumeIntegral(const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane, void *const context) const;

private:
	virtual dgInt32 CalculateSignature() const;
	virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);

	virtual dgFloat32 GetBoxMinRadius() const;
	virtual dgFloat32 GetBoxMaxRadius() const;

	virtual dgInt32 CalculatePlaneIntersection(const dgVector &normal, const dgVector &point, dgVector *const contactsOut)  const;
	virtual dgInt32 CalculatePlaneIntersectionSimd(const dgVector &normal, const dgVector &point, dgVector *const contactsOut) const;

	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void Serialize(dgSerialize callback, void *const userData) const;


	dgMatrix m_modifierMatrix;
	dgMatrix m_modifierInvMatrix;

	dgWorld *m_world;
	dgCollisionConvex *m_convexCollision;
	dgFloat32 m_det;


	friend class dgCollisionCompound;
};


#endif

