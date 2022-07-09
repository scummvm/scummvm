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

#if !defined(AFX_DGCONSTRAINT_H__F9EC24E0_6E0F_4CD5_909E_A5F5E1AC7C0B__INCLUDED_)
#define AFX_DGCONSTRAINT_H__F9EC24E0_6E0F_4CD5_909E_A5F5E1AC7C0B__INCLUDED_

#include "dgBodyMasterList.h"

#define DG_MAX_BOUND						dgFloat32 (1.0e15f)
#define DG_MIN_BOUND						(-DG_MAX_BOUND)


#define DG_BILATERAL_CONSTRAINT				-1
#define DG_NORMAL_CONSTRAINT				-2 
#define DG_BILATERAL_FRICTION_CONSTRAINT	-3 

#define DG_CONSTRAINT_MAX_ROWS				 (3 * 16)

#define MIN_JOINT_PIN_LENGTH				dgFloat32 (16.0f)

class dgBody;
class dgWorld;
class dgConstraint;
class dgBilateralBounds;

typedef void (dgApi *ConstraintsForceFeeback) (const dgConstraint& me, dgFloat32 timestep, dgInt32 threadIndex);

class dgConstraintInfo
{
	public:

	dgMatrix m_attachMatrix_0;
	dgMatrix m_attachMatrix_1;
	dgFloat32 m_minLinearDof[3];
	dgFloat32 m_maxLinearDof[3];
	dgFloat32 m_minAngularDof[3];
	dgFloat32 m_maxAngularDof[3];
	dgBody* m_attachBody_0;
	dgBody* m_attachBody_1;
	dgFloat32 m_extraParameters[16];
	dgInt32 m_collideCollisionOn;
	char m_discriptionType[16];
};


class dgJointCallBackParam
{
	public:
	dgFloat32 m_accel;
	dgFloat32 m_minFriction;
	dgFloat32 m_maxFriction;
	dgFloat32 m_timestep;
};


class dgBilateralBounds
{
	public:
	dgFloat32 m_low;
	dgFloat32 m_upper;
	dgInt32 m_normalIndex;
	dgFloat32 *m_jointForce;
};

DG_MSC_VECTOR_ALIGMENT
class dgJacobian
{
	public:
	dgVector m_linear;
	dgVector m_angular;
} DG_GCC_VECTOR_ALIGMENT;

DG_MSC_VECTOR_ALIGMENT
class dgJacobianPair
{
	public:
	dgJacobian m_jacobian_IM0;
	dgJacobian m_jacobian_IM1;
} DG_GCC_VECTOR_ALIGMENT;

class dgJointAccelerationDecriptor
{
	public: 
	dgInt32 m_rowsCount;
	dgFloat32 m_timeStep;
	dgFloat32 m_invTimeStep;
	dgFloat32 m_firstPassCoefFlag;
//	dgBody *m_body0;
//	dgBody *m_body1;
	dgFloat32* m_penetration;
	dgFloat32* m_coordenateAccel;
	const dgJacobianPair* m_Jt;
	const dgFloat32* m_restitution;
	const dgInt32* m_accelIsMotor;
	const dgInt32* m_normalForceIndex;
	const dgFloat32* m_externAccelaration;
	const dgFloat32* m_penetrationStiffness;
	
};


DG_MSC_VECTOR_ALIGMENT
class dgContraintDescritor
{
	public:
	dgJacobianPair m_jacobian[DG_CONSTRAINT_MAX_ROWS];
	dgBilateralBounds m_forceBounds[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 m_jointAccel[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 m_jointStiffness[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 m_restitution[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 m_penetration[DG_CONSTRAINT_MAX_ROWS];
	dgFloat32 m_penetrationStiffness[DG_CONSTRAINT_MAX_ROWS];
	dgUnsigned32 m_isMotor[DG_CONSTRAINT_MAX_ROWS];
	dgWorld* m_world;
	dgInt32 m_threadIndex;
	dgFloat32 m_timestep;
	dgFloat32 m_invTimestep;
}DG_GCC_VECTOR_ALIGMENT;

enum dgConstraintID
{
	dgBallConstraintId,
	dgHingeConstraintId,
	dgSliderConstraintId,
	dgContactConstraintId,
	dgUpVectorConstraintId,
	dgUniversalConstraintId,
	dgCorkscrewConstraintId,
	dgPointToCurveConstraintId,

	dgUnknownConstraintId
};

typedef void (dgApi *OnConstraintDestroy) (dgConstraint& me);

DG_MSC_VECTOR_ALIGMENT
class dgConstraint
{
	public:
	DG_CLASS_ALLOCATOR(allocator)

	dgUnsigned32 GetId () const;
	dgBody* GetBody0 ()	const;
	dgBody* GetBody1 ()	const;
	dgBodyMasterListRow::dgListNode* GetLink0()	const;
	dgBodyMasterListRow::dgListNode* GetLink1()	const;
	void* GetUserData () const;
	bool IsCollidable () const;


	dgInt32 GetMaxDOF() const;
	void SetUserData (void *userData);
	void SetCollidable (bool state);
	virtual void SetDestructorCallback (OnConstraintDestroy destructor) = 0;

	virtual dgFloat32 GetStiffness() const;
	virtual void SetStiffness(dgFloat32 stiffness);
	virtual void GetInfo (dgConstraintInfo* const info) const;

	class dgPointParam
	{
		public:
		dgVector m_r0;
		dgVector m_r1;
		dgVector m_posit0;
		dgVector m_posit1;
		dgVector m_veloc0;
		dgVector m_veloc1;
		dgVector m_centripetal0;
		dgVector m_centripetal1;
		dgFloat32 m_stiffness;
	};


	protected:
	dgConstraint();
	virtual ~dgConstraint();

	virtual bool IsBilateral () const;
	

	virtual dgUnsigned32 JacobianDerivative (dgContraintDescritor& params) = 0; 
	virtual void JointAccelerations(const dgJointAccelerationDecriptor& params) = 0; 
	virtual void JointAccelerationsSimd(const dgJointAccelerationDecriptor& params) = 0;  

	virtual void JointVelocityCorrection(const dgJointAccelerationDecriptor& params) = 0; 

	void SetUpdateFeedbackFunction (ConstraintsForceFeeback function);
	void InitPointParam (dgPointParam& param, dgFloat32 stiffness, const dgVector& p0Global, const dgVector& p1Global) const;

	
	void InitInfo (dgConstraintInfo* const info) const;

	void* m_userData;
	dgBody* m_body0;
	dgBody* m_body1;
	dgBodyMasterListRow::dgListNode* m_link0;
	dgBodyMasterListRow::dgListNode* m_link1;
	ConstraintsForceFeeback m_updaFeedbackCallback;
	dgUnsigned32 m_dynamicsLru;

	dgUnsigned32 m_index			: 16;
	dgUnsigned32 m_maxDOF			:  6;
	dgUnsigned32 m_constId			:  6;		
	dgUnsigned32 m_enableCollision	:  1;
	dgUnsigned32 m_isUnilateral		:  1;

	friend class dgWorld;
	friend class dgJacobianMemory;
	friend class dgBodyMasterList;
	friend class dgWorldDynamicUpdate;
	friend class dgParallelSolverJointAcceleration;
	friend class dgParallelSolverInitFeedbackUpdate;
	friend class dgParallelSolverBuildJacobianMatrix;
	friend class dgBroadPhaseMaterialCallbackWorkerThread;
}DG_GCC_VECTOR_ALIGMENT;

inline dgConstraint::dgConstraint() 
{
	_ASSERTE ((((dgUnsigned64) this) & 15) == 0);

	m_link0 = NULL;
	m_link1 = NULL;
	m_body0 = NULL;
	m_body1 = NULL;
	m_userData = NULL;

	m_maxDOF = 6;
	m_dynamicsLru = 0;
	m_isUnilateral = false;
	m_enableCollision = false;
	m_constId = dgUnknownConstraintId;
	m_updaFeedbackCallback = NULL;
}

inline dgConstraint::~dgConstraint()
{
}

inline void dgConstraint::SetUpdateFeedbackFunction (ConstraintsForceFeeback function)
{
	m_updaFeedbackCallback = function;
}

inline bool dgConstraint::IsCollidable () const
{
	return m_enableCollision ? true : false;
}

inline void dgConstraint::SetCollidable (bool state)
{
	m_enableCollision = dgUnsigned32 (state);
}

inline dgUnsigned32 dgConstraint::GetId () const
{
	return m_constId;
}

inline dgBody* dgConstraint::GetBody0 () const
{
	return m_body0;
}

inline dgBody* dgConstraint::GetBody1 () const
{
	return m_body1;
}

inline dgBodyMasterListRow::dgListNode* dgConstraint::GetLink0()	const
{
	return m_link0;
}
inline dgBodyMasterListRow::dgListNode* dgConstraint::GetLink1()	const
{
	return m_link1;
}


inline dgFloat32 dgConstraint::GetStiffness() const
{
	return dgFloat32 (1.0f);
}

inline void dgConstraint::SetStiffness(dgFloat32 stiffness)
{
}

inline dgInt32 dgConstraint::GetMaxDOF() const
{
	return dgInt32 (m_maxDOF);
}

#endif // !defined(AFX_DGCONSTRAINT_H__F9EC24E0_6E0F_4CD5_909E_A5F5E1AC7C0B__INCLUDED_)

