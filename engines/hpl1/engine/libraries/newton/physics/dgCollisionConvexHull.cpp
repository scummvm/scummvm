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

#include "dgCollisionConvexHull.h"
#include "dgBody.h"
#include "dgContact.h"
#include "dgMeshEffect.h"
#include "hpl1/engine/libraries/newton/core/dg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

struct dgPlaneLocation : public dgPlane {
	int m_index;
	const dgConvexSimplexEdge *m_face;
};

dgCollisionConvexHull::dgCollisionConvexHull(dgMemoryAllocator *const allocator,
											 dgUnsigned32 signature, dgInt32 count, dgInt32 strideInBytes,
											 dgFloat32 tolerance, const dgFloat32 *vertexArray, const dgMatrix &matrix) : dgCollisionConvex(allocator, signature, matrix, m_convexHullCollision) {
	m_faceCount = 0;
	m_edgeCount = 0;
	m_vertexCount = 0;
	m_vertex = NULL;
	m_simplex = NULL;
	m_faceArray = NULL;
	m_boundPlanesCount = 0;

	m_rtti |= dgCollisionConvexHull_RTTI;

	Create(count, strideInBytes, vertexArray, tolerance);

	dgInt32 planeCount = 0;
	dgPlaneLocation planesArray[1024];
	const dgConvexSimplexEdge *const *faceArray = m_faceArray;
	for (dgInt32 i = 0; i < m_faceCount; i++) {
		const dgConvexSimplexEdge *const face = faceArray[i];

		dgInt32 i0 = face->m_prev->m_vertex;
		dgInt32 i1 = face->m_vertex;
		dgInt32 i2 = face->m_next->m_vertex;
		const dgBigVector p0(m_vertex[i0]);
		const dgBigVector p1(m_vertex[i1]);
		const dgBigVector p2(m_vertex[i2]);

		dgBigVector normal1((p1 - p0) * (p2 - p0));

		dgVector normal(
			(m_vertex[i1] - m_vertex[i0]) * (m_vertex[i2] - m_vertex[i0]));
		normal = normal.Scale(dgFloat32(1.0f) / dgSqrt(normal % normal));
		dgInt32 add = 1;
		for (dgInt32 j = 0; j < 3; j++) {
			if (dgAbsf(normal[j]) > dgFloat32(0.98f)) {
				add = 0;
			}
		}

		if (add) {
			for (dgInt32 j = 0; j < planeCount; j++) {
				dgFloat32 coplanar;
				coplanar = normal % planesArray[j];
				if (coplanar > 0.98f) {
					add = 0;
					break;
				}
			}

			if (add) {
				dgPlane plane(normal, dgFloat32(0.0f));
				dgVector planeSupport(SupportVertex(plane));
				plane.m_w = -(plane % planeSupport);
				//				_ASSERTE (plane.Evalue(m_boxOrigin) < 0.0f);
				dgPlane &tmpPlane = planesArray[planeCount];
				tmpPlane = plane;
				planesArray[planeCount].m_index = i;
				planesArray[planeCount].m_face = face;
				planeCount++;
				_ASSERTE(planeCount < dgInt32(sizeof(planesArray) / sizeof(planesArray[0])));
			}
		}
	}

	m_boundPlanesCount = 0;
	for (dgInt32 i = 0; i < planeCount; i++) {
		dgPlaneLocation &plane = planesArray[i];
		if (plane.m_face == m_faceArray[plane.m_index]) {
			Swap(m_faceArray[plane.m_index], m_faceArray[m_boundPlanesCount]);
		} else {
			dgInt32 j;
			for (j = m_boundPlanesCount; j < m_faceCount; j++) {
				if (plane.m_face == m_faceArray[j]) {
					Swap(m_faceArray[j], m_faceArray[m_boundPlanesCount]);
					break;
				}
			}
			_ASSERTE(j < m_faceCount);
		}
		m_boundPlanesCount++;
	}

	m_destructionImpulse = dgFloat32(1.0e20f);
}

dgCollisionConvexHull::dgCollisionConvexHull(dgWorld *const world,
											 dgDeserialize deserialization, void *const userData) : dgCollisionConvex(world, deserialization, userData) {
	m_rtti |= dgCollisionConvexHull_RTTI;
	deserialization(userData, &m_vertexCount, sizeof(dgInt32));
	deserialization(userData, &m_vertexCount, sizeof(dgInt32));
	deserialization(userData, &m_faceCount, sizeof(dgInt32));
	deserialization(userData, &m_edgeCount, sizeof(dgInt32));
	deserialization(userData, &m_boundPlanesCount, sizeof(dgInt32));
	deserialization(userData, &m_destructionImpulse, sizeof(dgFloat32));

	m_vertex = (dgVector *)m_allocator->Malloc(
		dgInt32(m_vertexCount * sizeof(dgVector)));
	m_simplex = (dgConvexSimplexEdge *)m_allocator->Malloc(
		dgInt32(m_edgeCount * sizeof(dgConvexSimplexEdge)));
	m_faceArray = (dgConvexSimplexEdge **)m_allocator->Malloc(
		dgInt32(m_faceCount * sizeof(dgConvexSimplexEdge *)));

	deserialization(userData, m_vertex, m_vertexCount * sizeof(dgVector));

	for (dgInt32 i = 0; i < m_edgeCount; i++) {
		dgInt32 serialization[4];
		deserialization(userData, serialization, sizeof(serialization));

		m_simplex[i].m_vertex = serialization[0];
		m_simplex[i].m_twin = m_simplex + serialization[1];
		m_simplex[i].m_next = m_simplex + serialization[2];
		m_simplex[i].m_prev = m_simplex + serialization[3];
	}

	for (dgInt32 i = 0; i < m_faceCount; i++) {
		dgInt32 faceOffset;
		deserialization(userData, &faceOffset, sizeof(dgInt32));
		m_faceArray[i] = m_simplex + faceOffset;
	}

	SetVolumeAndCG();
}

dgCollisionConvexHull::~dgCollisionConvexHull() {
	if (m_faceArray) {
		m_allocator->Free(m_faceArray);
	}
}

dgInt32 dgCollisionConvexHull::GetFaceIndices(dgInt32 index,
											  dgInt32 *indices) const {
	dgInt32 count;

	count = 0;
	const dgConvexSimplexEdge *face = m_faceArray[index];
	do {
		indices[count] = face->m_vertex;
		count++;
		face = face->m_next;
	} while (face != m_faceArray[index]);

	return count;
}

dgBigVector dgCollisionConvexHull::FaceNormal(const dgEdge *face,
											  const dgBigVector *const pool) const {
	const dgEdge *edge = face;
	dgBigVector p0(pool[edge->m_incidentVertex]);
	edge = edge->m_next;

	dgBigVector p1(pool[edge->m_incidentVertex]);
	dgBigVector e1(p1 - p0);

	dgBigVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					   dgFloat32(0.0f));
	for (edge = edge->m_next; edge != face; edge = edge->m_next) {
		dgBigVector p2(pool[edge->m_incidentVertex]);
		dgBigVector e2(p2 - p0);
		dgBigVector n1(e1 * e2);
#ifdef _DEBUG
		dgFloat64 mag = normal % n1;
		_ASSERTE(mag >= -dgFloat32(0.1f));
#endif
		normal += n1;
		e1 = e2;
	}
	dgFloat64 den = sqrt(normal % normal) + dgFloat64(1.0e-24f);
	normal = normal.Scale(dgFloat64(1.0f) / den);

#ifdef _DEBUG
	edge = face;
	dgBigVector e0(
		pool[edge->m_incidentVertex] - pool[edge->m_prev->m_incidentVertex]);
	do {
		dgBigVector e1(
			pool[edge->m_next->m_incidentVertex] - pool[edge->m_incidentVertex]);
		dgBigVector n1(e0 * e1);
		dgFloat64 x = normal % n1;
		_ASSERTE(x > -dgFloat64(0.01f));
		e0 = e1;
		edge = edge->m_next;
	} while (edge != face);
#endif

	return normal;
}

bool dgCollisionConvexHull::RemoveCoplanarEdge(dgPolyhedra &polyhedra,
											   const dgBigVector *const hullVertexArray) const {
	bool removeEdge = false;
	// remove coplanar edges
	dgInt32 mark = polyhedra.IncLRU();
	dgPolyhedra::Iterator iter(polyhedra);
	for (iter.Begin(); iter;) {
		dgEdge *edge0 = &(*iter);
		iter++;

		if (edge0->m_incidentFace != -1) {

			if (edge0->m_mark < mark) {
				edge0->m_mark = mark;
				edge0->m_twin->m_mark = mark;
				dgBigVector normal0(FaceNormal(edge0, &hullVertexArray[0]));
				dgBigVector normal1(FaceNormal(edge0->m_twin, &hullVertexArray[0]));

				dgFloat64 test = normal0 % normal1;
				if (test > dgFloat64(0.99995f)) {

					if ((edge0->m_twin->m_next->m_twin->m_next != edge0) && (edge0->m_next->m_twin->m_next != edge0->m_twin)) {
#define DG_MAX_EDGE_ANGLE dgFloat32(1.0e-3f)

						if (edge0->m_twin == &(*iter)) {
							if (iter) {
								iter++;
							}
						}

						dgBigVector e1(
							hullVertexArray[edge0->m_twin->m_next->m_next->m_incidentVertex] - hullVertexArray[edge0->m_incidentVertex]);
						dgBigVector e0(
							hullVertexArray[edge0->m_incidentVertex] - hullVertexArray[edge0->m_prev->m_incidentVertex]);

						_ASSERTE((e0 % e0) >= dgFloat64(0.0f));
						_ASSERTE((e1 % e1) >= dgFloat64(0.0f));

						e0 = e0.Scale(dgFloat64(1.0f) / sqrt(e0 % e0));
						e1 = e1.Scale(dgFloat64(1.0f) / sqrt(e1 % e1));
						dgBigVector n1(e0 * e1);

						dgFloat64 projection = n1 % normal0;
						if (projection >= DG_MAX_EDGE_ANGLE) {

							dgBigVector e1(
								hullVertexArray[edge0->m_next->m_next->m_incidentVertex] - hullVertexArray[edge0->m_twin->m_incidentVertex]);
							dgBigVector e0(
								hullVertexArray[edge0->m_twin->m_incidentVertex] - hullVertexArray[edge0->m_twin->m_prev->m_incidentVertex]);
							_ASSERTE((e0 % e0) >= dgFloat64(0.0f));
							_ASSERTE((e1 % e1) >= dgFloat64(0.0f));
							// e0 = e0.Scale (dgRsqrt (e0 % e0));
							// e1 = e1.Scale (dgRsqrt (e1 % e1));
							e0 = e0.Scale(dgFloat64(1.0f) / sqrt(e0 % e0));
							e1 = e1.Scale(dgFloat64(1.0f) / sqrt(e1 % e1));

							dgBigVector n1(e0 * e1);
							projection = n1 % normal0;
							if (projection >= DG_MAX_EDGE_ANGLE) {
								_ASSERTE(&(*iter) != edge0);
								_ASSERTE(&(*iter) != edge0->m_twin);
								polyhedra.DeleteEdge(edge0);
								removeEdge = true;
							}
						}

					} else {
						dgEdge *next = edge0->m_next;
						dgEdge *prev = edge0->m_prev;
						polyhedra.DeleteEdge(edge0);
						for (edge0 = next; edge0->m_prev->m_twin == edge0; edge0 = next) {
							next = edge0->m_next;
							polyhedra.DeleteEdge(edge0);
						}

						for (edge0 = prev; edge0->m_next->m_twin == edge0; edge0 = prev) {
							prev = edge0->m_prev;
							polyhedra.DeleteEdge(edge0);
						}
						iter.Begin();
						removeEdge = true;
					}
				}
			}
		}
	}

	return removeEdge;
}

bool dgCollisionConvexHull::CheckConvex(dgPolyhedra &polyhedra1,
										const dgBigVector *hullVertexArray) const {
	dgPolyhedra polyhedra(polyhedra1);

	dgPolyhedra::Iterator iter(polyhedra);
	dgBigVector center(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
					   dgFloat32(0.0f));

	dgInt32 count = 0;
	dgInt32 mark = polyhedra.IncLRU();
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_mark < mark) {
			count++;
			center += hullVertexArray[edge->m_incidentVertex];
			dgEdge *ptr = edge;
			do {
				ptr->m_mark = mark;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge);
		}
	}
	center = center.Scale(dgFloat64(1.0f) / dgFloat64(count));

	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		dgBigVector normal0(FaceNormal(edge, hullVertexArray));
		dgBigVector normal1(FaceNormal(edge->m_twin, hullVertexArray));

		dgBigPlane plane0(normal0,
						  -(normal0 % hullVertexArray[edge->m_incidentVertex]));
		dgBigPlane plane1(normal1,
						  -(normal1 % hullVertexArray[edge->m_twin->m_incidentVertex]));
		dgFloat64 test0 = plane0.Evalue(center);
		if (test0 > dgFloat64(1.0e-3f)) {
			return false;
		}
		dgFloat64 test1 = plane1.Evalue(center);
		//		if (test1 > dgFloat64 (0.0f)) {
		if (test1 > dgFloat64(1.0e-3f)) {
			return false;
		}
	}

	return true;
}

bool dgCollisionConvexHull::Create(dgInt32 count, dgInt32 strideInBytes,
								   const dgFloat32 *const vertexArray, dgFloat32 tolerance) {
	dgInt32 stride = strideInBytes / sizeof(dgFloat32);
	dgStack<dgFloat64> buffer(3 * count);
	for (dgInt32 i = 0; i < count; i++) {
		buffer[i * 3 + 0] = vertexArray[i * stride + 0];
		buffer[i * 3 + 1] = vertexArray[i * stride + 1];
		buffer[i * 3 + 2] = vertexArray[i * stride + 2];
	}

	dgConvexHull3d *convexHull = new (GetAllocator()) dgConvexHull3d(
		GetAllocator(), &buffer[0], 3 * sizeof(dgFloat64), count, tolerance);
	if (!convexHull->GetCount()) {
		delete convexHull;
		return false;
	}

	// check for degenerated faces
	for (bool success = false; !success;) {
		success = true;
		const dgBigVector *const hullVertexArray = convexHull->GetVertexPool();

		dgStack<dgInt8> mask(convexHull->GetVertexCount());
		memset(&mask[0], 1, mask.GetSizeInBytes());
		for (dgConvexHull3d::dgListNode *node = convexHull->GetFirst(); node; node =
																				  node->GetNext()) {
			dgConvexHull3DFace &face = node->GetInfo();
			const dgBigVector &p0 = hullVertexArray[face.m_index[0]];
			const dgBigVector &p1 = hullVertexArray[face.m_index[1]];
			const dgBigVector &p2 = hullVertexArray[face.m_index[2]];
			dgBigVector p1p0(p1 - p0);
			dgBigVector p2p0(p2 - p0);
			dgBigVector normal(p2p0 * p1p0);
			dgFloat64 mag2 = normal % normal;
			if (mag2 < dgFloat64(1.0e-6f * 1.0e-6f)) {
				success = false;
				dgInt32 index = -1;
				dgBigVector p2p1(p2 - p1);
				dgFloat64 dist10 = p1p0 % p1p0;
				dgFloat64 dist20 = p2p0 % p2p0;
				dgFloat64 dist21 = p2p1 % p2p1;
				if ((dist10 >= dist20) && (dist10 >= dist21)) {
					index = 2;
				} else if ((dist20 >= dist10) && (dist20 >= dist21)) {
					index = 1;
				} else if ((dist21 >= dist10) && (dist21 >= dist20)) {
					index = 0;
				}
				_ASSERTE(index != -1);
				mask[face.m_index[index]] = 0;
			}
		}
		if (!success) {
			dgInt32 count = 0;
			dgInt32 vertexCount = convexHull->GetVertexCount();
			for (dgInt32 i = 0; i < vertexCount; i++) {
				if (mask[i]) {
					buffer[count * 3 + 0] = hullVertexArray[i].m_x;
					buffer[count * 3 + 1] = hullVertexArray[i].m_y;
					buffer[count * 3 + 2] = hullVertexArray[i].m_z;
					count++;
				}
			}
			delete convexHull;
			convexHull = new (GetAllocator()) dgConvexHull3d(GetAllocator(),
															 &buffer[0], 3 * sizeof(dgFloat64), count, tolerance);
		}
	}

	dgInt32 vertexCount = convexHull->GetVertexCount();
	const dgBigVector *const hullVertexArray = convexHull->GetVertexPool();

	dgPolyhedra polyhedra(GetAllocator());
	polyhedra.BeginFace();
	for (dgConvexHull3d::dgListNode *node = convexHull->GetFirst(); node; node =
																			  node->GetNext()) {
		dgConvexHull3DFace &face = node->GetInfo();
		polyhedra.AddFace(face.m_index[0], face.m_index[1], face.m_index[2]);
	}

	polyhedra.EndFace();

	if (vertexCount > 4) {
		bool edgeRemoved = false;
		while (RemoveCoplanarEdge(polyhedra, hullVertexArray)) {
			edgeRemoved = true;
		}
		if (edgeRemoved) {
			if (!CheckConvex(polyhedra, hullVertexArray)) {
				return false;
			}
		}
	}

	dgInt32 maxEdgeCount = polyhedra.GetCount();

	dgStack<dgEdge *> stack(1024 + maxEdgeCount);
	dgEdge *firstFace = &polyhedra.GetRoot()->GetInfo();

	_ASSERTE(firstFace->m_twin->m_next != firstFace);

	dgInt32 stackIndex = 1;
	stack[0] = firstFace;

	dgStack<dgInt32> vertexMap(vertexCount);
	memset(&vertexMap[0], -1, vertexCount * sizeof(dgInt32));

	//	m_edgeCount = 0;
	//	m_vertexCount = 0;

	dgInt32 i1 = polyhedra.IncLRU();
	while (stackIndex) {
		stackIndex--;
		dgEdge *const edge0 = stack[stackIndex];

		if (edge0->m_mark != i1) {
			if (vertexMap[edge0->m_incidentVertex] == -1) {
				vertexMap[edge0->m_incidentVertex] = m_vertexCount;
				m_vertexCount++;
			}
			dgEdge *ptr = edge0;
			do {
				stack[stackIndex] = ptr->m_twin;
				stackIndex++;
				ptr->m_mark = i1;
				ptr->m_userData = m_edgeCount;
				m_edgeCount++;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge0);
		}
	}

	m_vertex = (dgVector *)m_allocator->Malloc(
		dgInt32(m_vertexCount * sizeof(dgVector)));
	m_simplex = (dgConvexSimplexEdge *)m_allocator->Malloc(
		dgInt32(m_edgeCount * sizeof(dgConvexSimplexEdge)));

	for (dgInt32 i = 0; i < vertexCount; i++) {
		if (vertexMap[i] != -1) {
			m_vertex[vertexMap[i]] = hullVertexArray[i];
			m_vertex[vertexMap[i]].m_w = dgFloat32(1.0f);
		}
	}

	i1 = polyhedra.IncLRU();
	stackIndex = 1;
	stack[0] = firstFace;
	while (stackIndex) {

		stackIndex--;
		dgEdge *const edge0 = stack[stackIndex];

		if (edge0->m_mark != i1) {

			dgEdge *ptr = edge0;
			do {
				ptr->m_mark = i1;
				stack[stackIndex] = ptr->m_twin;
				stackIndex++;

				dgConvexSimplexEdge *const simplexPtr = &m_simplex[ptr->m_userData];
				simplexPtr->m_vertex = vertexMap[ptr->m_incidentVertex];
				simplexPtr->m_next = &m_simplex[ptr->m_next->m_userData];
				simplexPtr->m_prev = &m_simplex[ptr->m_prev->m_userData];
				simplexPtr->m_twin = &m_simplex[ptr->m_twin->m_userData];

				ptr = ptr->m_twin->m_next;
			} while (ptr != edge0);
		}
	}

	SetVolumeAndCG();
	m_faceCount = 0;
	dgStack<char> mark(m_edgeCount);
	memset(&mark[0], 0, m_edgeCount * sizeof(dgInt8));

	dgStack<dgConvexSimplexEdge *> faceArray(m_edgeCount);
	for (dgInt32 i = 0; i < m_edgeCount; i++) {
		dgConvexSimplexEdge *const face = &m_simplex[i];
		if (!mark[i]) {
			dgConvexSimplexEdge *ptr = face;
			do {
				_ASSERTE((ptr - m_simplex) >= 0);
				mark[dgInt32(ptr - m_simplex)] = '1';
				ptr = ptr->m_next;
			} while (ptr != face);

			faceArray[m_faceCount] = face;
			m_faceCount++;
		}
	}
	m_faceArray = (dgConvexSimplexEdge **)m_allocator->Malloc(
		dgInt32(m_faceCount * sizeof(dgConvexSimplexEdge *)));
	memcpy(m_faceArray, &faceArray[0],
		   m_faceCount * sizeof(dgConvexSimplexEdge *));

	delete convexHull;
	return true;
}

dgInt32 dgCollisionConvexHull::CalculateSignature() const {
	_ASSERTE(0);
	return dgInt32(GetSignature());
}

void dgCollisionConvexHull::SetBreakImpulse(dgFloat32 force) {
	m_destructionImpulse = force;
}

dgFloat32 dgCollisionConvexHull::GetBreakImpulse() const {
	return m_destructionImpulse;
}

void dgCollisionConvexHull::SetCollisionBBox(const dgVector &p0__,
											 const dgVector &p1__) {
	_ASSERTE(0);
}

void dgCollisionConvexHull::DebugCollision(const dgMatrix &matrixPtr,
										   OnDebugCollisionMeshCallback callback, void *const userData) const {
	dgInt32 i;
	dgInt32 count;
	dgConvexSimplexEdge *ptr;
	dgConvexSimplexEdge *face;

	dgStack<dgTriplex> tmp(m_vertexCount);
	dgMatrix matrix(GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex(&tmp[0].m_x, sizeof(dgTriplex), &m_vertex[0].m_x,
							sizeof(dgVector), m_vertexCount);

	for (i = 0; i < m_faceCount; i++) {
		face = m_faceArray[i];
		ptr = face;
		count = 0;
		dgTriplex vertex[256];
		do {
			vertex[count] = tmp[ptr->m_vertex];
			count++;
			ptr = ptr->m_next;
		} while (ptr != face);
		callback(userData, count, &vertex[0].m_x, 0);
	}
}

void dgCollisionConvexHull::GetCollisionInfo(dgCollisionInfo *info) const {
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_offsetMatrix = GetOffsetMatrix();
	//	strcpy (info->m_collisionType, "convexHull");
	info->m_collisionType = m_collsionId;

	info->m_convexHull.m_vertexCount = m_vertexCount;
	info->m_convexHull.m_strideInBytes = sizeof(dgVector);
	info->m_convexHull.m_faceCount = m_faceCount;
	info->m_convexHull.m_vertex = &m_vertex[0];
}

void dgCollisionConvexHull::Serialize(dgSerialize callback,
									  void *const userData) const {
	SerializeLow(callback, userData);

	callback(userData, &m_vertexCount, sizeof(dgInt32));
	callback(userData, &m_vertexCount, sizeof(dgInt32));
	callback(userData, &m_faceCount, sizeof(dgInt32));
	callback(userData, &m_edgeCount, sizeof(dgInt32));
	callback(userData, &m_boundPlanesCount, sizeof(dgInt32));
	callback(userData, &m_destructionImpulse, sizeof(dgFloat32));

	callback(userData, m_vertex, m_vertexCount * sizeof(dgVector));

	for (dgInt32 i = 0; i < m_edgeCount; i++) {
		dgInt32 serialization[4];
		serialization[0] = m_simplex[i].m_vertex;
		serialization[1] = dgInt32(m_simplex[i].m_twin - m_simplex);
		serialization[2] = dgInt32(m_simplex[i].m_next - m_simplex);
		serialization[3] = dgInt32(m_simplex[i].m_prev - m_simplex);
		callback(userData, serialization, sizeof(serialization));
	}

	for (dgInt32 i = 0; i < m_faceCount; i++) {
		dgInt32 faceOffset;
		faceOffset = dgInt32(m_faceArray[i] - m_simplex);
		callback(userData, &faceOffset, sizeof(dgInt32));
	}
}

bool dgCollisionConvexHull::OOBBTest(const dgMatrix &matrix,
									 const dgCollisionConvex *const shape, void *const cacheOrder) const {
	bool ret;
	_ASSERTE(cacheOrder);

	ret = dgCollisionConvex::OOBBTest(matrix, shape, cacheOrder);
	if (ret) {
		const dgConvexSimplexEdge *const *faceArray = m_faceArray;
		dgCollisionBoundPlaneCache *const cache =
			(dgCollisionBoundPlaneCache *)cacheOrder;

		for (dgInt32 i = 0; i < dgInt32(sizeof(cache->m_planes) / sizeof(dgPlane));
			 i++) {
			dgFloat32 dist;
			const dgPlane &plane = cache->m_planes[i];
			if ((plane % plane) > dgFloat32(0.0f)) {
				dgVector dir(matrix.UnrotateVector(plane.Scale(-1.0f)));
				dir.m_w = dgFloat32(0.0f);
				dgVector p(matrix.TransformVector(shape->SupportVertex(dir)));
				dist = plane.Evalue(p);
				if (dist > dgFloat32(0.1f)) {
					return false;
				}
			}
		}

		for (dgInt32 i = 0; i < m_boundPlanesCount; i++) {
			dgInt32 i0;
			dgInt32 i1;
			dgInt32 i2;
			dgFloat32 dist;

			const dgConvexSimplexEdge *const face = faceArray[i];
			i0 = face->m_prev->m_vertex;
			i1 = face->m_vertex;
			i2 = face->m_next->m_vertex;
			const dgVector &p0 = m_vertex[i0];

			dgVector normal((m_vertex[i1] - p0) * (m_vertex[i2] - p0));
			normal = normal.Scale(dgFloat32(1.0f) / dgSqrt(normal % normal));

			dgVector dir(matrix.UnrotateVector(normal.Scale(-1.0f)));
			dir.m_w = dgFloat32(0.0f);
			dgVector p(matrix.TransformVector(shape->SupportVertex(dir)));

			//_ASSERTE ((normal % (m_boxOrigin - p0)) < 0.0f);
			dist = normal % (p - p0);
			if (dist > dgFloat32(0.1f)) {
				for (dgInt32 j = 0;
					 j < (dgInt32(sizeof(cache->m_planes) / sizeof(dgPlane)) - 1); j++) {
					cache->m_planes[j + 1] = cache->m_planes[j];
				}
				cache->m_planes[1] = dgPlane(normal, -(normal % p0));
				return false;
			}
		}
	}
	return ret;
}
