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

#if !defined(AFX_DGCOLLISIONSCENE_H_INCLUDED_)
#define AFX_DGCOLLISIONSCENE_H_INCLUDED_

#include "dgCollision.h"
#include "dgCollisionMesh.h"
#include "dgContact.h"

class dgCollisionScene : public dgCollision {
public:
	class dgNode {
	public:
		DG_CLASS_ALLOCATOR(allocator)
		dgNode();
		dgNode(dgNode *const sibling, dgNode *const myNode);
		~dgNode();

		dgVector m_minBox;
		dgVector m_maxBox;
		dgFloat32 m_surfaceArea;
		dgNode *m_parent;
		dgNode *m_left;
		dgNode *m_right;
		dgList<dgNode *>::dgListNode *m_fitnessNode;
	} DG_GCC_VECTOR_ALIGMENT;

	class dgProxy : public dgNode {
	public:
		DG_CLASS_ALLOCATOR(allocator)

		dgProxy(dgCollision *shape, const dgMatrix &matrix, dgCollisionScene *const owner);
		~dgProxy();

		dgMatrix m_matrix;
		void *m_userData;
		dgCollision *m_shape;
		dgCollisionScene *m_owner;
		dgList<dgProxy *>::dgListNode *m_myNode;
	};

	dgCollisionScene(dgWorld *const world);
	dgCollisionScene(dgWorld *const world, dgDeserialize deserialization, void *const userData);
	virtual ~dgCollisionScene();

	void *GetFirstProxy() const;
	void *GetNextProxy(void *const proxy) const;
	void *AddProxy(dgCollision *const shape, const dgMatrix &matrix);
	void SetProxyUserData(void *const proxy, void *const userData);
	void *GetProxyUserData(void *const proxy) const;
	void RemoveProxy(void *const proxy);
	void ImproveTotalFitness();

	dgMatrix GetProxyMatrix(void *const proxy);
	void SetProxyMatrix(void *const proxy, const dgMatrix &matrix);

	void CollidePair(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;
	void CollidePairSimd(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;

	void SetCollisionCallback(dgCollisionMeshCollisionCallback debugCallback);

private:
	virtual dgVector SupportVertex(const dgVector &dir) const;
	virtual void CalcAABB(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual void CalcAABBSimd(const dgMatrix &matrix, dgVector &p0, dgVector &p1) const;
	virtual bool OOBBTest(const dgMatrix &matrix, const dgCollisionConvex *const shape, void *const cacheOrder) const;

	virtual void DebugCollision(const dgMatrix &matrix, OnDebugCollisionMeshCallback callback, void *const userData) const;
	virtual dgFloat32 RayCast(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
	virtual dgFloat32 RayCastSimd(const dgVector &localP0, const dgVector &localP1, dgContactPoint &contactOut, OnRayPrecastAction preFilter, const dgBody *const body, void *const userData) const;
	virtual dgFloat32 GetVolume() const;

	virtual dgInt32 CalculateSignature() const;
	virtual dgFloat32 GetBoxMinRadius() const;
	virtual dgFloat32 GetBoxMaxRadius() const;
	virtual void SetCollisionBBox(const dgVector &p0, const dgVector &p1);
	virtual void CalculateInertia(dgVector &inertia, dgVector &origin) const;
	virtual dgVector CalculateVolumeIntegral(const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane, void *const context) const;

	virtual void GetCollisionInfo(dgCollisionInfo *info) const;
	virtual void Serialize(dgSerialize callback, void *const userData) const;

	void ImproveNodeFitness(dgNode *const node);
	dgFloat32 CalculateSurfaceArea(const dgNode *const node0, const dgNode *const node1, dgVector &minBox, dgVector &maxBox) const;

	dgVector m_boxSize;
	dgVector m_boxOrigin;

	dgInt32 m_lock;
	dgWorld *m_world;
	dgNode *m_rootNode;
	dgList<dgProxy *> m_list;
	dgList<dgNode *> m_fitnessList;
};

#endif
