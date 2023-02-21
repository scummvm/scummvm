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

#if !defined(AFX_DGBODY_H__C16EDCD6_53C4_4C6F_A70A_591819F7187E__INCLUDED_)
#define AFX_DGBODY_H__C16EDCD6_53C4_4C6F_A70A_591819F7187E__INCLUDED_

#include "dgBodyMasterList.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


class dgLink;
class dgBody;
class dgWorld;
class dgCollision;
class dgBroadPhaseCell;

#define DG_MIN_SPEED_ATT dgFloat32(0.0f)
#define DG_MAX_SPEED_ATT dgFloat32(0.02f)
#define DG_INFINITE_MASS dgFloat32(1.0e15f)
#define DG_FREEZE_MAG dgFloat32(0.1f)
#define DG_FREEZE_MAG2 dgFloat32(DG_FREEZE_MAG *DG_FREEZE_MAG)

#define DG_ErrTolerance (1.0e-2f)
#define DG_ErrTolerance2 (DG_ErrTolerance * DG_ErrTolerance)

DG_MSC_VECTOR_ALIGMENT
struct dgLineBox {
	dgVector m_l0;
	dgVector m_l1;
	dgVector m_boxL0;
	dgVector m_boxL1;
} DG_GCC_VECTOR_ALIGMENT;

class dgConvexCastReturnInfo {
public:
	dgFloat32 m_point[4];            // collision point in global space
	dgFloat32 m_normal[4];           // surface normal at collision point in global space
	dgFloat32 m_normalOnHitPoint[4]; // surface normal at the surface of the hit body,
	// is the same as the normal calculate by a raycast passing by the hit point in the direction of the cast
	dgFloat32 m_penetration;         // contact penetration at collision point
	dgInt32 m_contaID;               // collision ID at contact point
	const dgBody *m_hitBody;         // body hit at contact point
};

typedef void(dgApi *OnBodyDestroy)(dgBody &me);
typedef void(dgApi *OnApplyExtForceAndTorque)(dgBody &me, dgFloat32 timestep, dgInt32 threadIndex);
typedef void(dgApi *OnMatrixUpdateCallback)(const dgBody &me, const dgMatrix &matrix, dgInt32 threadIndex);
typedef dgUnsigned32(dgApi *OnRayPrecastAction)(const dgBody *const body, const dgCollision *const collision, void *const userData);
typedef dgFloat32(dgApi *OnRayCastAction)(const dgBody *const body, const dgVector &normal, dgInt32 collisionID, void *const userData, dgFloat32 intersetParam);
typedef dgUnsigned32(dgApi *GetBuoyancyPlane)(void *collisionID, void *context, const dgMatrix &matrix, dgPlane &plane);

#define OverlapTest(body0, body1) dgOverlapTest((body0)->m_minAABB, (body0)->m_maxAABB, (body1)->m_minAABB, (body1)->m_maxAABB)
//#define OverlapTest_SSE(body0,body1) dgOverlapTest_SSE ((body0)->m_minAABB, (body0)->m_maxAABB, (body1)->m_minAABB, (body1)->m_maxAABB)

class dgBroadPhaseList {
public:
	dgBroadPhaseCell *m_cell;
	void *m_axisArrayNode[3];

	void reset() {
		m_cell = NULL;
		for (uint i = 0; i < ARRAYSIZE(m_axisArrayNode); i++) m_axisArrayNode[i] = NULL;
	}
};

DG_MSC_VECTOR_ALIGMENT
class dgBody {
public:
	DG_CLASS_ALLOCATOR(allocator)

	dgBody();
	~dgBody();

	void AddForce(const dgVector &force);
	void AddTorque(const dgVector &torque);
	void AttachCollision(dgCollision *collision);

	void SetGroupID(dgUnsigned32 id);
	void SetMatrix(const dgMatrix &matrix);
	void SetMatrixIgnoreSleep(const dgMatrix &matrix);
	void SetUserData(void *const userData);
	void SetForce(const dgVector &force);
	void SetTorque(const dgVector &torque);
	void SetOmega(const dgVector &omega);
	void SetVelocity(const dgVector &velocity);
	void SetLinearDamping(dgFloat32 linearDamp);
	void SetAngularDamping(const dgVector &angularDamp);
	void SetCentreOfMass(const dgVector &com);
	void SetAparentMassMatrix(const dgVector &massMatrix);
	void SetMassMatrix(dgFloat32 mass, dgFloat32 Ix, dgFloat32 Iy, dgFloat32 Iz);
	//  void SetGyroscopicTorqueMode (bool mode);
	void SetCollisionWithLinkedBodies(bool state);
	//  void SetFreezeTreshhold (dgFloat32 freezeAccel2, dgFloat32 freezeAlpha2, dgFloat32 freezeSpeed2, dgFloat32 freezeOmega2);

	void SetContinuesCollisionMode(bool mode);
	void SetDestructorCallback(OnBodyDestroy destructor);
	void SetMatrixUpdateCallback(OnMatrixUpdateCallback callback);
	OnMatrixUpdateCallback GetMatrixUpdateCallback() const;
	//  void SetAutoactiveNotify (OnActivation activate);
	void SetExtForceAndTorqueCallback(OnApplyExtForceAndTorque callback);
	OnApplyExtForceAndTorque GetExtForceAndTorqueCallback() const;

	dgConstraint *GetFirstJoint() const;
	dgConstraint *GetNextJoint(const dgConstraint *joint) const;

	dgConstraint *GetFirstContact() const;
	dgConstraint *GetNextContact(const dgConstraint *joint) const;

	void *GetUserData() const;
	dgWorld *GetWorld() const;
	const dgVector &GetMass() const;
	const dgVector &GetInvMass() const;
	const dgVector &GetAparentMass() const;

	const dgVector &GetOmega() const;
	const dgVector &GetVelocity() const;
	const dgVector &GetForce() const;
	const dgVector &GetTorque() const;
	const dgVector &GetNetForce() const;
	const dgVector &GetNetTorque() const;

	dgCollision *GetCollision() const;
	dgUnsigned32 GetGroupID() const;
	const dgMatrix &GetMatrix() const;
	const dgVector &GetPosition() const;
	const dgQuaternion &GetRotation() const;

	dgFloat32 GetLinearDamping() const;
	dgVector GetAngularDamping() const;
	dgVector GetCentreOfMass() const;
	bool IsInEquelibrium() const;

	void GetAABB(dgVector &p0, dgVector &p1) const;

	bool GetSleepState() const;
	bool GetAutoSleep() const;
	void SetAutoSleep(bool state);

	bool GetFreeze() const;
	void SetFreeze(bool state);

	void Freeze();
	void Unfreeze();

	dgInt32 GetUniqueID() const;

	bool GetCollisionWithLinkedBodies() const;
	bool GetContinuesCollisionMode() const;

	void AddBuoyancyForce(dgFloat32 fluidDensity, dgFloat32 fluidLinearViscousity, dgFloat32 fluidAngularViscousity,
	                      const dgVector &gravityVector, GetBuoyancyPlane buoyancyPlane, void *const context);

	dgVector CalculateInverseDynamicForce(const dgVector &desiredVeloc, dgFloat32 timestep) const;

	//  dgFloat32 RayCast (const dgVector& globalP0, const dgVector& globalP1,
	dgFloat32 RayCast(const dgLineBox &line,
	                  OnRayCastAction filter, OnRayPrecastAction preFilter, void *const userData, dgFloat32 minT) const;
	//  dgFloat32 RayCastSimd (const dgVector& globalP0, const dgVector& globalP1,
	//                     OnRayCastAction filter, OnRayPrecastAction preFilter, void* userData, dgFloat32 minT) const;

	void CalcInvInertiaMatrix();
	void CalcInvInertiaMatrixSimd();
	const dgMatrix &GetCollisionMatrix() const;

	dgBodyMasterList::dgListNode *GetMasterList() const;

	void InvalidateCache();

private:
	void SetMatrixOriginAndRotation(const dgMatrix &matrix);

	void CalculateContinueVelocity(dgFloat32 timestep, dgVector &veloc, dgVector &omega) const;
	void CalculateContinueVelocitySimd(dgFloat32 timestep, dgVector &veloc, dgVector &omega) const;

	dgVector GetTrajectory(const dgVector &veloc, const dgVector &omega) const;
	void IntegrateVelocity(dgFloat32 timestep);
	void UpdateMatrix(dgFloat32 timestep, dgInt32 threadIndex);
	void UpdateCollisionMatrix(dgFloat32 timestep, dgInt32 threadIndex);
	void UpdateCollisionMatrixSimd(dgFloat32 timestep, dgInt32 threadIndex);

	void ApplyExtenalForces(dgFloat32 timestep, dgInt32 threadIndex);
	void AddImpulse(const dgVector &pointVeloc, const dgVector &pointPosit);

	void ApplyImpulseArray(dgInt32 count, dgInt32 strideInBytes, const dgFloat32 *const impulseArray, const dgFloat32 *const pointArray);

	//  void AddGyroscopicTorque();
	void AddDamingAcceleration();

	dgMatrix CalculateInertiaMatrix() const;
	dgMatrix CalculateInvInertiaMatrix() const;

	dgMatrix m_matrix;
	dgMatrix m_collisionWorldMatrix;
	dgMatrix m_invWorldInertiaMatrix;
	dgQuaternion m_rotation;

	dgVector m_veloc;
	dgVector m_omega;
	dgVector m_accel;
	dgVector m_alpha;
	dgVector m_netForce;
	dgVector m_netTorque;
	dgVector m_prevExternalForce;
	dgVector m_prevExternalTorque;

	dgVector m_mass;
	dgVector m_invMass;
	dgVector m_aparentMass;
	dgVector m_localCentreOfMass;
	dgVector m_globalCentreOfMass;
	dgVector m_minAABB;
	dgVector m_maxAABB;
	dgVector m_dampCoef;

	dgInt32 m_index;
	dgInt32 m_uniqueID;
	dgInt32 m_bodyGroupId;
	dgInt32 m_genericLRUMark;
	dgInt32 m_sleepingCounter;
	dgUnsigned32 m_dynamicsLru;
	dgUnsigned32 m_isInDerstruionArrayLRU;

	dgUnsigned32 m_freeze : 1;
	dgUnsigned32 m_sleeping : 1;
	dgUnsigned32 m_autoSleep : 1;
	dgUnsigned32 m_isInWorld : 1;
	dgUnsigned32 m_equilibrium : 1;
	dgUnsigned32 m_continueCollisionMode : 1;
	dgUnsigned32 m_spawnnedFromCallback : 1;
	dgUnsigned32 m_collideWithLinkedBodies : 1;
	dgUnsigned32 m_solverInContinueCollision : 1;
	dgUnsigned32 m_inCallback : 1;

	void *m_userData;
	dgWorld *m_world;
	dgCollision *m_collision;
	dgBroadPhaseList m_collisionCell;
	dgBodyMasterList::dgListNode *m_masterNode;

	OnBodyDestroy m_destructor;
	OnMatrixUpdateCallback m_matrixUpdate;
	OnApplyExtForceAndTorque m_applyExtForces;

	void reset();

	friend class dgWorld;
	friend class dgContact;
	friend class dgCollision;
	friend class dgBodyChunk;
	friend class dgSortArray;
	friend class dgConstraint;
	friend class dgContactArray;
	friend class dgContactSolver;
	friend class dgBroadPhaseCell;
	friend class dgCollisionConvex;
	friend class dgCollisionEllipse;
	friend class dgCollisionCompound;
	friend class dgCollisionUserMesh;
	friend class dgWorldDynamicUpdate;
	friend class dgCollisionConvexHull;
	friend class dgCollisionScene;
	friend class dgCollisionBVH;
	friend class dgBodyMasterList;
	friend class dgJacobianMemory;
	friend class dgBilateralConstraint;
	friend class dgBroadPhaseCollision;
	friend class dgSolverWorlkerThreads;
	friend class dgCollisionConvexModifier;
	friend class dgCollidingPairCollector;
	friend class dgAABBOverlapPairList;
	friend class dgParallelSolverClear;
	friend class dgParallelSolverUpdateForce;
	friend class dgParallelSolverUpdateVeloc;
	friend class dgParallelSolverBodyInertia;
	friend class dgBroadPhaseApplyExternalForce;
	friend class dgParallelSolverBuildJacobianRows;
	friend class dgParallelSolverBuildJacobianMatrix;
} DG_GCC_VECTOR_ALIGMENT;

// *****************************************************************************
//
//	 Implementation
//
// *****************************************************************************

inline void dgBody::reset() {
	m_matrix = dgGetZeroMatrix();
	m_collisionWorldMatrix = dgGetZeroMatrix();
	m_invWorldInertiaMatrix = dgGetZeroMatrix();
	m_rotation = dgQuaternion(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));

	m_veloc = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_omega = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_accel = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_alpha = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_netForce = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_netTorque = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_prevExternalForce = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_prevExternalTorque = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));

	m_mass = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_invMass = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_aparentMass = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_localCentreOfMass = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_globalCentreOfMass = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_minAABB = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_maxAABB = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_dampCoef = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));

	m_index = 0;
	m_uniqueID = 0;
	m_bodyGroupId = 0;
	m_genericLRUMark = 0;
	m_sleepingCounter = 0;
	m_dynamicsLru = 0;
	m_isInDerstruionArrayLRU = 0;

	m_freeze = 0;
	m_sleeping = 0;
	m_autoSleep = 0;
	m_isInWorld = 0;
	m_equilibrium = 0;
	m_continueCollisionMode = 0;
	m_spawnnedFromCallback = 0;
	m_collideWithLinkedBodies = 0;
	m_solverInContinueCollision = 0;
	m_inCallback = 0;

	m_userData = NULL;
	m_world = NULL;
	m_collision = NULL;
	m_collisionCell.reset();
	m_masterNode = NULL;

	m_destructor = NULL;
	m_matrixUpdate = NULL;
	m_applyExtForces = NULL;
}

inline void dgBody::SetAutoSleep(bool state) {
	m_autoSleep = dgUnsigned32(state);
	if (m_autoSleep == 0) {
		m_sleeping = false;
	}
}

inline bool dgBody::GetAutoSleep() const {
	return m_autoSleep;
}

inline bool dgBody::GetSleepState() const {
	return m_sleeping;
}

/*
inline bool dgBody::GetActive () const
{
    return m_active;
}
*/

inline bool dgBody::GetCollisionWithLinkedBodies() const {
	return m_collideWithLinkedBodies;
}

inline void dgBody::SetCollisionWithLinkedBodies(bool state) {
	m_collideWithLinkedBodies = dgUnsigned32(state);
}

inline void dgBody::SetUserData(void *const userData) {
	m_userData = userData;
}

inline void *dgBody::GetUserData() const {
	return m_userData;
}

inline dgWorld *dgBody::GetWorld() const {
	return m_world;
}

inline dgUnsigned32 dgBody::GetGroupID() const {
	return dgUnsigned32(m_bodyGroupId);
}

inline void dgBody::SetGroupID(dgUnsigned32 id) {
	m_bodyGroupId = dgInt32(id);
}

inline void dgBody::SetDestructorCallback(OnBodyDestroy destructor) {
	m_destructor = destructor;
}

inline void dgBody::SetExtForceAndTorqueCallback(OnApplyExtForceAndTorque callback) {
	m_applyExtForces = callback;
}

inline OnApplyExtForceAndTorque dgBody::GetExtForceAndTorqueCallback() const {
	return m_applyExtForces;
}

/*
inline void dgBody::SetAutoactiveNotify (OnActivation activate)
{
    m_activation = activate;
    if (m_activation) {
        m_activation (*this, m_active ? 1 : 0);
    }
}
*/

inline void dgBody::SetMatrixUpdateCallback(OnMatrixUpdateCallback callback) {
	m_matrixUpdate = callback;
}

inline OnMatrixUpdateCallback dgBody::GetMatrixUpdateCallback() const {
	return m_matrixUpdate;
}

/*
inline void dgBody::SetFreezeTreshhold (dgFloat32 freezeAccel2, dgFloat32 freezeAlpha2, dgFloat32 freezeSpeed2, dgFloat32 freezeOmega2)
{
    m_freezeAccel2 = GetMax (freezeAccel2, dgFloat32(DG_FREEZE_MAG2));
    m_freezeAlpha2 = GetMax (freezeAlpha2, dgFloat32(DG_FREEZE_MAG2));
    m_freezeSpeed2 = GetMax (freezeSpeed2, dgFloat32(DG_FREEZE_MAG2));
    m_freezeOmega2 = GetMax (freezeOmega2, dgFloat32(DG_FREEZE_MAG2));
}

inline void dgBody::GetFreezeTreshhold (dgFloat32& freezeAccel2, dgFloat32& freezeAlpha2, dgFloat32& freezeSpeed2, dgFloat32& freezeOmega2) const
{
    freezeAccel2 = m_freezeAccel2;
    freezeAlpha2 = m_freezeAlpha2;
    freezeSpeed2 = m_freezeSpeed2;
    freezeOmega2 = m_freezeOmega2;
}
*/

inline void dgBody::SetOmega(const dgVector &omega) {
	m_omega = omega;
}

inline void dgBody::SetVelocity(const dgVector &velocity) {
	m_veloc = velocity;
}

inline void dgBody::SetCentreOfMass(const dgVector &com) {
	m_localCentreOfMass.m_x = com.m_x;
	m_localCentreOfMass.m_y = com.m_y;
	m_localCentreOfMass.m_z = com.m_z;
	m_localCentreOfMass.m_w = dgFloat32(1.0f);
	m_globalCentreOfMass = m_matrix.TransformVector(m_localCentreOfMass);
}

inline void dgBody::AddForce(const dgVector &force) {
	SetForce(m_accel + force);
}

inline void dgBody::AddTorque(const dgVector &torque) {
	SetTorque(torque + m_alpha);
}

inline const dgVector &dgBody::GetMass() const {
	return m_mass;
}

inline const dgVector &dgBody::GetAparentMass() const {
	return m_aparentMass;
}

inline const dgVector &dgBody::GetInvMass() const {
	return m_invMass;
}

inline const dgVector &dgBody::GetOmega() const {
	return m_omega;
}

inline const dgVector &dgBody::GetVelocity() const {
	return m_veloc;
}

inline const dgVector &dgBody::GetForce() const {
	return m_accel;
}

inline const dgVector &dgBody::GetTorque() const {
	return m_alpha;
}

inline const dgVector &dgBody::GetNetForce() const {
	return m_netForce;
}

inline const dgVector &dgBody::GetNetTorque() const {
	return m_netTorque;
}

inline dgCollision *dgBody::GetCollision() const {
	return m_collision;
}

inline const dgVector &dgBody::GetPosition() const {
	return m_matrix.m_posit;
}

inline const dgQuaternion &dgBody::GetRotation() const {
	return m_rotation;
}

inline const dgMatrix &dgBody::GetMatrix() const {
	return m_matrix;
}

inline dgVector dgBody::GetCentreOfMass() const {
	return m_localCentreOfMass;
}

inline void dgBody::GetAABB(dgVector &p0, dgVector &p1) const {
	p0.m_x = m_minAABB.m_x;
	p0.m_y = m_minAABB.m_y;
	p0.m_z = m_minAABB.m_z;
	p1.m_x = m_maxAABB.m_x;
	p1.m_y = m_maxAABB.m_y;
	p1.m_z = m_maxAABB.m_z;
}

/*
inline void dgBody::SetGyroscopicTorqueMode (bool mode)
{
    m_applyGyroscopic = mode;
}

inline bool dgBody::GetGyroscopicTorqueMode () const
{
    return m_applyGyroscopic;
}
*/

inline const dgMatrix &dgBody::GetCollisionMatrix() const {
	return m_collisionWorldMatrix;
}

inline void dgBody::SetContinuesCollisionMode(bool mode) {
	m_continueCollisionMode = dgUnsigned32(mode);
}

inline bool dgBody::GetContinuesCollisionMode() const {
	return m_continueCollisionMode;
}

inline void dgBody::ApplyExtenalForces(dgFloat32 timestep, dgInt32 threadIndex) {
	m_accel = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	m_alpha = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
	if (m_applyExtForces) {
		m_applyExtForces(*this, timestep, threadIndex);
	}
}

inline dgFloat32 dgBody::GetLinearDamping() const {
	//  return (m_linearDampCoef - DG_MIN_SPEED_ATT) / (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT);
	return (m_dampCoef.m_w - DG_MIN_SPEED_ATT) / (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT);
}

inline dgVector dgBody::GetAngularDamping() const {
	return dgVector((m_dampCoef.m_x - DG_MIN_SPEED_ATT) / (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT),
	                (m_dampCoef.m_y - DG_MIN_SPEED_ATT) / (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT),
	                (m_dampCoef.m_z - DG_MIN_SPEED_ATT) / (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT), dgFloat32(0.0f));
}

inline void dgBody::SetLinearDamping(dgFloat32 linearDamp) {
	linearDamp = ClampValue(linearDamp, dgFloat32(0.0f), dgFloat32(1.0f));
	m_dampCoef.m_w = DG_MIN_SPEED_ATT + (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT) * linearDamp;
}

inline void dgBody::SetAngularDamping(const dgVector &angularDamp) {
	dgFloat32 tmp;

	tmp = ClampValue(angularDamp.m_x, dgFloat32(0.0f), dgFloat32(1.0f));
	m_dampCoef.m_x = DG_MIN_SPEED_ATT + (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT) * tmp;

	tmp = ClampValue(angularDamp.m_y, dgFloat32(0.0f), dgFloat32(1.0f));
	m_dampCoef.m_y = DG_MIN_SPEED_ATT + (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT) * tmp;

	tmp = ClampValue(angularDamp.m_z, dgFloat32(0.0f), dgFloat32(1.0f));
	m_dampCoef.m_z = DG_MIN_SPEED_ATT + (DG_MAX_SPEED_ATT - DG_MIN_SPEED_ATT) * tmp;
}

inline void dgBody::AddDamingAcceleration() {
	m_veloc -= m_veloc.Scale(m_dampCoef.m_w);
	dgVector omega(m_matrix.UnrotateVector(m_omega));
	omega -= omega.CompProduct(m_dampCoef);
	m_omega = m_matrix.RotateVector(omega);
}

/*
inline void dgBody::AddGyroscopicTorque()
{
    NEWTON_ASSERT (0);
    if (m_applyGyroscopic) {
        const dgVector inertia = m_mass;
        dgVector omega (m_matrix.UnrotateVector (m_omega));
        m_alpha -= m_matrix.RotateVector(omega.CompProduct(inertia) * omega);
    }
}
*/

inline void dgBody::SetForce(const dgVector &force) {
	dgFloat32 errMag2;
	dgVector error;

	m_accel = force;
	error = m_accel - m_prevExternalForce;
	errMag2 = (error % error) * m_invMass[3] * m_invMass[3];
	if (errMag2 > DG_ErrTolerance2) {
		m_sleepingCounter = 0;
	}
}

inline void dgBody::SetTorque(const dgVector &torque) {
	dgFloat32 errMag2;
	dgVector error;

	m_alpha = torque;
	error = m_alpha - m_prevExternalTorque;
	errMag2 = (error % error) * m_invMass[3] * m_invMass[3];
	if (errMag2 > DG_ErrTolerance2) {
		m_sleepingCounter = 0;
	}
}

// inline int dgBody::GetApplicationFreezeState() const
//{
//	return m_aplycationFreeze ? 1 : 0;
// }
// inline void dgBody::SetApplicationFreezeState(dgInt32 state)
//{
//	m_aplycationFreeze = state ? true : false;
// }

inline dgBodyMasterList::dgListNode *dgBody::GetMasterList() const {
	return m_masterNode;
}

inline bool dgBody::GetFreeze() const {
	return m_freeze;
}

inline dgInt32 dgBody::GetUniqueID() const {
	return m_uniqueID;
}

inline bool dgBody::IsInEquelibrium() const {
	dgFloat32 invMassMag2 = m_invMass[3] * m_invMass[3];
	if (m_equilibrium) {
		dgVector error(m_accel - m_prevExternalForce);
		dgFloat32 errMag2 = (error % error) * invMassMag2;
		if (errMag2 < DG_ErrTolerance2) {
			error = m_alpha - m_prevExternalTorque;
			errMag2 = (error % error) * invMassMag2;
			if (errMag2 < DG_ErrTolerance2) {
				errMag2 = (m_netForce % m_netForce) * invMassMag2;
				if (errMag2 < DG_ErrTolerance2) {
					errMag2 = (m_netTorque % m_netTorque) * invMassMag2;
					if (errMag2 < DG_ErrTolerance2) {
						errMag2 = m_veloc % m_veloc;
						if (errMag2 < DG_ErrTolerance2) {
							errMag2 = m_omega % m_omega;
							if (errMag2 < DG_ErrTolerance2) {
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

inline void dgBody::SetMatrixOriginAndRotation(const dgMatrix &matrix) {
	m_matrix = matrix;

#ifdef _DEBUG
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			NEWTON_ASSERT(dgCheckFloat(m_matrix[i][j]));
		}
	}

	int j0 = 1;
	int j1 = 2;
	for (dgInt32 i = 0; i < 3; i++) {
		dgFloat32 val;
		NEWTON_ASSERT(m_matrix[i][3] == 0.0f);
		val = m_matrix[i] % m_matrix[i];
		NEWTON_ASSERT(dgAbsf(val - 1.0f) < 1.0e-5f);
		dgVector tmp(m_matrix[j0] * m_matrix[j1]);
		val = tmp % m_matrix[i];
		NEWTON_ASSERT(dgAbsf(val - 1.0f) < 1.0e-5f);
		j0 = j1;
		j1 = i;
	}
#endif

	m_rotation = dgQuaternion(m_matrix);
	m_globalCentreOfMass = m_matrix.TransformVector(m_localCentreOfMass);

	//  matrix.m_front = matrix.m_front.Scale (dgRsqrt (matrix.m_front % matrix.m_front));
	//  matrix.m_right = matrix.m_front * matrix.m_up;
	//  matrix.m_right = matrix.m_right.Scale (dgRsqrt (matrix.m_right % matrix.m_right));
	//  matrix.m_up = matrix.m_right * matrix.m_front;
}

#endif // !defined(AFX_DGBODY_H__C16EDCD6_53C4_4C6F_A70A_591819F7187E__INCLUDED_)
