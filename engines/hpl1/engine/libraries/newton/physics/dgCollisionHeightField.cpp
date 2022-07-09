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

#include "dgCollisionHeightField.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


#define DG_HIGHTFILD_DATA_ID 0x45AF5E07

dgCollisionHeightField::dgCollisionHeightField(dgWorld *const world,
											   dgInt32 width, dgInt32 height, dgInt32 contructionMode,
											   const dgUnsigned16 *const elevationMap, const dgInt8 *const atributeMap,
											   dgFloat32 horizontalScale, dgFloat32 vertcalScale) : dgCollisionMesh(world->GetAllocator(), m_heightField) {
	dgFloat32 y0;
	dgFloat32 y1;

	m_userRayCastCallback = NULL;
	m_rtti |= dgCollisionHeightField_RTTI;
	m_width = width;
	m_height = height;
	m_diagonalMode = contructionMode;
	m_verticalScale = vertcalScale;
	m_horizontalScale = horizontalScale;

	m_elevationMap = (dgUnsigned16 *)dgMallocStack(
		m_width * m_height * sizeof(dgUnsigned16));
	memcpy(m_elevationMap, elevationMap,
		   m_width * m_height * sizeof(dgUnsigned16));

	m_atributeMap = (dgInt8 *)dgMallocStack(m_width * m_height * sizeof(dgInt8));
	memcpy(m_atributeMap, atributeMap, m_width * m_height * sizeof(dgInt8));

	y0 = dgFloat32(0.0f);
	y1 = dgFloat32(-dgFloat32(1.0e10f));
	for (dgInt32 i = 0; i < m_width * m_height; i++) {
		y1 = GetMax(y1, dgFloat32(m_elevationMap[i]));
	}

	m_minBox = dgVector(dgFloat32(dgFloat32(0.0f)), y0 * m_verticalScale,
						dgFloat32(dgFloat32(0.0f)), dgFloat32(dgFloat32(1.0f)));
	m_maxBox = dgVector(dgFloat32(m_width - 1) * m_horizontalScale,
						y1 * m_verticalScale, dgFloat32(m_height - 1) * m_horizontalScale,
						dgFloat32(dgFloat32(1.0f)));

	//	m_verticalScaleInv = dgFloat32 (1.0f) / m_verticalScale;
	m_horizontalScaleInv = dgFloat32(1.0f) / m_horizontalScale;

	dgTree<void *, unsigned>::dgTreeNode *nodeData = world->m_perInstanceData.Find(
		DG_HIGHTFILD_DATA_ID);
	if (!nodeData) {
		m_instanceData = (dgPerIntanceData *)dgMallocStack(
			sizeof(dgPerIntanceData));
		m_instanceData->m_refCount = 0;
		m_instanceData->m_world = world;
		for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
			m_instanceData->m_vertex[i] = NULL;
			m_instanceData->m_vertexCount[i] = 8 * 8;
			AllocateVertex(world, i);
		}
		nodeData = world->m_perInstanceData.Insert(m_instanceData,
												   DG_HIGHTFILD_DATA_ID);
	}
	m_instanceData = (dgPerIntanceData *)nodeData->GetInfo();

	m_instanceData->m_refCount++;
	SetCollisionBBox(m_minBox, m_maxBox);
}

dgCollisionHeightField::dgCollisionHeightField(dgWorld *const world,
											   dgDeserialize deserialization, void *const userData) : dgCollisionMesh(world, deserialization, userData) {
	m_rtti |= dgCollisionHeightField_RTTI;

	m_userRayCastCallback = NULL;
	deserialization(userData, &m_width, sizeof(dgInt32));
	deserialization(userData, &m_height, sizeof(dgInt32));
	deserialization(userData, &m_diagonalMode, sizeof(dgInt32));
	deserialization(userData, &m_verticalScale, sizeof(dgFloat32));
	deserialization(userData, &m_horizontalScale, sizeof(dgFloat32));
	deserialization(userData, &m_minBox.m_x, sizeof(dgVector));
	deserialization(userData, &m_maxBox.m_x, sizeof(dgVector));

	m_elevationMap = (dgUnsigned16 *)dgMallocStack(
		m_width * m_height * sizeof(dgUnsigned16));
	m_atributeMap = (dgInt8 *)dgMallocStack(m_width * m_height * sizeof(dgInt8));

	deserialization(userData, m_elevationMap,
					m_width * m_height * sizeof(dgUnsigned16));
	deserialization(userData, m_atributeMap, m_width * m_height * sizeof(dgInt8));

	//	m_verticalScaleInv = dgFloat32 (1.0f) / m_verticalScale;
	m_horizontalScaleInv = dgFloat32(1.0f) / m_horizontalScale;

	dgTree<void *, unsigned>::dgTreeNode *nodeData = world->m_perInstanceData.Find(
		DG_HIGHTFILD_DATA_ID);
	if (!nodeData) {
		m_instanceData = (dgPerIntanceData *)dgMallocStack(
			sizeof(dgPerIntanceData));
		m_instanceData->m_refCount = 0;
		m_instanceData->m_world = world;
		for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
			m_instanceData->m_vertex[i] = NULL;
			m_instanceData->m_vertexCount[i] = 8 * 8;
			AllocateVertex(world, i);
		}
		nodeData = world->m_perInstanceData.Insert(m_instanceData,
												   DG_HIGHTFILD_DATA_ID);
	}
	m_instanceData = (dgPerIntanceData *)nodeData->GetInfo();

	m_instanceData->m_refCount++;
	SetCollisionBBox(m_minBox, m_maxBox);
}

dgCollisionHeightField::~dgCollisionHeightField(void) {
	m_instanceData->m_refCount--;
	if (!m_instanceData->m_refCount) {
		dgWorld *world = m_instanceData->m_world;

		for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i++) {
			dgFreeStack(m_instanceData->m_vertex[i]);
		}
		dgFreeStack(m_instanceData);
		world->m_perInstanceData.Remove(DG_HIGHTFILD_DATA_ID);
	}
	dgFreeStack(m_elevationMap);
	dgFreeStack(m_atributeMap);
}

void dgCollisionHeightField::Serialize(dgSerialize callback,
									   void *const userData) const {
	//	dgVector size (m_size[0].Scale (dgFloat32 (2.0f)));

	SerializeLow(callback, userData);
	//	callback (userData, &size, sizeof (dgVector));

	callback(userData, &m_width, sizeof(dgInt32));
	callback(userData, &m_height, sizeof(dgInt32));
	callback(userData, &m_diagonalMode, sizeof(dgInt32));
	callback(userData, &m_verticalScale, sizeof(dgFloat32));
	callback(userData, &m_horizontalScale, sizeof(dgFloat32));
	callback(userData, &m_minBox.m_x, sizeof(dgVector));
	callback(userData, &m_maxBox.m_x, sizeof(dgVector));

	callback(userData, m_elevationMap, m_width * m_height * sizeof(dgInt16));
	callback(userData, m_atributeMap, m_width * m_height * sizeof(dgInt8));
}

void dgCollisionHeightField::SetCollisionRayCastCallback(
	dgCollisionHeightFieldRayCastCallback rayCastCallback) {
	m_userRayCastCallback = rayCastCallback;
}

void dgCollisionHeightField::AllocateVertex(dgWorld *const world,
											dgInt32 threadIndex) const {
	dgVector *vertex;
	vertex = (dgVector *)dgMallocStack(
		2 * m_instanceData->m_vertexCount[threadIndex] * sizeof(dgVector));
	if (m_instanceData->m_vertex[threadIndex]) {
		memcpy(vertex, m_instanceData->m_vertex[threadIndex],
			   m_instanceData->m_vertexCount[threadIndex] * sizeof(dgVector));
		dgFreeStack(m_instanceData->m_vertex[threadIndex]);
	}

	m_instanceData->m_vertexCount[threadIndex] *= 2;
	m_instanceData->m_vertex[threadIndex] = vertex;
}

void dgCollisionHeightField::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollision::GetCollisionInfo(info);

	info->m_offsetMatrix = GetOffsetMatrix();
	info->m_collisionType = m_collsionId;

	dgCollisionInfo::dgHeightMapCollisionData &data = info->m_heightFieldCollision;
	data.m_width = m_width;
	data.m_height = m_height;
	data.m_gridsDiagonals = m_diagonalMode;
	data.m_verticalScale = m_verticalScale;
	data.m_horizonalScale = m_horizontalScale;
	data.m_atributes = m_atributeMap;
	data.m_elevation = m_elevationMap;
}

void dgCollisionHeightField::CalculateMinExtend2d(const dgVector &p0,
												  const dgVector &p1, dgVector &boxP0, dgVector &boxP1) const {
	dgFloat32 x0 = GetMin(p0.m_x, p1.m_x) - dgFloat32(1.0e-3f);
	dgFloat32 z0 = GetMin(p0.m_z, p1.m_z) - dgFloat32(1.0e-3f);

	dgFloat32 x1 = GetMax(p0.m_x, p1.m_x) + dgFloat32(1.0e-3f);
	dgFloat32 z1 = GetMax(p0.m_z, p1.m_z) + dgFloat32(1.0e-3f);

	x0 = m_horizontalScale * dgFloor(x0 * m_horizontalScaleInv);
	z0 = m_horizontalScale * dgFloor(z0 * m_horizontalScaleInv);
	x1 = m_horizontalScale * dgFloor(x1 * m_horizontalScaleInv) + m_horizontalScale;
	z1 = m_horizontalScale * dgFloor(z1 * m_horizontalScaleInv) + m_horizontalScale;

	boxP0.m_x = GetMax(x0, m_minBox.m_x);
	boxP0.m_z = GetMax(z0, m_minBox.m_z);
	boxP0.m_y = -dgFloat32(1.0e10f);
	boxP0.m_w = dgFloat32(0.0f);

	boxP1.m_x = GetMin(x1, m_maxBox.m_x);
	boxP1.m_z = GetMin(z1, m_maxBox.m_z);
	boxP1.m_y = dgFloat32(1.0e10f);
	boxP1.m_w = dgFloat32(0.0f);
}

void dgCollisionHeightField::CalculateMinExtend3d(const dgVector &p0,
												  const dgVector &p1, dgVector &boxP0, dgVector &boxP1) const {
	_ASSERTE(p0.m_x <= p1.m_x);
	_ASSERTE(p0.m_y <= p1.m_y);
	_ASSERTE(p0.m_z <= p1.m_z);

	dgFloat32 x0 = m_horizontalScale * dgFloor((p0.m_x - dgFloat32(1.0e-3f)) * m_horizontalScaleInv);
	dgFloat32 z0 = m_horizontalScale * dgFloor((p0.m_z - dgFloat32(1.0e-3f)) * m_horizontalScaleInv);
	dgFloat32 x1 = m_horizontalScale * dgFloor((p1.m_x + dgFloat32(1.0e-3f)) * m_horizontalScaleInv) + m_horizontalScale;
	dgFloat32 z1 = m_horizontalScale * dgFloor((p1.m_z + dgFloat32(1.0e-3f)) * m_horizontalScaleInv) + m_horizontalScale;

	boxP0.m_x = GetMax(x0, m_minBox.m_x);
	boxP0.m_z = GetMax(z0, m_minBox.m_z);
	boxP0.m_y = p0.m_y - dgFloat32(1.0e-3f);
	boxP0.m_w = dgFloat32(0.0f);

	boxP1.m_x = GetMin(x1, m_maxBox.m_x);
	boxP1.m_z = GetMin(z1, m_maxBox.m_z);
	boxP1.m_y = p1.m_y + dgFloat32(1.0e-3f);
	boxP1.m_w = dgFloat32(0.0f);
}

dgFloat32 dgCollisionHeightField::RayCastCellSimd(const dgFastRayTest &ray,
												  dgInt32 xIndex0, dgInt32 zIndex0, dgVector &normalOut) const {
	dgFloat32 t;
	dgInt32 base;
	dgInt32 triangle[3];
	dgVector points[4];

	// get the 3d point at the corner of the cell

	if ((xIndex0 < 0) || (zIndex0 < 0) || (xIndex0 >= (m_width - 1)) || (zIndex0 >= (m_height - 1))) {
		return dgFloat32(1.2f);
	}

	base = zIndex0 * m_width + xIndex0;

	points[0 * 2 + 0] = dgVector((xIndex0 + 0) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base]) * m_verticalScale,
								 (zIndex0 + 0) * m_horizontalScale, dgFloat32(0.0f));
	points[0 * 2 + 1] = dgVector((xIndex0 + 1) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base + 1]) * m_verticalScale,
								 (zIndex0 + 0) * m_horizontalScale, dgFloat32(0.0f));
	points[1 * 2 + 1] = dgVector((xIndex0 + 1) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base + m_width + 1]) * m_verticalScale,
								 (zIndex0 + 1) * m_horizontalScale, dgFloat32(0.0f));
	points[1 * 2 + 0] = dgVector((xIndex0 + 0) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base + m_width + 0]) * m_verticalScale,
								 (zIndex0 + 1) * m_horizontalScale, dgFloat32(0.0f));

	t = dgFloat32(1.2f);
	if (!m_diagonalMode) {
		triangle[0] = 1;
		triangle[1] = 2;
		triangle[2] = 3;

		dgVector e10(points[2] - points[1]);
		dgVector e20(points[3] - points[1]);
		dgVector normal(e10 * e20);
		t = ray.PolygonIntersectSimd(normal, &points[0].m_x, sizeof(dgVector),
									 triangle, 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}

		triangle[0] = 1;
		triangle[1] = 0;
		triangle[2] = 2;

		dgVector e30(points[0] - points[1]);
		normal = e30 * e10;
		t = ray.PolygonIntersectSimd(normal, &points[0].m_x, sizeof(dgVector),
									 triangle, 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}

	} else {

		triangle[0] = 0;
		triangle[1] = 2;
		triangle[2] = 3;

		dgVector e10(points[2] - points[0]);
		dgVector e20(points[3] - points[0]);
		dgVector normal(e10 * e20);
		t = ray.PolygonIntersectSimd(normal, &points[0].m_x, sizeof(dgVector),
									 triangle, 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}

		triangle[0] = 0;
		triangle[1] = 3;
		triangle[2] = 1;

		dgVector e30(points[1] - points[0]);
		normal = e20 * e30;
		t = ray.PolygonIntersectSimd(normal, &points[0].m_x, sizeof(dgVector),
									 triangle, 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}
	}

	return t;
}

dgFloat32 dgCollisionHeightField::RayCastCell(const dgFastRayTest &ray,
											  dgInt32 xIndex0, dgInt32 zIndex0, dgVector &normalOut) const {
	dgFloat32 t;
	dgInt32 base;
	dgInt32 triangle[3];
	dgVector points[4];

	// get the 3d point at the corner of the cell
	if ((xIndex0 < 0) || (zIndex0 < 0) || (xIndex0 >= (m_width - 1)) || (zIndex0 >= (m_height - 1))) {
		return dgFloat32(1.2f);
	}

	base = zIndex0 * m_width + xIndex0;

	points[0 * 2 + 0] = dgVector((xIndex0 + 0) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base]) * m_verticalScale,
								 (zIndex0 + 0) * m_horizontalScale, dgFloat32(0.0f));
	points[0 * 2 + 1] = dgVector((xIndex0 + 1) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base + 1]) * m_verticalScale,
								 (zIndex0 + 0) * m_horizontalScale, dgFloat32(0.0f));
	points[1 * 2 + 1] = dgVector((xIndex0 + 1) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base + m_width + 1]) * m_verticalScale,
								 (zIndex0 + 1) * m_horizontalScale, dgFloat32(0.0f));
	points[1 * 2 + 0] = dgVector((xIndex0 + 0) * m_horizontalScale,
								 dgFloat32(m_elevationMap[base + m_width + 0]) * m_verticalScale,
								 (zIndex0 + 1) * m_horizontalScale, dgFloat32(0.0f));

	t = dgFloat32(1.2f);
	if (!m_diagonalMode) {
		triangle[0] = 1;
		triangle[1] = 2;
		triangle[2] = 3;

		dgVector e10(points[2] - points[1]);
		dgVector e20(points[3] - points[1]);
		dgVector normal(e10 * e20);
		t = ray.PolygonIntersect(normal, &points[0].m_x, sizeof(dgVector), triangle,
								 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}

		triangle[0] = 1;
		triangle[1] = 0;
		triangle[2] = 2;

		dgVector e30(points[0] - points[1]);
		normal = e30 * e10;
		t = ray.PolygonIntersect(normal, &points[0].m_x, sizeof(dgVector), triangle,
								 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}

	} else {

		triangle[0] = 0;
		triangle[1] = 2;
		triangle[2] = 3;

		dgVector e10(points[2] - points[0]);
		dgVector e20(points[3] - points[0]);
		dgVector normal(e10 * e20);
		t = ray.PolygonIntersect(normal, &points[0].m_x, sizeof(dgVector), triangle,
								 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}

		triangle[0] = 0;
		triangle[1] = 3;
		triangle[2] = 1;

		dgVector e30(points[1] - points[0]);
		normal = e20 * e30;
		t = ray.PolygonIntersect(normal, &points[0].m_x, sizeof(dgVector), triangle,
								 3);
		if (t < dgFloat32(1.0f)) {
			normalOut = normal;
			return t;
		}
	}
	return t;
}

dgFloat32 dgCollisionHeightField::RayCastSimd(const dgVector &q0,
											  const dgVector &q1, dgContactPoint &contactOut,
											  OnRayPrecastAction preFilter, const dgBody *const body,
											  void *const userData) const {
	dgVector boxP0;
	dgVector boxP1;

	// set the debug line counter to zero
	//	debugRayCast = 0;

	// calculate the ray bounding box
	CalculateMinExtend2d(q0, q1, boxP0, boxP1);

	dgVector dq(q1 - q0);
	dgVector padding(
		dq.Scale(
			m_horizontalScale * dgFloat32(10.0f) / (dgSqrt(dq % dq) + dgFloat32(1.0e-6f))));

	// make sure the line segment crosses the original segment box
	dgVector p0(q0 - padding);
	dgVector p1(q1 + padding);

	// clip the line against the bounding box
	if (dgRayBoxClip(p0, p1, boxP0, boxP1)) {
		dgVector dp(p1 - p0);
		dgVector normalOut(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						   dgFloat32(0.0f));

		dgFloat32 scale = m_horizontalScale;
		dgFloat32 invScale = m_horizontalScaleInv;
		dgInt32 ix0 = dgFastInt(p0.m_x * invScale);
		dgInt32 iz0 = dgFastInt(p0.m_z * invScale);

		// implement a 3ddda line algorithm
		dgInt32 xInc;
		dgFloat32 tx;
		dgFloat32 stepX;
		if (dp.m_x > dgFloat32(0.0f)) {
			xInc = 1;
			dgFloat32 val = dgFloat32(1.0f) / dp.m_x;
			stepX = scale * val;
			tx = (scale * (ix0 + dgFloat32(1.0f)) - p0.m_x) * val;
		} else if (dp.m_x < dgFloat32(0.0f)) {
			xInc = -1;
			dgFloat32 val = -dgFloat32(1.0f) / dp.m_x;
			stepX = scale * val;
			tx = -(scale * ix0 - p0.m_x) * val;
		} else {
			xInc = 0;
			stepX = dgFloat32(0.0f);
			tx = dgFloat32(1.0e10f);
		}

		dgInt32 zInc;
		dgFloat32 stepZ;
		dgFloat32 tz;
		if (dp.m_z > dgFloat32(0.0f)) {
			zInc = 1;
			dgFloat32 val = dgFloat32(1.0f) / dp.m_z;
			stepZ = scale * val;
			tz = (scale * (iz0 + dgFloat32(1.0f)) - p0.m_z) * val;
		} else if (dp.m_z < dgFloat32(0.0f)) {
			zInc = -1;
			dgFloat32 val = -dgFloat32(1.0f) / dp.m_z;
			stepZ = scale * val;
			tz = -(scale * iz0 - p0.m_z) * val;
		} else {
			zInc = 0;
			stepZ = dgFloat32(0.0f);
			tz = dgFloat32(1.0e10f);
		}

		dgFloat32 txAcc = tx;
		dgFloat32 tzAcc = tz;
		dgInt32 xIndex0 = ix0;
		dgInt32 zIndex0 = iz0;
		dgFastRayTest ray(q0, q1);

		// for each cell touched by the line
		do {
			dgFloat32 t = RayCastCellSimd(ray, xIndex0, zIndex0, normalOut);
			if (t < dgFloat32(1.0f)) {
				// bail out at the first intersection and copy the data into the descriptor
				contactOut.m_normal = normalOut.Scale(
					dgFloat32(1.0f) / dgSqrt(normalOut % normalOut));
				contactOut.m_userId = m_atributeMap[zIndex0 * m_width + xIndex0];

				if (m_userRayCastCallback) {
					dgVector normal(
						body->GetCollisionMatrix().RotateVector(contactOut.m_normal));
					m_userRayCastCallback(body, this, t, xIndex0, zIndex0, &normal,
										  dgInt32(contactOut.m_userId), userData);
				}

				return t;
			}

			if (txAcc < tzAcc) {
				xIndex0 += xInc;
				tx = txAcc;
				txAcc += stepX;
			} else {
				zIndex0 += zInc;
				tz = txAcc;
				tzAcc += stepZ;
			}
		} while ((tx <= dgFloat32(1.0f)) || (tz <= dgFloat32(1.0f)));
	}

	// if no cell was hit, return a large value
	return dgFloat32(1.2f);
}

dgFloat32 dgCollisionHeightField::RayCast(const dgVector &q0,
										  const dgVector &q1, dgContactPoint &contactOut,
										  OnRayPrecastAction preFilter, const dgBody *const body,
										  void *const userData) const {
	dgVector boxP0;
	dgVector boxP1;

	// set the debug line counter to zero
	//	debugRayCast = 0;

	// calculate the ray bounding box
	CalculateMinExtend2d(q0, q1, boxP0, boxP1);

	//	dgVector dq (q1 - q0);
	//	dgVector padding (dq.Scale (m_horizontalScale * dgFloat32 (10.0f) / (dgSqrt (dq % dq) + dgFloat32 (1.0e-6f))));

	// make sure the line segment crosses the original segment box
	//	dgVector p0 (q0 - padding);
	//	dgVector p1 (q1 + padding);

	dgVector p0(q0);
	dgVector p1(q1);

	// clip the line against the bounding box
	if (dgRayBoxClip(p0, p1, boxP0, boxP1)) {
		dgVector dp(p1 - p0);
		dgVector normalOut(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
						   dgFloat32(0.0f));

		dgFloat32 scale = m_horizontalScale;
		dgFloat32 invScale = m_horizontalScaleInv;
		dgInt32 ix0 = dgFastInt(p0.m_x * invScale);
		dgInt32 iz0 = dgFastInt(p0.m_z * invScale);

		// implement a 3ddda line algorithm
		dgInt32 xInc;
		dgFloat32 tx;
		dgFloat32 stepX;
		if (dp.m_x > dgFloat32(0.0f)) {
			xInc = 1;
			dgFloat32 val = dgFloat32(1.0f) / dp.m_x;
			stepX = scale * val;
			tx = (scale * (ix0 + dgFloat32(1.0f)) - p0.m_x) * val;
		} else if (dp.m_x < dgFloat32(0.0f)) {
			xInc = -1;
			dgFloat32 val = -dgFloat32(1.0f) / dp.m_x;
			stepX = scale * val;
			tx = -(scale * ix0 - p0.m_x) * val;
		} else {
			xInc = 0;
			stepX = dgFloat32(0.0f);
			tx = dgFloat32(1.0e10f);
		}

		dgInt32 zInc;
		dgFloat32 stepZ;
		dgFloat32 tz;
		if (dp.m_z > dgFloat32(0.0f)) {
			zInc = 1;
			dgFloat32 val = dgFloat32(1.0f) / dp.m_z;
			stepZ = scale * val;
			tz = (scale * (iz0 + dgFloat32(1.0f)) - p0.m_z) * val;
		} else if (dp.m_z < dgFloat32(0.0f)) {
			zInc = -1;
			dgFloat32 val = -dgFloat32(1.0f) / dp.m_z;
			stepZ = scale * val;
			tz = -(scale * iz0 - p0.m_z) * val;
		} else {
			zInc = 0;
			stepZ = dgFloat32(0.0f);
			tz = dgFloat32(1.0e10f);
		}

		dgFloat32 txAcc = tx;
		dgFloat32 tzAcc = tz;
		dgInt32 xIndex0 = ix0;
		dgInt32 zIndex0 = iz0;
		dgFastRayTest ray(q0, q1);

		// for each cell touched by the line
		do {
			dgFloat32 t = RayCastCell(ray, xIndex0, zIndex0, normalOut);
			if (t < dgFloat32(1.0f)) {
				// bail out at the first intersection and copy the data into the descriptor
				contactOut.m_normal = normalOut.Scale(
					dgFloat32(1.0f) / dgSqrt(normalOut % normalOut));
				contactOut.m_userId = m_atributeMap[zIndex0 * m_width + xIndex0];

				if (m_userRayCastCallback) {
					dgVector normal(
						body->GetCollisionMatrix().RotateVector(contactOut.m_normal));
					m_userRayCastCallback(body, this, t, xIndex0, zIndex0, &normal,
										  dgInt32(contactOut.m_userId), userData);
				}

				return t;
			}

			if (txAcc < tzAcc) {
				xIndex0 += xInc;
				tx = txAcc;
				txAcc += stepX;
			} else {
				zIndex0 += zInc;
				tz = txAcc;
				tzAcc += stepZ;
			}
		} while ((tx <= dgFloat32(1.0f)) || (tz <= dgFloat32(1.0f)));
	}

	// if no cell was hit, return a large value
	return dgFloat32(1.2f);
}

void dgCollisionHeightField::GetVertexListIndexList(const dgVector &p0,
													const dgVector &p1, dgGetVertexListIndexList &data) const {
	_ASSERTE(0);
	data.m_vertexCount = 0;
}

struct dgCollisionHeightFieldShowPolyContext {
	dgMatrix m_matrix;
	void *m_userData;
	OnDebugCollisionMeshCallback m_callback;
};

void dgCollisionHeightField::DebugCollision(const dgMatrix &matrix,
											OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt32 base;
	dgVector points[4];

	base = 0;
	for (dgInt32 z = 0; z < m_height - 1; z++) {
		points[0 * 2 + 0] = matrix.TransformVector(
			dgVector((0 + 0) * m_horizontalScale,
					 dgFloat32(m_elevationMap[base + 0]) * m_verticalScale,
					 (z + 0) * m_horizontalScale, dgFloat32(0.0f)));
		points[1 * 2 + 0] = matrix.TransformVector(
			dgVector((0 + 0) * m_horizontalScale,
					 dgFloat32(m_elevationMap[base + 0 + m_width + 0]) * m_verticalScale,
					 (z + 1) * m_horizontalScale, dgFloat32(0.0f)));

		for (dgInt32 x = 0; x < m_width - 1; x++) {
			dgTriplex triangle[3];
			points[0 * 2 + 1] = matrix.TransformVector(
				dgVector((x + 1) * m_horizontalScale,
						 dgFloat32(m_elevationMap[base + x + 1]) * m_verticalScale,
						 (z + 0) * m_horizontalScale, dgFloat32(0.0f)));
			points[1 * 2 + 1] = matrix.TransformVector(
				dgVector(
					(x + 1) * m_horizontalScale,
					dgFloat32(m_elevationMap[base + x + m_width + 1]) * m_verticalScale, (z + 1) * m_horizontalScale,
					dgFloat32(0.0f)));

			if (m_diagonalMode) {
				triangle[0].m_x = points[0].m_x;
				triangle[0].m_y = points[0].m_y;
				triangle[0].m_z = points[0].m_z;

				triangle[1].m_x = points[2].m_x;
				triangle[1].m_y = points[2].m_y;
				triangle[1].m_z = points[2].m_z;

				triangle[2].m_x = points[1].m_x;
				triangle[2].m_y = points[1].m_y;
				triangle[2].m_z = points[1].m_z;
				callback(userData, 3, &triangle[0].m_x, m_atributeMap[base]);

				triangle[0].m_x = points[1].m_x;
				triangle[0].m_y = points[1].m_y;
				triangle[0].m_z = points[1].m_z;

				triangle[1].m_x = points[2].m_x;
				triangle[1].m_y = points[2].m_y;
				triangle[1].m_z = points[2].m_z;

				triangle[2].m_x = points[3].m_x;
				triangle[2].m_y = points[3].m_y;
				triangle[2].m_z = points[3].m_z;
				callback(userData, 3, &triangle[0].m_x, m_atributeMap[base]);

			} else {
				triangle[0].m_x = points[0].m_x;
				triangle[0].m_y = points[0].m_y;
				triangle[0].m_z = points[0].m_z;

				triangle[1].m_x = points[2].m_x;
				triangle[1].m_y = points[2].m_y;
				triangle[1].m_z = points[2].m_z;

				triangle[2].m_x = points[3].m_x;
				triangle[2].m_y = points[3].m_y;
				triangle[2].m_z = points[3].m_z;
				callback(userData, 3, &triangle[0].m_x, m_atributeMap[base]);

				triangle[0].m_x = points[0].m_x;
				triangle[0].m_y = points[0].m_y;
				triangle[0].m_z = points[0].m_z;

				triangle[1].m_x = points[3].m_x;
				triangle[1].m_y = points[3].m_y;
				triangle[1].m_z = points[3].m_z;

				triangle[2].m_x = points[1].m_x;
				triangle[2].m_y = points[1].m_y;
				triangle[2].m_z = points[1].m_z;
				callback(userData, 3, &triangle[0].m_x, m_atributeMap[base]);
			}
			points[0 * 2 + 0] = points[0 * 2 + 1];
			points[1 * 2 + 0] = points[1 * 2 + 1];
		}
		base += m_width;
	}
}

void dgCollisionHeightField::GetLocalAABB(const dgVector &p0,
										  const dgVector &p1, dgVector &boxP0, dgVector &boxP1) const {
	dgInt32 x0;
	dgInt32 x1;
	dgInt32 z0;
	dgInt32 z1;
	dgInt32 base;
	dgInt32 minHeight;
	dgInt32 maxHeight;

	// the user data is the pointer to the collision geometry
	CalculateMinExtend3d(p0, p1, boxP0, boxP1);

	x0 = dgFastInt(boxP0.m_x * m_horizontalScaleInv);
	x1 = dgFastInt(boxP1.m_x * m_horizontalScaleInv);
	z0 = dgFastInt(boxP0.m_z * m_horizontalScaleInv);
	z1 = dgFastInt(boxP1.m_z * m_horizontalScaleInv);

	minHeight = 0x7fffffff;
	maxHeight = -0x7fffffff;
	base = z0 * m_width;
	for (dgInt32 z = z0; z <= z1; z++) {
		for (dgInt32 x = x0; x <= x1; x++) {
			dgInt32 high;
			high = m_elevationMap[base + x];
			if (high < minHeight) {
				minHeight = high;
			}
			if (high > maxHeight) {
				maxHeight = high;
			}
		}
		base += m_width;
	}

	boxP0.m_y = m_verticalScale * minHeight;
	boxP1.m_y = m_verticalScale * maxHeight;
}

void dgCollisionHeightField::GetCollidingFacesSimd(
	dgPolygonMeshDesc *const data) const {
	GetCollidingFaces(data);
}

void dgCollisionHeightField::GetCollidingFaces(
	dgPolygonMeshDesc *const data) const {
	dgInt32 x0;
	dgInt32 x1;
	dgInt32 z0;
	dgInt32 z1;
	dgInt32 base;
	dgInt32 vertexIndex;
	dgVector boxP0;
	dgVector boxP1;
	dgWorld *world;

	// the user data is the pointer to the collision geometry
	CalculateMinExtend3d(data->m_boxP0, data->m_boxP1, boxP0, boxP1);

	world = data->m_objBody->GetWorld();
	x0 = dgFastInt(boxP0.m_x * m_horizontalScaleInv);
	x1 = dgFastInt(boxP1.m_x * m_horizontalScaleInv);
	z0 = dgFastInt(boxP0.m_z * m_horizontalScaleInv);
	z1 = dgFastInt(boxP1.m_z * m_horizontalScaleInv);

	dgInt32 minHeight;
	dgInt32 maxHeight;

	minHeight = 0x7fffffff;
	maxHeight = -0x7fffffff;

	base = z0 * m_width;
	for (dgInt32 z = z0; z <= z1; z++) {
		for (dgInt32 x = x0; x <= x1; x++) {
			dgInt32 high;
			high = m_elevationMap[base + x];
			if (high < minHeight) {
				minHeight = high;
			}
			if (high > maxHeight) {
				maxHeight = high;
			}
		}
		base += m_width;
	}

	if (!(((m_verticalScale * maxHeight) < boxP0.m_y) || ((m_verticalScale * minHeight) > boxP1.m_y))) {
		dgInt32 step;
		dgInt32 index;
		dgInt32 faceCount;
		dgInt32 normalBase;
		dgInt32 normalIndex;

		// scan the vertices's intersected by the box extend
		base = (z1 - z0 + 1) * (x1 - x0 + 1) + 2 * (z1 - z0) * (x1 - x0);
		while (base > m_instanceData->m_vertexCount[data->m_threadNumber]) {
			AllocateVertex(world, data->m_threadNumber);
		}

		vertexIndex = 0;
		base = z0 * m_width;
		dgVector *const vertex = m_instanceData->m_vertex[data->m_threadNumber];

		for (dgInt32 z = z0; z <= z1; z++) {
			for (dgInt32 x = x0; x <= x1; x++) {
				vertex[vertexIndex] = dgVector(m_horizontalScale * x,
											   m_verticalScale * dgFloat32(m_elevationMap[base + x]),
											   m_horizontalScale * z, dgFloat32(0.0f));
				vertexIndex++;
				_ASSERTE(
					vertexIndex <= m_instanceData->m_vertexCount[data->m_threadNumber]);
			}
			base += m_width;
		}

		normalBase = vertexIndex;
		index = 0;
		faceCount = 0;
		vertexIndex = 0;
		normalIndex = 0;
		step = x1 - x0 + 1;

		dgInt32 *const indices = data->m_globalFaceVertexIndex;
		dgInt32 *const attributes = data->m_globalUserAttribute;
		dgInt32 *const faceIndexCount = data->m_globalFaceIndexCount;
		dgInt32 *const normalIndexCount = data->m_globalFaceNormalIndex;
		dgInt32 *const faceAdjencentEdgeNormal = data->m_globalAdjencentEdgeNormal;
		dgFloat32 *const facefaceMaxSize = data->m_globalFaceMaxSize;
		dgFloat32 faceSize = GetMax(m_horizontalScale * dgFloat32(2.0f),
									dgFloat32(64.0f));

		if (!m_diagonalMode) {
			for (dgInt32 z = z0; z < z1; z++) {
				dgInt32 zStep;
				zStep = z * m_width;

				for (dgInt32 x = x0; x < x1; x++) {
					dgInt32 i0;
					dgInt32 i1;
					dgInt32 i2;
					dgInt32 i3;

					i0 = vertexIndex;
					i1 = vertexIndex + step;
					i2 = vertexIndex + 1;
					i3 = vertexIndex + step + 1;

					faceIndexCount[faceCount] = 3;
					facefaceMaxSize[faceCount] = faceSize;
					attributes[faceCount] = m_atributeMap[zStep + x];
					indices[index + 0] = i0;
					indices[index + 1] = i1;
					indices[index + 2] = i2;
					index += 3;
					faceCount++;

					faceIndexCount[faceCount] = 3;
					attributes[faceCount] = m_atributeMap[zStep + x];
					facefaceMaxSize[faceCount] = faceSize;
					indices[index + 0] = i2;
					indices[index + 1] = i1;
					indices[index + 2] = i3;
					index += 3;
					faceCount++;
					vertexIndex++;

					// calculate the the normal
					dgVector e0(vertex[i0] - vertex[i2]);
					dgVector e1(vertex[i1] - vertex[i2]);
					dgVector e2(vertex[i3] - vertex[i2]);
					dgVector n0(e0 * e1);
					dgVector n1(e1 * e2);

					// normalBase
					vertex[normalBase] = n0.Scale(dgRsqrt(n0 % n0));
					normalIndexCount[normalIndex] = normalBase;
					normalIndex++;
					normalBase++;

					vertex[normalBase] = n1.Scale(dgRsqrt(n1 % n1));
					normalIndexCount[normalIndex] = normalBase;
					normalIndex++;
					normalBase++;
				}
				vertexIndex++;
			}

			base = 0;
			index = 0;
			step = x1 - x0;
			for (dgInt32 z = z0; z < z1; z++) {
				dgInt32 z0Flag;
				dgInt32 z1Flag;

				z0Flag = ((z - z0 - 1) >> 31);
				z1Flag = ((z1 - z - 2) >> 31);
				for (dgInt32 x = x0; x < x1; x++) {
					dgInt32 xA0;
					dgInt32 xA1;
					dgInt32 zA0;
					dgInt32 zA1;
					dgInt32 zxA;
					dgInt32 x0Flag;
					dgInt32 x1Flag;
					dgFloat32 side;
					dgFloat32 diagSide;

					x0Flag = ((x - x0 - 1) >> 31);
					x1Flag = ((x1 - x - 2) >> 31);

					const dgVector &point = vertex[indices[base + 1]];
					const dgVector &n = vertex[normalIndexCount[index * 2]];

					xA0 = ((~x0Flag) & (index * 2 - 1)) | (x0Flag & ((index - x + x0) * 2));
					side = n % (vertex[indices[xA0 * 3 + 1]] - point);
					faceAdjencentEdgeNormal[base + 0] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[xA0] : -1;

					zxA = index * 2 + 1;
					diagSide = n % (vertex[indices[xA0 * 3 + 2]] - point);
					faceAdjencentEdgeNormal[base + 1] =
						(diagSide < dgFloat32(-1.0e-5f)) ? normalIndexCount[zxA] : -1;

					zA0 = ((~z0Flag) & ((index - step) * 2 + 1)) | (z0Flag & (index * 2));
					side = n % (vertex[indices[zA0 * 3]] - point);
					faceAdjencentEdgeNormal[base + 2] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[zA0] : -1;

					const dgVector &n1 = vertex[normalIndexCount[index * 2 + 1]];
					xA1 = ((~x1Flag) & (index * 2 + 2)) | (x1Flag & (index * 2 + 1));
					side = n1 % (vertex[indices[xA1 * 3 + 2]] - point);
					faceAdjencentEdgeNormal[base + 5] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[xA1] : -1;

					zxA = index * 2;
					faceAdjencentEdgeNormal[base + 3] =
						(diagSide < dgFloat32(-1.0e-5f)) ? normalIndexCount[zxA] : -1;
					;

					zA1 = ((~z1Flag) & ((index + step) * 2)) | (z1Flag & (index * 2 + 1));
					side = n % (vertex[indices[zA1 * 3 + 1]] - point);
					faceAdjencentEdgeNormal[base + 4] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[zA1] : -1;
					index++;
					base += 6;
				}
			}
		} else {

			for (dgInt32 z = z0; z < z1; z++) {
				dgInt32 zStep;
				zStep = z * m_width;
				for (dgInt32 x = x0; x < x1; x++) {

					dgInt32 i0;
					dgInt32 i1;
					dgInt32 i2;
					dgInt32 i3;

					i0 = vertexIndex;
					i1 = vertexIndex + step + 1;
					i2 = vertexIndex + 1;
					i3 = vertexIndex + step;

					faceIndexCount[faceCount] = 3;
					facefaceMaxSize[faceCount] = faceSize;
					attributes[faceCount] = m_atributeMap[zStep + x];
					indices[index + 0] = i0;
					indices[index + 1] = i1;
					indices[index + 2] = i2;
					index += 3;
					faceCount++;

					faceIndexCount[faceCount] = 3;
					facefaceMaxSize[faceCount] = faceSize;
					attributes[faceCount] = m_atributeMap[zStep + x];
					indices[index + 0] = i0;
					indices[index + 1] = i3;
					indices[index + 2] = i1;
					index += 3;
					faceCount++;
					vertexIndex++;

					// calculate the the normal
					dgVector e0(vertex[i3] - vertex[i0]);
					dgVector e1(vertex[i1] - vertex[i0]);
					dgVector e2(vertex[i2] - vertex[i0]);
					dgVector n0(e0 * e1);
					dgVector n1(e1 * e2);

					vertex[normalBase] = n0.Scale(dgRsqrt(n0 % n0));
					normalIndexCount[normalIndex] = normalBase;
					normalIndex++;
					normalBase++;

					vertex[normalBase] = n1.Scale(dgRsqrt(n1 % n1));
					normalIndexCount[normalIndex] = normalBase;
					normalIndex++;
					normalBase++;
				}
				vertexIndex++;
			}

			base = 0;
			index = 0;
			step = x1 - x0;
			for (dgInt32 z = z0; z < z1; z++) {
				dgInt32 z0Flag;
				dgInt32 z1Flag;

				z0Flag = ((z - z0 - 1) >> 31);
				z1Flag = ((z1 - z - 2) >> 31);
				for (dgInt32 x = x0; x < x1; x++) {
					dgInt32 xA0;
					dgInt32 xA1;
					dgInt32 zA0;
					dgInt32 zA1;
					dgInt32 zxA;
					dgInt32 x0Flag;
					dgInt32 x1Flag;
					dgFloat32 side;
					dgFloat32 diagSide;

					x1Flag = ((x - x0 - 1) >> 31);
					x0Flag = ((x1 - x - 2) >> 31);

					const dgVector &point = vertex[indices[base]];
					const dgVector &n = vertex[normalIndexCount[index * 2]];
					xA0 = ((~x0Flag) & (index * 2 + 3)) | (x0Flag & (index * 2));
					side = n % (vertex[indices[xA0 * 3 + 2]] - point);
					faceAdjencentEdgeNormal[base + 1] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[xA0] : -1;

					zxA = index * 2 + 1;
					diagSide = n % (vertex[indices[xA0 * 3 + 1]] - point);
					faceAdjencentEdgeNormal[base + 0] =
						(diagSide < dgFloat32(-1.0e-5f)) ? normalIndexCount[zxA] : -1;

					zA0 = ((~z0Flag) & ((index - step) * 2 + 1)) | (z0Flag & (index * 2));
					side = n % (vertex[indices[zA0 * 3]] - point);
					faceAdjencentEdgeNormal[base + 2] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[zA0] : -1;

					const dgVector &n1 = vertex[normalIndexCount[index * 2 + 1]];
					xA1 = ((~x1Flag) & (index * 2 - 2)) | (x1Flag & ((index - x + x0) * 2 + 1));
					side = n1 % (vertex[indices[xA1 * 3]] - point);
					faceAdjencentEdgeNormal[base + 3] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[xA1] : -1;

					zxA = index * 2;
					faceAdjencentEdgeNormal[base + 5] =
						(diagSide < dgFloat32(-1.0e-5f)) ? normalIndexCount[zxA] : -1;
					;

					zA1 = ((~z1Flag) & ((index + step) * 2)) | (z1Flag & (index * 2 + 1));
					side = n % (vertex[indices[zA1 * 3 + 1]] - point);
					faceAdjencentEdgeNormal[base + 4] =
						(side < dgFloat32(-1.0e-5f)) ? normalIndexCount[zA1] : -1;

					index++;
					base += 6;
				}
			}
		}

		data->m_faceCount = faceCount;

		// initialize the callback data structure
		data->m_vertexStrideInBytes = sizeof(dgVector);
		data->m_faceVertexIndex = indices;
		data->m_faceNormalIndex = normalIndexCount;
		data->m_faceMaxSize = facefaceMaxSize;
		data->m_faceAdjencentEdgeNormal = faceAdjencentEdgeNormal;
		data->m_userAttribute = attributes;
		data->m_faceIndexCount = faceIndexCount;
		data->m_vertex = &vertex[0].m_x;

		if (GetDebugCollisionCallback()) {
			dgTriplex triplex[3];
			const dgMatrix &matrix = data->m_polySoupBody->GetCollisionMatrix();
			for (dgInt32 i = 0; i < faceCount; i++) {
				for (dgInt32 j = 0; j < 3; j++) {
					dgVector p(matrix.TransformVector(vertex[indices[i * 3 + j]]));
					triplex[j].m_x = p.m_x;
					triplex[j].m_y = p.m_y;
					triplex[j].m_z = p.m_z;
				}
				GetDebugCollisionCallback()(data->m_polySoupBody, data->m_objBody,
											attributes[i], 3, &triplex[0].m_x, sizeof(dgTriplex));
			}
		}
	}
}
