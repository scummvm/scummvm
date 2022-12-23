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

#include "NewtonStdAfx.h"
#include "NewtonClass.h"

NewtonDeadBodies::NewtonDeadBodies(dgMemoryAllocator *const allocator) :
	dgTree<dgBody *, void *>(allocator) {
	Insert((dgBody *) NULL, 0);
}

void NewtonDeadBodies::DestroyBodies(Newton &world) {
	dgBody *body;
	dgTreeNode *node;

	Iterator iter(*this);
	for (iter.Begin(); iter;) {
		node = iter.GetNode();
		iter++;
		body = node->GetInfo();
		if (body) {
			Remove(node);
			world.DestroyBody(body);
		}
	}
}

NewtonDeadJoints::NewtonDeadJoints(dgMemoryAllocator *const allocator) :
	dgTree<dgConstraint *, void *>(allocator) {
	Insert((dgConstraint *) NULL, 0);
}

void NewtonDeadJoints::DestroyJoints(Newton &world) {
	dgTreeNode *node;
	dgConstraint *joint;

	Iterator iter(*this);
	for (iter.Begin(); iter;) {
		node = iter.GetNode();
		iter++;
		joint = node->GetInfo();
		if (joint) {
			Remove(node);
			world.DestroyConstraint(joint);
		}
	}
}

void *Newton::DefaultAllocMemory(dgInt32 size) {
	return malloc(size_t(size));
}

void Newton::DefaultFreeMemory(void *ptr, dgInt32 size) {
	free(ptr);
}

Newton::Newton(dgFloat32 scale, dgMemoryAllocator *const allocator) :
	dgWorld(allocator), NewtonDeadBodies(allocator), NewtonDeadJoints(allocator) {
	m_updating = false;
	g_maxTimeStep = dgFloat32(1.0f / 60.0f);

	m_destructor = NULL;
//	SetGlobalScale (scale);
}

Newton::~Newton() {
	if (m_destructor) {
		m_destructor((NewtonWorld *) this);
	}
}

void Newton::UpdatePhysics(dgFloat32 timestep) {
	m_updating = true;
	Update(timestep);

//	RagdollHeaderActiveList::UpdateMatrix();
	m_updating = false;

	NewtonDeadBodies &bodyList = *this;
	NewtonDeadJoints &jointList = *this;

	jointList.DestroyJoints(*this);
	bodyList.DestroyBodies(*this);
}

void Newton::DestroyJoint(dgConstraint *joint) {
	if (m_updating) {
		NewtonDeadJoints &jointList = *this;
		jointList.Insert(joint, joint);
	} else {
		dgWorld::DestroyConstraint(joint);
	}
}

void Newton::DestroyBody(dgBody *body) {
	if (m_updating) {
		NewtonDeadBodies &bodyList = *this;
		bodyList.Insert(body, body);
	} else {
		dgWorld::DestroyBody(body);
	}
}

NewtonUserJoint::NewtonUserJoint(dgWorld *world, dgInt32 maxDof,
                                 NewtonUserBilateralCallBack callback,
                                 NewtonUserBilateralGetInfoCallBack getInfo, dgBody *dyn0, dgBody *dyn1) :
	dgUserConstraint(world, dyn0, dyn1, 1) {
	m_rows = 0;
	m_maxDOF = dgUnsigned8(maxDof);
	m_jacobianFnt = callback;
	m_getInfoCallback = getInfo;

	NEWTON_ASSERT(world);
	m_forceArray = m_jointForce;
	if (m_maxDOF > 24) {
		NEWTON_ASSERT(0);
		m_forceArray = (dgFloat32 *) world->GetAllocator()->Malloc(
		                   dgInt32(m_maxDOF * sizeof(dgFloat32)));
	}
	memset(m_forceArray, 0, m_maxDOF * sizeof(dgFloat32));
}

NewtonUserJoint::~NewtonUserJoint() {
	if (m_forceArray != m_jointForce) {
		m_body0->GetWorld()->GetAllocator()->Free(m_forceArray);
	}

}

dgUnsigned32 NewtonUserJoint::JacobianDerivative(dgContraintDescritor &params) {
	m_rows = 0;
	m_param = &params;
	m_jacobianFnt((NewtonJoint *) this, params.m_timestep, params.m_threadIndex);
	return dgUnsigned32(m_rows);
}

void NewtonUserJoint::AddLinearRowJacobian(const dgVector &pivot0,
        const dgVector &pivot1, const dgVector &dir) {
	dgPointParam pointData;
	InitPointParam(pointData, m_stiffness, pivot0, pivot1);

	m_lastPosit0 = pivot0;
	m_lastPosit1 = pivot1;
	m_lastJointAngle = dgFloat32(0.0f);
	CalculatePointDerivative(m_rows, *m_param, dir, pointData,
	                         &m_forceArray[m_rows]);
	m_rows++;
	NEWTON_ASSERT(m_rows <= dgInt32(m_maxDOF));
}

void NewtonUserJoint::AddAngularRowJacobian(const dgVector &dir,
        dgFloat32 relAngle) {
	m_lastPosit0 = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                        dgFloat32(0.0f));
	m_lastPosit1 = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                        dgFloat32(0.0f));
	m_lastJointAngle = relAngle;
	CalculateAngularDerivative(m_rows, *m_param, dir, m_stiffness, relAngle,
	                           &m_forceArray[m_rows]);
	m_rows++;
	NEWTON_ASSERT(m_rows <= dgInt32(m_maxDOF));
}

void NewtonUserJoint::AddGeneralRowJacobian(const dgFloat32 *jacobian0,
        const dgFloat32 *jacobian1) {
	m_lastPosit0 = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                        dgFloat32(0.0f));
	m_lastPosit1 = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                        dgFloat32(0.0f));
	m_lastJointAngle = 0.0f;

	SetJacobianDerivative(m_rows, *m_param, jacobian0, jacobian1,
	                      &m_forceArray[m_rows]);
	m_rows++;
	NEWTON_ASSERT(m_rows <= dgInt32(m_maxDOF));
}

void NewtonUserJoint::SetAcceleration(dgFloat32 acceleration) {
	dgInt32 index;
	index = m_rows - 1;
	if ((index >= 0) && (index < dgInt32(m_maxDOF))) {
//		m_param->m_jointAccel[index] = acceleration;
		SetMotorAcceleration(index, acceleration, *m_param);
	}
}

void NewtonUserJoint::SetSpringDamperAcceleration(dFloat springK,
        dFloat springD) {
	dgInt32 index;
	index = m_rows - 1;
	if ((index >= 0) && (index < dgInt32(m_maxDOF))) {
		dgFloat32 accel;
		accel = CalculateSpringDamperAcceleration(index, *m_param, m_lastJointAngle,
		        m_lastPosit0, m_lastPosit1, springK, springD);
		NEWTON_ASSERT(0);
//		m_param->m_jointAccel[index] = accel;
		SetMotorAcceleration(index, accel, *m_param);
	}
}

void NewtonUserJoint::SetHighFriction(dgFloat32 friction) {
	dgInt32 index;
	index = m_rows - 1;
	if ((index >= 0) && (index < dgInt32(m_maxDOF))) {
		m_param->m_forceBounds[index].m_upper = ClampValue(friction,
		                                        dgFloat32(0.001f), dgFloat32(DG_MAX_BOUND));
		m_param->m_forceBounds[index].m_normalIndex =
		    DG_BILATERAL_FRICTION_CONSTRAINT;
	}
}

void NewtonUserJoint::SetLowerFriction(dgFloat32 friction) {
	dgInt32 index;
	index = m_rows - 1;
	if ((index >= 0) && (index < dgInt32(m_maxDOF))) {
		m_param->m_forceBounds[index].m_low = ClampValue(friction,
		                                      dgFloat32(DG_MIN_BOUND), dgFloat32(-0.001f));
		m_param->m_forceBounds[index].m_normalIndex =
		    DG_BILATERAL_FRICTION_CONSTRAINT;
	}
}

void NewtonUserJoint::SetRowStiffness(dgFloat32 stiffness) {
	dgInt32 index;
	index = m_rows - 1;
	if ((index >= 0) && (index < dgInt32(m_maxDOF))) {
		stiffness = ClampValue(stiffness, dgFloat32(0.0f), dgFloat32(1.0f));
		stiffness = 100.0f - stiffness * 99.0f;
		m_param->m_jointStiffness[index] = stiffness;
	}
}

dgFloat32 NewtonUserJoint::GetRowForce(dgInt32 row) const {
	dgFloat32 force;

	force = 0.0f;
	if ((row >= 0) && (row < dgInt32(m_maxDOF))) {
		force = m_forceArray[row];
	}
	return force;
}

void NewtonUserJoint::GetInfo(dgConstraintInfo *const info) const {
	info->clear();
	if (m_getInfoCallback) {
		InitInfo(info);
		m_getInfoCallback((const NewtonJoint *)this, (NewtonJointRecord *) info);
	}
}

void NewtonUserJoint::SetUpdateFeedbackFunction(
    NewtonUserBilateralCallBack getFeedback) {
	dgUserConstraint::SetUpdateFeedbackFunction(
	    (ConstraintsForceFeeback) getFeedback);
}
