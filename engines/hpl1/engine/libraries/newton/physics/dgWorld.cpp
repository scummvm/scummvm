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

#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#include "dgCollisionBox.h"
#include "dgCollisionCapsule.h"
#include "dgCollisionChamferCylinder.h"
#include "dgCollisionCompound.h"
#include "dgCollisionCone.h"
#include "dgCollisionConvexHull.h"
#include "dgCollisionConvexModifier.h"
#include "dgCollisionCylinder.h"
#include "dgCollisionEllipse.h"
#include "dgCollisionNull.h"
#include "dgCollisionScene.h"
#include "dgCollisionSphere.h"
#include "dgMinkowskiConv.h"
#include "dgWorldDynamicUpdate.h"


#include "dgBallConstraint.h"
#include "dgConnectorConstraint.h"
#include "dgCorkscrewConstraint.h"
#include "dgHingeConstraint.h"
#include "dgSlidingConstraint.h"
#include "dgUniversalConstraint.h"
#include "dgUpVectorConstraint.h"
#include "dgUserConstraint.h"

//#include "dgPointToCurveConstraint.h"

#define DG_INITIAL_ISLAND_SIZE (1024 * 4)
#define DG_INITIAL_BODIES_SIZE (1024 * 4)
#define DG_INITIAL_JOINTS_SIZE (1024 * 4)
#define DG_INITIAL_JACOBIAN_SIZE (1024 * 16)
#define DG_INITIAL_CONTATCT_SIZE (1024 * 64)

/*
 static  char *xxx[10] = {"bbbbbbbbbb",
 "baaaaaaaab",
 "babbbbbaab",
 "babaaabaab",
 "baaababaab",
 "bbbaaabaab",
 "bbbbaabaab",
 "babbbbbaab",
 "baaaaaaaab",
 "bbbbbbbbbb"};

 struct myPath: public dgPathFinder<dgInt32, float>
 {
 dgInt32 goalx;
 dgInt32 goaly;
 myPath ()
 :dgPathFinder<dgInt32, float>(1024, 10)
 {
 for (int i = 0; i < 10; i ++) {
 strcpy (&m_map[i][0], xxx[i]);
 }
 }

 const dgPathNode<dgInt32, float>* CalCulatePath (dgInt32 source, dgInt32 goal)
 {
 goalx = goal % 10;
 goaly = goal / 10;
 return dgPathFinder<dgInt32, float>::CalCulatePath (source, goal) ;
 }


 float GetCostFromParent(const dgPathNode<dgInt32, float>& node) const
 {
 dgInt32 x;
 dgInt32 y;
 dgInt32 x0;
 dgInt32 y0;
 dgInt32 x1;
 dgInt32 y1;
 dgInt32 id;

 id = node.GetId();
 x = id % 10;
 y = id / 10;

 const dgPathNode<dgInt32, float>* parent = node.GetParent();
 id = parent->GetId();
 x0 = id % 10;
 y0 = id / 10;

 const dgPathNode<dgInt32, float>* grandParent = parent->GetParent();
 x1 = 2 * x0 - x;
 y1 = 2 * y0 - y;
 if (grandParent) {
 id = grandParent->GetId();
 x1 = id % 10;
 y1 = id / 10;
 }

 dgInt32 dx0;
 dgInt32 dy0;
 dgInt32 dx1;
 dgInt32 dy1;
 float penalty;

 dx0 = x0 - x;
 dy0 = y0 - y;
 dx1 = x1 - x0;
 dy1 = y1 - y0;
 penalty = 0.0f;
 if (dx1 * dy0 - dx0 * dy1) {
 penalty = dgFloat32(1.0f);
 }

 static dgInt32 xxxx;
 if (!xxxx){
 xxxx = 1;
 penalty = 9.1f;
 }

 return (xxx[y][x] == 'a') ? (dgFloat32(1.0f) + penalty): 50.0f;
 }

 float GetEstimatedCostToGoal(dgInt32 id) const
 {
 dgInt32 x;
 dgInt32 y;

 x = id % 10 - goalx;
 y = id / 10 - goaly;
 return dgSqrt ((float)(x * x + y * y));
 }

 dgInt32 EnumerateChildren(dgInt32 parent, dgInt32 array[]) const
 {
 dgInt32 x;
 dgInt32 y;

 x = parent % 10;
 y = parent / 10;

 array[0]   = (y - 1) * 10 + x;
 array[1]   = (y - 0) * 10 + x - 1;
 array[2]   = (y + 1) * 10 + x;
 array[3]   = (y + 0) * 10 + x + 1;
 return 4;
 }

 char m_map[20][20];
 };


 void xxxxx()
 {
 myPath path;
 const dgPathNode<dgInt32, float>* firtNode;
 for (firtNode = path.CalCulatePath (5 * 10 + 3, 4 * 10 + 8); firtNode; firtNode= firtNode->GetNext()) {
 dgInt32 id;
 dgInt32 x;
 dgInt32 y;

 id = firtNode->GetId();
 x = id % 10;
 y = id / 10;
 path.m_map[y][x] = '_';
 }
 }
 */

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgWorld::dgWorld(dgMemoryAllocator *allocator) : // dgThreadHive(),
	dgBodyMasterList(allocator), dgBroadPhaseCollision(allocator), dgBodyMaterialList(
	    allocator),
	dgBodyCollisionList(allocator), dgActiveContacts(allocator), dgCollidingPairCollector(), m_perInstanceData(
	    allocator),
	m_threadsManager(), m_dynamicSolver() {
	dgInt32 steps;
	dgFloat32 freezeAccel2;
	dgFloat32 freezeAlpha2;
	dgFloat32 freezeSpeed2;
	dgFloat32 freezeOmega2;

	// init exact arithmetic functions
	m_allocator = allocator;

	// dgThreadHive::SetThreadCount(16);
	// dgThreadHive::SetThreadCount(32);

	//_control87 (_EM_ZERODIVIDE | _EM_INEXACT | _EM_OVERFLOW | _EM_INVALID, _MCW_EM);

	m_inUpdate = 0;
	m_bodyGroupID = 0;
	//  m_activeBodiesCount = 0;

	m_defualtBodyGroupID = CreateBodyGroupID();
	m_islandMemorySizeInBytes = DG_INITIAL_ISLAND_SIZE;
	m_bodiesMemorySizeInBytes = DG_INITIAL_BODIES_SIZE;
	m_jointsMemorySizeInBytes = DG_INITIAL_JOINTS_SIZE;

	m_pairMemoryBufferSizeInBytes = 1024 * 64 * sizeof(void *);
	m_pairMemoryBuffer = m_allocator->MallocLow(m_pairMemoryBufferSizeInBytes);
	m_islandMemory = m_allocator->MallocLow(m_islandMemorySizeInBytes);
	m_jointsMemory = m_allocator->MallocLow(m_jointsMemorySizeInBytes);
	m_bodiesMemory = m_allocator->MallocLow(m_bodiesMemorySizeInBytes);
	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
		m_jacobiansMemorySizeInBytes[i] = DG_INITIAL_JACOBIAN_SIZE;
		m_jacobiansMemory[i] = m_allocator->MallocLow(
		                           m_jacobiansMemorySizeInBytes[i]);

		m_internalForcesMemorySizeInBytes[i] = DG_INITIAL_BODIES_SIZE;
		m_internalForcesMemory[i] = m_allocator->MallocLow(
		                                m_internalForcesMemorySizeInBytes[i]);

		m_contactBuffersSizeInBytes[i] = DG_INITIAL_CONTATCT_SIZE;
		m_contactBuffers[i] = m_allocator->MallocLow(
		                          m_contactBuffersSizeInBytes[i]);
	}

	m_genericLRUMark = 0;
	m_singleIslandMultithreading = 1;

	m_solverMode = 0;
	m_frictionMode = 0;
	m_dynamicsLru = 0;
	m_broadPhaseLru = 0;

	m_bodiesUniqueID = 0;
	//  m_bodiesCount = 0;
	m_frictiomTheshold = dgFloat32(0.25f);

	m_userData = NULL;
	m_islandUpdate = NULL;
	m_destroyCollision = NULL;
	m_leavingWorldNotify = NULL;
	m_destroyBodyByExeciveForce = NULL;

	m_freezeAccel2 = DG_FREEZE_MAG2;
	m_freezeAlpha2 = DG_FREEZE_MAG2;
	m_freezeSpeed2 = DG_FREEZE_MAG2 * dgFloat32(0.1f);
	m_freezeOmega2 = DG_FREEZE_MAG2 * dgFloat32(0.1f);

	steps = 1;
	freezeAccel2 = m_freezeAccel2;
	freezeAlpha2 = m_freezeAlpha2;
	freezeSpeed2 = m_freezeSpeed2;
	freezeOmega2 = m_freezeOmega2;
	for (dgInt32 i = 0; i < DG_SLEEP_ENTRIES; i++) {
		m_sleepTable[i].m_maxAccel = freezeAccel2;
		m_sleepTable[i].m_maxAlpha = freezeAlpha2;
		m_sleepTable[i].m_maxVeloc = freezeSpeed2;
		m_sleepTable[i].m_maxOmega = freezeOmega2;
		m_sleepTable[i].m_steps = steps;
		steps += 7;
		freezeAccel2 *= dgFloat32(1.5f);
		freezeAlpha2 *= dgFloat32(1.4f);
		freezeSpeed2 *= dgFloat32(1.5f);
		freezeOmega2 *= dgFloat32(1.5f);
	}

	steps += 300;
	m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxAccel *= dgFloat32(100.0f);
	m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxAlpha *= dgFloat32(100.0f);
	m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxVeloc = 0.25f;
	m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxOmega = 0.1f;
	m_sleepTable[DG_SLEEP_ENTRIES - 1].m_steps = steps;

	m_cpu = dgNoSimdPresent;
	m_numberOfTheads = 1;

	SetHardwareMode(1);
	SetThreadsCount(DG_MAXIMUN_THREADS);
	m_maxTheads = m_numberOfTheads;

	SetHardwareMode(0);
	SetThreadsCount(1);

	dgBroadPhaseCollision::Init();
	dgCollidingPairCollector::Init();

	m_pointCollision = new (m_allocator) dgCollisionPoint(m_allocator);
	AddSentinelBody();
	SetPerfomanceCounter(NULL);
}

dgWorld::~dgWorld() {
	DestroyAllBodies();
	RemoveAllGroupID();
	m_destroyCollision = NULL;
	ReleaseCollision(m_pointCollision);
	DestroyBody(m_sentionelBody);

	m_allocator->FreeLow(m_jointsMemory);
	m_allocator->FreeLow(m_bodiesMemory);
	m_allocator->FreeLow(m_islandMemory);
	m_allocator->FreeLow(m_pairMemoryBuffer);
	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
		m_allocator->FreeLow(m_jacobiansMemory[i]);
		m_allocator->FreeLow(m_internalForcesMemory[i]);
		m_allocator->FreeLow(m_contactBuffers[i]);
	}
}

dgUnsigned32 dgWorld::GetPerformanceCount() {
	return 0;
}

void dgWorld::AddSentinelBody() {
	dgCollision *collision;

	collision = new (m_allocator) dgCollisionNull(m_allocator, 0x4352fe67);
	m_sentionelBody = CreateBody(collision, dgGetIdentityMatrix());
	ReleaseCollision(collision);

	//  dgBodyMasterList::m_sentinel = m_sentionelBody;
	dgCollidingPairCollector::m_sentinel = m_sentionelBody;
}

dgBody *dgWorld::GetSentinelBody() const {
	return m_sentionelBody;
}

void dgWorld::SetSolverMode(dgInt32 mode) {
	m_solverMode = dgUnsigned32(GetMax(dgInt32(0), mode));
}

void dgWorld::SetFrictionMode(dgInt32 mode) {
	m_frictionMode = dgUnsigned32(mode);
}

void dgWorld::SetHardwareMode(dgInt32 mode) {
	m_cpu = dgNoSimdPresent;
	if (mode) {
		m_cpu = dgGetCpuType();
	}
}

dgInt32 dgWorld::GetHardwareMode(char *description) const {
	dgInt32 mode;

	//  if (0) {
	//  } else {
	if (m_cpu == dgNoSimdPresent) {
		mode = 0;
		if (description) {
			snprintf(description, 5, "x87");
		}
	} else {
		mode = 1;
		if (description) {
			snprintf(description, 5, "simd");
		}
	}
	//  }
	return mode;
}

void dgWorld::SetThreadsCount(dgInt32 count) {
	// count = 1;

	m_threadsManager.CreateThreaded(count);
	m_numberOfTheads = dgUnsigned32(m_threadsManager.GetThreadCount());
}

dgInt32 dgWorld::GetThreadsCount() const {
	return dgInt32(m_numberOfTheads);
}

dgInt32 dgWorld::GetMaxThreadsCount() const {
	return dgInt32(m_maxTheads);
}

void dgWorld::EnableThreadOnSingleIsland(dgInt32 mode) {
	// for now disable this option
	//  m_singleIslandMultithreading = 1;
	m_singleIslandMultithreading = mode ? 1 : 0;
}

dgInt32 dgWorld::GetThreadOnSingleIsland() const {
	return m_singleIslandMultithreading ? 1 : 0;
}

void dgWorld::SetFrictionThreshold(dgFloat32 acceleration) {
	m_frictiomTheshold = GetMax(dgFloat32(1.0e-2f), acceleration);
}

void dgWorld::RemoveAllGroupID() {
	while (dgBodyMaterialList::GetCount()) {
		dgBodyMaterialList::Remove(dgBodyMaterialList::GetRoot());
	}
	m_bodyGroupID = 0;
	m_defualtBodyGroupID = CreateBodyGroupID();
}

void dgWorld::DestroyAllBodies() {
	dgBody *body;
	dgBodyMasterList::dgListNode *node;

	dgBodyMasterList &me = *this;
	//  dgBodyMasterList::Iterator iter(me);
	//  for (iter.Begin(); iter; ) {

	NEWTON_ASSERT(
	    dgBodyMasterList::GetFirst()->GetInfo().GetBody() == m_sentionelBody);
	for (node = me.GetFirst()->GetNext(); node;) {
		body = node->GetInfo().GetBody();
		node = node->GetNext();
		DestroyBody(body);
	}

	NEWTON_ASSERT(me.GetFirst()->GetInfo().GetCount() == 0);
	NEWTON_ASSERT(dgBodyCollisionList::GetCount() == 0);
}

dgBody *dgWorld::CreateBody(dgCollision *const collision,
                            const dgMatrix &matrix) {
	dgBody *body;

	NEWTON_ASSERT(collision);

	body = new (m_allocator) dgBody();
	NEWTON_ASSERT((sizeof(dgBody) & 0xf) == 0);
	NEWTON_ASSERT((dgUnsigned64(body) & 0xf) == 0);

	body->reset();

	//  m_bodiesCount ++;
	m_bodiesUniqueID++;

	body->m_world = this;

	body->m_freeze = false;
	body->m_sleeping = false;
	body->m_autoSleep = true;
	body->m_isInWorld = true;
	body->m_equilibrium = false;
	body->m_continueCollisionMode = false;
	body->m_collideWithLinkedBodies = true;
	body->m_solverInContinueCollision = false;
	body->m_spawnnedFromCallback = dgUnsigned32(m_inUpdate ? true : false);
	body->m_uniqueID = dgInt32(m_bodiesUniqueID);

	dgBodyMasterList::AddBody(body);

	//  dgBodyActiveList___::AddBody(body);
	//  NEWTON_ASSERT (body->m_activeNode);

	//  body->m_freezeAccel2 = m_freezeAccel2;
	//  body->m_freezeAlpha2 = m_freezeAlpha2;
	//  body->m_freezeSpeed2 = m_freezeSpeed2;
	//  body->m_freezeOmega2 = m_freezeOmega2;

	body->SetCentreOfMass(
	    dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	             dgFloat32(1.0f)));
	body->SetLinearDamping(dgFloat32(0.1045f));
	body->SetAngularDamping(
	    dgVector(dgFloat32(0.1045f), dgFloat32(0.1045f), dgFloat32(0.1045f),
	             dgFloat32(0.0f)));

	body->AttachCollision(collision);
	body->m_bodyGroupId = dgInt32(m_defualtBodyGroupID);

	body->SetMassMatrix(DG_INFINITE_MASS * dgFloat32(2.0f), DG_INFINITE_MASS,
	                    DG_INFINITE_MASS, DG_INFINITE_MASS);
	dgBroadPhaseCollision::Add(body);

	// body->SetMatrix (dgGetIdentityMatrix());
	body->SetMatrix(matrix);
	body->m_invWorldInertiaMatrix[3][3] = dgFloat32(1.0f);
	return body;
}

void dgWorld::DestroyBody(dgBody *const body) {
	if (body->m_destructor) {
		body->m_destructor(*body);
	}

	dgBroadPhaseCollision::Remove(body);

	//  m_bodiesCount --;
	//  while (body->m_firstConstraintLink) {
	//      DestroyConstraint (body->m_firstConstraintLink->m_constraint);
	//  }
	dgBodyMasterList::RemoveBody(body);

	NEWTON_ASSERT(body->m_collision);
	ReleaseCollision(body->m_collision);

	delete body;
}

void dgWorld::DestroyConstraint(dgConstraint *const constraint) {
	RemoveConstraint(constraint);
	delete constraint;
}

/*
 void dgWorld::UnfreezeBody (dgBody *body)
 {
 if (body->m_activation && !body->m_active) {
 body->m_activation (*body, 1);
 }
 body->m_active = true;

 }

 void dgWorld::FreezeBody (dgBody *body)
 {
 NEWTON_ASSERT (0);
 if (body->m_activeNode) {
 NEWTON_ASSERT (0);
 dgBodyActiveList___::RemoveBody(body);
 if (body->m_activation && body->m_active) {
 body->m_activation (*body, 0);
 }
 body->m_active = false;
 }
 }
 */

void dgWorld::UpdateCollision() {
	dgFloat32 timestep;
	NEWTON_ASSERT(m_inUpdate == 0);

	m_threadsManager.ClearTimers();
	memset(m_perfomanceCounters, 0, sizeof(m_perfomanceCounters));
	dgUnsigned32 ticks = m_getPerformanceCount();

	m_inUpdate++;
	NEWTON_ASSERT(m_numberOfTheads >= 1);

#ifdef _WIN32
#ifndef __USE_DOUBLE_PRECISION__
	dgUnsigned32 controlWorld = dgControlFP(0xffffffff, 0);
	dgControlFP(_PC_53, _MCW_PC);
#endif
#endif

	timestep = dgFloat32(0.0f);

	if (m_cpu == dgSimdPresent) {
#ifdef DG_BUILD_SIMD_CODE
		simd_env rounding;
		rounding = simd_get_ctrl();
		simd_set_FZ_mode();

		UpdateContactsSimd(timestep, true);
		simd_set_ctrl(rounding);
#endif

	} else {
		UpdateContacts(timestep, true);
	}
	m_inUpdate--;

#ifdef _WIN32
#ifndef __USE_DOUBLE_PRECISION__
	dgControlFP(controlWorld, _MCW_PC);
#endif
#endif

	m_perfomanceCounters[m_worldTicks] = m_getPerformanceCount() - ticks;
}

void dgWorld::Update(dgFloat32 timestep) {
	dgUnsigned32 ticks;
// timestep = 1.0f/ 60.0f;
// timestep = 1.0f/ 120.0f;
// timestep = 1.0f/ 180.0f;
// timestep = 1.0f/ 240.0f;
// timestep = 1.0f/ 300.0f;
// timestep = 1.0f/ 600.0f;
// timestep = 1.0f/ 1000.0f;

// m_cpu = dgNoSimdPresent;
// m_cpu = dgSimdPresent;
// m_solverMode = 1;

// xxxxx();

// static int xxx;
// dgTrace (("pass %d\n", xxx));
// xxx ++;

// m_cpu = dgNoSimdPresent;
#ifdef _LINUX_VER
//		m_cpu = dgNoSimdPresent;
#endif

	NEWTON_ASSERT(m_inUpdate == 0);

	m_threadsManager.ClearTimers();
	memset(m_perfomanceCounters, 0, sizeof(m_perfomanceCounters));

	ticks = m_getPerformanceCount();

	m_destroyeddBodiesPool.m_count = 0;

	m_inUpdate++;
	NEWTON_ASSERT(m_numberOfTheads >= 1);

#ifdef _WIN32
#ifndef __USE_DOUBLE_PRECISION__
	dgUnsigned32 controlWorld = dgControlFP(0xffffffff, 0);
	dgControlFP(_PC_53, _MCW_PC);
#endif
#endif

	if (m_cpu == dgSimdPresent) {
#ifdef DG_BUILD_SIMD_CODE
		simd_env rounding = simd_get_ctrl();
		simd_set_FZ_mode();

		UpdateContactsSimd(timestep, false);
		m_dynamicSolver.UpdateDynamics(this, 1, timestep);

		simd_set_ctrl(rounding);
#endif

	} else {
		UpdateContacts(timestep, false);
		m_dynamicSolver.UpdateDynamics(this, 0, timestep);
	}
	m_inUpdate--;

#ifdef _WIN32
#ifndef __USE_DOUBLE_PRECISION__
	dgControlFP(controlWorld, _MCW_PC);
#endif
#endif

	if (m_destroyBodyByExeciveForce) {
		for (dgInt32 i = 0; i < m_destroyeddBodiesPool.m_count; i++) {
			m_destroyBodyByExeciveForce(m_destroyeddBodiesPool.m_bodies[i],
			                            m_destroyeddBodiesPool.m_joint[i]);
		}
	}

	m_perfomanceCounters[m_worldTicks] = m_getPerformanceCount() - ticks;
}

OnGetPerformanceCountCallback dgWorld::GetPerformaceFuntion() const {
	return m_getPerformanceCount;
}

void dgWorld::SetPerfomanceCounter(OnGetPerformanceCountCallback callback) {
	m_threadsManager.SetPerfomanceCounter(callback);

	if (!callback) {
		callback = GetPerformanceCount;
	}
	m_getPerformanceCount = callback;
	memset(m_perfomanceCounters, 0, sizeof(m_perfomanceCounters));
}

// dgUnsigned32 dgWorld::GetPerfomanceTicks (dgInt32 thread, dgUnsigned32 entry) const
dgUnsigned32 dgWorld::GetPerfomanceTicks(dgUnsigned32 entry) const {
	entry = ClampValue(dgUnsigned32(entry), dgUnsigned32(0),
	                   dgUnsigned32(m_counterSize - 1));
	return m_perfomanceCounters[entry];
}

dgUnsigned32 dgWorld::GetThreadPerfomanceTicks(dgUnsigned32 threadIndex) const {
	return m_threadsManager.GetPerfomanceTicks(threadIndex);
}

void dgWorld::SetUserData(void *const userData) {
	m_userData = userData;
}

void *dgWorld::GetUserData() const {
	return m_userData;
}

void dgWorld::SetIslandUpdateCallback(OnIslandUpdate callback) {
	m_islandUpdate = callback;
}

void dgWorld::SetDestroyCollisionCallback(OnDestroyCollision callback) {
	m_destroyCollision = callback;
}

void dgWorld::SetLeavingWorldCallback(OnLeavingWorldAction callback) {
	m_leavingWorldNotify = callback;
}

void dgWorld::SetBodyDestructionByExeciveForce(
    OnBodyDestructionByExeciveForce callback) {
	m_destroyBodyByExeciveForce = callback;
}

dgBallConstraint *dgWorld::CreateBallConstraint(const dgVector &pivot,
        dgBody *const body0, dgBody *const body1) {
	dgBallConstraint *constraint;

	NEWTON_ASSERT(body0);
	NEWTON_ASSERT(body0 != body1);
	//  constraint = dgBallConstraint::Create(this);
	constraint = new (m_allocator) dgBallConstraint;

	AttachConstraint(constraint, body0, body1);
	constraint->SetPivotPoint(pivot);
	return constraint;
}

dgHingeConstraint *dgWorld::CreateHingeConstraint(const dgVector &pivot,
        const dgVector &pinDir, dgBody *const body0, dgBody *const body1) {
	dgHingeConstraint *constraint;

	NEWTON_ASSERT(body0);
	NEWTON_ASSERT(body0 != body1);
	//  constraint = dgHingeConstraint::Create(this);
	constraint = new (m_allocator) dgHingeConstraint;

	AttachConstraint(constraint, body0, body1);
	constraint->SetPivotAndPinDir(pivot, pinDir);
	return constraint;
}

dgUpVectorConstraint *dgWorld::CreateUpVectorConstraint(const dgVector &pin,
        dgBody *body) {
	dgUpVectorConstraint *constraint;

	NEWTON_ASSERT(body);
	//  constraint = dgUpVectorConstraint::Create(this);
	constraint = new (m_allocator) dgUpVectorConstraint;

	AttachConstraint(constraint, body, NULL);
	constraint->InitPinDir(pin);
	return constraint;
}

dgSlidingConstraint *dgWorld::CreateSlidingConstraint(const dgVector &pivot,
        const dgVector &pinDir, dgBody *const body0, dgBody *const body1) {
	dgSlidingConstraint *constraint;

	NEWTON_ASSERT(body0);
	NEWTON_ASSERT(body0 != body1);
	//  constraint = dgSlidingConstraint::Create(this);
	constraint = new (m_allocator) dgSlidingConstraint;

	AttachConstraint(constraint, body0, body1);
	constraint->SetPivotAndPinDir(pivot, pinDir);
	return constraint;
}

dgCorkscrewConstraint *dgWorld::CreateCorkscrewConstraint(const dgVector &pivot,
        const dgVector &pinDir, dgBody *const body0, dgBody *const body1) {
	dgCorkscrewConstraint *constraint;

	NEWTON_ASSERT(body0);
	NEWTON_ASSERT(body0 != body1);
	//  constraint = dgCorkscrewConstraint::Create(this);
	constraint = new (m_allocator) dgCorkscrewConstraint;

	AttachConstraint(constraint, body0, body1);
	constraint->SetPivotAndPinDir(pivot, pinDir);
	return constraint;
}

dgUniversalConstraint *dgWorld::CreateUniversalConstraint(const dgVector &pivot,
        const dgVector &pin0, const dgVector &pin1, dgBody *const body0,
        dgBody *const body1) {
	dgUniversalConstraint *constraint;

	NEWTON_ASSERT(body0);
	NEWTON_ASSERT(body0 != body1);
	//  constraint = dgUniversalConstraint::Create(this);
	constraint = new (m_allocator) dgUniversalConstraint;

	AttachConstraint(constraint, body0, body1);
	constraint->SetPivotAndPinDir(pivot, pin0, pin1);
	return constraint;
}

/*
 dgPointToCurveConstraint* dgWorld::AttachPointToCurveConstraint (
 const dgVector& pivot,
 dgBody* const body0,
 OnPointToCurveCallback funt,
 void *curveContext)
 {
 dgPointToCurveConstraint *constraint;

 NEWTON_ASSERT (body0);
 constraint = dgPointToCurveConstraint::Create();

 AttachConstraint (constraint, body0, NULL);
 constraint->SetPivotPoint (pivot, funt, curveContext);
 return constraint;
 }
 */

/*
 dgConnectorConstraint* dgWorld::CreateConnectorConstraint (dgBody *body0__, dgBody *body1__)
 {
 NEWTON_ASSERT (0);
 return NULL;

 dgConnectorConstraint *constraint;

 NEWTON_ASSERT (body0);
 NEWTON_ASSERT (body0 != body1);
 constraint = dgConnectorConstraint::Create();

 AttachConstraint (constraint, body0, body1);
 constraint->Setup();
 return constraint;
 }
 */

/*
 dgInt32 dgWorld::GetActiveBodiesCount() const
 {
 return m_activeBodiesCount;
 }
 */

dgInt32 dgWorld::GetBodiesCount() const {
	const dgBodyMasterList &list = *this;
	return list.GetCount() - 1;
}

dgInt32 dgWorld::GetConstraintsCount() const {
	const dgBodyMasterList &list = *this;
	return dgInt32(list.m_constraintCount);
}

/*
 dgLink* dgWorld::FindConstraintLink (const dgBody* const body0, const dgBody* const body1) const
 {
 NEWTON_ASSERT (0);

 dgLink *ptr;
 dgLink *link;

 NEWTON_ASSERT (body0);
 if (!body0) {
 NEWTON_ASSERT (0);
 Swap (body0, body1);
 }

 if (body0) {
 link = body0->m_firstConstraintLink;
 if (link) {
 ptr = link;
 do {
 NEWTON_ASSERT (ptr->m_body == body0);
 if (ptr->m_twin->m_body == body1) {
 return ptr;
 }

 ptr = ptr->m_twin->m_next;
 } while (ptr != link);
 }
 }

 return NULL;
 }
 */
/*
 dgConstraint* dgWorld::GetConstraint (const dgLink* constraintLink) const
 {
 return constraintLink->m_constraint;
 }
 */

void dgWorld::BodySetMatrix(dgBody *body, const dgMatrix &matrix) {
#define DG_RECURSIVE_SIZE 1024
	dgInt32 index;
	dgBody *queue[DG_RECURSIVE_SIZE];

	index = 1;
	queue[0] = body;
	m_genericLRUMark++;
	body->m_genericLRUMark = m_genericLRUMark;
	dgMatrix relMatrix(body->GetMatrix().Inverse() * matrix);
	while (index) {
		dgBody *bodyI;

		index--;
		bodyI = queue[index];
		NEWTON_ASSERT(bodyI != m_sentionelBody);

		dgBroadPhaseCollision::Remove(bodyI);
		dgBroadPhaseCollision::Add(bodyI);

		dgMatrix matrixI(bodyI->GetMatrix() * relMatrix);
		bodyI->SetVelocity(
		    dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		             dgFloat32(0.0f)));
		bodyI->SetOmega(
		    dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
		             dgFloat32(0.0f)));
		bodyI->SetMatrix(matrixI);
		bodyI->m_isInWorld = true;

		for (dgBodyMasterListRow::dgListNode *jointNode =
		            bodyI->m_masterNode->GetInfo().GetFirst();
		        jointNode; jointNode =
		            jointNode->GetNext()) {
			dgBodyMasterListCell &cell = jointNode->GetInfo();
			bodyI = cell.m_bodyNode;
			if (bodyI != m_sentionelBody) {
				if (bodyI->m_genericLRUMark != m_genericLRUMark) {
					dgConstraint *constraint;
					constraint = cell.m_joint;
					if (constraint->GetId() != dgContactConstraintId) {
						bodyI->m_genericLRUMark = m_genericLRUMark;
						queue[index] = bodyI;
						index++;
						NEWTON_ASSERT(index < DG_RECURSIVE_SIZE);
					}
				}
			}
		}
	}
}

void dgWorld::AddBodyImpulse(dgBody *body, const dgVector &pointVeloc,
                             const dgVector &pointPosit) {
	if (body->m_invMass.m_w > dgFloat32(0.0f)) {
		//      UnfreezeBody (body);
		body->AddImpulse(pointVeloc, pointPosit);
	}
}

void dgWorld::ApplyImpulseArray(dgBody *body, dgInt32 count,
                                dgInt32 strideInBytes, const dgFloat32 *const impulseArray,
                                const dgFloat32 *const pointArray) {
	if (body->m_invMass.m_w > dgFloat32(0.0f)) {
		//      UnfreezeBody (body);
		body->ApplyImpulseArray(count, strideInBytes, impulseArray, pointArray);
	}
}

/*
 dgInt32 dgWorld::GetBodyArray (dgBody* root, dgBody** array, dgInt32 maxSize) const
 {
 NEWTON_ASSERT (0);
 return 0;

 dgInt32 i;
 dgInt32 count;
 dgLink* ptr;
 dgLink* link;
 dgBody* body;
 dgConstraint* constraint;
 dgLink* stack[2048];
 const dgInt64 mask = dgInt64 (65536) * dgInt64 (65536) * dgInt64 (65536) * dgInt64 (4096);

 count = 0;
 link = root->m_firstConstraintLink;
 if (link) {
 root->m_lru |= mask;
 stack[0] = link;
 i = 1;
 while (i) {
 i --;
 link = stack[i];
 ptr = link;
 do {
 body = ptr->m_twin->m_body;
 if (body) {
 NEWTON_ASSERT (body);
 if (~body->m_lru & mask) {
 constraint = ptr->m_constraint;
 if (constraint->IsBilateral()) {
 if (count <    maxSize) {
 array[count] = body;
 count ++;
 }
 stack[i] = ptr->m_twin;
 i ++;
 }
 body->m_lru |= mask;
 }
 }

 ptr = ptr->m_twin->m_next;
 } while (ptr != link);
 }

 link = root->m_firstConstraintLink;
 root->m_lru &= ~ mask;
 stack[0] = link;
 i = 1;
 while (i) {
 i --;
 link = stack[i];
 ptr = link;
 do {
 body = ptr->m_twin->m_body;
 if (body) {
 NEWTON_ASSERT (body);
 if (body->m_lru & mask) {
 constraint = ptr->m_constraint;
 if (constraint->IsBilateral()) {
 stack[i] = ptr->m_twin;
 i ++;
 }
 body->m_lru &= ~ mask;
 }
 }
 ptr = ptr->m_twin->m_next;
 } while (ptr != link);
 }
 }

 return count;
 }


 dgInt32 dgWorld::GetConstraintArray (dgConstraint* root, dgConstraint** constraintArray, dgInt32 maxSize) const
 {
 NEWTON_ASSERT (0);
 return 0;

 dgInt32 i;
 dgInt32 stack;
 dgInt32 count;
 dgLink* ptr;
 dgLink* link;
 dgBody* body;
 dgConstraint* constraint;
 dgConstraint* constraintPtr;
 dgConstraint* stackPool[2048];
 const dgInt64 mask = dgInt64 (65536) * dgInt64 (65536) * dgInt64 (65536) * dgInt64 (4096);

 count = 0;
 stack = 1;
 stackPool[0] = root;
 root->m_lru |= mask;

 while (stack && (count < maxSize)) {
 stack --;
 constraint = stackPool[stack];
 constraintArray[count] = constraint;
 count ++;

 body = constraint->m_body0;
 if (body && (body->m_invMass.m_w != dgFloat32 (0.0f))) {
 link = body->m_firstConstraintLink;
 NEWTON_ASSERT (link);
 ptr = link;
 do {
 constraintPtr = ptr->m_constraint;
 if (!(constraintPtr->m_lru & mask)) {
 constraintPtr->m_lru |= mask;
 stackPool[stack] = constraintPtr;
 stack ++;
 }

 ptr = ptr->m_twin->m_next;
 } while (ptr != link);
 }

 body = constraint->m_body1;
 if (body && (body->m_invMass.m_w != dgFloat32 (0.0f))) {
 link = body->m_firstConstraintLink;
 NEWTON_ASSERT (link);
 ptr = link;
 do {
 constraintPtr = ptr->m_constraint;
 if (!(constraintPtr->m_lru & mask)) {
 constraintPtr->m_lru |= mask;
 stackPool[stack] = constraintPtr;
 stack ++;
 }

 ptr = ptr->m_twin->m_next;
 } while (ptr != link);
 }
 }

 for (i = 0; i < stack; i ++) {
 constraint = stackPool[i];
 constraint->m_lru &= ~mask;
 }

 for (i = 0; i < count; i ++) {
 constraint = constraintArray[i];
 constraint->m_lru &= ~mask;

 #ifdef _DEBUG
 for (stack = i + 1; stack < count; stack ++) {
 NEWTON_ASSERT (constraint != constraintArray[stack]);
 }
 #endif
 }

 return count;

 }
 */

bool dgWorld::AreBodyConnectedByJoints(dgBody *const originSrc,
                                       dgBody *const targetSrc) {
#define DG_QEUEU_SIZE 1024
	dgBody *queue[DG_QEUEU_SIZE];

	m_genericLRUMark++;

	dgBody *origin1 = originSrc;
	dgBody *target1 = targetSrc;
	if (origin1->m_mass[3] == dgFloat32(0.0f)) {
		Swap(origin1, target1);
	}

	dgBody *const origin = origin1;
	dgBody *const target = target1;

	dgInt32 end = 1;
	dgInt32 start = 0;
	queue[0] = origin;
	origin->m_genericLRUMark = m_genericLRUMark;

	while (start != end) {
		dgBody *const originI = queue[start];
		start++;
		start &= (DG_QEUEU_SIZE - 1);

		for (dgBodyMasterListRow::dgListNode *jointNode =
		            originI->m_masterNode->GetInfo().GetFirst();
		        jointNode; jointNode =
		            jointNode->GetNext()) {
			dgBodyMasterListCell &cell = jointNode->GetInfo();

			dgBody *const body = cell.m_bodyNode;
			if (body->m_genericLRUMark != m_genericLRUMark) {
				dgConstraint *const constraint = cell.m_joint;
				if (constraint->GetId() != dgContactConstraintId) {
					if (body == target) {
						return true;
					}
					body->m_genericLRUMark = m_genericLRUMark;
					queue[end] = body;
					end++;
					end &= (DG_QEUEU_SIZE - 1);
				}
			}
		}
	}

	return false;
}

void dgWorld::FlushCache() {

	// delete all contacts
	dgActiveContacts &contactList = *this;
	for (dgActiveContacts::dgListNode *contactNode = contactList.GetFirst();
	        contactNode;) {
		dgContact *contact;
		contact = contactNode->GetInfo();
		contactNode = contactNode->GetNext();
		DestroyConstraint(contact);
	}

	// clean up memory in bradPhase
	dgBroadPhaseCollision::InvalidateCache();

	// sort body list
	SortMasterList();
}
