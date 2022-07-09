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

#include "dgCorkscrewConstraint.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgCorkscrewConstraint::dgCorkscrewConstraint() : dgBilateralConstraint() {

	_ASSERTE((((dgUnsigned64)&m_localMatrix0) & 15) == 0);

	m_maxDOF = 6;
	m_constId = dgCorkscrewConstraintId;
	m_posit = dgFloat32(0.0f);
	m_angle = dgFloat32(0.0f);
	m_jointAccelFnt = NULL;
}

dgCorkscrewConstraint::~dgCorkscrewConstraint() {
}

/*
 dgCorkscrewConstraint* dgCorkscrewConstraint::Create(dgWorld* world)
 {
 dgCorkscrewConstraint*	constraint;

 //	constraint = dgCorkscrewConstraintArray::GetPool().GetElement();
 dgCorkscrewConstraintArray& array = * world;
 constraint = array.GetElement();

 _ASSERTE ((((dgUnsigned64) &constraint->m_localMatrix0) & 15) == 0);
 constraint->Init ();
 constraint->m_maxDOF = 6;
 constraint->m_constId = dgCorkscrewConstraintId;

 constraint->m_posit = dgFloat32 (0.0f);
 constraint->m_angle = dgFloat32 (0.0f);
 constraint->m_jointAccelFnt = NULL;
 return constraint;
 }

 void dgCorkscrewConstraint::Remove(dgWorld* world)
 {
 dgCorkscrewConstraintArray& array = * world;

 dgBilateralConstraint::Remove (world);
 //	dgCorkscrewConstraintArray::GetPool().RemoveElement (this);
 array.RemoveElement (this);
 }
 */

void dgCorkscrewConstraint::SetJointParameterCallBack(
	dgCorkscrewJointAcceleration callback) {
	m_jointAccelFnt = callback;
}

dgFloat32 dgCorkscrewConstraint::GetJointAngle() const {
	return m_angle;
}

dgFloat32 dgCorkscrewConstraint::GetJointPosit() const {
	return m_posit;
}

dgFloat32 dgCorkscrewConstraint::GetJointOmega() const {
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);
	dgVector dir(m_body0->GetMatrix().RotateVector(m_localMatrix0[0]));
	const dgVector &omega0 = m_body0->GetOmega();
	const dgVector &omega1 = m_body1->GetOmega();

	//	dgVector omega1 (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	//	if (m_body1) {
	//		omega1 = m_body1->GetOmega();
	//	}
	return (omega0 - omega1) % dir;
}

dgFloat32 dgCorkscrewConstraint::GetJointVeloc() const {
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);
	dgVector dir(m_body0->GetMatrix().RotateVector(m_localMatrix0[0]));
	const dgVector &veloc0 = m_body0->GetVelocity();
	const dgVector &veloc1 = m_body1->GetVelocity();

	//	dgVector veloc1 (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	//	if (m_body1) {
	//		veloc1 = m_body1->GetVelocity();
	//	}
	return (veloc0 - veloc1) % dir;
}

dgFloat32 dgCorkscrewConstraint::CalculateStopAlpha(dgFloat32 angle,
													const dgJointCallBackParam *param) const {
	dgFloat32 alpha;
	dgFloat32 omega;
	dgFloat32 penetrationErr;

	alpha = dgFloat32(0.0f);
	if (m_angle > angle) {
		omega = GetJointOmega();
		if (omega < dgFloat32(0.0f)) {
			omega = dgFloat32(0.0f);
		}
		penetrationErr = angle - m_angle;
		alpha = dgFloat32(100.0f) * penetrationErr - omega * dgFloat32(1.01f) / param->m_timestep;

	} else if (m_angle < angle) {
		omega = GetJointOmega();
		if (omega > dgFloat32(0.0f)) {
			omega = dgFloat32(0.0f);
		}

		penetrationErr = angle - m_angle;
		alpha = dgFloat32(100.0f) * penetrationErr - omega * dgFloat32(1.01f) / param->m_timestep;
	}
	return alpha;
}

dgFloat32 dgCorkscrewConstraint::CalculateStopAccel(dgFloat32 distance,
													const dgJointCallBackParam *param) const {
	dgFloat32 accel;
	dgFloat32 speed;
	dgFloat32 penetrationErr;

	accel = dgFloat32(0.0f);
	if (m_posit > distance) {
		speed = GetJointVeloc();
		if (speed < dgFloat32(0.0f)) {
			speed = dgFloat32(0.0f);
		}
		penetrationErr = (distance - m_posit);
		accel = dgFloat32(100.0f) * penetrationErr - speed * dgFloat32(1.01f) / param->m_timestep;

	} else if (m_posit < distance) {
		speed = GetJointVeloc();
		if (speed > dgFloat32(0.0f)) {
			speed = dgFloat32(0.0f);
		}
		penetrationErr = distance - m_posit;
		_ASSERTE(penetrationErr >= dgFloat32(0.0f));
		accel = dgFloat32(100.0f) * penetrationErr - speed * dgFloat32(1.01f) / param->m_timestep;
	}
	return accel;
}

dgVector dgCorkscrewConstraint::GetJointForce() const {
	dgMatrix matrix0;
	dgMatrix matrix1;

	CalculateGlobalMatrixAndAngle(matrix0, matrix1);

	return dgVector(
		matrix0.m_up.Scale(m_jointForce[0]) + matrix0.m_right.Scale(m_jointForce[1]) + matrix0.m_up.Scale(m_jointForce[2]) + matrix0.m_right.Scale(m_jointForce[3]));
}

dgUnsigned32 dgCorkscrewConstraint::JacobianDerivative(
	dgContraintDescritor &params) {
	dgMatrix matrix0;
	dgMatrix matrix1;

	dgVector angle(CalculateGlobalMatrixAndAngle(matrix0, matrix1));

	m_angle = -angle.m_x;
	m_posit = (matrix0.m_posit - matrix1.m_posit) % matrix0.m_front;
	matrix1.m_posit += matrix1.m_front.Scale(m_posit);

	_ASSERTE(
		dgAbsf(dgFloat32(1.0f) - (matrix0.m_front % matrix0.m_front)) < dgFloat32(1.0e-5f));
	_ASSERTE(
		dgAbsf(dgFloat32(1.0f) - (matrix0.m_up % matrix0.m_up)) < dgFloat32(1.0e-5f));
	_ASSERTE(
		dgAbsf(dgFloat32(1.0f) - (matrix0.m_right % matrix0.m_right)) < dgFloat32(1.0e-5f));

	const dgVector &dir1 = matrix0.m_up;
	const dgVector &dir2 = matrix0.m_right;

	//	const dgVector& p0 = matrix0.m_posit;
	//	const dgVector& p1 = matrix1.m_posit;
	dgVector p0(matrix0.m_posit);
	dgVector p1(
		matrix1.m_posit + matrix1.m_front.Scale((p0 - matrix1.m_posit) % matrix1.m_front));

	dgVector q0(p0 + matrix0.m_front.Scale(MIN_JOINT_PIN_LENGTH));
	dgVector q1(p1 + matrix1.m_front.Scale(MIN_JOINT_PIN_LENGTH));

	dgPointParam pointDataP;
	dgPointParam pointDataQ;
	InitPointParam(pointDataP, m_stiffness, p0, p1);
	InitPointParam(pointDataQ, m_stiffness, q0, q1);

	CalculatePointDerivative(0, params, dir1, pointDataP, &m_jointForce[0]);
	CalculatePointDerivative(1, params, dir2, pointDataP, &m_jointForce[1]);
	CalculatePointDerivative(2, params, dir1, pointDataQ, &m_jointForce[2]);
	CalculatePointDerivative(3, params, dir2, pointDataQ, &m_jointForce[3]);

	dgInt32 ret = 4;
	if (m_jointAccelFnt) {
		dgUnsigned32 code;
		dgJointCallBackParam axisParam[2];

		// linear acceleration
		axisParam[0].m_accel = dgFloat32(0.0f);
		axisParam[0].m_timestep = params.m_timestep;
		axisParam[0].m_minFriction = DG_MIN_BOUND;
		axisParam[0].m_maxFriction = DG_MAX_BOUND;

		// angular acceleration
		axisParam[1].m_accel = dgFloat32(0.0f);
		axisParam[1].m_timestep = params.m_timestep;
		axisParam[1].m_minFriction = DG_MIN_BOUND;
		axisParam[1].m_maxFriction = DG_MAX_BOUND;

		code = m_jointAccelFnt(*this, axisParam);
		if (code & 1) {
			if ((axisParam[0].m_minFriction > DG_MIN_BOUND) || (axisParam[0].m_maxFriction < DG_MAX_BOUND)) {
				params.m_forceBounds[ret].m_low = axisParam[0].m_minFriction;
				params.m_forceBounds[ret].m_upper = axisParam[0].m_maxFriction;
				params.m_forceBounds[ret].m_normalIndex =
					DG_BILATERAL_FRICTION_CONSTRAINT;
			}

			CalculatePointDerivative(ret, params, matrix0.m_front, pointDataP,
									 &m_jointForce[ret]);
			// params.m_jointAccel[ret] = axisParam[0].m_accel;
			SetMotorAcceleration(ret, axisParam[0].m_accel, params);
			ret++;
		}

		if (code & 2) {
			if ((axisParam[1].m_minFriction > DG_MIN_BOUND) || (axisParam[1].m_maxFriction < DG_MAX_BOUND)) {
				params.m_forceBounds[ret].m_low = axisParam[1].m_minFriction;
				params.m_forceBounds[ret].m_upper = axisParam[1].m_maxFriction;
				params.m_forceBounds[ret].m_normalIndex =
					DG_BILATERAL_FRICTION_CONSTRAINT;
			}

			//			dgVector p (p0 +  dir1);
			//			dgPointParam pointData;
			//			InitPointParam (pointData, m_stiffness, p, p);
			//			CalculatePointDerivative (ret, params, dir2, pointData, &m_jointForce[ret]);
			CalculateAngularDerivative(ret, params, matrix0.m_front, m_stiffness,
									   dgFloat32(0.0f), &m_jointForce[ret]);
			// params.m_jointAccel[ret] = axisParam[1].m_accel;
			SetMotorAcceleration(ret, axisParam[1].m_accel, params);
			ret++;
		}
	}

	return dgUnsigned32(ret);
}
