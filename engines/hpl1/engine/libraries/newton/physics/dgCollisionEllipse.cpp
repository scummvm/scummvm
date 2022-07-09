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

#include "dgCollisionEllipse.h"
#include "dgBody.h"
#include "dgContact.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

dgCollisionEllipse::dgCollisionEllipse(dgMemoryAllocator *const allocator,
									   dgUnsigned32 signature, dgFloat32 rx, dgFloat32 ry, dgFloat32 rz,
									   const dgMatrix &offsetMatrix) : dgCollisionSphere(allocator, signature, dgFloat32(1.0f), offsetMatrix), m_scale(rx, ry, rz, dgFloat32(0.0f)), m_invScale(dgFloat32(1.0f) / rx,
																																																dgFloat32(1.0f) / ry, dgFloat32(1.0f) / rz, dgFloat32(0.0f)) {
	m_rtti |= dgCollisionEllipse_RTTI;
	m_collsionId = m_ellipseCollision;
}

dgCollisionEllipse::dgCollisionEllipse(dgWorld *const world,
									   dgDeserialize deserialization, void *const userData) : dgCollisionSphere(world, deserialization, userData) {
	dgVector size;

	m_rtti |= dgCollisionEllipse_RTTI;
	deserialization(userData, &m_scale, sizeof(dgVector));
	m_invScale.m_x = dgFloat32(1.0f) / m_scale.m_x;
	m_invScale.m_y = dgFloat32(1.0f) / m_scale.m_y;
	m_invScale.m_z = dgFloat32(1.0f) / m_scale.m_z;
	m_invScale.m_w = dgFloat32(0.0f);
}

dgCollisionEllipse::~dgCollisionEllipse() {
}

dgInt32 dgCollisionEllipse::CalculateSignature() const {
	dgUnsigned32 buffer[2 * sizeof(dgMatrix) / sizeof(dgInt32)];

	memset(buffer, 0, sizeof(buffer));
	buffer[0] = m_ellipseCollision;
	buffer[1] = Quantize(m_scale.m_x);
	buffer[2] = Quantize(m_scale.m_y);
	buffer[3] = Quantize(m_scale.m_z);
	memcpy(&buffer[4], &m_offset, sizeof(dgMatrix));
	return dgInt32(MakeCRC(buffer, sizeof(buffer)));
}

void dgCollisionEllipse::SetCollisionBBox(const dgVector &p0__,
										  const dgVector &p1__) {
	_ASSERTE(0);
}

void dgCollisionEllipse::CalcAABB(const dgMatrix &matrix, dgVector &p0,
								  dgVector &p1) const {
	dgMatrix mat(matrix);
	mat.m_front = mat.m_front.Scale(m_scale.m_x);
	mat.m_up = mat.m_up.Scale(m_scale.m_y);
	mat.m_right = mat.m_right.Scale(m_scale.m_z);
	dgCollisionConvex::CalcAABB(mat, p0, p1);
}

void dgCollisionEllipse::CalcAABBSimd(const dgMatrix &matrix, dgVector &p0,
									  dgVector &p1) const {
	dgMatrix mat(matrix);
	mat.m_front = mat.m_front.Scale(m_scale.m_x);
	mat.m_up = mat.m_up.Scale(m_scale.m_y);
	mat.m_right = mat.m_right.Scale(m_scale.m_z);
	dgCollisionConvex::CalcAABBSimd(mat, p0, p1);

#if 0
  dgVector xxx0;
  dgVector xxx1;
  CalcAABB (matrix, xxx0, xxx1);
  _ASSERTE (dgAbsf(xxx0.m_x - p0.m_x) < 1.0e-3f);
  _ASSERTE (dgAbsf(xxx0.m_y - p0.m_y) < 1.0e-3f);
  _ASSERTE (dgAbsf(xxx0.m_z - p0.m_z) < 1.0e-3f);
  _ASSERTE (dgAbsf(xxx1.m_x - p1.m_x) < 1.0e-3f);
  _ASSERTE (dgAbsf(xxx1.m_y - p1.m_y) < 1.0e-3f);
  _ASSERTE (dgAbsf(xxx1.m_z - p1.m_z) < 1.0e-3f);
#endif
}

dgVector dgCollisionEllipse::SupportVertex(const dgVector &dir) const {
	_ASSERTE((dir % dir) > dgFloat32(0.999f));
	dgVector dir1(dir.m_x * m_scale.m_x, dir.m_y * m_scale.m_y,
				  dir.m_z * m_scale.m_z, dgFloat32(0.0f));

	dir1 = dir1.Scale(dgRsqrt(dir1 % dir1));

	dgVector p(dgCollisionSphere::SupportVertex(dir1));
	return dgVector(p.m_x * m_scale.m_x, p.m_y * m_scale.m_y, p.m_z * m_scale.m_z,
					dgFloat32(0.0f));
}

dgVector dgCollisionEllipse::SupportVertexSimd(const dgVector &dir) const {
#ifdef DG_BUILD_SIMD_CODE
	_ASSERTE((dir % dir) > dgFloat32(0.999f));
	_ASSERTE((dgUnsigned64(&dir) & 0x0f) == 0);
	_ASSERTE((dgUnsigned64(&m_scale) & 0x0f) == 0);

	dgVector dir1;
	simd_type n;
	simd_type tmp;
	simd_type mag2;

	//	dir1 = dgVector (dir.m_x * m_scale.m_x, dir.m_y * m_scale.m_y, dir.m_z * m_scale.m_z, dgFloat32 (0.0f));
	n = simd_mul_v(*(simd_type *)&dir, *(simd_type *)&m_scale);

	//	dir1 = dir1.Scale (dgRsqrt (dir1 % dir1));
	mag2 = simd_mul_v(n, n);
	mag2 =
		simd_add_s(simd_add_v(mag2, simd_move_hl_v(mag2, mag2)), simd_permut_v(mag2, mag2, PURMUT_MASK(3, 3, 3, 1)));
	tmp = simd_rsqrt_s(mag2);
	mag2 =
		simd_mul_s(simd_mul_s(*(simd_type *)&m_nrh0p5, tmp), simd_mul_sub_s(*(simd_type *)&m_nrh3p0, simd_mul_s(mag2, tmp), tmp));
	(*(simd_type *)&dir1) =
		simd_mul_v(n, simd_permut_v(mag2, mag2, PURMUT_MASK(3, 0, 0, 0)));

	dgVector p(dgCollisionSphere::SupportVertexSimd(dir1));
	return dgVector(p.m_x * m_scale.m_x, p.m_y * m_scale.m_y, p.m_z * m_scale.m_z,
					dgFloat32(0.0f));

#else
	return dgVector(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f));
#endif
}

dgInt32 dgCollisionEllipse::CalculatePlaneIntersection(const dgVector &normal,
													   const dgVector &point, dgVector *const contactsOut) const {
	_ASSERTE((normal % normal) > dgFloat32(0.999f));
	//	contactsOut[0] = point;

	dgVector n(normal.m_x * m_scale.m_x, normal.m_y * m_scale.m_y,
			   normal.m_z * m_scale.m_z, dgFloat32(0.0f));
	n = n.Scale((normal % point) / (n % n));
	contactsOut[0] = dgVector(n.m_x * m_scale.m_x, n.m_y * m_scale.m_y,
							  n.m_z * m_scale.m_z, dgFloat32(0.0f));
	return 1;
}

dgInt32 dgCollisionEllipse::CalculatePlaneIntersectionSimd(
	const dgVector &normal, const dgVector &point,
	dgVector *const contactsOut) const {
#ifdef DG_BUILD_SIMD_CODE
	_ASSERTE((normal % normal) > dgFloat32(0.999f));
	dgVector n(normal.m_x * m_scale.m_x, normal.m_y * m_scale.m_y,
			   normal.m_z * m_scale.m_z, dgFloat32(0.0f));
	n = n.Scale((normal % point) / (n % n));
	contactsOut[0] = dgVector(n.m_x * m_scale.m_x, n.m_y * m_scale.m_y,
							  n.m_z * m_scale.m_z, dgFloat32(0.0f));
	return 1;

#else
	return 0;
#endif
}

void dgCollisionEllipse::DebugCollision(const dgMatrix &matrixPtr,
										OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgMatrix mat(GetOffsetMatrix() * matrixPtr);
	mat.m_front = mat.m_front.Scale(m_scale.m_x);
	mat.m_up = mat.m_up.Scale(m_scale.m_y);
	mat.m_right = mat.m_right.Scale(m_scale.m_z);
	mat = GetOffsetMatrix().Inverse() * mat;
	dgCollisionSphere::DebugCollision(mat, callback, userData);
}

dgFloat32 dgCollisionEllipse::RayCast(const dgVector &p0, const dgVector &p1,
									  dgContactPoint &contactOut, OnRayPrecastAction preFilter,
									  const dgBody *const body, void *const userData) const {
	dgFloat32 t;

	if (PREFILTER_RAYCAST(preFilter, body, this, userData)) {
		return dgFloat32(1.2f);
	}

	dgVector q0(p0.m_x * m_invScale.m_x, p0.m_y * m_invScale.m_y,
				p0.m_z * m_invScale.m_z, dgFloat32(0.0f));
	dgVector q1(p1.m_x * m_invScale.m_x, p1.m_y * m_invScale.m_y,
				p1.m_z * m_invScale.m_z, dgFloat32(0.0f));
	t = dgCollisionSphere::RayCast(q0, q1, contactOut, NULL, NULL, NULL);
	return t;
}

dgFloat32 dgCollisionEllipse::RayCastSimd(const dgVector &p0,
										  const dgVector &p1, dgContactPoint &contactOut,
										  OnRayPrecastAction preFilter, const dgBody *const body,
										  void *const userData) const {
	return RayCast(p0, p1, contactOut, preFilter, body, userData);
}

dgFloat32 dgCollisionEllipse::CalculateMassProperties(dgVector &inertia,
													  dgVector &crossInertia, dgVector &centerOfMass) const {
	return dgCollisionConvex::CalculateMassProperties(inertia, crossInertia,
													  centerOfMass);
}

void dgCollisionEllipse::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_sphere.m_r0 = m_scale.m_x;
	info->m_sphere.m_r1 = m_scale.m_y;
	info->m_sphere.m_r2 = m_scale.m_z;
	info->m_offsetMatrix = GetOffsetMatrix();
	//	strcpy (info->m_collisionType, "sphere");
	info->m_collisionType = m_sphereCollision;
}

void dgCollisionEllipse::Serialize(dgSerialize callback,
								   void *const userData) const {
	dgCollisionSphere::Serialize(callback, userData);
	callback(userData, &m_scale, sizeof(dgVector));
}
