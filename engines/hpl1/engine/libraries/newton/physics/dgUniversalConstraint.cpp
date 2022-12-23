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

#include "dgUniversalConstraint.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgUniversalConstraint::dgUniversalConstraint() : dgBilateralConstraint() {
	NEWTON_ASSERT((((dgUnsigned64)&m_localMatrix0) & 15) == 0);
	m_maxDOF = 6;
	m_constId = dgUniversalConstraintId;

	m_angle0 = dgFloat32(0.0f);
	m_angle1 = dgFloat32(0.0f);
	m_jointAccelFnt = NULL;
}

dgUniversalConstraint::~dgUniversalConstraint() {
}

/*
 dgUniversalConstraint* dgUniversalConstraint::Create(dgWorld* world)
 {
 dgUniversalConstraint* constraint;

 // constraint = dgUniversalConstraintArray::GetPool().GetElement();

 dgUniversalConstraintArray& array = * world;
 constraint = array.GetElement();

 NEWTON_ASSERT ((((dgUnsigned64) &constraint->m_localMatrix0) & 15) == 0);
 constraint->Init ();
 constraint->m_maxDOF = 6;
 constraint->m_constId = dgUniversalConstraintId;

 constraint->m_angle0 = dgFloat32 (0.0f);
 constraint->m_angle1 = dgFloat32 (0.0f);
 constraint->m_jointAccelFnt = NULL;
 return constraint;
 }

 void dgUniversalConstraint::Remove(dgWorld* world)
 {
 dgUniversalConstraintArray& array = * world;

 dgBilateralConstraint::Remove (world);
 // dgUniversalConstraintArray::GetPool().RemoveElement (this);
 array.RemoveElement (this);
 }
 */

void dgUniversalConstraint::SetJointParameterCallBack(
    dgUniversalJointAcceleration callback) {
	m_jointAccelFnt = callback;
}

dgFloat32 dgUniversalConstraint::GetJointAngle0() const {
	return m_angle0;
}

dgFloat32 dgUniversalConstraint::GetJointAngle1() const {
	return m_angle1;
}

dgFloat32 dgUniversalConstraint::GetJointOmega0() const {
	NEWTON_ASSERT(m_body0);
	NEWTON_ASSERT(m_body1);

	dgVector dir(m_body0->GetMatrix().RotateVector(m_localMatrix0[0]));
	const dgVector &omega0 = m_body0->GetOmega();
	const dgVector &omega1 = m_body1->GetOmega();

	//  dgVector omega1 (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	//  if (m_body1) {
	//      omega1 = m_body1->GetOmega();
	//  }
	return (omega0 - omega1) % dir;
}

dgFloat32 dgUniversalConstraint::GetJointOmega1() const {
	NEWTON_ASSERT(m_body0);
	NEWTON_ASSERT(m_body1);

	dgVector dir(m_body1->GetMatrix().RotateVector(m_localMatrix1[1]));
	const dgVector &omega0 = m_body0->GetOmega();
	const dgVector &omega1 = m_body1->GetOmega();

	//  dgMatrix matrix (dgGetIdentityMatrix());
	//  if (m_body1) {
	//      matrix = m_body1->GetMatrix();
	//      omega1 = m_body1->GetOmega();
	//  }
	//  dgVector dir (matrix.RotateVector (m_localMatrix1[1]));

	return (omega0 - omega1) % dir;
}

dgFloat32 dgUniversalConstraint::CalculateStopAlpha0(dgFloat32 angle,
        const dgJointCallBackParam *param) const {
	dgFloat32 alpha;
	dgFloat32 omega;
	dgFloat32 penetrationErr;

	alpha = dgFloat32(0.0f);
	if (m_angle0 > angle) {
		omega = GetJointOmega0();
		if (omega < dgFloat32(0.0f)) {
			omega = dgFloat32(0.0f);
		}
		penetrationErr = angle - m_angle0;
		alpha = 100.0f * penetrationErr - omega * 1.01f / param->m_timestep;

	} else if (m_angle0 < angle) {
		omega = GetJointOmega0();
		if (omega > dgFloat32(0.0f)) {
			omega = dgFloat32(0.0f);
		}

		penetrationErr = angle - m_angle0;
		alpha = 100.0f * penetrationErr - omega * 1.01f / param->m_timestep;
	}
	return alpha;
}

dgFloat32 dgUniversalConstraint::CalculateStopAlpha1(dgFloat32 angle,
        const dgJointCallBackParam *param) const {
	dgFloat32 alpha;
	dgFloat32 omega;
	dgFloat32 penetrationErr;

	alpha = dgFloat32(0.0f);
	if (m_angle1 > angle) {
		omega = GetJointOmega1();
		if (omega > dgFloat32(0.0f)) {
			omega = dgFloat32(0.0f);
		}
		penetrationErr = m_angle1 - angle;
		//      alpha = (penetrationErr + omega * 1.01f) / param->m_timestep;
		alpha = penetrationErr * 100.0f - omega * 1.01f / param->m_timestep;
	} else if (m_angle1 < angle) {
		omega = GetJointOmega1();
		if (omega < dgFloat32(0.0f)) {
			omega = dgFloat32(0.0f);
		}

		penetrationErr = m_angle1 - angle;
		//      alpha = (penetrationErr + omega * 1.01f) / param->m_timestep;
		alpha = penetrationErr * 100.0f - omega * 1.01f / param->m_timestep;
	}
	return alpha;
}

dgVector dgUniversalConstraint::GetJointForce() const {
	dgMatrix matrix0;
	dgMatrix matrix1;

	CalculateGlobalMatrixAndAngle(matrix0, matrix1);

	return dgVector(
	           matrix0.m_up.Scale(m_jointForce[0]) + matrix0.m_right.Scale(m_jointForce[1]) + matrix0.m_up.Scale(m_jointForce[2]) + matrix0.m_right.Scale(m_jointForce[3]));
}

dgUnsigned32 dgUniversalConstraint::JacobianDerivative(
    dgContraintDescritor &params) {
	dgInt32 ret;
	dgFloat32 sinAngle;
	dgFloat32 cosAngle;
	dgMatrix matrix0;
	dgMatrix matrix1;

	CalculateGlobalMatrixAndAngle(matrix0, matrix1);

	const dgVector &dir0 = matrix0.m_front;
	const dgVector &dir1 = matrix1.m_up;
	dgVector dir2(dir0 * dir1);

	dgVector dir3(dir2 * dir0);
	dir3 = dir3.Scale(dgRsqrt(dir3 % dir3));

	const dgVector &p0 = matrix0.m_posit;
	const dgVector &p1 = matrix1.m_posit;

	dgVector q0(p0 + dir3.Scale(MIN_JOINT_PIN_LENGTH));
	dgVector q1(p1 + dir1.Scale(MIN_JOINT_PIN_LENGTH));

	dgPointParam pointDataP;
	dgPointParam pointDataQ;
	InitPointParam(pointDataP, m_stiffness, p0, p1);
	InitPointParam(pointDataQ, m_stiffness, q0, q1);

	CalculatePointDerivative(0, params, dir0, pointDataP, &m_jointForce[0]);
	CalculatePointDerivative(1, params, dir1, pointDataP, &m_jointForce[1]);
	CalculatePointDerivative(2, params, dir2, pointDataP, &m_jointForce[2]);
	CalculatePointDerivative(3, params, dir0, pointDataQ, &m_jointForce[3]);
	ret = 4;

	//  dgVector sinAngle0 (matrix1.m_up * matrix0.m_up);
	//  m_angle0 = dgAsin (ClampValue (sinAngle0 % dir0, -0.9999999f, 0.9999999f));
	//  if ((matrix0.m_up % matrix1.m_up) < dgFloat32 (0.0f)) {
	//      m_angle0 = (m_angle0 >= dgFloat32 (0.0f)) ? dgPI - m_angle0 : dgPI + m_angle0;
	//  }

	sinAngle = (matrix1.m_up * matrix0.m_up) % matrix0.m_front;
	cosAngle = matrix0.m_up % matrix1.m_up;
	//  NEWTON_ASSERT (dgAbsf (m_angle0 - dgAtan2 (sinAngle, cosAngle)) < 1.0e-1f);
	m_angle0 = dgAtan2(sinAngle, cosAngle);

	//  dgVector sinAngle1 (matrix0.m_front * matrix1.m_front);
	//  m_angle1 = dgAsin (ClampValue (sinAngle1 % dir1, -0.9999999f, 0.9999999f));
	//  if ((matrix0.m_front % matrix1.m_front) < dgFloat32 (0.0f)) {
	//      m_angle1 = (m_angle1 >= dgFloat32 (0.0f)) ? dgPI - m_angle1 : dgPI + m_angle1;
	//  }

	sinAngle = (matrix0.m_front * matrix1.m_front) % matrix1.m_up;
	cosAngle = matrix0.m_front % matrix1.m_front;
	//  NEWTON_ASSERT (dgAbsf (m_angle1 - dgAtan2 (sinAngle, cosAngle)) < 1.0e-1f);
	m_angle1 = dgAtan2(sinAngle, cosAngle);

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

			//          CalculatePointDerivative (ret, params, dir0, pointDataP, &m_jointForce[ret]);
			CalculateAngularDerivative(ret, params, dir0, m_stiffness,
			                           dgFloat32(0.0f), &m_jointForce[ret]);
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
			CalculateAngularDerivative(ret, params, dir1, m_stiffness,
			                           dgFloat32(0.0f), &m_jointForce[ret]);
			// params.m_jointAccel[ret] = axisParam[1].m_accel;
			SetMotorAcceleration(ret, axisParam[1].m_accel, params);
			ret++;
		}
	}
	return dgUnsigned32(ret);
}
