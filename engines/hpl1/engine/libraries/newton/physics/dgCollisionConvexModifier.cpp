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

#include "dgCollisionConvexModifier.h"
#include "dgBody.h"
#include "dgCollisionEllipse.h"
#include "dgContact.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgCollisionConvexModifier::dgCollisionConvexModifier(
	dgCollisionConvex *convexChild, dgWorld *world) : dgCollisionConvex(world->GetAllocator(), 0, dgGetIdentityMatrix(),
																		m_convexCollisionModifier),
													  m_modifierMatrix(dgGetIdentityMatrix()), m_modifierInvMatrix(
																								   dgGetIdentityMatrix()) {
	m_world = world;
	m_det = dgFloat32(1.0f);
	m_convexCollision = convexChild;
	convexChild->AddRef();

	m_rtti |= dgCollisionConvexModifier_RTTI;
	// hack to make the collision work
	m_vertexCount = 1;
	SetUserData(convexChild->GetUserData());

	SetOffsetMatrix(m_convexCollision->GetOffsetMatrix());
}

dgCollisionConvexModifier::dgCollisionConvexModifier(dgWorld *const world,
													 dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	dgMatrix matrix;

	m_rtti |= dgCollisionConvexModifier_RTTI;

	m_world = world;
	deserialization(userData, &matrix, sizeof(dgMatrix));
	m_convexCollision = (dgCollisionConvex *)world->CreateFromSerialization(
		deserialization, userData);

	m_det = dgFloat32(1.0f);
	SetUserData(m_convexCollision->GetUserData());
	SetUserDataID(SetUserDataID());

	// hack to make the collision work
	m_vertexCount = 1;
	ModifierSetMatrix(matrix);
}

dgCollisionConvexModifier::~dgCollisionConvexModifier() {
	m_vertexCount = 0;
	m_world->ReleaseCollision(m_convexCollision);
}

dgMatrix dgCollisionConvexModifier::ModifierGetMatrix() const {
	return m_offset.Inverse() * m_modifierMatrix * m_offset;
}

dgFloat32 dgCollisionConvexModifier::GetBoxMinRadius() const {
	return m_convexCollision->GetBoxMinRadius();
}
dgFloat32 dgCollisionConvexModifier::GetBoxMaxRadius() const {
	return m_convexCollision->GetBoxMaxRadius();
}

void dgCollisionConvexModifier::ModifierSetMatrix(const dgMatrix &matrix) {

	dgInt32 i;
	dgInt32 j;
	dgInt32 k;
	dgFloat32 val;
	dgFloat32 mat[4][8];

	// copy the matrix into the modifier
	m_modifierMatrix = m_offset * matrix * m_offset.Inverse();

	// create a matrix matrix array set to identity
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			mat[i][j] = m_modifierMatrix[i][j];
			mat[i][j + 4] = dgFloat32(0.0f);
		}
		mat[i][i + 4] = dgFloat32(1.0f);
	}

	// calculate the inverse matrix of the modifier using full
	// Gauss Jordan pivoting method
	m_det = dgFloat32(1.0f);
	for (i = 0; i < 4; i++) {
		if (dgAbsf(mat[i][i]) < dgFloat32(1.0e-3f)) {
			for (j = i + 1; j < 4; j++) {
				if (dgAbsf(mat[j][i]) > dgFloat32(1.0e-3f)) {
					for (k = 0; k < 8; k++) {
						mat[i][k] += mat[j][k];
					}
					break;
				}
			}
		}
		val = dgFloat32(1.0f) / mat[i][i];
		m_det *= mat[i][i];
		for (j = 0; j < 8; j++) {
			mat[i][j] *= val;
		}
		mat[i][i] = dgFloat32(1.0f);

		for (j = 0; j < 4; j++) {
			if (j != i) {
				val = mat[j][i];
				for (k = 0; k < 8; k++) {
					mat[j][k] -= val * mat[i][k];
				}
			}
		}
	}

	// copy the inverted matrix into the modifier
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			m_modifierInvMatrix[i][j] = mat[i][j + 4];
		}
	}

#ifdef _DEBUG
	// check the the matrix is correctly inverted
	dgMatrix tmp(m_modifierInvMatrix * m_modifierMatrix);
	for (i = 0; i < 4; i++) {
		_ASSERTE(dgAbsf(tmp[i][i] - dgFloat32(1.0f)) < dgFloat32(1.0e-5f));
		for (j = i + 1; j < 4; j++) {
			_ASSERTE(dgAbsf(tmp[i][j]) < dgFloat32(1.0e-5f));
			_ASSERTE(dgAbsf(tmp[j][i]) < dgFloat32(1.0e-5f));
		}
	}
#endif
}

dgInt32 dgCollisionConvexModifier::CalculateSignature() const {
	return 0;
}

void dgCollisionConvexModifier::SetCollisionBBox(const dgVector &p0__,
												 const dgVector &p1__) {
	_ASSERTE(0);
}

void dgCollisionConvexModifier::CalcAABB(const dgMatrix &matrix, dgVector &p0,
										 dgVector &p1) const {
	dgMatrix trans(matrix.Transpose());

	for (dgInt32 i = 0; i < 3; i++) {
		p0[i] =
			matrix.m_posit[i] + matrix.RotateVector(SupportVertex(trans[i].Scale(-dgFloat32(1.0f))))[i] - dgFloat32(5.0e-2f);
		p1[i] = matrix.m_posit[i] + matrix.RotateVector(SupportVertex(trans[i]))[i] + dgFloat32(5.0e-2f);
	}
}

void dgCollisionConvexModifier::CalcAABBSimd(const dgMatrix &matrix,
											 dgVector &p0, dgVector &p1) const {
#ifdef DG_BUILD_SIMD_CODE
	dgInt32 i;
	dgMatrix trans(matrix.Transpose());
	for (i = 0; i < 3; i++) {
		p0[i] = matrix.m_posit[i] + matrix.RotateVectorSimd(SupportVertexSimd(trans[i].Scale(-dgFloat32(1.0f))))[i] - dgFloat32(5.0e-2f);
		p1[i] = matrix.m_posit[i] + matrix.RotateVectorSimd(SupportVertexSimd(trans[i]))[i] + dgFloat32(5.0e-2f);
	}
#endif
}

dgVector dgCollisionConvexModifier::SupportVertex(const dgVector &dir) const {
	_ASSERTE(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-2f));

	dgVector dir1(m_modifierMatrix.UnrotateVector(dir));
	dir1 = dir1.Scale(dgRsqrt(dir1 % dir1));

	_ASSERTE(dgAbsf(dir1 % dir1 - dgFloat32(1.0f)) < dgFloat32(1.0e-2f));
	return m_modifierMatrix.TransformVector(
		m_convexCollision->SupportVertex(dir1));
}

dgVector dgCollisionConvexModifier::SupportVertexSimd(const dgVector &dir) const {
#ifdef DG_BUILD_SIMD_CODE
	simd_type tmp1;
	simd_type tmp0;
	dgVector localDir;
	dgVector dir1(m_modifierMatrix.UnrotateVectorSimd(dir));

	_ASSERTE(dgAbsf(dir % dir - dgFloat32(1.0f)) < dgFloat32(1.0e-2f));
	//	dir1 = dir1.Scale (dgRsqrt (dir1 % dir1));
	tmp1 =
		simd_mul_v((simd_type &)dir1, simd_and_v((simd_type &)dir1, (simd_type &)m_triplexMask));
	tmp1 = simd_add_v(tmp1, simd_move_hl_v(tmp1, tmp1));
	tmp1 = simd_add_s(tmp1, simd_permut_v(tmp1, tmp1, PURMUT_MASK(0, 0, 0, 1)));

	tmp0 = simd_rsqrt_s(tmp1);
	tmp0 =
		simd_mul_s(simd_mul_s((simd_type &)m_nrh0p5, tmp0), simd_mul_sub_s((simd_type &)m_nrh3p0, simd_mul_s(tmp1, tmp0), tmp0));
	(simd_type &)localDir =
		simd_mul_v((simd_type &)dir1, simd_permut_v(tmp0, tmp0, PURMUT_MASK(3, 0, 0, 0)));

	_ASSERTE(
		dgAbsf(localDir % localDir - dgFloat32(1.0f)) < dgFloat32(1.0e-2f));
	return m_modifierMatrix.TransformVectorSimd(
		m_convexCollision->SupportVertexSimd(localDir));

#else
	return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
#endif
}

dgInt32 dgCollisionConvexModifier::CalculatePlaneIntersection(
	const dgVector &normal, const dgVector &point,
	dgVector *const contactsOut) const {
	dgInt32 i;
	dgInt32 count;

	dgVector n(m_modifierMatrix.UnrotateVector(normal));
	n = n.Scale(dgRsqrt(n % n));

	dgVector p(m_modifierInvMatrix.TransformVector(point));
	count = m_convexCollision->CalculatePlaneIntersection(n, p, contactsOut);
	for (i = 0; i < count; i++) {
		contactsOut[i] = m_modifierMatrix.TransformVector(contactsOut[i]);
	}

	return count;
}

dgInt32 dgCollisionConvexModifier::CalculatePlaneIntersectionSimd(
	const dgVector &normal, const dgVector &point,
	dgVector *const contactsOut) const {
	return CalculatePlaneIntersection(normal, point, contactsOut);
}

void dgCollisionConvexModifier::DebugCollision(const dgMatrix &matrixPtr,
											   OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgMatrix matrixInv(m_offset.Inverse());
	dgMatrix matrix(matrixInv * m_modifierMatrix * m_offset * matrixPtr);
	m_convexCollision->DebugCollision(matrix, callback, userData);
}

dgFloat32 dgCollisionConvexModifier::RayCast(const dgVector &p0,
											 const dgVector &p1, dgContactPoint &contactOut,
											 OnRayPrecastAction preFilter, const dgBody *const body,
											 void *const userData) const {
	dgFloat32 t;
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgVector q0(m_modifierInvMatrix.TransformVector(p0));
	dgVector q1(m_modifierInvMatrix.TransformVector(p1));

	t = m_convexCollision->RayCast(q0, q1, contactOut, NULL, NULL, NULL);
	if ((t >= dgFloat32(0.0f)) && (t <= dgFloat32(1.0f))) {
		dgVector n(m_modifierMatrix.RotateVector(contactOut.m_normal));
		contactOut.m_normal = n.Scale(dgRsqrt(n % n));
	}
	return t;
}

dgFloat32 dgCollisionConvexModifier::RayCastSimd(const dgVector &p0,
												 const dgVector &p1, dgContactPoint &contactOut,
												 OnRayPrecastAction preFilter, const dgBody *const body,
												 void *const userData) const {
	dgFloat32 t;
	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgVector q0(m_modifierInvMatrix.TransformVectorSimd(p0));
	dgVector q1(m_modifierInvMatrix.TransformVectorSimd(p1));

	t = m_convexCollision->RayCastSimd(q0, q1, contactOut, NULL, NULL, NULL);
	if ((t >= dgFloat32(0.0f)) && (t <= dgFloat32(1.0f))) {
		dgVector n(m_modifierMatrix.RotateVectorSimd(contactOut.m_normal));
		contactOut.m_normal = n.Scale(dgRsqrt(n % n));
	}
	return t;
}

dgFloat32 dgCollisionConvexModifier::GetVolume() const {
	return m_convexCollision->GetVolume() * m_det;
}

dgVector dgCollisionConvexModifier::CalculateVolumeIntegral(
	const dgMatrix &globalMatrix, GetBuoyancyPlane bouyancyPlane,
	void *const context) const {
	dgFloat32 volume;

	dgPlane plane(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
				  dgFloat32(-1.0e8f));
	if (bouyancyPlane) {
		dgPlane globalPlane;
		if (bouyancyPlane(GetUserData(), context, globalMatrix, globalPlane)) {
			plane = globalMatrix.UntransformPlane(globalPlane);

			dgVector point(plane.Scale(-plane.m_w));

			dgVector n(m_modifierMatrix.UnrotateVector(plane));
			n = n.Scale(dgRsqrt(n % n));

			dgVector p(m_modifierInvMatrix.TransformVector(point));

			plane = dgPlane(n, -(n % p));
		}
	}

	dgVector cg(m_convexCollision->CalculateVolumeIntegral(plane));

	volume = cg.m_w;
	if (volume > dgFloat32(1.0e-8f)) {
		cg = cg.Scale(dgFloat32(0.5f) / cg.m_w);
	}

	cg = globalMatrix.TransformVector(m_modifierMatrix.TransformVector(cg));
	cg.m_w = volume * m_det;

	return cg;
}

void dgCollisionConvexModifier::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);
	info->m_offsetMatrix = GetOffsetMatrix();
	info->m_convexModifierData.m_child = m_convexCollision;
	//	strcpy (info->m_collisionType, "modifier");
	info->m_collisionType = m_collsionId;
}

void dgCollisionConvexModifier::Serialize(dgSerialize callback,
										  void *const userData) const {
	SerializeLow(callback, userData);
	callback(userData, &m_modifierMatrix, sizeof(dgMatrix));
	m_world->Serialize(m_convexCollision, callback, userData);
}

bool dgCollisionConvexModifier::OOBBTest(const dgMatrix &matrix,
										 const dgCollisionConvex *const shape, void *const cacheOrder) const {
	return true;
}
