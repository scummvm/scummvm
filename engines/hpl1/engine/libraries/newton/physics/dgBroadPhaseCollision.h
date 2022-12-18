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

#ifndef __AFX_BROADPHASE__INCLUDED__
#define __AFX_BROADPHASE__INCLUDED__

#include "dgBody.h"
#include "dgContact.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//#define DG_OCTREE_MAX_DEPTH       1
//#define DG_OCTREE_MAX_DEPTH       6
#define DG_OCTREE_MAX_DEPTH 7

typedef void(dgApi *OnBodiesInAABB)(dgBody *body, void *const userData);
typedef void(dgApi *OnLeavingWorldAction)(dgBody *body, dgInt32 threadIndex);

class dgCellPair {
public:
	dgBroadPhaseCell *m_cell_A;
	dgBroadPhaseCell *m_cell_B;
};

class dgSortArrayEntry {
public:
	dgBody *m_body;
	dgFloat32 m_key;
};

class dgSortArray : public dgList<dgSortArrayEntry> {
public:
	dgSortArray();
	~dgSortArray();

	void Add(dgBody *const body);
	void Remove(dgBody *const body);
	dgFloat32 Sort();
	void InvalidateCache();

	dgFloat32 RayCast(dgFloat32 minT, const dgLineBox &line, OnRayCastAction filter, OnRayPrecastAction prefilter, void *const userData) const;
	bool SanityCheck();

	dgInt8 m_index;
	dgInt8 m_isSorted;
};

class dgBroadPhaseCell {
public:
	dgBroadPhaseCell();
	~dgBroadPhaseCell();

private:
	void Init(dgInt32 layer, dgMemoryAllocator *allocator);
	void Add(dgBody *const body);
	void Remove(dgBody *const body);

	void Sort();
	void UpdateAutoPair(dgWorld *const world, dgInt32 threadIndex);

	dgSortArray m_sort[3];
	dgSortArray *m_lastSortArray;

	dgInt32 m_count;
	dgInt8 m_active;
	dgInt8 m_layerIndex;

	friend class dgBody;
	friend class dgBroadPhaseLayer;
	friend class dgBroadPhaseCollision;
	friend class dgBroadPhaseCellPairsWorkerThread;
};

class dgBroadPhaseLayer : public dgTree<dgBroadPhaseCell, dgUnsigned32> {
public:
	dgBroadPhaseLayer();
	~dgBroadPhaseLayer();

private:
	void Init(dgWorld *const world, dgFloat32 cellSize, dgInt32 layerIndex);

	inline dgUnsigned32 GetKey(dgInt32 x, dgInt32 z) const {
		dgUnsigned32 key = dgUnsigned32((z << DG_OCTREE_MAX_DEPTH) + x);
		return key;
	}

	inline void KeyToIndex(dgInt32 key, dgInt32 &x, dgInt32 &z) const {
		x = key & ((1 << DG_OCTREE_MAX_DEPTH) - 1);
		z = key >> DG_OCTREE_MAX_DEPTH;
	}

	inline dgBroadPhaseCell *Find(dgInt32 x, dgInt32 z) const {
		dgTreeNode *const node = dgTree<dgBroadPhaseCell, dgUnsigned32>::Find(GetKey(x, z));
		return node ? &node->GetInfo() : NULL;
	}
	dgBroadPhaseCell *FindCreate(dgInt32 x, dgInt32 z);

	dgWorld *m_me;
	dgFloat32 m_cellSize;
	dgFloat32 m_invCellSize;
	dgInt16 m_layerIndex;

	friend class dgBroadPhaseCollision;
};

class dgBroadPhaseApplyExternalForce : public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_step;
	dgInt32 m_count;
	dgInt32 m_skipForceUpdate;
	dgFloat32 m_timeStep;
	dgWorld *m_world;
	dgBody **m_bodies;
};

class dgBroadPhaseCellPairsWorkerThread : public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_step;
	dgInt32 m_count;
	dgWorld *m_world;
	dgCellPair *m_pairs;
};

class dgBroadPhaseCalculateContactsWorkerThread : public dgWorkerThread {
public:
	void Realloc(dgInt32 jointsCount, dgInt32 contactCount, dgInt32 threadIndex);
	virtual void ThreadExecute();

	dgInt32 m_step;
	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgFloat32 m_timestep;
	dgWorld *m_world;
};

class dgBroadPhaseMaterialCallbackWorkerThread : public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_step;
	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgFloat32 m_timestep;
	dgWorld *m_world;
	dgCollidingPairCollector::dgPair *m_pairs;
};

class dgBroadPhaseCollision {
public:
	void GetWorldSize(dgVector &p0, dgVector &p1) const;
	void SetWorldSize(const dgVector &min, const dgVector &max);
	void RayCast(const dgVector &p0, const dgVector &p1, OnRayCastAction filter, OnRayPrecastAction prefilter, void *const userData);
	dgInt32 ConvexCast(dgCollision *const shape, const dgMatrix &p0, const dgVector &p1, dgFloat32 &timetoImpact, OnRayPrecastAction prefilter, void *const userData, dgConvexCastReturnInfo *const info, dgInt32 maxContacts, dgInt32 threadIndex);
	void ForEachBodyInAABB(const dgVector &q0, const dgVector &q1, OnBodiesInAABB callback, void *const userData) const;

private:
	dgBroadPhaseCollision(dgMemoryAllocator *allocator);
	~dgBroadPhaseCollision();

	void Init();
	void Add(dgBody *const body);
	void Remove(dgBody *const body);
	void InvalidateCache();

	dgUnsigned32 UpdateContactsBroadPhaseBegin(dgFloat32 tiemstep, bool collisioUpdate, dgUnsigned32 ticks);
	void UpdateContactsBroadPhaseEnd(dgFloat32 tiemstep);

	void UpdateContacts(dgFloat32 tiemstep, bool collisioUpdate);
	void UpdateContactsSimd(dgFloat32 tiemstep, bool collisioUpdate);

	void UpdateBodyBroadphase(dgBody *const body, dgInt32 threadIndex);

	void UpdatePairs(dgBroadPhaseCell &cellA, dgBroadPhaseCell &cellB, dgInt32 threadIndex);
	void UpdatePairs(dgBody *const body0, dgSortArray::dgListNode *const listNode, dgInt32 axisX, dgInt32 threadIndex);

	dgVector m_min;
	dgVector m_max;
	dgVector m_appMinBox;
	dgVector m_appMaxBox;

	dgVector m_boxSize;
	dgBroadPhaseCell m_inactiveList;
	dgBroadPhaseLayer m_layerMap[DG_OCTREE_MAX_DEPTH];
	dgBroadPhaseApplyExternalForce m_applyExtForces[DG_MAXIMUN_THREADS];
	dgBroadPhaseCellPairsWorkerThread m_cellPairsWorkerThreads[DG_MAXIMUN_THREADS];
	dgBroadPhaseMaterialCallbackWorkerThread m_materialCallbackWorkerThreads[DG_MAXIMUN_THREADS];
	dgBroadPhaseCalculateContactsWorkerThread m_calculateContactsWorkerThreads[DG_MAXIMUN_THREADS];

	//  static void ForceAndtorque (void** const m_userParamArray, dgInt32 threadID);
	//  dgWorld* m_me;
	dgFloat32 m_worlSize;

	friend class dgBody;
	friend class dgWorld;
	friend class dgBroadPhaseCellPairsWorkerThread;
};

#endif
