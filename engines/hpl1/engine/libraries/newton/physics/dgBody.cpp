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
#include "dgCollision.h"
#include "dgCollisionCompound.h"
#include "dgCollisionCompoundBreakable.h"
#include "dgContact.h"
#include "dgWorld.h"
#include "dgWorldDynamicUpdate.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define DG_AABB_ERROR dgFloat32(1.0e-4f)
#define DG_MAX_ANGLE_STEP dgFloat32(45.0f * dgDEG2RAD)

dgBody::dgBody() {
	//	int xx = sizeof (dgBody);
	_ASSERTE((sizeof(dgBody) & 0x0f) == 0);
}

dgBody::~dgBody() {
	//	_ASSERTE (m_collisionCell.GetCount() == 0);
}

void dgBody::SetAparentMassMatrix(const dgVector &massMatrix) {
	m_aparentMass = massMatrix;
	if (m_collision->IsType(dgCollision::dgCollisionMesh_RTTI)) {
		m_aparentMass.m_w = DG_INFINITE_MASS * 2.0f;
	}

	if (m_aparentMass.m_w >= DG_INFINITE_MASS) {
		m_aparentMass.m_x = DG_INFINITE_MASS;
		m_aparentMass.m_y = DG_INFINITE_MASS;
		m_aparentMass.m_z = DG_INFINITE_MASS;
		m_aparentMass.m_w = DG_INFINITE_MASS;
	}
}

void dgBody::SetMassMatrix(dgFloat32 mass, dgFloat32 Ix, dgFloat32 Iy,
						   dgFloat32 Iz) {
	//	_ASSERTE (mass > dgFloat32 (0.0f));
	if (m_collision->IsType(dgCollision::dgCollisionMesh_RTTI)) {
		mass = DG_INFINITE_MASS * 2.0f;
	}

	if (mass >= DG_INFINITE_MASS) {
		m_mass.m_x = DG_INFINITE_MASS;
		m_mass.m_y = DG_INFINITE_MASS;
		m_mass.m_z = DG_INFINITE_MASS;
		m_mass.m_w = DG_INFINITE_MASS;
		m_invMass.m_x = dgFloat32(0.0f);
		m_invMass.m_y = dgFloat32(0.0f);
		m_invMass.m_z = dgFloat32(0.0f);
		m_invMass.m_w = dgFloat32(0.0f);

		dgBodyMasterList &masterList(*m_world);
		if (masterList.GetFirst() != m_masterNode) {
			masterList.InsertAfter(masterList.GetFirst(), m_masterNode);
		}

	} else {
		_ASSERTE(Ix > dgFloat32(0.0f));
		_ASSERTE(Iy > dgFloat32(0.0f));
		_ASSERTE(Iz > dgFloat32(0.0f));

		//		dgVector omega (m_mass.CompProduct(m_matrix.UnrotateVector(m_omega)));

		m_mass.m_x = Ix;
		m_mass.m_y = Iy;
		m_mass.m_z = Iz;
		m_mass.m_w = mass;

		m_invMass.m_x = dgFloat32(1.0f) / Ix;
		m_invMass.m_y = dgFloat32(1.0f) / Iy;
		m_invMass.m_z = dgFloat32(1.0f) / Iz;
		m_invMass.m_w = dgFloat32(1.0f) / mass;
		//		if (m_applyGyroscopic) {
		//			m_omega = m_matrix.UnrotateVector (omega.CompProduct(m_invMass));
		//		}

		dgBodyMasterList &masterList(*m_world);
		masterList.RotateToEnd(m_masterNode);
	}

#ifdef _DEBUG
	dgBodyMasterList &me = *m_world;
	dgBodyMasterList::dgListNode *refNode;
	for (refNode = me.GetFirst(); refNode; refNode = refNode->GetNext()) {
		if (refNode->GetInfo().GetBody()->m_invMass.m_w != 0.0f) {
			for (; refNode; refNode = refNode->GetNext()) {
				dgBody *body = refNode->GetInfo().GetBody();
				_ASSERTE(body->m_invMass.m_w != 0.0f);
			}
			break;
		}
	}

#endif

	SetAparentMassMatrix(m_mass);
}

void dgBody::AttachCollision(dgCollision *collision) {
	_ASSERTE(collision);
	if (collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {

		if (collision->IsType(dgCollision::dgCollisionCompoundBreakable_RTTI)) {
			dgCollisionCompoundBreakable *const compound =
				(dgCollisionCompoundBreakable *)collision;
			collision = new (m_world->GetAllocator()) dgCollisionCompoundBreakable(
				*compound);

		} else {
			dgCollisionCompound *const compound = (dgCollisionCompound *)collision;
			collision = new (m_world->GetAllocator()) dgCollisionCompound(*compound);
		}

	} else {
		collision->AddRef();
	}

	if (m_collision) {
		m_world->ReleaseCollision(m_collision);
		m_collision = collision;
		if (m_collision->IsType(dgCollision::dgCollisionMesh_RTTI)) {
			SetMassMatrix(m_mass.m_w, m_mass.m_x, m_mass.m_y, m_mass.m_z);
		}
		SetMatrix(m_matrix);
	} else {
		m_collision = collision;
		if (m_collision->IsType(dgCollision::dgCollisionMesh_RTTI)) {
			SetMassMatrix(m_mass.m_w, m_mass.m_x, m_mass.m_y, m_mass.m_z);
		}
	}
}

void dgBody::SetMatrix(const dgMatrix &matrix) {
	SetMatrixOriginAndRotation(matrix);

	if (!m_inCallback) {
		if (m_world->m_cpu == dgSimdPresent) {
			UpdateCollisionMatrixSimd(dgFloat32(0.0f), 0);
		} else {
			UpdateCollisionMatrix(dgFloat32(0.0f), 0);
		}
	}
}

void dgBody::SetMatrixIgnoreSleep(const dgMatrix &matrix) {
	dgBroadPhaseCollision &collisionSystem = *m_world;
	if (m_collisionCell.m_cell == &collisionSystem.m_inactiveList) {
		if (!m_spawnnedFromCallback) {
			m_world->dgGetUserLock();
			collisionSystem.Remove(this);
			collisionSystem.Add(this);
			m_world->dgReleasedUserLock();
		} else {
			collisionSystem.Remove(this);
			collisionSystem.Add(this);
		}
	}

	m_sleeping = false;
	m_prevExternalForce = dgVector(0.0f, 0.0f, 0.0f, 0.0f);
	m_prevExternalTorque = dgVector(0.0f, 0.0f, 0.0f, 0.0f);
	SetMatrix(matrix);
}

void dgBody::UpdateCollisionMatrixSimd(dgFloat32 timestep, dgInt32 threadIndex) {
#ifdef DG_BUILD_SIMD_CODE

	m_collisionWorldMatrix = m_collision->m_offset.MultiplySimd(m_matrix);

	dgVector oldP0(m_minAABB);
	dgVector oldP1(m_maxAABB);
	m_collision->CalcAABBSimd(m_collisionWorldMatrix, m_minAABB, m_maxAABB);

	if (m_continueCollisionMode) {
		dgFloat32 padding = m_collision->GetBoxMaxRadius() - m_collision->GetBoxMinRadius();
		dgFloat32 maxOmega = (m_omega % m_omega) * timestep * timestep;
		padding = (maxOmega > 1.0f) ? padding : padding * dgSqrt(maxOmega);
		dgVector step(
			m_veloc.Scale(timestep) + m_accel.Scale(m_invMass.m_w * timestep * timestep));
		step.m_x += (step.m_x > 0.0f) ? padding : -padding;
		step.m_y += (step.m_y > 0.0f) ? padding : -padding;
		step.m_z += (step.m_z > 0.0f) ? padding : -padding;

		dgVector boxSize((m_maxAABB - m_minAABB).Scale(dgFloat32(0.25f)));
		for (dgInt32 j = 0; j < 3; j++) {
			if (dgAbsf(step[j]) > boxSize[j]) {
				if (step[j] > dgFloat32(0.0f)) {
					m_maxAABB[j] += step[j];
				} else {
					m_minAABB[j] += step[j];
				}
			}
		}

		if (m_collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {
			dgCollisionCompound *const compoundCollision =
				(dgCollisionCompound *)m_collision;
			//			dgInt32 count = compoundCollision->m_count;
			//			for (dgInt32 i = 0 ; i < count; i ++) {
			//				dgVector& box1Min = compoundCollision->m_aabb[i].m_p0;
			//				dgVector& box1Max = compoundCollision->m_aabb[i].m_p1;
			dgVector &box1Min = compoundCollision->m_root->m_p0;
			dgVector &box1Max = compoundCollision->m_root->m_p1;

			dgVector boxSize((box1Max - box1Min).Scale(dgFloat32(0.25f)));
			for (dgInt32 j = 0; j < 3; j++) {
				if (dgAbsf(step[j]) > boxSize[j]) {
					if (step[j] > dgFloat32(0.0f)) {
						box1Max[j] += step[j];
					} else {
						box1Min[j] += step[j];
					}
				}
			}
			//			}
		}
	}

	if (m_collisionCell.m_cell) {
		_ASSERTE(m_world);
		if (!m_sleeping) {
			if ((dgAbsf(oldP0.m_x - m_minAABB.m_x) > DG_AABB_ERROR) || (dgAbsf(oldP0.m_y - m_minAABB.m_y) > DG_AABB_ERROR) || (dgAbsf(oldP0.m_z - m_minAABB.m_z) > DG_AABB_ERROR) || (dgAbsf(oldP1.m_x - m_maxAABB.m_x) > DG_AABB_ERROR) || (dgAbsf(oldP1.m_y - m_maxAABB.m_y) > DG_AABB_ERROR) || (dgAbsf(oldP1.m_z - m_maxAABB.m_z) > DG_AABB_ERROR)) {
				m_world->UpdateBodyBroadphase(this, threadIndex);
			} else {
				m_collisionCell.m_cell->m_active = 1;
			}
		}
	}
#endif
}

void dgBody::UpdateCollisionMatrix(dgFloat32 timestep, dgInt32 threadIndex) {
	m_collisionWorldMatrix = m_collision->m_offset * m_matrix;

	dgVector oldP0(m_minAABB);
	dgVector oldP1(m_maxAABB);
	m_collision->CalcAABB(m_collisionWorldMatrix, m_minAABB, m_maxAABB);

	if (m_continueCollisionMode) {
		dgFloat32 padding = m_collision->GetBoxMaxRadius() - m_collision->GetBoxMinRadius();
		dgFloat32 maxOmega = (m_omega % m_omega) * timestep * timestep;
		padding = (maxOmega > 1.0f) ? padding : padding * dgSqrt(maxOmega);
		dgVector step(
			m_veloc.Scale(timestep) + m_accel.Scale(m_invMass.m_w * timestep * timestep));
		step.m_x += (step.m_x > 0.0f) ? padding : -padding;
		step.m_y += (step.m_y > 0.0f) ? padding : -padding;
		step.m_z += (step.m_z > 0.0f) ? padding : -padding;

		dgVector boxSize((m_maxAABB - m_minAABB).Scale(dgFloat32(0.25f)));
		for (dgInt32 j = 0; j < 3; j++) {
			if (dgAbsf(step[j]) > boxSize[j]) {
				if (step[j] > dgFloat32(0.0f)) {
					m_maxAABB[j] += step[j];
				} else {
					m_minAABB[j] += step[j];
				}
			}
		}

		if (m_collision->IsType(dgCollision::dgCollisionCompound_RTTI)) {
			//_ASSERTE (0);

			dgCollisionCompound *const compoundCollision =
				(dgCollisionCompound *)m_collision;
			//			dgInt32 count = compoundCollision->m_count;
			//			for (dgInt32 i = 0 ; i < count; i ++) {
			//				dgVector& box1Min = compoundCollision->m_aabb[i].m_p0;
			//				dgVector& box1Max = compoundCollision->m_aabb[i].m_p1;
			dgVector &box1Min = compoundCollision->m_root->m_p0;
			dgVector &box1Max = compoundCollision->m_root->m_p1;

			dgVector boxSize((box1Max - box1Min).Scale(dgFloat32(0.25f)));
			for (dgInt32 j = 0; j < 3; j++) {
				if (dgAbsf(step[j]) > boxSize[j]) {
					if (step[j] > dgFloat32(0.0f)) {
						box1Max[j] += step[j];
					} else {
						box1Min[j] += step[j];
					}
				}
			}
			//			}
		}
	}

	if (m_collisionCell.m_cell) {
		_ASSERTE(m_world);

		if (!m_sleeping) {
			if ((dgAbsf(oldP0.m_x - m_minAABB.m_x) > DG_AABB_ERROR) || (dgAbsf(oldP0.m_y - m_minAABB.m_y) > DG_AABB_ERROR) || (dgAbsf(oldP0.m_z - m_minAABB.m_z) > DG_AABB_ERROR) || (dgAbsf(oldP1.m_x - m_maxAABB.m_x) > DG_AABB_ERROR) || (dgAbsf(oldP1.m_y - m_maxAABB.m_y) > DG_AABB_ERROR) || (dgAbsf(oldP1.m_z - m_maxAABB.m_z) > DG_AABB_ERROR)) {
				m_world->UpdateBodyBroadphase(this, threadIndex);
			} else {
				m_collisionCell.m_cell->m_active = 1;
			}
		}
	}
}

dgFloat32 dgBody::RayCast(const dgLineBox &line, OnRayCastAction filter,
						  OnRayPrecastAction preFilter, void *const userData, dgFloat32 minT) const {
	dgContactPoint contactOut;

	_ASSERTE(filter);
	if (m_world->m_cpu == dgSimdPresent) {
		if (dgOverlapTestSimd(line.m_boxL0, line.m_boxL1, m_minAABB, m_maxAABB)) {
			dgVector localP0(m_collisionWorldMatrix.UntransformVector(line.m_l0));
			dgVector localP1(m_collisionWorldMatrix.UntransformVector(line.m_l1));
			dgFloat32 t = m_collision->RayCastSimd(localP0, localP1, contactOut,
												   preFilter, this, userData);
			if (t < minT) {
				_ASSERTE(t >= 0.0f);
				_ASSERTE(t <= 1.0f);

				contactOut.m_normal = m_collisionWorldMatrix.RotateVectorSimd(
					contactOut.m_normal);
				minT = filter(this, contactOut.m_normal, dgInt32(contactOut.m_userId),
							  userData, t);
			}
		}
	} else {
		if (dgOverlapTest(line.m_boxL0, line.m_boxL1, m_minAABB, m_maxAABB)) {
			dgVector localP0(m_collisionWorldMatrix.UntransformVector(line.m_l0));
			dgVector localP1(m_collisionWorldMatrix.UntransformVector(line.m_l1));
			dgFloat32 t = m_collision->RayCast(localP0, localP1, contactOut,
											   preFilter, this, userData);
			if (t < minT) {
				_ASSERTE(t >= 0.0f);
				_ASSERTE(t <= 1.0f);
				contactOut.m_normal = m_collisionWorldMatrix.RotateVector(
					contactOut.m_normal);
				minT = filter(this, contactOut.m_normal, dgInt32(contactOut.m_userId),
							  userData, t);
			}
		}
	}
	return minT;
}

void dgBody::AddBuoyancyForce(dgFloat32 fluidDensity,
							  dgFloat32 fluidLinearViscousity, dgFloat32 fluidAngularViscousity,
							  const dgVector &gravityVector, GetBuoyancyPlane buoyancuPlane,
							  void *const context) {
	if (m_mass.m_w > dgFloat32(1.0e-2f)) {
		dgVector volumeIntegral(
			m_collision->CalculateVolumeIntegral(m_collisionWorldMatrix,
												 buoyancuPlane, context));
		if (volumeIntegral.m_w > dgFloat32(1.0e-4f)) {

			//			dgVector buoyanceCenter (volumeIntegral - m_matrix.m_posit);
			dgVector buoyanceCenter(volumeIntegral - m_globalCentreOfMass);

			dgVector force(gravityVector.Scale(-fluidDensity * volumeIntegral.m_w));
			dgVector torque(buoyanceCenter * force);

			dgFloat32 damp = GetMax(
				GetMin(
					(m_veloc % m_veloc) * dgFloat32(100.0f) * fluidLinearViscousity,
					dgFloat32(dgFloat32(1.0f))),
				dgFloat32(dgFloat32(10.0f)));
			force -= m_veloc.Scale(damp);

			// damp = (m_omega % m_omega) * dgFloat32 (10.0f) * fluidAngularViscousity;
			damp = GetMax(
				GetMin(
					(m_omega % m_omega) * dgFloat32(1000.0f) * fluidAngularViscousity,
					dgFloat32(0.25f)),
				dgFloat32(2.0f));
			torque -= m_omega.Scale(damp);

			//			_ASSERTE (dgSqrt (force % force) < (dgSqrt (gravityVector % gravityVector) * m_mass.m_w * dgFloat32 (100.0f)));
			//			_ASSERTE (dgSqrt (torque % torque) < (dgSqrt (gravityVector % gravityVector) * m_mass.m_w * dgFloat32 (100.0f) * dgFloat32 (10.0f)));

			m_world->dgGetUserLock();
			m_accel += force;
			m_alpha += torque;
			m_world->dgReleasedUserLock();
		}
	}
}

// void dgBody::CalcInvInertiaMatrix (dgMatrix& matrix) const
void dgBody::CalcInvInertiaMatrix() {
	_ASSERTE(m_invWorldInertiaMatrix[0][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[1][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[2][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[3][3] == dgFloat32(1.0f));

	m_invWorldInertiaMatrix[0][0] = m_invMass[0] * m_matrix[0][0];
	m_invWorldInertiaMatrix[0][1] = m_invMass[1] * m_matrix[1][0];
	m_invWorldInertiaMatrix[0][2] = m_invMass[2] * m_matrix[2][0];

	m_invWorldInertiaMatrix[1][0] = m_invMass[0] * m_matrix[0][1];
	m_invWorldInertiaMatrix[1][1] = m_invMass[1] * m_matrix[1][1];
	m_invWorldInertiaMatrix[1][2] = m_invMass[2] * m_matrix[2][1];

	m_invWorldInertiaMatrix[2][0] = m_invMass[0] * m_matrix[0][2];
	m_invWorldInertiaMatrix[2][1] = m_invMass[1] * m_matrix[1][2];
	m_invWorldInertiaMatrix[2][2] = m_invMass[2] * m_matrix[2][2];
	m_invWorldInertiaMatrix = m_invWorldInertiaMatrix * m_matrix;

	m_invWorldInertiaMatrix[3][0] = dgFloat32(0.0f);
	m_invWorldInertiaMatrix[3][1] = dgFloat32(0.0f);
	m_invWorldInertiaMatrix[3][2] = dgFloat32(0.0f);

	_ASSERTE(m_invWorldInertiaMatrix[0][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[1][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[2][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[3][3] == dgFloat32(1.0f));
}

void dgBody::CalcInvInertiaMatrixSimd() {
#ifdef DG_BUILD_SIMD_CODE
	//	simd_type m0;
	//	simd_type m1;
	//	simd_type m2;
	//	simd_type r0;
	//	simd_type r1;
	//	simd_type r2;

	simd_type r0 =
		simd_pack_lo_v((simd_type &)m_matrix[0], (simd_type &)m_matrix[1]);
	simd_type r1 =
		simd_pack_lo_v((simd_type &)m_matrix[2], (simd_type &)m_matrix[3]);
	simd_type m0 = simd_move_lh_v(r0, r1);
	simd_type m1 = simd_move_hl_v(r1, r0);
	r0 = simd_pack_hi_v((simd_type &)m_matrix[0], (simd_type &)m_matrix[1]);
	r1 = simd_pack_hi_v((simd_type &)m_matrix[2], (simd_type &)m_matrix[3]);
	simd_type m2 = simd_move_lh_v(r0, r1);

	r0 = simd_mul_v((simd_type &)m_matrix[0], simd_set1(m_invMass[0]));
	r1 = simd_mul_v((simd_type &)m_matrix[1], simd_set1(m_invMass[1]));
	simd_type r2 = simd_mul_v((simd_type &)m_matrix[2], simd_set1(m_invMass[2]));

	(simd_type &)m_invWorldInertiaMatrix[0] =
		simd_mul_add_v(
			simd_mul_add_v(simd_mul_v(r0, simd_permut_v(m0, m0, PURMUT_MASK(0, 0, 0, 0))),
						   r1, simd_permut_v(m0, m0, PURMUT_MASK(1, 1, 1, 1))),
			r2, simd_permut_v(m0, m0, PURMUT_MASK(2, 2, 2, 2)));

	(simd_type &)m_invWorldInertiaMatrix[1] =
		simd_mul_add_v(
			simd_mul_add_v(simd_mul_v(r0, simd_permut_v(m1, m1, PURMUT_MASK(0, 0, 0, 0))),
						   r1, simd_permut_v(m1, m1, PURMUT_MASK(1, 1, 1, 1))),
			r2, simd_permut_v(m1, m1, PURMUT_MASK(2, 2, 2, 2)));

	(simd_type &)m_invWorldInertiaMatrix[2] =
		simd_mul_add_v(
			simd_mul_add_v(simd_mul_v(r0, simd_permut_v(m2, m2, PURMUT_MASK(0, 0, 0, 0))),
						   r1, simd_permut_v(m2, m2, PURMUT_MASK(1, 1, 1, 1))),
			r2, simd_permut_v(m2, m2, PURMUT_MASK(2, 2, 2, 2)));

#ifdef _DEBUG
//	dgMatrix aaa (m_invWorldInertiaMatrix);
//	CalcInvInertiaMatrix ();
//	for (int i = 0; i < 4; i ++) {
//		for (int j = 0; j < 4; j ++) {
//			_ASSERTE (dgAbsf (aaa[i][j] - m_invWorldInertiaMatrix[i][j]) <= dgAbsf (aaa[i][j] * dgFloat32 (1.0e-3f)));
//		}
//	}
#endif

	_ASSERTE(m_invWorldInertiaMatrix[0][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[1][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[2][3] == dgFloat32(0.0f));
	_ASSERTE(m_invWorldInertiaMatrix[3][3] == dgFloat32(1.0f));

#else

#endif
}

/*
 void dgBody::IntegrateVelocity (dgFloat32 timestep, bool update)
 {
 dgFloat32 omegaMag2;
 dgFloat32 omegaAngle;
 dgFloat32 invOmegaMag;

 //	IntegrateNotUpdate (timestep);

 m_globalCentreOfMass += m_veloc.Scale (timestep);
 while (((m_omega % m_omega) * timestep * timestep) > (DG_MAX_ANGLE_STEP * DG_MAX_ANGLE_STEP)) {
 m_omega = m_omega.Scale (dgFloat32 (0.8f));
 }

 // this is correct
 omegaMag2 = m_omega % m_omega;
 if (omegaMag2 > ((dgFloat32 (0.0125f) * dgDEG2RAD) * (dgFloat32 (0.0125f) * dgDEG2RAD))) {
 invOmegaMag = dgRsqrt (omegaMag2);
 dgVector omegaAxis (m_omega.Scale (invOmegaMag));
 omegaAngle = invOmegaMag * omegaMag2 * timestep;
 dgQuaternion rotation (omegaAxis, omegaAngle);
 m_rotation = m_rotation * rotation;
 m_rotation.Scale(dgRsqrt (m_rotation.DotProduct (m_rotation)));
 m_matrix = dgMatrix (m_rotation, m_matrix.m_posit);
 }

 m_matrix.m_posit = m_globalCentreOfMass - m_matrix.RotateVector(m_localCentreOfMass);

 if (update) {
 if (m_matrixUpdate) {
 m_world->dgGetUserLock();
 m_matrixUpdate (*this, m_matrix);
 m_world->dgReleasedUserLock();
 }
 UpdateCollisionMatrix (timestep);
 }
 }
 */

void dgBody::UpdateMatrix(dgFloat32 timestep, dgInt32 threadIndex) {
	if (m_matrixUpdate) {
		//		m_world->dgGetUserLock_();
		m_matrixUpdate(*this, m_matrix, threadIndex);
		//		m_world->dgReleasedUserLock_();
	}
	//	UpdateCollisionMatrix (timestep, threadIndex);
	if (m_world->m_cpu == dgSimdPresent) {
		UpdateCollisionMatrixSimd(timestep, threadIndex);
	} else {
		UpdateCollisionMatrix(timestep, threadIndex);
	}
}

void dgBody::IntegrateVelocity(dgFloat32 timestep) {

	//	IntegrateNotUpdate (timestep);

	m_globalCentreOfMass += m_veloc.Scale(timestep);
	while (((m_omega % m_omega) * timestep * timestep) > (DG_MAX_ANGLE_STEP * DG_MAX_ANGLE_STEP)) {
		m_omega = m_omega.Scale(dgFloat32(0.8f));
	}

	// this is correct
	dgFloat32 omegaMag2 = m_omega % m_omega;
	if (omegaMag2 > ((dgFloat32(0.0125f) * dgDEG2RAD) * (dgFloat32(0.0125f) * dgDEG2RAD))) {
		dgFloat32 invOmegaMag = dgRsqrt(omegaMag2);
		dgVector omegaAxis(m_omega.Scale(invOmegaMag));
		dgFloat32 omegaAngle = invOmegaMag * omegaMag2 * timestep;
		dgQuaternion rotation(omegaAxis, omegaAngle);
		m_rotation = m_rotation * rotation;
		m_rotation.Scale(dgRsqrt(m_rotation.DotProduct(m_rotation)));
		m_matrix = dgMatrix(m_rotation, m_matrix.m_posit);
	}

	m_matrix.m_posit = m_globalCentreOfMass - m_matrix.RotateVector(m_localCentreOfMass);

#ifdef _DEBUG
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			_ASSERTE(dgCheckFloat(m_matrix[i][j]));
		}
	}

	int j0 = 1;
	int j1 = 2;
	for (int i = 0; i < 3; i++) {
		dgFloat32 val;
		_ASSERTE(m_matrix[i][3] == 0.0f);
		val = m_matrix[i] % m_matrix[i];
		_ASSERTE(dgAbsf(val - 1.0f) < 1.0e-5f);
		dgVector tmp(m_matrix[j0] * m_matrix[j1]);
		val = tmp % m_matrix[i];
		_ASSERTE(dgAbsf(val - 1.0f) < 1.0e-5f);
		j0 = j1;
		j1 = i;
	}
#endif
}

void dgBody::CalculateContinueVelocity(dgFloat32 timestep, dgVector &veloc,
									   dgVector &omega) const {
	//	timestep = m_world->m_currTimestep;
	veloc = m_veloc + m_accel.Scale(timestep * m_invMass.m_w);

	dgVector localAlpha(m_matrix.UnrotateVector(m_alpha));
	dgVector alpha(m_matrix.RotateVector(localAlpha.CompProduct(m_invMass)));
	omega = m_omega + alpha.Scale(timestep);
}

void dgBody::CalculateContinueVelocitySimd(dgFloat32 timestep, dgVector &veloc,
										   dgVector &omega) const {
#ifdef DG_BUILD_SIMD_CODE
	//	timestep = m_world->m_currTimestep;
	veloc = m_veloc + m_accel.Scale(timestep * m_invMass.m_w);

	dgVector localAlpha(m_matrix.UnrotateVectorSimd(m_alpha));
	dgVector alpha(
		m_matrix.RotateVectorSimd(localAlpha.CompProductSimd(m_invMass)));
	omega = m_omega + alpha.Scale(timestep);
#endif
}

dgVector dgBody::GetTrajectory(const dgVector &velocParam,
							   const dgVector &omegaParam) const {
	_ASSERTE(0);
	return dgVector(0, 0, 0, 0);
	/*
	 dgFloat32 timestep;
	 dgFloat32 omegaMag2;
	 dgFloat32 omegaAngle;
	 dgFloat32 invOmegaMag;
	 dgVector rotdisp (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f));

	 timestep = m_world->m_currTimestep;
	 dgVector omega (m_matrix.UnrotateVector (omegaParam));
	 omegaMag2 = omega % omega;
	 if (omegaMag2 > ((dgFloat32 (0.0125f) * dgDEG2RAD) * (dgFloat32 (0.0125f) * dgDEG2RAD))) {
	 invOmegaMag = dgRsqrt (omegaMag2);
	 dgVector omegaAxis (omega.Scale (invOmegaMag));
	 omegaAngle = invOmegaMag * omegaMag2 * timestep;
	 dgQuaternion rotation (omegaAxis, omegaAngle);
	 //m_rotation = rotation * m_rotation;
	 //m_rotation.Scale(dgRsqrt (m_rotation.DotProduct (m_rotation)));
	 //m_matrix = dgMatrix (m_rotation, m_matrix.m_posit);
	 dgMatrix matrix (dgQuaternion (omegaAxis, omegaAngle), dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f)));
	 rotdisp = matrix.RotateVector (m_collision->GetOffsetMatrix().m_posit);
	 }

	 return velocParam.Scale (timestep) + rotdisp;
	 */
}

/*
 void dgBody::IntegrateNotUpdate (dgFloat32 timestep)
 {
 dgFloat32 omegaMag2;
 dgFloat32 omegaAngle;
 dgFloat32 invOmegaMag;

 m_accel = m_accel.Scale (m_invMass.m_w);
 dgVector localAlpha (m_matrix.UnrotateVector (m_alpha));
 m_alpha = m_matrix.RotateVector (localAlpha.CompProduct (m_invMass));

 m_veloc += m_accel.Scale (timestep);
 m_omega += m_alpha.Scale (timestep);

 m_globalCentreOfMass += m_veloc.Scale (timestep);
 while (((m_omega % m_omega) * timestep * timestep) > (DG_MAX_ANGLE_STEP * DG_MAX_ANGLE_STEP)) {
 m_omega = m_omega.Scale (dgFloat32 (0.8f));
 }

 // this is correct
 omegaMag2 = m_omega % m_omega;
 if (omegaMag2 > ((dgFloat32 (0.0125f) * dgDEG2RAD) * (dgFloat32 (0.0125f) * dgDEG2RAD))) {
 invOmegaMag = dgRsqrt (omegaMag2);
 dgVector omegaAxis (m_omega.Scale (invOmegaMag));
 omegaAngle = invOmegaMag * omegaMag2 * timestep;
 dgQuaternion rotation (omegaAxis, omegaAngle);
 m_rotation = m_rotation * rotation;
 m_rotation.Scale(dgRsqrt (m_rotation.DotProduct (m_rotation)));
 m_matrix = dgMatrix (m_rotation, m_matrix.m_posit);
 }

 m_matrix.m_posit = m_globalCentreOfMass - m_matrix.RotateVector(m_localCentreOfMass);
 }
 */

dgVector dgBody::CalculateInverseDynamicForce(const dgVector &desiredVeloc,
											  dgFloat32 timestep) const {
	// dgWord* world;
	dgFloat32 massAccel;
	massAccel = m_mass.m_w / timestep;

	if (m_world->m_solverMode) {
		if (m_masterNode->GetInfo().GetCount() >= 2) {
			massAccel *= (dgFloat32(2.0f) * dgFloat32(LINEAR_SOLVER_SUB_STEPS) / dgFloat32(LINEAR_SOLVER_SUB_STEPS + 1));
		}
	}
	return (desiredVeloc - m_veloc).Scale(massAccel);
}

dgConstraint *dgBody::GetFirstJoint() const {
	dgBodyMasterListRow::dgListNode *node;

	for (node = m_masterNode->GetInfo().GetFirst(); node; node = node->GetNext()) {
		dgConstraint *joint;
		joint = node->GetInfo().m_joint;
		if (joint->GetId() >= dgUnknownConstraintId) {
			return joint;
		}
	}
	return NULL;
}

dgConstraint *dgBody::GetNextJoint(dgConstraint *joint) const {
	dgBodyMasterListRow::dgListNode *node;

	node = joint->GetLink0();
	if (joint->GetBody0() != this) {
		node = joint->GetLink1();
	}

	if (node->GetInfo().m_joint == joint) {
		for (node = node->GetNext(); node; node = node->GetNext()) {
			dgConstraint *joint;
			joint = node->GetInfo().m_joint;
			if (joint->GetId() >= dgUnknownConstraintId) {
				return joint;
			}
		}
	}

	return NULL;
}

dgConstraint *dgBody::GetFirstContact() const {
	dgBodyMasterListRow::dgListNode *node;

	for (node = m_masterNode->GetInfo().GetFirst(); node; node = node->GetNext()) {
		dgConstraint *joint;
		joint = node->GetInfo().m_joint;
		if (joint->GetId() == dgContactConstraintId) {
			return joint;
		}
	}
	return NULL;
}

dgConstraint *dgBody::GetNextContact(dgConstraint *joint) const {
	dgBodyMasterListRow::dgListNode *node;

	node = joint->GetLink0();
	if (joint->GetBody0() != this) {
		node = joint->GetLink1();
	}

	if (node->GetInfo().m_joint == joint) {
		for (node = node->GetNext(); node; node = node->GetNext()) {
			dgConstraint *joint;
			joint = node->GetInfo().m_joint;
			if (joint->GetId() == dgContactConstraintId) {
				return joint;
			}
		}
	}

	return NULL;
}

void dgBody::SetFreeze(bool state) {
	if (state) {
		Freeze();
	} else {
		Unfreeze();
	}
}

void dgBody::Freeze() {
	if (m_invMass.m_w > dgFloat32(0.0f)) {
		if (!m_freeze) {
			m_freeze = true;
			for (dgBodyMasterListRow::dgListNode *node =
					 m_masterNode->GetInfo().GetFirst();
				 node; node = node->GetNext()) {
				dgBody *const body = node->GetInfo().m_bodyNode;
				body->Freeze();
			}
		}
	}
}

void dgBody::Unfreeze() {
	if (m_invMass.m_w > dgFloat32(0.0f)) {
		// note this is in observation (to prevent bodies from not going to sleep  inside triggers
		//		m_equilibrium = false;
		if (m_freeze) {
			m_freeze = false;
			for (dgBodyMasterListRow::dgListNode *node =
					 m_masterNode->GetInfo().GetFirst();
				 node; node = node->GetNext()) {
				dgBody *const body = node->GetInfo().m_bodyNode;
				body->Unfreeze();
			}
		}
	}
}

dgMatrix dgBody::CalculateInertiaMatrix() const {
	dgMatrix tmpMatrix;

	tmpMatrix[0][0] = m_mass[0] * m_matrix[0][0];
	tmpMatrix[0][1] = m_mass[1] * m_matrix[1][0];
	tmpMatrix[0][2] = m_mass[2] * m_matrix[2][0];
	tmpMatrix[0][3] = dgFloat32(0.0f);

	tmpMatrix[1][0] = m_mass[0] * m_matrix[0][1];
	tmpMatrix[1][1] = m_mass[1] * m_matrix[1][1];
	tmpMatrix[1][2] = m_mass[2] * m_matrix[2][1];
	tmpMatrix[1][3] = dgFloat32(0.0f);

	tmpMatrix[2][0] = m_mass[0] * m_matrix[0][2];
	tmpMatrix[2][1] = m_mass[1] * m_matrix[1][2];
	tmpMatrix[2][2] = m_mass[2] * m_matrix[2][2];
	tmpMatrix[2][3] = dgFloat32(0.0f);

	tmpMatrix[3][0] = dgFloat32(0.0f);
	tmpMatrix[3][1] = dgFloat32(0.0f);
	tmpMatrix[3][2] = dgFloat32(0.0f);
	tmpMatrix[3][3] = dgFloat32(1.0f);
	return tmpMatrix * m_matrix;
}

dgMatrix dgBody::CalculateInvInertiaMatrix() const {
	dgMatrix tmpMatrix;

	tmpMatrix[0][0] = m_invMass[0] * m_matrix[0][0];
	tmpMatrix[0][1] = m_invMass[1] * m_matrix[1][0];
	tmpMatrix[0][2] = m_invMass[2] * m_matrix[2][0];
	tmpMatrix[0][3] = dgFloat32(0.0f);

	tmpMatrix[1][0] = m_invMass[0] * m_matrix[0][1];
	tmpMatrix[1][1] = m_invMass[1] * m_matrix[1][1];
	tmpMatrix[1][2] = m_invMass[2] * m_matrix[2][1];
	tmpMatrix[1][3] = dgFloat32(0.0f);

	tmpMatrix[2][0] = m_invMass[0] * m_matrix[0][2];
	tmpMatrix[2][1] = m_invMass[1] * m_matrix[1][2];
	tmpMatrix[2][2] = m_invMass[2] * m_matrix[2][2];
	tmpMatrix[2][3] = dgFloat32(0.0f);

	tmpMatrix[3][0] = dgFloat32(0.0f);
	tmpMatrix[3][1] = dgFloat32(0.0f);
	tmpMatrix[3][2] = dgFloat32(0.0f);
	tmpMatrix[3][3] = dgFloat32(1.0f);
	return tmpMatrix * m_matrix;
}

void dgBody::AddImpulse(const dgVector &pointDeltaVeloc,
						const dgVector &pointPosit) {
	//	dgMatrix tmpMatrix;
	//	tmpMatrix[0][0] = m_invMass[0] * m_matrix[0][0];
	//	tmpMatrix[0][1] = m_invMass[1] * m_matrix[1][0];
	//	tmpMatrix[0][2] = m_invMass[2] * m_matrix[2][0];
	//	tmpMatrix[0][3] = dgFloat32 (0.0f);
	//	tmpMatrix[1][0] = m_invMass[0] * m_matrix[0][1];
	//	tmpMatrix[1][1] = m_invMass[1] * m_matrix[1][1];
	//	tmpMatrix[1][2] = m_invMass[2] * m_matrix[2][1];
	//	tmpMatrix[1][3] = dgFloat32 (0.0f);
	//	tmpMatrix[2][0] = m_invMass[0] * m_matrix[0][2];
	//	tmpMatrix[2][1] = m_invMass[1] * m_matrix[1][2];
	//	tmpMatrix[2][2] = m_invMass[2] * m_matrix[2][2];
	//	tmpMatrix[2][3] = dgFloat32 (0.0f);
	//	tmpMatrix[3][0] = dgFloat32 (0.0f);
	//	tmpMatrix[3][1] = dgFloat32 (0.0f);
	//	tmpMatrix[3][2] = dgFloat32 (0.0f);
	//	tmpMatrix[3][3] = dgFloat32 (1.0f);
	dgMatrix invInertia(CalculateInvInertiaMatrix());

	// get contact matrix
	dgMatrix tmp;
	//	dgVector globalContact (pointPosit - m_matrix.m_posit);
	dgVector globalContact(pointPosit - m_globalCentreOfMass);

	// globalContact[0] = dgFloat32 (0.0f);
	// globalContact[1] = dgFloat32 (0.0f);
	// globalContact[2] = dgFloat32 (0.0f);

	tmp[0][0] = dgFloat32(0.0f);
	tmp[0][1] = +globalContact[2];
	tmp[0][2] = -globalContact[1];
	tmp[0][3] = dgFloat32(0.0f);

	tmp[1][0] = -globalContact[2];
	tmp[1][1] = dgFloat32(0.0f);
	tmp[1][2] = +globalContact[0];
	tmp[1][3] = dgFloat32(0.0f);

	tmp[2][0] = +globalContact[1];
	tmp[2][1] = -globalContact[0];
	tmp[2][2] = dgFloat32(0.0f);
	tmp[2][3] = dgFloat32(0.0f);

	tmp[3][0] = dgFloat32(0.0f);
	tmp[3][1] = dgFloat32(0.0f);
	tmp[3][2] = dgFloat32(0.0f);
	tmp[3][3] = dgFloat32(1.0f);

	dgMatrix contactMatrix(tmp * invInertia * tmp);
	for (dgInt32 i = 0; i < 3; i++) {
		for (dgInt32 j = 0; j < 3; j++) {
			contactMatrix[i][j] *= -dgFloat32(1.0f);
		}
	}
	contactMatrix[0][0] += m_invMass.m_w;
	contactMatrix[1][1] += m_invMass.m_w;
	contactMatrix[2][2] += m_invMass.m_w;

	contactMatrix = contactMatrix.Symetric3by3Inverse();

	// change of momentum
	dgVector changeOfMomentum(contactMatrix.RotateVector(pointDeltaVeloc));

	dgVector dv(changeOfMomentum.Scale(m_invMass.m_w));
	dgVector dw(invInertia.RotateVector(globalContact * changeOfMomentum));

	m_veloc += dv;
	m_omega += dw;

	m_sleeping = false;
	m_equilibrium = false;
	Unfreeze();
}

void dgBody::ApplyImpulseArray(dgInt32 count, dgInt32 strideInBytes,
							   const dgFloat32 *const impulseArray, const dgFloat32 *const pointArray) {
	dgInt32 stride = strideInBytes / sizeof(dgFloat32);

	dgMatrix inertia(CalculateInertiaMatrix());

	dgVector impulse(m_veloc.Scale(m_mass.m_w));
	dgVector angularImpulse(inertia.RotateVector(m_omega));

	dgVector com(m_globalCentreOfMass);
	for (dgInt32 i = 0; i < count; i++) {
		dgInt32 index = i * stride;
		dgVector r(pointArray[index], pointArray[index + 1], pointArray[index + 2],
				   dgFloat32(0.0f));
		dgVector L(impulseArray[index], impulseArray[index + 1],
				   impulseArray[index + 2], dgFloat32(0.0f));
		dgVector Q((r - com) * L);

		impulse += L;
		angularImpulse += Q;
	}

	dgMatrix invInertia(CalculateInvInertiaMatrix());
	m_veloc = impulse.Scale(m_invMass.m_w);
	m_omega = invInertia.RotateVector(angularImpulse);

	m_sleeping = false;
	m_equilibrium = false;
	Unfreeze();
}

void dgBody::InvalidateCache() {
	m_sleeping = false;
	//	m_isInWorld = true;
	m_equilibrium = false;
	m_genericLRUMark = 0;
	m_sleepingCounter = 0;
	m_prevExternalForce = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
								   dgFloat32(0.0f), dgFloat32(0.0f));
	m_prevExternalTorque = dgVector(dgFloat32(0.0f), dgFloat32(0.0f),
									dgFloat32(0.0f), dgFloat32(0.0f));

	dgMatrix matrix(m_matrix);
	SetMatrixOriginAndRotation(matrix);
}
