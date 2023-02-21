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

#include "dgContact.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#define REST_RELATIVE_VELOCITY dgFloat32(1.0e-3f)
#define MAX_DYNAMIC_FRICTION_SPEED dgFloat32(0.3f)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgContactMaterial::dgContactMaterial() {
	//  NEWTON_ASSERT ((sizeof (dgContactMaterial) & 15) == 0);
	NEWTON_ASSERT((((dgUnsigned64)this) & 15) == 0);
	m_point.m_x = dgFloat32(0.0f);
	m_point.m_y = dgFloat32(0.0f);
	m_point.m_z = dgFloat32(0.0f);
	m_softness = dgFloat32(0.1f);
	m_restitution = dgFloat32(0.4f);

	m_staticFriction0 = dgFloat32(0.9f);
	m_staticFriction1 = dgFloat32(0.9f);
	m_dynamicFriction0 = dgFloat32(0.5f);
	m_dynamicFriction1 = dgFloat32(0.5f);
	m_dir0_Force = dgFloat32(0.0f);
	m_dir1_Force = dgFloat32(0.0f);
	m_normal_Force = dgFloat32(0.0f);
	m_penetrationPadding = dgFloat32(0.0f);
	// m_penetrationPadding = 0.5f;

	//  m_collisionEnable = true;
	//  m_friction0Enable = true;
	//  m_friction1Enable = true;
	//  m_collisionContinueCollisionEnable = true;
	m_flags = m_collisionEnable__ | m_friction0Enable__ | m_friction1Enable__ | m_collisionContinueCollisionEnable__;
}

dgContact::dgContact(dgWorld *world) : dgConstraint(), dgList<dgContactMaterial>(world->GetAllocator()) {
	NEWTON_ASSERT((((dgUnsigned64)this) & 15) == 0);

	dgActiveContacts &activeContacts = *world;

	m_contactNode = activeContacts.Append(this);

	m_maxDOF = 3;
	m_broadphaseLru = 0;
	m_enableCollision = true;
	m_constId = dgContactConstraintId;
	m_world = world;
	m_myCacheMaterial = NULL;
}

inline dgContact::~dgContact() {
	dgActiveContacts &activeContacts = *m_world;
	dgList<dgContactMaterial>::RemoveAll();
	activeContacts.Remove(m_contactNode);
}

void dgContact::GetInfo(dgConstraintInfo *const info) const {
	info->clear();
	InitInfo(info);
	info->m_collideCollisionOn = GetCount();
	strncpy(info->m_discriptionType, "contact", 8);
}

void dgContact::CalculatePointDerivative(dgInt32 index,
        dgContraintDescritor &desc, const dgVector &dir,
        const dgPointParam &param) const {
	NEWTON_ASSERT(m_body0);
	NEWTON_ASSERT(m_body1);

	dgVector r0CrossDir(param.m_r0 * dir);
	dgJacobian &jacobian0 = desc.m_jacobian[index].m_jacobian_IM0;
	jacobian0.m_linear[0] = dir.m_x;
	jacobian0.m_linear[1] = dir.m_y;
	jacobian0.m_linear[2] = dir.m_z;
	jacobian0.m_linear[3] = dgFloat32(0.0f);
	jacobian0.m_angular[0] = r0CrossDir.m_x;
	jacobian0.m_angular[1] = r0CrossDir.m_y;
	jacobian0.m_angular[2] = r0CrossDir.m_z;
	jacobian0.m_angular[3] = dgFloat32(0.0f);

	dgVector r1CrossDir(dir * param.m_r1);
	dgJacobian &jacobian1 = desc.m_jacobian[index].m_jacobian_IM1;
	jacobian1.m_linear[0] = -dir.m_x;
	jacobian1.m_linear[1] = -dir.m_y;
	jacobian1.m_linear[2] = -dir.m_z;
	jacobian1.m_linear[3] = dgFloat32(0.0f);
	jacobian1.m_angular[0] = r1CrossDir.m_x;
	jacobian1.m_angular[1] = r1CrossDir.m_y;
	jacobian1.m_angular[2] = r1CrossDir.m_z;
	jacobian1.m_angular[3] = dgFloat32(0.0f);
}

dgUnsigned32 dgContact::JacobianDerivative(dgContraintDescritor &params) {
	dgInt32 frictionIndex = 0;
	if (m_maxDOF) {
		dgInt32 i = 0;
		frictionIndex = GetCount();
		for (dgList<dgContactMaterial>::dgListNode *node = GetFirst(); node; node =
		            node->GetNext()) {
			dgContactMaterial &contact = node->GetInfo();
			JacobianContactDerivative(params, contact, i, frictionIndex);
			i++;
		}
	}

	return dgUnsigned32(frictionIndex);
}

void dgContact::JacobianContactDerivative(dgContraintDescritor &params,
        dgContactMaterial &contact, dgInt32 normalIndex,
        dgInt32 &frictionIndex) {
	dgPointParam pointData;

	InitPointParam(pointData, dgFloat32(1.0f), contact.m_point, contact.m_point);
	CalculatePointDerivative(normalIndex, params, contact.m_normal, pointData);

	dgVector velocError(pointData.m_veloc1 - pointData.m_veloc0);
	dgFloat32 restitution = contact.m_restitution;

	dgFloat32 relVelocErr = velocError % contact.m_normal;

	dgFloat32 penetration = GetMin(contact.m_penetration, dgFloat32(0.5f));
	dgFloat32 penetrationStiffness = dgFloat32(50.0f) * contact.m_softness;
	dgFloat32 penetrationVeloc = penetration * penetrationStiffness;
	NEWTON_ASSERT(
	    dgAbsf(penetrationVeloc - dgFloat32(50.0f) * contact.m_softness * GetMin(contact.m_penetration, dgFloat32(0.5f))) < dgFloat32(1.0e-6f));
	if (relVelocErr > REST_RELATIVE_VELOCITY) {
		relVelocErr *= (restitution + dgFloat32(1.0f));
	}

	params.m_restitution[normalIndex] = restitution;
	params.m_penetration[normalIndex] = penetration;
	params.m_penetrationStiffness[normalIndex] = penetrationStiffness;
	params.m_forceBounds[normalIndex].m_low = dgFloat32(0.0f);
	params.m_forceBounds[normalIndex].m_normalIndex = DG_NORMAL_CONSTRAINT;
	params.m_forceBounds[normalIndex].m_jointForce =
	    (dgFloat32 *)&contact.m_normal_Force;
	params.m_jointStiffness[normalIndex] = dgFloat32(1.0f);
	params.m_isMotor[normalIndex] = 0;

	params.m_jointAccel[normalIndex] = GetMax(dgFloat32(-4.0f),
	                                   relVelocErr + penetrationVeloc) *
	                                   params.m_invTimestep;
	//  params.m_jointAccel[normalIndex] = (penetrationVeloc + relVelocErr) * params.m_invTimestep;
	if (contact.m_flags & dgContactMaterial::m_overrideNormalAccel__) {
		params.m_jointAccel[normalIndex] += contact.m_normal_Force;
	}

	// first dir friction force
	if (contact.m_flags & dgContactMaterial::m_friction0Enable__) {
		dgInt32 jacobIndex = frictionIndex;
		frictionIndex += 1;
		CalculatePointDerivative(jacobIndex, params, contact.m_dir0, pointData);
		relVelocErr = velocError % contact.m_dir0;
		params.m_forceBounds[jacobIndex].m_normalIndex = normalIndex;
		params.m_jointStiffness[jacobIndex] = dgFloat32(1.0f);

		params.m_restitution[jacobIndex] = dgFloat32(0.0f);
		params.m_penetration[jacobIndex] = dgFloat32(0.0f);
		params.m_penetrationStiffness[jacobIndex] = dgFloat32(0.0f);
		//      if (contact.m_override0Accel) {
		if (contact.m_flags & dgContactMaterial::m_override0Accel__) {
			params.m_jointAccel[jacobIndex] = contact.m_dir0_Force;
			params.m_isMotor[jacobIndex] = 1;
		} else {
			params.m_jointAccel[jacobIndex] = relVelocErr * params.m_invTimestep;
			params.m_isMotor[jacobIndex] = 0;
		}
		if (dgAbsf(relVelocErr) > MAX_DYNAMIC_FRICTION_SPEED) {
			params.m_forceBounds[jacobIndex].m_low = -contact.m_dynamicFriction0;
			params.m_forceBounds[jacobIndex].m_upper = contact.m_dynamicFriction0;
		} else {
			params.m_forceBounds[jacobIndex].m_low = -contact.m_staticFriction0;
			params.m_forceBounds[jacobIndex].m_upper = contact.m_staticFriction0;
		}
		params.m_forceBounds[jacobIndex].m_jointForce =
		    (dgFloat32 *)&contact.m_dir0_Force;
	}

	//  if (contact.m_friction1Enable) {
	if (contact.m_flags & dgContactMaterial::m_friction1Enable__) {
		dgInt32 jacobIndex = frictionIndex;
		frictionIndex += 1;
		CalculatePointDerivative(jacobIndex, params, contact.m_dir1, pointData);
		relVelocErr = velocError % contact.m_dir1;
		params.m_forceBounds[jacobIndex].m_normalIndex = normalIndex;
		params.m_jointStiffness[jacobIndex] = dgFloat32(1.0f);

		params.m_restitution[jacobIndex] = dgFloat32(0.0f);
		params.m_penetration[jacobIndex] = dgFloat32(0.0f);
		params.m_penetrationStiffness[jacobIndex] = dgFloat32(0.0f);
		//      if (contact.m_override1Accel) {
		if (contact.m_flags & dgContactMaterial::m_override1Accel__) {
			NEWTON_ASSERT(0);
			params.m_jointAccel[jacobIndex] = contact.m_dir1_Force;
			params.m_isMotor[jacobIndex] = 1;
		} else {
			params.m_jointAccel[jacobIndex] = relVelocErr * params.m_invTimestep;
			params.m_isMotor[jacobIndex] = 0;
		}
		if (dgAbsf(relVelocErr) > MAX_DYNAMIC_FRICTION_SPEED) {
			params.m_forceBounds[jacobIndex].m_low = -contact.m_dynamicFriction1;
			params.m_forceBounds[jacobIndex].m_upper = contact.m_dynamicFriction1;
		} else {
			params.m_forceBounds[jacobIndex].m_low = -contact.m_staticFriction1;
			params.m_forceBounds[jacobIndex].m_upper = contact.m_staticFriction1;
		}
		params.m_forceBounds[jacobIndex].m_jointForce =
		    (dgFloat32 *)&contact.m_dir1_Force;
	}

	// dgTrace (("p(%f %f %f)\n", params.m_jointAccel[normalIndex], params.m_jointAccel[normalIndex + 1], params.m_jointAccel[normalIndex + 2]));
}

void dgContact::JointAccelerationsSimd(
    const dgJointAccelerationDecriptor &params) {
#ifdef DG_BUILD_SIMD_CODE
	//  simd_type bodyVeloc0;
	//  simd_type bodyOmega0;
	//  simd_type bodyVeloc1;
	//  simd_type bodyOmega1;
	//  simd_type zero;
	//  simd_type four;
	//  simd_type negOne;
	//  simd_type tol002;
	//  simd_type timeStep;
	//  simd_type invTimeStep;

	const dgJacobianPair *const Jt = params.m_Jt;

	simd_type zero = simd_set1(dgFloat32(0.0f));
	simd_type four = simd_set1(dgFloat32(4.0f));
	simd_type negOne = simd_set1(dgFloat32(-1.0f));
	simd_type tol002 = simd_set1(dgFloat32(1.0e-2f));
	simd_type timeStep = simd_set1(params.m_timeStep);
	simd_type invTimeStep = simd_set1(params.m_invTimeStep);

	simd_type bodyVeloc0 = (simd_type &)m_body0->m_veloc;
	simd_type bodyOmega0 = (simd_type &)m_body0->m_omega;
	simd_type bodyVeloc1 = (simd_type &)m_body1->m_veloc;
	simd_type bodyOmega1 = (simd_type &)m_body1->m_omega;

	for (dgInt32 k = 0; k < params.m_rowsCount; k++) {
		if (!params.m_accelIsMotor[k]) {
			// dgVector relVeloc (Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
			// relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
			// relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
			// relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);
			simd_type relVeloc =
			    simd_mul_v((simd_type &)Jt[k].m_jacobian_IM0.m_linear, bodyVeloc0);
			relVeloc =
			    simd_mul_add_v(relVeloc, (simd_type &)Jt[k].m_jacobian_IM0.m_angular, bodyOmega0);
			relVeloc =
			    simd_mul_add_v(relVeloc, (simd_type &)Jt[k].m_jacobian_IM1.m_linear, bodyVeloc1);
			relVeloc =
			    simd_mul_add_v(relVeloc, (simd_type &)Jt[k].m_jacobian_IM1.m_angular, bodyOmega1);

			//      vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;
			//      aRel = relAccel.m_x + relAccel.m_y + relAccel.m_z;
			relVeloc = simd_add_v(relVeloc, simd_move_hl_v(relVeloc, relVeloc));
			relVeloc =
			    simd_add_s(relVeloc, simd_permut_v(relVeloc, relVeloc, PURMUT_MASK(3, 3, 3, 1)));
			simd_type relAccel = simd_load_s(params.m_externAccelaration[k]);
			if (params.m_normalForceIndex[k] < 0) {
				//          dgFloat32 restitution;
				// simd_type penetration;
				// simd_type restitution;
				// simd_type velocMask;
				// simd_type penetrationVeloc;
				// simd_type penetrationMask;

				//          restitution = dgFloat32 (1.0f);
				//          if (vRel <= dgFloat32 (0.0f)) {
				//              restitution += params.m_restitution[k];
				//          }
				simd_type restitution =
				    simd_sub_s(simd_and_v(simd_set1(params.m_restitution[k]), simd_cmplt_s(relVeloc, zero)), negOne);

				simd_type penetration = simd_load_s(params.m_penetration[k]);
				//          dgFloat32 penetrationVeloc;
				//          penetrationVeloc = 0.0f;
				//          if (params.m_penetration[k] > dgFloat32 (1.0e-2f)) {
				//              if (vRel > dgFloat32 (0.0f)) {
				//                  NEWTON_ASSERT (penetrationCorrection >= dgFloat32 (0.0f));
				//                  params.m_penetration[k] = GetMax (dgFloat32 (0.0f), params.m_penetration[k] - vRel * params.m_timeStep);
				//              }
				//              penetrationVeloc = -(params.m_penetration[k] * params.m_penetrationStiffness[k]);
				//          }
				simd_type penetrationMask = simd_cmpgt_s(penetration, tol002);
				simd_type velocMask =
				    simd_and_v(penetrationMask, simd_cmpgt_s(relVeloc, zero));
				penetration =
				    simd_max_s(zero, simd_sub_s(penetration, simd_and_v(simd_mul_s(relVeloc, timeStep), velocMask)));
				simd_type penetrationVeloc =
				    simd_and_v(simd_mul_s(penetration, simd_load_s(params.m_penetrationStiffness[k])), penetrationMask);
				simd_store_s(penetration, &params.m_penetration[k]);

				//          vRel *= restitution;
				//          vRel = GetMin (dgFloat32 (4.0f), vRel + penetrationVeloc);
				//          relVeloc = simd_mul_s (relVeloc, restitution);
				relVeloc =
				    simd_min_s(four, simd_sub_s(simd_mul_s(relVeloc, restitution), penetrationVeloc));
			}

			// params.m_coordenateAccel[k] = -aRel;
			// relAccel = simd_mul_add_s (relAccel, relVeloc, invTimeStep);
			// simd_store_s (simd_mul_s (relAccel, negOne), &params.m_coordenateAccel[k]);

			// params.m_coordenateAccel[k] = (aRel - vRel * params.m_invTimeStep);
			simd_store_s(simd_mul_sub_s(relAccel, relVeloc, invTimeStep),
			             &params.m_coordenateAccel[k]);
		}
	}
#endif
}

void dgContact::JointAccelerations(const dgJointAccelerationDecriptor &params) {
	const dgJacobianPair *const Jt = params.m_Jt;
	const dgVector &bodyVeloc0 = m_body0->m_veloc;
	const dgVector &bodyOmega0 = m_body0->m_omega;
	const dgVector &bodyVeloc1 = m_body1->m_veloc;
	const dgVector &bodyOmega1 = m_body1->m_omega;

	for (dgInt32 k = 0; k < params.m_rowsCount; k++) {
		if (!params.m_accelIsMotor[k]) {
			dgFloat32 vRel;
			dgFloat32 aRel;

			dgVector relVeloc(Jt[k].m_jacobian_IM0.m_linear.CompProduct(bodyVeloc0));
			relVeloc += Jt[k].m_jacobian_IM0.m_angular.CompProduct(bodyOmega0);
			relVeloc += Jt[k].m_jacobian_IM1.m_linear.CompProduct(bodyVeloc1);
			relVeloc += Jt[k].m_jacobian_IM1.m_angular.CompProduct(bodyOmega1);
			vRel = relVeloc.m_x + relVeloc.m_y + relVeloc.m_z;
			aRel = params.m_externAccelaration[k];

			if (params.m_normalForceIndex[k] < 0) {
				dgFloat32 restitution;
				dgFloat32 penetrationVeloc;

				restitution = dgFloat32(1.0f);
				if (vRel <= dgFloat32(0.0f)) {
					restitution += params.m_restitution[k];
				}

				penetrationVeloc = 0.0f;
				if (params.m_penetration[k] > dgFloat32(1.0e-2f)) {
					dgFloat32 penetrationCorrection;
					if (vRel > dgFloat32(0.0f)) {
						penetrationCorrection = vRel * params.m_timeStep;
						NEWTON_ASSERT(penetrationCorrection >= dgFloat32(0.0f));
						params.m_penetration[k] = GetMax(dgFloat32(0.0f),
						                                 params.m_penetration[k] - penetrationCorrection);
					}
					penetrationVeloc = -(params.m_penetration[k] * params.m_penetrationStiffness[k]);
				}

				vRel *= restitution;
				vRel = GetMin(dgFloat32(4.0f), vRel + penetrationVeloc);
			}
			params.m_coordenateAccel[k] = (aRel - vRel * params.m_invTimeStep);
		}
	}
}

void dgContact::JointVelocityCorrection(
    const dgJointAccelerationDecriptor &params) {
	NEWTON_ASSERT(0);
}

dgCollidingPairCollector::dgCollidingPairCollector() {
	m_count = 0;
	m_maxSize = 0;
	//  m_world = NULL;
	m_pairs = NULL;
	m_sentinel = NULL;
}

dgCollidingPairCollector::~dgCollidingPairCollector() {
}

void dgCollidingPairCollector::Init() {
	//  m_world = me;
	dgWorld *const world = (dgWorld *)this;
	m_count = 0;
	m_maxSize = dgInt32(world->m_pairMemoryBufferSizeInBytes / sizeof(dgPair));
	m_pairs = (dgPair *)world->m_pairMemoryBuffer;
}

void dgCollidingPairCollector::SetCaches(dgThreadPairCache *const chaches) {
	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
		m_chacheBuffers[i] = &chaches[i];
		m_chacheBuffers[i]->m_count = 0;
	}
}

void dgCollidingPairCollector::FlushChache(dgThreadPairCache *const pairChache) {
	dgWorld *const world = (dgWorld *)this;
	while ((m_count + pairChache->m_count) > m_maxSize) {
		void *newBuffer;
		newBuffer = world->m_allocator->Malloc(
		                2 * world->m_pairMemoryBufferSizeInBytes);

		world->m_pairMemoryBufferSizeInBytes *= 2;
		memcpy(newBuffer, world->m_pairMemoryBuffer, m_maxSize * sizeof(dgPair));
		world->m_allocator->Free(world->m_pairMemoryBuffer);

		world->m_pairMemoryBuffer = newBuffer;
		m_maxSize = dgInt32(world->m_pairMemoryBufferSizeInBytes / sizeof(dgPair));
		m_pairs = (dgPair *)world->m_pairMemoryBuffer;
	}

	memcpy(&m_pairs[m_count], pairChache->m_chacheBuffer,
	       sizeof(dgPair) * pairChache->m_count);
	m_count += pairChache->m_count;
	pairChache->m_count = 0;
}

void dgCollidingPairCollector::AddPair(dgBody *const bodyPtr0,
                                       dgBody *const bodyPtr1, dgInt32 threadIndex) {
	if ((bodyPtr0 != m_sentinel) && (bodyPtr1 != m_sentinel)) {
		dgWorld *const world = (dgWorld *)this;
		if (bodyPtr0->GetSleepState() && bodyPtr1->GetSleepState()) {
			dgContact *contact = NULL;
			if (bodyPtr0->m_invMass.m_w != dgFloat32(0.0f)) {
				for (dgBodyMasterListRow::dgListNode *link = world->FindConstraintLink(
				            bodyPtr0, bodyPtr1);
				        link;
				        link = world->FindConstraintLinkNext(link, bodyPtr1)) {
					dgConstraint *const constraint = link->GetInfo().m_joint;
					if (constraint->GetId() == dgContactConstraintId) {
						contact = (dgContact *)constraint;
						break;
					}
				}
			} else if (bodyPtr1->m_invMass.m_w != dgFloat32(0.0f)) {
				NEWTON_ASSERT(bodyPtr1->m_invMass.m_w != dgFloat32(0.0f));
				for (dgBodyMasterListRow::dgListNode *link = world->FindConstraintLink(
				            bodyPtr1, bodyPtr0);
				        link;
				        link = world->FindConstraintLinkNext(link, bodyPtr0)) {
					dgConstraint *const constraint = link->GetInfo().m_joint;
					if (constraint->GetId() == dgContactConstraintId) {
						contact = (dgContact *)constraint;
						break;
					}
				}
			} else {
				return;
			}

			if (contact) {
				NEWTON_ASSERT(contact->GetId() == dgContactConstraintId);
				contact->m_broadphaseLru = dgInt32(world->m_broadPhaseLru);
			}

		} else {
			dgBody *tmpbody0(bodyPtr0);
			dgBody *tmpbody1(bodyPtr1);
			if (tmpbody0->m_uniqueID > tmpbody1->m_uniqueID) {
				Swap(tmpbody0, tmpbody1);
			}
			dgBody *const body0(tmpbody0);
			dgBody *const body1(tmpbody1);

			NEWTON_ASSERT(body0->GetWorld());
			NEWTON_ASSERT(body1->GetWorld());
			NEWTON_ASSERT(body0->GetWorld() == world);
			NEWTON_ASSERT(body1->GetWorld() == world);

			dgContact *contact = NULL;
			if (body0->m_invMass.m_w != dgFloat32(0.0f)) {
				for (dgBodyMasterListRow::dgListNode *link = world->FindConstraintLink(
				            body0, body1);
				        link;
				        link = world->FindConstraintLinkNext(link, body1)) {
					dgConstraint *const constraint = link->GetInfo().m_joint;
					if (constraint->GetId() == dgContactConstraintId) {
						contact = (dgContact *)constraint;
					} else {
						if (!constraint->IsCollidable()) {
							return;
						}
					}
				}
			} else if (body1->m_invMass.m_w != dgFloat32(0.0f)) {
				NEWTON_ASSERT(body1->m_invMass.m_w != dgFloat32(0.0f));
				for (dgBodyMasterListRow::dgListNode *link = world->FindConstraintLink(
				            body1, body0);
				        link;
				        link = world->FindConstraintLinkNext(link, body0)) {
					dgConstraint *const constraint = link->GetInfo().m_joint;
					if (constraint->GetId() == dgContactConstraintId) {
						contact = (dgContact *)constraint;
					} else {
						if (!constraint->IsCollidable()) {
							return;
						}
					}
				}
			} else {
				return;
			}

			if (!(body0->m_collideWithLinkedBodies & body1->m_collideWithLinkedBodies)) {
				if (world->AreBodyConnectedByJoints(body0, body1)) {
					return;
				}
			}

			NEWTON_ASSERT(!contact || contact->GetId() == dgContactConstraintId);

			dgUnsigned32 group0_ID = dgUnsigned32(body0->m_bodyGroupId);
			dgUnsigned32 group1_ID = dgUnsigned32(body1->m_bodyGroupId);
			if (group1_ID < group0_ID) {
				Swap(group0_ID, group1_ID);
			}

			dgUnsigned32 key = (group1_ID << 16) + group0_ID;
			const dgBodyMaterialList &materialList = *world;

			const dgContactMaterial *const material =
			    &materialList.Find(key)->GetInfo();
			//          if (material->m_collisionEnable) {
			if (material->m_flags & dgContactMaterial::m_collisionEnable__) {
				dgInt32 processContacts;

				processContacts = 1;
				if (material->m_aabbOverlap) {
					processContacts = material->m_aabbOverlap(*material, *body0, *body1,
					                  threadIndex);
				}
				if (processContacts) {
					NEWTON_ASSERT(
					    !body0->m_collision->IsType(dgCollision::dgCollisionNull_RTTI));
					NEWTON_ASSERT(
					    !body1->m_collision->IsType(dgCollision::dgCollisionNull_RTTI));

					dgThreadPairCache &pairChache = *m_chacheBuffers[threadIndex];

					if (pairChache.m_count >= DG_CACHE_PAIR_BUFFER) {
						world->dgGetUserLock();
						FlushChache(&pairChache);
						world->dgReleasedUserLock();
					}

					dgInt32 count = pairChache.m_count;
					pairChache.m_chacheBuffer[count].m_body0 = body0;
					pairChache.m_chacheBuffer[count].m_body1 = body1;
					pairChache.m_chacheBuffer[count].m_material = material;
					pairChache.m_chacheBuffer[count].m_contact = contact;
					pairChache.m_count = count + 1;
				}
			}
		}
	}
}
