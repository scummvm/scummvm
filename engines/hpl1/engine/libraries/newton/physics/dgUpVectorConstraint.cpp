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

#include "dgUpVectorConstraint.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgUpVectorConstraint::dgUpVectorConstraint() : dgBilateralConstraint() {
	NEWTON_ASSERT((sizeof(dgUpVectorConstraint) & 15) == 0);
	NEWTON_ASSERT((((dgUnsigned64)&m_localMatrix0) & 15) == 0);

	//  dgUpVectorConstraintArray& array = * world;
	//  constraint = array.GetElement();

	SetStiffness(dgFloat32(0.995f));
	m_maxDOF = 2;
	m_constId = dgUpVectorConstraintId;
	m_callBack = NULL;
}

dgUpVectorConstraint::~dgUpVectorConstraint() {
}

/*
 dgUpVectorConstraint* dgUpVectorConstraint::Create(dgWorld* world)
 {
 dgUpVectorConstraint* constraint;
 // constraint = dgUpVectorConstraintArray::GetPool().GetElement();

 dgUpVectorConstraintArray& array = * world;
 constraint = array.GetElement();

 NEWTON_ASSERT ((((dgUnsigned64) &constraint->m_localMatrix0) & 15) == 0);

 constraint->Init ();


 constraint->SetStiffness (dgFloat32 (0.995f));
 constraint->m_maxDOF = 2;
 constraint->m_constId = dgUpVectorConstraintId;
 constraint->m_callBack = NULL;
 return constraint;
 }

 void dgUpVectorConstraint::Remove(dgWorld* world)
 {
 dgUpVectorConstraintArray& array = * world;
 dgBilateralConstraint::Remove (world);
 // dgUpVectorConstraintArray::GetPool().RemoveElement (this);
 array.RemoveElement (this);
 }
 */

bool dgUpVectorConstraint::IsBilateral() const {
	return false;
}

void dgUpVectorConstraint::InitPinDir(const dgVector &pin) {

	const dgMatrix &matrix = m_body0->GetMatrix();

	dgVector pivot(matrix.m_posit);
	SetPivotAndPinDir(pivot, pin);
}

void dgUpVectorConstraint::SetPinDir(const dgVector &pin) {
	m_localMatrix1 = dgMatrix(pin);
}

dgVector dgUpVectorConstraint::GetPinDir() const {
	return m_localMatrix1.m_front;
}

void dgUpVectorConstraint::SetJointParameterCallBack(
    dgUpVectorJointCallBack callback) {
	m_callBack = callback;
}

dgUnsigned32 dgUpVectorConstraint::JacobianDerivative(
    dgContraintDescritor &params) {
	dgInt32 ret;
	dgFloat32 mag;
	dgFloat32 angle;
	dgMatrix matrix0;
	dgMatrix matrix1;

	CalculateGlobalMatrixAndAngle(matrix0, matrix1);

	dgVector lateralDir(matrix0.m_front * matrix1.m_front);

	ret = 0;
	mag = lateralDir % lateralDir;
	if (mag > 1.0e-6f) {
		mag = dgSqrt(mag);
		lateralDir = lateralDir.Scale(dgFloat32(1.0f) / mag);
		angle = dgAsin(mag);
		CalculateAngularDerivative(0, params, lateralDir, m_stiffness, angle,
		                           &m_jointForce[0]);

		dgVector frontDir(lateralDir * matrix1.m_front);
		CalculateAngularDerivative(1, params, frontDir, m_stiffness,
		                           dgFloat32(0.0f), &m_jointForce[1]);
		ret = 2;
	} else {
		CalculateAngularDerivative(0, params, matrix0.m_up, m_stiffness, 0.0,
		                           &m_jointForce[0]);
		CalculateAngularDerivative(1, params, matrix0.m_right, m_stiffness,
		                           dgFloat32(0.0f), &m_jointForce[1]);
		ret = 2;
	}
	return dgUnsigned32(ret);
}
