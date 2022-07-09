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

#include "dgBallConstraint.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
dgBallConstraint::dgBallConstraint() : dgBilateralConstraint() {

	//	dgBallConstraint* constraint;

	//	dgBallConstraintArray& array = * world;
	//	constraint = array.GetElement();
	_ASSERTE((((dgUnsigned64)&m_localMatrix0) & 15) == 0);

	// constraint->SetStiffness (dgFloat32 (0.5f));
	m_maxDOF = 6;
	m_jointUserCallback = NULL;
	m_constId = dgBallConstraintId;
	limits.m_ballLimits = 0;
	m_angles = dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						dgFloat32(0.0f));
}

dgBallConstraint::~dgBallConstraint() {
}

/*
 dgBallConstraint* dgBallConstraint::Create(dgWorld* world)
 {
 dgBallConstraint* constraint;


 dgBallConstraintArray& array = * world;
 constraint = array.GetElement();
 _ASSERTE ((((dgUnsigned64) &constraint->m_localMatrix0) & 15) == 0);

 constraint->Init();

 //	constraint->SetStiffness (dgFloat32 (0.5f));
 constraint->m_maxDOF = 6;
 constraint->m_jointUserCallback = NULL;
 constraint->m_constId = dgBallConstraintId;
 constraint->m_ballLimits = 0;
 constraint->m_angles = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
 return constraint;
 }


 void dgBallConstraint::Remove(dgWorld* world)
 {
 dgBallConstraintArray& array = *world;
 dgBilateralConstraint::Remove (world);
 array.RemoveElement (this);
 }
 */

void dgBallConstraint::SetJointParameterCallBack(dgBallJointFriction callback) {
	m_jointUserCallback = callback;
}

dgVector dgBallConstraint::GetJointAngle() const {
	return m_angles;
}

dgVector dgBallConstraint::GetJointOmega() const {
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);
	const dgMatrix &matrix = m_body0->GetMatrix();

	dgVector dir0(matrix.RotateVector(m_localMatrix0[0]));
	dgVector dir1(matrix.RotateVector(m_localMatrix0[1]));
	dgVector dir2(matrix.RotateVector(m_localMatrix0[2]));

	const dgVector &omega0 = m_body0->GetOmega();
	const dgVector &omega1 = m_body1->GetOmega();

	//	dgVector omega1 (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	//	if (m_body1) {
	//		omega1 = m_body1->GetOmega();
	//	}

	dgVector relOmega(omega0 - omega1);
	return dgVector(relOmega % dir0, relOmega % dir1, relOmega % dir2,
					dgFloat32(0.0f));
}

dgVector dgBallConstraint::GetJointForce() const {
	dgMatrix matrix0;
	dgMatrix matrix1;

	CalculateGlobalMatrixAndAngle(matrix0, matrix1);
	return dgVector(
		matrix0.m_front.Scale(m_jointForce[0]) + matrix0.m_up.Scale(m_jointForce[1]) + matrix0.m_right.Scale(m_jointForce[2]));
}

bool dgBallConstraint::GetTwistLimitState() const {
	return limits.s.m_twistLimit;
}

bool dgBallConstraint::GetConeLimitState() const {
	return limits.s.m_coneLimit;
}

bool dgBallConstraint::GetLatealLimitState() const {
	return limits.s.m_lateralLimit;
}

void dgBallConstraint::SetTwistLimitState(bool state) {
	limits.s.m_twistLimit = dgUnsigned32(state);
}

void dgBallConstraint::SetConeLimitState(bool state) {
	limits.s.m_coneLimit = dgUnsigned32(state);
}

void dgBallConstraint::SetLatealLimitState(bool state) {
	limits.s.m_lateralLimit = dgUnsigned32(state);
}

void dgBallConstraint::SetPivotPoint(const dgVector &pivot) {
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);
	const dgMatrix &matrix = m_body0->GetMatrix();

	dgVector pin(pivot - matrix.m_posit);
	if ((pin % pin) < dgFloat32(1.0e-3f)) {
		pin = matrix.m_front;
	}

	SetPivotAndPinDir(pivot, pin);

	dgMatrix matrix0;
	dgMatrix matrix1;
	CalculateGlobalMatrixAndAngle(matrix0, matrix1);
	SetLimits(matrix0.m_front, -dgPI * dgFloat32(0.5f), dgPI * dgFloat32(0.5f),
			  dgPI * dgFloat32(0.5f), matrix0.m_right, dgFloat32(0.0f),
			  dgFloat32(0.0f));
}

void dgBallConstraint::SetLimits(const dgVector &coneDir,
								 dgFloat32 minConeAngle, dgFloat32 maxConeAngle, dgFloat32 maxTwistAngle,
								 const dgVector &bilateralDir, dgFloat32 negativeBilateralConeAngle__,
								 dgFloat32 positiveBilateralConeAngle__) {
	dgMatrix matrix0;
	dgMatrix matrix1;
	CalculateGlobalMatrixAndAngle(matrix0, matrix1);

	_ASSERTE(m_body0);
	_ASSERTE(m_body1);
	const dgMatrix &body0_Matrix = m_body0->GetMatrix();

	dgVector lateralDir(bilateralDir * coneDir);
	if ((lateralDir % lateralDir) < dgFloat32(1.0e-3f)) {
		dgMatrix tmp(coneDir);
		lateralDir = tmp.m_up;
	}

	m_localMatrix0.m_front = body0_Matrix.UnrotateVector(coneDir);
	m_localMatrix0.m_up = body0_Matrix.UnrotateVector(lateralDir);
	m_localMatrix0.m_posit = body0_Matrix.UntransformVector(matrix1.m_posit);

	m_localMatrix0.m_front =
		m_localMatrix0.m_front.Scale(
			dgFloat32(
				1.0f) /
			dgSqrt(m_localMatrix0.m_front % m_localMatrix0.m_front));
	m_localMatrix0.m_up = m_localMatrix0.m_up.Scale(
		dgFloat32(1.0f) / dgSqrt(m_localMatrix0.m_up % m_localMatrix0.m_up));
	m_localMatrix0.m_right = m_localMatrix0.m_front * m_localMatrix0.m_up;

	m_localMatrix0.m_front.m_w = dgFloat32(0.0f);
	m_localMatrix0.m_up.m_w = dgFloat32(0.0f);
	m_localMatrix0.m_right.m_w = dgFloat32(0.0f);
	m_localMatrix0.m_posit.m_w = dgFloat32(1.0f);

	//	dgMatrix body1_Matrix (dgGetIdentityMatrix());
	//	if (m_body1) {
	//		body1_Matrix = m_body1->GetMatrix();
	//	}
	const dgMatrix &body1_Matrix = m_body1->GetMatrix();

	m_twistAngle = ClampValue(maxTwistAngle, dgFloat32(5.0f) * dgDEG2RAD,
							  dgFloat32(90.0f) * dgDEG2RAD);
	m_coneAngle = ClampValue((maxConeAngle - minConeAngle) * dgFloat32(0.5f),
							 dgFloat32(5.0f) * dgDEG2RAD, 175.0f * dgDEG2RAD);
	m_coneAngleCos = dgCos(m_coneAngle);

	dgMatrix coneMatrix(
		dgPitchMatrix((maxConeAngle + minConeAngle) * dgFloat32(0.5f)));

	m_localMatrix0 = coneMatrix * m_localMatrix0;

	m_localMatrix1 = m_localMatrix0 * body0_Matrix * body1_Matrix.Inverse();
}

dgUnsigned32 dgBallConstraint::JacobianDerivative(dgContraintDescritor &params) {
	dgInt32 ret;
	dgFloat32 relVelocErr;
	dgFloat32 penetrationErr;
	dgMatrix matrix0;
	dgMatrix matrix1;

	if (m_jointUserCallback) {
		m_jointUserCallback(*this, params.m_timestep);
	}

	dgVector angle(CalculateGlobalMatrixAndAngle(matrix0, matrix1));
	m_angles = angle.Scale(-dgFloat32(1.0f));

	const dgVector &dir0 = matrix0.m_front;
	const dgVector &dir1 = matrix0.m_up;
	const dgVector &dir2 = matrix0.m_right;
	const dgVector &p0 = matrix0.m_posit;
	const dgVector &p1 = matrix1.m_posit;

	dgPointParam pointData;
	InitPointParam(pointData, m_stiffness, p0, p1);
	CalculatePointDerivative(0, params, dir0, pointData, &m_jointForce[0]);
	CalculatePointDerivative(1, params, dir1, pointData, &m_jointForce[1]);
	CalculatePointDerivative(2, params, dir2, pointData, &m_jointForce[2]);
	ret = 3;

	if (limits.s.m_twistLimit) {
		if (angle.m_x > m_twistAngle) {
			dgVector p0(matrix0.m_posit + matrix0.m_up.Scale(MIN_JOINT_PIN_LENGTH));
			InitPointParam(pointData, m_stiffness, p0, p0);

			const dgVector &dir = matrix0.m_right;
			CalculatePointDerivative(ret, params, dir, pointData, &m_jointForce[ret]);

			dgVector velocError(pointData.m_veloc1 - pointData.m_veloc0);
			relVelocErr = velocError % dir;
			if (relVelocErr > dgFloat32(1.0e-3f)) {
				relVelocErr *= dgFloat32(1.1f);
			}

			penetrationErr = MIN_JOINT_PIN_LENGTH * (angle.m_x - m_twistAngle);
			_ASSERTE(penetrationErr >= dgFloat32(0.0f));

			params.m_forceBounds[ret].m_low = dgFloat32(0.0f);
			params.m_forceBounds[ret].m_normalIndex = DG_NORMAL_CONSTRAINT;
			params.m_forceBounds[ret].m_jointForce = &m_jointForce[ret];
			//			params.m_jointAccel[ret] = (relVelocErr + penetrationErr) * params.m_invTimestep;
			SetMotorAcceleration(ret,
								 (relVelocErr + penetrationErr) * params.m_invTimestep, params);
			ret++;
		} else if (angle.m_x < -m_twistAngle) {
			dgVector p0(matrix0.m_posit + matrix0.m_up.Scale(MIN_JOINT_PIN_LENGTH));
			InitPointParam(pointData, m_stiffness, p0, p0);
			dgVector dir(matrix0.m_right.Scale(-dgFloat32(1.0f)));
			CalculatePointDerivative(ret, params, dir, pointData, &m_jointForce[ret]);

			dgVector velocError(pointData.m_veloc1 - pointData.m_veloc0);
			relVelocErr = velocError % dir;
			if (relVelocErr > dgFloat32(1.0e-3f)) {
				relVelocErr *= dgFloat32(1.1f);
			}

			penetrationErr = MIN_JOINT_PIN_LENGTH * (-m_twistAngle - angle.m_x);
			_ASSERTE(penetrationErr >= dgFloat32(0.0f));

			params.m_forceBounds[ret].m_low = dgFloat32(0.0f);
			params.m_forceBounds[ret].m_normalIndex = DG_NORMAL_CONSTRAINT;
			params.m_forceBounds[ret].m_jointForce = &m_jointForce[ret];
			//			params.m_jointAccel[ret] = (relVelocErr + penetrationErr) * params.m_invTimestep;
			SetMotorAcceleration(ret,
								 (relVelocErr + penetrationErr) * params.m_invTimestep, params);
			ret++;
		}
	}

	if (limits.s.m_coneLimit) {

		dgFloat32 coneCos;
		coneCos = matrix0.m_front % matrix1.m_front;
		if (coneCos < m_coneAngleCos) {
			dgVector p0(
				matrix0.m_posit + matrix0.m_front.Scale(MIN_JOINT_PIN_LENGTH));
			InitPointParam(pointData, m_stiffness, p0, p0);

			dgVector tangentDir(matrix0.m_front * matrix1.m_front);
			tangentDir = tangentDir.Scale(
				dgRsqrt((tangentDir % tangentDir) + 1.0e-8f));
			CalculatePointDerivative(ret, params, tangentDir, pointData,
									 &m_jointForce[ret]);
			ret++;

			dgVector normalDir(tangentDir * matrix0.m_front);

			dgVector velocError(pointData.m_veloc1 - pointData.m_veloc0);
			// restitution = contact.m_restitution;
			relVelocErr = velocError % normalDir;
			if (relVelocErr > dgFloat32(1.0e-3f)) {
				relVelocErr *= dgFloat32(1.1f);
			}

			penetrationErr = MIN_JOINT_PIN_LENGTH * (dgAcos(GetMax(coneCos, dgFloat32(-0.9999f))) - m_coneAngle);
			_ASSERTE(penetrationErr >= dgFloat32(0.0f));

			CalculatePointDerivative(ret, params, normalDir, pointData,
									 &m_jointForce[ret]);
			params.m_forceBounds[ret].m_low = dgFloat32(0.0f);
			params.m_forceBounds[ret].m_normalIndex = DG_NORMAL_CONSTRAINT;
			params.m_forceBounds[ret].m_jointForce = &m_jointForce[ret];
			//			params.m_jointAccel[ret] = (relVelocErr + penetrationErr) * params.m_invTimestep;
			SetMotorAcceleration(ret,
								 (relVelocErr + penetrationErr) * params.m_invTimestep, params);
			ret++;
		}
	}

	return dgUnsigned32(ret);
}
