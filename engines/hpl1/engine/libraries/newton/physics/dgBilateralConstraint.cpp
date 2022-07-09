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

#include "dgBody.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#include "dgBilateralConstraint.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DG_JOINT_STIFFNESS_RANGE (dgFloat32(5.0f))

#define DG_VEL_DAMP (dgFloat32(100.0f))
#define DG_POS_DAMP (dgFloat32(1500.0f))

dgBilateralConstraint::dgBilateralConstraint() : dgConstraint() {
	_ASSERTE((sizeof(dgBilateralConstraint) & 15) == 0);
	_ASSERTE((((dgUnsigned64)&m_localMatrix0) & 15) == 0);

	//	dgConstraint::Init ();

	m_maxDOF = 6;
	m_destructor = NULL;
	m_localMatrix0 = dgGetIdentityMatrix();
	m_localMatrix1 = dgGetIdentityMatrix();

	// SetStiffness (90.0f/99.0f);
	SetStiffness(dgFloat32(0.9f));

	memset(m_jointForce, 0, sizeof(m_jointForce));
	memset(m_rowIsMotor, 0, sizeof(m_rowIsMotor));
	memset(m_motorAcceleration, 0, sizeof(m_motorAcceleration));
}

dgBilateralConstraint::~dgBilateralConstraint() {
	if (m_destructor) {
		m_destructor(*this);
	}
}

bool dgBilateralConstraint::IsBilateral() const {
	return true;
}

dgFloat32 dgBilateralConstraint::GetStiffness() const {
	return (DG_JOINT_STIFFNESS_RANGE - m_stiffness) / (DG_JOINT_STIFFNESS_RANGE - dgFloat32(1.0f));
}

void dgBilateralConstraint::SetStiffness(dgFloat32 stiffness) {
	stiffness = ClampValue(stiffness, dgFloat32(0.0f), dgFloat32(1.0f));
	m_stiffness = DG_JOINT_STIFFNESS_RANGE - stiffness * (DG_JOINT_STIFFNESS_RANGE - dgFloat32(1.0f));
}

void dgBilateralConstraint::SetDestructorCallback(
	OnConstraintDestroy destructor) {
	m_destructor = destructor;
}

void dgBilateralConstraint::CalculateMatrixOffset(const dgVector &pivot,
												  const dgVector &dir, dgMatrix &matrix0, dgMatrix &matrix1) {
	dgFloat32 length;
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);

	const dgMatrix &body0_Matrix = m_body0->GetMatrix();

	length = dir % dir;
	length = dgSqrt(length);
	_ASSERTE(length > dgFloat32(0.0f));
	//	matrix0.m_front = body0_Matrix.UnrotateVector (dir.Scale (dgFloat32 (1.0f) / length));
	//	Create__Basis (matrix0.m_front, matrix0.m_up, matrix0.m_right);
	matrix0 = dgMatrix(
		body0_Matrix.UnrotateVector(dir.Scale(dgFloat32(1.0f) / length)));
	matrix0.m_posit = body0_Matrix.UntransformVector(pivot);

	matrix0.m_front.m_w = dgFloat32(0.0f);
	matrix0.m_up.m_w = dgFloat32(0.0f);
	matrix0.m_right.m_w = dgFloat32(0.0f);
	matrix0.m_posit.m_w = dgFloat32(1.0f);

	//	dgMatrix body1_Matrix (dgGetIdentityMatrix());
	//	if (m_body1) {
	//		body1_Matrix = m_body1->GetMatrix();
	//	}
	const dgMatrix &body1_Matrix = m_body1->GetMatrix();

	matrix1 = matrix0 * body0_Matrix * body1_Matrix.Inverse();
}

void dgBilateralConstraint::SetPivotAndPinDir(const dgVector &pivot,
											  const dgVector &pinDirection) {
	CalculateMatrixOffset(pivot, pinDirection, m_localMatrix0, m_localMatrix1);
}

void dgBilateralConstraint::SetPivotAndPinDir(const dgVector &pivot,
											  const dgVector &pinDirection0, const dgVector &pinDirection1) {
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);

	const dgMatrix &body0_Matrix = m_body0->GetMatrix();

	_ASSERTE((pinDirection0 % pinDirection0) > dgFloat32(0.0f));
	m_localMatrix0.m_front = pinDirection0.Scale(
		dgFloat32(1.0f) / dgSqrt(pinDirection0 % pinDirection0));
	m_localMatrix0.m_right = m_localMatrix0.m_front * pinDirection1;
	m_localMatrix0.m_right =
		m_localMatrix0.m_right.Scale(
			dgFloat32(
				1.0f) /
			dgSqrt(m_localMatrix0.m_right % m_localMatrix0.m_right));
	m_localMatrix0.m_up = m_localMatrix0.m_right * m_localMatrix0.m_front;
	m_localMatrix0.m_posit = pivot;

	m_localMatrix0.m_front.m_w = dgFloat32(0.0f);
	m_localMatrix0.m_up.m_w = dgFloat32(0.0f);
	m_localMatrix0.m_right.m_w = dgFloat32(0.0f);
	m_localMatrix0.m_posit.m_w = dgFloat32(1.0f);

	//	dgMatrix body1_Matrix (dgGetIdentityMatrix());
	//	if (m_body1) {
	//		body1_Matrix = m_body1->GetMatrix();
	//	}
	const dgMatrix &body1_Matrix = m_body1->GetMatrix();

	m_localMatrix1 = m_localMatrix0 * body1_Matrix.Inverse();
	m_localMatrix0 = m_localMatrix0 * body0_Matrix.Inverse();
}

dgVector dgBilateralConstraint::CalculateGlobalMatrixAndAngle(
	dgMatrix &globalMatrix0, dgMatrix &globalMatrix1) const {
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);
	const dgMatrix &body0Matrix = m_body0->GetMatrix();
	const dgMatrix &body1Matrix = m_body1->GetMatrix();
	//	dgMatrix body1Matrix (dgGetIdentityMatrix());
	//	if (m_body1) {
	//		body1Matrix = m_body1->GetMatrix();
	//	}

	globalMatrix0 = m_localMatrix0 * body0Matrix;
	globalMatrix1 = m_localMatrix1 * body1Matrix;

	dgMatrix relMatrix(globalMatrix1 * globalMatrix0.Inverse());

	_ASSERTE(
		dgAbsf(dgFloat32(1.0f) - (relMatrix.m_front % relMatrix.m_front)) < 1.0e-5f);
	_ASSERTE(
		dgAbsf(dgFloat32(1.0f) - (relMatrix.m_up % relMatrix.m_up)) < 1.0e-5f);
	_ASSERTE(
		dgAbsf(dgFloat32(1.0f) - (relMatrix.m_right % relMatrix.m_right)) < 1.0e-5f);
	//	_ASSERTE ((relMatrix.m_posit % relMatrix.m_posit) < 1.0e-3f);

	return relMatrix.CalcPitchYawRoll();
}

void dgBilateralConstraint::SetMotorAcceleration(dgInt32 index,
												 dgFloat32 acceleration, dgContraintDescritor &desc) {
	m_rowIsMotor[index] = -1;
	m_motorAcceleration[index] = acceleration;
	desc.m_isMotor[index] = 1;
	desc.m_jointAccel[index] = acceleration;
}

void dgBilateralConstraint::SetJacobianDerivative(dgInt32 index,
												  dgContraintDescritor &desc, const dgFloat32 *jacobianA,
												  const dgFloat32 *jacobianB, dgFloat32 *jointForce) {
	dgJacobian &jacobian0 = desc.m_jacobian[index].m_jacobian_IM0;
	dgJacobian &jacobian1 = desc.m_jacobian[index].m_jacobian_IM1;

	jacobian0.m_linear[0] = jacobianA[0];
	jacobian0.m_linear[1] = jacobianA[1];
	jacobian0.m_linear[2] = jacobianA[2];
	jacobian0.m_linear[3] = dgFloat32(0.0f);
	jacobian0.m_angular[0] = jacobianA[3];
	jacobian0.m_angular[1] = jacobianA[4];
	jacobian0.m_angular[2] = jacobianA[5];
	jacobian0.m_angular[3] = dgFloat32(0.0f);

	jacobian1.m_linear[0] = jacobianB[0];
	jacobian1.m_linear[1] = jacobianB[1];
	jacobian1.m_linear[2] = jacobianB[2];
	jacobian1.m_linear[3] = dgFloat32(0.0f);
	jacobian1.m_angular[0] = jacobianB[3];
	jacobian1.m_angular[1] = jacobianB[4];
	jacobian1.m_angular[2] = jacobianB[5];
	jacobian1.m_angular[3] = dgFloat32(0.0f);

	m_rowIsMotor[index] = -1;
	m_motorAcceleration[index] = dgFloat32(0.0f);

	desc.m_restitution[index] = dgFloat32(0.0f);
	desc.m_jointAccel[index] = dgFloat32(0.0f);
	desc.m_penetration[index] = dgFloat32(0.0f);
	desc.m_penetrationStiffness[index] = dgFloat32(0.0f);
	desc.m_jointStiffness[index] = dgFloat32(1.0f);
	desc.m_forceBounds[index].m_jointForce = jointForce;
}

dgFloat32 dgBilateralConstraint::CalculateSpringDamperAcceleration(
	dgInt32 index, const dgContraintDescritor &desc, dgFloat32 jointAngle,
	const dgVector &p0Global, const dgVector &p1Global, dgFloat32 springK,
	dgFloat32 springD) {
	dgFloat32 relPosit;
	dgFloat32 relVeloc;

	const dgJacobian &jacobian0 = desc.m_jacobian[index].m_jacobian_IM0;
	const dgJacobian &jacobian1 = desc.m_jacobian[index].m_jacobian_IM1;

	dgVector veloc0(m_body0->m_veloc);
	dgVector omega0(m_body0->m_omega);

	dgVector veloc1(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					dgFloat32(0.0f));
	dgVector omega1(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					dgFloat32(0.0f));
	;
	if (m_body1) {
		veloc1 = m_body1->m_veloc;
		omega1 = m_body1->m_omega;
	}
	relPosit = (p1Global - p0Global) % jacobian0.m_linear + jointAngle;
	relVeloc = -(veloc0 % jacobian0.m_linear + veloc1 % jacobian1.m_linear + omega0 % jacobian0.m_angular + omega1 % jacobian1.m_angular);

	// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
	dgFloat32 dt = desc.m_timestep;
	dgFloat32 ks = springK;
	dgFloat32 kd = springD;
	dgFloat32 ksd = dt * ks;
	dgFloat32 num = ks * relPosit + kd * relVeloc + ksd * relVeloc;
	dgFloat32 den = dgFloat32(1.0f) + dt * kd + dt * ksd;

	return num / den;
}

void dgBilateralConstraint::CalculateAngularDerivative(dgInt32 index,
													   dgContraintDescritor &desc, const dgVector &dir, dgFloat32 stiffness,
													   dgFloat32 jointAngle, dgFloat32 *jointForce) {
	dgFloat32 alphaError;
	dgFloat32 omegaError;

	_ASSERTE(jointForce);

	dgVector omega1;
	_ASSERTE(m_body0);
	dgVector omega0(m_body0->GetOmega());
	dgJacobian &jacobian0 = desc.m_jacobian[index].m_jacobian_IM0;
	jacobian0.m_linear[0] = dgFloat32(0.0f);
	jacobian0.m_linear[1] = dgFloat32(0.0f);
	jacobian0.m_linear[2] = dgFloat32(0.0f);
	jacobian0.m_linear[3] = dgFloat32(0.0f);
	jacobian0.m_angular[0] = dir.m_x;
	jacobian0.m_angular[1] = dir.m_y;
	jacobian0.m_angular[2] = dir.m_z;
	jacobian0.m_angular[3] = dgFloat32(0.0f);

	dgJacobian &jacobian1 = desc.m_jacobian[index].m_jacobian_IM1;
	_ASSERTE(m_body1);
	omega1 = m_body1->GetOmega();
	jacobian1.m_linear[0] = dgFloat32(0.0f);
	jacobian1.m_linear[1] = dgFloat32(0.0f);
	jacobian1.m_linear[2] = dgFloat32(0.0f);
	jacobian1.m_linear[3] = dgFloat32(0.0f);
	jacobian1.m_angular[0] = -dir.m_x;
	jacobian1.m_angular[1] = -dir.m_y;
	jacobian1.m_angular[2] = -dir.m_z;
	jacobian1.m_angular[3] = dgFloat32(0.0f);

	omegaError = (omega1 - omega0) % dir;

	// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
	dgFloat32 dt = desc.m_timestep;
	dgFloat32 ks = DG_POS_DAMP;
	dgFloat32 kd = DG_VEL_DAMP;
	dgFloat32 ksd = dt * ks;
	dgFloat32 num = ks * jointAngle + kd * omegaError + ksd * omegaError;
	dgFloat32 den = dgFloat32(1.0f) + dt * kd + dt * ksd;
	alphaError = num / den;

	m_rowIsMotor[index] = 0;
	desc.m_isMotor[index] = 0;
	m_motorAcceleration[index] = dgFloat32(0.0f);

	//_ASSERTE (dgAbsf (alphaError - CalculateSpringDamperAcceleration (index, desc, jointAngle,  dgVector (0, 0, 0), dgVector (0, 0, 0),	ANGULAR_POS_DAMP, ANGULAR_VEL_DAMP)) < 1.0e-2f);

	desc.m_penetration[index] = jointAngle;
	desc.m_jointAccel[index] = alphaError;
	desc.m_restitution[index] = dgFloat32(0.0f);
	desc.m_jointStiffness[index] = stiffness;
	desc.m_penetrationStiffness[index] = dgFloat32(0.0f);
	desc.m_forceBounds[index].m_jointForce = jointForce;
}

void dgBilateralConstraint::CalculatePointDerivative(dgInt32 index,
													 dgContraintDescritor &desc, const dgVector &dir, const dgPointParam &param,
													 dgFloat32 *jointForce) {
	dgFloat32 relPosit;
	dgFloat32 relVeloc;
	dgFloat32 relCentr;
	dgFloat32 accelError;

	_ASSERTE(jointForce);
	_ASSERTE(m_body0);
	_ASSERTE(m_body1);

	dgJacobian &jacobian0 = desc.m_jacobian[index].m_jacobian_IM0;
	dgVector r0CrossDir(param.m_r0 * dir);
	jacobian0.m_linear[0] = dir.m_x;
	jacobian0.m_linear[1] = dir.m_y;
	jacobian0.m_linear[2] = dir.m_z;
	jacobian0.m_linear[3] = dgFloat32(0.0f);
	jacobian0.m_angular[0] = r0CrossDir.m_x;
	jacobian0.m_angular[1] = r0CrossDir.m_y;
	jacobian0.m_angular[2] = r0CrossDir.m_z;
	jacobian0.m_angular[3] = dgFloat32(0.0f);

	dgJacobian &jacobian1 = desc.m_jacobian[index].m_jacobian_IM1;
	dgVector r1CrossDir(dir * param.m_r1);
	jacobian1.m_linear[0] = -dir.m_x;
	jacobian1.m_linear[1] = -dir.m_y;
	jacobian1.m_linear[2] = -dir.m_z;
	jacobian1.m_linear[3] = dgFloat32(0.0f);
	jacobian1.m_angular[0] = r1CrossDir.m_x;
	jacobian1.m_angular[1] = r1CrossDir.m_y;
	jacobian1.m_angular[2] = r1CrossDir.m_z;
	jacobian1.m_angular[3] = dgFloat32(0.0f);

	dgVector velocError(param.m_veloc1 - param.m_veloc0);
	dgVector positError(param.m_posit1 - param.m_posit0);
	dgVector centrError(param.m_centripetal1 - param.m_centripetal0);

	relPosit = positError % dir;
	relVeloc = velocError % dir;
	relCentr = centrError % dir;
	relCentr = ClampValue(relCentr, dgFloat32(-10000.0f), dgFloat32(10000.0f));
	// relCentr = 0.0f;

	// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
	dgFloat32 dt = desc.m_timestep;
	dgFloat32 ks = DG_POS_DAMP;
	dgFloat32 kd = DG_VEL_DAMP;
	dgFloat32 ksd = dt * ks;
	dgFloat32 num = ks * relPosit + kd * relVeloc + ksd * relVeloc;
	dgFloat32 den = dgFloat32(1.0f) + dt * kd + dt * ksd;
	accelError = num / den;

	//_ASSERTE (dgAbsf (accelError - CalculateSpringDamperAcceleration (index, desc, 0.0f, param.m_posit0, param.m_posit1,	LINEAR_POS_DAMP, LINEAR_VEL_DAMP)) < 1.0e-2f);

	m_rowIsMotor[index] = 0;
	desc.m_isMotor[index] = 0;
	m_motorAcceleration[index] = dgFloat32(0.0f);

	//	dgJacobianPair m_jacobian[DG_CONSTRAINT_MAX_ROWS];
	//	dgBilateralBounds m_forceBounds[DG_CONSTRAINT_MAX_ROWS];
	//	dgFloat32 m_jointAccel[DG_CONSTRAINT_MAX_ROWS];
	//	dgFloat32 m_jointStiffness[DG_CONSTRAINT_MAX_ROWS];
	//	dgFloat32 m_restitution[DG_CONSTRAINT_MAX_ROWS];
	//	dgFloat32 m_penetration[DG_CONSTRAINT_MAX_ROWS];
	//	dgFloat32 m_penetrationStiffness[DG_CONSTRAINT_MAX_ROWS];

	desc.m_penetration[index] = relPosit;
	desc.m_penetrationStiffness[index] = dgFloat32(0.01f / 4.0f);
	desc.m_jointStiffness[index] = param.m_stiffness;
	desc.m_jointAccel[index] = accelError + relCentr;
	// save centripetal acceleration in the restitution member
	desc.m_restitution[index] = relCentr;
	desc.m_forceBounds[index].m_jointForce = jointForce;
}

void dgBilateralConstraint::JointAccelerationsSimd(
	const dgJointAccelerationDecriptor &params) {
#ifdef DG_BUILD_SIMD_CODE
	dgFloat32 dt;

	const dgJacobianPair *const Jt = params.m_Jt;
	const dgVector &bodyVeloc0 = m_body0->m_veloc;
	const dgVector &bodyOmega0 = m_body0->m_omega;
	const dgVector &bodyVeloc1 = m_body1->m_veloc;
	const dgVector &bodyOmega1 = m_body1->m_omega;

#if 1
	dgFloat32 kd = DG_VEL_DAMP * dgFloat32(4.0f);
	dgFloat32 ks = DG_POS_DAMP * dgFloat32(0.25f);

	dt = params.m_timeStep;
	for (dgInt32 k = 0; k < params.m_rowsCount; k++) {
		if (m_rowIsMotor[k]) {
			params.m_coordenateAccel[k] = m_motorAcceleration[k] + params.m_externAccelaration[k];
		} else {
			dgFloat32 num;
			dgFloat32 den;
			dgFloat32 ksd;
			dgFloat32 vRel;
			dgFloat32 aRel;
			dgFloat32 aRelErr;
			dgFloat32 relPosit;

			dgVector relVeloc(Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
			relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
			relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
			relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);

			vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;
			aRel = params.m_externAccelaration[k];
			// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
			//		alphaError = num / den;

			// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
			//		dgFloat32 dt = desc.m_timestep;
			//		dgFloat32 ks = DG_POS_DAMP;
			//		dgFloat32 kd = DG_VEL_DAMP;
			//		dgFloat32 ksd = dt * ks;
			//		dgFloat32 num = ks * relPosit + kd * relVeloc + ksd * relVeloc;
			//		dgFloat32 den = dgFloat32 (1.0f) + dt * kd + dt * ksd;
			//		accelError = num / den;

			ksd = dt * ks;
			//			relPosit = params.m_penetration[k];
			relPosit = params.m_penetration[k] - vRel * dt * params.m_firstPassCoefFlag;

			//			if (relPosit > dgFloat32 (1.0f) ) {
			//				relPosit = dgFloat32 (1.0f);
			//			} else if (params.m_penetration[k] < dgFloat32 (-1.0f) ) {
			//				relPosit = dgFloat32 (-1.0f);
			//			}
			params.m_penetration[k] = relPosit;

			num = ks * relPosit - kd * vRel - ksd * vRel;
			den = dgFloat32(1.0f) + dt * kd + dt * ksd;
			aRelErr = num / den;

			// centripetal acceleration is stored restitution member
			params.m_coordenateAccel[k] = aRelErr + params.m_restitution[k] + aRel;

#else
	dgFloat32 vRel;
	dgFloat32 aRel;
	dgFloat32 penetrationVeloc;
	dgFloat32 penetrationCorrection;

	dgVector relVeloc(Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
	relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
	relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
	relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);

	vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;

	penetrationCorrection = vRel * params.m_timeStep * params.m_firstPassCoefFlag;
	params.m_penetration[k] = params.m_penetration[k] - penetrationCorrection;
	if (params.m_penetration[k] > dgFloat32(1.0f)) {
		params.m_penetration[k] = dgFloat32(1.0f);
	} else if (params.m_penetration[k] < dgFloat32(-1.0f)) {
		params.m_penetration[k] = dgFloat32(-1.0f);
	}
	penetrationVeloc = -(params.m_penetration[k] * params.m_penetrationStiffness[k] * params.m_invTimeStep);

	vRel += penetrationVeloc;
	// centripetal acceleration is stored restitution member
	aRel = params.m_externAccelaration[k] + params.m_restitution[k];
	params.m_coordenateAccel[k] = (aRel - vRel * params.m_invTimeStep);
#endif
		}
	}
#endif
}

void dgBilateralConstraint::JointAccelerations(
	const dgJointAccelerationDecriptor &params) {
	dgFloat32 dt;

	const dgJacobianPair *const Jt = params.m_Jt;
	const dgVector &bodyVeloc0 = m_body0->m_veloc;
	const dgVector &bodyOmega0 = m_body0->m_omega;
	const dgVector &bodyVeloc1 = m_body1->m_veloc;
	const dgVector &bodyOmega1 = m_body1->m_omega;

#if 1
	dgFloat32 kd = DG_VEL_DAMP * dgFloat32(4.0f);
	dgFloat32 ks = DG_POS_DAMP * dgFloat32(0.25f);

	dt = params.m_timeStep;
	for (dgInt32 k = 0; k < params.m_rowsCount; k++) {
		if (m_rowIsMotor[k]) {
			params.m_coordenateAccel[k] = m_motorAcceleration[k] + params.m_externAccelaration[k];
		} else {
			dgFloat32 num;
			dgFloat32 den;
			dgFloat32 ksd;
			dgFloat32 vRel;
			dgFloat32 aRel;
			dgFloat32 aRelErr;
			dgFloat32 relPosit;

			dgVector relVeloc(Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
			relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
			relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
			relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);

			vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;
			aRel = params.m_externAccelaration[k];
			// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
			//		alphaError = num / den;

			// at =  [- ks (x2 - x1) - kd * (v2 - v1) - dt * ks * (v2 - v1)] / [1 + dt * kd + dt * dt * ks]
			//		dgFloat32 dt = desc.m_timestep;
			//		dgFloat32 ks = DG_POS_DAMP;
			//		dgFloat32 kd = DG_VEL_DAMP;
			//		dgFloat32 ksd = dt * ks;
			//		dgFloat32 num = ks * relPosit + kd * relVeloc + ksd * relVeloc;
			//		dgFloat32 den = dgFloat32 (1.0f) + dt * kd + dt * ksd;
			//		accelError = num / den;

			ksd = dt * ks;
			//			relPosit = params.m_penetration[k];
			relPosit = params.m_penetration[k] - vRel * dt * params.m_firstPassCoefFlag;

			//			if (relPosit > dgFloat32 (1.0f) ) {
			//				relPosit = dgFloat32 (1.0f);
			//			} else if (params.m_penetration[k] < dgFloat32 (-1.0f) ) {
			//				relPosit = dgFloat32 (-1.0f);
			//			}
			params.m_penetration[k] = relPosit;

			num = ks * relPosit - kd * vRel - ksd * vRel;
			den = dgFloat32(1.0f) + dt * kd + dt * ksd;
			aRelErr = num / den;

			// centripetal acceleration is stored restitution member
			params.m_coordenateAccel[k] = aRelErr + params.m_restitution[k] + aRel;

#else
	dgFloat32 vRel;
	dgFloat32 aRel;
	dgFloat32 penetrationVeloc;
	dgFloat32 penetrationCorrection;

	dgVector relVeloc(Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
	relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
	relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
	relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);

	vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;

	penetrationCorrection = vRel * params.m_timeStep * params.m_firstPassCoefFlag;
	params.m_penetration[k] = params.m_penetration[k] - penetrationCorrection;
	if (params.m_penetration[k] > dgFloat32(1.0f)) {
		params.m_penetration[k] = dgFloat32(1.0f);
	} else if (params.m_penetration[k] < dgFloat32(-1.0f)) {
		params.m_penetration[k] = dgFloat32(-1.0f);
	}
	penetrationVeloc = -(params.m_penetration[k] * params.m_penetrationStiffness[k] * params.m_invTimeStep);

	vRel += penetrationVeloc;
	// centripetal acceleration is stored restitution member
	aRel = params.m_externAccelaration[k] + params.m_restitution[k];
	params.m_coordenateAccel[k] = (aRel - vRel * params.m_invTimeStep);
#endif
		}
	}
}

void dgBilateralConstraint::JointVelocityCorrection(
	const dgJointAccelerationDecriptor &params) {
	/*
	 const dgJacobianPair* const Jt = params.m_Jt;
	 const dgVector& bodyVeloc0 = params.m_body0->m_correctionVeloc;
	 const dgVector& bodyOmega0 = params.m_body0->m_correctionOmega;
	 const dgVector& bodyVeloc1 = params.m_body1->m_correctionVeloc;
	 const dgVector& bodyOmega1 = params.m_body1->m_correctionOmega;

	 for (dgInt32 k = 0; k < params.m_rowsCount; k ++) {
	 dgFloat32 vRel;
	 dgFloat32 penetrationCorrection;

	 dgVector relVeloc (Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
	 relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
	 relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
	 relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);
	 vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;

	 penetrationCorrection = vRel * params.m_timeStep;
	 params.m_penetration[k] -= penetrationCorrection;
	 params.m_coordenateAccel[k] =  - vRel + params.m_penetration[k] * params.m_invTimeStep * 0.25f;
	 }
	 */
}
