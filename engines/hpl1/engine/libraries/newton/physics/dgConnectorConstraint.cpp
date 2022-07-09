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

#include "dgConnectorConstraint.h"
#include "dgBody.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


/*
 class dgConnectorConstraintArray: public dgPoolContainer<dgConnectorConstraint>
 {
 public:

 static dgConnectorConstraintArray& GetPool()
 {
 static dgConnectorConstraintArray pool;
 return pool;
 }
 };


 //////////////////////////////////////////////////////////////////////
 // Construction/Destruction
 //////////////////////////////////////////////////////////////////////

 dgConnectorConstraint::dgConnectorConstraint()
 {

 }

 dgConnectorConstraint::~dgConnectorConstraint()
 {

 }



 dgConnectorConstraint* dgConnectorConstraint::Create()
 {
 dgConnectorConstraint*	constraint;

 constraint = dgConnectorConstraintArray::GetPool().GetElement();

 constraint->Init();
 constraint->m_maxDOF = 6;
 constraint->m_constId = dgConectorConstraintId;
 return constraint;
 }

 void dgConnectorConstraint::Remove()
 {
 _ASSERTE (0);
 }


 void dgConnectorConstraint::Setup ()
 {
 _ASSERTE (0);

 dgVector p0 (m_dynamic0->GetBody()->GetMatrix().posit);
 dgVector p1 (p0);
 if (m_dynamic1) {
 p1 = m_dynamic1->GetBody()->GetMatrix().posit;
 }

 dgVector pivet (p1 + p0);
 pivet = pivet.Scale (0.5f);
 dgBallConstraint::SetPivotPoint (pivet);

 }


 dgUnsigned32 dgConnectorConstraint::JacobianDerivative (dgContraintDescritor& params)
 {
 _ASSERTE (0);
 return 0;

 //	dgFloat32 k;
 dgBody* body0;
 dgBallConstraint::JacobianDerivative (params);

 body0 = m_dynamic0->GetBody();
 const dgMatrix& mat0 = body0->GetMatrix();
 dgJacobian* jacobian01 = &params.m_jacobian_01[3];

 jacobian01[0].derivative[0] = 0.0f;
 jacobian01[0].derivative[1] = 0.0f;
 jacobian01[0].derivative[2] = 0.0f;
 jacobian01[0].derivative[3] = mat0.front.m_x;
 jacobian01[0].derivative[4] = mat0.front.m_y;
 jacobian01[0].derivative[5] = mat0.front.m_z;

 jacobian01[1].derivative[0] = 0.0f;
 jacobian01[1].derivative[1] = 0.0f;
 jacobian01[1].derivative[2] = 0.0f;
 jacobian01[1].derivative[3] = mat0.up.m_x;
 jacobian01[1].derivative[4] = mat0.up.m_y;
 jacobian01[1].derivative[5] = mat0.up.m_z;

 jacobian01[2].derivative[0] = 0.0f;
 jacobian01[2].derivative[1] = 0.0f;
 jacobian01[2].derivative[2] = 0.0f;
 jacobian01[2].derivative[3] = mat0.right.m_x;
 jacobian01[2].derivative[4] = mat0.right.m_y;
 jacobian01[2].derivative[5] = mat0.right.m_z;

 if (m_dynamic1) {
 dgJacobian* jacobian10 = &params.m_jacobian_10[3];
 jacobian10[0].derivative[0] = 0.0f;
 jacobian10[0].derivative[1] = 0.0f;
 jacobian10[0].derivative[2] = 0.0f;
 jacobian10[0].derivative[3] = -mat0.front.m_x;
 jacobian10[0].derivative[4] = -mat0.front.m_y;
 jacobian10[0].derivative[5] = -mat0.front.m_z;

 jacobian10[1].derivative[0] = 0.0f;
 jacobian10[1].derivative[1] = 0.0f;
 jacobian10[1].derivative[2] = 0.0f;
 jacobian10[1].derivative[3] = -mat0.up.m_x;
 jacobian10[1].derivative[4] = -mat0.up.m_y;
 jacobian10[1].derivative[5] = -mat0.up.m_z;

 jacobian10[2].derivative[0] = 0.0f;
 jacobian10[2].derivative[1] = 0.0f;
 jacobian10[2].derivative[2] = 0.0f;
 jacobian10[2].derivative[3] = -mat0.right.m_x;
 jacobian10[2].derivative[4] = -mat0.right.m_y;
 jacobian10[2].derivative[5] = -mat0.right.m_z;
 }

 //  k = 0.75f * params.m_invTimestep;
 //  dgVector angle (front0 * front1);
 }
 */
