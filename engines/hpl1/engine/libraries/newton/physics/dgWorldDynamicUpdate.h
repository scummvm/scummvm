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

#if !defined(AFX_DYNAMIC_UPDATE_H__EC18C699_D48D_448F_A510_A865B2CC0789__INCLUDED_)
#define AFX_DYNAMIC_UPDATE_H__EC18C699_D48D_448F_A510_A865B2CC0789__INCLUDED_


#include "dgPhysicsStdafx.h"


//#define DG_PSD_DAMP_TOL           dgFloat32 (1.0e-2f)
#define DG_PSD_DAMP_TOL             dgFloat32 (1.0e-3f)
//#define DG_PSD_DAMP_TOL           dgFloat32 (1.0e-4f)


#define LINEAR_SOLVER_SUB_STEPS         4


class dgIsland;
class dgJointInfo;
class dgBodyInfo;
class dgJacobianMemory;
class dgWorldDynamicUpdate;


class dgIslandCallbackStruct {
public:
	dgWorld *m_world;
	dgInt32 m_count;
	dgInt32 m_strideInByte;
	void *m_bodyArray;
};



template<class T>
class dgQueue {
public:
	dgQueue(T *pool, dgInt32 size)
		: m_pool(pool) {

		m_mod = size;
		m_lastIndex = 0;
		m_firstIndex = 0;
	}

	void Insert(T info) {
		m_pool[m_firstIndex] = info;
		m_firstIndex ++;
		if (m_firstIndex >= m_mod) {
			m_firstIndex = 0;
		}
		NEWTON_ASSERT(m_firstIndex != m_lastIndex);
	}

	T Remove() {
		NEWTON_ASSERT(m_firstIndex != m_lastIndex);

		T element = m_pool[m_lastIndex];
		m_lastIndex ++;
		if (m_lastIndex >= m_mod) {
			m_lastIndex = 0;
		}

		return element;
	}

	void Reset() {
		m_lastIndex = m_firstIndex;
	}

	bool IsEmpty() const {
		return (m_firstIndex == m_lastIndex);
	}

	dgInt32 m_mod;
	dgInt32 m_firstIndex;
	dgInt32 m_lastIndex;
	T *m_pool;
};


class dgSolverWorlkerThreads: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_threads;
	dgInt32 m_useSimd;
	dgInt32 m_solverMode;
	dgFloat32 m_timestep;
	dgWorld *m_world;
	dgJacobianMemory *m_system;
	dgWorldDynamicUpdate *m_dynamics;
};



class dgParallelSolverClear: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgWorld *m_world;
	const dgBodyInfo *m_bodyArray;
	dgJacobian *m_internalVeloc;
	dgJacobian *m_internalForces;
};

class dgParallelSolverInitInternalForces: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgInt32 *m_locks;
	dgWorld *m_world;
	dgFloat32 *m_force;
	const dgJacobianPair *m_Jt;
	dgJacobian *m_internalForces;
	const dgJointInfo *m_constraintArray;
};

class dgParallelSolverInitFeedbackUpdate: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_hasJointFeeback;
	dgInt32 *m_locks;
	dgWorld *m_world;
	dgFloat32 m_timestep;
	dgFloat32 *m_force;
	dgFloat32 **m_jointForceFeeback;
	const dgJointInfo *m_constraintArray;
};


class dgParallelSolverBodyInertia: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgWorld *m_world;
	dgBodyInfo *m_bodyArray;
};

class dgParallelSolverBuildJacobianRows: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgInt32 m_bodyCount;
	dgWorld *m_world;
	dgFloat32 *m_force;
	dgFloat32 *m_diagDamp;
	dgFloat32 *m_deltaAccel;
	dgFloat32 *m_invDJMinvJt;
	dgFloat32 *m_coordenateAccel;
	dgFloat32 **m_jointFeebackForce;
	dgJacobianPair *m_Jt;
	dgJacobianPair *m_JMinv;
	dgBodyInfo *m_bodyArray;
	dgJointInfo *m_constraintArray;
};

class dgParallelSolverBuildJacobianMatrix: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	bool m_bitMode;
	dgInt32 m_count;
	dgInt32 m_jointSolved;
	dgInt32 *m_rowsCount;
	dgFloat32 m_timestep;
	dgFloat32 m_invTimestep;
	dgWorld *m_world;
	dgJacobianMemory *m_solverMemory;
	dgJointInfo *m_constraintArray;
};

class dgParallelSolverJointAcceleration: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgInt32 m_jointStart;
	dgWorld *m_world;
	dgFloat32 m_timeStep;
	dgFloat32 m_invTimeStep;
	dgFloat32 m_firstPassCoef;
	const dgJacobianPair *m_Jt;
	dgFloat32 *m_penetration;
	const dgFloat32 *m_restitution;
	const dgFloat32 *m_externAccel;
	dgFloat32 *m_coordenateAccel;
	const dgInt32 *m_accelIsMortor;
	const dgInt32 *m_normalForceIndex;
	const dgFloat32 *m_penetrationStiffness;
	const dgJointInfo *m_constraintArray;
};


class dgParallelSolverUpdateVeloc: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgWorld *m_world;
	dgFloat32 m_timeStep;
	dgJacobian *m_internalVeloc;
	const dgJacobian *m_internalForces;
	const dgBodyInfo *m_bodyArray;
};

class dgParallelSolverUpdateForce: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgWorld *m_world;

	dgFloat32 m_invStep;
	dgFloat32 m_invTimeStep;
	dgFloat32 m_maxAccNorm2;
	const dgJacobian *m_internalVeloc;
	const dgBodyInfo *m_bodyArray;
};




class dgParallelSolverCalculateForces: public dgWorkerThread {
public:
	virtual void ThreadExecute();

	dgInt32 m_count;
	dgInt32 m_useSimd;
	dgInt32 m_threads;
	dgWorld *m_world;
	dgInt32 *m_locks;
	dgJacobian *m_internalForces;
	dgFloat32 m_accNorm;
	dgFloat32 *m_force;
	const dgFloat32 *m_diagDamp;
	const dgInt32 *m_normalForceIndex;
	const dgFloat32 *m_invDJMinvJt;
	const dgFloat32 *m_coordenateAccel;
	const dgFloat32 *m_lowerFrictionCoef;
	const dgFloat32 *m_upperFrictionCoef;
	const dgJacobianPair *m_Jt;
	const dgJacobianPair *m_JMinv;
	const dgJointInfo *m_constraintArray;
};



class dgJacobianMemory {
public:
	dgInt32 m_rowCount;
	dgInt32 m_bodyCount;
	dgInt32 m_jointCount;
	dgInt32 m_maxBodiesCount;
	dgInt32 m_maxJacobiansCount;
	dgInt32 m_threadIndex;
	dgFloat32 m_timeStep;
	dgFloat32 m_invTimeStep;
	dgWorld *m_world;
	dgInt32 *m_treadLocks;
	dgJacobian *m_internalVeloc;
	dgJacobian *m_internalForces;
	dgJointInfo *m_constraintArray;
	dgJacobianPair *m_Jt;
	dgJacobianPair *m_JMinv;
	dgFloat32 *m_force;
	dgFloat32 *m_accel;
	dgFloat32 *m_deltaAccel;
	dgFloat32 *m_deltaForce;
	dgFloat32 *m_diagDamp;
	dgFloat32 *m_invDJMinvJt;
	dgFloat32 *m_restitution;
	dgFloat32 *m_penetration;
	dgFloat32 *m_coordenateAccel;
	dgFloat32 *m_penetrationStiffness;
	dgFloat32 *m_lowerBoundFrictionCoefficent;
	dgFloat32 *m_upperBoundFrictionCoefficent;
	dgFloat32 **m_jointFeebackForce;
	dgBodyInfo *m_bodyArray;
	dgInt32 *m_normalForceIndex;
	dgInt32 *m_accelIsMotor;

	void SwapRows(dgInt32 i, dgInt32 j) const;
	void SwapRowsSimd(dgInt32 i, dgInt32 j) const;

	void CalculateReactionsForces(dgInt32 solverMode, dgFloat32 maxAccNorm) const;
	void CalculateReactionsForcesSimd(dgInt32 solverMode, dgFloat32 maxAccNorm) const;

	void ApplyExternalForcesAndAcceleration(dgFloat32 maxAccNorm) const;
	void ApplyExternalForcesAndAccelerationSimd(dgFloat32 maxAccNorm) const;

	void CalculateSimpleBodyReactionsForces(dgFloat32 maxAccNorm) const;
	void CalculateSimpleBodyReactionsForcesSimd(dgFloat32 maxAccNorm) const;

	dgFloat32 CalculateJointForces(dgInt32 joint, dgFloat32 *forceStep, dgFloat32 maxAccNorm) const;
	dgFloat32 CalculateJointForcesSimd(dgInt32 joint, dgFloat32 *forceStep, dgFloat32 maxAccNorm) const;

	void CalculateForcesSimulationMode(dgFloat32 maxAccNorm) const;
	void CalculateForcesSimulationModeSimd(dgFloat32 maxAccNorm) const;

	void CalculateForcesGameMode(dgInt32 itertions, dgFloat32 maxAccNorm) const;
	void CalculateForcesGameModeSimd(dgInt32 itertions, dgFloat32 maxAccNorm) const;

	void CalculateForcesGameModeParallel(dgInt32 itertions, dgFloat32 maxAccNorm, dgInt32 archModel) const;
	void CalculateReactionsForcesParallel(dgInt32 solverMode, dgFloat32 maxAccNorm, dgInt32 archModel) const;
};


class dgWorldDynamicUpdate {

	dgWorldDynamicUpdate();
	void UpdateDynamics(dgWorld *const world, dgInt32 archMode, dgFloat32 timestep);

private:
	// single core functions
	void BuildIsland(dgQueue<dgBody *> &queue, dgInt32 jountCount, dgInt32 hasUnilateralJoints, dgInt32 isContinueCollisionIsland);
//	dgBody* SpanningTree (dgBody* body, dgBody** const queuePool, dgInt32 queueSize, dgInt32 solveMode);
	void SpanningTree(dgBody *const body);
	void ReallocBodyMemory(dgInt32 count);
	void ReallocJointsMemory(dgInt32 count);
	void ReallocIslandMemory(dgInt32 count);


	// multi-cores functions
	void BuildJacobianMatrix(const dgIsland &island, dgInt32 threadIndex, dgFloat32 timestep);
	void BuildJacobianMatrixSimd(const dgIsland &island, dgInt32 threadIndex, dgFloat32 timestep);
	dgInt32 GetJacobialDerivatives(const dgIsland &island, dgInt32 threadIndex, bool bitMode, dgInt32 rowCount, dgFloat32 timestep);


	void BuildJacobianMatrixParallel(const dgIsland &island, dgFloat32 timestep, dgInt32 archModel);
	dgInt32 GetJacobialDerivativesParallel(const dgIsland &island, bool bitMode, dgInt32 rowCount, dgFloat32 timestep);

	void ReallocIntenalForcesMemory(dgInt32 count, dgInt32 threadIndex);
	void ReallocJacobiansMemory(dgInt32 count, dgInt32 threadIndex);

//	void SortIslands ();
//	dgInt32 CompareIslands (const dgIsland& A, const dgIsland& B) const;
	void IntegrateArray(const dgBodyInfo *body, dgInt32 count, dgFloat32 accelTolerance, dgFloat32 timestep, dgInt32 threadIndex, bool update) const;

	dgInt32 m_bodies;
	dgInt32 m_joints;
	dgInt32 m_islands;
	dgInt32 m_markLru;
	dgInt32 m_maxJointCount;
	dgInt32 m_maxBodiesCount;
	dgInt32 m_maxIslandCount;

	dgIsland *m_islandArray;
	dgBodyInfo *m_bodyArray;
	dgJointInfo *m_constraintArray;


	dgJacobianMemory m_solverMemory [DG_MAXIMUN_THREADS];
	dgSolverWorlkerThreads m_workerThreads[DG_MAXIMUN_THREADS];
	dgParallelSolverClear m_clearAccumulators[DG_MAXIMUN_THREADS];
	dgParallelSolverBodyInertia m_parallelBodyInertiaMatrix[DG_MAXIMUN_THREADS];
	dgParallelSolverUpdateVeloc m_parallelSolverUpdateVeloc[DG_MAXIMUN_THREADS];
	dgParallelSolverUpdateForce m_parallelSolverUpdateForce[DG_MAXIMUN_THREADS];
	dgParallelSolverCalculateForces m_parallelSolverCalculateForces[DG_MAXIMUN_THREADS];
	dgParallelSolverInitInternalForces m_parallelInitIntenalForces[DG_MAXIMUN_THREADS];
	dgParallelSolverBuildJacobianRows m_parallelSolverBuildJacobianRows[DG_MAXIMUN_THREADS];
	dgParallelSolverJointAcceleration m_parallelSolverJointAcceleration[DG_MAXIMUN_THREADS];
	dgParallelSolverInitFeedbackUpdate m_parallelSolverInitFeedbackUpdate[DG_MAXIMUN_THREADS];
	dgParallelSolverBuildJacobianMatrix m_parallelSolverBuildJacobianMatrix[DG_MAXIMUN_THREADS];


	dgBody *m_sentinelBody;
	dgWorld *m_world;
	friend class dgWorld;
	friend class dgJacobianMemory;
	friend class dgSolverWorlkerThreads;
};


#define DG_CHECK_ACTIVE(body) NEWTON_ASSERT (((body->m_invMass.m_w == dgFloat32 (0.0f)) && !body->m_active) || (body->m_invMass.m_w > dgFloat32 (0.0f)))
#endif
