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

#if !defined(AFX_DGPHYSICSWORLD_H__EC18C699_D48D_448F_A510_A865B2CC0789__INCLUDED_)
#define AFX_DGPHYSICSWORLD_H__EC18C699_D48D_448F_A510_A865B2CC0789__INCLUDED_


#include "dgBody.h"
#include "dgContact.h"
#include "dgCollision.h"
#include "dgCollisionScene.h"
#include "dgBodyMasterList.h"
#include "dgWorldDynamicUpdate.h"

//#include "dgBallConstraint.h"
//#include "dgHingeConstraint.h"
//#include "dgSlidingConstraint.h"
//#include "dgUniversalConstraint.h"
//#include "dgCorkscrewConstraint.h"
#include "dgBroadPhaseCollision.h"
//#include "dgUpVectorConstraint.h"
//#include "dgPointToCurveConstraint.h"


#define DG_REDUCE_CONTACT_TOLERANCE         dgFloat32 (1.0e-2f)
#define DG_PRUNE_CONTACT_TOLERANCE          dgFloat32 (1.0e-2f)
#define DG_RESTING_CONTACT_PENETRATION      dgFloat32 (1.0f / 256.0f)
#define DG_IMPULSIVE_CONTACT_PENETRATION    dgFloat32 (1.0f / 256.0f + DG_RESTING_CONTACT_PENETRATION)

#define DG_SLEEP_ENTRIES                    8
#define DG_MAX_DESTROYED_BODIES_BY_FORCE    8

//#define DG_SPANNING_MEMORY_POOL_SIZE  ((1<<16))

class dgCollisionPoint;
class dgUserConstraint;
class dgBallConstraint;
class dgHingeConstraint;
class dgSlidingConstraint;
class dgUniversalConstraint;
class dgCorkscrewConstraint;
class dgUpVectorConstraint;
class dgUserMeshCreation;
//class dgConnectorConstraint;
//class dgPointToCurveConstraint;


class dgBodyCollisionList: public dgTree<dgCollision *, dgUnsigned32> {
public:
	dgBodyCollisionList(dgMemoryAllocator *const allocator)
		: dgTree<dgCollision *, dgUnsigned32>(allocator) {

	}
};

class dgBodyMaterialList: public dgTree<dgContactMaterial, dgUnsigned32> {
public:
	dgBodyMaterialList(dgMemoryAllocator *const allocator)
		: dgTree<dgContactMaterial, dgUnsigned32>(allocator) {

	}
};

class dgCollisionParamProxy;

enum dgPerformanceCounters {



	m_worldTicks = 0,

	m_collisionTicks,
	m_broadPhaceTicks,
	m_narrowPhaseTicks,

	m_dynamicsTicks,
	m_dynamicsBuildSpanningTreeTicks,
	m_dynamicsSolveSpanningTreeTicks,

	m_forceCallback,
	m_counterSize,
};

class dgWorld;

typedef dgUnsigned32(dgApi *OnIslandUpdate)(const dgWorld *const world, void *island, dgInt32 bodyCount);
typedef void (dgApi *OnDestroyCollision)(const dgWorld *const world, dgCollision *shape);
typedef void (dgApi *OnBodyDestructionByExeciveForce)(const dgBody *const body, const dgContact *joint);

class dgSolverSleepTherfesholds {
public:
	dgFloat32 m_maxAccel;
	dgFloat32 m_maxAlpha;
	dgFloat32 m_maxVeloc;
	dgFloat32 m_maxOmega;
	dgInt32 m_steps;
};

DG_MSC_VECTOR_ALIGMENT
class dgWorld:
	public dgBodyMasterList,
	public dgBroadPhaseCollision,
	public dgBodyMaterialList,
	public dgBodyCollisionList,
	public dgActiveContacts,
	public dgCollidingPairCollector {
public:

	class dgDetroyBodyByForce {
	public:
		dgDetroyBodyByForce()
			: m_count(0) {
		}
		dgInt32 m_count;
		dgFloat32 m_force[DG_MAX_DESTROYED_BODIES_BY_FORCE];
		const dgBody *m_bodies[DG_MAX_DESTROYED_BODIES_BY_FORCE];
		const dgContact *m_joint[DG_MAX_DESTROYED_BODIES_BY_FORCE];
	};


	DG_CLASS_ALLOCATOR(allocator)

	dgWorld(dgMemoryAllocator *const allocator);
	~dgWorld();

//	void SetGlobalScale (dgFloat32 scale);
//	dgFloat32 GetGlobalScale () const;

	void SetSolverMode(dgInt32 mode);
	void SetFrictionMode(dgInt32 mode);
	void SetHardwareMode(dgInt32 mode);
	dgInt32 GetHardwareMode(char *description) const;

	void SetThreadsCount(dgInt32 count);
	dgInt32 GetThreadsCount() const;
	dgInt32 GetMaxThreadsCount() const;
//	dgInt32 GetThreadNumber() const;
	void EnableThreadOnSingleIsland(dgInt32 mode);
	dgInt32 GetThreadOnSingleIsland() const;

	void FlushCache();

	void *GetUserData() const;
	void SetUserData(void *const userData);

	void Update(dgFloat32 timestep);
	void UpdateCollision();
	dgInt32 Collide(dgCollision *const collisionA, const dgMatrix &matrixA, dgCollision *const collisionB, const dgMatrix &matrixB,
	                dgTriplex *const points, dgTriplex *const normals, dgFloat32 *const penetration, dgInt32 maxSize, dgInt32 threadIndex);

	dgInt32 CollideContinue(dgCollision *const collisionA, const dgMatrix &matrixA, const dgVector &velocA, const dgVector &omegaA,
	                        dgCollision *const collisionB, const dgMatrix &matrixB, const dgVector &velocB, const dgVector &omegaB,
	                        dgFloat32 &timeStep, dgTriplex *const points, dgTriplex *const normals, dgFloat32 *const penetration, dgInt32 maxSize, dgInt32 threadIndex);
	dgInt32 CollideContinueSimd(dgCollision *const collisionA, const dgMatrix &matrixA, const dgVector &velocA, const dgVector &omegaA,
	                            dgCollision *const collisionB, const dgMatrix &matrixB, const dgVector &velocB, const dgVector &omegaB,
	                            dgFloat32 &timeStep, dgTriplex *const points, dgTriplex *const normals, dgFloat32 *const penetration, dgInt32 maxSize, dgInt32 threadIndex);


	dgInt32 ClosestPoint(const dgTriplex &point, dgCollision *const collision, const dgMatrix &matrix, dgTriplex &contact, dgTriplex &normal, dgInt32 threadIndex) const;
	dgInt32 ClosestPoint(dgCollision *const collisionA, const dgMatrix &matrixA, dgCollision *const collisionB, const dgMatrix &matrixB,
	                     dgTriplex &contactA, dgTriplex &contactB, dgTriplex &normalAB, dgInt32 threadIndex) const;


	void SetFrictionThreshold(dgFloat32 acceletion);


	dgBody *GetIslandBody(const void *const island, dgInt32 index) const;


	void SetIslandUpdateCallback(OnIslandUpdate callback);
	void SetDestroyCollisionCallback(OnDestroyCollision shape);
	void SetLeavingWorldCallback(OnLeavingWorldAction callback);
	void SetBodyDestructionByExeciveForce(OnBodyDestructionByExeciveForce callback);
//	void ForEachBodyInAABB (const dgVector& q0, const dgVector& q1, OnLeavingWorldAction callback);

	dgBody *CreateBody(dgCollision *const collision, const dgMatrix &matrix);
	void DestroyBody(dgBody *const body);
	void DestroyAllBodies();

//	void AddToBreakQueue (const dgContact* const contactJoint, dgFloat32 masValue);
	void AddToBreakQueue(const dgContact *const contactJoint, dgBody *const body, dgFloat32 maxForce);

	// modify the velocity and angular velocity of a body in such a way
	// that the velocity of pointPosit is increase by pointDeltaVeloc
	// pointVeloc and pointPosit are in world space
	void AddBodyImpulse(dgBody *body, const dgVector &pointDeltaVeloc, const dgVector &pointPosit);

	void ApplyImpulseArray(dgBody *body, dgInt32 count, dgInt32 strideInBytes, const dgFloat32 *const impulseArray, const dgFloat32 *const pointArray);

	// apply the transform matrix to the body and recurse trough all bodies attached to this body with a
	// bilateral joint contact joint are ignored.
	void BodySetMatrix(dgBody *body, const dgMatrix &matrix);
//	void FreezeBody (dgBody *body);
//	void UnfreezeBody (dgBody *body);
//	dgInt32 GetActiveBodiesCount() const;

	dgInt32 GetBodiesCount() const;
	dgInt32 GetConstraintsCount() const;


	dgUnsigned32 GetBoxID() const;
	dgUnsigned32 GetConeID() const;
	dgUnsigned32 GetSphereID() const;
	dgUnsigned32 GetConvexID() const;
	dgUnsigned32 GetEllipseID() const;
	dgUnsigned32 GetCapsuleID() const;
	dgUnsigned32 GetCylinderID() const;
	dgUnsigned32 GetConvexHullID() const;
	dgUnsigned32 GetChamferCylinderID() const;
	dgUnsigned32 GetConvexHullModifierID() const;
	dgUnsigned32 GetPolygonSoupID() const;
	dgUnsigned32 GetSceneID() const;
	dgUnsigned32 GetCompoundCollisionID() const;

	dgCollision *CreateNull();
	dgCollision *CreateSphere(dgFloat32 radiusdg, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateCone(dgFloat32 radius, dgFloat32 height, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateCapsule(dgFloat32 radius, dgFloat32 height, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateCylinder(dgFloat32 radius, dgFloat32 height, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateBox(dgFloat32 dx, dgFloat32 dy, dgFloat32 dz, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateEllipse(dgFloat32 rx, dgFloat32 ry, dgFloat32 rz, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateConvexHull(dgInt32 count, const dgFloat32 *vertexArray, dgInt32 strideInBytes, dgFloat32 tolerance, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateChamferCylinder(dgFloat32 radius, dgFloat32 height, dgInt32 shapeID, const dgMatrix &offsetMatrix = dgGetIdentityMatrix());
	dgCollision *CreateConvexModifier(dgCollision *convexCollision);
	dgCollision *CreateCollisionCompound(dgInt32 count, dgCollision *const array[]);
//	dgCollision* CreateCollisionCompoundBreakable (dgInt32 count, dgMeshEffect* const solidArray[], dgMeshEffect* const splitePlanes[],
//													  dgMatrix* const matrixArray, dgInt32* const idArray, dgFloat32* const mass, dgInt32 debriID,
//													  dgCollisionCompoundBreakableCallback callback, void* buildUsedData);

	dgCollision *CreateCollisionCompoundBreakable(dgInt32 count, dgMeshEffect *const solidArray[], const dgInt32 *const idArray,
	        const dgFloat32 *const densities, const dgInt32 *const internalFaceMaterial, dgInt32 debriID, dgFloat32 gap);


	dgCollision *CreateBVH();
	dgCollision *CreateStaticUserMesh(const dgVector &boxP0, const dgVector &boxP1, const dgUserMeshCreation &data);
	dgCollision *CreateBVHFieldCollision(dgInt32 width, dgInt32 height, dgInt32 contructionMode,
	                                     const dgUnsigned16 *const elevationMap, const dgInt8 *const atributeMap, dgFloat32 horizontalScale, dgFloat32 vertcalScale);

	dgCollision *CreateScene();

	void Serialize(const dgCollision *shape, dgSerialize deserialization, void *const userData) const;
	dgCollision *CreateFromSerialization(dgDeserialize deserialization, void *const userData);

	void RemoveFromCache(dgCollision *const collision);
	void ReleaseCollision(dgCollision *const collision);

	dgUpVectorConstraint *CreateUpVectorConstraint(const dgVector &pin, dgBody *body);

	dgBallConstraint *CreateBallConstraint(const dgVector &pivot, dgBody *const body0, dgBody *refBody = NULL);
	dgHingeConstraint *CreateHingeConstraint(const dgVector &pivot, const dgVector &pin, dgBody *const body0, dgBody *refBody = NULL);
	dgSlidingConstraint *CreateSlidingConstraint(const dgVector &pivot, const dgVector &pin, dgBody *const body0, dgBody *refBody = NULL);
	dgCorkscrewConstraint *CreateCorkscrewConstraint(const dgVector &pivot, const dgVector &pin, dgBody *const body0, dgBody *refBody = NULL);
	dgUniversalConstraint *CreateUniversalConstraint(const dgVector &pivot, const dgVector &pin0, const dgVector &pin1, dgBody *const body0, dgBody *body1 = NULL);


	void DestroyConstraint(dgConstraint *constraint);
	dgUnsigned32 CreateBodyGroupID();
	void RemoveAllGroupID();

	dgUnsigned32 GetDefualtBodyGroupID() const;
	dgContactMaterial *GetMaterial(dgUnsigned32 bodyGroupId0, dgUnsigned32 bodyGroupId1) const;

	dgContactMaterial *GetFirstMaterial() const;
	dgContactMaterial *GetNextMaterial(dgContactMaterial *material) const;

	OnGetPerformanceCountCallback GetPerformaceFuntion()const ;
	void SetPerfomanceCounter(OnGetPerformanceCountCallback callback);

//	dgUnsigned32 GetPerfomanceTicks (dgInt32 thread, dgUnsigned32 entry) const;
	dgUnsigned32 GetPerfomanceTicks(dgUnsigned32 entry) const;
	dgUnsigned32 GetThreadPerfomanceTicks(dgUnsigned32 threadIndex) const;

	void dgGetUserLock() const;
	void dgReleasedUserLock() const;
	void dgGetIndirectLock(dgInt32 *lockVar);
	void dgReleaseIndirectLock(dgInt32 *lockVar);


	dgBody *GetSentinelBody() const;
	dgMemoryAllocator *GetAllocator() const;

private:
	void CalculateContacts(dgCollidingPairCollector::dgPair *const pair, dgFloat32 timestep, dgInt32 threadIndex);
	void CalculateContactsSimd(dgCollidingPairCollector::dgPair *const pair, dgFloat32 timestep, dgInt32 threadIndex);

	void SortContacts(dgContactPoint *const contact, dgInt32 count) const;
	dgInt32 ReduceContacts(dgInt32 count, dgContactPoint *const contact, dgInt32 maxCount, dgFloat32 tol, dgInt32 arrayIsSorted = 0) const;
	dgInt32 PruneContacts(dgInt32 count, dgContactPoint *const contact, dgInt32 maxCount = (DG_CONSTRAINT_MAX_ROWS / 3)) const;

	dgInt32 CalculateHullToHullContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateHullToHullContactsSimd(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateBoxToSphereContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateSphereToSphereContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateCapsuleToSphereContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateCapsuleToCapsuleContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 SphereSphereCollision(const dgVector &sph0, dgFloat32 radius0, const dgVector &sph1, dgFloat32 radius1, dgCollisionParamProxy &proxy) const;


	dgInt32 ValidateContactCache(dgBody *const convexBody, dgBody *const otherBody, dgContact *const contact) const;


	dgInt32 CalculatePolySoupToBoxContactsDescrete(dgBody *soup, dgBody *box, dgContactPoint *const contact, dgInt32 maxContacts) const;
	dgInt32 CalculatePolySoupToHullContactsDescrete(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculatePolySoupToHullContactsDescreteSimd(dgCollisionParamProxy &proxy) const;

	dgInt32 CalculatePolySoupToSphereContactsDescrete(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculatePolySoupToElipseContactsDescrete(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculatePolySoupToSphereContactsContinue(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateConvexToNonConvexContactsContinue(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateConvexToNonConvexContactsContinueSimd(dgCollisionParamProxy &proxy) const;


//	dgInt32 CalculateConvexToConvexContinuesContacts (dgFloat32& timestep, dgBody* body1, dgBody* body2, dgContactPoint contactOut[]) const;
//	dgInt32 CalculateConvexToConvexContinuesContacts (dgCollisionParamProxy& proxy) const;
//	dgInt32 CalculateConvexToConvexContinuesContacts (dgCollisionParamProxy& proxy) const;
//	dgInt32 CalculateConvexToConvexContacts (dgFloat32& timestep, dgBody* conv1, dgBody* conv2, dgFloat32 penetrationPadding, dgContactPoint* const contact) const;
//	dgInt32 CalculateConvexToConvexContactsSimd (dgFloat32& timestep, dgBody* conv1, dgBody* conv2, dgFloat32 penetrationPadding, dgContactPoint* const contact) const;
	dgInt32 CalculateConvexToConvexContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateConvexToConvexContactsSimd(dgCollisionParamProxy &proxy) const;

//	dgInt32 CalculateConvexToNonConvexContacts (dgFloat32& timestep, dgBody* conv, dgBody* nConv, dgContactPoint* const contact, dgInt32 maxContacts) const;
	dgInt32 CalculateConvexToNonConvexContacts(dgCollisionParamProxy &proxy) const;
	dgInt32 CalculateConvexToNonConvexContactsSimd(dgCollisionParamProxy &proxy) const;

	dgInt32 FilterPolygonEdgeContacts(dgInt32 count, dgContactPoint *const contact) const;

	void ProcessTriggers(dgCollidingPairCollector::dgPair *const pair, dgFloat32 timestep, dgInt32 threadIndex);
	void ProcessContacts(dgCollidingPairCollector::dgPair *const pair, dgFloat32 timestep, dgInt32 threadIndex);
	void ProcessCachedContacts(dgContact *const contact, const dgContactMaterial *const material, dgFloat32 timestep, dgInt32 threadIndex) const;

	void ConvexContacts(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;
	void ConvexContactsSimd(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;
	void CompoundContacts(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;
	void CompoundContactsSimd(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;

	void SceneContacts(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;
	void SceneContacts(const dgCollisionScene::dgProxy &sceneProxy, dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;

	void SceneContactsSimd(dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;
	void SceneContactsSimd(const dgCollisionScene::dgProxy &sceneProxy, dgCollidingPairCollector::dgPair *const pair, dgCollisionParamProxy &proxy) const;

	dgInt32 ClosestPoint(dgCollisionParamProxy &proxy) const;
	dgInt32 ClosestCompoundPoint(dgBody *const compoundConvexA, dgBody *const collisionB, dgTriplex &contactA, dgTriplex &contactB, dgTriplex &normalAB, dgInt32 threadIndex) const;

	bool AreBodyConnectedByJoints(dgBody *const origin, dgBody *const target);

	void AddSentinelBody();


	static void InitConvexCollision();
	static dgUnsigned32 dgApi GetPerformanceCount();

	dgUnsigned32 m_dynamicsLru;
	dgUnsigned32 m_broadPhaseLru;
	dgUnsigned32 m_inUpdate;
	dgUnsigned32 m_solverMode;
	dgUnsigned32 m_frictionMode;
	dgUnsigned32 m_bodyGroupID;
	dgUnsigned32 m_defualtBodyGroupID;
	dgUnsigned32 m_bodiesUniqueID;
	dgUnsigned32 m_numberOfTheads;
	dgUnsigned32 m_maxTheads;


	dgFloat32 m_freezeAccel2;
	dgFloat32 m_freezeAlpha2;
	dgFloat32 m_freezeSpeed2;
	dgFloat32 m_freezeOmega2;
	dgFloat32 m_frictiomTheshold;

	dgSolverSleepTherfesholds m_sleepTable[DG_SLEEP_ENTRIES];

	dgInt32 m_genericLRUMark;
	dgInt32 m_islandMemorySizeInBytes;
	dgInt32 m_bodiesMemorySizeInBytes;
	dgInt32 m_jointsMemorySizeInBytes;
	dgInt32 m_pairMemoryBufferSizeInBytes;
	void *m_jointsMemory;
	void *m_bodiesMemory;
	void *m_islandMemory;
	void *m_pairMemoryBuffer;


	dgInt32 m_singleIslandMultithreading;
	dgInt32 m_contactBuffersSizeInBytes[DG_MAXIMUN_THREADS];
	dgInt32 m_jacobiansMemorySizeInBytes[DG_MAXIMUN_THREADS];
	dgInt32 m_internalForcesMemorySizeInBytes[DG_MAXIMUN_THREADS];
	void *m_jacobiansMemory[DG_MAXIMUN_THREADS];
	void *m_internalForcesMemory[DG_MAXIMUN_THREADS];
	void *m_contactBuffers[DG_MAXIMUN_THREADS];

	dgBody *m_sentionelBody;
	dgCollisionPoint *m_pointCollision;

	void *m_userData;
	dgMemoryAllocator *m_allocator;
	dgCpuClass m_cpu;
	OnIslandUpdate m_islandUpdate;
	OnDestroyCollision m_destroyCollision;
	OnLeavingWorldAction m_leavingWorldNotify;
	OnGetPerformanceCountCallback m_getPerformanceCount;
	OnBodyDestructionByExeciveForce m_destroyBodyByExeciveForce;

	dgDetroyBodyByForce m_destroyeddBodiesPool;
	dgUnsigned32 m_perfomanceCounters[m_counterSize];

	dgTree<void *, unsigned> m_perInstanceData;

	dgThreads m_threadsManager;
	dgWorldDynamicUpdate m_dynamicSolver;

	friend class dgBody;
	friend class dgActiveContacts;
	friend class dgBroadPhaseCell;
	friend class dgUserConstraint;
	friend class dgBodyMasterList;
	friend class dgJacobianMemory;
	friend class dgCollisionScene;
	friend class dgCollisionConvex;
	friend class dgCollisionCompound;
	friend class dgCollisionHeightField;
	friend class dgWorldDynamicUpdate;
	friend class dgParallelSolverSolve;
	friend class dgBroadPhaseCollision;
	friend class dgSolverWorlkerThreads;
	friend class dgCollidingPairCollector;
	friend class dgParallelSolverClear;
	friend class dgParallelSolverUpdateForce;
	friend class dgParallelSolverUpdateVeloc;
	friend class dgParallelSolverBodyInertia;
	friend class dgParallelSolverInitFeedbackUpdate;

	friend class dgBroadPhaseApplyExternalForce;
	friend class dgParallelSolverCalculateForces;
	friend class dgParallelSolverJointAcceleration;
	friend class dgParallelSolverBuildJacobianRows;
	friend class dgBroadPhaseCellPairsWorkerThread;
	friend class dgParallelSolverInitInternalForces;
	friend class dgParallelSolverBuildJacobianMatrix;

	friend class dgBroadPhaseMaterialCallbackWorkerThread;
	friend class dgBroadPhaseCalculateContactsWorkerThread;
} DG_GCC_VECTOR_ALIGMENT ;


inline void dgWorld::dgGetUserLock() const {
	if (m_numberOfTheads > 1) {
		m_threadsManager.dgGetLock();
	}
}

inline void dgWorld::dgReleasedUserLock() const {
	if (m_numberOfTheads > 1) {
		m_threadsManager.dgReleaseLock();
	}
}


inline void dgWorld::dgGetIndirectLock(dgInt32 *lockVar) {
	m_threadsManager.dgGetIndirectLock(lockVar);
}

inline void dgWorld::dgReleaseIndirectLock(dgInt32 *lockVar) {
	m_threadsManager.dgReleaseIndirectLock(lockVar);
}

inline dgMemoryAllocator *dgWorld::GetAllocator() const {
	return m_allocator;
}

inline void dgWorld::AddToBreakQueue(const dgContact *const contactJoint, dgBody *const body, dgFloat32 maxForce) {
//	if (body->GetCollision()->GetBreakImpulse() < maxForce) {
	if (m_destroyeddBodiesPool.m_count < DG_MAX_DESTROYED_BODIES_BY_FORCE) {
		if (body->m_isInDerstruionArrayLRU != body->m_dynamicsLru) {
			body->m_isInDerstruionArrayLRU = body->m_dynamicsLru;
			m_destroyeddBodiesPool.m_force[m_destroyeddBodiesPool.m_count] = maxForce;
			m_destroyeddBodiesPool.m_bodies[m_destroyeddBodiesPool.m_count] = body;
			m_destroyeddBodiesPool.m_joint[m_destroyeddBodiesPool.m_count] = contactJoint;
			m_destroyeddBodiesPool.m_count ++;
		} else {
			for (dgInt32 i = 0; i < m_destroyeddBodiesPool.m_count; i ++) {
				if (m_destroyeddBodiesPool.m_bodies[i] == body) {
					if (maxForce > m_destroyeddBodiesPool.m_force[i]) {
						m_destroyeddBodiesPool.m_force[i] = maxForce;
						m_destroyeddBodiesPool.m_joint[i] = contactJoint;
					}
				}
			}
		}
	}
//	}
}


//inline void dgWorld::AddToBreakQueue (const dgContact* const contactJoint, dgFloat32 maxImpulse)
//{
//	AddToBreakQueue (contactJoint, contactJoint->m_body0, maxForce);
//	AddToBreakQueue (contactJoint, contactJoint->m_body1, maxForce);
//}



#endif // !defined(AFX_DGPHYSICSWORLD_H__EC18C699_D48D_448F_A510_A865B2CC0789__INCLUDED_)
