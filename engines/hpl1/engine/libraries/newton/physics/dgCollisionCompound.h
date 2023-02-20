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

#if !defined(AFX_DGCOLLISIONCOMPOUND_H__E164CA46_D0ED_46EE_8130_46C4004940BA__INCLUDED_)
#define AFX_DGCOLLISIONCOMPOUND_H__E164CA46_D0ED_46EE_8130_46C4004940BA__INCLUDED_

#include "dgCollision.h"
#include "dgContact.h"
class dgCollisionConvex;

#define DG_COMPOUND_STACK_DEPTH 256

class dgCollisionCompound : public dgCollision {
protected:
	enum dgNodeType {
		m_leaf,
		m_node,
	};

	class OOBBTestData {
	public:
		OOBBTestData(const dgMatrix &matrix);
		OOBBTestData(const dgMatrix &matrix, const dgVector &p0, const dgVector &p1);

		dgMatrix m_matrix;
		dgMatrix m_absMatrix;
		dgVector m_origin;
		dgVector m_size;
		dgVector m_localP0;
		dgVector m_localP1;
		dgVector m_aabbP0;
		dgVector m_aabbP1;
		dgVector m_crossAxis[3][3];
		dgVector m_crossAxisAbs[3][3];
		dgVector m_crossAxisDotAbs[3][3];
		dgVector m_extends[3][3];
	};

	DG_MSC_VECTOR_ALIGMENT
	class dgNodeBase {
	public:
		DG_CLASS_ALLOCATOR(allocator)
		dgNodeBase();
		dgNodeBase(dgCollisionConvex *const m_shape, dgInt32 id);
		dgNodeBase(dgNodeBase *const left, dgNodeBase *const right, dgInt32 id);
		~dgNodeBase();
		void reset();
		bool BoxTest(const OOBBTestData &data) const;
		bool BoxTest(const OOBBTestData &data, const dgNodeBase *const otherNode) const;

		dgFloat32 BoxClosestDistance(const dgVector *const points, dgInt32 count) const;

		dgVector m_p0;
		dgVector m_p1;
		dgVector m_size;
		dgVector m_origin;
		dgInt32 m_type;
		dgInt32 m_id;
		dgFloat32 m_area;
		dgNodeBase *m_left;
		dgNodeBase *m_right;
		dgNodeBase *m_parent;
		dgCollisionConvex *m_shape;
	} DG_GCC_VECTOR_ALIGMENT;

	class dgHeapNodePair {
	public:
		dgInt32 m_nodeA;
		dgInt32 m_nodeB;
	};

public:
	dgCollisionCompound(dgWorld *world);
	dgCollisionCompound(const dgCollisionCompound &source);
	dgCollisionCompound(dgInt32 count, dgCollisionConvex *const shapeArray[], dgWorld *world);
	dgCollisionCompound(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollisionCompound();

	void AddCollision(dgCollisionConvex *part);
	//  void RemoveCollision (dgCollisionConvex* part);
	void RemoveCollision(dgNodeBase *node);

protected:
	void Init(dgInt32 count, dgCollisionConvex *const shapeArray[]);
	virtual dgFloat32 GetVolume() const;
	virtual dgFloat32 GetBoxMinRadius() const;
	virtual dgFloat32 GetBoxMaxRadius() const;

	virtual dgVector SupportVertex(const dgVector &dir) const;
	virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const;

	virtual void DebugCollision(const dgMatrix &matrix, OnDebugCollisionMeshCallback callback, void *const userData) const;
	virtual void CalculateInertia(dgVector &inertia, dgVector &origin) const;
	virtual dgVector CalculateVolumeIntegral(const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane, void *const context) const;
	virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
	virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;

	dgNodeBase *BuildTree(dgInt32 count, dgCollisionConvex *const shapeArray[]);

	virtual dgInt32 CalculateSignature() const;
	virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);
	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void Serialize(dgSerialize callback, void *const userData) const;
	void LinkParentNodes();

	dgInt32 CalculateContacts(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy, dgInt32 useSimd) const;
	dgInt32 CalculateContactsToSingle(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy, dgInt32 useSimd) const;
	dgInt32 CalculateContactsToCompound(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy, dgInt32 useSimd) const;
	dgInt32 CalculateContactsToCollisionTree(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy, dgInt32 useSimd) const;
	dgInt32 CalculateContactsToHeightField(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy, dgInt32 useSimd) const;
	dgInt32 CalculateContactsBruteForce(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy, dgInt32 useSimd) const;

	dgInt32 ClosestDitance(dgBody *const bodyA, dgTriplex &contactA, dgBody *const bodyB, dgTriplex &contactB, dgTriplex &normalAB) const;

#ifdef _DEBUG
	dgVector IntenalSupportVertex(const dgVector &dir) const;
#endif

protected:
	//  dgInt32 GetAxis (dgNodeBase** const proxiArray, dgInt32 boxCount) const;
	//  static dgInt32 CompareBox (const dgNodeBase* const boxA, const dgNodeBase* const boxB, void* const context);
	//  dgNodeBase* BuildBottomUpTree(dgInt32 count, dgNodeBase** const proxiArray);

	void ImproveNodeFitness(dgNodeBase *const node) const;
	dgNodeBase *BuildTopDownTree(dgInt32 count, dgNodeBase **const proxiArray, dgInt32 &id);
	void PushNodes(dgNodeBase *const root, dgNodeBase **const proxiArray, dgInt32 &index) const;
	dgFloat32 CalculateSurfaceArea(dgNodeBase *const node0, dgNodeBase *const node1, dgVector &minBox, dgVector &maxBox) const;

	dgInt32 m_count;
	dgFloat32 m_boxMinRadius;
	dgFloat32 m_boxMaxRadius;
	dgWorld *m_world;
	dgNodeBase *m_root;
	dgCollisionConvex **m_array;
	OnCompoundCollisionPrefilter *m_preCollisionFilter;
	friend class dgBody;
	friend class dgWorld;
};

#endif
