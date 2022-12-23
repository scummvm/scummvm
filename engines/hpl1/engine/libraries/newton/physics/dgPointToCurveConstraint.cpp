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

#include "dgPointToCurveConstraint.h"
#include "dgBody.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


/*


 //////////////////////////////////////////////////////////////////////
 // Construction/Destruction
 //////////////////////////////////////////////////////////////////////

 dgPointToCurveConstraint::dgPointToCurveConstraint()
 {

 }

 dgPointToCurveConstraint::~dgPointToCurveConstraint()
 {
 }


 dgPointToCurveConstraint* dgPointToCurveConstraint::Create(dgWorld* world)
 {
 dgPointToCurveConstraint*  constraint;

 constraint = dgPointToCurveConstraintArray::GetPool().GetElement();

 constraint->Init();
 constraint->m_maxDOF = 2;
 constraint->m_destructor = NULL;
 constraint->m_constId = dgPointToCurveConstraintId;
 return constraint;
 }

 void dgPointToCurveConstraint::Remove(dgWorld* world)
 {
 dgBallConstraintArray& array = *world;
 dgBilateralConstraint::Remove ();
 // dgBallConstraintArray::GetPool().RemoveElement (this);
 array.RemoveElement (this);

 }

 void dgPointToCurveConstraint::SetDestructorCallback (OnConstraintDestroy destructor)
 {
 m_destructor = destructor;
 }


 //dgUnsigned32 dgPointToCurveConstraint::CalcRestrictedDOF()
 //{
 // NEWTON_ASSERT (0);
 // return 0;
 //}


 void dgPointToCurveConstraint::SetPivotPoint(
 const dgVector &pivot,
 OnPointToCurveCallback curve,
 void *curveContext)
 {
 NEWTON_ASSERT (0);

 // NEWTON_ASSERT (m_dynamic0);

 // m_curve = curve;
 // m_context = curveContext;
 // m_pivot = m_dynamic0->GetBody()->GetMatrix().UntransformVector (pivot);

 }


 dgUnsigned32 dgPointToCurveConstraint::JacobianDerivative (dgContraintDescritor& params)
 {
 NEWTON_ASSERT (0);
 return 0;
 // dgBody* body0;
 // dgFloat32 kPosit;
 //
 // NEWTON_ASSERT (m_dynamic0);
 //
 // body0 = m_dynamic0->GetBody();
 // NEWTON_ASSERT (body0);
 //
 // NEWTON_ASSERT (body0->GetDynamic() == m_dynamic0);
 //
 // const dgMatrix& mat0 = body0->GetMatrix();
 //
 // dgVector front;
 // dgVector up;
 // dgVector right;
 // dgVector tangent;
 //  dgVector pivet1;
 // dgVector pivet0 (mat0.RotateVector (m_pivot));
 //
 // dgVector    point (pivet0 + mat0.posit);
 // m_curve (m_context, pivet0, pivet1, front);

 // front = front.Scale (dgRsqrt (front % front));
 // Create__Basis(front, up, right);

 // dgVector pivetUp (pivet0 * up);
 // dgVector pivetRight (pivet0 * right);

 // dgJacobian* jacobian01 = params.m_jacobian_01;

 // jacobian01[0].derivative[0] = up.m_x;
 // jacobian01[0].derivative[1] = up.m_y;
 // jacobian01[0].derivative[2] = up.m_z;
 // jacobian01[0].derivative[3] = pivetUp.m_x;
 // jacobian01[0].derivative[4] = pivetUp.m_y;
 // jacobian01[0].derivative[5] = pivetUp.m_z;

 // jacobian01[1].derivative[0] = right.m_x;
 // jacobian01[1].derivative[1] = right.m_y;
 // jacobian01[1].derivative[2] = right.m_z;
 // jacobian01[1].derivative[3] = pivetRight.m_x;
 // jacobian01[1].derivative[4] = pivetRight.m_y;
 // jacobian01[1].derivative[5] = pivetRight.m_z;

 // kPosit = 0.7f * params.m_invTimestep * params.m_invTimestep;
 // dgVector positError (pivet1 - point);

 // params.m_jointAccel[0] = (positError % up) * kPosit;
 // params.m_jointAccel[1] = (positError % right) * kPosit;
 }
 */
