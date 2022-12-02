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

#ifndef __NewotnClass_3HL6356GYL459020__
#define __NewotnClass_3HL6356GYL459020__

#include "NewtonStdAfx.h"
#include "Newton.h"



#define MAX_TIMESTEP (1.0f / 60.0f)
#define MIN_TIMESTEP (1.0f / 1000.0f)

class Newton;


class NewtonDeadBodies: public dgTree<dgBody *, void * > {
public:
	NewtonDeadBodies(dgMemoryAllocator *const allocator);
	void DestroyBodies(Newton &world);
};


class NewtonDeadJoints: public dgTree<dgConstraint *, void *> {
public:
	NewtonDeadJoints(dgMemoryAllocator *const allocator);
	void DestroyJoints(Newton &world);
};


class Newton:
	public dgWorld,
	public NewtonDeadBodies,
	public NewtonDeadJoints {
public:
	DG_CLASS_ALLOCATOR(allocator)

	Newton(dgFloat32 scale, dgMemoryAllocator *const allocator);
	~Newton();

	void DestroyBody(dgBody *body);
	void DestroyJoint(dgConstraint *joint);

	void UpdatePhysics(dgFloat32 timestep);
	static void *DefaultAllocMemory(dgInt32 size);
	static void DefaultFreeMemory(void *ptr, dgInt32 size);

	dgFloat32 g_maxTimeStep;
	bool m_updating;

	NewtonDestroyWorld m_destructor;

};



class NewtonUserJoint: public dgUserConstraint {
public:
	NewtonUserJoint(dgWorld *world, dgInt32 maxDof,
	                NewtonUserBilateralCallBack callback, NewtonUserBilateralGetInfoCallBack getInfo,
	                dgBody *dyn0, dgBody *dyn1);
	~NewtonUserJoint();

	dgUnsigned32 JacobianDerivative(dgContraintDescritor &params);

	void AddAngularRowJacobian(const dgVector &dir, dgFloat32 relAngle);
	void AddGeneralRowJacobian(const dgFloat32 *jacobian0, const dgFloat32 *jacobian1);
	void AddLinearRowJacobian(const dgVector &pivot0, const dgVector &pivot1, const dgVector &dir);

	dgFloat32 GetRowForce(dgInt32 row) const;
	void SetHighFriction(dgFloat32 friction);
	void SetLowerFriction(dgFloat32 friction);
	void SetRowStiffness(dgFloat32 stiffness);
	void SetAcceleration(dgFloat32 acceleration);
	void SetSpringDamperAcceleration(dgFloat32 springK, dgFloat32 springD);
	void GetInfo(dgConstraintInfo *const info) const;

	void SetUpdateFeedbackFunction(NewtonUserBilateralCallBack getFeedback);



private:
	NewtonUserBilateralCallBack m_jacobianFnt;
	NewtonUserBilateralGetInfoCallBack m_getInfoCallback;

	dgInt32 m_rows;
	dgFloat32 *m_forceArray;
	dgContraintDescritor *m_param;

	dgFloat32 m_lastJointAngle;
	dgVector m_lastPosit0;
	dgVector m_lastPosit1;
};



#endif
