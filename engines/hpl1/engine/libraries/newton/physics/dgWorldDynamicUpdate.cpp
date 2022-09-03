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

#include "hpl1/engine/libraries/newton/core/dg.h"

#include "dgBody.h"
#include "dgConstraint.h"
#include "dgWorld.h"
#include "dgWorldDynamicUpdate.h"


#define DG_FREEZZING_VELOCITY_DRAG dgFloat32(0.9f)
#define DG_SOLVER_MAX_ERROR (DG_FREEZE_MAG * dgFloat32(0.5f))

#ifdef DG_BUILD_SIMD_CODE
#define DG_SIMD_WORD_SIZE dgInt32(sizeof(simd_type) / sizeof(dgFloat32))
#else
#define DG_SIMD_WORD_SIZE dgInt32(sizeof(dgVector) / sizeof(dgFloat32))
#endif

#define DG_PARALLEL_JOINT_COUNT 64

// in my twist to RK4 I am no sure if the final derivative have to be weighted, it seems a mistake, I need to investigate more
// uncomment this out for more stable behavior but no exactly correct solution on the iterative solver
//#define DG_WIGHT_FINAL_RK4_DERIVATIVES

#ifdef TARGET_OS_IPHONE
#define DG_BASE_ITERATION_COUNT 2
#else
#define DG_BASE_ITERATION_COUNT 3
#endif

class dgBodyInfo {
public:
	dgBody *m_body;
};

class dgJointInfo {
public:
	dgConstraint *m_joint;
	dgInt32 m_autoPairstart;
	dgInt32 m_autoPaircount;
	dgInt32 m_autoPairActiveCount;
	dgInt32 m_m0;
	dgInt32 m_m1;
};

class dgIsland {
public:
	dgInt32 m_bodyCount;
	dgInt32 m_bodyStart;
	dgInt32 m_jointCount;
	dgInt32 m_jointStart;
	dgInt32 m_hasUnilateralJoints : 1;
	dgInt32 m_isContinueCollision : 1;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static inline dgInt32 CompareIslands(const dgIsland *const islandA,
									 const dgIsland *const islandB, void *notUsed) {
	dgInt32 countA;
	dgInt32 countB;

	countA = islandA->m_jointCount - (islandA->m_hasUnilateralJoints << 23);
	countB = islandB->m_jointCount - (islandB->m_hasUnilateralJoints << 23);

	if (countA < countB) {
		return -1;
	}
	if (countA > countB) {
		return 1;
	}
	return 0;
}

dgBody *dgWorld::GetIslandBody(const void *const islandPtr, dgInt32 index) const {
	const dgIslandCallbackStruct &island = *(dgIslandCallbackStruct *)islandPtr;

	char *ptr = &((char *)island.m_bodyArray)[island.m_strideInByte * index];
	dgBody **bodyPtr = (dgBody **)ptr;
	return (index < island.m_count) ? ((index >= 0) ? *bodyPtr : NULL) : NULL;
}

dgWorldDynamicUpdate::dgWorldDynamicUpdate() {
	m_bodies = 0;
	;
	m_joints = 0;
	m_islands = 0;
	m_markLru = 0;
	m_maxJointCount = 0;
	m_maxBodiesCount = 0;
	m_maxIslandCount = 0;
}

void dgWorldDynamicUpdate::UpdateDynamics(dgWorld *const world,
										  dgInt32 archModel, dgFloat32 timestep) {
	dgUnsigned32 updateTime = world->m_getPerformanceCount();

	m_bodies = 0;
	m_joints = 0;
	m_islands = 0;
	m_markLru = 0;
	m_world = world;
	ReallocBodyMemory(0);
	ReallocIslandMemory(0);
	ReallocJointsMemory(0);

	dgInt32 threadCounts = dgInt32(m_world->m_numberOfTheads);
	for (dgInt32 i = 0; i < threadCounts; i++) {
		m_solverMemory[i].m_world = m_world;
		m_solverMemory[i].m_threadIndex = i;
		ReallocJacobiansMemory(0, i);
		ReallocIntenalForcesMemory(0, i);
	}

	m_world->m_dynamicsLru = m_world->m_dynamicsLru + 2;
	m_markLru = dgInt32(m_world->m_dynamicsLru);
	dgInt32 lru = m_markLru - 1;

	dgBodyMasterList &me = *m_world;

	_ASSERTE(me.GetFirst()->GetInfo().GetBody() == m_world->m_sentionelBody);

	dgUnsigned32 solverMode = m_world->m_solverMode;

	m_world->m_sentionelBody->m_index = 0;
	m_world->m_sentionelBody->m_dynamicsLru = dgUnsigned32(m_markLru);

	for (dgBodyMasterList::dgListNode *node = me.GetLast(); node;
		 node = node->GetPrev()) {

		const dgBodyMasterListRow &graphNode = node->GetInfo();
		dgBody *const body = graphNode.GetBody();
		if (body->m_invMass.m_w == dgFloat32(0.0f)) {
#ifdef _DEBUG
			for (; node; node = node->GetPrev()) {
				_ASSERTE(node->GetInfo().GetBody()->m_invMass.m_w == dgFloat32(0.0f));
			}
#endif
			break;
		}

		if (dgInt32(body->m_dynamicsLru) < lru) {
			if (!(body->m_freeze | body->m_spawnnedFromCallback | body->m_sleeping | !body->m_isInWorld)) {
				SpanningTree(body);
			}
		}
		body->m_spawnnedFromCallback = false;
	}

	dgSort(m_islandArray, m_islands, CompareIslands);
	//	dgRadixSort (m_islandArray, &m_islandArray[m_islands], m_islands, 3, GetIslandsKey);

	dgUnsigned32 dynamicsTime = m_world->m_getPerformanceCount();
	m_world->m_perfomanceCounters[m_dynamicsBuildSpanningTreeTicks] = dynamicsTime - updateTime;

	if (threadCounts > 1) {
		dgInt32 chunkSizes[DG_MAXIMUN_THREADS];

		if (m_world->m_singleIslandMultithreading) {
			const dgJacobianMemory &system = m_solverMemory[0];
			while (m_islands && (m_islandArray[m_islands - 1].m_jointCount >= DG_PARALLEL_JOINT_COUNT)) {
				m_islands--;

				_ASSERTE(!m_islandArray[m_islands].m_isContinueCollision);
				BuildJacobianMatrixParallel(m_islandArray[m_islands], timestep, archModel);
				system.CalculateReactionsForcesParallel(dgInt32(solverMode), DG_SOLVER_MAX_ERROR, archModel);
				IntegrateArray(&system.m_bodyArray[1], system.m_bodyCount - 1, DG_SOLVER_MAX_ERROR, timestep, 0, true);
			}
		}

		if (m_islands) {
			m_world->m_threadsManager.CalculateChunkSizes(m_islands, chunkSizes);
			for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
				m_workerThreads[threadIndex].m_useSimd = archModel;
				m_workerThreads[threadIndex].m_world = m_world;
				m_workerThreads[threadIndex].m_dynamics = this;
				m_workerThreads[threadIndex].m_count = chunkSizes[threadIndex] * threadCounts;
				m_workerThreads[threadIndex].m_threads = threadCounts;
				m_workerThreads[threadIndex].m_timestep = timestep;
				m_workerThreads[threadIndex].m_solverMode = dgInt32(solverMode);
				m_workerThreads[threadIndex].m_threadIndex = threadIndex;
				m_workerThreads[threadIndex].m_system = &m_solverMemory[threadIndex];
				m_world->m_threadsManager.SubmitJob(&m_workerThreads[threadIndex]);
			}
			m_world->m_threadsManager.SynchronizationBarrier();
		}

	} else {
		m_workerThreads[0].m_threads = 1;
		m_workerThreads[0].m_useSimd = archModel;
		m_workerThreads[0].m_count = m_islands;
		m_workerThreads[0].m_world = m_world;
		m_workerThreads[0].m_dynamics = this;
		m_workerThreads[0].m_threadIndex = 0;
		m_workerThreads[0].m_solverMode = dgInt32(solverMode);
		m_workerThreads[0].m_timestep = timestep;
		m_workerThreads[0].m_system = &m_solverMemory[0];
		m_workerThreads[0].ThreadExecute();
	}

	dgUnsigned32 ticks = m_world->m_getPerformanceCount();
	m_world->m_perfomanceCounters[m_dynamicsSolveSpanningTreeTicks] = ticks - dynamicsTime;
	m_world->m_perfomanceCounters[m_dynamicsTicks] = ticks - updateTime;
}

void dgSolverWorlkerThreads::ThreadExecute() {
	const dgIsland *const m_islandArray = m_dynamics->m_islandArray;
	dgContactPoint *const contactBuffer =
		(dgContactPoint *)m_world->m_contactBuffers[m_threadIndex];

	dgInt32 step = m_threads;
	dgInt32 count = m_count;
	if (m_useSimd) {
		for (dgInt32 i = 0; i < count; i += step) {
			const dgIsland &island = m_islandArray[i + m_threadIndex];
			if (!island.m_isContinueCollision) {
				m_dynamics->BuildJacobianMatrixSimd(island, m_threadIndex, m_timestep);
				m_system->CalculateReactionsForcesSimd(m_solverMode,
													   DG_SOLVER_MAX_ERROR);
				m_dynamics->IntegrateArray(&m_system->m_bodyArray[1],
										   m_system->m_bodyCount - 1, DG_SOLVER_MAX_ERROR, m_timestep,
										   m_threadIndex, true);
			} else {
				dgBodyInfo *const bodyArray =
					&m_dynamics->m_bodyArray[island.m_bodyStart];
				dgJointInfo *const constraintArray =
					&m_dynamics->m_constraintArray[island.m_jointStart];

				dgFloat32 dist = dgFloat32(0.0f);
				dgFloat32 maxSpeed2 = dgFloat32(0.0f);
				for (dgInt32 k = 1; k < island.m_bodyCount; k++) {
					dgVector veloc;
					dgVector omega;

					dgBody *const body = bodyArray[k].m_body;
					_ASSERTE(body->m_mass.m_w > dgFloat32(0.0f));
					body->CalculateContinueVelocity(m_timestep, veloc, omega);

					dgFloat32 mag2 = veloc % veloc;
					if (mag2 > maxSpeed2) {
						maxSpeed2 = mag2;
						dist = body->m_collision->GetBoxMinRadius();
					}
				}

				_ASSERTE(dist > dgFloat32(0.0f));
				dgInt32 steps = dgInt32(
									dgFloat32(4.0f) * dgSqrt(maxSpeed2) * m_timestep / dist) +
								1;
				dgFloat32 timestep = m_timestep / dgFloat32(steps);

				if (steps > 8) {
					steps = 8;
				}

				for (dgInt32 j = 0; j < steps - 1; j++) {
					m_dynamics->BuildJacobianMatrixSimd(island, m_threadIndex, timestep);
					m_system->CalculateReactionsForcesSimd(m_solverMode,
														   DG_SOLVER_MAX_ERROR);
					m_dynamics->IntegrateArray(&m_system->m_bodyArray[1],
											   m_system->m_bodyCount - 1, DG_SOLVER_MAX_ERROR, timestep,
											   m_threadIndex, false);

					for (dgInt32 k = 1; k < island.m_bodyCount; k++) {
						dgBody *const body = bodyArray[k].m_body;
						body->m_accel = body->m_prevExternalForce;
						body->m_alpha = body->m_prevExternalTorque;
					}

					for (dgInt32 k = 0; k < island.m_jointCount; k++) {
						dgConstraint *const constraint = constraintArray[k].m_joint;
						if (constraint->GetId() == dgContactConstraintId) {
							dgContact *const contact = (dgContact *)constraint;
							const dgContactMaterial *const material =
								contact->m_myCacheMaterial;
							if (material->m_flags & dgContactMaterial::m_collisionEnable__) {
								if (material->m_aabbOverlap) {
									_ASSERTE(contact->m_body0);
									_ASSERTE(contact->m_body1);
									material->m_aabbOverlap(*material, *contact->m_body0,
															*contact->m_body1, m_threadIndex);
								}
								dgCollidingPairCollector::dgPair pair;
								pair.m_body0 = contact->m_body0;
								pair.m_body1 = contact->m_body1;
								pair.m_contact = contact;
								pair.m_material = material;
								pair.m_contactBuffer = contactBuffer;

								m_world->CalculateContactsSimd(&pair, timestep, m_threadIndex);
								//_ASSERTE (contact->m_maxDOF == 0);

								_ASSERTE(pair.m_contact);
								if (pair.m_contactCount) {
									_ASSERTE(pair.m_contactCount <= (DG_CONSTRAINT_MAX_ROWS / 3));
									m_world->ProcessContacts(&pair, timestep, m_threadIndex);
								} else if (!pair.m_contactBuffer) {
									m_world->ProcessCachedContacts(pair.m_contact,
																   pair.m_material, timestep, m_threadIndex);
								}
							}
						}
					}
				}
				m_dynamics->BuildJacobianMatrixSimd(island, m_threadIndex, timestep);
				m_system->CalculateReactionsForcesSimd(m_solverMode,
													   DG_SOLVER_MAX_ERROR);
				m_dynamics->IntegrateArray(&m_system->m_bodyArray[1],
										   m_system->m_bodyCount - 1, DG_SOLVER_MAX_ERROR, timestep,
										   m_threadIndex, true);
			}
		}
	} else {
		for (dgInt32 i = 0; i < count; i += step) {
			const dgIsland &island = m_islandArray[i + m_threadIndex];

			if (!island.m_isContinueCollision) {
				m_dynamics->BuildJacobianMatrix(island, m_threadIndex, m_timestep);
				m_system->CalculateReactionsForces(m_solverMode, DG_SOLVER_MAX_ERROR);
				m_dynamics->IntegrateArray(&m_system->m_bodyArray[1],
										   m_system->m_bodyCount - 1, DG_SOLVER_MAX_ERROR, m_timestep,
										   m_threadIndex, true);
			} else {
				dgBodyInfo *const bodyArray =
					&m_dynamics->m_bodyArray[island.m_bodyStart];
				dgJointInfo *const constraintArray =
					&m_dynamics->m_constraintArray[island.m_jointStart];

				dgFloat32 dist = dgFloat32(0.0f);
				dgFloat32 maxSpeed2 = dgFloat32(0.0f);
				for (dgInt32 k = 1; k < island.m_bodyCount; k++) {
					dgVector veloc;
					dgVector omega;

					dgBody *const body = bodyArray[k].m_body;
					_ASSERTE(body->m_mass.m_w > dgFloat32(0.0f));
					body->CalculateContinueVelocity(m_timestep, veloc, omega);

					dgFloat32 mag2 = veloc % veloc;
					if (mag2 > maxSpeed2) {
						maxSpeed2 = mag2;
						dist = body->m_collision->GetBoxMinRadius();
					}
				}

				_ASSERTE(dist > dgFloat32(0.0f));
				dgInt32 steps = dgInt32(
									dgFloat32(4.0f) * dgSqrt(maxSpeed2) * m_timestep / dist) +
								1;
				dgFloat32 timestep = m_timestep / dgFloat32(steps);

				if (steps > 8) {
					steps = 8;
				}

				for (dgInt32 j = 0; j < steps - 1; j++) {
					m_dynamics->BuildJacobianMatrix(island, m_threadIndex, timestep);
					m_system->CalculateReactionsForces(m_solverMode, DG_SOLVER_MAX_ERROR);
					m_dynamics->IntegrateArray(&m_system->m_bodyArray[1],
											   m_system->m_bodyCount - 1, DG_SOLVER_MAX_ERROR, timestep,
											   m_threadIndex, false);

					for (dgInt32 k = 1; k < island.m_bodyCount; k++) {
						dgBody *const body = bodyArray[k].m_body;
						body->m_accel = body->m_prevExternalForce;
						body->m_alpha = body->m_prevExternalTorque;
					}

					for (dgInt32 k = 0; k < island.m_jointCount; k++) {
						dgConstraint *const constraint = constraintArray[k].m_joint;
						if (constraint->GetId() == dgContactConstraintId) {
							dgContact *const contact = (dgContact *)constraint;
							const dgContactMaterial *const material =
								contact->m_myCacheMaterial;
							if (material->m_flags & dgContactMaterial::m_collisionEnable__) {
								if (material->m_aabbOverlap) {
									_ASSERTE(contact->m_body0);
									_ASSERTE(contact->m_body1);
									material->m_aabbOverlap(*material, *contact->m_body0,
															*contact->m_body1, m_threadIndex);
								}
								dgCollidingPairCollector::dgPair pair;
								pair.m_body0 = contact->m_body0;
								pair.m_body1 = contact->m_body1;
								pair.m_contact = contact;
								pair.m_material = material;
								pair.m_contactBuffer = contactBuffer;
								m_world->CalculateContacts(&pair, timestep, m_threadIndex);

								_ASSERTE(pair.m_contact);

								if (pair.m_contactCount) {
									_ASSERTE(pair.m_contactCount <= (DG_CONSTRAINT_MAX_ROWS / 3));
									m_world->ProcessContacts(&pair, timestep, m_threadIndex);
								} else if (!pair.m_contactBuffer) {
									m_world->ProcessCachedContacts(pair.m_contact,
																   pair.m_material, timestep, m_threadIndex);
								}
							}
						}
					}
				}
				m_dynamics->BuildJacobianMatrix(island, m_threadIndex, timestep);
				m_system->CalculateReactionsForces(m_solverMode, DG_SOLVER_MAX_ERROR);
				m_dynamics->IntegrateArray(&m_system->m_bodyArray[1],
										   m_system->m_bodyCount - 1, DG_SOLVER_MAX_ERROR, timestep,
										   m_threadIndex, true);
			}
		}
	}
}

void dgWorldDynamicUpdate::ReallocBodyMemory(dgInt32 bodyCount) {
	void *memory;
	dgInt32 stride;
	dgInt32 newCount;
	dgBodyInfo *bodyArray;

	stride = sizeof(dgBodyInfo);
	if (bodyCount) {
		m_world->m_bodiesMemorySizeInBytes = m_world->m_bodiesMemorySizeInBytes * 2;
		memory = m_world->GetAllocator()->MallocLow(
			m_world->m_bodiesMemorySizeInBytes);

		newCount = (m_world->m_bodiesMemorySizeInBytes / stride) & (-4);
		bodyArray = (dgBodyInfo *)memory;

		memcpy(bodyArray, m_bodyArray, bodyCount * sizeof(dgBodyInfo));
		m_world->GetAllocator()->FreeLow(m_world->m_bodiesMemory);

	} else {
		memory = m_world->m_bodiesMemory;
		newCount = (m_world->m_bodiesMemorySizeInBytes / stride) & (-4);
		bodyArray = (dgBodyInfo *)memory;
	}

	m_maxBodiesCount = newCount;
	m_world->m_bodiesMemory = memory;
	m_bodyArray = bodyArray;
	_ASSERTE((dgUnsigned64(m_bodyArray) & 0x0f) == 0);
}

void dgWorldDynamicUpdate::ReallocIslandMemory(dgInt32 islandCount) {
	void *memory;
	dgInt32 stride;
	dgInt32 newCount;
	dgIsland *islandArray;

	stride = sizeof(dgIsland);
	if (islandCount) {
		m_world->m_islandMemorySizeInBytes = m_world->m_islandMemorySizeInBytes * 2;
		memory = m_world->GetAllocator()->MallocLow(
			m_world->m_islandMemorySizeInBytes);

		newCount = (m_world->m_islandMemorySizeInBytes / stride) & (-4);

		islandArray = (dgIsland *)memory;

		memcpy(islandArray, m_islandArray, islandCount * sizeof(dgIsland));
		m_world->GetAllocator()->FreeLow(m_world->m_islandMemory);

	} else {
		memory = m_world->m_islandMemory;
		newCount = (m_world->m_islandMemorySizeInBytes / stride) & (-4);
		islandArray = (dgIsland *)memory;
	}

	m_maxIslandCount = newCount;
	m_world->m_islandMemory = memory;

	m_islandArray = islandArray;
	_ASSERTE((dgUnsigned64(m_islandArray) & 0x0f) == 0);
}

void dgWorldDynamicUpdate::ReallocJointsMemory(dgInt32 count) {
	void *memory;
	dgInt32 stride;
	dgInt32 newCount;
	dgJointInfo *constraintArray;

	stride = sizeof(dgJointInfo);

	if (count) {
		m_world->m_jointsMemorySizeInBytes = m_world->m_jointsMemorySizeInBytes * 2;
		memory = m_world->GetAllocator()->MallocLow(
			m_world->m_jointsMemorySizeInBytes);
		newCount = (m_world->m_jointsMemorySizeInBytes / stride) & (-4);

		constraintArray = (dgJointInfo *)memory;

		memcpy(constraintArray, m_constraintArray, count * sizeof(dgJointInfo));

		m_world->GetAllocator()->FreeLow(m_world->m_jointsMemory);
	} else {
		memory = m_world->m_jointsMemory;
		newCount = (m_world->m_jointsMemorySizeInBytes / stride) & (-4);

		constraintArray = (dgJointInfo *)memory;
	}

	m_maxJointCount = newCount;
	m_world->m_jointsMemory = memory;

	m_constraintArray = constraintArray;
	_ASSERTE((dgUnsigned64(m_constraintArray) & 0x0f) == 0);
}

void dgWorldDynamicUpdate::ReallocJacobiansMemory(dgInt32 count,
												  dgInt32 threadIndex) {
	void *memory;
	dgInt32 stride;
	dgInt32 newCount;
	dgJacobianPair *Jt;
	dgJacobianPair *JMinv;
	dgFloat32 *force;
	dgFloat32 *accel;
	dgFloat32 *deltaAccel;
	dgFloat32 *deltaForce;
	dgFloat32 *diagDamp;
	dgFloat32 *invDJMinvJt;
	dgFloat32 *restitution;
	dgFloat32 *penetration;
	dgFloat32 *coordenateAccel;
	dgFloat32 *penetrationStiffness;
	dgFloat32 *lowerBoundFrictionCoefficent;
	dgFloat32 *upperBoundFrictionCoefficent;
	dgFloat32 **forceFeedback;
	dgInt32 *normalForceIndex;
	dgInt32 *accelIsMotor;

	stride = sizeof(dgJacobianPair) + // Jt
			 sizeof(dgJacobianPair) + // JtMinv
			 sizeof(dgFloat32) +      // lowerBoundFrictionCoefficent
			 sizeof(dgFloat32) +      // upperBoundFrictionCoefficent
			 sizeof(dgFloat32 *) +    // forceFeedback
			 sizeof(dgFloat32) +      // force
			 sizeof(dgFloat32) +      // accel
			 sizeof(dgFloat32) +      // deltaAccel
			 sizeof(dgFloat32) +      // deltaForce
			 sizeof(dgFloat32) +      // diagDamp
			 sizeof(dgFloat32) +      // invDJMinvJt
			 sizeof(dgFloat32) +      // penetration
			 sizeof(dgFloat32) +      // restitution
			 sizeof(dgFloat32) +      // coordinateAccel
			 sizeof(dgFloat32) +      // penetrationStiffness
			 sizeof(dgInt32) +        // normalForceIndex
			 sizeof(dgInt32) +        // accelIsMotor;
			 0;

	if (count) {
		m_world->m_jacobiansMemorySizeInBytes[threadIndex] =
			m_world->m_jacobiansMemorySizeInBytes[threadIndex] * 2;
		memory = m_world->GetAllocator()->MallocLow(
			m_world->m_jacobiansMemorySizeInBytes[threadIndex] + 64);
		newCount = ((m_world->m_jacobiansMemorySizeInBytes[threadIndex] - 16) / stride) & (-8);

		Jt = (dgJacobianPair *)memory;
		JMinv = (dgJacobianPair *)&Jt[newCount];
		force = (dgFloat32 *)&JMinv[newCount];
		force += 4;
		accel = (dgFloat32 *)&force[newCount];
		;
		deltaAccel = (dgFloat32 *)&accel[newCount];
		deltaForce = (dgFloat32 *)&deltaAccel[newCount];
		// forceStep = (dgFloat32*) &deltaForce [newCount];
		diagDamp = (dgFloat32 *)&deltaForce[newCount];
		coordenateAccel = (dgFloat32 *)&diagDamp[newCount];
		restitution = (dgFloat32 *)&coordenateAccel[newCount];
		penetration = (dgFloat32 *)&restitution[newCount];
		penetrationStiffness = (dgFloat32 *)&penetration[newCount];
		invDJMinvJt = (dgFloat32 *)&penetrationStiffness[newCount];
		lowerBoundFrictionCoefficent = (dgFloat32 *)&invDJMinvJt[newCount];
		upperBoundFrictionCoefficent =
			(dgFloat32 *)&lowerBoundFrictionCoefficent[newCount];
		forceFeedback = (dgFloat32 **)&upperBoundFrictionCoefficent[newCount];
		normalForceIndex = (dgInt32 *)&forceFeedback[newCount];
		accelIsMotor = (dgInt32 *)&normalForceIndex[newCount];

		m_world->GetAllocator()->FreeLow(m_world->m_jacobiansMemory[threadIndex]);

	} else {
		memory = m_world->m_jacobiansMemory[threadIndex];
		newCount = ((m_world->m_jacobiansMemorySizeInBytes[threadIndex] - 16) / stride) & (-8);

		Jt = (dgJacobianPair *)memory;
		JMinv = (dgJacobianPair *)&Jt[newCount];
		force = (dgFloat32 *)&JMinv[newCount];
		force += 4;
		accel = (dgFloat32 *)&force[newCount];
		;
		deltaAccel = (dgFloat32 *)&accel[newCount];
		deltaForce = (dgFloat32 *)&deltaAccel[newCount];
		diagDamp = (dgFloat32 *)&deltaForce[newCount];
		coordenateAccel = (dgFloat32 *)&diagDamp[newCount];
		restitution = (dgFloat32 *)&coordenateAccel[newCount];
		penetration = (dgFloat32 *)&restitution[newCount];
		penetrationStiffness = (dgFloat32 *)&penetration[newCount];
		invDJMinvJt = (dgFloat32 *)&penetrationStiffness[newCount];
		lowerBoundFrictionCoefficent = (dgFloat32 *)&invDJMinvJt[newCount];
		upperBoundFrictionCoefficent =
			(dgFloat32 *)&lowerBoundFrictionCoefficent[newCount];
		forceFeedback = (dgFloat32 **)&upperBoundFrictionCoefficent[newCount];
		normalForceIndex = (dgInt32 *)&forceFeedback[newCount];
		accelIsMotor = (dgInt32 *)&normalForceIndex[newCount];
	}

	m_solverMemory[threadIndex].m_maxJacobiansCount = newCount;
	m_world->m_jacobiansMemory[threadIndex] = memory;

	force[-1] = dgFloat32(1.0f);
	force[-2] = dgFloat32(1.0f);
	force[-3] = dgFloat32(1.0f);
	force[-4] = dgFloat32(1.0f);

	m_solverMemory[threadIndex].m_Jt = Jt;
	m_solverMemory[threadIndex].m_JMinv = JMinv;
	m_solverMemory[threadIndex].m_force = force;
	m_solverMemory[threadIndex].m_accel = accel;
	m_solverMemory[threadIndex].m_deltaForce = deltaForce;
	m_solverMemory[threadIndex].m_deltaAccel = deltaAccel;
	m_solverMemory[threadIndex].m_diagDamp = diagDamp;
	m_solverMemory[threadIndex].m_invDJMinvJt = invDJMinvJt;
	m_solverMemory[threadIndex].m_penetration = penetration;
	m_solverMemory[threadIndex].m_restitution = restitution;
	m_solverMemory[threadIndex].m_coordenateAccel = coordenateAccel;
	m_solverMemory[threadIndex].m_penetrationStiffness = penetrationStiffness;
	m_solverMemory[threadIndex].m_lowerBoundFrictionCoefficent =
		lowerBoundFrictionCoefficent;
	m_solverMemory[threadIndex].m_upperBoundFrictionCoefficent =
		upperBoundFrictionCoefficent;
	m_solverMemory[threadIndex].m_jointFeebackForce = forceFeedback;
	m_solverMemory[threadIndex].m_normalForceIndex = normalForceIndex;
	m_solverMemory[threadIndex].m_accelIsMotor = accelIsMotor;

	_ASSERTE((dgUnsigned64(Jt) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(JMinv) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(force) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(accel) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(deltaAccel) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(deltaForce) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(diagDamp) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(penetration) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(restitution) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(invDJMinvJt) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(coordenateAccel) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(penetrationStiffness) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(lowerBoundFrictionCoefficent) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(upperBoundFrictionCoefficent) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(forceFeedback) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(normalForceIndex) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(accelIsMotor) & 0x0f) == 0);
}

void dgWorldDynamicUpdate::ReallocIntenalForcesMemory(dgInt32 count,
													  dgInt32 threadIndex) {
	dgInt32 stride;
	dgInt32 newCount;
	void *memory;
	dgInt32 *treadLocks;
	dgJacobian *internalForces;
	dgJacobian *internalVeloc;

	stride = sizeof(dgJacobian) + sizeof(dgJacobian) + sizeof(dgInt32) + 0;

	if (count) {
		m_world->m_internalForcesMemorySizeInBytes[threadIndex] =
			m_world->m_internalForcesMemorySizeInBytes[threadIndex] * 2;
		memory = m_world->GetAllocator()->MallocLow(
			m_world->m_internalForcesMemorySizeInBytes[threadIndex]);
		//		newCount = m_world->m_internalForcesMemorySizeInBytes[threadIndex] / stride;
		newCount = ((m_world->m_internalForcesMemorySizeInBytes[threadIndex] - 16) / stride) & (-8);

		internalForces = (dgJacobian *)memory;
		internalVeloc = &internalForces[newCount];
		treadLocks = (dgInt32 *)&internalVeloc[newCount];

		m_world->GetAllocator()->FreeLow(
			m_world->m_internalForcesMemory[threadIndex]);
	} else {
		memory = m_world->m_internalForcesMemory[threadIndex];
		newCount = ((m_world->m_internalForcesMemorySizeInBytes[threadIndex] - 16) / stride) & (-8);

		internalForces = (dgJacobian *)memory;
		internalVeloc = &internalForces[newCount];
		treadLocks = (dgInt32 *)&internalVeloc[newCount];
	}

	_ASSERTE(
		(((dgInt8 *)&treadLocks[newCount - 1]) - ((dgInt8 *)memory)) < m_world->m_internalForcesMemorySizeInBytes[threadIndex]);

	m_solverMemory[threadIndex].m_maxBodiesCount = newCount;
	m_world->m_internalForcesMemory[threadIndex] = memory;

	m_solverMemory[threadIndex].m_internalForces = internalForces;
	m_solverMemory[threadIndex].m_internalVeloc = internalVeloc;
	m_solverMemory[threadIndex].m_treadLocks = treadLocks;
	memset(treadLocks, 0, newCount * sizeof(dgInt32));
	// m_solverMemory[threadIndex].m_bodyFreeze = bodyFreeze;

	_ASSERTE((dgUnsigned64(internalForces) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(internalVeloc) & 0x0f) == 0);
}

void dgWorldDynamicUpdate::BuildIsland(dgQueue<dgBody *> &queue,
									   dgInt32 jointCount, dgInt32 hasUnilateralJoints,
									   dgInt32 isContinueCollisionIsland) {
	dgInt32 bodyCount = 1;
	dgInt32 lruMark = m_markLru;

	if (m_bodies >= m_maxBodiesCount) {
		ReallocBodyMemory(m_bodies);
	}

	m_bodyArray[m_bodies].m_body = m_world->m_sentionelBody;
	_ASSERTE(m_world->m_sentionelBody->m_index == 0);
	_ASSERTE(dgInt32(m_world->m_sentionelBody->m_dynamicsLru) == m_markLru);
	while (!queue.IsEmpty()) {
		dgInt32 count = queue.m_firstIndex - queue.m_lastIndex;
		if (count < 0) {
			_ASSERTE(0);
			count += queue.m_mod;
		}

		dgInt32 index = queue.m_lastIndex;
		queue.Reset();

		for (dgInt32 j = 0; j < count; j++) {
			dgBody *const body = queue.m_pool[index];
			_ASSERTE(body);
			_ASSERTE(dgInt32(body->m_dynamicsLru) == lruMark);
			_ASSERTE(body->m_masterNode);

			if (body->m_invMass.m_w > dgFloat32(0.0f)) {
				dgInt32 bodyIndex = m_bodies + bodyCount;
				if (bodyIndex >= m_maxBodiesCount) {
					ReallocBodyMemory(bodyIndex);
				}
				body->m_index = bodyCount;
				m_bodyArray[bodyIndex].m_body = body;

				bodyCount++;
			}
			for (dgBodyMasterListRow::dgListNode *jointNode =
					 body->m_masterNode->GetInfo().GetFirst();
				 jointNode; jointNode =
								jointNode->GetNext()) {
				dgBodyMasterListCell &cell = jointNode->GetInfo();

				dgBody *const body = cell.m_bodyNode;
				dgConstraint *const constraint = cell.m_joint;
				if (dgInt32(constraint->m_dynamicsLru) != lruMark) {
					constraint->m_dynamicsLru = dgUnsigned32(lruMark);

					dgInt32 jointIndex = m_joints + jointCount;
					if (jointIndex >= m_maxJointCount) {
						ReallocJointsMemory(jointIndex);
					}

					if (constraint->m_isUnilateral) {
						hasUnilateralJoints = 1;
						_ASSERTE(
							(constraint->m_body0 == m_world->m_sentionelBody) || (constraint->m_body1 == m_world->m_sentionelBody));
					}

					constraint->m_index = dgUnsigned32(jointCount);
					m_constraintArray[jointIndex].m_joint = constraint;

					jointCount++;

					_ASSERTE(constraint->m_body0);
					_ASSERTE(constraint->m_body1);
				}

				if (dgInt32(body->m_dynamicsLru) != lruMark) {
					if (body->m_invMass.m_w > dgFloat32(0.0f)) {
						queue.Insert(body);
						body->m_dynamicsLru = dgUnsigned32(lruMark);
					}
				}
			}

			index++;
			if (index >= queue.m_mod) {
				_ASSERTE(0);
				index = 0;
			}
		}
	}

	if (bodyCount > 1) {
		if (m_islands >= m_maxIslandCount) {
			ReallocIslandMemory(m_islands);
		}

		m_islandArray[m_islands].m_bodyStart = m_bodies;
		m_islandArray[m_islands].m_jointStart = m_joints;
		m_islandArray[m_islands].m_bodyCount = bodyCount;
		m_islandArray[m_islands].m_jointCount = jointCount;
		m_islandArray[m_islands].m_hasUnilateralJoints = hasUnilateralJoints;
		m_islandArray[m_islands].m_isContinueCollision = isContinueCollisionIsland;
		m_islands++;
		m_bodies += bodyCount;
		m_joints += jointCount;
	}
}

void dgWorldDynamicUpdate::SpanningTree(dgBody *const body) {
	dgInt32 bodyCount = 0;
	dgInt32 jointCount = 0;
	dgInt32 staticCount = 0;
	dgInt32 lruMark = m_markLru - 1;
	dgInt32 isInWorld = 0;
	dgInt32 isInEquilibrium = 1;
	dgInt32 hasUnilateralJoints = 0;
	dgInt32 isContinueCollisionIsland = 0;
	dgBody *heaviestBody = NULL;
	dgFloat32 haviestMass = dgFloat32(0.0f);

	dgQueue<dgBody *> queue(
		(dgBody **)m_world->m_pairMemoryBuffer,
		(dgInt32(m_world->m_pairMemoryBufferSizeInBytes >> 1)) / dgInt32(sizeof(void *)));
	dgBody **const staticPool = &queue.m_pool[queue.m_mod];

	body->m_dynamicsLru = dgUnsigned32(lruMark);

	queue.Insert(body);
	while (!queue.IsEmpty()) {
		dgInt32 count = queue.m_firstIndex - queue.m_lastIndex;
		if (count < 0) {
			_ASSERTE(0);
			count += queue.m_mod;
		}

		dgInt32 index = queue.m_lastIndex;
		queue.Reset();
		for (dgInt32 j = 0; j < count; j++) {

			dgBody *const srcBody = queue.m_pool[index];
			_ASSERTE(srcBody);
			_ASSERTE(srcBody->m_invMass.m_w > dgFloat32(0.0f));
			_ASSERTE(dgInt32(srcBody->m_dynamicsLru) == lruMark);
			_ASSERTE(srcBody->m_masterNode);

			dgInt32 bodyIndex = m_bodies + bodyCount;
			if (bodyIndex >= m_maxBodiesCount) {
				ReallocBodyMemory(bodyIndex);
			}
			m_bodyArray[bodyIndex].m_body = srcBody;

			isInWorld |= srcBody->m_isInWorld;
			isInEquilibrium &= srcBody->m_equilibrium;
			isInEquilibrium &= srcBody->m_autoSleep;

			// isInEquilibrium = 0;

			srcBody->m_sleeping = false;
			isContinueCollisionIsland |= srcBody->m_solverInContinueCollision;

			if (srcBody->m_mass.m_w > haviestMass) {
				haviestMass = srcBody->m_mass.m_w;
				heaviestBody = srcBody;
			}

			bodyCount++;
			for (dgBodyMasterListRow::dgListNode *jointNode =
					 srcBody->m_masterNode->GetInfo().GetFirst();
				 jointNode; jointNode =
								jointNode->GetNext()) {
				dgBodyMasterListCell &cell = jointNode->GetInfo();

				dgBody *const body = cell.m_bodyNode;
				if (dgInt32(body->m_dynamicsLru) < lruMark) {
					_ASSERTE(body == cell.m_bodyNode);
					body->m_dynamicsLru = dgUnsigned32(lruMark);

					if (body->m_invMass.m_w > dgFloat32(0.0f)) {
						queue.Insert(body);
					} else {

						dgInt32 duplicateBody = 0;
						for (; duplicateBody < staticCount; duplicateBody++) {
							if (staticPool[duplicateBody] == srcBody) {
								break;
							}
						}
						if (duplicateBody == staticCount) {
							staticPool[staticCount] = srcBody;
							staticCount++;
							_ASSERTE(srcBody->m_invMass.m_w > dgFloat32(0.0f));
						}

						dgConstraint *const constraint = cell.m_joint;
						_ASSERTE(dgInt32(constraint->m_dynamicsLru) != m_markLru);

						dgInt32 jointIndex = m_joints + jointCount;
						if (jointIndex >= m_maxJointCount) {
							ReallocJointsMemory(jointIndex);
						}

						if (constraint->m_isUnilateral) {
							hasUnilateralJoints = 1;
							_ASSERTE(
								(constraint->m_body0 == m_world->m_sentionelBody) || (constraint->m_body1 == m_world->m_sentionelBody));
						}

						constraint->m_index = dgUnsigned32(jointCount);
						m_constraintArray[jointIndex].m_joint = constraint;

						jointCount++;

						_ASSERTE(constraint->m_body0);
						_ASSERTE(constraint->m_body1);
					}
				} else if (cell.m_bodyNode->m_invMass.m_w == dgFloat32(0.0f)) {
					dgInt32 duplicateBody = 0;
					for (; duplicateBody < staticCount; duplicateBody++) {
						if (staticPool[duplicateBody] == srcBody) {
							break;
						}
					}
					if (duplicateBody == staticCount) {
						staticPool[staticCount] = srcBody;
						staticCount++;
						_ASSERTE(srcBody->m_invMass.m_w > dgFloat32(0.0f));
					}

					dgConstraint *const constraint = cell.m_joint;
					_ASSERTE(dgInt32(constraint->m_dynamicsLru) != m_markLru);

					dgInt32 jointIndex = m_joints + jointCount;
					if (jointIndex >= m_maxJointCount) {
						ReallocJointsMemory(jointIndex);
					}

					if (constraint->m_isUnilateral) {
						hasUnilateralJoints = 1;
						_ASSERTE(
							(constraint->m_body0 == m_world->m_sentionelBody) || (constraint->m_body1 == m_world->m_sentionelBody));
					}

					constraint->m_index = dgUnsigned32(jointCount);
					m_constraintArray[jointIndex].m_joint = constraint;

					jointCount++;

					_ASSERTE(constraint->m_body0);
					_ASSERTE(constraint->m_body1);
				}
			}

			index++;
			if (index >= queue.m_mod) {
				_ASSERTE(0);
				index = 0;
			}
		}
	}

	if (isInEquilibrium | !isInWorld) {
		for (dgInt32 i = 0; i < bodyCount; i++) {
			dgBody *const body = m_bodyArray[m_bodies + i].m_body;
			body->m_dynamicsLru = dgUnsigned32(m_markLru);
			body->m_sleeping = true;
		}
	} else {
		if (m_world->m_islandUpdate) {
			dgIslandCallbackStruct record;
			record.m_world = m_world;
			record.m_count = bodyCount;
			record.m_strideInByte = sizeof(dgBodyInfo);
			record.m_bodyArray = &m_bodyArray[m_bodies].m_body;
			if (!m_world->m_islandUpdate(m_world, &record, bodyCount)) {
				for (dgInt32 i = 0; i < bodyCount; i++) {
					dgBody *const body = m_bodyArray[m_bodies + i].m_body;
					body->m_dynamicsLru = dgUnsigned32(m_markLru);
				}
				return;
			}
		}

		if (staticCount) {
			queue.Reset();
			for (dgInt32 i = 0; i < staticCount; i++) {
				dgBody *const body = staticPool[i];
				body->m_dynamicsLru = dgUnsigned32(m_markLru);
				queue.Insert(body);
				_ASSERTE(dgInt32(body->m_dynamicsLru) == m_markLru);
			}

			for (dgInt32 i = 0; i < jointCount; i++) {
				dgConstraint *const constraint = m_constraintArray[m_joints + i].m_joint;
				constraint->m_dynamicsLru = dgUnsigned32(m_markLru);
			}
		} else {
			_ASSERTE(heaviestBody);
			queue.Insert(heaviestBody);
			// body->m_dynamicsLru = m_markLru;
			heaviestBody->m_dynamicsLru = dgUnsigned32(m_markLru);
		}
		BuildIsland(queue, jointCount, hasUnilateralJoints,
					isContinueCollisionIsland);
	}
}

void dgWorldDynamicUpdate::IntegrateArray(const dgBodyInfo *bodyArray,
										  dgInt32 count, dgFloat32 accelTolerance, dgFloat32 timestep,
										  dgInt32 threadIndex, bool update) const {
	bool isAutoSleep = true;
	bool stackSleeping = true;
	dgInt32 sleepCounter = 10000;

	dgFloat32 speedFreeze = m_world->m_freezeSpeed2;
	dgFloat32 accelFreeze = m_world->m_freezeAccel2;
	dgFloat32 maxAccel = dgFloat32(0.0f);
	dgFloat32 maxAlpha = dgFloat32(0.0f);
	dgFloat32 maxSpeed = dgFloat32(0.0f);
	dgFloat32 maxOmega = dgFloat32(0.0f);

	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));

	dgFloat32 forceDamp = DG_FREEZZING_VELOCITY_DRAG;
	if (count <= 2) {
		bool autosleep = bodyArray[0].m_body->m_autoSleep;
		if (count == 2) {
			autosleep &= bodyArray[1].m_body->m_autoSleep;
		}
		if (!autosleep) {
			forceDamp = dgFloat32(0.9999f);
		}
	}
	for (dgInt32 i = 0; i < count; i++) {
		dgBody *const body = bodyArray[i].m_body;

		if (body->m_invMass.m_w && body->m_isInWorld) {
			body->IntegrateVelocity(timestep);

			dgFloat32 accel2 = body->m_accel % body->m_accel;
			dgFloat32 alpha2 = body->m_alpha % body->m_alpha;
			dgFloat32 speed2 = body->m_veloc % body->m_veloc;
			dgFloat32 omega2 = body->m_omega % body->m_omega;

			maxAccel = GetMax(maxAccel, accel2);
			maxAlpha = GetMax(maxAlpha, alpha2);
			maxSpeed = GetMax(maxSpeed, speed2);
			maxOmega = GetMax(maxOmega, omega2);

			bool equilibrium = (accel2 < accelFreeze) && (alpha2 < accelFreeze) && (speed2 < speedFreeze) && (omega2 < speedFreeze);
			if (equilibrium) {
				body->m_veloc = body->m_veloc.Scale(forceDamp);
				body->m_omega = body->m_omega.Scale(forceDamp);
			}
			body->m_equilibrium = dgUnsigned32(equilibrium);
			stackSleeping &= equilibrium;
			isAutoSleep &= body->m_autoSleep;

			sleepCounter = GetMin(sleepCounter, body->m_sleepingCounter);
		}
	}

	if (update) {
		for (dgInt32 i = 0; i < count; i++) {
			dgBody *const body = bodyArray[i].m_body;
			if (body->m_invMass.m_w && body->m_isInWorld) {
				body->UpdateMatrix(timestep, threadIndex);
			}
		}

		if (isAutoSleep) {
			if (stackSleeping) {
				for (dgInt32 i = 0; i < count; i++) {
					dgBody *const body = bodyArray[i].m_body;
					body->m_netForce = zero;
					body->m_netTorque = zero;
					body->m_veloc = zero;
					body->m_omega = zero;
				}
			} else {
				if ((maxAccel > m_world->m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxAccel) || (maxAlpha > m_world->m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxAlpha) || (maxSpeed > m_world->m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxVeloc) || (maxOmega > m_world->m_sleepTable[DG_SLEEP_ENTRIES - 1].m_maxOmega)) {
					for (dgInt32 i = 0; i < count; i++) {
						dgBody *const body = bodyArray[i].m_body;
						body->m_sleepingCounter = 0;
					}
				} else {
					dgInt32 index = 0;
					for (dgInt32 i = 0; i < DG_SLEEP_ENTRIES; i++) {
						if ((maxAccel <= m_world->m_sleepTable[i].m_maxAccel) && (maxAlpha <= m_world->m_sleepTable[i].m_maxAlpha) && (maxSpeed <= m_world->m_sleepTable[i].m_maxVeloc) && (maxOmega <= m_world->m_sleepTable[i].m_maxOmega)) {
							index = i;
							break;
						}
					}

					dgInt32 timeScaleSleepCount = dgInt32(
						dgFloat32(60.0f) * sleepCounter * timestep);
					if (timeScaleSleepCount > m_world->m_sleepTable[index].m_steps) {
						for (dgInt32 i = 0; i < count; i++) {
							dgBody *const body = bodyArray[i].m_body;
							body->m_netForce = zero;
							body->m_netTorque = zero;
							body->m_veloc = zero;
							body->m_omega = zero;
							body->m_equilibrium = true;
						}
					} else {
						sleepCounter++;
						for (dgInt32 i = 0; i < count; i++) {
							dgBody *const body = bodyArray[i].m_body;
							body->m_sleepingCounter = sleepCounter;
						}
					}
				}
			}
		}
	}
}

dgInt32 dgWorldDynamicUpdate::GetJacobialDerivatives(const dgIsland &island,
													 dgInt32 threadIndex, bool bitMode, dgInt32 rowCount, dgFloat32 timestep) {
	dgContraintDescritor constraintParams;

	dgInt32 jointCount = island.m_jointCount;

	constraintParams.m_world = m_world;
	constraintParams.m_threadIndex = threadIndex;
	constraintParams.m_timestep = timestep;
	constraintParams.m_invTimestep = dgFloat32(1.0f / timestep);

	dgJacobianMemory &solverMemory = m_solverMemory[threadIndex];
	dgJointInfo *const constraintArray = &m_constraintArray[island.m_jointStart];

	for (dgInt32 j = 0; j < jointCount; j++) {
		dgConstraint *const constraint = constraintArray[j].m_joint;
		;
		if (constraint->m_isUnilateral ^ bitMode) {
			dgInt32 dof = dgInt32(constraint->m_maxDOF);
			_ASSERTE(dof <= DG_CONSTRAINT_MAX_ROWS);
			for (dgInt32 i = 0; i < dof; i++) {
				constraintParams.m_forceBounds[i].m_low = DG_MIN_BOUND;
				constraintParams.m_forceBounds[i].m_upper = DG_MAX_BOUND;
				constraintParams.m_forceBounds[i].m_jointForce = NULL;
				constraintParams.m_forceBounds[i].m_normalIndex =
					DG_BILATERAL_CONSTRAINT;
			}

			_ASSERTE(constraint->m_body0);
			_ASSERTE(constraint->m_body1);

			constraint->m_body0->m_inCallback = true;
			constraint->m_body1->m_inCallback = true;

			dof = dgInt32(constraint->JacobianDerivative(constraintParams));

			constraint->m_body0->m_inCallback = false;
			constraint->m_body1->m_inCallback = false;

			dgInt32 m0 =
				(constraint->m_body0->m_invMass.m_w != dgFloat32(0.0f)) ? constraint->m_body0->m_index : 0;
			dgInt32 m1 =
				(constraint->m_body1->m_invMass.m_w != dgFloat32(0.0f)) ? constraint->m_body1->m_index : 0;

			_ASSERTE(constraint->m_index == dgUnsigned32(j));
			constraintArray[j].m_autoPairstart = rowCount;
			constraintArray[j].m_autoPaircount = dof;
			constraintArray[j].m_autoPairActiveCount = dof;
			constraintArray[j].m_m0 = m0;
			constraintArray[j].m_m1 = m1;

			for (dgInt32 i = 0; i < dof; i++) {
				_ASSERTE(constraintParams.m_forceBounds[i].m_jointForce);
				solverMemory.m_Jt[rowCount] = constraintParams.m_jacobian[i];

				_ASSERTE(constraintParams.m_jointStiffness[i] >= dgFloat32(0.1f));
				_ASSERTE(constraintParams.m_jointStiffness[i] <= dgFloat32(100.0f));
				solverMemory.m_diagDamp[rowCount] =
					constraintParams.m_jointStiffness[i];
				solverMemory.m_coordenateAccel[rowCount] =
					constraintParams.m_jointAccel[i];
				solverMemory.m_accelIsMotor[rowCount] = dgInt32(
					constraintParams.m_isMotor[i]);
				solverMemory.m_restitution[rowCount] =
					constraintParams.m_restitution[i];
				solverMemory.m_penetration[rowCount] =
					constraintParams.m_penetration[i];
				solverMemory.m_penetrationStiffness[rowCount] =
					constraintParams.m_penetrationStiffness[i];

				solverMemory.m_lowerBoundFrictionCoefficent[rowCount] =
					constraintParams.m_forceBounds[i].m_low;
				solverMemory.m_upperBoundFrictionCoefficent[rowCount] =
					constraintParams.m_forceBounds[i].m_upper;
				solverMemory.m_jointFeebackForce[rowCount] =
					constraintParams.m_forceBounds[i].m_jointForce;
				solverMemory.m_normalForceIndex[rowCount] =
					constraintParams.m_forceBounds[i].m_normalIndex + ((constraintParams.m_forceBounds[i].m_normalIndex >= 0) ? (rowCount - i) : 0);

				rowCount++;
			}

#ifdef _DEBUG
			for (dgInt32 i = 0; i < ((rowCount + 3) & 0xfffc) - rowCount; i++) {
				solverMemory.m_diagDamp[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_coordenateAccel[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_restitution[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_penetration[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_penetrationStiffness[rowCount + i] = dgFloat32(0.0f);

				solverMemory.m_lowerBoundFrictionCoefficent[rowCount + i] = dgFloat32(
					0.0f);
				solverMemory.m_upperBoundFrictionCoefficent[rowCount + i] = dgFloat32(
					0.0f);
				solverMemory.m_jointFeebackForce[rowCount + i] = 0;
				solverMemory.m_normalForceIndex[rowCount + i] = 0;
			}
#endif

			rowCount =
				(rowCount & (DG_SIMD_WORD_SIZE - 1)) ? ((rowCount & (-DG_SIMD_WORD_SIZE)) + DG_SIMD_WORD_SIZE) : rowCount;
			_ASSERTE((rowCount & (DG_SIMD_WORD_SIZE - 1)) == 0);
		}
	}
	return rowCount;
}

void dgWorldDynamicUpdate::BuildJacobianMatrixSimd(const dgIsland &island,
												   dgInt32 threadIndex, dgFloat32 timestep) {
#ifdef DG_BUILD_SIMD_CODE
	//	dgInt32 threads;
	//	dgInt32 rowCount;
	//	dgInt32 bodyCount;
	//	dgInt32 jointCount;

	//	dgContraintDescritor constraintParams;
	//	dgVector zeroVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));

	_ASSERTE(island.m_bodyCount >= 2);
	//_ASSERTE (island.m_jointCount >= 1);

	//	dgInt32 threads = m_world->m_numberOfTheads;
	dgJacobianMemory &solverMemory = m_solverMemory[threadIndex];

	dgInt32 bodyCount = island.m_bodyCount;
	dgBodyInfo *const bodyArray = &m_bodyArray[island.m_bodyStart];

	bodyArray[0].m_body = m_world->GetSentinelBody();

	// body = bodyArray[0].m_body;
	// body->m_hasRelativeVelocity = false;
	// body->m_accel = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	// body->m_alpha = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	_ASSERTE(
		(bodyArray[0].m_body->m_accel % bodyArray[0].m_body->m_accel) == dgFloat32(0.0f));
	_ASSERTE(
		(bodyArray[0].m_body->m_alpha % bodyArray[0].m_body->m_alpha) == dgFloat32(0.0f));

	for (dgInt32 i = 1; i < bodyCount; i++) {
		dgBody *const body = bodyArray[i].m_body;
		_ASSERTE(body->m_invMass.m_w > dgFloat32(0.0f));

		// body->m_hasRelativeVelocity = false;
		// body->AddGyroscopicTorque();
		body->AddDamingAcceleration();
		body->CalcInvInertiaMatrixSimd();
	}

	while (bodyCount >= solverMemory.m_maxBodiesCount) {
		m_world->dgGetUserLock();
		ReallocIntenalForcesMemory(bodyCount, threadIndex);
		m_world->dgReleasedUserLock();
	}

	dgInt32 jointCount = island.m_jointCount;
	// constraintArray = &m_constraintArray[island.m_jointStart];
	dgJointInfo *const constraintArray = &m_constraintArray[island.m_jointStart];
	solverMemory.m_constraintArray = constraintArray;

	dgInt32 rowCount = 0;
	dgFloat32 maxRowCount = 0;
	for (dgInt32 j = 0; j < jointCount; j++) {
		dgConstraint *const constraint = constraintArray[j].m_joint;
		dgInt32 dof =
			dgInt32(
				(constraint->m_maxDOF & (DG_SIMD_WORD_SIZE - 1)) ? ((constraint->m_maxDOF & (-DG_SIMD_WORD_SIZE)) + DG_SIMD_WORD_SIZE) : constraint->m_maxDOF);
		maxRowCount += dof;
	}

	while (maxRowCount > solverMemory.m_maxJacobiansCount) {
		m_world->dgGetUserLock();
		ReallocJacobiansMemory(solverMemory.m_maxJacobiansCount * 2, threadIndex);
		m_world->dgReleasedUserLock();
	}

	if (island.m_hasUnilateralJoints) {
		rowCount = GetJacobialDerivatives(island, threadIndex, false, rowCount,
										  timestep);
	}
	rowCount = GetJacobialDerivatives(island, threadIndex, true, rowCount,
									  timestep);

	solverMemory.m_rowCount = rowCount;
	solverMemory.m_bodyCount = bodyCount;
	solverMemory.m_bodyArray = bodyArray;
	solverMemory.m_jointCount = jointCount;
	solverMemory.m_timeStep = timestep;
	solverMemory.m_invTimeStep = dgFloat32(1.0f) / solverMemory.m_timeStep;

	dgFloat32 *const force = solverMemory.m_force;
	const dgJacobianPair *const Jt = solverMemory.m_Jt;
	dgJacobianPair *const JMinv = solverMemory.m_JMinv;
	dgFloat32 *const diagDamp = solverMemory.m_diagDamp;
	dgFloat32 *const extAccel = solverMemory.m_deltaAccel;
	dgFloat32 *const invDJMinvJt = solverMemory.m_invDJMinvJt;
	dgFloat32 *const coordenateAccel = solverMemory.m_coordenateAccel;
	dgFloat32 **const jointForceFeeback = solverMemory.m_jointFeebackForce;
	//	dgJacobianIndex* const jacobianIndexArray = solverMemory.m_jacobianIndexArray;
	//	dgBilateralBounds* const bilateralForceBounds = solverMemory.m_bilateralForceBounds;

	simd_type zero;
	zero = simd_set1(dgFloat32(0.0f));

	for (dgInt32 k = 0; k < jointCount; k++) {
		//		dgInt32 m0;
		//		dgInt32 m1;
		//		dgInt32 index;
		//		dgInt32 count;
		//		dgFloat32 diag;
		//		dgFloat32 stiffness;
		//		simd_type tmp0;
		//		simd_type tmp1;
		//		simd_type invMass0;
		//		simd_type invMass1;
		//		simd_type tmpDiag;
		//		simd_type tmpAccel;
		//		dgBody* body0;
		//		dgBody* body1;

		dgInt32 index = constraintArray[k].m_autoPairstart;
		dgInt32 count = constraintArray[k].m_autoPaircount;
		dgInt32 m0 = constraintArray[k].m_m0;
		dgInt32 m1 = constraintArray[k].m_m1;
		_ASSERTE(m0 >= 0);
		_ASSERTE(m0 < bodyCount);
		_ASSERTE(m1 >= 0);
		_ASSERTE(m1 < bodyCount);

		dgBody *const body0 = bodyArray[m0].m_body;
		// invMass0 = body0->m_invMass[3];
		simd_type invMass0 = simd_set1(body0->m_invMass[3]);
		_ASSERTE((dgUnsigned64(&body0->m_invWorldInertiaMatrix) & 0x0f) == 0);
		const dgMatrix &invInertia0 = body0->m_invWorldInertiaMatrix;

		dgBody *const body1 = bodyArray[m1].m_body;
		// invMass1 = body1->m_invMass[3];
		simd_type invMass1 = simd_set1(body1->m_invMass[3]);
		_ASSERTE((dgUnsigned64(&body1->m_invWorldInertiaMatrix) & 0x0f) == 0);
		const dgMatrix &invInertia1 = body1->m_invWorldInertiaMatrix;
		for (dgInt32 i = 0; i < count; i++) {
			// JMinv[index].m_jacobian_IM0.m_linear = Jt[index].m_jacobian_IM0.m_linear.Scale (invMass0);
			// JMinv[index].m_jacobian_IM0.m_angular = invInertia0.UnrotateVector (Jt[index].m_jacobian_IM0.m_angular);
			// dgVector tmpDiag (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(Jt[index].m_jacobian_IM0.m_linear));
			// tmpDiag += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(Jt[index].m_jacobian_IM0.m_angular);
			// dgVector tmpAccel (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(body0->m_accel));
			// tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(body0->m_alpha);

			((simd_type &)JMinv[index].m_jacobian_IM0.m_linear) =
				simd_mul_v((simd_type &)Jt[index].m_jacobian_IM0.m_linear, invMass0);
			simd_type tmp0 = (simd_type &)Jt[index].m_jacobian_IM0.m_angular;
			simd_type tmp1 =
				simd_mul_v((simd_type &)invInertia0.m_front, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 0, 0, 0)));
			tmp1 =
				simd_mul_add_v(tmp1, (simd_type &)invInertia0.m_up, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 1, 1, 1)));
			((simd_type &)JMinv[index].m_jacobian_IM0.m_angular) =
				simd_mul_add_v(tmp1, (simd_type &)invInertia0.m_right, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 2, 2, 2)));
			simd_type tmpDiag =
				simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear);
			tmpDiag =
				simd_mul_add_v(tmpDiag, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular);
			simd_type tmpAccel =
				simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, (simd_type &)body0->m_accel);
			tmpAccel =
				simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, (simd_type &)body0->m_alpha);

			// JMinv[index].m_jacobian_IM1.m_linear = Jt[index].m_jacobian_IM1.m_linear.Scale (invMass1);
			// JMinv[index].m_jacobian_IM1.m_angular = invInertia1.UnrotateVector (Jt[index].m_jacobian_IM1.m_angular);
			// tmpDiag += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(Jt[index].m_jacobian_IM1.m_linear);
			// tmpDiag += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(Jt[index].m_jacobian_IM1.m_angular);
			// tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(body1->m_accel);
			// tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(body1->m_alpha);

			((simd_type &)JMinv[index].m_jacobian_IM1.m_linear) =
				simd_mul_v((simd_type &)Jt[index].m_jacobian_IM1.m_linear, invMass1);
			tmp0 = (simd_type &)Jt[index].m_jacobian_IM1.m_angular;
			tmp1 =
				simd_mul_v((simd_type &)invInertia1.m_front, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 0, 0, 0)));
			tmp1 =
				simd_mul_add_v(tmp1, (simd_type &)invInertia1.m_up, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 1, 1, 1)));
			((simd_type &)JMinv[index].m_jacobian_IM1.m_angular) =
				simd_mul_add_v(tmp1, (simd_type &)invInertia1.m_right, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 2, 2, 2)));
			tmpDiag =
				simd_mul_add_v(tmpDiag, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear);
			tmpDiag =
				simd_mul_add_v(tmpDiag, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular);
			tmpAccel =
				simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, (simd_type &)body1->m_accel);
			tmpAccel =
				simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, (simd_type &)body1->m_alpha);

			// coordenateAccel[index] -= (tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z);
			//_ASSERTE(tmpAccel.m128_f32[3] == dgFloat32 (0.0f));
			tmpAccel = simd_add_v(tmpAccel, simd_move_hl_v(tmpAccel, tmpAccel));
			tmpAccel =
				simd_sub_s(zero, simd_add_s(tmpAccel, simd_permut_v(tmpAccel, tmpAccel, PURMUT_MASK(3, 3, 3, 1))));

			simd_store_s(tmpAccel, &extAccel[index]);
			simd_store_s(simd_add_s(simd_load_s(coordenateAccel[index]), tmpAccel),
						 &coordenateAccel[index]);

			// force[index] = bilateralForceBounds[index].m_jointForce[0];
			force[index] = jointForceFeeback[index][0];

			_ASSERTE(diagDamp[index] >= dgFloat32(0.1f));
			_ASSERTE(diagDamp[index] <= dgFloat32(100.0f));
			dgFloat32 stiffness = DG_PSD_DAMP_TOL * diagDamp[index];

			// diag = (tmpDiag.m_x + tmpDiag.m_y + tmpDiag.m_z);
			tmpDiag = simd_add_v(tmpDiag, simd_move_hl_v(tmpDiag, tmpDiag));
			dgFloat32 diag;
			simd_store_s(
				simd_add_s(tmpDiag, simd_permut_v(tmpDiag, tmpDiag, PURMUT_MASK(3, 3, 3, 1))),
				&diag);
			_ASSERTE(diag > dgFloat32(0.0f));
			diagDamp[index] = diag * stiffness;

			diag *= (dgFloat32(1.0f) + stiffness);
			invDJMinvJt[index] = dgFloat32(1.0f) / diag;

			index++;
		}
	}

#endif
}

void dgWorldDynamicUpdate::BuildJacobianMatrix(const dgIsland &island,
											   dgInt32 threadIndex, dgFloat32 timestep) {
	dgVector zeroVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						dgFloat32(0.0f));

	_ASSERTE(island.m_bodyCount >= 2);
	//_ASSERTE (island.m_jointCount >= 1);

	dgJacobianMemory &solverMemory = m_solverMemory[threadIndex];

	dgInt32 bodyCount = island.m_bodyCount;
	dgBodyInfo *const bodyArray = &m_bodyArray[island.m_bodyStart];

	_ASSERTE(
		(bodyArray[0].m_body->m_accel % bodyArray[0].m_body->m_accel) == dgFloat32(0.0f));
	_ASSERTE(
		(bodyArray[0].m_body->m_alpha % bodyArray[0].m_body->m_alpha) == dgFloat32(0.0f));

	for (dgInt32 i = 1; i < bodyCount; i++) {
		dgBody *const body = bodyArray[i].m_body;
		_ASSERTE(body->m_invMass.m_w > dgFloat32(0.0f));
		body->AddDamingAcceleration();
		body->CalcInvInertiaMatrix();
	}

	while (bodyCount >= solverMemory.m_maxBodiesCount) {
		m_world->dgGetUserLock();
		ReallocIntenalForcesMemory(bodyCount, threadIndex);
		m_world->dgReleasedUserLock();
	}

	dgInt32 jointCount = island.m_jointCount;
	dgJointInfo *const constraintArray = &m_constraintArray[island.m_jointStart];
	solverMemory.m_constraintArray = constraintArray;

	dgInt32 maxRowCount = 0;
	for (dgInt32 j = 0; j < jointCount; j++) {
		dgConstraint *const constraint = constraintArray[j].m_joint;
		;
		dgInt32 dof =
			dgInt32(
				(constraint->m_maxDOF & (DG_SIMD_WORD_SIZE - 1)) ? ((constraint->m_maxDOF & (-DG_SIMD_WORD_SIZE)) + DG_SIMD_WORD_SIZE) : constraint->m_maxDOF);
		maxRowCount += dof;
	}

	while (maxRowCount > solverMemory.m_maxJacobiansCount) {
		m_world->dgGetUserLock();
		ReallocJacobiansMemory(solverMemory.m_maxJacobiansCount * 2, threadIndex);
		m_world->dgReleasedUserLock();
	}

	dgInt32 rowCount = 0;
	if (island.m_hasUnilateralJoints) {
		rowCount = GetJacobialDerivatives(island, threadIndex, false, rowCount,
										  timestep);
	}
	rowCount = GetJacobialDerivatives(island, threadIndex, true, rowCount,
									  timestep);

	solverMemory.m_rowCount = rowCount;
	solverMemory.m_bodyCount = bodyCount;
	solverMemory.m_bodyArray = bodyArray;
	solverMemory.m_jointCount = jointCount;
	solverMemory.m_timeStep = timestep;
	solverMemory.m_invTimeStep = dgFloat32(1.0f) / solverMemory.m_timeStep;

	dgFloat32 *const force = solverMemory.m_force;
	const dgJacobianPair *const Jt = solverMemory.m_Jt;
	dgJacobianPair *const JMinv = solverMemory.m_JMinv;
	dgFloat32 *const diagDamp = solverMemory.m_diagDamp;
	dgFloat32 *const extAccel = solverMemory.m_deltaAccel;
	dgFloat32 *const invDJMinvJt = solverMemory.m_invDJMinvJt;
	dgFloat32 *const coordenateAccel = solverMemory.m_coordenateAccel;
	dgFloat32 **const jointForceFeeback = solverMemory.m_jointFeebackForce;
	//	dgInt32* const accelIsMotor = solverMemory.m_accelIsMotor;

	for (dgInt32 k = 0; k < jointCount; k++) {
		dgInt32 index = constraintArray[k].m_autoPairstart;
		dgInt32 count = constraintArray[k].m_autoPaircount;
		dgInt32 m0 = constraintArray[k].m_m0;
		dgInt32 m1 = constraintArray[k].m_m1;

		_ASSERTE(m0 >= 0);
		_ASSERTE(m0 < bodyCount);
		dgBody *const body0 = bodyArray[m0].m_body;
		dgFloat32 invMass0 = body0->m_invMass[3];
		const dgMatrix &invInertia0 = body0->m_invWorldInertiaMatrix;

		_ASSERTE(m1 >= 0);
		_ASSERTE(m1 < bodyCount);
		dgBody *const body1 = bodyArray[m1].m_body;
		dgFloat32 invMass1 = body1->m_invMass[3];
		const dgMatrix &invInertia1 = body1->m_invWorldInertiaMatrix;
		for (dgInt32 i = 0; i < count; i++) {
			// dgFloat32 extenalAcceleration;

			JMinv[index].m_jacobian_IM0.m_linear =
				Jt[index].m_jacobian_IM0.m_linear.Scale(invMass0);
			JMinv[index].m_jacobian_IM0.m_angular = invInertia0.UnrotateVector(
				Jt[index].m_jacobian_IM0.m_angular);
			dgVector tmpDiag(
				JMinv[index].m_jacobian_IM0.m_linear.CompProduct(
					Jt[index].m_jacobian_IM0.m_linear));
			tmpDiag += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(
				Jt[index].m_jacobian_IM0.m_angular);
			dgVector tmpAccel(
				JMinv[index].m_jacobian_IM0.m_linear.CompProduct(body0->m_accel));
			tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(
				body0->m_alpha);

			JMinv[index].m_jacobian_IM1.m_linear =
				Jt[index].m_jacobian_IM1.m_linear.Scale(invMass1);
			JMinv[index].m_jacobian_IM1.m_angular = invInertia1.UnrotateVector(
				Jt[index].m_jacobian_IM1.m_angular);
			tmpDiag += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(
				Jt[index].m_jacobian_IM1.m_linear);
			tmpDiag += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(
				Jt[index].m_jacobian_IM1.m_angular);
			tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(
				body1->m_accel);
			tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(
				body1->m_alpha);

			dgFloat32 extenalAcceleration = -(tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z);
			extAccel[index] = extenalAcceleration;
			coordenateAccel[index] += extenalAcceleration;
			force[index] = jointForceFeeback[index][0];

			// force[index] = 0.0f;

			_ASSERTE(diagDamp[index] >= dgFloat32(0.1f));
			_ASSERTE(diagDamp[index] <= dgFloat32(100.0f));
			dgFloat32 stiffness = DG_PSD_DAMP_TOL * diagDamp[index];

			dgFloat32 diag = (tmpDiag.m_x + tmpDiag.m_y + tmpDiag.m_z);
			_ASSERTE(diag > dgFloat32(0.0f));
			diagDamp[index] = diag * stiffness;

			diag *= (dgFloat32(1.0f) + stiffness);
			// solverMemory.m_diagJMinvJt[index] = diag;
			invDJMinvJt[index] = dgFloat32(1.0f) / diag;

			index++;
		}
	}
}

void dgJacobianMemory::CalculateReactionsForcesSimd(dgInt32 solverMode,
													dgFloat32 tolerance) const {
#ifdef DG_BUILD_SIMD_CODE

	if (m_jointCount == 0) {
		// ApplyExternalForcesAndAccelerationSimd (tolerance);
		ApplyExternalForcesAndAccelerationSimd(0.0f);
		return;
	}

	if (m_jointCount == 1) {
		CalculateSimpleBodyReactionsForcesSimd(tolerance);
		ApplyExternalForcesAndAccelerationSimd(tolerance * 0.001f);
		return;
	}

	if (solverMode) {
		CalculateForcesGameModeSimd(solverMode, tolerance);
	} else {
		CalculateForcesSimulationModeSimd(tolerance);
	}
#endif
}

void dgJacobianMemory::CalculateReactionsForces(dgInt32 solverMode,
												dgFloat32 tolerance) const {
	if (m_jointCount == 0) {
		// ApplyExternalForcesAndAcceleration (tolerance);
		ApplyExternalForcesAndAcceleration(0.0f);
		return;
	}

	if (m_jointCount == 1) {
		CalculateSimpleBodyReactionsForces(tolerance);
		ApplyExternalForcesAndAcceleration(tolerance * 0.001f);
		return;
	}

	if (solverMode) {
		CalculateForcesGameMode(solverMode, tolerance);
	} else {
		CalculateForcesSimulationMode(tolerance);
	}
}

void dgJacobianMemory::ApplyExternalForcesAndAccelerationSimd(
	dgFloat32 tolerance) const {
#ifdef DG_BUILD_SIMD_CODE
	//	dgInt32 hasJointFeeback;
	//	simd_type zero;
	//	simd_type absMask;
	//	simd_type timeStep;
	//	simd_type toleranceSimd;

	dgFloat32 *const force = m_force;
	dgJacobian *const internalForces = m_internalForces;
	const dgJacobianPair *const Jt = m_Jt;
	const dgBodyInfo *const bodyArray = m_bodyArray;
	const dgJointInfo *const constraintArray = m_constraintArray;
	dgFloat32 **const jointForceFeeback = m_jointFeebackForce;

	simd_type zero = simd_set1(dgFloat32(0.0f));

	for (dgInt32 i = 0; i < m_bodyCount; i++) {
		(simd_type &)internalForces[i].m_linear = zero;
		(simd_type &)internalForces[i].m_angular = zero;
	}

	//	dgInt32 tmp = 0x7fffffff;
	dgInt32 hasJointFeeback = 0;
	//	simd_type absMask = simd_set1(*((dgFloat32*)&tmp));
	simd_type timeStep = simd_set1(m_timeStep);
	simd_type toleranceSimd = simd_set1(tolerance);
	toleranceSimd = simd_mul_s(toleranceSimd, toleranceSimd);
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		//		dgInt32 m0;
		//		dgInt32 m1;
		//		dgInt32 first;
		//		dgInt32 count;
		//		dgInt32 index;
		//		simd_type y0_linear;
		//		simd_type y0_angular;
		//		simd_type y1_linear;
		//		simd_type y1_angular;

		dgInt32 first = constraintArray[i].m_autoPairstart;
		dgInt32 count = constraintArray[i].m_autoPaircount;
		dgInt32 m0 = constraintArray[i].m_m0;
		dgInt32 m1 = constraintArray[i].m_m1;

		simd_type y0_linear = zero;
		simd_type y0_angular = zero;
		simd_type y1_linear = zero;
		simd_type y1_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 index = j + first;
			// val = force[index];
			//_ASSERTE (dgCheckFloat(val));
			simd_type val = simd_set1(force[index]);
			simd_store_s(val, &jointForceFeeback[index][0]);
			//			maxForce = simd_max_s (maxForce, simd_and_v (val, absMask));

			y0_linear =
				simd_mul_add_v(y0_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, val);
			y0_angular =
				simd_mul_add_v(y0_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, val);
			y1_linear =
				simd_mul_add_v(y1_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, val);
			y1_angular =
				simd_mul_add_v(y1_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, val);
		}

		//		if (constraintArray[i].m_joint->GetId() == dgContactConstraintId) {
		//			dgFloat32 force;
		//			simd_store_s (maxForce, &force);
		//			m_world->AddToBreakQueue ((dgContact*)constraintArray[i].m_joint, force);
		//		}

		hasJointFeeback |= (constraintArray[i].m_joint->m_updaFeedbackCallback ? 1 : 0);
		//		if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
		//			constraintArray[i].m_joint->m_updaFeedbackCallback (*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
		//		}

		(simd_type &)internalForces[m0].m_linear =
			simd_add_v((simd_type &)internalForces[m0].m_linear, y0_linear);
		(simd_type &)internalForces[m0].m_angular =
			simd_add_v((simd_type &)internalForces[m0].m_angular, y0_angular);
		(simd_type &)internalForces[m1].m_linear =
			simd_add_v((simd_type &)internalForces[m1].m_linear, y1_linear);
		(simd_type &)internalForces[m1].m_angular =
			simd_add_v((simd_type &)internalForces[m1].m_angular, y1_angular);
	}

	for (dgInt32 i = 1; i < m_bodyCount; i++) {
		//		simd_type tmp;
		//		simd_type accel;
		//		simd_type alpha;
		//		dgBody* body;
		dgBody *const body = bodyArray[i].m_body;
		(simd_type &)body->m_accel =
			simd_add_v((simd_type &)internalForces[i].m_linear, (simd_type &)body->m_accel);
		(simd_type &)body->m_alpha =
			simd_add_v((simd_type &)internalForces[i].m_angular, (simd_type &)body->m_alpha);

		// dgVector accel (body->m_accel.Scale (body->m_invMass.m_w));
		simd_type accel =
			simd_mul_v((simd_type &)body->m_accel, simd_set1(body->m_invMass.m_w));
		// dgVector alpha (body->m_invWorldInertiaMatrix.RotateVector (body->m_alpha));
		simd_type alpha =
			simd_mul_add_v(simd_mul_add_v(simd_mul_v((simd_type &)body->m_invWorldInertiaMatrix[0], simd_set1(body->m_alpha.m_x)),
										  (simd_type &)body->m_invWorldInertiaMatrix[1], simd_set1(body->m_alpha.m_y)),
						   (simd_type &)body->m_invWorldInertiaMatrix[2], simd_set1(body->m_alpha.m_z));

		// dgVector accel (body->m_accel.Scale (body->m_invMass.m_w));
		// dgVector alpha (body->m_invWorldInertiaMatrix.RotateVector (body->m_alpha));
		// error = accel % accel;
		// if (error < accelTol2) {
		//	accel = zero;
		//	body->m_accel = zero;
		// }
		simd_type tmp = simd_mul_v(accel, accel);
		tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
		tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
		tmp = simd_cmplt_s(tmp, toleranceSimd);
		tmp = simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 0));
		accel = simd_andnot_v(accel, tmp);
		(simd_type &)body->m_accel = simd_andnot_v((simd_type &)body->m_accel, tmp);

		//		error = alpha % alpha;
		//		if (error < accelTol2) {
		//			alpha = zero;
		//			body->m_alpha = zero;
		//		}
		tmp = simd_mul_v(alpha, alpha);
		tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
		tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
		tmp = simd_cmplt_s(tmp, toleranceSimd);
		tmp = simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 0));
		alpha = simd_andnot_v(alpha, tmp);
		(simd_type &)body->m_alpha = simd_andnot_v((simd_type &)body->m_alpha, tmp);

		(simd_type &)body->m_netForce = (simd_type &)body->m_accel;
		(simd_type &)body->m_netTorque = (simd_type &)body->m_alpha;

		(simd_type &)body->m_veloc =
			simd_mul_add_v((simd_type &)body->m_veloc, accel, timeStep);
		(simd_type &)body->m_omega =
			simd_mul_add_v((simd_type &)body->m_omega, alpha, timeStep);
	}

	if (hasJointFeeback) {
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
				constraintArray[i].m_joint->m_updaFeedbackCallback(
					*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
			}
		}
	}

#endif
}

void dgJacobianMemory::ApplyExternalForcesAndAcceleration(
	dgFloat32 tolerance) const {
	//	dgInt32 hasJointFeeback;
	//	dgFloat32 accelTol2;
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgBodyInfo *const bodyArray = m_bodyArray;
	const dgJointInfo *const constraintArray = m_constraintArray;
	dgFloat32 **const jointForceFeeback = m_jointFeebackForce;
	dgJacobian *const internalForces = m_internalForces;

	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));
	for (dgInt32 i = 0; i < m_bodyCount; i++) {
		internalForces[i].m_linear = zero;
		internalForces[i].m_angular = zero;
	}

	dgInt32 hasJointFeeback = 0;
	dgFloat32 accelTol2 = tolerance * tolerance;
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		//		dgInt32 m0;
		//		dgInt32 m1;
		//		dgInt32 first;
		//		dgInt32 count;
		//		dgInt32 index;
		//		dgFloat32 val;
		dgInt32 first = constraintArray[i].m_autoPairstart;
		dgInt32 count = constraintArray[i].m_autoPaircount;

		dgInt32 m0 = constraintArray[i].m_m0;
		dgInt32 m1 = constraintArray[i].m_m1;

		dgJacobian y0;
		dgJacobian y1;
		y0.m_linear = zero;
		y0.m_angular = zero;
		y1.m_linear = zero;
		y1.m_angular = zero;

		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 index = j + first;
			dgFloat32 val = force[index];

			_ASSERTE(dgCheckFloat(val));
			jointForceFeeback[index][0] = val;

			y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale(val);
			y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale(val);
			y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale(val);
			y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale(val);
		}

		//		if (constraintArray[i].m_joint->GetId() == dgContactConstraintId) {
		//			m_world->AddToBreakQueue ((dgContact*)constraintArray[i].m_joint, maxForce);
		//		}

		//		hasJointFeeback |= dgUnsigned32 (constraintArray[i].m_joint->m_updaFeedbackCallback);
		hasJointFeeback |= (constraintArray[i].m_joint->m_updaFeedbackCallback ? 1 : 0);

		internalForces[m0].m_linear += y0.m_linear;
		internalForces[m0].m_angular += y0.m_angular;
		internalForces[m1].m_linear += y1.m_linear;
		internalForces[m1].m_angular += y1.m_angular;
	}

	for (dgInt32 i = 1; i < m_bodyCount; i++) {
		dgBody *const body = bodyArray[i].m_body;
		body->m_accel += internalForces[i].m_linear;
		body->m_alpha += internalForces[i].m_angular;

		dgVector accel(body->m_accel.Scale(body->m_invMass.m_w));
		dgVector alpha(body->m_invWorldInertiaMatrix.RotateVector(body->m_alpha));
		dgFloat32 error = accel % accel;
		if (error < accelTol2) {
			accel = zero;
			body->m_accel = zero;
		}

		error = alpha % alpha;
		if (error < accelTol2) {
			alpha = zero;
			body->m_alpha = zero;
		}

		body->m_netForce = body->m_accel;
		body->m_netTorque = body->m_alpha;

		body->m_veloc += accel.Scale(m_timeStep);
		body->m_omega += alpha.Scale(m_timeStep);
	}

	if (hasJointFeeback) {
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
				constraintArray[i].m_joint->m_updaFeedbackCallback(
					*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
			}
		}
	}
}

void dgJacobianMemory::SwapRowsSimd(dgInt32 i, dgInt32 j) const {
#ifdef DG_BUILD_SIMD_CODE
	//	simd_type tmp0;
	//	simd_type tmp1;
	//	simd_type tmp2;
	//	simd_type tmp3;
	//	simd_type tmp4;
	//	simd_type tmp5;
	//	simd_type tmp6;
	//	simd_type tmp7;

	_ASSERTE(i != j);
	// Swap (m_Jt[i], m_Jt[j]);
	dgJacobianPair *const ptr0 = m_Jt;
	simd_type tmp0 = (simd_type &)ptr0[i].m_jacobian_IM0.m_linear;
	simd_type tmp1 = (simd_type &)ptr0[i].m_jacobian_IM0.m_angular;
	simd_type tmp2 = (simd_type &)ptr0[i].m_jacobian_IM1.m_linear;
	simd_type tmp3 = (simd_type &)ptr0[i].m_jacobian_IM1.m_angular;
	simd_type tmp4 = (simd_type &)ptr0[j].m_jacobian_IM0.m_linear;
	simd_type tmp5 = (simd_type &)ptr0[j].m_jacobian_IM0.m_angular;
	simd_type tmp6 = (simd_type &)ptr0[j].m_jacobian_IM1.m_linear;
	simd_type tmp7 = (simd_type &)ptr0[j].m_jacobian_IM1.m_angular;

	(simd_type &)ptr0[j].m_jacobian_IM0.m_linear = tmp0;
	(simd_type &)ptr0[j].m_jacobian_IM0.m_angular = tmp1;
	(simd_type &)ptr0[j].m_jacobian_IM1.m_linear = tmp2;
	(simd_type &)ptr0[j].m_jacobian_IM1.m_angular = tmp3;
	(simd_type &)ptr0[i].m_jacobian_IM0.m_linear = tmp4;
	(simd_type &)ptr0[i].m_jacobian_IM0.m_angular = tmp5;
	(simd_type &)ptr0[i].m_jacobian_IM1.m_linear = tmp6;
	(simd_type &)ptr0[i].m_jacobian_IM1.m_angular = tmp7;

	// Swap (m_JMinv[i], m_JMinv[j]);
	dgJacobianPair *const ptr1 = m_JMinv;
	tmp0 = (simd_type &)ptr1[i].m_jacobian_IM0.m_linear;
	tmp1 = (simd_type &)ptr1[i].m_jacobian_IM0.m_angular;
	tmp2 = (simd_type &)ptr1[i].m_jacobian_IM1.m_linear;
	tmp3 = (simd_type &)ptr1[i].m_jacobian_IM1.m_angular;
	tmp4 = (simd_type &)ptr1[j].m_jacobian_IM0.m_linear;
	tmp5 = (simd_type &)ptr1[j].m_jacobian_IM0.m_angular;
	tmp6 = (simd_type &)ptr1[j].m_jacobian_IM1.m_linear;
	tmp7 = (simd_type &)ptr1[j].m_jacobian_IM1.m_angular;

	(simd_type &)ptr1[j].m_jacobian_IM0.m_linear = tmp0;
	(simd_type &)ptr1[j].m_jacobian_IM0.m_angular = tmp1;
	(simd_type &)ptr1[j].m_jacobian_IM1.m_linear = tmp2;
	(simd_type &)ptr1[j].m_jacobian_IM1.m_angular = tmp3;
	(simd_type &)ptr1[i].m_jacobian_IM0.m_linear = tmp4;
	(simd_type &)ptr1[i].m_jacobian_IM0.m_angular = tmp5;
	(simd_type &)ptr1[i].m_jacobian_IM1.m_linear = tmp6;
	(simd_type &)ptr1[i].m_jacobian_IM1.m_angular = tmp7;

	Swap(m_diagDamp[i], m_diagDamp[j]);
	Swap(m_invDJMinvJt[i], m_invDJMinvJt[j]);
	//	Swap (m_jacobianIndexArray[i], m_jacobianIndexArray[j]);
	Swap(m_normalForceIndex[i], m_normalForceIndex[j]);
	//	Swap (m_lowerBoundFrictionForce[i], m_lowerBoundFrictionForce[j]);
	//	Swap (m_upperBoundFrictionForce[i], m_upperBoundFrictionForce[j]);
	Swap(m_lowerBoundFrictionCoefficent[i], m_lowerBoundFrictionCoefficent[j]);
	Swap(m_upperBoundFrictionCoefficent[i], m_upperBoundFrictionCoefficent[j]);
	Swap(m_jointFeebackForce[i], m_jointFeebackForce[j]);
	Swap(m_coordenateAccel[i], m_coordenateAccel[j]);
	Swap(m_force[i], m_force[j]);
	Swap(m_accel[i], m_accel[j]);
	//	Swap (m_forceStep[i], m_forceStep[j]);
	Swap(m_deltaAccel[i], m_deltaAccel[j]);
	Swap(m_deltaForce[i], m_deltaForce[j]);

#else

#endif
}

void dgJacobianMemory::SwapRows(dgInt32 i, dgInt32 j) const {
	_ASSERTE(i != j);
#define SwapMacro(a, b) Swap(a, b)

	SwapMacro(m_Jt[i], m_Jt[j]);
	SwapMacro(m_JMinv[i], m_JMinv[j]);

	SwapMacro(m_diagDamp[i], m_diagDamp[j]);
	SwapMacro(m_invDJMinvJt[i], m_invDJMinvJt[j]);
	// SwapMacro (m_diagJMinvJt[i], m_diagJMinvJt[j]);
	// SwapMacro (m_frictionThreshold[i], m_frictionThreshold[j]);
	// SwapMacro (m_jacobianIndexArray[i], m_jacobianIndexArray[j]);
	// SwapMacro (m_bilateralForceBounds[i], m_bilateralForceBounds[j]);

	SwapMacro(m_normalForceIndex[i], m_normalForceIndex[j]);
	//	SwapMacro (m_lowerBoundFrictionForce[i], m_lowerBoundFrictionForce[j]);
	//	SwapMacro (m_upperBoundFrictionForce[i], m_upperBoundFrictionForce[j]);
	SwapMacro(m_lowerBoundFrictionCoefficent[i],
			  m_lowerBoundFrictionCoefficent[j]);
	SwapMacro(m_upperBoundFrictionCoefficent[i],
			  m_upperBoundFrictionCoefficent[j]);
	SwapMacro(m_jointFeebackForce[i], m_jointFeebackForce[j]);

	SwapMacro(m_coordenateAccel[i], m_coordenateAccel[j]);

	SwapMacro(m_force[i], m_force[j]);
	SwapMacro(m_accel[i], m_accel[j]);
	//	SwapMacro (m_forceStep[i], m_forceStep[j]);
	SwapMacro(m_deltaAccel[i], m_deltaAccel[j]);
	SwapMacro(m_deltaForce[i], m_deltaForce[j]);
}

void dgJacobianMemory::CalculateSimpleBodyReactionsForcesSimd(
	dgFloat32 maxAccNorm) const {
#ifdef DG_BUILD_SIMD_CODE

	simd_type accelPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type activeRowPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type lowBoundPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type highBoundPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type deltaAccelPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type deltaForcePtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];

	dgFloat32 *const force = m_force;
	dgFloat32 *const activeRow = (dgFloat32 *)activeRowPtr;
	dgFloat32 *const lowBound = (dgFloat32 *)lowBoundPtr;
	dgFloat32 *const highBound = (dgFloat32 *)highBoundPtr;
	dgFloat32 *const accel = (dgFloat32 *)accelPtr;
	dgFloat32 *const deltaAccel = (dgFloat32 *)deltaAccelPtr;
	dgFloat32 *const deltaForce = (dgFloat32 *)deltaForcePtr;

	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgFloat32 *const coordenateAccel = m_coordenateAccel;
	const dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const lowerFriction = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFriction = m_upperBoundFrictionCoefficent;

	dgFloatSign tmpIndex;
	tmpIndex.m_integer.m_iVal = 0x7fffffff;

	simd_type signMask = simd_set1(tmpIndex.m_fVal);
	simd_type one = simd_set1(dgFloat32(1.0f));
	simd_type zero = simd_set1(dgFloat32(0.0f));
	simd_type tol_pos_1eNeg5 = simd_set1(dgFloat32(1.0e-5f));
	simd_type tol_pos_1eNeg8 = simd_set1(dgFloat32(1.0e-8f));
	simd_type tol_pos_1eNeg16 = simd_set1(dgFloat32(1.0e-16f));
	simd_type tol_neg_1eNeg16 = simd_set1(dgFloat32(-1.0e-16f));
	// dgVector zero (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));

	dgInt32 count = constraintArray[0].m_autoPaircount;
	_ASSERTE(constraintArray[0].m_autoPairstart == 0);
	dgInt32 roundCount = count & (-DG_SIMD_WORD_SIZE);

	if (roundCount != count) {
		roundCount += 4;
		for (dgInt32 i = count; i < roundCount; i++) {
			// force[i] = dgFloat32 (0.0f);
			// accel[i] = dgFloat32 (0.0f);
			// activeRow[i] = dgFloat32 (0.0f);
			// deltaAccel[i] = dgFloat32 (0.0f);
			// deltaForce[i] = dgFloat32 (0.0f);
			m_normalForceIndex[i] = -1;
			simd_store_s(zero, &force[i]);
			simd_store_s(zero, &accel[i]);
			simd_store_s(zero, &activeRow[i]);
			simd_store_s(zero, &deltaAccel[i]);
			simd_store_s(zero, &deltaForce[i]);
			simd_store_s(one, &m_lowerBoundFrictionCoefficent[i]);
			simd_store_s(zero, &m_upperBoundFrictionCoefficent[i]);
		}
	}

	//	j0 = jacobianIndexArray[0].m_m0;
	//	j1 = jacobianIndexArray[0].m_m1;
	// dgInt32 m0 = constraintArray[0].m_m0;
	// dgInt32 m1 = constraintArray[0].m_m1;

	//	dgBody* const body0 = bodyArray[m0].m_body;
	//	dgBody* const body1 = bodyArray[m1].m_body;

	// ak = dgSqrt((body1->m_accel % body1->m_accel) + (body0->m_accel % body0->m_accel));
	//	tmp3 = simd_mul_add_v (simd_mul_v ((simd_type&)body1->m_accel, (simd_type&)body1->m_accel),
	//									    (simd_type&)body0->m_accel, (simd_type&)body0->m_accel);
	//	tmp3 = simd_add_v (tmp3, simd_permut_v (tmp3, tmp3, PURMUT_MASK (0, 0, 3, 2)));
	//	tmp3 = simd_add_s (tmp3, simd_permut_v (tmp3, tmp3, PURMUT_MASK (0, 0, 0, 1)));
	//	tmp3 = simd_mul_s (tmp3, simd_rsqrt_s(tmp3));

	//  maxPasses = 0;
	simd_type tmp2 = zero;
	//	tmp3 = simd_permut_v (tmp3, tmp3, PURMUT_MASK (0, 0, 0, 0));
	//	for (i = 0; i <roundCount; i ++) {
	for (dgInt32 i = 0; i < roundCount; i += DG_SIMD_WORD_SIZE) {
		//		simd_type tmp5;
		//		simd_type tmp6;
		//		simd_type tmp7;
		//		bool test;
		//		bool lowBoundTest;
		//		bool highBoundTest;
		//		dgFloat32 normalForce;

		// simd_type lowBoundTest;
		// simd_type highBoundTest;

		//		k = normalForceIndex[i];
		//		if (k >= 0) {
		//			val = force[k];
		//			if (val < dgFloat32 (1.0e-2f)) {
		//				val = ak;
		//			}
		//		} else {
		//			val = dgFloat32 (1.0f);
		//		}

		//		normalForce = force[normalForceIndex[i]] ;
		//		normalForce = (normalForce > dgFloat32 (1.0e-2f)) ? normalForce : ak;

		simd_type normalForce =
			simd_move_lh_v(simd_pack_lo_v(simd_load_s(force[normalForceIndex[i + 0]]),
										  simd_load_s(force[normalForceIndex[i + 1]])),
						   simd_pack_lo_v(simd_load_s(force[normalForceIndex[i + 2]]),
										  simd_load_s(force[normalForceIndex[i + 3]])));
		//		test = simd_cmplt_v (normalForce, tol_pos_1eNeg2);
		//		normalForce = simd_or_v (simd_and_v (tmp3, test), simd_andnot_v (normalForce, test));

		//		lowBound[i] = normalForce * lowerFriction[i];
		//		highBound[i] = normalForce * upperFriction[i];
		(simd_type &)lowBound[i] =
			simd_mul_v(normalForce, (simd_type &)lowerFriction[i]);
		(simd_type &)highBound[i] =
			simd_mul_v(normalForce, (simd_type &)upperFriction[i]);

		//		activeRow[i] = dgFloat32 (1.0f);
		//		if (force[i] < lowBound[i]) {
		//			maxPasses --;
		//			force[i] = lowBound[i];
		//			activeRow[i] = dgFloat32 (0.0f);
		//		} else if (force[i] > highBound[i]) {
		//			maxPasses --;
		//			force[i] = highBound[i];
		//			activeRow[i] = dgFloat32 (0.0f);
		//		}

		//		lowBoundTest = force[i] < lowBound[i];
		//		highBoundTest = force[i] > highBound[i];
		//		lowBoundTest = simd_cmplt_v ((simd_type&)force[i], (simd_type&)lowBound[i]);
		//		highBoundTest = simd_cmpgt_v ((simd_type&)force[i], (simd_type&)highBound[i]);
		//		test = lowBoundTest | highBoundTest;
		//		test = simd_or_v (lowBoundTest, highBoundTest);
		simd_type test =
			simd_or_v(simd_cmplt_v((simd_type &)force[i], (simd_type &)lowBound[i]), simd_cmpgt_v((simd_type &)force[i], (simd_type &)highBound[i]));

		//		activeRow[i] = test ? dgFloat32 (0.0f) : dgFloat32 (1.0f);
		(simd_type &)activeRow[i] = simd_andnot_v(one, test);

		//		maxPasses += test ? 0 : 1;
		tmp2 = simd_add_v(tmp2, (simd_type &)activeRow[i]);

		//		force[i] = lowBoundTest ? lowBound[i] : (highBoundTest ? highBound[i] : force[i]);
		//		normalForce = simd_or_v (simd_and_v ((simd_type&)lowBound[i], lowBoundTest), simd_and_v ((simd_type&)highBound[i], highBoundTest));
		//		(simd_type&)force[i] = simd_or_v (simd_and_v (normalForce, test), simd_andnot_v ((simd_type&)force[i], test));
		(simd_type &)force[i] =
			simd_min_v((simd_type &)highBound[i], simd_max_v((simd_type &)force[i], (simd_type &)lowBound[i]));
	}

	tmp2 = simd_add_v(tmp2, simd_move_hl_v(tmp2, tmp2));
	dgInt32 maxPasses =
		simd_store_is(simd_add_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(0, 0, 0, 1))));

	// y0.m_linear = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	// y0.m_angular = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	// y1.m_linear = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	// y1.m_angular = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	simd_type y0_linear = zero;
	simd_type y0_angular = zero;
	simd_type y1_linear = zero;
	simd_type y1_angular = zero;
	for (dgInt32 i = 0; i < count; i++) {
		// force = m_force[i];
		simd_type tmp1 = simd_set1(force[i]);

		// y0.m_linear += m_Jt[i].m_jacobian_IM0.m_linear.Scale (force);
		// y0.m_angular += m_Jt[i].m_jacobian_IM0.m_angular.Scale (force);
		// y1.m_linear += m_Jt[i].m_jacobian_IM1.m_linear.Scale (force);
		// y1.m_angular += m_Jt[i].m_jacobian_IM1.m_angular.Scale (force);
		y0_linear =
			simd_mul_add_v(y0_linear, (simd_type &)Jt[i].m_jacobian_IM0.m_linear, tmp1);
		y0_angular =
			simd_mul_add_v(y0_angular, (simd_type &)Jt[i].m_jacobian_IM0.m_angular, tmp1);
		y1_linear =
			simd_mul_add_v(y1_linear, (simd_type &)Jt[i].m_jacobian_IM1.m_linear, tmp1);
		y1_angular =
			simd_mul_add_v(y1_angular, (simd_type &)Jt[i].m_jacobian_IM1.m_angular, tmp1);
	}

	// akNum = dgFloat32 (0.0f);
	// accNorm = dgFloat32(0.0f);
	simd_type tmp0 = zero;
	simd_type tmp1 = zero;
	for (dgInt32 i = 0; i < count; i++) {
		// dgVector acc (m_JMinv[i].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
		// acc += m_JMinv[i].m_jacobian_IM0.m_angular.CompProduct (y0.m_angular);
		// acc += m_JMinv[i].m_jacobian_IM1.m_linear.CompProduct (y1.m_linear);
		// acc += m_JMinv[i].m_jacobian_IM1.m_angular.CompProduct (y1.m_angular);
		simd_type tmp2 =
			simd_mul_v((simd_type &)JMinv[i].m_jacobian_IM0.m_linear, y0_linear);
		tmp2 =
			simd_mul_add_v(tmp2, (simd_type &)JMinv[i].m_jacobian_IM0.m_angular, y0_angular);
		tmp2 =
			simd_mul_add_v(tmp2, (simd_type &)JMinv[i].m_jacobian_IM1.m_linear, y1_linear);
		tmp2 =
			simd_mul_add_v(tmp2, (simd_type &)JMinv[i].m_jacobian_IM1.m_angular, y1_angular);

		// m_accel[i] = m_coordenateAccel[i] - acc.m_x - acc.m_y - acc.m_z - m_force[i] * m_diagDamp[i];
		tmp2 = simd_add_v(tmp2, simd_move_hl_v(tmp2, tmp2));
		tmp2 = simd_add_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(3, 3, 3, 1)));
		tmp2 =
			simd_sub_s(simd_load_s(coordenateAccel[i]), simd_mul_add_s(tmp2, simd_load_s(force[i]), simd_load_s(diagDamp[i])));
		simd_store_s(tmp2, &accel[i]);

		// m_deltaForce[i] = m_accel[i] * m_invDJMinvJt[i] * activeRow[i];
		simd_type tmp3 =
			simd_mul_s(tmp2, simd_mul_s(simd_load_s(invDJMinvJt[i]), simd_load_s(activeRow[i])));
		simd_store_s(tmp3, &deltaForce[i]);

		// akNum += m_accel[i] * m_deltaForce[i];
		tmp0 = simd_mul_add_v(tmp0, tmp3, tmp2);

		// accNorm = GetMax (dgAbsf (m_accel[i] * activeRow[i]), accNorm);
		tmp1 =
			simd_max_s(tmp1, simd_and_v(simd_mul_v(tmp2, simd_load_s(activeRow[i])), signMask));
	}

	dgFloat32 akNum;
	dgFloat32 accNorm;
	simd_store_s(tmp0, &akNum);
	simd_store_s(tmp1, &accNorm);
	for (dgInt32 i = 0; (i < maxPasses) && (accNorm > maxAccNorm); i++) {
		//		dgInt32 clampedForceIndex;
		//		dgFloat32 ak;
		//		dgFloat32 clampedForceIndexValue;

		// MatrixTimeVector (forceRows, m_deltaForce, m_deltaAccel);
		// y0.m_linear = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		// y0.m_angular = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		// y1.m_linear = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		// y1.m_angular = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		simd_type y0_linear = zero;
		simd_type y0_angular = zero;
		simd_type y1_linear = zero;
		simd_type y1_angular = zero;
		for (dgInt32 k = 0; k < count; k++) {
			// ak = m_deltaForce[k];
			simd_type tmp1 = simd_set1(deltaForce[k]);

			// y0.m_linear += m_Jt[k].m_jacobian_IM0.m_linear.Scale (ak);
			// y0.m_angular += m_Jt[k].m_jacobian_IM0.m_angular.Scale (ak);
			// y1.m_linear += m_Jt[k].m_jacobian_IM1.m_linear.Scale (ak);
			// y1.m_angular += m_Jt[k].m_jacobian_IM1.m_angular.Scale (ak);
			y0_linear =
				simd_mul_add_v(y0_linear, (simd_type &)Jt[k].m_jacobian_IM0.m_linear, tmp1);
			y0_angular =
				simd_mul_add_v(y0_angular, (simd_type &)Jt[k].m_jacobian_IM0.m_angular, tmp1);
			y1_linear =
				simd_mul_add_v(y1_linear, (simd_type &)Jt[k].m_jacobian_IM1.m_linear, tmp1);
			y1_angular =
				simd_mul_add_v(y1_angular, (simd_type &)Jt[k].m_jacobian_IM1.m_angular, tmp1);
		}

		// akDen = dgFloat32 (0.0f);
		simd_type tmp0 = zero;
		for (dgInt32 k = 0; k < count; k++) {
			// dgVector acc (m_JMinv[k].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
			// acc += m_JMinv[k].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
			// acc += m_JMinv[k].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
			// acc += m_JMinv[k].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);
			simd_type tmp2 =
				simd_mul_v((simd_type &)JMinv[k].m_jacobian_IM0.m_linear, y0_linear);
			tmp2 =
				simd_mul_add_v(tmp2, (simd_type &)JMinv[k].m_jacobian_IM0.m_angular, y0_angular);
			tmp2 =
				simd_mul_add_v(tmp2, (simd_type &)JMinv[k].m_jacobian_IM1.m_linear, y1_linear);
			tmp2 =
				simd_mul_add_v(tmp2, (simd_type &)JMinv[k].m_jacobian_IM1.m_angular, y1_angular);

			// m_deltaAccel[k] = acc.m_x + acc.m_y + acc.m_z + m_deltaForce[k] * m_diagDamp[k];
			simd_type tmp1 = simd_load_s(deltaForce[k]);
			tmp2 = simd_add_v(tmp2, simd_move_hl_v(tmp2, tmp2));
			tmp2 =
				simd_add_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(3, 3, 3, 1)));
			tmp2 = simd_mul_add_s(tmp2, tmp1, simd_load_s(diagDamp[k]));
			simd_store_s(tmp2, &deltaAccel[k]);

			// akDen += m_deltaAccel[k] * m_deltaForce[k];
			tmp0 = simd_mul_add_v(tmp0, tmp2, tmp1);
		}

		//_ASSERTE (akDen > dgFloat32 (0.0f));
		// akDen = GetMax (akDen, dgFloat32(1.0e-16f));
		//_ASSERTE (dgAbsf (akDen) >= dgFloat32(1.0e-16f));
		// ak = akNum / akDen;
		tmp0 = simd_div_s(simd_load_s(akNum), simd_max_s(tmp0, tol_pos_1eNeg16));

		//		simd_type min_index;
		//		simd_type minClampIndex;
		//		simd_type min_index_step;
		//		simd_type campedIndexValue;
		simd_type campedIndexValue = zero;
		simd_type minClampIndex = simd_set1(dgFloat32(-1.0f));
		simd_type min_index_step = simd_set1(dgFloat32(4.0f));
		tmp0 = simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 0));
		simd_type min_index =
			simd_set(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(2.0f), dgFloat32(3.0f));

		//		for (k = 0; k < roundCount; k ++) {
		for (dgInt32 k = 0; k < roundCount; k += DG_SIMD_WORD_SIZE) {
			//			simd_type val;
			//			simd_type num;
			//			simd_type den;
			//			simd_type test;
			//			simd_type negTest;
			//			simd_type posTest;
			//			simd_type negValTest;
			//			simd_type posValTest;
			//			simd_type negDeltaForceTest;
			//			simd_type posDeltaForceTest;

			// Make sure AK is not negative
			//			val = force[k] + ak * deltaForce[k];
			simd_type val =
				simd_mul_add_v((simd_type &)force[k], tmp0, (simd_type &)deltaForce[k]);

			//			negValTest = val < lowBound[k];
			simd_type negValTest = simd_cmplt_v(val, (simd_type &)lowBound[k]);

			//			posValTest = val > highBound[k];
			simd_type posValTest = simd_cmpgt_v(val, (simd_type &)highBound[k]);

			//			negDeltaForceTest = deltaForce[k] < dgFloat32 (-1.0e-16f);
			simd_type negDeltaForceTest =
				simd_cmplt_v((simd_type &)deltaForce[k], tol_neg_1eNeg16);

			//			posDeltaForceTest = deltaForce[k] > dgFloat32 ( 1.0e-16f);
			simd_type posDeltaForceTest =
				simd_cmpgt_v((simd_type &)deltaForce[k], tol_pos_1eNeg16);

			//			negTest = negValTest & negDeltaForceTest;
			simd_type negTest = simd_and_v(negValTest, negDeltaForceTest);

			//			posTest = posValTest & posDeltaForceTest;
			simd_type posTest = simd_and_v(posValTest, posDeltaForceTest);

			//			test = negTest | posTest;
			simd_type test = simd_or_v(negTest, posTest);

			//			num = negTest ? lowBound[k] : (posTest ? highBound[k] : force[k]);
			simd_type num =
				simd_or_v(simd_and_v((simd_type &)lowBound[k], negTest), simd_and_v((simd_type &)highBound[k], posTest));
			num =
				simd_or_v(simd_and_v(num, test), simd_andnot_v((simd_type &)force[k], test));

			//			den = test ? deltaForce[k] : dgFloat32 (1.0f);
			simd_type den =
				simd_or_v(simd_and_v((simd_type &)deltaForce[k], test), simd_andnot_v(one, test));

			//			test = test & (activeRow[k] > dgFloat32 (0.0f));
			test = simd_and_v(test, simd_cmpgt_v((simd_type &)activeRow[k], zero));

			//			_ASSERTE (dgAbsf (den) > 1.0e-16f);
			//			ak = test ? (num - force[k]) / den : ak;
			tmp0 =
				simd_or_v(simd_div_v(simd_sub_v(num, (simd_type &)force[k]), den), simd_andnot_v(tmp0, test));

			//			ak = (ak < dgFloat32 (1.0e-8f)) ? dgFloat32 (0.0f) : ak;
			tmp0 = simd_and_v(tmp0, simd_cmpgt_v(tmp0, tol_pos_1eNeg8));

			//			clampedForceIndex = test ? k : clampedForceIndex;
			minClampIndex =
				simd_or_v(simd_and_v(min_index, test), simd_andnot_v(minClampIndex, test));
			min_index = simd_add_v(min_index, min_index_step);

			//			clampedForceIndexValue = test ? num : clampedForceIndexValue;
			campedIndexValue =
				simd_or_v(simd_and_v(num, test), simd_andnot_v(campedIndexValue, test));
		}

		simd_type tmp2 = simd_move_hl_v(tmp0, tmp0);
		simd_type tmp3 = simd_cmplt_v(tmp0, tmp2);
		tmp0 = simd_min_v(tmp0, tmp2);
		minClampIndex =
			simd_or_v(simd_and_v(minClampIndex, tmp3), simd_andnot_v(simd_move_hl_v(minClampIndex, minClampIndex), tmp3));
		campedIndexValue =
			simd_or_v(simd_and_v(campedIndexValue, tmp3), simd_andnot_v(simd_move_hl_v(campedIndexValue, campedIndexValue), tmp3));

		tmp2 = simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 1));
		tmp3 = simd_cmplt_s(tmp0, tmp2);
		tmp0 = simd_min_s(tmp0, tmp2);
		minClampIndex =
			simd_or_v(simd_and_v(minClampIndex, tmp3), simd_andnot_v(simd_permut_v(minClampIndex, minClampIndex, PURMUT_MASK(0, 0, 0, 1)), tmp3));
		campedIndexValue =
			simd_or_v(simd_and_v(campedIndexValue, tmp3), simd_andnot_v(simd_permut_v(campedIndexValue, campedIndexValue, PURMUT_MASK(0, 0, 0, 1)), tmp3));

		dgFloat32 ak;
		simd_store_s(tmp0, &ak);
		dgInt32 clampedForceIndex = simd_store_is(minClampIndex);

		dgFloat32 clampedForceIndexValue;
		simd_store_s(campedIndexValue, &clampedForceIndexValue);

		tmp2 = zero;
		tmp0 = simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 0));
		//		if (ak == dgFloat32 (0.0f)) {
		if (ak == dgFloat32(0.0f) && (clampedForceIndex != -1)) {
			_ASSERTE(clampedForceIndex != -1);

			//			akNum = dgFloat32 (0.0f);
			//			accNorm = dgFloat32(0.0f);
			simd_type tmp1 = zero;
			//			tmp2 = zero;
			simd_type tmp3 = zero;
			//			for (k = 0; k < count; k ++) {
			//			for (k = 0; k < roundCount; k ++) {

			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			deltaForce[clampedForceIndex] = dgFloat32(0.0f);
			force[clampedForceIndex] = clampedForceIndexValue;

			for (dgInt32 k = 0; k < roundCount; k += DG_SIMD_WORD_SIZE) {
				// simd_type val_k;
				// simd_type test_0;
				// simd_type test_1;
				// simd_type accel_k;
				// simd_type force_k;

				// bool test0;
				// bool test1;
				// dgFloat32 val;

				simd_type accel_k = (simd_type &)accel[k];
				simd_type force_k = (simd_type &)force[k];
				// val = dgAbsf (lowBound[k] - force[k]);
				simd_type val_k =
					simd_and_v(simd_sub_v((simd_type &)lowBound[k], force_k), signMask);

				// test0 = (val < dgFloat32 (1.0e-5f)) & (accel[k] < dgFloat32 (0.0f));
				simd_type test_0 =
					simd_and_v(simd_cmplt_v(val_k, tol_pos_1eNeg5), simd_cmplt_v(accel_k, zero));

				// val = dgAbsf (highBound[k] - force[k]);
				val_k =
					simd_and_v(simd_sub_v((simd_type &)highBound[k], force_k), signMask);

				// test1 = (val < dgFloat32 (1.0e-5f)) & (accel[k] > dgFloat32 (0.0f));
				simd_type test_1 =
					simd_and_v(simd_cmplt_v(val_k, tol_pos_1eNeg5), simd_cmpgt_v(accel_k, zero));

				// force[k] = test0 ? lowBound[k] : (test1 ? highBound[k] : force[k]);
				// val_k = simd_or_v (simd_and_v ((simd_type&)lowBound[k], test_0), simd_and_v ((simd_type&)highBound[k], test_1));
				//(simd_type&) force[k] = simd_or_v (simd_and_v (val_k, test_2) , simd_andnot_v (force_k, test_2));
				(simd_type &)force[k] =
					simd_min_v((simd_type &)highBound[k], simd_max_v((simd_type &)force[k], (simd_type &)lowBound[k]));

				// activeRow[k] *= (test0 | test1) ? dgFloat32 (0.0f) : dgFloat32 (1.0f);
				// test_2 = simd_or_v (test_0, test_1);
				(simd_type &)activeRow[k] =
					simd_mul_v((simd_type &)activeRow[k], simd_andnot_v(one, simd_or_v(test_0, test_1)));

				// deltaForce[k] = accel[k] * invDJMinvJt[k] * activeRow[k];
				(simd_type &)deltaForce[k] =
					simd_mul_v(accel_k, simd_mul_v((simd_type &)invDJMinvJt[k], (simd_type &)activeRow[k]));

				// akNum += accel[k] * deltaForce[k];
				tmp1 = simd_mul_add_v(tmp1, (simd_type &)deltaForce[k], accel_k);

				// accNorm = GetMax (dgAbsf (accel[k] * activeRow[k]), accNorm);
				tmp2 =
					simd_max_v(tmp2, simd_and_v(simd_mul_v(accel_k, (simd_type &)activeRow[k]), signMask));

				// maxPases += 1;
				tmp3 = simd_add_v(tmp3, (simd_type &)activeRow[k]);
			}
			tmp1 = simd_add_v(tmp1, simd_move_hl_v(tmp1, tmp1));
			tmp1 =
				simd_add_s(tmp1, simd_permut_v(tmp1, tmp1, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp1, &akNum);

			tmp2 = simd_max_v(tmp2, simd_move_hl_v(tmp2, tmp2));
			tmp2 =
				simd_max_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp2, &accNorm);

			i = -1;
			// maxPasses = GetMax (maxPasses - 1, 1);
			tmp3 = simd_add_v(tmp3, simd_move_hl_v(tmp3, tmp3));
			maxPasses =
				simd_store_is(simd_add_s(tmp3, simd_permut_v(tmp3, tmp3, PURMUT_MASK(0, 0, 0, 1))));

		} else if (clampedForceIndex >= 0) {
			// akNum = dgFloat32(0.0f);
			// accNorm = dgFloat32(0.0f);
			tmp1 = zero;
			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			// for (k = 0; k < count; k ++) {
			for (dgInt32 k = 0; k < roundCount; k += DG_SIMD_WORD_SIZE) {
				// m_force[k] += ak * m_deltaForce[k];
				// m_accel[k] -= ak * m_deltaAccel[k];
				(simd_type &)force[k] =
					simd_mul_add_v((simd_type &)force[k], tmp0, (simd_type &)deltaForce[k]);
				(simd_type &)accel[k] =
					simd_mul_sub_v((simd_type &)accel[k], tmp0, (simd_type &)deltaAccel[k]);

				// accNorm = GetMax (dgAbsf (m_accel[k] * activeRow[k]), accNorm);
				tmp2 =
					simd_max_v(tmp2, simd_and_v(simd_mul_v((simd_type &)accel[k], (simd_type &)activeRow[k]), signMask));
				//_ASSERTE (dgCheckFloat(m_force[k]));
				//_ASSERTE (dgCheckFloat(m_accel[k]));

				// m_deltaForce[k] = m_accel[k] * m_invDJMinvJt[k] * activeRow[k];
				(simd_type &)deltaForce[k] =
					simd_mul_v((simd_type &)accel[k], simd_mul_v((simd_type &)invDJMinvJt[k], (simd_type &)activeRow[k]));

				// akNum += m_deltaForce[k] * m_accel[k];
				tmp1 =
					simd_mul_add_v(tmp1, (simd_type &)deltaForce[k], (simd_type &)accel[k]);
			}
			tmp1 = simd_add_v(tmp1, simd_move_hl_v(tmp1, tmp1));
			tmp1 =
				simd_add_s(tmp1, simd_permut_v(tmp1, tmp1, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp1, &akNum);

			tmp2 = simd_max_v(tmp2, simd_move_hl_v(tmp2, tmp2));
			tmp2 =
				simd_max_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp2, &accNorm);

			force[clampedForceIndex] = clampedForceIndexValue;
			i = -1;
			maxPasses = GetMax(maxPasses - 1, 1);

		} else {
			// accNorm = dgFloat32(0.0f);
			// for (k = 0; k < count; k ++) {
			for (dgInt32 k = 0; k < roundCount; k += DG_SIMD_WORD_SIZE) {
				// m_force[k] += ak * m_deltaForce[k];
				// m_accel[k] -= ak * m_deltaAccel[k];
				(simd_type &)force[k] =
					simd_mul_add_v((simd_type &)force[k], tmp0, (simd_type &)deltaForce[k]);
				(simd_type &)accel[k] =
					simd_mul_sub_v((simd_type &)accel[k], tmp0, (simd_type &)deltaAccel[k]);

				// accNorm = GetMax (dgAbsf (m_accel[k] * activeRow[k]), accNorm);
				tmp2 =
					simd_max_v(tmp2, simd_and_v(simd_mul_v((simd_type &)accel[k], (simd_type &)activeRow[k]), signMask));
			}
			tmp2 = simd_max_v(tmp2, simd_move_hl_v(tmp2, tmp2));
			tmp2 =
				simd_max_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp2, &accNorm);

			if (accNorm > maxAccNorm) {

				// akDen = akNum;
				// akNum = dgFloat32(0.0f);
				tmp1 = simd_set1(akNum);
				tmp0 = zero;
				// for (k = 0; k < count; k ++) {
				for (dgInt32 k = 0; k < roundCount; k += DG_SIMD_WORD_SIZE) {
					// m_deltaAccel[k] = m_accel[k] * m_invDJMinvJt[k] * activeRow[k];
					(simd_type &)deltaAccel[k] =
						simd_mul_v((simd_type &)accel[k], simd_mul_v((simd_type &)invDJMinvJt[k], (simd_type &)activeRow[k]));

					// akNum += m_accel[k] * m_deltaAccel[k];
					tmp0 =
						simd_mul_add_v(tmp0, (simd_type &)accel[k], (simd_type &)deltaAccel[k]);
				}

				tmp0 = simd_add_v(tmp0, simd_move_hl_v(tmp0, tmp0));
				tmp0 =
					simd_add_s(tmp0, simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 1)));
				simd_store_s(tmp0, &akNum);

				//_ASSERTE (bk > dgFloat32(0.0f));
				// bk = GetMax (bk, dgFloat32 (1.0e-17f));
				tmp1 = simd_max_s(tmp1, simd_set1(dgFloat32(1.0e-17f)));
				// ak = dgFloat32 (akNum / akDen);
				tmp0 = simd_div_s(tmp0, tmp1);
				tmp0 = simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 0));
				// for (k = 0; k < count; k ++) {
				for (dgInt32 k = 0; k < roundCount; k += DG_SIMD_WORD_SIZE) {
					// m_deltaForce[k] = m_deltaAccel[k] + ak * m_deltaForce[k];
					(simd_type &)deltaForce[k] =
						simd_mul_add_v((simd_type &)deltaAccel[k], tmp0, (simd_type &)deltaForce[k]);
				}
			}
		}
	}

#else
#endif
}

void dgJacobianMemory::CalculateSimpleBodyReactionsForces(
	dgFloat32 maxAccNorm) const {
	dgFloat32 accel[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 activeRow[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 lowBound[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 highBound[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 deltaForce[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 deltaAccel[DG_CONSTRAINT_MAX_ROWS];

	//	dgFloat32* const accel = m_accel;
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	const dgFloat32 *const lowerFriction = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFriction = m_upperBoundFrictionCoefficent;
	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgFloat32 *const coordenateAccel = m_coordenateAccel;
	const dgJointInfo *const constraintArray = m_constraintArray;
	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));

	dgInt32 count = constraintArray[0].m_autoPaircount;
	_ASSERTE(constraintArray[0].m_autoPairstart == 0);

	// ak = dgFloat32 (3.0f) * dgSqrt((body1->m_accel % body1->m_accel) + (body0->m_accel % body0->m_accel)) / dgFloat32 (count);
	//	ak = dgSqrt((body1->m_accel % body1->m_accel) + (body0->m_accel % body0->m_accel));
	// dTrace (("%f\n", ak));

	dgInt32 maxPasses = count;
	for (dgInt32 i = 0; i < count; i++) {

		// k = bilateralForceBounds[i].m_normalIndex;
		dgInt32 k = normalForceIndex[i];
		// force = (k >= 0) ? m_force[k] : dgFloat32 (1.0f);
		_ASSERTE(
			((k < 0) && (force[k] == dgFloat32(1.0f))) || ((k >= 0) && (force[k] >= dgFloat32(0.0f))));
		dgFloat32 val = force[k];
		// if (dgAbsf (val) < dgFloat32 (1.0e-2f)) {
		//	val = ak;
		// }
		// lowBound[i] = val * bilateralForceBounds[i].m_low;
		// highBound[i] = val * bilateralForceBounds[i].m_upper;
		lowBound[i] = val * lowerFriction[i];
		highBound[i] = val * upperFriction[i];

		activeRow[i] = dgFloat32(1.0f);
		if (force[i] < lowBound[i]) {
			maxPasses--;
			force[i] = lowBound[i];
			activeRow[i] = dgFloat32(0.0f);
		} else if (force[i] > highBound[i]) {
			maxPasses--;
			force[i] = highBound[i];
			activeRow[i] = dgFloat32(0.0f);
		}
	}

	dgJacobian y0;
	dgJacobian y1;
	y0.m_linear = zero;
	y0.m_angular = zero;
	y1.m_linear = zero;
	y1.m_angular = zero;
	for (dgInt32 i = 0; i < count; i++) {
		dgFloat32 val = force[i];
		y0.m_linear += Jt[i].m_jacobian_IM0.m_linear.Scale(val);
		y0.m_angular += Jt[i].m_jacobian_IM0.m_angular.Scale(val);
		y1.m_linear += Jt[i].m_jacobian_IM1.m_linear.Scale(val);
		y1.m_angular += Jt[i].m_jacobian_IM1.m_angular.Scale(val);
	}

	dgFloat32 akNum = dgFloat32(0.0f);
	dgFloat32 accNorm = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < count; i++) {
		dgVector acc(JMinv[i].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
		acc += JMinv[i].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
		acc += JMinv[i].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
		acc += JMinv[i].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);
		accel[i] = coordenateAccel[i] - acc.m_x - acc.m_y - acc.m_z - force[i] * diagDamp[i];

		deltaForce[i] = accel[i] * invDJMinvJt[i] * activeRow[i];
		akNum += accel[i] * deltaForce[i];
		accNorm = GetMax(dgAbsf(accel[i] * activeRow[i]), accNorm);
	}

	dgFloat32 clampedForceIndexValue = dgFloat32(0.0f);
	for (dgInt32 i = 0; (i < maxPasses) && (accNorm > maxAccNorm); i++) {

		// MatrixTimeVector (forceRows, m_deltaForce, m_deltaAccel);
		y0.m_linear = zero;
		y0.m_angular = zero;
		y1.m_linear = zero;
		y1.m_angular = zero;
		for (dgInt32 k = 0; k < count; k++) {
			dgFloat32 val = deltaForce[k];
			y0.m_linear += Jt[k].m_jacobian_IM0.m_linear.Scale(val);
			y0.m_angular += Jt[k].m_jacobian_IM0.m_angular.Scale(val);
			y1.m_linear += Jt[k].m_jacobian_IM1.m_linear.Scale(val);
			y1.m_angular += Jt[k].m_jacobian_IM1.m_angular.Scale(val);
		}

		dgFloat32 akDen = dgFloat32(0.0f);
		for (dgInt32 k = 0; k < count; k++) {
			dgVector acc(JMinv[k].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
			acc += JMinv[k].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
			acc += JMinv[k].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
			acc += JMinv[k].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);

			// m_accel[i0] = m_coordenateAccel[i0] - acc.m_x - acc.m_y - acc.m_z - m_force[i0] * m_diagDamp[i0];
			deltaAccel[k] = acc.m_x + acc.m_y + acc.m_z + deltaForce[k] * diagDamp[k];
			akDen += deltaAccel[k] * deltaForce[k];
		}

		_ASSERTE(akDen > dgFloat32(0.0f));
		akDen = GetMax(akDen, dgFloat32(1.0e-16f));
		_ASSERTE(dgAbsf(akDen) >= dgFloat32(1.0e-16f));
		dgFloat32 ak = akNum / akDen;

		dgInt32 clampedForceIndex = -1;
		for (dgInt32 k = 0; k < count; k++) {
			if (activeRow[k]) {
				dgFloat32 val = force[k] + ak * deltaForce[k];
				if (deltaForce[k] < dgFloat32(-1.0e-16f)) {
					if (val < lowBound[k]) {
						ak = GetMax((lowBound[k] - force[k]) / deltaForce[k],
									dgFloat32(0.0f));
						clampedForceIndex = k;
						clampedForceIndexValue = lowBound[k];
						if (ak < dgFloat32(1.0e-8f)) {
							ak = dgFloat32(0.0f);
							break;
						}
					}
				} else if (deltaForce[k] > dgFloat32(1.0e-16f)) {
					if (val >= highBound[k]) {
						ak = GetMax((highBound[k] - force[k]) / deltaForce[k],
									dgFloat32(0.0f));
						;
						clampedForceIndex = k;
						clampedForceIndexValue = highBound[k];
						if (ak < dgFloat32(1.0e-8f)) {
							ak = dgFloat32(0.0f);
							break;
						}
					}
				}
			}
		}

		//		if (ak == dgFloat32 (0.0f)) {
		if (ak == dgFloat32(0.0f) && (clampedForceIndex != -1)) {
			_ASSERTE(clampedForceIndex != -1);
			akNum = dgFloat32(0.0f);
			accNorm = dgFloat32(0.0f);

			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			deltaForce[clampedForceIndex] = dgFloat32(0.0f);
			force[clampedForceIndex] = clampedForceIndexValue;
			for (dgInt32 k = 0; k < count; k++) {
				if (activeRow[k]) {
					dgFloat32 val;
					val = lowBound[k] - force[k];
					if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[k] < dgFloat32(0.0f))) {
						force[k] = lowBound[k];
						activeRow[k] = dgFloat32(0.0f);
						deltaForce[k] = dgFloat32(0.0f);

					} else {
						val = highBound[k] - force[k];
						if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[k] > dgFloat32(0.0f))) {
							force[k] = highBound[k];
							activeRow[k] = dgFloat32(0.0f);
							deltaForce[k] = dgFloat32(0.0f);
						} else {
							_ASSERTE(activeRow[k] > dgFloat32(0.0f));
							deltaForce[k] = accel[k] * invDJMinvJt[k];
							akNum += accel[k] * deltaForce[k];
							accNorm = GetMax(dgAbsf(accel[k]), accNorm);
						}
					}
				}
			}

			i = -1;
			maxPasses = GetMax(maxPasses - 1, 1);

		} else if (clampedForceIndex >= 0) {
			akNum = dgFloat32(0.0f);
			accNorm = dgFloat32(0.0f);
			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			for (dgInt32 k = 0; k < count; k++) {
				force[k] += ak * deltaForce[k];
				accel[k] -= ak * deltaAccel[k];
				accNorm = GetMax(dgAbsf(accel[k] * activeRow[k]), accNorm);
				_ASSERTE(dgCheckFloat(force[k]));
				_ASSERTE(dgCheckFloat(accel[k]));

				deltaForce[k] = accel[k] * invDJMinvJt[k] * activeRow[k];
				akNum += deltaForce[k] * accel[k];
			}
			force[clampedForceIndex] = clampedForceIndexValue;

			i = -1;
			maxPasses = GetMax(maxPasses - 1, 1);

		} else {
			accNorm = dgFloat32(0.0f);
			for (dgInt32 k = 0; k < count; k++) {
				force[k] += ak * deltaForce[k];
				accel[k] -= ak * deltaAccel[k];
				accNorm = GetMax(dgAbsf(accel[k] * activeRow[k]), accNorm);
				_ASSERTE(dgCheckFloat(force[k]));
				_ASSERTE(dgCheckFloat(accel[k]));
			}

			if (accNorm > maxAccNorm) {

				akDen = akNum;
				akNum = dgFloat32(0.0f);
				for (dgInt32 k = 0; k < count; k++) {
					deltaAccel[k] = accel[k] * invDJMinvJt[k] * activeRow[k];
					akNum += accel[k] * deltaAccel[k];
				}

				_ASSERTE(akDen > dgFloat32(0.0f));
				akDen = GetMax(akDen, dgFloat32(1.0e-17f));
				ak = dgFloat32(akNum / akDen);
				for (dgInt32 k = 0; k < count; k++) {
					deltaForce[k] = deltaAccel[k] + ak * deltaForce[k];
				}
			}
		}
	}
}

void dgJacobianMemory::CalculateForcesSimulationModeSimd(
	dgFloat32 maxAccNorm) const {
#ifdef DG_BUILD_SIMD_CODE
	dgInt32 passes;
	dgInt32 prevJoint;
	dgInt32 maxPasses;
	dgInt32 forceRows;
	dgInt32 roundBodyCount;
	dgInt32 totalPassesCount;
	dgFloatSign tmpIndex;

	dgFloat32 akNum;
	dgFloat32 accNorm;
	dgFloat32 *const force = m_force;
	dgFloat32 *const accel = m_accel;
	const dgJacobianPair *const Jt = m_Jt;
	dgFloat32 *const deltaAccel = m_deltaAccel;
	dgFloat32 *const deltaForce = m_deltaForce;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	dgJacobian *const internalForces = m_internalForces;
	dgInt32 *const normalForceIndex = m_normalForceIndex;
	dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const coordenateAccel = m_coordenateAccel;
	dgFloat32 *const lowerForceBound = m_lowerBoundFrictionCoefficent;
	dgFloat32 *const upperForceBound = m_upperBoundFrictionCoefficent;
	simd_type forceStepPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];

	dgFloat32 *const forceStep = (dgFloat32 *)forceStepPtr;

	simd_type one;
	simd_type zero;
	simd_type signMask;
	simd_type tol_pos_1eNeg8;
	simd_type tol_pos_1eNeg16;
	simd_type tol_neg_1eNeg16;

	zero = simd_set1(dgFloat32(0.0f));

	roundBodyCount = m_bodyCount & -4;
	for (dgInt32 i = 0; i < roundBodyCount; i += 4) {
		((simd_type &)internalForces[i + 0].m_linear) = zero;
		((simd_type &)internalForces[i + 0].m_angular) = zero;
		((simd_type &)internalForces[i + 1].m_linear) = zero;
		((simd_type &)internalForces[i + 1].m_angular) = zero;
		((simd_type &)internalForces[i + 2].m_linear) = zero;
		((simd_type &)internalForces[i + 2].m_angular) = zero;
		((simd_type &)internalForces[i + 3].m_linear) = zero;
		((simd_type &)internalForces[i + 3].m_angular) = zero;
	}

	for (dgInt32 i = roundBodyCount; i < m_bodyCount; i++) {
		((simd_type &)internalForces[i].m_linear) = zero;
		((simd_type &)internalForces[i].m_angular) = zero;
	}

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 j;
		dgInt32 m0;
		dgInt32 m1;
		dgInt32 index;
		dgInt32 first;
		dgInt32 count;

		simd_type y0_linear;
		simd_type y0_angular;
		simd_type y1_linear;
		simd_type y1_angular;

		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;
		m0 = constraintArray[i].m_m0;
		m1 = constraintArray[i].m_m1;

		y0_linear = zero;
		y0_angular = zero;
		y1_linear = zero;
		y1_angular = zero;
		for (j = 0; j < count; j++) {
			simd_type tmp0;
			index = first + j;
			// force = m_force[index];
			tmp0 = simd_set1(force[index]);

			// y0.m_linear += m_Jt[index].m_jacobian_IM0.m_linear.Scale (m_force[index]);
			// y0.m_angular += m_Jt[index].m_jacobian_IM0.m_angular.Scale (m_force[index]);
			// y1.m_linear += m_Jt[index].m_jacobian_IM1.m_linear.Scale (m_force[index]);
			// y1.m_angular += m_Jt[index].m_jacobian_IM1.m_angular.Scale (m_force[index]);
			y0_linear =
				simd_mul_add_v(y0_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, tmp0);
			y0_angular =
				simd_mul_add_v(y0_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, tmp0);
			y1_linear =
				simd_mul_add_v(y1_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, tmp0);
			y1_angular =
				simd_mul_add_v(y1_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, tmp0);
		}
		// m_internalForces[j0] = y0;
		// m_internalForces[j1] = y1;
		(simd_type &)internalForces[m0].m_linear =
			simd_add_v((simd_type &)internalForces[m0].m_linear, y0_linear);
		(simd_type &)internalForces[m0].m_angular =
			simd_add_v((simd_type &)internalForces[m0].m_angular, y0_angular);
		(simd_type &)internalForces[m1].m_linear =
			simd_add_v((simd_type &)internalForces[m1].m_linear, y1_linear);
		(simd_type &)internalForces[m1].m_angular =
			simd_add_v((simd_type &)internalForces[m1].m_angular, y1_angular);
	}

	for (dgInt32 i = 0; i < DG_CONSTRAINT_MAX_ROWS; i += (DG_SIMD_WORD_SIZE * 4)) {
		(simd_type &)forceStep[i + DG_SIMD_WORD_SIZE * 0] = zero;
		(simd_type &)forceStep[i + DG_SIMD_WORD_SIZE * 1] = zero;
		(simd_type &)forceStep[i + DG_SIMD_WORD_SIZE * 2] = zero;
		(simd_type &)forceStep[i + DG_SIMD_WORD_SIZE * 3] = zero;
	}

	maxPasses = 4;
	prevJoint = 0;
	accNorm = maxAccNorm * dgFloat32(2.0f);
	for (passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm); passes++) {
		accNorm = dgFloat32(0.0f);
		for (dgInt32 currJoint = 0; currJoint < m_jointCount; currJoint++) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 index;
			//			dgInt32 currJoint;
			dgInt32 rowsCount;
			dgFloat32 jointAccel;
			simd_type y0_linear;
			simd_type y0_angular;
			simd_type y1_linear;
			simd_type y1_angular;

			index = constraintArray[prevJoint].m_autoPairstart;
			rowsCount = constraintArray[prevJoint].m_autoPaircount;
			m0 = constraintArray[prevJoint].m_m0;
			m1 = constraintArray[prevJoint].m_m1;
			y0_linear = zero;
			y0_angular = zero;
			y1_linear = zero;
			y1_angular = zero;

			for (dgInt32 i = 0; i < rowsCount; i++) {
				// dgFloat32 deltaForce;
				// deltaForce = forceStep[i];
				// y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale (deltaForce);
				// y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale (deltaForce);
				// y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale (deltaForce);
				// y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale (deltaForce);

				simd_type deltaForce;
				deltaForce = simd_set1(forceStep[i]);
				y0_linear =
					simd_mul_add_v(y0_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, deltaForce);
				y0_angular =
					simd_mul_add_v(y0_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, deltaForce);
				y1_linear =
					simd_mul_add_v(y1_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, deltaForce);
				y1_angular =
					simd_mul_add_v(y1_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, deltaForce);

				index++;
			}
			(simd_type &)internalForces[m0].m_linear =
				simd_add_v((simd_type &)internalForces[m0].m_linear, y0_linear);
			(simd_type &)internalForces[m0].m_angular =
				simd_add_v((simd_type &)internalForces[m0].m_angular, y0_angular);
			(simd_type &)internalForces[m1].m_linear =
				simd_add_v((simd_type &)internalForces[m1].m_linear, y1_linear);
			(simd_type &)internalForces[m1].m_angular =
				simd_add_v((simd_type &)internalForces[m1].m_angular, y1_angular);

			//			currJoint = jointRemapArray[j];
			index = constraintArray[currJoint].m_autoPairstart;
			rowsCount = constraintArray[currJoint].m_autoPaircount;
			m0 = constraintArray[currJoint].m_m0;
			m1 = constraintArray[currJoint].m_m1;
			// y0 = internalForces[m0];
			// y1 = internalForces[m1];
			y0_linear = (simd_type &)internalForces[m0].m_linear;
			y0_angular = (simd_type &)internalForces[m0].m_angular;
			y1_linear = (simd_type &)internalForces[m1].m_linear;
			y1_angular = (simd_type &)internalForces[m1].m_angular;
			for (dgInt32 i = 0; i < rowsCount; i++) {
				simd_type tmpAccel;

				//				dgVector acc (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
				//				acc += JMinv[index].m_jacobian_IM0.m_angular.CompProduct (y0.m_angular);
				//				acc += JMinv[index].m_jacobian_IM1.m_linear.CompProduct (y1.m_linear);
				//				acc += JMinv[index].m_jacobian_IM1.m_angular.CompProduct (y1.m_angular);

				tmpAccel =
					simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, y0_linear);
				tmpAccel =
					simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, y0_angular);
				tmpAccel =
					simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, y1_linear);
				tmpAccel =
					simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, y1_angular);

				//				accel[i] = coordenateAccel[index] - acc.m_x - acc.m_y - acc.m_z - force[index] * diagDamp[index];
				tmpAccel = simd_add_v(tmpAccel, simd_move_hl_v(tmpAccel, tmpAccel));
				tmpAccel =
					simd_add_s(tmpAccel, simd_permut_v(tmpAccel, tmpAccel, PURMUT_MASK(3, 3, 3, 1)));
				tmpAccel =
					simd_mul_add_s(tmpAccel, simd_load_s(force[index]), simd_load_s(diagDamp[index]));
				simd_store_s(simd_sub_s(simd_load_s(coordenateAccel[index]), tmpAccel),
							 &accel[i]);
				index++;
			}

			jointAccel = CalculateJointForcesSimd(currJoint, forceStep, maxAccNorm);
			accNorm = GetMax(accNorm, jointAccel);
			prevJoint = currJoint;
		}
	}

	one = simd_set1(dgFloat32(1.0f));
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		dgInt32 roundCount;
		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;

		roundCount = count & (-DG_SIMD_WORD_SIZE);
		if (roundCount != count) {
			roundCount += DG_SIMD_WORD_SIZE;
			for (dgInt32 k = count; k < roundCount; k++) {
				dgInt32 j;
				j = first + k;
				// force[j] = dgFloat32 (0.0f);
				// accel[j] -= dgFloat32 (0.0f);;
				// deltaAccel[j] = dgFloat32 (0.0f);
				// deltaForce[j] = dgFloat32 (0.0f);
				normalForceIndex[j] = -1;
				simd_store_s(zero, &force[j]);
				simd_store_s(zero, &accel[j]);
				simd_store_s(zero, &deltaAccel[j]);
				simd_store_s(zero, &deltaForce[j]);
				simd_store_s(one, &lowerForceBound[j]);
				simd_store_s(zero, &upperForceBound[j]);
			}
		}

		for (dgInt32 k = 0; k < count; k += DG_SIMD_WORD_SIZE) {
			simd_type normalForce;

			index = first + k;
			// dgBilateralBounds& forceBounds = bilateralForceBounds[index];
			// j = normalForceIndex[index];
			//_ASSERTE (((j < 0) && (force[j] == dgFloat32 (1.0f))) || ((j >= 0) && (force[j] >= dgFloat32 (0.0f))));
			// val = GetMax (force[j], dgFloat32(0.0f));

			// val = force[j];
			normalForce =
				simd_move_lh_v(simd_pack_lo_v(simd_load_s(force[normalForceIndex[index + 0]]), simd_load_s(force[normalForceIndex[index + 1]])), simd_pack_lo_v(simd_load_s(force[normalForceIndex[index + 2]]), simd_load_s(force[normalForceIndex[index + 3]])));

			// lowerForceBound[index] *= val;
			// upperForceBound[index] *= val;
			(simd_type &)lowerForceBound[index] =
				simd_mul_v(normalForce, (simd_type &)lowerForceBound[index]);
			(simd_type &)upperForceBound[index] =
				simd_mul_v(normalForce, (simd_type &)upperForceBound[index]);

			// force[index] = ClampValue(force[index], lowerForceBound[index], upperForceBound[index]);
			(simd_type &)force[index] =
				simd_min_v((simd_type &)upperForceBound[index], simd_max_v((simd_type &)force[index], (simd_type &)lowerForceBound[index]));
		}
	}

	for (dgInt32 i = 0; i < roundBodyCount; i += 4) {
		(simd_type &)internalForces[i + 0].m_linear = zero;
		(simd_type &)internalForces[i + 0].m_angular = zero;
		(simd_type &)internalForces[i + 1].m_linear = zero;
		(simd_type &)internalForces[i + 1].m_angular = zero;
		(simd_type &)internalForces[i + 2].m_linear = zero;
		(simd_type &)internalForces[i + 2].m_angular = zero;
		(simd_type &)internalForces[i + 3].m_linear = zero;
		(simd_type &)internalForces[i + 3].m_angular = zero;
	}
	for (dgInt32 i = roundBodyCount; i < m_bodyCount; i++) {
		(simd_type &)internalForces[i].m_linear = zero;
		(simd_type &)internalForces[i].m_angular = zero;
	}

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 m0;
		dgInt32 m1;
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		// dgFloat32 val;
		simd_type y0_linear;
		simd_type y0_angular;
		simd_type y1_linear;
		simd_type y1_angular;

		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;
		m0 = constraintArray[i].m_m0;
		m1 = constraintArray[i].m_m1;
		// dgJacobian y0 (internalForces[k0]);
		// dgJacobian y1 (internalForces[k1]);
		y0_linear = zero;
		y0_angular = zero;
		y1_linear = zero;
		y1_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			simd_type tmp0;

			index = j + first;

			// val = force[index];
			tmp0 = simd_set1(force[index]);

			// y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale (val);
			// y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale (val);
			// y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale (val);
			// y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale (val);
			y0_linear =
				simd_mul_add_v(y0_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, tmp0);
			y0_angular =
				simd_mul_add_v(y0_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, tmp0);
			y1_linear =
				simd_mul_add_v(y1_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, tmp0);
			y1_angular =
				simd_mul_add_v(y1_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, tmp0);
		}
		// internalForces[k0] = y0;
		// internalForces[k1] = y1;
		(simd_type &)internalForces[m0].m_linear =
			simd_add_v((simd_type &)internalForces[m0].m_linear, y0_linear);
		(simd_type &)internalForces[m0].m_angular =
			simd_add_v((simd_type &)internalForces[m0].m_angular, y0_angular);
		(simd_type &)internalForces[m1].m_linear =
			simd_add_v((simd_type &)internalForces[m1].m_linear, y1_linear);
		(simd_type &)internalForces[m1].m_angular =
			simd_add_v((simd_type &)internalForces[m1].m_angular, y1_angular);
	}

	forceRows = 0;
	akNum = dgFloat32(0.0f);
	accNorm = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 m0;
		dgInt32 m1;
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		dgInt32 activeCount;

		simd_type y0_linear;
		simd_type y0_angular;
		simd_type y1_linear;
		simd_type y1_angular;
		bool isClamped[DG_CONSTRAINT_MAX_ROWS];

		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;
		m0 = constraintArray[i].m_m0;
		m1 = constraintArray[i].m_m1;

		// const dgJacobian& y0 = internalForces[m0];
		// const dgJacobian& y1 = internalForces[m1];
		y0_linear = (simd_type &)internalForces[m0].m_linear;
		y0_angular = (simd_type &)internalForces[m0].m_angular;
		y1_linear = (simd_type &)internalForces[m1].m_linear;
		y1_angular = (simd_type &)internalForces[m1].m_angular;
		for (dgInt32 j = 0; j < count; j++) {
			simd_type tmp0;
			index = j + first;
			// dgVector tmpAccel (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
			// tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
			// tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
			// tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);

			tmp0 =
				simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, y0_linear);
			tmp0 =
				simd_mul_add_v(tmp0, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, y0_angular);
			tmp0 =
				simd_mul_add_v(tmp0, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, y1_linear);
			tmp0 =
				simd_mul_add_v(tmp0, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, y1_angular);

			// accel[index] = coordenateAccel[index] - (tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z + force[index] * diagDamp[index]);
			tmp0 = simd_add_v(tmp0, simd_move_hl_v(tmp0, tmp0));
			tmp0 =
				simd_add_s(tmp0, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 3, 3, 1)));
			tmp0 =
				simd_sub_s(simd_load_s(coordenateAccel[index]), simd_mul_add_s(tmp0, simd_load_s(force[index]), simd_load_s(diagDamp[index])));
			simd_store_s(tmp0, &accel[index]);
		}

		activeCount = 0;
		for (dgInt32 j = 0; j < count; j++) {
			dgFloat32 val;
			index = j + first;
			val = lowerForceBound[index] - force[index];
			if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[index] < dgFloat32(0.0f))) {
				force[index] = lowerForceBound[index];
				accel[index] = dgFloat32(0.0f);
				deltaForce[index] = dgFloat32(0.0f);
				deltaAccel[index] = dgFloat32(0.0f);

				isClamped[j] = true;
			} else {
				val = upperForceBound[index] - force[index];
				if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[index] > dgFloat32(0.0f))) {
					force[index] = upperForceBound[index];
					accel[index] = dgFloat32(0.0f);
					deltaForce[index] = dgFloat32(0.0f);
					deltaAccel[index] = dgFloat32(0.0f);
					isClamped[j] = true;
				} else {
					forceRows++;
					activeCount++;
					deltaForce[index] = accel[index] * invDJMinvJt[index];
					akNum += accel[index] * deltaForce[index];
					accNorm = GetMax(dgAbsf(accel[index]), accNorm);
					isClamped[j] = false;
				}
			}
		}

		if (activeCount < count) {
			dgInt32 i0;
			dgInt32 i1;

			i0 = 0;
			i1 = count - 1;
			constraintArray[i].m_autoPairActiveCount = activeCount;
			do {
				while ((i0 <= i1) && !isClamped[i0])
					i0++;
				while ((i0 <= i1) && isClamped[i1])
					i1--;
				if (i0 < i1) {
					SwapRowsSimd(first + i0, first + i1);
					i0++;
					i1--;
				}
			} while (i0 < i1);
		}
	}

	//	maxPasses = 0x7fffffff;
	//	signMask = simd_set1((dgFloat32&) maxPasses);
	tmpIndex.m_integer.m_iVal = 0x7fffffff;
	signMask = simd_set1(tmpIndex.m_fVal);
	zero = simd_set1(dgFloat32(0.0f));
	tol_pos_1eNeg8 = simd_set1(dgFloat32(1.0e-8f));
	tol_pos_1eNeg16 = simd_set1(dgFloat32(1.0e-16f));
	tol_neg_1eNeg16 = simd_set1(dgFloat32(-1.0e-16f));

	maxPasses = forceRows;
	totalPassesCount = 0;
	for (passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm); passes++) {

		dgInt32 clampedForceIndex;
		dgInt32 clampedForceJoint;
		//		dgFloat32 ak;
		//		dgFloat32 akDen;
		//		dgFloat32 clampedForceIndexValue;
		simd_type akSimd;
		simd_type akDenSimd;
		simd_type clampedForceIndexValue;

		for (dgInt32 i = 0; i < roundBodyCount; i += 4) {
			(simd_type &)internalForces[i + 0].m_linear = zero;
			(simd_type &)internalForces[i + 0].m_angular = zero;
			(simd_type &)internalForces[i + 1].m_linear = zero;
			(simd_type &)internalForces[i + 1].m_angular = zero;
			(simd_type &)internalForces[i + 2].m_linear = zero;
			(simd_type &)internalForces[i + 2].m_angular = zero;
			(simd_type &)internalForces[i + 3].m_linear = zero;
			(simd_type &)internalForces[i + 3].m_angular = zero;
		}
		for (dgInt32 i = roundBodyCount; i < m_bodyCount; i++) {
			(simd_type &)internalForces[i].m_linear = zero;
			(simd_type &)internalForces[i].m_angular = zero;
		}

		for (dgInt32 i = 0; i < m_jointCount; i++) {
			dgInt32 j;
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 first;
			dgInt32 count;
			dgInt32 index;
			// dgFloat32 ak;
			simd_type y0_linear;
			simd_type y0_angular;
			simd_type y1_linear;
			simd_type y1_angular;

			first = constraintArray[i].m_autoPairstart;
			count = constraintArray[i].m_autoPairActiveCount;
			m0 = constraintArray[i].m_m0;
			m1 = constraintArray[i].m_m1;

			// dgJacobian y0 (internalForces[m0]);
			// dgJacobian y1 (internalForces[m1]);
			y0_linear = zero;
			y0_angular = zero;
			y1_linear = zero;
			y1_angular = zero;

			for (j = 0; j < count; j++) {
				simd_type tmp0;
				index = j + first;
				// ak = deltaForce[index];
				tmp0 = simd_set1(deltaForce[index]);

				// y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale (ak);
				// y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale (ak);
				// y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale (ak);
				// y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale (ak);
				y0_linear =
					simd_mul_add_v(y0_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, tmp0);
				y0_angular =
					simd_mul_add_v(y0_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, tmp0);
				y1_linear =
					simd_mul_add_v(y1_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, tmp0);
				y1_angular =
					simd_mul_add_v(y1_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, tmp0);
			}
			// internalForces[m0] = y0;
			// internalForces[m1] = y1;
			(simd_type &)internalForces[m0].m_linear =
				simd_add_v((simd_type &)internalForces[m0].m_linear, y0_linear);
			(simd_type &)internalForces[m0].m_angular =
				simd_add_v((simd_type &)internalForces[m0].m_angular, y0_angular);
			(simd_type &)internalForces[m1].m_linear =
				simd_add_v((simd_type &)internalForces[m1].m_linear, y1_linear);
			(simd_type &)internalForces[m1].m_angular =
				simd_add_v((simd_type &)internalForces[m1].m_angular, y1_angular);
		}

		akDenSimd = zero;
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			dgInt32 j;
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 first;
			dgInt32 count;
			dgInt32 index;

			simd_type y0_linear;
			simd_type y0_angular;
			simd_type y1_linear;
			simd_type y1_angular;

			first = constraintArray[i].m_autoPairstart;
			count = constraintArray[i].m_autoPairActiveCount;
			m0 = constraintArray[i].m_m0;
			m1 = constraintArray[i].m_m1;
			// const dgJacobian& y0 = internalForces[k0];
			// const dgJacobian& y1 = internalForces[k1];

			y0_linear = (simd_type &)internalForces[m0].m_linear;
			y0_angular = (simd_type &)internalForces[m0].m_angular;
			y1_linear = (simd_type &)internalForces[m1].m_linear;
			y1_angular = (simd_type &)internalForces[m1].m_angular;
			for (j = 0; j < count; j++) {
				simd_type tmp1;
				simd_type tmp2;
				index = j + first;

				// dgVector tmpAccel (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
				// tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
				// tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
				// tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);

				tmp2 =
					simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, y0_linear);
				tmp2 =
					simd_mul_add_v(tmp2, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, y0_angular);
				tmp2 =
					simd_mul_add_v(tmp2, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, y1_linear);
				tmp2 =
					simd_mul_add_v(tmp2, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, y1_angular);

				// deltaAccel[index] = tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z + deltaForce[index] * diagDamp[index];
				tmp1 = simd_load_s(deltaForce[index]);
				tmp2 = simd_add_v(tmp2, simd_move_hl_v(tmp2, tmp2));
				tmp2 =
					simd_add_s(tmp2, simd_permut_v(tmp2, tmp2, PURMUT_MASK(3, 3, 3, 1)));
				tmp2 = simd_mul_add_s(tmp2, tmp1, simd_load_s(diagDamp[index]));
				simd_store_s(tmp2, &deltaAccel[index]);

				// akDen += deltaAccel[index] * deltaForce[index];
				akDenSimd = simd_mul_add_s(akDenSimd, tmp2, tmp1);
			}
		}

		//		_ASSERTE (akDen > dgFloat32 (0.0f));
		//		akDen = GetMax (akDen, dgFloat32(1.0e-16f));
		//		_ASSERTE (dgAbsf (akDen) >= dgFloat32(1.0e-16f));
		//		ak = akNum / akDen;

		akSimd =
			simd_div_s(simd_load_s(akNum), simd_max_s(akDenSimd, tol_pos_1eNeg16));

		clampedForceIndex = -1;
		clampedForceJoint = -1;
		clampedForceIndexValue = zero;
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			dgInt32 j;
			dgInt32 first;
			dgInt32 count;
			dgInt32 index;

			if (simd_store_is(simd_cmpgt_s(akSimd, tol_pos_1eNeg8))) {
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				for (j = 0; j < count; j++) {
					simd_type tmp1;
					simd_type test0;
					simd_type test1;

					index = j + first;
					// tmp = force[index] + ak * deltaForce[index];

					simd_type force_index;
					simd_type delta_force;
					simd_type low_force;
					simd_type high_force;

					// Make sure AK is not negative
					force_index = simd_set1(force[index]);
					delta_force = simd_set1(deltaForce[index]);
					low_force = simd_set1(lowerForceBound[index]);
					high_force = simd_set1(upperForceBound[index]);

					tmp1 = simd_mul_add_s(force_index, akSimd, delta_force);

					// test0 = (deltaForce[index] < dgFloat32 (-1.0e-16f)) & (tmp < lowerForceBound[index]);
					test0 =
						simd_and_v(simd_cmplt_s(delta_force, tol_neg_1eNeg16), simd_cmplt_s(tmp1, low_force));

					// test1 = (deltaForce[index] > dgFloat32 ( 1.0e-16f)) & (tmp > upperForceBound[index]);
					test1 =
						simd_and_v(simd_cmpgt_s(delta_force, tol_pos_1eNeg16), simd_cmpgt_s(tmp1, high_force));

					// test =  test0 | test1;
					if (simd_store_is(simd_or_v(test0, test1))) {
						// clampedForceIndexValue = lowerForceBound[index];
						clampedForceIndexValue =
							simd_or_v(simd_and_v(low_force, test0), simd_and_v(high_force, test1));

						akSimd =
							simd_div_s(simd_sub_s(clampedForceIndexValue, force_index), delta_force);

						// clampedForceIndex = test ? j : clampedForceIndex;
						// clampedForceJoint = test ? i : clampedForceJoint;
						clampedForceIndex = j;
						clampedForceJoint = i;
					}
				}
			}
		}

		akSimd = simd_permut_v(akSimd, akSimd, PURMUT_MASK(0, 0, 0, 0));
		if (clampedForceIndex >= 0) {
			dgInt32 first;
			dgInt32 count;
			dgInt32 activeCount;
			simd_type tmp0;
			simd_type tmp1;
			bool isClamped[DG_CONSTRAINT_MAX_ROWS];

			for (dgInt32 i = 0; i < m_jointCount; i++) {
				dgInt32 j;
				dgInt32 first;
				dgInt32 count;
				dgInt32 index;
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				for (j = 0; j < count; j += DG_SIMD_WORD_SIZE) {
					index = j + first;
					// force[index] += ak * deltaForce[index];
					// accel[index] -= ak * deltaAccel[index];
					(simd_type &)force[index] =
						simd_mul_add_v((simd_type &)force[index], akSimd, (simd_type &)deltaForce[index]);
					(simd_type &)accel[index] =
						simd_mul_sub_v((simd_type &)accel[index], akSimd, (simd_type &)deltaAccel[index]);
				}
			}

			first = constraintArray[clampedForceJoint].m_autoPairstart;
			count = constraintArray[clampedForceJoint].m_autoPairActiveCount;
			count--;
			// force[first + clampedForceIndex] = clampedForceIndexValue;
			simd_store_s(clampedForceIndexValue, &force[first + clampedForceIndex]);
			accel[first + clampedForceIndex] = dgFloat32(0.0f);
			deltaForce[first + clampedForceIndex] = dgFloat32(0.0f);
			deltaAccel[first + clampedForceIndex] = dgFloat32(0.0f);
			if (clampedForceIndex != count) {
				SwapRowsSimd(first + clampedForceIndex, first + count);
			}

			activeCount = count;
			for (dgInt32 i = 0; i < count; i++) {
				dgInt32 index;
				dgFloat32 val;
				index = first + i;
				isClamped[i] = false;
				val = lowerForceBound[index] - force[index];
				if ((val > dgFloat32(-1.0e-5f)) && (accel[index] < dgFloat32(0.0f))) {
					activeCount--;
					isClamped[i] = true;
					accel[index] = dgFloat32(0.0f);
					deltaForce[index] = dgFloat32(0.0f);
					deltaAccel[index] = dgFloat32(0.0f);

				} else {
					val = upperForceBound[index] - force[index];
					if ((val < dgFloat32(1.0e-5f)) && (accel[index] > dgFloat32(0.0f))) {
						activeCount--;
						isClamped[i] = true;
						accel[index] = dgFloat32(0.0f);
						deltaForce[index] = dgFloat32(0.0f);
						deltaAccel[index] = dgFloat32(0.0f);
					}
				}
			}

			if (activeCount < count) {
				dgInt32 i0;
				dgInt32 i1;
				i0 = 0;
				i1 = count - 1;
				do {
					while ((i0 <= i1) && !isClamped[i0])
						i0++;
					while ((i0 <= i1) && isClamped[i1])
						i1--;
					if (i0 < i1) {
						SwapRowsSimd(first + i0, first + i1);
						i0++;
						i1--;
					}
				} while (i0 < i1);
			}
			constraintArray[clampedForceJoint].m_autoPairActiveCount = activeCount;

			forceRows = 0;
			// akNum = dgFloat32 (0.0f);
			// accNorm = dgFloat32(0.0f);

			tmp0 = zero;
			tmp1 = zero;
			for (dgInt32 i = 0; i < m_jointCount; i++) {
				dgInt32 j;
				dgInt32 first;
				dgInt32 count;
				dgInt32 index;
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				forceRows += count;
				//				for (j = 0; j < count; j ++) {
				for (j = 0; j < count; j += DG_SIMD_WORD_SIZE) {
					index = first + j;
					//_ASSERTE ((i != clampedForceJoint) || !((dgAbsf (lowerForceBound[index] - force[index]) < dgFloat32 (1.0e-5f)) && (accel[index] < dgFloat32 (0.0f))));
					//_ASSERTE ((i != clampedForceJoint) || !((dgAbsf (upperForceBound[index] - force[index]) < dgFloat32 (1.0e-5f)) && (accel[index] > dgFloat32 (0.0f))));
					// deltaForce[index] = accel[index] * invDJMinvJt[index];
					(simd_type &)deltaForce[index] =
						simd_mul_v((simd_type &)accel[index], (simd_type &)invDJMinvJt[index]);

					// akNum += deltaForce[index] * accel[index];
					tmp0 =
						simd_mul_add_v(tmp0, (simd_type &)accel[index], (simd_type &)deltaForce[index]);

					// accNorm = GetMax (dgAbsf (accel[index]), accNorm);
					tmp1 =
						simd_max_v(tmp1, simd_and_v((simd_type &)accel[index], signMask));
					//_ASSERTE (dgCheckFloat(deltaForce[index]));
				}
			}

			tmp0 = simd_add_v(tmp0, simd_move_hl_v(tmp0, tmp0));
			tmp0 =
				simd_add_s(tmp0, simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp0, &akNum);

			tmp1 = simd_max_v(tmp1, simd_move_hl_v(tmp1, tmp1));
			tmp1 =
				simd_max_s(tmp1, simd_permut_v(tmp1, tmp1, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp1, &accNorm);

			_ASSERTE(akNum >= dgFloat32(0.0f));
			passes = -1;
			maxPasses = forceRows;

		} else {
			simd_type tmp1;

			// accNorm = dgFloat32(0.0f);
			tmp1 = zero;
			for (dgInt32 i = 0; i < m_jointCount; i++) {
				dgInt32 first;
				dgInt32 count;
				dgInt32 index;
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				for (dgInt32 j = 0; j < count; j += DG_SIMD_WORD_SIZE) {
					index = j + first;
					// force[index] += ak * deltaForce[index];
					// accel[index] -= ak * deltaAccel[index];
					(simd_type &)force[index] =
						simd_mul_add_v((simd_type &)force[index], akSimd, (simd_type &)deltaForce[index]);
					(simd_type &)accel[index] =
						simd_mul_sub_v((simd_type &)accel[index], akSimd, (simd_type &)deltaAccel[index]);

					// accNorm = GetMax (dgAbsf (accel[index]), accNorm);
					tmp1 =
						simd_max_v(tmp1, simd_and_v((simd_type &)accel[index], signMask));
				}
			}

			tmp1 = simd_max_v(tmp1, simd_move_hl_v(tmp1, tmp1));
			tmp1 =
				simd_max_s(tmp1, simd_permut_v(tmp1, tmp1, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(tmp1, &accNorm);

			if (accNorm > maxAccNorm) {
				simd_type tmp0;
				// simd_type akNum;
				// akDen = akNum;
				// akNum = dgFloat32(0.0f);

				tmp0 = zero;
				akDenSimd = simd_set1(akNum);
				for (dgInt32 i = 0; i < m_jointCount; i++) {
					dgInt32 first;
					dgInt32 count;
					dgInt32 index;
					first = constraintArray[i].m_autoPairstart;
					count = constraintArray[i].m_autoPairActiveCount;
					for (dgInt32 j = 0; j < count; j += DG_SIMD_WORD_SIZE) {
						index = j + first;
						// deltaAccel[index] = accel[index] * invDJMinvJt[index];
						(simd_type &)deltaAccel[index] =
							simd_mul_v((simd_type &)accel[index], (simd_type &)invDJMinvJt[index]);

						// akNum += accel[index] * deltaAccel[index];
						tmp0 =
							simd_mul_add_v(tmp0, (simd_type &)accel[index], (simd_type &)deltaAccel[index]);
					}
				}

				tmp0 = simd_add_v(tmp0, simd_move_hl_v(tmp0, tmp0));
				tmp0 =
					simd_add_s(tmp0, simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 1)));
				simd_store_s(tmp0, &akNum);

				//				_ASSERTE (akNum >= dgFloat32 (0.0f));
				//				_ASSERTE (akDen > dgFloat32(0.0f));
				//				akDen = GetMax (akDen, dgFloat32 (1.0e-17f));
				//				ak = dgFloat32 (akNum / akDen);

				tmp0 =
					simd_div_s(tmp0, simd_max_s(akDenSimd, simd_set1(dgFloat32(1.0e-17f))));
				tmp0 = simd_permut_v(tmp0, tmp0, PURMUT_MASK(0, 0, 0, 0));
				for (dgInt32 i = 0; i < m_jointCount; i++) {
					dgInt32 first;
					dgInt32 count;
					dgInt32 index;
					first = constraintArray[i].m_autoPairstart;
					count = constraintArray[i].m_autoPairActiveCount;
					for (dgInt32 j = 0; j < count; j += DG_SIMD_WORD_SIZE) {
						index = j + first;
						// deltaForce[index] = deltaAccel[index] + ak * deltaForce[index];
						(simd_type &)deltaForce[index] =
							simd_mul_add_v((simd_type &)deltaAccel[index], tmp0, (simd_type &)deltaForce[index]);
					}
				}
			}
		}
		totalPassesCount++;
	}
	ApplyExternalForcesAndAccelerationSimd(maxAccNorm);
#endif
}

void dgJacobianMemory::CalculateForcesSimulationMode(dgFloat32 maxAccNorm) const {
	dgInt32 passes;
	dgInt32 prevJoint;
	dgInt32 maxPasses;
	dgInt32 forceRows;
	dgInt32 totalPassesCount;
	dgFloat32 akNum;
	dgFloat32 accNorm;
	dgFloat32 *const force = m_force;
	dgFloat32 *const accel = m_accel;
	const dgJacobianPair *const Jt = m_Jt;
	dgFloat32 *const deltaAccel = m_deltaAccel;
	dgFloat32 *const deltaForce = m_deltaForce;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;

	dgJacobian *const internalForces = m_internalForces;
	dgInt32 *const normalForceIndex = m_normalForceIndex;
	dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const coordenateAccel = m_coordenateAccel;
	dgFloat32 *const lowerForceBound = m_lowerBoundFrictionCoefficent;
	dgFloat32 *const upperForceBound = m_upperBoundFrictionCoefficent;
	dgFloat32 forceStep[DG_CONSTRAINT_MAX_ROWS];

	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));

	// initialize the intermediate force accumulation to zero
	for (dgInt32 i = 0; i < m_bodyCount; i++) {
		internalForces[i].m_linear = zero;
		internalForces[i].m_angular = zero;
	}

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 m0;
		dgInt32 m1;
		dgInt32 index;
		dgInt32 first;
		dgInt32 count;
		dgFloat32 val;
		dgJacobian y0;
		dgJacobian y1;

		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;
		m0 = constraintArray[i].m_m0;
		m1 = constraintArray[i].m_m1;

		y0.m_linear = zero;
		y0.m_angular = zero;
		y1.m_linear = zero;
		y1.m_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			index = first + j;
			val = force[index];
			y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale(val);
			y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale(val);
			y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale(val);
			y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale(val);
		}
		internalForces[m0].m_linear += y0.m_linear;
		internalForces[m0].m_angular += y0.m_angular;
		internalForces[m1].m_linear += y1.m_linear;
		internalForces[m1].m_angular += y1.m_angular;
	}

	for (dgInt32 i = 0; i < DG_CONSTRAINT_MAX_ROWS; i++) {
		forceStep[i] = dgFloat32(0.0f);
	}

	maxPasses = 4;
	prevJoint = 0;
	accNorm = maxAccNorm * dgFloat32(2.0f);
	for (passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm); passes++) {
		accNorm = dgFloat32(0.0f);
		for (dgInt32 currJoint = 0; currJoint < m_jointCount; currJoint++) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 index;
			dgInt32 rowsCount;
			dgFloat32 jointAccel;
			dgJacobian y0;
			dgJacobian y1;

			index = constraintArray[prevJoint].m_autoPairstart;
			rowsCount = constraintArray[prevJoint].m_autoPaircount;
			m0 = constraintArray[prevJoint].m_m0;
			m1 = constraintArray[prevJoint].m_m1;

			y0.m_linear = zero;
			y0.m_angular = zero;
			y1.m_linear = zero;
			y1.m_angular = zero;
			for (dgInt32 i = 0; i < rowsCount; i++) {
				dgFloat32 deltaForce;
				deltaForce = forceStep[i];
				y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale(deltaForce);
				y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale(deltaForce);
				y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale(deltaForce);
				y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale(deltaForce);
				index++;
			}
			internalForces[m0].m_linear += y0.m_linear;
			internalForces[m0].m_angular += y0.m_angular;
			internalForces[m1].m_linear += y1.m_linear;
			internalForces[m1].m_angular += y1.m_angular;

			index = constraintArray[currJoint].m_autoPairstart;
			rowsCount = constraintArray[currJoint].m_autoPaircount;
			m0 = constraintArray[currJoint].m_m0;
			m1 = constraintArray[currJoint].m_m1;
			y0 = internalForces[m0];
			y1 = internalForces[m1];
			for (dgInt32 i = 0; i < rowsCount; i++) {
				dgVector acc(
					JMinv[index].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
				acc += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
				acc += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
				acc += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);

				accel[i] = coordenateAccel[index] - acc.m_x - acc.m_y - acc.m_z - force[index] * diagDamp[index];
				index++;
			}

			jointAccel = CalculateJointForces(currJoint, forceStep, maxAccNorm);
			accNorm = GetMax(accNorm, jointAccel);
			prevJoint = currJoint;
		}
	}

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 j;
		dgInt32 k;
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		dgFloat32 val;
		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPaircount;
		constraintArray[i].m_autoPaircount = count;
		for (k = 0; k < count; k++) {
			index = first + k;
			j = normalForceIndex[index];
			_ASSERTE(((j < 0) && (force[j] == dgFloat32(1.0f))) || ((j >= 0) && (force[j] >= dgFloat32(0.0f))));
			val = force[j];
			lowerForceBound[index] *= val;
			upperForceBound[index] *= val;

			val = force[index];
			force[index] = ClampValue(val, lowerForceBound[index], upperForceBound[index]);
		}
	}

	for (dgInt32 i = 0; i < m_bodyCount; i++) {
		internalForces[i].m_linear[0] = dgFloat32(0.0f);
		internalForces[i].m_linear[1] = dgFloat32(0.0f);
		internalForces[i].m_linear[2] = dgFloat32(0.0f);
		internalForces[i].m_linear[3] = dgFloat32(0.0f);
		internalForces[i].m_angular[0] = dgFloat32(0.0f);
		internalForces[i].m_angular[1] = dgFloat32(0.0f);
		internalForces[i].m_angular[2] = dgFloat32(0.0f);
		internalForces[i].m_angular[3] = dgFloat32(0.0f);
	}

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 j;
		dgInt32 m0;
		dgInt32 m1;
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		dgJacobian y0;
		dgJacobian y1;

		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;
		m0 = constraintArray[i].m_m0;
		m1 = constraintArray[i].m_m1;
		y0.m_linear = zero;
		y0.m_angular = zero;
		y1.m_linear = zero;
		y1.m_angular = zero;
		for (j = 0; j < count; j++) {
			dgFloat32 val;
			index = j + first;
			val = force[index];
			y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale(val);
			y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale(val);
			y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale(val);
			y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale(val);
		}
		internalForces[m0].m_linear += y0.m_linear;
		internalForces[m0].m_angular += y0.m_angular;
		internalForces[m1].m_linear += y1.m_linear;
		internalForces[m1].m_angular += y1.m_angular;
	}

	forceRows = 0;
	akNum = dgFloat32(0.0f);
	accNorm = dgFloat32(0.0f);
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 m0;
		dgInt32 m1;
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		dgInt32 activeCount;
		bool isClamped[DG_CONSTRAINT_MAX_ROWS];

		first = constraintArray[i].m_autoPairstart;
		count = constraintArray[i].m_autoPairActiveCount;
		m0 = constraintArray[i].m_m0;
		m1 = constraintArray[i].m_m1;

		const dgJacobian &y0 = internalForces[m0];
		const dgJacobian &y1 = internalForces[m1];
		for (dgInt32 j = 0; j < count; j++) {
			index = j + first;
			dgVector tmpAccel(JMinv[index].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
			tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
			tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
			tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);
			accel[index] = coordenateAccel[index] - (tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z + force[index] * diagDamp[index]);
		}

		activeCount = 0;
		for (dgInt32 j = 0; j < count; j++) {
			dgFloat32 val;
			index = j + first;
			val = lowerForceBound[index] - force[index];
			if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[index] < dgFloat32(0.0f))) {
				force[index] = lowerForceBound[index];
				isClamped[j] = true;
			} else {
				val = upperForceBound[index] - force[index];
				if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[index] > dgFloat32(0.0f))) {
					force[index] = upperForceBound[index];
					isClamped[j] = true;
				} else {
					forceRows++;
					activeCount++;
					deltaForce[index] = accel[index] * invDJMinvJt[index];
					akNum += accel[index] * deltaForce[index];
					accNorm = GetMax(dgAbsf(accel[index]), accNorm);
					isClamped[j] = false;
				}
			}
		}

		if (activeCount < count) {
			dgInt32 i0;
			dgInt32 i1;

			i0 = 0;
			i1 = count - 1;
			constraintArray[i].m_autoPairActiveCount = activeCount;
			do {
				while ((i0 <= i1) && !isClamped[i0])
					i0++;
				while ((i0 <= i1) && isClamped[i1])
					i1--;
				if (i0 < i1) {
					SwapRows(first + i0, first + i1);
					i0++;
					i1--;
				}
			} while (i0 < i1);
		}
	}

	maxPasses = forceRows;
	totalPassesCount = 0;
	for (passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm); passes++) {
		dgInt32 clampedForceIndex;
		dgInt32 clampedForceJoint;
		dgFloat32 ak;
		dgFloat32 akDen;
		dgFloat32 clampedForceIndexValue;

		for (dgInt32 i = 0; i < m_bodyCount; i++) {
			internalForces[i].m_linear[0] = dgFloat32(0.0f);
			internalForces[i].m_linear[1] = dgFloat32(0.0f);
			internalForces[i].m_linear[2] = dgFloat32(0.0f);
			internalForces[i].m_linear[3] = dgFloat32(0.0f);
			internalForces[i].m_angular[0] = dgFloat32(0.0f);
			internalForces[i].m_angular[1] = dgFloat32(0.0f);
			internalForces[i].m_angular[2] = dgFloat32(0.0f);
			internalForces[i].m_angular[3] = dgFloat32(0.0f);
		}

		for (dgInt32 i = 0; i < m_jointCount; i++) {
			dgInt32 j;
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 first;
			dgInt32 count;
			dgInt32 index;
			dgFloat32 ak;
			dgJacobian y0;
			dgJacobian y1;

			first = constraintArray[i].m_autoPairstart;
			count = constraintArray[i].m_autoPairActiveCount;
			m0 = constraintArray[i].m_m0;
			m1 = constraintArray[i].m_m1;
			y0.m_linear = zero;
			y0.m_angular = zero;
			y1.m_linear = zero;
			y1.m_angular = zero;
			for (j = 0; j < count; j++) {
				index = j + first;
				ak = deltaForce[index];
				y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale(ak);
				y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale(ak);
				y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale(ak);
				y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale(ak);
			}
			internalForces[m0].m_linear += y0.m_linear;
			internalForces[m0].m_angular += y0.m_angular;
			internalForces[m1].m_linear += y1.m_linear;
			internalForces[m1].m_angular += y1.m_angular;
		}

		akDen = dgFloat32(0.0f);
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			dgInt32 j;
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 first;
			dgInt32 count;
			dgInt32 index;
			first = constraintArray[i].m_autoPairstart;
			count = constraintArray[i].m_autoPairActiveCount;
			m0 = constraintArray[i].m_m0;
			m1 = constraintArray[i].m_m1;
			const dgJacobian &y0 = internalForces[m0];
			const dgJacobian &y1 = internalForces[m1];
			for (j = 0; j < count; j++) {
				index = j + first;
				dgVector tmpAccel(JMinv[index].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
				tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
				tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
				tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);
				deltaAccel[index] = tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z + deltaForce[index] * diagDamp[index];
				akDen += deltaAccel[index] * deltaForce[index];
			}
		}

		_ASSERTE(akDen > dgFloat32(0.0f));
		akDen = GetMax(akDen, dgFloat32(1.0e-16f));
		_ASSERTE(dgAbsf(akDen) >= dgFloat32(1.0e-16f));
		ak = akNum / akDen;
		clampedForceIndex = -1;
		clampedForceJoint = -1;
		clampedForceIndexValue = dgFloat32(0.0f);
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			dgInt32 j;
			dgInt32 first;
			dgInt32 count;
			dgInt32 index;
			dgFloat32 val;
			if (ak > dgFloat32(1.0e-8f)) {
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				for (j = 0; j < count; j++) {
					index = j + first;
					val = force[index] + ak * deltaForce[index];
					if (deltaForce[index] < dgFloat32(-1.0e-16f)) {
						// if (val < bilateralForceBounds[index].m_low) {
						if (val < lowerForceBound[index]) {
							ak = GetMax((lowerForceBound[index] - force[index]) / deltaForce[index], dgFloat32(0.0f));
							_ASSERTE(ak >= dgFloat32(0.0f));
							clampedForceIndex = j;
							clampedForceJoint = i;
							// clampedForceIndexValue = bilateralForceBounds[index].m_low;
							clampedForceIndexValue = lowerForceBound[index];
						}
					} else if (deltaForce[index] > dgFloat32(1.0e-16f)) {
						// if (val > bilateralForceBounds[index].m_upper) {
						if (val > upperForceBound[index]) {
							ak = GetMax((upperForceBound[index] - force[index]) / deltaForce[index], dgFloat32(0.0f));
							_ASSERTE(ak >= dgFloat32(0.0f));
							clampedForceIndex = j;
							clampedForceJoint = i;
							clampedForceIndexValue = upperForceBound[index];
						}
					}
				}
			}
		}

		if (clampedForceIndex >= 0) {
			dgInt32 first;
			dgInt32 count;
			dgInt32 activeCount;
			bool isClamped[DG_CONSTRAINT_MAX_ROWS];

			for (dgInt32 i = 0; i < m_jointCount; i++) {
				dgInt32 j;
				dgInt32 first;
				dgInt32 count;
				dgInt32 index;
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				for (j = 0; j < count; j++) {
					index = j + first;
					force[index] += ak * deltaForce[index];
					accel[index] -= ak * deltaAccel[index];
				}
			}

			first = constraintArray[clampedForceJoint].m_autoPairstart;
			count = constraintArray[clampedForceJoint].m_autoPairActiveCount;
			count--;
			force[first + clampedForceIndex] = clampedForceIndexValue;
			if (clampedForceIndex != count) {
				SwapRows(first + clampedForceIndex, first + count);
			}

			activeCount = count;
			for (dgInt32 i = 0; i < count; i++) {
				dgInt32 index;
				dgFloat32 val;
				index = first + i;
				isClamped[i] = false;
				val = lowerForceBound[index] - force[index];
				if ((val > dgFloat32(-1.0e-5f)) && (accel[index] < dgFloat32(0.0f))) {
					activeCount--;
					isClamped[i] = true;
				} else {
					val = upperForceBound[index] - force[index];
					if ((val < dgFloat32(1.0e-5f)) && (accel[index] > dgFloat32(0.0f))) {
						activeCount--;
						isClamped[i] = true;
					}
				}
			}

			if (activeCount < count) {
				dgInt32 i0;
				dgInt32 i1;
				i0 = 0;
				i1 = count - 1;
				do {
					while ((i0 <= i1) && !isClamped[i0])
						i0++;
					while ((i0 <= i1) && isClamped[i1])
						i1--;
					if (i0 < i1) {
						SwapRows(first + i0, first + i1);
						// Swap (isClamped[i0], isClamped[i1]);
						// Swap (permutationIndex[i0], permutationIndex[i1]);
						i0++;
						i1--;
					}
				} while (i0 < i1);
			}
			constraintArray[clampedForceJoint].m_autoPairActiveCount = activeCount;

			forceRows = 0;
			akNum = dgFloat32(0.0f);
			accNorm = dgFloat32(0.0f);
			for (dgInt32 i = 0; i < m_jointCount; i++) {
				dgInt32 j;
				dgInt32 first;
				dgInt32 count;
				dgInt32 index;
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				forceRows += count;
				for (j = 0; j < count; j++) {
					index = first + j;
					_ASSERTE((i != clampedForceJoint) || !((dgAbsf(lowerForceBound[index] - force[index]) < dgFloat32(1.0e-5f)) && (accel[index] < dgFloat32(0.0f))));
					_ASSERTE((i != clampedForceJoint) || !((dgAbsf(upperForceBound[index] - force[index]) < dgFloat32(1.0e-5f)) && (accel[index] > dgFloat32(0.0f))));
					deltaForce[index] = accel[index] * invDJMinvJt[index];
					akNum += deltaForce[index] * accel[index];
					accNorm = GetMax(dgAbsf(accel[index]), accNorm);
					_ASSERTE(dgCheckFloat(deltaForce[index]));
				}
			}

			_ASSERTE(akNum >= dgFloat32(0.0f));
			passes = -1;
			maxPasses = forceRows;

		} else {
			accNorm = dgFloat32(0.0f);
			for (dgInt32 i = 0; i < m_jointCount; i++) {
				dgInt32 j;
				dgInt32 first;
				dgInt32 count;
				dgInt32 index;
				first = constraintArray[i].m_autoPairstart;
				count = constraintArray[i].m_autoPairActiveCount;
				for (j = 0; j < count; j++) {
					index = j + first;
					force[index] += ak * deltaForce[index];
					accel[index] -= ak * deltaAccel[index];
					accNorm = GetMax(dgAbsf(accel[index]), accNorm);
				}
			}

			if (accNorm > maxAccNorm) {
				akDen = akNum;
				akNum = dgFloat32(0.0f);
				for (dgInt32 i = 0; i < m_jointCount; i++) {
					dgInt32 j;
					dgInt32 first;
					dgInt32 count;
					dgInt32 index;
					first = constraintArray[i].m_autoPairstart;
					count = constraintArray[i].m_autoPairActiveCount;
					for (j = 0; j < count; j++) {
						index = j + first;
						deltaAccel[index] = accel[index] * invDJMinvJt[index];
						akNum += accel[index] * deltaAccel[index];
					}
				}

				_ASSERTE(akNum >= dgFloat32(0.0f));
				_ASSERTE(akDen > dgFloat32(0.0f));
				akDen = GetMax(akDen, dgFloat32(1.0e-17f));
				ak = dgFloat32(akNum / akDen);
				for (dgInt32 i = 0; i < m_jointCount; i++) {
					dgInt32 j;
					dgInt32 first;
					dgInt32 count;
					dgInt32 index;
					first = constraintArray[i].m_autoPairstart;
					count = constraintArray[i].m_autoPairActiveCount;
					for (j = 0; j < count; j++) {
						index = j + first;
						deltaForce[index] = deltaAccel[index] + ak * deltaForce[index];
					}
				}
			}
		}
		totalPassesCount++;
	}
	ApplyExternalForcesAndAcceleration(maxAccNorm);
}

dgFloat32 dgJacobianMemory::CalculateJointForcesSimd(dgInt32 joint,
													 dgFloat32 *forceStep, dgFloat32 maxAccNorm) const {

#ifdef DG_BUILD_SIMD_CODE

	dgInt32 m0;
	dgInt32 m1;
	dgInt32 first;
	dgInt32 count;
	dgInt32 roundCount;
	dgInt32 maxPasses;
	dgInt32 clampedForceIndex;
	dgFloat32 ak;
	dgFloat32 akNum;
	dgFloat32 retAccel;
	dgFloatSign tmpIndex;
	// dgFloat32 akDen;
	// dgFloat32 force;
	dgFloat32 accNorm;
	// dgFloat32 retAccNorm;
	dgFloat32 clampedForceIndexValue;
	// dgJacobian y0;
	// dgJacobian y1;
	//	simd_type tmp0;
	//	simd_type tmp1;
	//	simd_type tmp2;
	//	simd_type tmp3;

	simd_type akNumSimd;
	simd_type accNormSimd;
	simd_type maxPassesSimd;
	simd_type y0_linear;
	simd_type y0_angular;
	simd_type y1_linear;
	simd_type y1_angular;
	simd_type one;
	simd_type zero;
	simd_type signMask;
	simd_type tol_pos_1eNeg5;
	simd_type tol_pos_1eNeg8;
	simd_type tol_neg_1eNeg16;
	simd_type tol_pos_1eNeg16;
	simd_type deltaAccelPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type deltaForcePtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type activeRowPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type lowBoundPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];
	simd_type highBoundPtr[DG_CONSTRAINT_MAX_ROWS / DG_SIMD_WORD_SIZE];

	dgFloat32 *const deltaAccel = (dgFloat32 *)deltaAccelPtr;
	dgFloat32 *const deltaForce = (dgFloat32 *)deltaForcePtr;
	dgFloat32 *const activeRow = (dgFloat32 *)activeRowPtr;
	dgFloat32 *const lowBound = (dgFloat32 *)lowBoundPtr;
	dgFloat32 *const highBound = (dgFloat32 *)highBoundPtr;

	dgFloat32 *const accel = m_accel;
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	//	const dgFloat32* const lowerFrictionForce = m_lowerBoundFrictionCoefficent;
	//	const dgFloat32* const upperFrictionForce = m_upperBoundFrictionCoefficent;
	const dgFloat32 *const lowerFrictionCoef = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFrictionCoef = m_upperBoundFrictionCoefficent;

	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgJointInfo *const constraintArray = m_constraintArray;
	//	const dgJacobianIndex* const jacobianIndexArray = m_jacobianIndexArray;

	//	count = 0x7fffffff;
	//	signMask = simd_set1((dgFloat32&) count);
	tmpIndex.m_integer.m_iVal = 0x7fffffff;
	signMask = simd_set1(tmpIndex.m_fVal);

	one = simd_set1(dgFloat32(1.0f));
	zero = simd_set1(dgFloat32(0.0f));
	tol_pos_1eNeg8 = simd_set1(dgFloat32(1.0e-8f));
	tol_pos_1eNeg5 = simd_set1(dgFloat32(1.0e-5f));
	tol_pos_1eNeg16 = simd_set1(dgFloat32(1.0e-16f));
	tol_neg_1eNeg16 = simd_set1(dgFloat32(-1.0e-16f));

	first = constraintArray[joint].m_autoPairstart;
	count = constraintArray[joint].m_autoPaircount;
	m0 = constraintArray[joint].m_m0;
	m1 = constraintArray[joint].m_m1;

	roundCount = count & (-DG_SIMD_WORD_SIZE);
	if (roundCount != count) {
		roundCount += 4;

		for (dgInt32 j = count; j < roundCount; j++) {
			dgInt32 i;
			i = first + j;
			// force[i] = dgFloat32 (0.0f);
			// accel[j] -= dgFloat32 (0.0f);;
			// activeRow[j] = dgFloat32 (0.0f);
			// deltaAccel[j] = dgFloat32 (0.0f);
			// deltaForce[j] = dgFloat32 (0.0f);
			m_normalForceIndex[i] = -1;
			simd_store_s(zero, &force[i]);
			simd_store_s(zero, &accel[j]);
			simd_store_s(zero, &activeRow[j]);
			simd_store_s(zero, &deltaAccel[j]);
			simd_store_s(zero, &deltaForce[j]);
			simd_store_s(one, &m_lowerBoundFrictionCoefficent[i]);
			simd_store_s(zero, &m_upperBoundFrictionCoefficent[i]);
		}
	}

	// akNum = dgFloat32 (0.0f);
	// accNorm = dgFloat32(0.0f);
	// maxPasses = count;
	akNumSimd = zero;
	accNormSimd = zero;
	maxPassesSimd = zero;
	//	tmp3 = simd_set1(dgFloat32 (1.0f));

	// for (j = 0; j < count; j ++) {
	for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
		dgInt32 i;
		// dgInt32 k0;
		// dgInt32 k1;
		// dgInt32 k2;
		// dgInt32 k3;
		// simd_type index_k;
		// simd_type accel_j;
		simd_type force_k;
		// simd_type force_i;
		// simd_type lowBound_j;
		// simd_type higntBound_j;
		// simd_type deltaforce_j;
		simd_type lowHighBound_test;

		i = first + j;
		// k = bilateralForceBounds[i].m_normalIndex;
		// k0 = normalForceIndex[i + 0];
		// k1 = normalForceIndex[i + 1];
		// k2 = normalForceIndex[i + 2];
		// k3 = normalForceIndex[i + 3];

		// val = (k >= 0) ? force[k] : dgFloat32 (1.0f);
		// val = force[k];
		// index_k = simd_move_lh_v (simd_pack_lo_v (simd_load_is (zero, k0), simd_load_is (zero, k1)), simd_pack_lo_v (simd_load_is (zero, k2), simd_load_is (zero, k3)));
		// index_k = simd_cmpge_v (index_k, zero);
		force_k =
			simd_move_lh_v(simd_pack_lo_v(simd_load_s(force[normalForceIndex[i + 0]]), simd_load_s(force[normalForceIndex[i + 1]])), simd_pack_lo_v(simd_load_s(force[normalForceIndex[i + 2]]), simd_load_s(force[normalForceIndex[i + 3]])));
		// force_k = simd_or_v (simd_and_v(force_k, index_k), simd_andnot_v (one, index_k));

		// lowBound[j] = val * bilateralForceBounds[i].m_low;
		// highBound[j] = val * bilateralForceBounds[i].m_upper;
		(simd_type &)lowBound[j] =
			simd_mul_v(force_k, (simd_type &)lowerFrictionCoef[i]);
		(simd_type &)highBound[j] =
			simd_mul_v(force_k, (simd_type &)upperFrictionCoef[i]);

		// activeRow[j] = dgFloat32 (1.0f);
		// forceStep[j] = m_force[i];
		// if (force[i] < lowBound[j]) {
		//	maxPasses --;
		//	force[i] = lowBound[j];
		//	activeRow[j] = dgFloat32 (0.0f);
		// } else if (force[i] > highBound[j]) {
		//	maxPasses --;
		//	force[i] = highBound[j];
		//	activeRow[j] = dgFloat32 (0.0f);
		// }

		(simd_type &)forceStep[j] = (simd_type &)force[i];
		//		lowBound_test = simd_cmplt_v (force_i, lowBound_j);
		//		higntBound_test = simd_cmpgt_v (force_i, higntBound_j);
		lowHighBound_test =
			simd_or_v(simd_cmplt_v((simd_type &)force[i], (simd_type &)lowBound[j]), simd_cmpgt_v((simd_type &)force[i], (simd_type &)highBound[j]));
		(simd_type &)activeRow[j] = simd_andnot_v(one, lowHighBound_test);
		maxPassesSimd = simd_add_v(maxPassesSimd, (simd_type &)activeRow[j]);

		// force_k = simd_or_v (simd_and_v (lowBound_j, lowBound_test), simd_and_v (higntBound_j, higntBound_test));
		//(simd_type&)force[i] = simd_mul_v (activeRow_j, simd_or_v (simd_and_v (force_k, lowHighBound_test), simd_andnot_v (force_i, lowHighBound_test)));
		(simd_type &)force[i] =
			simd_min_v((simd_type &)highBound[j], simd_max_v((simd_type &)force[i], (simd_type &)lowBound[j]));

		// deltaForce[j] = accel[j] * invDJMinvJt[i] * activeRow[j];
		(simd_type &)deltaForce[j] =
			simd_mul_v((simd_type &)accel[j], simd_mul_v((simd_type &)invDJMinvJt[i], (simd_type &)activeRow[j]));

		// akNum += accel[j] * deltaForce[j];
		akNumSimd =
			simd_mul_add_v(akNumSimd, (simd_type &)accel[j], (simd_type &)deltaForce[j]);

		// accNorm = GetMax (dgAbsf (accel[j] * activeRow[j]), accNorm);
		accNormSimd =
			simd_max_v(accNormSimd, simd_and_v(simd_mul_v((simd_type &)accel[j], (simd_type &)activeRow[j]), signMask));
	}
	akNumSimd = simd_add_v(akNumSimd, simd_move_hl_v(akNumSimd, akNumSimd));
	simd_store_s(
		simd_add_s(akNumSimd, simd_permut_v(akNumSimd, akNumSimd, PURMUT_MASK(0, 0, 0, 1))),
		&akNum);

	accNormSimd =
		simd_max_v(accNormSimd, simd_move_hl_v(accNormSimd, accNormSimd));
	simd_store_s(
		simd_max_s(accNormSimd, simd_permut_v(accNormSimd, accNormSimd, PURMUT_MASK(0, 0, 0, 1))),
		&accNorm);

	maxPassesSimd =
		simd_add_v(maxPassesSimd, simd_move_hl_v(maxPassesSimd, maxPassesSimd));
	maxPasses =
		simd_store_is(simd_add_s(maxPassesSimd, simd_permut_v(maxPassesSimd, maxPassesSimd, PURMUT_MASK(0, 0, 0, 1))));

	retAccel = accNorm;
	clampedForceIndexValue = dgFloat32(0.0f);
	for (dgInt32 i = 0; (i < maxPasses) && (accNorm > maxAccNorm); i++) {
		simd_type akSimd;
		simd_type akDenSimd;

		// y0.m_linear = zero;
		// y0.m_angular = zero;
		// y1.m_linear = zero;
		// y1.m_angular = zero;
		y0_linear = zero;
		y0_angular = zero;
		y1_linear = zero;
		y1_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 k;
			simd_type tmp1;
			k = j + first;
			// ak = deltaForce[j];
			tmp1 = simd_set1(deltaForce[j]);
			// y0.m_linear += m_Jt[k].m_jacobian_IM0.m_linear.Scale (ak);
			// y0.m_angular += m_Jt[k].m_jacobian_IM0.m_angular.Scale (ak);
			// y1.m_linear += m_Jt[k].m_jacobian_IM1.m_linear.Scale (ak);
			// y1.m_angular += m_Jt[k].m_jacobian_IM1.m_angular.Scale (ak);

			y0_linear =
				simd_mul_add_v(y0_linear, (simd_type &)Jt[k].m_jacobian_IM0.m_linear, tmp1);
			y0_angular =
				simd_mul_add_v(y0_angular, (simd_type &)Jt[k].m_jacobian_IM0.m_angular, tmp1);
			y1_linear =
				simd_mul_add_v(y1_linear, (simd_type &)Jt[k].m_jacobian_IM1.m_linear, tmp1);
			y1_angular =
				simd_mul_add_v(y1_angular, (simd_type &)Jt[k].m_jacobian_IM1.m_angular, tmp1);
		}

		// akDen = dgFloat32 (0.0f);
		akDenSimd = zero;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 k;
			simd_type tmp1;
			k = j + first;

			// dgVector acc (m_JMinv[k].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
			// acc += m_JMinv[k].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
			// acc += m_JMinv[k].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
			// acc += m_JMinv[k].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);
			tmp1 =
				simd_mul_v((simd_type &)JMinv[k].m_jacobian_IM0.m_linear, y0_linear);
			tmp1 =
				simd_mul_add_v(tmp1, (simd_type &)JMinv[k].m_jacobian_IM0.m_angular, y0_angular);
			tmp1 =
				simd_mul_add_v(tmp1, (simd_type &)JMinv[k].m_jacobian_IM1.m_linear, y1_linear);
			tmp1 =
				simd_mul_add_v(tmp1, (simd_type &)JMinv[k].m_jacobian_IM1.m_angular, y1_angular);

			// deltaAccel[j] = acc.m_x + acc.m_y + acc.m_z + deltaForce[j] * m_diagDamp[k];
			tmp1 = simd_add_v(tmp1, simd_move_hl_v(tmp1, tmp1));
			tmp1 =
				simd_add_s(tmp1, simd_permut_v(tmp1, tmp1, PURMUT_MASK(3, 3, 3, 1)));
			tmp1 =
				simd_mul_add_s(tmp1, simd_load_s(deltaForce[j]), simd_load_s(diagDamp[k]));
			simd_store_s(tmp1, &deltaAccel[j]);

			// akDen += deltaAccel[j] * deltaForce[j];
			akDenSimd = simd_mul_add_s(akDenSimd, tmp1, simd_load_s(deltaForce[j]));
		}

		//_ASSERTE (akDen > dgFloat32 (0.0f));
		// akDen = GetMax (akDen, dgFloat32(1.0e-16f));
		//_ASSERTE (dgAbsf (akDen) >= dgFloat32(1.0e-16f));
		// ak = akNum / akDen;
		akSimd =
			simd_div_s(simd_load_s(akNum), simd_max_s(akDenSimd, tol_pos_1eNeg16));

		//		simd_store_s (tmp0, &ak);
		//		clampedForceIndex = -1;
		simd_type min_index;
		simd_type minClampIndex;
		simd_type min_index_step;
		simd_type campedIndexValue;

		campedIndexValue = zero;
		minClampIndex = simd_set1(dgFloat32(-1.0f));
		min_index_step = simd_set1(dgFloat32(4.0f));
		akSimd = simd_permut_v(akSimd, akSimd, PURMUT_MASK(0, 0, 0, 0));
		min_index =
			simd_set(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(2.0f), dgFloat32(3.0f));

		//		for (j = 0; j < roundCount; j ++) {
		for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
			//			if (activeRow[j]) {
			//			dgFloat32 val;
			//			k = j + first;
			//			if (deltaForce[j] < dgFloat32 (-1.0e-16f)) {
			//			val = force[k] + ak * deltaForce[j];
			//			if (val < lowBound[j]) {
			//			ak = (lowBound[j] - force[k]) / deltaForce[j];
			//			clampedForceIndex = j;
			//			clampedForceIndexValue = lowBound[j];
			//			if (ak < dgFloat32 (1.0e-8f)) {
			//			ak = dgFloat32 (0.0f);
			//			break;
			//			}
			//			}
			//			} else if (deltaForce[j] > dgFloat32 (1.0e-16f)) {
			//			val = force[k] + ak * deltaForce[j];
			//			if (val > highBound[j]) {
			//			ak = (highBound[j] - force[k]) / deltaForce[j];
			//			clampedForceIndex = j;
			//			clampedForceIndexValue = highBound[j];
			//			if (ak < dgFloat32 (1.0e-8f)) {
			//			ak = dgFloat32 (0.0f);
			//			break;
			//			}
			//			}
			//			}
			//			}
			//
			// bool test;
			// bool negTest;
			// bool posTest;
			// bool negValTest;
			// bool posValTest;
			// bool negDeltaForceTest;
			// bool posDeltaForceTest;
			// dgFloat32 val;
			// dgFloat32 num;
			// dgFloat32 den;

			dgInt32 k;
			simd_type val;
			simd_type num;
			simd_type den;
			simd_type test;
			simd_type negTest;
			simd_type posTest;
			simd_type negValTest;
			simd_type posValTest;
			simd_type negDeltaForceTest;
			simd_type posDeltaForceTest;

			// Make sure AK is not negative
			k = j + first;
			// val = force[k] + ak * deltaForce[j];
			val =
				simd_mul_add_v((simd_type &)force[k], akSimd, (simd_type &)deltaForce[j]);

			// negValTest = val < lowBound[j];
			negValTest = simd_cmplt_v(val, (simd_type &)lowBound[j]);

			// posValTest = val > highBound[j];
			posValTest = simd_cmpgt_v(val, (simd_type &)highBound[j]);

			// negDeltaForceTest = deltaForce[j] < dgFloat32 (-1.0e-16f);
			negDeltaForceTest =
				simd_cmplt_v((simd_type &)deltaForce[j], tol_neg_1eNeg16);

			// posDeltaForceTest = deltaForce[j] > dgFloat32 ( 1.0e-16f);
			posDeltaForceTest =
				simd_cmpgt_v((simd_type &)deltaForce[j], tol_pos_1eNeg16);

			// negTest = negValTest & negDeltaForceTest;
			negTest = simd_and_v(negValTest, negDeltaForceTest);

			// posTest = posValTest & posDeltaForceTest;
			posTest = simd_and_v(posValTest, posDeltaForceTest);

			// test = negTest | posTest;
			test = simd_or_v(negTest, posTest);

			// num = negTest ? lowBound[j] : (posTest ? highBound[j] : force[k]);
			num =
				simd_or_v(simd_and_v((simd_type &)lowBound[j], negTest), simd_and_v((simd_type &)highBound[j], posTest));
			num =
				simd_or_v(simd_and_v(num, test), simd_andnot_v((simd_type &)force[k], test));

			// den = test ? deltaForce[j] : dgFloat32 (1.0f);
			den =
				simd_or_v(simd_and_v((simd_type &)deltaForce[j], test), simd_andnot_v(one, test));

			// test = test & (activeRow[j] > dgFloat32 (0.0f));
			test = simd_and_v(test, simd_cmpgt_v((simd_type &)activeRow[j], zero));

			//_ASSERTE (dgAbsf (den) > 1.0e-16f);
			// ak = test ? (num - force[k]) / den : ak;
			akSimd =
				simd_or_v(simd_div_v(simd_sub_v(num, (simd_type &)force[k]), den), simd_andnot_v(akSimd, test));

			// ak = (ak < dgFloat32 (1.0e-8f)) ? dgFloat32 (0.0f) : ak;
			akSimd = simd_and_v(akSimd, simd_cmpgt_v(akSimd, tol_pos_1eNeg8));

			// clampedForceIndex = test ? j : clampedForceIndex;
			minClampIndex =
				simd_or_v(simd_and_v(min_index, test), simd_andnot_v(minClampIndex, test));
			min_index = simd_add_v(min_index, min_index_step);

			// clampedForceIndexValue = test ? num : clampedForceIndexValue;
			campedIndexValue =
				simd_or_v(simd_and_v(num, test), simd_andnot_v(campedIndexValue, test));
		}

		akDenSimd = simd_move_hl_v(akSimd, akSimd);
		maxPassesSimd = simd_cmplt_v(akSimd, akDenSimd);
		akSimd = simd_min_v(akSimd, akDenSimd);
		minClampIndex =
			simd_or_v(simd_and_v(minClampIndex, maxPassesSimd), simd_andnot_v(simd_move_hl_v(minClampIndex, minClampIndex), maxPassesSimd));
		campedIndexValue =
			simd_or_v(simd_and_v(campedIndexValue, maxPassesSimd), simd_andnot_v(simd_move_hl_v(campedIndexValue, campedIndexValue), maxPassesSimd));

		akDenSimd = simd_permut_v(akSimd, akSimd, PURMUT_MASK(0, 0, 0, 1));
		maxPassesSimd = simd_cmplt_s(akSimd, akDenSimd);
		akSimd = simd_min_s(akSimd, akDenSimd);
		minClampIndex =
			simd_or_v(simd_and_v(minClampIndex, maxPassesSimd), simd_andnot_v(simd_permut_v(minClampIndex, minClampIndex, PURMUT_MASK(0, 0, 0, 1)), maxPassesSimd));
		campedIndexValue =
			simd_or_v(simd_and_v(campedIndexValue, maxPassesSimd), simd_andnot_v(simd_permut_v(campedIndexValue, campedIndexValue, PURMUT_MASK(0, 0, 0, 1)), maxPassesSimd));

		//		tmp2 = zero;
		simd_store_s(akSimd, &ak);
		clampedForceIndex = simd_store_is(minClampIndex);
		simd_store_s(campedIndexValue, &clampedForceIndexValue);
		if (ak == dgFloat32(0.0f) && (clampedForceIndex != -1)) {
			_ASSERTE(clampedForceIndex != -1);

			// akNum = dgFloat32 (0.0f);
			// accNorm = dgFloat32(0.0f);
			akNumSimd = zero;
			accNormSimd = zero;
			maxPassesSimd = zero;

			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			deltaForce[clampedForceIndex] = dgFloat32(0.0f);
			force[clampedForceIndex + first] = clampedForceIndexValue;

			// for (j = 0; j < count; j ++) {
			for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
				// for (j = 0; j < roundCount; j ++) {
				//				if (((dgFloat32*)activeRow)[j]) {
				//				bool test0;
				//				bool test1;
				//				k = j + first;
				//				val = ((dgFloat32*)lowBound)[j] - force[k];
				//				if ((dgAbsf (val) < dgFloat32 (1.0e-5f)) && (accel[j] < dgFloat32 (0.0f))) {
				//				force[k] = lowBound[j];
				//				activeRow[j] = dgFloat32 (0.0f);
				//				deltaForce[j] = dgFloat32 (0.0f);
				//
				//				} else {
				//				val = highBound[j] - force[k];
				//				if ((dgAbsf (val) < dgFloat32 (1.0e-5f)) && (accel[j] > dgFloat32 (0.0f))) {
				//				force[k] = highBound[j];
				//				activeRow[j] = dgFloat32 (0.0f);
				//				deltaForce[j] = dgFloat32 (0.0f);
				//				} else {
				//				_ASSERTE (activeRow[j] > dgFloat32 (0.0f));
				//				deltaForce[j] = accel[j] * invDJMinvJt[k];
				//				akNum += accel[j] * deltaForce[j];
				//				accNorm = GetMax (dgAbsf (accel[j]), accNorm);
				//				}
				//				}
				//				}

				dgInt32 k;
				simd_type val_k;
				simd_type test_0;
				simd_type test_1;
				// simd_type test_2;
				simd_type accel_k;
				simd_type force_k;

				k = j + first;
				accel_k = (simd_type &)accel[j];
				force_k = (simd_type &)force[k];
				// val = dgAbsf (lowBound[j] - force[k]);
				val_k =
					simd_and_v(simd_sub_v((simd_type &)lowBound[j], force_k), signMask);

				// test0 = (val < dgFloat32 (1.0e-5f)) & (accel[j] < dgFloat32 (0.0f));
				test_0 =
					simd_and_v(simd_cmplt_v(val_k, tol_pos_1eNeg5), simd_cmplt_v(accel_k, zero));

				// val = dgAbsf (highBound[j] - force[k]);
				val_k =
					simd_and_v(simd_sub_v((simd_type &)highBound[j], force_k), signMask);

				// test1 = (val < dgFloat32 (1.0e-5f)) & (accel[j] > dgFloat32 (0.0f));
				test_1 =
					simd_and_v(simd_cmplt_v(val_k, tol_pos_1eNeg5), simd_cmpgt_v(accel_k, zero));

				// force[k] = test0 ? lowBound[j] : (test1 ? highBound[j] : force[k]);
				// val_k = simd_or_v (simd_and_v ((simd_type&)lowBound[j], test_0), simd_and_v ((simd_type&)highBound[j], test_1));
				//(simd_type&) force[k] = simd_or_v (simd_and_v (val_k, test_2) , simd_andnot_v (force_k, test_2));
				(simd_type &)force[k] =
					simd_min_v((simd_type &)highBound[j], simd_max_v((simd_type &)force[k], (simd_type &)lowBound[j]));

				// activeRow[j] *= (test0 | test1) ? dgFloat32 (0.0f) : dgFloat32 (1.0f);
				// test_2 = simd_or_v (test_0, test_1);
				(simd_type &)activeRow[j] =
					simd_mul_v((simd_type &)activeRow[j], simd_andnot_v(one, simd_or_v(test_0, test_1)));

				// deltaForce[j] = accel[j] * invDJMinvJt[k] * activeRow[j];
				(simd_type &)deltaForce[j] =
					simd_mul_v(accel_k, simd_mul_v((simd_type &)invDJMinvJt[k], (simd_type &)activeRow[j]));

				// akNum += accel[j] * deltaForce[j];
				akNumSimd =
					simd_mul_add_v(akNumSimd, (simd_type &)deltaForce[j], accel_k);

				// accNorm = GetMax (dgAbsf (accel[j] * activeRow[j]), accNorm);
				accNormSimd =
					simd_max_v(accNormSimd, simd_and_v(simd_mul_v(accel_k, (simd_type &)activeRow[j]), signMask));

				// masPases += 1;
				maxPassesSimd = simd_add_v(maxPassesSimd, (simd_type &)activeRow[j]);
			}

			_ASSERTE(activeRow[clampedForceIndex] == dgFloat32(0.0f));

			akNumSimd = simd_add_v(akNumSimd, simd_move_hl_v(akNumSimd, akNumSimd));
			akNumSimd =
				simd_add_s(akNumSimd, simd_permut_v(akNumSimd, akNumSimd, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(akNumSimd, &akNum);

			accNormSimd =
				simd_max_v(accNormSimd, simd_move_hl_v(accNormSimd, accNormSimd));
			accNormSimd =
				simd_max_s(accNormSimd, simd_permut_v(accNormSimd, accNormSimd, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(accNormSimd, &accNorm);

			i = -1;
			// maxPasses = GetMax (maxPasses - 1, 1);
			maxPassesSimd =
				simd_add_v(maxPassesSimd, simd_move_hl_v(maxPassesSimd, maxPassesSimd));
			maxPasses =
				simd_store_is(simd_add_s(maxPassesSimd, simd_permut_v(maxPassesSimd, maxPassesSimd, PURMUT_MASK(0, 0, 0, 1))));

		} else if (clampedForceIndex >= 0) {
			// akNum = dgFloat32(0.0f);
			// accNorm = dgFloat32(0.0f);

			akNumSimd = zero;
			accNormSimd = zero;
			//			tmp2 = zero;
			akSimd = simd_permut_v(akSimd, akSimd, PURMUT_MASK(0, 0, 0, 0));
			activeRow[clampedForceIndex] = dgFloat32(0.0f);

			// for (j = 0; j < count; j ++) {
			for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
				dgInt32 k;
				k = j + first;
				// m_force[k] += ak * deltaForce[j];
				// m_accel[j] -= ak * deltaAccel[j];
				(simd_type &)force[k] =
					simd_mul_add_v((simd_type &)force[k], akSimd, (simd_type &)deltaForce[j]);
				(simd_type &)accel[j] =
					simd_mul_sub_v((simd_type &)accel[j], akSimd, (simd_type &)deltaAccel[j]);

				// accNorm = GetMax (dgAbsf (m_accel[j] * activeRow[j]), accNorm);
				accNormSimd =
					simd_max_v(accNormSimd, simd_and_v(simd_mul_v((simd_type &)accel[j], (simd_type &)activeRow[j]), signMask));
				//_ASSERTE (dgCheckFloat(m_force[k]));
				//_ASSERTE (dgCheckFloat(m_accel[j]));

				// deltaForce[j] = m_accel[j] * m_invDJMinvJt[k] * activeRow[j];
				(simd_type &)deltaForce[j] =
					simd_mul_v((simd_type &)accel[j], simd_mul_v((simd_type &)invDJMinvJt[k], (simd_type &)activeRow[j]));

				// akNum += deltaForce[j] * m_accel[j];
				akNumSimd =
					simd_mul_add_v(akNumSimd, (simd_type &)deltaForce[j], (simd_type &)accel[j]);
			}

			akNumSimd = simd_add_v(akNumSimd, simd_move_hl_v(akNumSimd, akNumSimd));
			akNumSimd =
				simd_add_s(akNumSimd, simd_permut_v(akNumSimd, akNumSimd, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(akNumSimd, &akNum);

			accNormSimd =
				simd_max_v(accNormSimd, simd_move_hl_v(accNormSimd, accNormSimd));
			accNormSimd =
				simd_max_s(accNormSimd, simd_permut_v(accNormSimd, accNormSimd, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(accNormSimd, &accNorm);

			force[clampedForceIndex + first] = clampedForceIndexValue;

			i = -1;
			maxPasses = GetMax(maxPasses - 1, 1);

		} else {
			// accNorm = dgFloat32(0.0f);
			//			tmp2 = zero;
			accNormSimd = zero;
			akSimd = simd_permut_v(akSimd, akSimd, PURMUT_MASK(0, 0, 0, 0));
			// for (j = 0; j < count; j ++) {
			for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
				dgInt32 k;
				k = j + first;
				// m_force[k] += ak * deltaForce[j];
				// m_accel[j] -= ak * deltaAccel[j];
				(simd_type &)force[k] =
					simd_mul_add_v((simd_type &)force[k], akSimd, (simd_type &)deltaForce[j]);
				(simd_type &)accel[j] =
					simd_mul_sub_v((simd_type &)accel[j], akSimd, (simd_type &)deltaAccel[j]);

				// accNorm = GetMax (dgAbsf (m_accel[j] * activeRow[j]), accNorm);
				accNormSimd =
					simd_max_v(accNormSimd, simd_and_v(simd_mul_v((simd_type &)accel[j], (simd_type &)activeRow[j]), signMask));
				//_ASSERTE (dgCheckFloat(m_force[k]));
				//_ASSERTE (dgCheckFloat(m_accel[j]));
			}
			accNormSimd =
				simd_max_v(accNormSimd, simd_move_hl_v(accNormSimd, accNormSimd));
			accNormSimd =
				simd_max_s(accNormSimd, simd_permut_v(accNormSimd, accNormSimd, PURMUT_MASK(0, 0, 0, 1)));
			simd_store_s(accNormSimd, &accNorm);
			if (accNorm > maxAccNorm) {

				// akDen = akNum;
				// akNum = dgFloat32(0.0f);
				akDenSimd = simd_set1(akNum);
				akNumSimd = zero;
				// for (j = 0; j < count; j ++) {
				for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
					dgInt32 k;
					k = j + first;
					// deltaAccel[j] = m_accel[j] * m_invDJMinvJt[k] * activeRow[j];
					(simd_type &)deltaAccel[j] =
						simd_mul_v((simd_type &)accel[j], simd_mul_v((simd_type &)invDJMinvJt[k], (simd_type &)activeRow[j]));
					// akNum += m_accel[j] * deltaAccel[j];
					akNumSimd =
						simd_mul_add_v(akNumSimd, (simd_type &)accel[j], (simd_type &)deltaAccel[j]);
				}

				akNumSimd = simd_add_v(akNumSimd, simd_move_hl_v(akNumSimd, akNumSimd));
				akNumSimd =
					simd_add_s(akNumSimd, simd_permut_v(akNumSimd, akNumSimd, PURMUT_MASK(0, 0, 0, 1)));
				simd_store_s(akNumSimd, &akNum);

				//_ASSERTE (akDen > dgFloat32(0.0f));
				// akDen = GetMax (akDen, dgFloat32 (1.0e-17f));
				akDenSimd = simd_max_s(akDenSimd, simd_set1(dgFloat32(1.0e-17f)));
				// ak = dgFloat32 (akNum / akDen);
				akSimd = simd_div_s(akSimd, akDenSimd);
				akSimd = simd_permut_v(akSimd, akSimd, PURMUT_MASK(0, 0, 0, 0));

				// for (j = 0; j < count; j ++) {
				for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
					// deltaForce[j] = deltaAccel[j] + ak * deltaForce[j];
					(simd_type &)deltaForce[j] =
						simd_mul_add_v((simd_type &)deltaAccel[j], akSimd, (simd_type &)deltaForce[j]);
				}
			}
		}
	}

	//	for (j = 0; j < count; j ++) {
	for (dgInt32 j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
		// forceStep[j] = force[j + first] - forceStep[j];
		(simd_type &)forceStep[j] =
			simd_sub_v((simd_type &)force[j + first], (simd_type &)forceStep[j]);
	}

	//	tmp0 = simd_set1(DG_SSOR_FACTOR);
	//	for (j = 0; j < roundCount; j += DG_SIMD_WORD_SIZE) {
	//		tmp1 = simd_mul_add_v((simd_type&)forceStep[j], simd_sub_v ((simd_type&)force[j + first], (simd_type&)forceStep[j]), tmp0);
	//		(simd_type&)force[j + first] = simd_min_v ((simd_type&) highBound[j], simd_max_v(tmp1, (simd_type&)lowBound[j]));
	//		(simd_type&)forceStep[j] = simd_sub_v ((simd_type&)force[j + first], (simd_type&)forceStep[j]);
	//	}

	return retAccel;

#else
	return dgFloat32(0.0f);
#endif
}

dgFloat32 dgJacobianMemory::CalculateJointForces(dgInt32 joint,
												 dgFloat32 *forceStep, dgFloat32 maxAccNorm) const {
	dgInt32 first;
	dgInt32 count;
	dgInt32 maxPasses;
	dgInt32 clampedForceIndex;
	dgFloat32 ak;
	dgFloat32 val;
	dgFloat32 akNum;
	dgFloat32 akDen;
	dgFloat32 accNorm;
	dgFloat32 retAccel;
	dgFloat32 clampedForceIndexValue;
	dgJacobian y0;
	dgJacobian y1;
	dgFloat32 deltaAccel[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 deltaForce[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 activeRow[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 lowBound[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 highBound[DG_CONSTRAINT_MAX_ROWS];

	dgFloat32 *const accel = m_accel;
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const lowerFriction = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFriction = m_upperBoundFrictionCoefficent;
	// dgBilateralBounds* const bilateralForceBounds = m_bilateralForceBounds;
	// const dgJacobianIndex* const jacobianIndexArray = m_jacobianIndexArray;

	first = constraintArray[joint].m_autoPairstart;
	count = constraintArray[joint].m_autoPaircount;

	akNum = dgFloat32(0.0f);
	accNorm = dgFloat32(0.0f);
	maxPasses = count;

	for (dgInt32 j = 0; j < count; j++) {
		dgInt32 i;
		dgInt32 k;
		i = first + j;
		// k = bilateralForceBounds[i].m_normalIndex;
		k = normalForceIndex[i];

		//		val = (k >= 0) ? force[k] : dgFloat32 (1.0f);
		_ASSERTE(
			((k < 0) && (force[k] == dgFloat32(1.0f))) || ((k >= 0) && (force[k] >= dgFloat32(0.0f))));
		val = force[k];
		lowBound[j] = val * lowerFriction[i];
		highBound[j] = val * upperFriction[i];

		activeRow[j] = dgFloat32(1.0f);
		forceStep[j] = force[i];
		if (force[i] < lowBound[j]) {
			maxPasses--;
			force[i] = lowBound[j];
			activeRow[j] = dgFloat32(0.0f);
		} else if (force[i] > highBound[j]) {
			maxPasses--;
			force[i] = highBound[j];
			activeRow[j] = dgFloat32(0.0f);
		}

		deltaForce[j] = accel[j] * invDJMinvJt[i] * activeRow[j];
		akNum += accel[j] * deltaForce[j];
		accNorm = GetMax(dgAbsf(accel[j] * activeRow[j]), accNorm);
	}

	retAccel = accNorm;
	clampedForceIndexValue = dgFloat32(0.0f);

	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));
	for (dgInt32 i = 0; (i < maxPasses) && (accNorm > maxAccNorm); i++) {
		y0.m_linear = zero;
		y0.m_angular = zero;
		y1.m_linear = zero;
		y1.m_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 k;
			k = j + first;
			ak = deltaForce[j];

			y0.m_linear += Jt[k].m_jacobian_IM0.m_linear.Scale(ak);
			y0.m_angular += Jt[k].m_jacobian_IM0.m_angular.Scale(ak);
			y1.m_linear += Jt[k].m_jacobian_IM1.m_linear.Scale(ak);
			y1.m_angular += Jt[k].m_jacobian_IM1.m_angular.Scale(ak);
		}

		akDen = dgFloat32(0.0f);
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 k;
			k = j + first;

			dgVector acc(JMinv[k].m_jacobian_IM0.m_linear.CompProduct(y0.m_linear));
			acc += JMinv[k].m_jacobian_IM0.m_angular.CompProduct(y0.m_angular);
			acc += JMinv[k].m_jacobian_IM1.m_linear.CompProduct(y1.m_linear);
			acc += JMinv[k].m_jacobian_IM1.m_angular.CompProduct(y1.m_angular);
			deltaAccel[j] = acc.m_x + acc.m_y + acc.m_z + deltaForce[j] * diagDamp[k];
			akDen += deltaAccel[j] * deltaForce[j];
		}

		_ASSERTE(akDen > dgFloat32(0.0f));
		akDen = GetMax(akDen, dgFloat32(1.0e-16f));
		_ASSERTE(dgAbsf(akDen) >= dgFloat32(1.0e-16f));
		ak = akNum / akDen;

		clampedForceIndex = -1;
		for (dgInt32 j = 0; j < count; j++) {
			if (activeRow[j]) {
				dgInt32 k;
				k = j + first;
				if (deltaForce[j] < dgFloat32(-1.0e-16f)) {
					val = force[k] + ak * deltaForce[j];
					if (val < lowBound[j]) {
						ak = GetMax((lowBound[j] - force[k]) / deltaForce[j],
									dgFloat32(0.0f));
						clampedForceIndex = j;
						clampedForceIndexValue = lowBound[j];
						if (ak < dgFloat32(1.0e-8f)) {
							ak = dgFloat32(0.0f);
							break;
						}
					}
				} else if (deltaForce[j] > dgFloat32(1.0e-16f)) {
					val = force[k] + ak * deltaForce[j];
					if (val > highBound[j]) {
						ak = GetMax((highBound[j] - force[k]) / deltaForce[j],
									dgFloat32(0.0f));
						clampedForceIndex = j;
						clampedForceIndexValue = highBound[j];
						if (ak < dgFloat32(1.0e-8f)) {
							ak = dgFloat32(0.0f);
							break;
						}
					}
				}
			}
		}

		//		if (ak == dgFloat32 (0.0f)) {
		if (ak == dgFloat32(0.0f) && (clampedForceIndex != -1)) {

			_ASSERTE(clampedForceIndex != -1);
			akNum = dgFloat32(0.0f);
			accNorm = dgFloat32(0.0f);

			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			deltaForce[clampedForceIndex] = dgFloat32(0.0f);
			force[clampedForceIndex + first] = clampedForceIndexValue;
			for (dgInt32 j = 0; j < count; j++) {
				if (activeRow[j]) {
					dgInt32 k;
					k = j + first;
					val = lowBound[j] - force[k];
					if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[j] < dgFloat32(0.0f))) {
						force[k] = lowBound[j];
						activeRow[j] = dgFloat32(0.0f);
						deltaForce[j] = dgFloat32(0.0f);

					} else {
						val = highBound[j] - force[k];
						if ((dgAbsf(val) < dgFloat32(1.0e-5f)) && (accel[j] > dgFloat32(0.0f))) {
							force[k] = highBound[j];
							activeRow[j] = dgFloat32(0.0f);
							deltaForce[j] = dgFloat32(0.0f);
						} else {
							_ASSERTE(activeRow[j] > dgFloat32(0.0f));
							deltaForce[j] = accel[j] * invDJMinvJt[k];
							akNum += accel[j] * deltaForce[j];
							accNorm = GetMax(dgAbsf(accel[j]), accNorm);
						}
					}
				}
			}
			_ASSERTE(activeRow[clampedForceIndex] == dgFloat32(0.0f));

			i = -1;
			maxPasses = GetMax(maxPasses - 1, 1);

		} else if (clampedForceIndex >= 0) {
			akNum = dgFloat32(0.0f);
			accNorm = dgFloat32(0.0f);
			activeRow[clampedForceIndex] = dgFloat32(0.0f);
			for (dgInt32 j = 0; j < count; j++) {
				dgInt32 k;
				k = j + first;
				force[k] += ak * deltaForce[j];
				accel[j] -= ak * deltaAccel[j];
				accNorm = GetMax(dgAbsf(accel[j] * activeRow[j]), accNorm);
				_ASSERTE(dgCheckFloat(force[k]));
				_ASSERTE(dgCheckFloat(accel[j]));

				deltaForce[j] = accel[j] * invDJMinvJt[k] * activeRow[j];
				akNum += deltaForce[j] * accel[j];
			}
			force[clampedForceIndex + first] = clampedForceIndexValue;

			i = -1;
			maxPasses = GetMax(maxPasses - 1, 1);

		} else {
			accNorm = dgFloat32(0.0f);
			for (dgInt32 j = 0; j < count; j++) {
				dgInt32 k;
				k = j + first;
				force[k] += ak * deltaForce[j];
				accel[j] -= ak * deltaAccel[j];
				accNorm = GetMax(dgAbsf(accel[j] * activeRow[j]), accNorm);
				_ASSERTE(dgCheckFloat(force[k]));
				_ASSERTE(dgCheckFloat(accel[j]));
			}

			if (accNorm > maxAccNorm) {

				akDen = akNum;
				akNum = dgFloat32(0.0f);
				for (dgInt32 j = 0; j < count; j++) {
					dgInt32 k;
					k = j + first;
					deltaAccel[j] = accel[j] * invDJMinvJt[k] * activeRow[j];
					akNum += accel[j] * deltaAccel[j];
				}

				_ASSERTE(akDen > dgFloat32(0.0f));
				akDen = GetMax(akDen, dgFloat32(1.0e-17f));
				ak = dgFloat32(akNum / akDen);
				for (dgInt32 j = 0; j < count; j++) {
					deltaForce[j] = deltaAccel[j] + ak * deltaForce[j];
				}
			}
		}
	}

	for (dgInt32 j = 0; j < count; j++) {
		forceStep[j] = force[j + first] - forceStep[j];
	}
	return retAccel;
}

void dgJacobianMemory::CalculateForcesGameModeSimd(dgInt32 iterations,
												   dgFloat32 maxAccNorm) const {
#ifdef DG_BUILD_SIMD_CODE
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	const dgBodyInfo *bodyArray = m_bodyArray;
	dgFloat32 *const penetration = m_penetration;
	const dgFloat32 *const externAccel = m_deltaAccel;
	const dgFloat32 *const restitution = m_restitution;
	dgFloat32 *const coordenateAccel = m_coordenateAccel;
	dgJacobian *const internalVeloc = m_internalVeloc;
	dgJacobian *const internalForces = m_internalForces;
	dgFloat32 **const jointForceFeeback = m_jointFeebackForce;
	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgInt32 *const accelIsMortor = m_accelIsMotor;
	const dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const penetrationStiffness = m_penetrationStiffness;
	const dgFloat32 *const lowerFrictionCoef = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFrictionCoef = m_upperBoundFrictionCoefficent;

	dgFloat32 invStep = (dgFloat32(1.0f) / dgFloat32(LINEAR_SOLVER_SUB_STEPS));
	dgFloat32 timeStep = m_timeStep * invStep;
	dgFloat32 invTimeStep = m_invTimeStep * dgFloat32(LINEAR_SOLVER_SUB_STEPS);

	dgFloatSign tmpIndex;
	tmpIndex.m_integer.m_iVal = 0x7fffffff;
	simd_type signMask = simd_set1(tmpIndex.m_fVal);

	simd_type zero = simd_set1(dgFloat32(0.0f));
	for (dgInt32 i = 1; i < m_bodyCount; i++) {
		dgBody *const body = m_bodyArray[i].m_body;
		(simd_type &)internalVeloc[i].m_linear = zero;
		(simd_type &)internalVeloc[i].m_angular = zero;
		(simd_type &)internalForces[i].m_linear = zero;
		(simd_type &)internalForces[i].m_angular = zero;
		(simd_type &)body->m_netForce = (simd_type &)body->m_veloc;
		(simd_type &)body->m_netTorque = (simd_type &)body->m_omega;
	}
	(simd_type &)internalVeloc[0].m_linear = zero;
	(simd_type &)internalVeloc[0].m_angular = zero;
	(simd_type &)internalForces[0].m_linear = zero;
	(simd_type &)internalForces[0].m_angular = zero;

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 first = constraintArray[i].m_autoPairstart;
		dgInt32 count = constraintArray[i].m_autoPairActiveCount;
		dgInt32 m0 = constraintArray[i].m_m0;
		dgInt32 m1 = constraintArray[i].m_m1;
		// dgJacobian y0 (internalForces[k0]);
		// dgJacobian y1 (internalForces[k1]);
		simd_type y0_linear = zero;
		simd_type y0_angular = zero;
		simd_type y1_linear = zero;
		simd_type y1_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 index = j + first;
			// val = force[index];
			simd_type tmp0 = simd_set1(force[index]);
			// y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale (val);
			// y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale (val);
			// y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale (val);
			// y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale (val);
			y0_linear =
				simd_mul_add_v(y0_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, tmp0);
			y0_angular =
				simd_mul_add_v(y0_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, tmp0);
			y1_linear =
				simd_mul_add_v(y1_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, tmp0);
			y1_angular =
				simd_mul_add_v(y1_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, tmp0);
		}
		// internalForces[k0] = y0;
		// internalForces[k1] = y1;
		(simd_type &)internalForces[m0].m_linear =
			simd_add_v((simd_type &)internalForces[m0].m_linear, y0_linear);
		(simd_type &)internalForces[m0].m_angular =
			simd_add_v((simd_type &)internalForces[m0].m_angular, y0_angular);
		(simd_type &)internalForces[m1].m_linear =
			simd_add_v((simd_type &)internalForces[m1].m_linear, y1_linear);
		(simd_type &)internalForces[m1].m_angular =
			simd_add_v((simd_type &)internalForces[m1].m_angular, y1_angular);
	}

	simd_type timeStepSimd = simd_set1(timeStep);
	dgFloat32 firstPassCoef = dgFloat32(0.0f);
	dgInt32 maxPasses = iterations + DG_BASE_ITERATION_COUNT;
	for (dgInt32 step = 0; step < LINEAR_SOLVER_SUB_STEPS; step++) {
		for (dgInt32 curJoint = 0; curJoint < m_jointCount; curJoint++) {
			dgJointAccelerationDecriptor joindDesc;

			dgInt32 index = constraintArray[curJoint].m_autoPairstart;
			joindDesc.m_rowsCount = constraintArray[curJoint].m_autoPaircount;

			joindDesc.m_timeStep = timeStep;
			joindDesc.m_invTimeStep = invTimeStep;
			joindDesc.m_firstPassCoefFlag = firstPassCoef;

			joindDesc.m_Jt = &Jt[index];

			joindDesc.m_penetration = &penetration[index];
			joindDesc.m_restitution = &restitution[index];
			joindDesc.m_accelIsMotor = &accelIsMortor[index];
			joindDesc.m_externAccelaration = &externAccel[index];
			joindDesc.m_coordenateAccel = &coordenateAccel[index];
			joindDesc.m_normalForceIndex = &normalForceIndex[index];
			joindDesc.m_penetrationStiffness = &penetrationStiffness[index];
			constraintArray[curJoint].m_joint->JointAccelerationsSimd(joindDesc);
		}
		firstPassCoef = dgFloat32(1.0f);

		dgFloat32 accNorm;
		accNorm = maxAccNorm * dgFloat32(2.0f);
		for (dgInt32 passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm);
			 passes++) {
			simd_type accNormSimd = zero;
			for (dgInt32 curJoint = 0; curJoint < m_jointCount; curJoint++) {
				dgInt32 index = constraintArray[curJoint].m_autoPairstart;
				dgInt32 rowsCount = constraintArray[curJoint].m_autoPaircount;
				dgInt32 m0 = constraintArray[curJoint].m_m0;
				dgInt32 m1 = constraintArray[curJoint].m_m1;

				simd_type linearM0 = (simd_type &)internalForces[m0].m_linear;
				simd_type angularM0 = (simd_type &)internalForces[m0].m_angular;
				simd_type linearM1 = (simd_type &)internalForces[m1].m_linear;
				simd_type angularM1 = (simd_type &)internalForces[m1].m_angular;
				for (dgInt32 k = 0; k < rowsCount; k++) {
					//				dgVector acc (m_JMinv[index].m_jacobian_IM0.m_linear.CompProduct(linearM0));
					//				acc += m_JMinv[index].m_jacobian_IM0.m_angular.CompProduct (angularM0);
					//				acc += m_JMinv[index].m_jacobian_IM1.m_linear.CompProduct (linearM1);
					//				acc += m_JMinv[index].m_jacobian_IM1.m_angular.CompProduct (angularM1);
					simd_type a =
						simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, linearM0);
					a =
						simd_mul_add_v(a, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, angularM0);
					a =
						simd_mul_add_v(a, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, linearM1);
					a =
						simd_mul_add_v(a, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, angularM1);

					// a = coordenateAccel[index] - acc.m_x - acc.m_y - acc.m_z - force[index] * diagDamp[index];
					a = simd_add_v(a, simd_move_hl_v(a, a));
					a = simd_add_s(a, simd_permut_v(a, a, PURMUT_MASK(3, 3, 3, 1)));
					a =
						simd_sub_s(simd_load_s(coordenateAccel[index]), simd_mul_add_s(a, simd_load_s(force[index]), simd_load_s(diagDamp[index])));

					// f = force[index] + invDJMinvJt[index] * a;
					simd_type f =
						simd_mul_add_s(simd_load_s(force[index]), simd_load_s(invDJMinvJt[index]), a);

					dgInt32 frictionIndex = m_normalForceIndex[index];
					_ASSERTE(
						((frictionIndex < 0) && (force[frictionIndex] == dgFloat32(1.0f))) || ((frictionIndex >= 0) && (force[frictionIndex] >= dgFloat32(0.0f))));

					// frictionNormal = force[frictionIndex];
					// lowerFrictionForce = frictionNormal * lowerFrictionCoef[index];
					// upperFrictionForce = frictionNormal * upperFrictionCoef[index];
					simd_type frictionNormal = simd_load_s(force[frictionIndex]);
					simd_type lowerFrictionForce =
						simd_mul_s(frictionNormal, simd_load_s(lowerFrictionCoef[index]));
					simd_type upperFrictionForce =
						simd_mul_s(frictionNormal, simd_load_s(upperFrictionCoef[index]));

					// if (f > upperFrictionForce) {
					//	a = dgFloat32 (0.0f);
					//	f = upperFrictionForce;
					// } else if (f < lowerFrictionForce) {
					//	a = dgFloat32 (0.0f);
					//	f = lowerFrictionForce;
					// }
					f =
						simd_min_s(simd_max_s(f, lowerFrictionForce), upperFrictionForce);
					a =
						simd_andnot_v(a, simd_or_v(simd_cmplt_s(f, lowerFrictionForce), simd_cmpgt_s(f, upperFrictionForce)));
					accNormSimd = simd_max_s(accNormSimd, simd_and_v(a, signMask));

					// prevValue = f - force[index]);
					a = simd_sub_s(f, simd_load_s(force[index]));
					a = simd_permut_v(a, a, PURMUT_MASK(0, 0, 0, 0));

					// force[index] = f;
					simd_store_s(f, &force[index]);

					linearM0 =
						simd_mul_add_v(linearM0, (simd_type &)Jt[index].m_jacobian_IM0.m_linear, a);
					angularM0 =
						simd_mul_add_v(angularM0, (simd_type &)Jt[index].m_jacobian_IM0.m_angular, a);
					linearM1 =
						simd_mul_add_v(linearM1, (simd_type &)Jt[index].m_jacobian_IM1.m_linear, a);
					angularM1 =
						simd_mul_add_v(angularM1, (simd_type &)Jt[index].m_jacobian_IM1.m_angular, a);
					index++;
				}

				// internalForces[prevM0].m_linear += Jt[prevIndex].m_jacobian_IM0.m_linear.Scale (prevValue);
				// internalForces[prevM0].m_angular += Jt[prevIndex].m_jacobian_IM0.m_angular.Scale (prevValue);
				// internalForces[prevM1].m_linear += Jt[prevIndex].m_jacobian_IM1.m_linear.Scale (prevValue);
				// internalForces[prevM1].m_angular += Jt[prevIndex].m_jacobian_IM1.m_angular.Scale (prevValue);
				(simd_type &)internalForces[m0].m_linear = linearM0;
				(simd_type &)internalForces[m0].m_angular = angularM0;
				(simd_type &)internalForces[m1].m_linear = linearM1;
				(simd_type &)internalForces[m1].m_angular = angularM1;
			}
			simd_store_s(accNormSimd, &accNorm);
		}

		for (dgInt32 i = 1; i < m_bodyCount; i++) {
			dgBody *const body = bodyArray[i].m_body;
			// dgVector force (body->m_accel + internalForces[i].m_linear);
			// dgVector torque (body->m_alpha + internalForces[i].m_angular);

			simd_type force =
				simd_add_v((simd_type &)body->m_accel, (simd_type &)internalForces[i].m_linear);
			simd_type torque =
				simd_add_v((simd_type &)body->m_alpha, (simd_type &)internalForces[i].m_angular);

			// dgVector accel (force.Scale (body->m_invMass.m_w));
			simd_type accel = simd_mul_v(force, simd_set1(body->m_invMass.m_w));

			// dgVector alpha (body->m_invWorldInertiaMatrix.RotateVector (torque));
			simd_type alpha =
				simd_mul_add_v(simd_mul_add_v(simd_mul_v((simd_type &)body->m_invWorldInertiaMatrix[0], simd_permut_v(torque, torque, PURMUT_MASK(0, 0, 0, 0))),
											  (simd_type &)body->m_invWorldInertiaMatrix[1], simd_permut_v(torque, torque, PURMUT_MASK(1, 1, 1, 1))),
							   (simd_type &)body->m_invWorldInertiaMatrix[2], simd_permut_v(torque, torque, PURMUT_MASK(2, 2, 2, 2)));

			// body->m_veloc += accel.Scale(timeStep);
			(simd_type &)body->m_veloc =
				simd_mul_add_v((simd_type &)body->m_veloc, accel, timeStepSimd);
			// body->m_omega += alpha.Scale(timeStep);
			(simd_type &)body->m_omega =
				simd_mul_add_v((simd_type &)body->m_omega, alpha, timeStepSimd);

			// body->m_netForce += body->m_veloc;
			(simd_type &)internalVeloc[i].m_linear =
				simd_add_v((simd_type &)internalVeloc[i].m_linear, (simd_type &)body->m_veloc);
			// body->m_netTorque += body->m_omega;
			(simd_type &)internalVeloc[i].m_angular =
				simd_add_v((simd_type &)internalVeloc[i].m_angular, (simd_type &)body->m_omega);
		}
	}

	dgInt32 hasJointFeeback = 0;
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		//		maxForce = dgFloat32 (0.0f);
		dgInt32 first = constraintArray[i].m_autoPairstart;
		dgInt32 count = constraintArray[i].m_autoPaircount;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 index = j + first;
			dgFloat32 val = force[index];
			_ASSERTE(dgCheckFloat(val));
			//			maxForce = GetMax (dgAbsf (val), maxForce);
			jointForceFeeback[index][0] = val;
		}
		//		if (constraintArray[i].m_joint->GetId() == dgContactConstraintId) {
		//			m_world->AddToBreakQueue ((dgContact*)constraintArray[i].m_joint, maxForce);
		//		}

		//		hasJointFeeback |= dgUnsigned32 (constraintArray[i].m_joint->m_updaFeedbackCallback);
		hasJointFeeback |= (constraintArray[i].m_joint->m_updaFeedbackCallback ? 1 : 0);
		//		if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
		//			constraintArray[i].m_joint->m_updaFeedbackCallback (*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
		//		}
	}

//	simd_type invStepSimd;
//	signMask = simd_set1 (invStep);
#ifdef DG_WIGHT_FINAL_RK4_DERIVATIVES
	simd_type invStepSimd = simd_set1(invStep);
#endif
	simd_type invTimeStepSimd = simd_set1(m_invTimeStep);
	simd_type accelerationTolerance = simd_set1(maxAccNorm);
	accelerationTolerance =
		simd_mul_s(accelerationTolerance, accelerationTolerance);
	for (dgInt32 i = 1; i < m_bodyCount; i++) {
		dgBody *const body = bodyArray[i].m_body;

#ifdef DG_WIGHT_FINAL_RK4_DERIVATIVES
		// body->m_veloc = internalVeloc[i].m_linear.Scale(invStep);
		// body->m_omega = internalVeloc[i].m_angular.Scale(invStep);
		(simd_type &)body->m_veloc = simd_mul_v((simd_type &)internalVeloc[i].m_linear, invStepSimd);
		(simd_type &)body->m_omega = simd_mul_v((simd_type &)internalVeloc[i].m_angular, invStepSimd);
#endif

		// dgVector accel = (body->m_veloc - body->m_netForce).Scale (m_invTimeStep);
		// dgVector alpha = (body->m_omega - body->m_netTorque).Scale (m_invTimeStep);
		simd_type accel = simd_mul_v(simd_sub_v((simd_type &)body->m_veloc, (simd_type &)body->m_netForce), invTimeStepSimd);
		simd_type alpha = simd_mul_v(simd_sub_v((simd_type &)body->m_omega, (simd_type &)body->m_netTorque), invTimeStepSimd);

		// if ((accel % accel) < maxAccNorm2) {
		//	accel = zero;
		// }
		// body->m_accel = accel;
		// body->m_netForce = accel.Scale (body->m_mass[3]);
		simd_type tmp = simd_mul_v(accel, accel);
		tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
		tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
		tmp = simd_cmplt_s(tmp, accelerationTolerance);
		(simd_type &)body->m_accel =
			simd_andnot_v(accel, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 0)));
		(simd_type &)body->m_netForce =
			simd_mul_v((simd_type &)body->m_accel, simd_set1(body->m_mass[3]));

		// if ((alpha % alpha) < maxAccNorm2) {
		//	alpha = zero;
		// }
		// body->m_alpha = alpha;
		tmp = simd_mul_v(alpha, alpha);
		tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
		tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
		tmp = simd_cmplt_s(tmp, accelerationTolerance);
		(simd_type &)body->m_alpha =
			simd_andnot_v(alpha, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 0)));

		// alpha = body->m_matrix.UnrotateVector(alpha);
		alpha =
			simd_mul_v((simd_type &)body->m_matrix[0], (simd_type &)body->m_alpha);
		alpha = simd_add_v(alpha, simd_move_hl_v(alpha, alpha));
		alpha =
			simd_add_s(alpha, simd_permut_v(alpha, alpha, PURMUT_MASK(0, 0, 0, 1)));

		tmp = simd_mul_v((simd_type &)body->m_matrix[1], (simd_type &)body->m_alpha);
		tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
		tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
		alpha = simd_pack_lo_v(alpha, tmp);

		tmp = simd_mul_v((simd_type &)body->m_matrix[2], (simd_type &)body->m_alpha);
		tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
		tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
		alpha = simd_permut_v(alpha, tmp, PURMUT_MASK(3, 0, 1, 0));

		// body->m_netTorque = body->m_matrix.RotateVector (alpha.CompProduct(body->m_mass));
		alpha = simd_mul_v(alpha, (simd_type &)body->m_mass);
		(simd_type &)body->m_netTorque =
			simd_mul_add_v(simd_mul_add_v(simd_mul_v((simd_type &)body->m_matrix[0], simd_permut_v(alpha, alpha, PURMUT_MASK(0, 0, 0, 0))),
										  (simd_type &)body->m_matrix[1], simd_permut_v(alpha, alpha, PURMUT_MASK(1, 1, 1, 1))),
						   (simd_type &)body->m_matrix[2], simd_permut_v(alpha, alpha, PURMUT_MASK(2, 2, 2, 2)));
	}

	if (hasJointFeeback) {
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
				constraintArray[i].m_joint->m_updaFeedbackCallback(
					*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
			}
		}
	}
#endif
}

void dgJacobianMemory::CalculateForcesGameMode(dgInt32 iterations,
											   dgFloat32 maxAccNorm) const {
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	const dgBodyInfo *bodyArray = m_bodyArray;
	dgFloat32 *const penetration = m_penetration;
	const dgFloat32 *const externAccel = m_deltaAccel;
	const dgFloat32 *const restitution = m_restitution;
	dgFloat32 *const coordenateAccel = m_coordenateAccel;
	dgJacobian *const internalVeloc = m_internalVeloc;
	dgJacobian *const internalForces = m_internalForces;
	;
	dgFloat32 **const jointForceFeeback = m_jointFeebackForce;
	const dgInt32 *const accelIsMortor = m_accelIsMotor;
	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const penetrationStiffness = m_penetrationStiffness;
	const dgFloat32 *const lowerFrictionCoef = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFrictionCoef = m_upperBoundFrictionCoefficent;
	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));

	//	steps = 4;
	dgFloat32 invStep = (dgFloat32(1.0f) / dgFloat32(LINEAR_SOLVER_SUB_STEPS));
	dgFloat32 timeStep = m_timeStep * invStep;
	dgFloat32 invTimeStep = m_invTimeStep * dgFloat32(LINEAR_SOLVER_SUB_STEPS);

	_ASSERTE(m_bodyArray[0].m_body == m_world->m_sentionelBody);
	for (dgInt32 i = 1; i < m_bodyCount; i++) {
		dgBody *const body = m_bodyArray[i].m_body;

		body->m_netForce = body->m_veloc;
		body->m_netTorque = body->m_omega;
		internalVeloc[i].m_linear = zero;
		internalVeloc[i].m_angular = zero;
		internalForces[i].m_linear = zero;
		internalForces[i].m_angular = zero;
	}

	internalVeloc[0].m_linear = zero;
	internalVeloc[0].m_angular = zero;
	internalForces[0].m_linear = zero;
	internalForces[0].m_angular = zero;

	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgJacobian y0;
		dgJacobian y1;

		dgInt32 first = constraintArray[i].m_autoPairstart;
		dgInt32 count = constraintArray[i].m_autoPaircount;

		dgInt32 m0 = constraintArray[i].m_m0;
		dgInt32 m1 = constraintArray[i].m_m1;

		y0.m_linear = zero;
		y0.m_angular = zero;
		y1.m_linear = zero;
		y1.m_angular = zero;
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 index = j + first;
			dgFloat32 val = force[index];
			_ASSERTE(dgCheckFloat(val));
			y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale(val);
			y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale(val);
			y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale(val);
			y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale(val);
		}
		internalForces[m0].m_linear += y0.m_linear;
		internalForces[m0].m_angular += y0.m_angular;
		internalForces[m1].m_linear += y1.m_linear;
		internalForces[m1].m_angular += y1.m_angular;
	}

	//	if (sleepCount) {
	//		for (dgInt32 i = 1; i < m_bodyCount; i ++) {
	//			dgBody* body;
	//			body = m_bodyArray[i].m_body;
	//			if (body->m_equilibrium)
	//				body = m_bodyArray[i].m_body;
	//		}
	//	}

	dgFloat32 firstPassCoef = dgFloat32(0.0f);
	dgInt32 maxPasses = iterations + DG_BASE_ITERATION_COUNT;
	for (dgInt32 step = 0; step < LINEAR_SOLVER_SUB_STEPS; step++) {
		for (dgInt32 curJoint = 0; curJoint < m_jointCount; curJoint++) {
			dgJointAccelerationDecriptor joindDesc;

			dgInt32 index = constraintArray[curJoint].m_autoPairstart;
			joindDesc.m_rowsCount = constraintArray[curJoint].m_autoPaircount;

			joindDesc.m_timeStep = timeStep;
			joindDesc.m_invTimeStep = invTimeStep;
			joindDesc.m_firstPassCoefFlag = firstPassCoef;

			joindDesc.m_Jt = &Jt[index];
			joindDesc.m_penetration = &penetration[index];
			joindDesc.m_restitution = &restitution[index];
			joindDesc.m_externAccelaration = &externAccel[index];
			joindDesc.m_coordenateAccel = &coordenateAccel[index];
			joindDesc.m_accelIsMotor = &accelIsMortor[index];
			joindDesc.m_normalForceIndex = &normalForceIndex[index];
			joindDesc.m_penetrationStiffness = &penetrationStiffness[index];
			constraintArray[curJoint].m_joint->JointAccelerations(joindDesc);
		}
		firstPassCoef = dgFloat32(1.0f);

		dgFloat32 accNorm = maxAccNorm * dgFloat32(2.0f);
		for (dgInt32 passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm);
			 passes++) {
			accNorm = dgFloat32(0.0f);
			for (dgInt32 curJoint = 0; curJoint < m_jointCount; curJoint++) {
				dgInt32 index = constraintArray[curJoint].m_autoPairstart;
				dgInt32 rowsCount = constraintArray[curJoint].m_autoPaircount;
				dgInt32 m0 = constraintArray[curJoint].m_m0;
				dgInt32 m1 = constraintArray[curJoint].m_m1;

				dgVector linearM0(internalForces[m0].m_linear);
				dgVector angularM0(internalForces[m0].m_angular);
				dgVector linearM1(internalForces[m1].m_linear);
				dgVector angularM1(internalForces[m1].m_angular);
				for (dgInt32 k = 0; k < rowsCount; k++) {
					dgVector acc(
						JMinv[index].m_jacobian_IM0.m_linear.CompProduct(linearM0));
					acc += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(angularM0);
					acc += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(linearM1);
					acc += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(angularM1);

					dgFloat32 a = coordenateAccel[index] - acc.m_x - acc.m_y - acc.m_z - force[index] * diagDamp[index];
					dgFloat32 f = force[index] + invDJMinvJt[index] * a;

					dgInt32 frictionIndex = normalForceIndex[index];
					_ASSERTE(
						((frictionIndex < 0) && (force[frictionIndex] == dgFloat32(1.0f))) || ((frictionIndex >= 0) && (force[frictionIndex] >= dgFloat32(0.0f))));

					dgFloat32 frictionNormal = force[frictionIndex];
					dgFloat32 lowerFrictionForce = frictionNormal * lowerFrictionCoef[index];
					dgFloat32 upperFrictionForce = frictionNormal * upperFrictionCoef[index];

					if (f > upperFrictionForce) {
						a = dgFloat32(0.0f);
						f = upperFrictionForce;
					} else if (f < lowerFrictionForce) {
						a = dgFloat32(0.0f);
						f = lowerFrictionForce;
					}

					accNorm = GetMax(accNorm, dgAbsf(a));
					dgFloat32 prevValue = f - force[index];
					force[index] = f;

					linearM0 += Jt[index].m_jacobian_IM0.m_linear.Scale(prevValue);
					angularM0 += Jt[index].m_jacobian_IM0.m_angular.Scale(prevValue);
					linearM1 += Jt[index].m_jacobian_IM1.m_linear.Scale(prevValue);
					angularM1 += Jt[index].m_jacobian_IM1.m_angular.Scale(prevValue);
					index++;
				}

				internalForces[m0].m_linear = linearM0;
				internalForces[m0].m_angular = angularM0;
				internalForces[m1].m_linear = linearM1;
				internalForces[m1].m_angular = angularM1;
			}
			// accNorm  = 1.0f;
		}

		for (dgInt32 i = 1; i < m_bodyCount; i++) {
			dgBody *const body = bodyArray[i].m_body;
			dgVector force(body->m_accel + internalForces[i].m_linear);
			dgVector torque(body->m_alpha + internalForces[i].m_angular);

			dgVector accel(force.Scale(body->m_invMass.m_w));
			dgVector alpha(body->m_invWorldInertiaMatrix.RotateVector(torque));
			body->m_veloc += accel.Scale(timeStep);
			body->m_omega += alpha.Scale(timeStep);
			internalVeloc[i].m_linear += body->m_veloc;
			internalVeloc[i].m_angular += body->m_omega;
		}
	}

	dgInt32 hasJointFeeback = 0;
	for (dgInt32 i = 0; i < m_jointCount; i++) {
		dgInt32 first = constraintArray[i].m_autoPairstart;
		dgInt32 count = constraintArray[i].m_autoPaircount;

		//		maxForce = dgFloat32 (0.0f);
		for (dgInt32 j = 0; j < count; j++) {
			dgInt32 index = j + first;
			dgFloat32 val = force[index];
			//			maxForce = GetMax (dgAbsf (val), maxForce);
			_ASSERTE(dgCheckFloat(val));
			jointForceFeeback[index][0] = val;
		}
		//		if (constraintArray[i].m_joint->GetId() == dgContactConstraintId) {
		//			m_world->AddToBreakQueue ((dgContact*)constraintArray[i].m_joint, maxForce);
		//		}

		hasJointFeeback |= (constraintArray[i].m_joint->m_updaFeedbackCallback ? 1 : 0);
		//		if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
		//			constraintArray[i].m_joint->m_updaFeedbackCallback (*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
		//		}
	}

	dgFloat32 maxAccNorm2 = maxAccNorm * maxAccNorm;
	for (dgInt32 i = 1; i < m_bodyCount; i++) {
		dgBody *const body = bodyArray[i].m_body;

#ifdef DG_WIGHT_FINAL_RK4_DERIVATIVES
		body->m_veloc = internalVeloc[i].m_linear.Scale(invStep);
		body->m_omega = internalVeloc[i].m_angular.Scale(invStep);
#endif

		dgVector accel = (body->m_veloc - body->m_netForce).Scale(m_invTimeStep);
		dgVector alpha = (body->m_omega - body->m_netTorque).Scale(m_invTimeStep);
		if ((accel % accel) < maxAccNorm2) {
			accel = zero;
		}

		if ((alpha % alpha) < maxAccNorm2) {
			alpha = zero;
		}

		body->m_accel = accel;
		body->m_alpha = alpha;
		body->m_netForce = accel.Scale(body->m_mass[3]);

		alpha = body->m_matrix.UnrotateVector(alpha);
		body->m_netTorque = body->m_matrix.RotateVector(
			alpha.CompProduct(body->m_mass));
	}

	if (hasJointFeeback) {
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
				constraintArray[i].m_joint->m_updaFeedbackCallback(
					*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
//  Parallel solver
//
////////////////////////////////////////////////////////////////////////////////////////

void dgParallelSolverBodyInertia::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	if (m_useSimd) {
		for (dgInt32 j = 0; j < m_count; j++) {
			dgBody *body;
			body = m_bodyArray[j].m_body;
			_ASSERTE(body->m_invMass.m_w > dgFloat32(0.0f));
			body->AddDamingAcceleration();
			body->CalcInvInertiaMatrixSimd();
		}
	} else {
		for (dgInt32 j = 0; j < m_count; j++) {
			dgBody *body;
			body = m_bodyArray[j].m_body;
			_ASSERTE(body->m_invMass.m_w > dgFloat32(0.0f));
			body->AddDamingAcceleration();
			body->CalcInvInertiaMatrix();
		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverBuildJacobianRows::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	dgJacobianPair *const JMinv = m_JMinv;
	dgFloat32 *const diagDamp = m_diagDamp;
	dgFloat32 *const extAccel = m_deltaAccel;
	dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	dgFloat32 *const coordenateAccel = m_coordenateAccel;
	dgFloat32 **const jointForceFeeback = m_jointFeebackForce;
	//	dgInt32* const accelIsMotor = solverMemory.m_accelIsMotor;

	dgBodyInfo *const bodyArray = m_bodyArray;
	dgJointInfo *const constraintArray = m_constraintArray;

	if (m_useSimd) {
#ifdef DG_BUILD_SIMD_CODE
		simd_type zero;
		zero = simd_set1(dgFloat32(0.0f));
		for (dgInt32 k = 0; k < m_count; k++) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 index;
			dgInt32 count;
			dgFloat32 diag;
			dgFloat32 stiffness;
			simd_type tmp0;
			simd_type tmp1;
			simd_type invMass0;
			simd_type invMass1;
			simd_type tmpDiag;
			simd_type tmpAccel;
			dgBody *body0;
			dgBody *body1;

			index = constraintArray[k].m_autoPairstart;
			count = constraintArray[k].m_autoPaircount;
			m0 = constraintArray[k].m_m0;
			m1 = constraintArray[k].m_m1;
			_ASSERTE(m0 >= 0);
			_ASSERTE(m0 < m_bodyCount);
			_ASSERTE(m1 >= 0);
			_ASSERTE(m1 < m_bodyCount);

			body0 = bodyArray[m0].m_body;
			// invMass0 = body0->m_invMass[3];
			invMass0 = simd_set1(body0->m_invMass[3]);
			_ASSERTE((dgUnsigned64(&body0->m_invWorldInertiaMatrix) & 0x0f) == 0);
			const dgMatrix &invInertia0 = body0->m_invWorldInertiaMatrix;

			body1 = bodyArray[m1].m_body;
			// invMass1 = body1->m_invMass[3];
			invMass1 = simd_set1(body1->m_invMass[3]);
			_ASSERTE((dgUnsigned64(&body1->m_invWorldInertiaMatrix) & 0x0f) == 0);
			const dgMatrix &invInertia1 = body1->m_invWorldInertiaMatrix;
			for (dgInt32 i = 0; i < count; i++) {
				// JMinv[index].m_jacobian_IM0.m_linear = Jt[index].m_jacobian_IM0.m_linear.Scale (invMass0);
				// JMinv[index].m_jacobian_IM0.m_angular = invInertia0.UnrotateVector (Jt[index].m_jacobian_IM0.m_angular);
				// dgVector tmpDiag (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(Jt[index].m_jacobian_IM0.m_linear));
				// tmpDiag += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(Jt[index].m_jacobian_IM0.m_angular);
				// dgVector tmpAccel (JMinv[index].m_jacobian_IM0.m_linear.CompProduct(body0->m_accel));
				// tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(body0->m_alpha);

				((simd_type &)JMinv[index].m_jacobian_IM0.m_linear) =
					simd_mul_v((simd_type &)Jt[index].m_jacobian_IM0.m_linear, invMass0);
				tmp0 = (simd_type &)Jt[index].m_jacobian_IM0.m_angular;
				tmp1 =
					simd_mul_v((simd_type &)invInertia0.m_front, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 0, 0, 0)));
				tmp1 =
					simd_mul_add_v(tmp1, (simd_type &)invInertia0.m_up, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 1, 1, 1)));
				((simd_type &)JMinv[index].m_jacobian_IM0.m_angular) =
					simd_mul_add_v(tmp1, (simd_type &)invInertia0.m_right, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 2, 2, 2)));
				tmpDiag =
					simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, (simd_type &)Jt[index].m_jacobian_IM0.m_linear);
				tmpDiag =
					simd_mul_add_v(tmpDiag, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, (simd_type &)Jt[index].m_jacobian_IM0.m_angular);
				tmpAccel =
					simd_mul_v((simd_type &)JMinv[index].m_jacobian_IM0.m_linear, (simd_type &)body0->m_accel);
				tmpAccel =
					simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM0.m_angular, (simd_type &)body0->m_alpha);

				// JMinv[index].m_jacobian_IM1.m_linear = Jt[index].m_jacobian_IM1.m_linear.Scale (invMass1);
				// JMinv[index].m_jacobian_IM1.m_angular = invInertia1.UnrotateVector (Jt[index].m_jacobian_IM1.m_angular);
				// tmpDiag += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(Jt[index].m_jacobian_IM1.m_linear);
				// tmpDiag += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(Jt[index].m_jacobian_IM1.m_angular);
				// tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(body1->m_accel);
				// tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(body1->m_alpha);

				((simd_type &)JMinv[index].m_jacobian_IM1.m_linear) =
					simd_mul_v((simd_type &)Jt[index].m_jacobian_IM1.m_linear, invMass1);
				tmp0 = (simd_type &)Jt[index].m_jacobian_IM1.m_angular;
				tmp1 =
					simd_mul_v((simd_type &)invInertia1.m_front, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 0, 0, 0)));
				tmp1 =
					simd_mul_add_v(tmp1, (simd_type &)invInertia1.m_up, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 1, 1, 1)));
				((simd_type &)JMinv[index].m_jacobian_IM1.m_angular) =
					simd_mul_add_v(tmp1, (simd_type &)invInertia1.m_right, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 2, 2, 2)));
				tmpDiag =
					simd_mul_add_v(tmpDiag, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, (simd_type &)Jt[index].m_jacobian_IM1.m_linear);
				tmpDiag =
					simd_mul_add_v(tmpDiag, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, (simd_type &)Jt[index].m_jacobian_IM1.m_angular);
				tmpAccel =
					simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM1.m_linear, (simd_type &)body1->m_accel);
				tmpAccel =
					simd_mul_add_v(tmpAccel, (simd_type &)JMinv[index].m_jacobian_IM1.m_angular, (simd_type &)body1->m_alpha);

				// coordenateAccel[index] -= (tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z);
				tmpAccel = simd_add_v(tmpAccel, simd_move_hl_v(tmpAccel, tmpAccel));
				tmpAccel =
					simd_sub_s(zero, simd_add_s(tmpAccel, simd_permut_v(tmpAccel, tmpAccel, PURMUT_MASK(3, 3, 3, 1))));

				simd_store_s(tmpAccel, &extAccel[index]);
				simd_store_s(simd_add_s(simd_load_s(coordenateAccel[index]), tmpAccel),
							 &coordenateAccel[index]);

				// force[index] = bilateralForceBounds[index].m_jointForce[0];
				force[index] = jointForceFeeback[index][0];

				_ASSERTE(diagDamp[index] >= dgFloat32(0.1f));
				_ASSERTE(diagDamp[index] <= dgFloat32(100.0f));
				stiffness = DG_PSD_DAMP_TOL * diagDamp[index];

				// diag = (tmpDiag.m_x + tmpDiag.m_y + tmpDiag.m_z);
				tmpDiag = simd_add_v(tmpDiag, simd_move_hl_v(tmpDiag, tmpDiag));
				simd_store_s(
					simd_add_s(tmpDiag, simd_permut_v(tmpDiag, tmpDiag, PURMUT_MASK(3, 3, 3, 1))),
					&diag);
				_ASSERTE(diag > dgFloat32(0.0f));
				diagDamp[index] = diag * stiffness;

				diag *= (dgFloat32(1.0f) + stiffness);
				invDJMinvJt[index] = dgFloat32(1.0f) / diag;

				index++;
			}
		}
#endif
	} else {
		for (dgInt32 k = 0; k < m_count; k++) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 index;
			dgInt32 count;
			dgFloat32 invMass0;
			dgFloat32 invMass1;
			dgBody *body0;
			dgBody *body1;

			index = constraintArray[k].m_autoPairstart;
			count = constraintArray[k].m_autoPaircount;
			m0 = constraintArray[k].m_m0;
			m1 = constraintArray[k].m_m1;

			_ASSERTE(m0 >= 0);
			_ASSERTE(m0 < m_bodyCount);
			body0 = bodyArray[m0].m_body;
			invMass0 = body0->m_invMass[3];
			const dgMatrix &invInertia0 = body0->m_invWorldInertiaMatrix;

			_ASSERTE(m1 >= 0);
			_ASSERTE(m1 < m_bodyCount);
			body1 = bodyArray[m1].m_body;
			invMass1 = body1->m_invMass[3];
			const dgMatrix &invInertia1 = body1->m_invWorldInertiaMatrix;

			for (dgInt32 i = 0; i < count; i++) {
				dgFloat32 diag;
				dgFloat32 stiffness;
				dgFloat32 extenalAcceleration;

				JMinv[index].m_jacobian_IM0.m_linear =
					Jt[index].m_jacobian_IM0.m_linear.Scale(invMass0);
				JMinv[index].m_jacobian_IM0.m_angular = invInertia0.UnrotateVector(
					Jt[index].m_jacobian_IM0.m_angular);
				dgVector tmpDiag(
					JMinv[index].m_jacobian_IM0.m_linear.CompProduct(
						Jt[index].m_jacobian_IM0.m_linear));
				tmpDiag += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(
					Jt[index].m_jacobian_IM0.m_angular);
				dgVector tmpAccel(
					JMinv[index].m_jacobian_IM0.m_linear.CompProduct(body0->m_accel));
				tmpAccel += JMinv[index].m_jacobian_IM0.m_angular.CompProduct(
					body0->m_alpha);

				JMinv[index].m_jacobian_IM1.m_linear =
					Jt[index].m_jacobian_IM1.m_linear.Scale(invMass1);
				JMinv[index].m_jacobian_IM1.m_angular = invInertia1.UnrotateVector(
					Jt[index].m_jacobian_IM1.m_angular);
				tmpDiag += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(
					Jt[index].m_jacobian_IM1.m_linear);
				tmpDiag += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(
					Jt[index].m_jacobian_IM1.m_angular);
				tmpAccel += JMinv[index].m_jacobian_IM1.m_linear.CompProduct(
					body1->m_accel);
				tmpAccel += JMinv[index].m_jacobian_IM1.m_angular.CompProduct(
					body1->m_alpha);

				extenalAcceleration = -(tmpAccel.m_x + tmpAccel.m_y + tmpAccel.m_z);
				extAccel[index] = extenalAcceleration;
				coordenateAccel[index] += extenalAcceleration;
				force[index] = jointForceFeeback[index][0];

				_ASSERTE(diagDamp[index] >= dgFloat32(0.1f));
				_ASSERTE(diagDamp[index] <= dgFloat32(100.0f));
				stiffness = DG_PSD_DAMP_TOL * diagDamp[index];

				diag = (tmpDiag.m_x + tmpDiag.m_y + tmpDiag.m_z);
				_ASSERTE(diag > dgFloat32(0.0f));
				diagDamp[index] = diag * stiffness;

				diag *= (dgFloat32(1.0f) + stiffness);
				// solverMemory.m_diagJMinvJt[index] = diag;
				invDJMinvJt[index] = dgFloat32(1.0f) / diag;
				index++;
			}
		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverBuildJacobianMatrix::ThreadExecute() {
	//	dgInt32 count;
	//	dgInt32 rowCount;
	//	dgInt32 jointSolved;
	dgContraintDescritor constraintParams;
	dgJointInfo *const constraintArray = m_constraintArray;

	//	ticks = m_world->m_getPerformanceCount();

	constraintParams.m_world = m_world;
	constraintParams.m_threadIndex = m_threadIndex;
	constraintParams.m_timestep = m_timestep;
	constraintParams.m_invTimestep = m_invTimestep;

	dgJacobianMemory &solverMemory = *m_solverMemory;

	dgInt32 count = m_count;
	dgInt32 jointSolved = m_jointSolved;
	for (dgInt32 j = 0; j < count; j++) {
		dgConstraint *const constraint = constraintArray[j].m_joint;

		if (constraint->m_isUnilateral ^ m_bitMode) {

			dgInt32 dof = dgInt32(constraint->m_maxDOF);
			jointSolved++;

			_ASSERTE(dof <= DG_CONSTRAINT_MAX_ROWS);
			for (dgInt32 i = 0; i < dof; i++) {
				constraintParams.m_forceBounds[i].m_low = DG_MIN_BOUND;
				constraintParams.m_forceBounds[i].m_upper = DG_MAX_BOUND;
				constraintParams.m_forceBounds[i].m_jointForce = NULL;
				constraintParams.m_forceBounds[i].m_normalIndex =
					DG_BILATERAL_CONSTRAINT;
			}

			_ASSERTE(constraint->m_body0);
			_ASSERTE(constraint->m_body1);

			constraint->m_body0->m_inCallback = true;
			constraint->m_body1->m_inCallback = true;

			dof = dgInt32(constraint->JacobianDerivative(constraintParams));

			constraint->m_body0->m_inCallback = false;
			constraint->m_body1->m_inCallback = false;

			dgInt32 m0 =
				(constraint->m_body0->m_invMass.m_w != dgFloat32(0.0f)) ? constraint->m_body0->m_index : 0;
			dgInt32 m1 =
				(constraint->m_body1->m_invMass.m_w != dgFloat32(0.0f)) ? constraint->m_body1->m_index : 0;

			m_world->dgGetUserLock();
			dgInt32 rowCount = m_rowsCount[0];
			m_rowsCount[0] = rowCount + ((dof & (DG_SIMD_WORD_SIZE - 1)) ? ((dof & (-DG_SIMD_WORD_SIZE)) + DG_SIMD_WORD_SIZE) : dof);
			m_world->dgReleasedUserLock();

			constraintArray[j].m_autoPairstart = rowCount;
			constraintArray[j].m_autoPaircount = dgInt32(dof);
			constraintArray[j].m_autoPairActiveCount = dgInt32(dof);
			constraintArray[j].m_m0 = m0;
			constraintArray[j].m_m1 = m1;

			for (dgInt32 i = 0; i < dof; i++) {
				_ASSERTE(constraintParams.m_forceBounds[i].m_jointForce);
				solverMemory.m_Jt[rowCount] = constraintParams.m_jacobian[i];

				_ASSERTE(constraintParams.m_jointStiffness[i] >= dgFloat32(0.1f));
				_ASSERTE(constraintParams.m_jointStiffness[i] <= dgFloat32(100.0f));
				solverMemory.m_diagDamp[rowCount] =
					constraintParams.m_jointStiffness[i];
				solverMemory.m_coordenateAccel[rowCount] =
					constraintParams.m_jointAccel[i];
				solverMemory.m_accelIsMotor[rowCount] = dgInt32(
					constraintParams.m_isMotor[i]);
				solverMemory.m_restitution[rowCount] =
					constraintParams.m_restitution[i];
				solverMemory.m_penetration[rowCount] =
					constraintParams.m_penetration[i];
				solverMemory.m_penetrationStiffness[rowCount] =
					constraintParams.m_penetrationStiffness[i];

				solverMemory.m_lowerBoundFrictionCoefficent[rowCount] =
					constraintParams.m_forceBounds[i].m_low;
				solverMemory.m_upperBoundFrictionCoefficent[rowCount] =
					constraintParams.m_forceBounds[i].m_upper;
				solverMemory.m_jointFeebackForce[rowCount] =
					constraintParams.m_forceBounds[i].m_jointForce;
				solverMemory.m_normalForceIndex[rowCount] =
					constraintParams.m_forceBounds[i].m_normalIndex + ((constraintParams.m_forceBounds[i].m_normalIndex >= 0) ? (rowCount - i) : 0);

				rowCount++;
			}

#ifdef _DEBUG
			for (dgInt32 i = 0; i < ((rowCount + 3) & 0xfffc) - rowCount; i++) {
				solverMemory.m_diagDamp[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_coordenateAccel[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_restitution[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_penetration[rowCount + i] = dgFloat32(0.0f);
				solverMemory.m_penetrationStiffness[rowCount + i] = dgFloat32(0.0f);

				solverMemory.m_lowerBoundFrictionCoefficent[rowCount + i] = dgFloat32(
					0.0f);
				solverMemory.m_upperBoundFrictionCoefficent[rowCount + i] = dgFloat32(
					0.0f);
				solverMemory.m_jointFeebackForce[rowCount + i] = 0;
				solverMemory.m_normalForceIndex[rowCount + i] = 0;
			}
#endif

			//			rowCount = (rowCount & (DG_SIMD_WORD_SIZE - 1)) ? ((rowCount & (-DG_SIMD_WORD_SIZE)) + DG_SIMD_WORD_SIZE) : rowCount;
			//			_ASSERTE ((rowCount & (DG_SIMD_WORD_SIZE - 1)) == 0);
		}
	}

	m_jointSolved = jointSolved;

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

dgInt32 dgWorldDynamicUpdate::GetJacobialDerivativesParallel(
	const dgIsland &island, bool bitMode, dgInt32 rowCount, dgFloat32 timestep) {
	dgInt32 chunkSizes[DG_MAXIMUN_THREADS];
	dgJointInfo *const constraintArray = &m_constraintArray[island.m_jointStart];

	dgInt32 acc = 0;
	dgInt32 threadCounts = dgInt32(m_world->m_numberOfTheads);
	dgInt32 jointCount = island.m_jointCount;
	m_world->m_threadsManager.CalculateChunkSizes(jointCount, chunkSizes);
	for (dgInt32 threadIndex = 0; threadIndex < threadCounts; threadIndex++) {
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_bitMode = bitMode;
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_world = m_world;
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_jointSolved = 0;
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_count =
			chunkSizes[threadIndex];
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_timestep = timestep;
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_invTimestep = dgFloat32(
			1.0f / timestep);
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_threadIndex =
			threadIndex;
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_rowsCount = &rowCount;
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_constraintArray =
			&constraintArray[acc];
		m_parallelSolverBuildJacobianMatrix[threadIndex].m_solverMemory =
			&m_solverMemory[0];
		m_world->m_threadsManager.SubmitJob(
			&m_parallelSolverBuildJacobianMatrix[threadIndex]);
		acc += chunkSizes[threadIndex];
	}
	m_world->m_threadsManager.SynchronizationBarrier();

	return rowCount;
}

void dgWorldDynamicUpdate::BuildJacobianMatrixParallel(const dgIsland &island,
													   dgFloat32 timestep, dgInt32 archModel) {
	dgVector zeroVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						dgFloat32(0.0f));

	_ASSERTE(island.m_bodyCount >= 2);
	//_ASSERTE (island.m_jointCount >= 1);

	dgInt32 threads = dgInt32(m_world->m_numberOfTheads);
	dgJacobianMemory &solverMemory = m_solverMemory[0];

	dgInt32 bodyCount = island.m_bodyCount;
	dgBodyInfo *const bodyArray = &m_bodyArray[island.m_bodyStart];

	bodyArray[0].m_body = m_world->GetSentinelBody();
	_ASSERTE(
		(bodyArray[0].m_body->m_accel % bodyArray[0].m_body->m_accel) == dgFloat32(0.0f));
	_ASSERTE(
		(bodyArray[0].m_body->m_alpha % bodyArray[0].m_body->m_alpha) == dgFloat32(0.0f));

	dgInt32 acc = 0;
	dgInt32 chunkSizes[DG_MAXIMUN_THREADS];
	m_world->m_threadsManager.CalculateChunkSizes(bodyCount - 1, chunkSizes);
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		m_parallelBodyInertiaMatrix[threadIndex].m_useSimd = archModel;
		m_parallelBodyInertiaMatrix[threadIndex].m_bodyArray = &bodyArray[acc + 1];
		m_parallelBodyInertiaMatrix[threadIndex].m_threadIndex = threadIndex;
		m_parallelBodyInertiaMatrix[threadIndex].m_world = m_world;
		m_parallelBodyInertiaMatrix[threadIndex].m_count = chunkSizes[threadIndex];
		m_world->m_threadsManager.SubmitJob(
			&m_parallelBodyInertiaMatrix[threadIndex]);
		acc += chunkSizes[threadIndex];
	}
	m_world->m_threadsManager.SynchronizationBarrier();

	while (bodyCount > solverMemory.m_maxBodiesCount) {
		m_world->dgGetUserLock();
		ReallocIntenalForcesMemory(bodyCount, 0);
		m_world->dgReleasedUserLock();
	}

	dgInt32 jointCount = island.m_jointCount;
	dgJointInfo *const constraintArray = &m_constraintArray[island.m_jointStart];
	solverMemory.m_constraintArray = constraintArray;

	dgInt32 maxRowCount = 0;
	for (dgInt32 j = 0; j < jointCount; j++) {
		dgConstraint *const constraint = constraintArray[j].m_joint;
		;
		dgInt32 dof =
			dgInt32(
				(constraint->m_maxDOF & (DG_SIMD_WORD_SIZE - 1)) ? ((constraint->m_maxDOF & (-DG_SIMD_WORD_SIZE)) + DG_SIMD_WORD_SIZE) : constraint->m_maxDOF);
		maxRowCount += dof;
	}

	while (maxRowCount > solverMemory.m_maxJacobiansCount) {
		m_world->dgGetUserLock();
		ReallocJacobiansMemory(solverMemory.m_maxJacobiansCount * 2, 0);
		m_world->dgReleasedUserLock();
	}

	dgInt32 rowCount = 0;
	if (island.m_hasUnilateralJoints) {
		rowCount = GetJacobialDerivativesParallel(island, false, rowCount,
												  timestep);
	}
	rowCount = GetJacobialDerivativesParallel(island, true, rowCount, timestep);

	solverMemory.m_rowCount = rowCount;
	solverMemory.m_bodyCount = bodyCount;
	solverMemory.m_bodyArray = bodyArray;
	solverMemory.m_jointCount = jointCount;
	solverMemory.m_timeStep = timestep;
	solverMemory.m_invTimeStep = dgFloat32(1.0f) / solverMemory.m_timeStep;

	acc = 0;
	m_world->m_threadsManager.CalculateChunkSizes(jointCount, chunkSizes);
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		m_parallelSolverBuildJacobianRows[threadIndex].m_useSimd = archModel;
		m_parallelSolverBuildJacobianRows[threadIndex].m_bodyCount = bodyCount;
		m_parallelSolverBuildJacobianRows[threadIndex].m_bodyArray = bodyArray;
		m_parallelSolverBuildJacobianRows[threadIndex].m_threadIndex = threadIndex;
		m_parallelSolverBuildJacobianRows[threadIndex].m_world = m_world;
		m_parallelSolverBuildJacobianRows[threadIndex].m_count =
			chunkSizes[threadIndex];
		m_parallelSolverBuildJacobianRows[threadIndex].m_force =
			solverMemory.m_force;
		m_parallelSolverBuildJacobianRows[threadIndex].m_Jt = solverMemory.m_Jt;
		m_parallelSolverBuildJacobianRows[threadIndex].m_JMinv =
			solverMemory.m_JMinv;
		m_parallelSolverBuildJacobianRows[threadIndex].m_diagDamp =
			solverMemory.m_diagDamp;
		m_parallelSolverBuildJacobianRows[threadIndex].m_deltaAccel =
			solverMemory.m_deltaAccel;
		m_parallelSolverBuildJacobianRows[threadIndex].m_invDJMinvJt =
			solverMemory.m_invDJMinvJt;
		m_parallelSolverBuildJacobianRows[threadIndex].m_coordenateAccel =
			solverMemory.m_coordenateAccel;
		m_parallelSolverBuildJacobianRows[threadIndex].m_jointFeebackForce =
			solverMemory.m_jointFeebackForce;
		m_parallelSolverBuildJacobianRows[threadIndex].m_constraintArray =
			&constraintArray[acc];
		m_world->m_threadsManager.SubmitJob(
			&m_parallelSolverBuildJacobianRows[threadIndex]);
		acc += chunkSizes[threadIndex];
	}
	m_world->m_threadsManager.SynchronizationBarrier();
}

void dgParallelSolverClear::ThreadExecute() {
	if (m_useSimd) {
#ifdef DG_BUILD_SIMD_CODE
		simd_type zero;
		zero = simd_set1(dgFloat32(0.0f));
		for (dgInt32 i = 0; i < m_count; i++) {
			dgBody *body;
			body = m_bodyArray[i].m_body;

			((simd_type &)body->m_netForce) = zero;
			((simd_type &)body->m_netTorque) = zero;
			((simd_type &)m_internalVeloc[i].m_linear) = zero;
			((simd_type &)m_internalVeloc[i].m_angular) = zero;
			((simd_type &)m_internalForces[i].m_linear) = zero;
			((simd_type &)m_internalForces[i].m_angular) = zero;
		}
#endif
	} else {
		dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					  dgFloat32(0.0f));
		for (dgInt32 i = 0; i < m_count; i++) {
			dgBody *body;
			body = m_bodyArray[i].m_body;
			body->m_netForce = body->m_veloc;
			body->m_netTorque = body->m_omega;
			m_internalVeloc[i].m_linear = zero;
			m_internalVeloc[i].m_angular = zero;
			m_internalForces[i].m_linear = zero;
			m_internalForces[i].m_angular = zero;
		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverInitInternalForces::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	if (m_useSimd) {
#ifdef DG_BUILD_SIMD_CODE
		simd_type zero;
		zero = simd_set1(dgFloat32(0.0f));

		for (dgInt32 i = 0; i < m_count; i++) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 first;
			dgInt32 count;

			simd_type y0_linear;
			simd_type y0_angular;
			simd_type y1_linear;
			simd_type y1_angular;

			first = m_constraintArray[i].m_autoPairstart;
			count = m_constraintArray[i].m_autoPairActiveCount;
			m0 = m_constraintArray[i].m_m0;
			m1 = m_constraintArray[i].m_m1;
			// dgJacobian y0 (internalForces[k0]);
			// dgJacobian y1 (internalForces[k1]);
			y0_linear = zero;
			y0_angular = zero;
			y1_linear = zero;
			y1_angular = zero;
			for (dgInt32 j = 0; j < count; j++) {
				dgInt32 index;
				simd_type tmp0;
				index = j + first;
				// val = force[index];
				tmp0 = simd_set1(m_force[index]);
				// y0.m_linear += Jt[index].m_jacobian_IM0.m_linear.Scale (val);
				// y0.m_angular += Jt[index].m_jacobian_IM0.m_angular.Scale (val);
				// y1.m_linear += Jt[index].m_jacobian_IM1.m_linear.Scale (val);
				// y1.m_angular += Jt[index].m_jacobian_IM1.m_angular.Scale (val);
				y0_linear =
					simd_mul_add_v(y0_linear, (simd_type &)m_Jt[index].m_jacobian_IM0.m_linear, tmp0);
				y0_angular =
					simd_mul_add_v(y0_angular, (simd_type &)m_Jt[index].m_jacobian_IM0.m_angular, tmp0);
				y1_linear =
					simd_mul_add_v(y1_linear, (simd_type &)m_Jt[index].m_jacobian_IM1.m_linear, tmp0);
				y1_angular =
					simd_mul_add_v(y1_angular, (simd_type &)m_Jt[index].m_jacobian_IM1.m_angular, tmp0);
			}
			// internalForces[k0] = y0;
			// internalForces[k1] = y1;
			//			m_world->dgGetUserLock();
			m_world->dgGetIndirectLock(&m_locks[m0]);
			(simd_type &)m_internalForces[m0].m_linear =
				simd_add_v((simd_type &)m_internalForces[m0].m_linear, y0_linear);
			(simd_type &)m_internalForces[m0].m_angular =
				simd_add_v((simd_type &)m_internalForces[m0].m_angular, y0_angular);
			m_world->dgReleaseIndirectLock(&m_locks[m0]);

			m_world->dgGetIndirectLock(&m_locks[m1]);
			(simd_type &)m_internalForces[m1].m_linear =
				simd_add_v((simd_type &)m_internalForces[m1].m_linear, y1_linear);
			(simd_type &)m_internalForces[m1].m_angular =
				simd_add_v((simd_type &)m_internalForces[m1].m_angular, y1_angular);
			m_world->dgReleaseIndirectLock(&m_locks[m1]);
			//			m_world->dgReleasedUserLock();
		}
#endif
	} else {
		dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					  dgFloat32(0.0f));
		for (dgInt32 i = 0; i < m_count; i++) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 first;
			dgInt32 count;

			dgFloat32 val;
			dgJacobian y0;
			dgJacobian y1;
			first = m_constraintArray[i].m_autoPairstart;
			count = m_constraintArray[i].m_autoPaircount;

			m0 = m_constraintArray[i].m_m0;
			m1 = m_constraintArray[i].m_m1;

			y0.m_linear = zero;
			y0.m_angular = zero;
			y1.m_linear = zero;
			y1.m_angular = zero;
			for (dgInt32 j = 0; j < count; j++) {
				dgInt32 index;

				index = j + first;
				val = m_force[index];
				_ASSERTE(dgCheckFloat(val));
				y0.m_linear += m_Jt[index].m_jacobian_IM0.m_linear.Scale(val);
				y0.m_angular += m_Jt[index].m_jacobian_IM0.m_angular.Scale(val);
				y1.m_linear += m_Jt[index].m_jacobian_IM1.m_linear.Scale(val);
				y1.m_angular += m_Jt[index].m_jacobian_IM1.m_angular.Scale(val);
			}

			m_world->dgGetIndirectLock(&m_locks[m0]);
			m_internalForces[m0].m_linear += y0.m_linear;
			m_internalForces[m0].m_angular += y0.m_angular;
			m_world->dgReleaseIndirectLock(&m_locks[m0]);

			m_world->dgGetIndirectLock(&m_locks[m1]);
			m_internalForces[m1].m_linear += y1.m_linear;
			m_internalForces[m1].m_angular += y1.m_angular;
			m_world->dgReleaseIndirectLock(&m_locks[m1]);
		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverJointAcceleration::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	if (m_useSimd) {
		dgInt32 currJoint;

		currJoint = m_jointStart;
		for (dgInt32 i = 0; i < m_count; i++) {
			dgInt32 index;

			dgJointAccelerationDecriptor joindDesc;
			index = m_constraintArray[currJoint].m_autoPairstart;
			joindDesc.m_rowsCount = m_constraintArray[currJoint].m_autoPaircount;

			joindDesc.m_timeStep = m_timeStep;
			joindDesc.m_invTimeStep = m_invTimeStep;
			joindDesc.m_firstPassCoefFlag = m_firstPassCoef;

			joindDesc.m_Jt = &m_Jt[index];
			joindDesc.m_penetration = &m_penetration[index];
			joindDesc.m_restitution = &m_restitution[index];
			joindDesc.m_externAccelaration = &m_externAccel[index];
			joindDesc.m_coordenateAccel = &m_coordenateAccel[index];
			joindDesc.m_accelIsMotor = &m_accelIsMortor[index];
			joindDesc.m_normalForceIndex = &m_normalForceIndex[index];
			joindDesc.m_penetrationStiffness = &m_penetrationStiffness[index];
			m_constraintArray[currJoint].m_joint->JointAccelerationsSimd(joindDesc);

			currJoint++;
		}
	} else {
		dgInt32 currJoint;

		currJoint = m_jointStart;
		for (dgInt32 i = 0; i < m_count; i++) {
			dgInt32 index;
			dgJointAccelerationDecriptor joindDesc;

			index = m_constraintArray[currJoint].m_autoPairstart;
			joindDesc.m_rowsCount = m_constraintArray[currJoint].m_autoPaircount;

			joindDesc.m_timeStep = m_timeStep;
			joindDesc.m_invTimeStep = m_invTimeStep;
			joindDesc.m_firstPassCoefFlag = m_firstPassCoef;

			joindDesc.m_Jt = &m_Jt[index];
			joindDesc.m_penetration = &m_penetration[index];
			joindDesc.m_restitution = &m_restitution[index];
			joindDesc.m_externAccelaration = &m_externAccel[index];
			joindDesc.m_coordenateAccel = &m_coordenateAccel[index];
			joindDesc.m_accelIsMotor = &m_accelIsMortor[index];
			joindDesc.m_normalForceIndex = &m_normalForceIndex[index];
			joindDesc.m_penetrationStiffness = &m_penetrationStiffness[index];
			m_constraintArray[currJoint].m_joint->JointAccelerations(joindDesc);

			currJoint++;
		}
	}

	m_firstPassCoef = dgFloat32(1.0f);
	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverUpdateVeloc::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	if (m_useSimd) {
#ifdef DG_BUILD_SIMD_CODE
		simd_type timeStepSimd;
		timeStepSimd = simd_set1(m_timeStep);
		for (dgInt32 i = 0; i < m_count; i++) {
			dgBody *body;
			simd_type force;
			simd_type torque;
			simd_type accel;
			simd_type alpha;

			body = m_bodyArray[i].m_body;
			force =
				simd_add_v((simd_type &)body->m_accel, (simd_type &)m_internalForces[i].m_linear);
			torque =
				simd_add_v((simd_type &)body->m_alpha, (simd_type &)m_internalForces[i].m_angular);

			// dgVector accel (force.Scale (body->m_invMass.m_w));
			accel = simd_mul_v(force, simd_set1(body->m_invMass.m_w));
			// dgVector alpha (body->m_invWorldInertiaMatrix.RotateVector (torque));
			alpha =
				simd_mul_add_v(simd_mul_add_v(simd_mul_v((simd_type &)body->m_invWorldInertiaMatrix[0], simd_permut_v(torque, torque, PURMUT_MASK(0, 0, 0, 0))),
											  (simd_type &)body->m_invWorldInertiaMatrix[1], simd_permut_v(torque, torque, PURMUT_MASK(1, 1, 1, 1))),
							   (simd_type &)body->m_invWorldInertiaMatrix[2], simd_permut_v(torque, torque, PURMUT_MASK(2, 2, 2, 2)));

			// body->m_veloc += accel.Scale(timeStep);
			(simd_type &)body->m_veloc =
				simd_mul_add_v((simd_type &)body->m_veloc, accel, timeStepSimd);
			// body->m_omega += alpha.Scale(timeStep);
			(simd_type &)body->m_omega =
				simd_mul_add_v((simd_type &)body->m_omega, alpha, timeStepSimd);

			// body->m_netForce += body->m_veloc;
			(simd_type &)m_internalVeloc[i].m_linear =
				simd_add_v((simd_type &)m_internalVeloc[i].m_linear, (simd_type &)body->m_veloc);
			// body->m_netTorque += body->m_omega;
			(simd_type &)m_internalVeloc[i].m_angular =
				simd_add_v((simd_type &)m_internalVeloc[i].m_angular, (simd_type &)body->m_omega);
		}
#endif
	} else {

		for (dgInt32 i = 0; i < m_count; i++) {
			dgBody *body;
			body = m_bodyArray[i].m_body;
			dgVector force(body->m_accel + m_internalForces[i].m_linear);
			dgVector torque(body->m_alpha + m_internalForces[i].m_angular);

			dgVector accel(force.Scale(body->m_invMass.m_w));
			dgVector alpha(body->m_invWorldInertiaMatrix.RotateVector(torque));
			body->m_veloc += accel.Scale(m_timeStep);
			body->m_omega += alpha.Scale(m_timeStep);
			m_internalVeloc[i].m_linear += body->m_veloc;
			m_internalVeloc[i].m_angular += body->m_omega;
		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverUpdateForce::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	if (m_useSimd) {
#ifdef DG_BUILD_SIMD_CODE
		simd_type invStepSimd;
		simd_type invTimeStepSimd;
		simd_type accelerationTolerance;
		//	signMask = simd_set1 (invStep);
		invStepSimd = simd_set1(m_invStep);

		invTimeStepSimd = simd_set1(m_invTimeStep);
		accelerationTolerance = simd_set1(m_maxAccNorm2);
		for (dgInt32 i = 0; i < m_count; i++) {
			dgBody *body;
			simd_type tmp;
			simd_type accel;
			simd_type alpha;
			body = m_bodyArray[i].m_body;

#ifdef DG_WIGHT_FINAL_RK4_DERIVATIVES
			// body->m_veloc = internalVeloc[i].m_linear.Scale(invStep);
			// body->m_omega = internalVeloc[i].m_angular.Scale(invStep);
			(simd_type &)body->m_veloc = simd_mul_v((simd_type &)m_internalVeloc[i].m_linear, invStepSimd);
			(simd_type &)body->m_omega = simd_mul_v((simd_type &)m_internalVeloc[i].m_angular, invStepSimd);
#endif

			// dgVector accel = (body->m_veloc - body->m_netForce).Scale (m_invTimeStep);
			// dgVector alpha = (body->m_omega - body->m_netTorque).Scale (m_invTimeStep);
			accel =
				simd_mul_v(simd_sub_v((simd_type &)body->m_veloc, (simd_type &)body->m_netForce), invTimeStepSimd);
			alpha =
				simd_mul_v(simd_sub_v((simd_type &)body->m_omega, (simd_type &)body->m_netTorque), invTimeStepSimd);

			// if ((accel % accel) < maxAccNorm2) {
			//	accel = zero;
			// }
			// body->m_accel = accel;
			// body->m_netForce = accel.Scale (body->m_mass[3]);
			tmp = simd_mul_v(accel, accel);
			tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
			tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
			tmp = simd_cmplt_s(tmp, accelerationTolerance);
			(simd_type &)body->m_accel =
				simd_andnot_v(accel, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 0)));
			(simd_type &)body->m_netForce =
				simd_mul_v((simd_type &)body->m_accel, simd_set1(body->m_mass[3]));

			// if ((alpha % alpha) < maxAccNorm2) {
			//	alpha = zero;
			// }
			// body->m_alpha = alpha;
			tmp = simd_mul_v(alpha, alpha);
			tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
			tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
			tmp = simd_cmplt_s(tmp, accelerationTolerance);
			(simd_type &)body->m_alpha =
				simd_andnot_v(alpha, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 0)));

			// alpha = body->m_matrix.UnrotateVector(alpha);
			alpha =
				simd_mul_v((simd_type &)body->m_matrix[0], (simd_type &)body->m_alpha);
			alpha = simd_add_v(alpha, simd_move_hl_v(alpha, alpha));
			alpha =
				simd_add_s(alpha, simd_permut_v(alpha, alpha, PURMUT_MASK(0, 0, 0, 1)));

			tmp =
				simd_mul_v((simd_type &)body->m_matrix[1], (simd_type &)body->m_alpha);
			tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
			tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
			alpha = simd_pack_lo_v(alpha, tmp);

			tmp =
				simd_mul_v((simd_type &)body->m_matrix[2], (simd_type &)body->m_alpha);
			tmp = simd_add_v(tmp, simd_move_hl_v(tmp, tmp));
			tmp = simd_add_s(tmp, simd_permut_v(tmp, tmp, PURMUT_MASK(0, 0, 0, 1)));
			alpha = simd_permut_v(alpha, tmp, PURMUT_MASK(3, 0, 1, 0));

			// body->m_netTorque = body->m_matrix.RotateVector (alpha.CompProduct(body->m_mass));
			alpha = simd_mul_v(alpha, (simd_type &)body->m_mass);
			(simd_type &)body->m_netTorque =
				simd_mul_add_v(simd_mul_add_v(simd_mul_v((simd_type &)body->m_matrix[0], simd_permut_v(alpha, alpha, PURMUT_MASK(0, 0, 0, 0))),
											  (simd_type &)body->m_matrix[1], simd_permut_v(alpha, alpha, PURMUT_MASK(1, 1, 1, 1))),
							   (simd_type &)body->m_matrix[2], simd_permut_v(alpha, alpha, PURMUT_MASK(2, 2, 2, 2)));
		}
#endif

	} else {
		dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					  dgFloat32(0.0f));
		for (dgInt32 i = 0; i < m_count; i++) {
			dgBody *body;
			body = m_bodyArray[i].m_body;

			body->m_veloc = m_internalVeloc[i].m_linear.Scale(m_invStep);
			body->m_omega = m_internalVeloc[i].m_angular.Scale(m_invStep);

			dgVector accel = (body->m_veloc - body->m_netForce).Scale(m_invTimeStep);
			dgVector alpha = (body->m_omega - body->m_netTorque).Scale(m_invTimeStep);
			if ((accel % accel) < m_maxAccNorm2) {
				accel = zero;
			}

			if ((alpha % alpha) < m_maxAccNorm2) {
				alpha = zero;
			}

			body->m_accel = accel;
			body->m_alpha = alpha;
			body->m_netForce = accel.Scale(body->m_mass[3]);

			alpha = body->m_matrix.UnrotateVector(alpha);
			body->m_netTorque = body->m_matrix.RotateVector(
				alpha.CompProduct(body->m_mass));
		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverInitFeedbackUpdate::ThreadExecute() {
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	for (dgInt32 i = 0; i < m_count; i++) {
		dgInt32 first;
		dgInt32 count;
		dgInt32 index;
		dgFloat32 val;
		//		dgFloat32 maxForce;

		//		maxForce = dgFloat32 (0.0f);
		first = m_constraintArray[i].m_autoPairstart;
		count = m_constraintArray[i].m_autoPaircount;
		for (dgInt32 j = 0; j < count; j++) {
			index = j + first;
			val = m_force[index];
			//			maxForce = GetMax(dgAbsf (val), maxForce);
			_ASSERTE(dgCheckFloat(val));
			m_jointForceFeeback[index][0] = val;
		}
		m_hasJointFeeback |= (m_constraintArray[i].m_joint->m_updaFeedbackCallback ? 1 : 0);
		//		if (m_constraintArray[i].m_joint->m_updaFeedbackCallback) {
		//			m_constraintArray[i].m_joint->m_updaFeedbackCallback (*m_constraintArray[i].m_joint, m_timestep, m_threadIndex);
		//		}

		//		if (m_constraintArray[i].m_joint->GetId() == dgContactConstraintId) {
		//			dgInt32 m0;
		//			dgInt32 m1;
		//			m0 = m_constraintArray[i].m_m0;
		//			m1 = m_constraintArray[i].m_m1;
		//			m_world->dgGetIndirectLock(&m_locks[m0]);
		//				m_world->AddToBreakQueue ((dgContact*)m_constraintArray[i].m_joint, m_constraintArray[i].m_joint->m_body0, maxForce);
		//			m_world->dgReleaseIndirectLock(&m_locks[m0]);
		//			m_world->dgGetIndirectLock(&m_locks[m1]);
		//				m_world->AddToBreakQueue ((dgContact*)m_constraintArray[i].m_joint, m_constraintArray[i].m_joint->m_body1, maxForce);
		//			m_world->dgReleaseIndirectLock(&m_locks[m1]);
		//		}
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgParallelSolverCalculateForces::ThreadExecute() {
	//	dgInt32 threadIndex;
	//	dgUnsigned32 ticks;
	//	ticks = m_world->m_getPerformanceCount();

	//	threadIndex = m_threadIndex;
	if (m_useSimd) {
#ifdef DG_BUILD_SIMD_CODE
		dgFloatSign tmpIndex;
		simd_type signMask;
		simd_type accNorm;

		tmpIndex.m_integer.m_iVal = 0x7fffffff;
		signMask = simd_set1(tmpIndex.m_fVal);

		accNorm = simd_set1(dgFloat32(0.0f));
		for (dgInt32 i = 0; i < m_count; i += m_threads) {
			//		for (dgInt32 currJoint = 0; currJoint < m_count; currJoint += m_threads) {

			dgInt32 m0;
			dgInt32 m1;
			dgInt32 index;
			//			dgInt32 rowIndex;
			dgInt32 currJoint;
			dgInt32 rowsCount;
			simd_type linearM0;
			simd_type angularM0;
			simd_type linearM1;
			simd_type angularM1;

			currJoint = m_threadIndex + i;
			//			rowIndex = m_jointRemapArray[i];
			index = m_constraintArray[currJoint].m_autoPairstart;
			rowsCount = m_constraintArray[currJoint].m_autoPaircount;
			m0 = m_constraintArray[currJoint].m_m0;
			m1 = m_constraintArray[currJoint].m_m1;

			if (m0) {
				m_world->dgGetIndirectLock(&m_locks[m0]);
			}
			if (m1) {
				m_world->dgGetIndirectLock(&m_locks[m1]);
			}

			linearM0 = (simd_type &)m_internalForces[m0].m_linear;
			angularM0 = (simd_type &)m_internalForces[m0].m_angular;
			linearM1 = (simd_type &)m_internalForces[m1].m_linear;
			angularM1 = (simd_type &)m_internalForces[m1].m_angular;

			for (dgInt32 k = 0; k < rowsCount; k++) {
				dgInt32 frictionIndex;
				simd_type a;
				simd_type f;
				simd_type frictionNormal;
				simd_type lowerFrictionForce;
				simd_type upperFrictionForce;

				//				dgVector acc (m_JMinv[index].m_jacobian_IM0.m_linear.CompProduct(linearM0));
				//				acc += m_JMinv[index].m_jacobian_IM0.m_angular.CompProduct (angularM0);
				//				acc += m_JMinv[index].m_jacobian_IM1.m_linear.CompProduct (linearM1);
				//				acc += m_JMinv[index].m_jacobian_IM1.m_angular.CompProduct (angularM1);

				a =
					simd_mul_v((simd_type &)m_JMinv[index].m_jacobian_IM0.m_linear, linearM0);
				a =
					simd_mul_add_v(a, (simd_type &)m_JMinv[index].m_jacobian_IM0.m_angular, angularM0);
				a =
					simd_mul_add_v(a, (simd_type &)m_JMinv[index].m_jacobian_IM1.m_linear, linearM1);
				a =
					simd_mul_add_v(a, (simd_type &)m_JMinv[index].m_jacobian_IM1.m_angular, angularM1);

				// a = coordenateAccel[index] - acc.m_x - acc.m_y - acc.m_z - force[index] * diagDamp[index];
				a = simd_add_v(a, simd_move_hl_v(a, a));
				a = simd_add_s(a, simd_permut_v(a, a, PURMUT_MASK(3, 3, 3, 1)));
				a =
					simd_sub_s(simd_load_s(m_coordenateAccel[index]), simd_mul_add_s(a, simd_load_s(m_force[index]), simd_load_s(m_diagDamp[index])));

				// f = force[index] + invDJMinvJt[index] * a;
				f =
					simd_mul_add_s(simd_load_s(m_force[index]), simd_load_s(m_invDJMinvJt[index]), a);

				frictionIndex = m_normalForceIndex[index];
				_ASSERTE(
					((frictionIndex < 0) && (m_force[frictionIndex] == dgFloat32(1.0f))) || ((frictionIndex >= 0) && (m_force[frictionIndex] >= dgFloat32(0.0f))));

				// frictionNormal = force[frictionIndex];
				// lowerFrictionForce = frictionNormal * lowerFrictionCoef[index];
				// upperFrictionForce = frictionNormal * upperFrictionCoef[index];
				frictionNormal = simd_load_s(m_force[frictionIndex]);
				lowerFrictionForce =
					simd_mul_s(frictionNormal, simd_load_s(m_lowerFrictionCoef[index]));
				upperFrictionForce =
					simd_mul_s(frictionNormal, simd_load_s(m_upperFrictionCoef[index]));

				// if (f > upperFrictionForce) {
				//	a = dgFloat32 (0.0f);
				//	f = upperFrictionForce;
				// } else if (f < lowerFrictionForce) {
				//	a = dgFloat32 (0.0f);
				//	f = lowerFrictionForce;
				// }
				// frictionNormal = simd_or_v (simd_cmplt_s (f, lowerFrictionForce), simd_cmpgt_s (f, upperFrictionForce));
				f = simd_min_s(simd_max_s(f, lowerFrictionForce), upperFrictionForce);
				a =
					simd_andnot_v(a, simd_or_v(simd_cmplt_s(f, lowerFrictionForce), simd_cmpgt_s(f, upperFrictionForce)));
				accNorm = simd_max_s(accNorm, simd_and_v(a, signMask));

				// prevValue = f - force[index]);
				a = simd_sub_s(f, simd_load_s(m_force[index]));
				a = simd_permut_v(a, a, PURMUT_MASK(0, 0, 0, 0));

				// force[index] = f;
				simd_store_s(f, &m_force[index]);

				linearM0 =
					simd_mul_add_v(linearM0, (simd_type &)m_Jt[index].m_jacobian_IM0.m_linear, a);
				angularM0 =
					simd_mul_add_v(angularM0, (simd_type &)m_Jt[index].m_jacobian_IM0.m_angular, a);
				linearM1 =
					simd_mul_add_v(linearM1, (simd_type &)m_Jt[index].m_jacobian_IM1.m_linear, a);
				angularM1 =
					simd_mul_add_v(angularM1, (simd_type &)m_Jt[index].m_jacobian_IM1.m_angular, a);
				index++;
			}

			(simd_type &)m_internalForces[m0].m_linear = linearM0;
			(simd_type &)m_internalForces[m0].m_angular = angularM0;
			(simd_type &)m_internalForces[m1].m_linear = linearM1;
			(simd_type &)m_internalForces[m1].m_angular = angularM1;

			if (m1) {
				m_world->dgReleaseIndirectLock(&m_locks[m1]);
			}
			if (m0) {
				m_world->dgReleaseIndirectLock(&m_locks[m0]);
			}
		}
		simd_store_s(accNorm, &m_accNorm);
#endif

	} else {
		dgFloat32 accNorm;
		accNorm = dgFloat32(0.0f);
		for (dgInt32 i = 0; i < m_count; i += m_threads) {
			dgInt32 m0;
			dgInt32 m1;
			dgInt32 index;
			dgInt32 currJoint;
			dgInt32 rowsCount;

			currJoint = m_threadIndex + i;
			index = m_constraintArray[currJoint].m_autoPairstart;
			rowsCount = m_constraintArray[currJoint].m_autoPaircount;
			m0 = m_constraintArray[currJoint].m_m0;
			m1 = m_constraintArray[currJoint].m_m1;

			if (m0) {
				m_world->dgGetIndirectLock(&m_locks[m0]);
			}
			if (m1) {
				m_world->dgGetIndirectLock(&m_locks[m1]);
			}

			dgVector linearM0(m_internalForces[m0].m_linear);
			dgVector angularM0(m_internalForces[m0].m_angular);
			dgVector linearM1(m_internalForces[m1].m_linear);
			dgVector angularM1(m_internalForces[m1].m_angular);
			for (dgInt32 k = 0; k < rowsCount; k++) {
				dgInt32 frictionIndex;
				dgFloat32 a;
				dgFloat32 f;
				dgFloat32 prevValue;
				dgFloat32 frictionNormal;
				dgFloat32 lowerFrictionForce;
				dgFloat32 upperFrictionForce;

				dgVector acc(
					m_JMinv[index].m_jacobian_IM0.m_linear.CompProduct(linearM0));
				acc += m_JMinv[index].m_jacobian_IM0.m_angular.CompProduct(angularM0);
				acc += m_JMinv[index].m_jacobian_IM1.m_linear.CompProduct(linearM1);
				acc += m_JMinv[index].m_jacobian_IM1.m_angular.CompProduct(angularM1);

				a = m_coordenateAccel[index] - acc.m_x - acc.m_y - acc.m_z - m_force[index] * m_diagDamp[index];
				f = m_force[index] + m_invDJMinvJt[index] * a;

				frictionIndex = m_normalForceIndex[index];
				_ASSERTE(
					((frictionIndex < 0) && (m_force[frictionIndex] == dgFloat32(1.0f))) || ((frictionIndex >= 0) && (m_force[frictionIndex] >= dgFloat32(0.0f))));

				frictionNormal = m_force[frictionIndex];
				lowerFrictionForce = frictionNormal * m_lowerFrictionCoef[index];
				upperFrictionForce = frictionNormal * m_upperFrictionCoef[index];

				if (f > upperFrictionForce) {
					a = dgFloat32(0.0f);
					f = upperFrictionForce;
				} else if (f < lowerFrictionForce) {
					a = dgFloat32(0.0f);
					f = lowerFrictionForce;
				}

				accNorm = GetMax(accNorm, dgAbsf(a));
				prevValue = f - m_force[index];
				m_force[index] = f;

				linearM0 += m_Jt[index].m_jacobian_IM0.m_linear.Scale(prevValue);
				angularM0 += m_Jt[index].m_jacobian_IM0.m_angular.Scale(prevValue);
				linearM1 += m_Jt[index].m_jacobian_IM1.m_linear.Scale(prevValue);
				angularM1 += m_Jt[index].m_jacobian_IM1.m_angular.Scale(prevValue);
				index++;
			}
			m_internalForces[m0].m_linear = linearM0;
			m_internalForces[m0].m_angular = angularM0;
			m_internalForces[m1].m_linear = linearM1;
			m_internalForces[m1].m_angular = angularM1;

			if (m1) {
				m_world->dgReleaseIndirectLock(&m_locks[m1]);
			}
			if (m0) {
				m_world->dgReleaseIndirectLock(&m_locks[m0]);
			}
		}
		m_accNorm = accNorm;
	}

	//	m_world->m_perfomanceCounters[m_threadIndex][m_dynamicsSolveSpanningTreeTicks] += (m_world->m_getPerformanceCount() - ticks);
}

void dgJacobianMemory::CalculateForcesGameModeParallel(dgInt32 iterations,
													   dgFloat32 maxAccNorm, dgInt32 archModel) const {
	dgFloat32 *const force = m_force;
	const dgJacobianPair *const Jt = m_Jt;
	const dgJacobianPair *const JMinv = m_JMinv;
	const dgFloat32 *const diagDamp = m_diagDamp;
	const dgFloat32 *const invDJMinvJt = m_invDJMinvJt;
	const dgBodyInfo *bodyArray = m_bodyArray;
	dgFloat32 *const penetration = m_penetration;
	const dgFloat32 *const externAccel = m_deltaAccel;
	const dgFloat32 *const restitution = m_restitution;
	dgFloat32 *const coordenateAccel = m_coordenateAccel;
	dgJacobian *const internalVeloc = m_internalVeloc;
	dgJacobian *const internalForces = m_internalForces;
	;
	dgFloat32 **const jointForceFeeback = m_jointFeebackForce;
	const dgInt32 *const accelIsMortor = m_accelIsMotor;
	const dgInt32 *const normalForceIndex = m_normalForceIndex;
	const dgJointInfo *const constraintArray = m_constraintArray;
	const dgFloat32 *const penetrationStiffness = m_penetrationStiffness;
	const dgFloat32 *const lowerFrictionCoef = m_lowerBoundFrictionCoefficent;
	const dgFloat32 *const upperFrictionCoef = m_upperBoundFrictionCoefficent;
	dgVector zero(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
				  dgFloat32(0.0f));

	dgFloat32 invStep = (dgFloat32(1.0f) / dgFloat32(LINEAR_SOLVER_SUB_STEPS));
	dgFloat32 timeStep = m_timeStep * invStep;
	dgFloat32 invTimeStep = m_invTimeStep * dgFloat32(LINEAR_SOLVER_SUB_STEPS);

	dgInt32 threads = dgInt32(m_world->m_numberOfTheads);

	dgInt32 chunkJointSizes[DG_MAXIMUN_THREADS];
	dgInt32 chunkBodiesSizes[DG_MAXIMUN_THREADS];

	dgInt32 acc = 0;
	m_world->m_threadsManager.CalculateChunkSizes(m_bodyCount - 1,
												  chunkBodiesSizes);
	dgParallelSolverClear *const clearAccumulators =
		m_world->m_dynamicSolver.m_clearAccumulators;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		clearAccumulators[threadIndex].m_useSimd = archModel;
		clearAccumulators[threadIndex].m_world = m_world;
		clearAccumulators[threadIndex].m_threadIndex = threadIndex;
		clearAccumulators[threadIndex].m_count = chunkBodiesSizes[threadIndex];

		clearAccumulators[threadIndex].m_bodyArray = &bodyArray[acc + 1];
		clearAccumulators[threadIndex].m_internalVeloc = &internalVeloc[acc + 1];
		clearAccumulators[threadIndex].m_internalForces = &internalForces[acc + 1];
		m_world->m_threadsManager.SubmitJob(&clearAccumulators[threadIndex]);
		acc += chunkBodiesSizes[threadIndex];
	}
	m_world->m_threadsManager.SynchronizationBarrier();

	internalVeloc[0].m_linear = zero;
	internalVeloc[0].m_angular = zero;
	internalForces[0].m_linear = zero;
	internalForces[0].m_angular = zero;

	acc = 0;
	m_world->m_threadsManager.CalculateChunkSizes(m_jointCount, chunkJointSizes);
	dgParallelSolverInitInternalForces *const parallelInitIntenalForces =
		m_world->m_dynamicSolver.m_parallelInitIntenalForces;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		parallelInitIntenalForces[threadIndex].m_useSimd = archModel;
		parallelInitIntenalForces[threadIndex].m_world = m_world;
		parallelInitIntenalForces[threadIndex].m_locks = m_treadLocks;
		parallelInitIntenalForces[threadIndex].m_threadIndex = threadIndex;
		parallelInitIntenalForces[threadIndex].m_Jt = Jt;
		parallelInitIntenalForces[threadIndex].m_force = force;
		parallelInitIntenalForces[threadIndex].m_internalForces = internalForces;
		parallelInitIntenalForces[threadIndex].m_count =
			chunkJointSizes[threadIndex];
		parallelInitIntenalForces[threadIndex].m_constraintArray =
			&constraintArray[acc];
		m_world->m_threadsManager.SubmitJob(
			&parallelInitIntenalForces[threadIndex]);
		acc += chunkJointSizes[threadIndex];
	}
	m_world->m_threadsManager.SynchronizationBarrier();

	acc = 0;
	dgParallelSolverJointAcceleration *const parallelSolverJointAcceleration =
		m_world->m_dynamicSolver.m_parallelSolverJointAcceleration;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		parallelSolverJointAcceleration[threadIndex].m_useSimd = archModel;
		parallelSolverJointAcceleration[threadIndex].m_world = m_world;
		parallelSolverJointAcceleration[threadIndex].m_threadIndex = threadIndex;
		parallelSolverJointAcceleration[threadIndex].m_Jt = Jt;
		parallelSolverJointAcceleration[threadIndex].m_count =
			chunkJointSizes[threadIndex];
		parallelSolverJointAcceleration[threadIndex].m_constraintArray =
			constraintArray;
		parallelSolverJointAcceleration[threadIndex].m_jointStart = acc;
		//		parallelSolverJointAcceleration[threadIndex].m_jointRemapArray = &jointRemapArray[acc];
		parallelSolverJointAcceleration[threadIndex].m_timeStep = timeStep;
		parallelSolverJointAcceleration[threadIndex].m_invTimeStep = invTimeStep;
		parallelSolverJointAcceleration[threadIndex].m_firstPassCoef = dgFloat32(
			1.0f);
		parallelSolverJointAcceleration[threadIndex].m_penetration = penetration;
		parallelSolverJointAcceleration[threadIndex].m_restitution = restitution;
		parallelSolverJointAcceleration[threadIndex].m_externAccel = externAccel;
		parallelSolverJointAcceleration[threadIndex].m_coordenateAccel =
			coordenateAccel;
		parallelSolverJointAcceleration[threadIndex].m_accelIsMortor =
			accelIsMortor;
		parallelSolverJointAcceleration[threadIndex].m_normalForceIndex =
			normalForceIndex;
		parallelSolverJointAcceleration[threadIndex].m_penetrationStiffness =
			penetrationStiffness;
		acc += chunkJointSizes[threadIndex];
	}

	acc = 0;
	dgParallelSolverUpdateVeloc *const parallelSolverUpdateVeloc =
		m_world->m_dynamicSolver.m_parallelSolverUpdateVeloc;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		parallelSolverUpdateVeloc[threadIndex].m_useSimd = archModel;
		parallelSolverUpdateVeloc[threadIndex].m_world = m_world;
		parallelSolverUpdateVeloc[threadIndex].m_threadIndex = threadIndex;
		parallelSolverUpdateVeloc[threadIndex].m_timeStep = timeStep;
		parallelSolverUpdateVeloc[threadIndex].m_count =
			chunkBodiesSizes[threadIndex];
		parallelSolverUpdateVeloc[threadIndex].m_bodyArray = &bodyArray[acc + 1];
		parallelSolverUpdateVeloc[threadIndex].m_internalVeloc = &internalVeloc[acc + 1];
		parallelSolverUpdateVeloc[threadIndex].m_internalForces =
			&internalForces[acc + 1];
		acc += chunkBodiesSizes[threadIndex];
	}

	dgParallelSolverCalculateForces *const parallelSolverCalculateForces =
		m_world->m_dynamicSolver.m_parallelSolverCalculateForces;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		parallelSolverCalculateForces[threadIndex].m_useSimd = archModel;
		parallelSolverCalculateForces[threadIndex].m_world = m_world;
		parallelSolverCalculateForces[threadIndex].m_threads = threads;
		parallelSolverCalculateForces[threadIndex].m_threadIndex = threadIndex;

		parallelSolverCalculateForces[threadIndex].m_Jt = Jt;
		parallelSolverCalculateForces[threadIndex].m_JMinv = JMinv;
		parallelSolverCalculateForces[threadIndex].m_count =
			chunkJointSizes[threadIndex] * threads;
		//		parallelSolverCalculateForces[threadIndex].m_jointRemapArray = &jointRemapArray[threadIndex];
		parallelSolverCalculateForces[threadIndex].m_constraintArray =
			constraintArray;

		parallelSolverCalculateForces[threadIndex].m_locks = m_treadLocks;
		parallelSolverCalculateForces[threadIndex].m_force = force;
		parallelSolverCalculateForces[threadIndex].m_diagDamp = diagDamp;
		parallelSolverCalculateForces[threadIndex].m_invDJMinvJt = invDJMinvJt;
		parallelSolverCalculateForces[threadIndex].m_normalForceIndex =
			normalForceIndex;
		parallelSolverCalculateForces[threadIndex].m_internalForces =
			internalForces;
		parallelSolverCalculateForces[threadIndex].m_coordenateAccel =
			coordenateAccel;
		parallelSolverCalculateForces[threadIndex].m_lowerFrictionCoef =
			lowerFrictionCoef;
		parallelSolverCalculateForces[threadIndex].m_upperFrictionCoef =
			upperFrictionCoef;
	}

	dgInt32 maxPasses = iterations + DG_BASE_ITERATION_COUNT;
	for (dgInt32 step = 0; step < LINEAR_SOLVER_SUB_STEPS; step++) {
		for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
			m_world->m_threadsManager.SubmitJob(
				&parallelSolverJointAcceleration[threadIndex]);
		}
		m_world->m_threadsManager.SynchronizationBarrier();

		dgFloat32 accNorm = maxAccNorm * dgFloat32(2.0f);
		for (dgInt32 passes = 0; (passes < maxPasses) && (accNorm > maxAccNorm);
			 passes++) {

			for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
				m_world->m_threadsManager.SubmitJob(
					&parallelSolverCalculateForces[threadIndex]);
			}
			m_world->m_threadsManager.SynchronizationBarrier();

			accNorm = dgFloat32(0.0f);
			for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
				if (parallelSolverCalculateForces[threadIndex].m_accNorm > accNorm) {
					accNorm = parallelSolverCalculateForces[threadIndex].m_accNorm;
				}
			}
			// accNorm = 1.0f;
		}

		for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
			m_world->m_threadsManager.SubmitJob(
				&parallelSolverUpdateVeloc[threadIndex]);
		}
		m_world->m_threadsManager.SynchronizationBarrier();
	}

	acc = 0;
	m_world->m_threadsManager.CalculateChunkSizes(m_jointCount, chunkJointSizes);
	dgParallelSolverInitFeedbackUpdate *const parallelSolverInitFeedbackUpdate =
		m_world->m_dynamicSolver.m_parallelSolverInitFeedbackUpdate;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		parallelSolverInitFeedbackUpdate[threadIndex].m_locks = m_treadLocks;
		parallelSolverInitFeedbackUpdate[threadIndex].m_world = m_world;
		parallelSolverInitFeedbackUpdate[threadIndex].m_hasJointFeeback = 0;
		parallelSolverInitFeedbackUpdate[threadIndex].m_threadIndex = threadIndex;
		parallelSolverInitFeedbackUpdate[threadIndex].m_force = force;
		parallelSolverInitFeedbackUpdate[threadIndex].m_timestep = timeStep;
		parallelSolverInitFeedbackUpdate[threadIndex].m_jointForceFeeback =
			jointForceFeeback;
		parallelSolverInitFeedbackUpdate[threadIndex].m_count =
			chunkJointSizes[threadIndex];
		parallelSolverInitFeedbackUpdate[threadIndex].m_constraintArray =
			&constraintArray[acc];
		m_world->m_threadsManager.SubmitJob(
			&parallelSolverInitFeedbackUpdate[threadIndex]);
		acc += chunkJointSizes[threadIndex];
	}
	m_world->m_threadsManager.SynchronizationBarrier();

	acc = 0;
	dgFloat32 maxAccNorm2 = maxAccNorm * maxAccNorm;
	dgParallelSolverUpdateForce *const parallelSolverUpdateForce =
		m_world->m_dynamicSolver.m_parallelSolverUpdateForce;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		parallelSolverUpdateForce[threadIndex].m_useSimd = archModel;
		parallelSolverUpdateForce[threadIndex].m_world = m_world;
		parallelSolverUpdateForce[threadIndex].m_threadIndex = threadIndex;
		parallelSolverUpdateForce[threadIndex].m_invStep = invStep;
		parallelSolverUpdateForce[threadIndex].m_invTimeStep = m_invTimeStep;
		parallelSolverUpdateForce[threadIndex].m_maxAccNorm2 = maxAccNorm2;
		parallelSolverUpdateForce[threadIndex].m_count =
			chunkBodiesSizes[threadIndex];
		parallelSolverUpdateForce[threadIndex].m_bodyArray = &bodyArray[acc + 1];
		parallelSolverUpdateForce[threadIndex].m_internalVeloc = &internalVeloc[acc + 1];
		acc += chunkBodiesSizes[threadIndex];
		m_world->m_threadsManager.SubmitJob(
			&parallelSolverUpdateForce[threadIndex]);
	}
	m_world->m_threadsManager.SynchronizationBarrier();

	dgInt32 hasJointFeeback = 0;
	for (dgInt32 threadIndex = 0; threadIndex < threads; threadIndex++) {
		hasJointFeeback |=
			parallelSolverInitFeedbackUpdate[threadIndex].m_hasJointFeeback;
	}

	if (hasJointFeeback) {
		for (dgInt32 i = 0; i < m_jointCount; i++) {
			if (constraintArray[i].m_joint->m_updaFeedbackCallback) {
				constraintArray[i].m_joint->m_updaFeedbackCallback(
					*constraintArray[i].m_joint, m_timeStep, m_threadIndex);
			}
		}
	}
}

void dgJacobianMemory::CalculateReactionsForcesParallel(dgInt32 solverMode,
														dgFloat32 tolerance, dgInt32 archModel) const {
	//	_ASSERTE (m_jointCount >= DG_PARALLEL_JOINT_COUNT);
	if (solverMode) {
		CalculateForcesGameModeParallel(solverMode, tolerance, archModel);
	} else if (archModel) {
		CalculateForcesSimulationModeSimd(tolerance);
	} else {
		CalculateForcesSimulationMode(tolerance);
	}
}
