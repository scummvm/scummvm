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

#include "dgStdafx.h"
#include "dgHeap.h"
#include "dgDebug.h"
#include "dgStack.h"
#include "dgSphere.h"
#include "dgPolyhedra.h"
#include "dgSmallDeterminant.h"

#include "common/util.h"

//#define DG_MIN_EDGE_ASPECT_RATIO  dgFloat64 (0.02f)

class dgDiagonalEdge {
public:
	dgDiagonalEdge(dgEdge *const edge) :
		m_i0(edge->m_incidentVertex), m_i1(edge->m_twin->m_incidentVertex) {
	}
	dgInt32 m_i0;
	dgInt32 m_i1;
};

struct dgEdgeCollapseEdgeHandle {
	dgEdgeCollapseEdgeHandle(dgEdge *const newEdge) :
		m_inList(false), m_edge(newEdge) {
	}

	dgEdgeCollapseEdgeHandle(const dgEdgeCollapseEdgeHandle &dataHandle) :
		m_inList(true), m_edge(dataHandle.m_edge) {
		dgEdgeCollapseEdgeHandle *const handle =
		    (dgEdgeCollapseEdgeHandle *) IntToPointer(m_edge->m_userData);
		if (handle) {
			NEWTON_ASSERT(handle != this);
			handle->m_edge = NULL;
		}
		m_edge->m_userData = dgUnsigned64(PointerToInt(this));
	}

	~dgEdgeCollapseEdgeHandle() {
		if (m_inList) {
			if (m_edge) {
				dgEdgeCollapseEdgeHandle *const handle =
				    (dgEdgeCollapseEdgeHandle *) IntToPointer(m_edge->m_userData);
				if (handle == this) {
					m_edge->m_userData = PointerToInt(NULL);
				}
			}
		}
		m_edge = NULL;
	}

	dgUnsigned32 m_inList;
	dgEdge *m_edge;
};

class dgVertexCollapseVertexMetric {
public:
	dgFloat64 elem[10];

	dgVertexCollapseVertexMetric(const dgBigPlane &plane) {
		elem[0] = plane.m_x * plane.m_x;
		elem[1] = plane.m_y * plane.m_y;
		elem[2] = plane.m_z * plane.m_z;
		elem[3] = plane.m_w * plane.m_w;
		elem[4] = dgFloat64(2.0) * plane.m_x * plane.m_y;
		elem[5] = dgFloat64(2.0) * plane.m_x * plane.m_z;
		elem[6] = dgFloat64(2.0) * plane.m_x * plane.m_w;
		elem[7] = dgFloat64(2.0) * plane.m_y * plane.m_z;
		elem[8] = dgFloat64(2.0) * plane.m_y * plane.m_w;
		elem[9] = dgFloat64(2.0) * plane.m_z * plane.m_w;
	}

	void Clear() {
		for (uint i = 0; i < ARRAYSIZE(elem); i++) elem[i] = dgFloat64(0.0);
	}

	void Accumulate(const dgVertexCollapseVertexMetric &p) {
		elem[0] += p.elem[0];
		elem[1] += p.elem[1];
		elem[2] += p.elem[2];
		elem[3] += p.elem[3];
		elem[4] += p.elem[4];
		elem[5] += p.elem[5];
		elem[6] += p.elem[6];
		elem[7] += p.elem[7];
		elem[8] += p.elem[8];
		elem[9] += p.elem[9];
	}

	void Accumulate(const dgBigPlane &plane) {
		elem[0] += plane.m_x * plane.m_x;
		elem[1] += plane.m_y * plane.m_y;
		elem[2] += plane.m_z * plane.m_z;
		elem[3] += plane.m_w * plane.m_w;

		elem[4] += dgFloat64(2.0f) * plane.m_x * plane.m_y;
		elem[5] += dgFloat64(2.0f) * plane.m_x * plane.m_z;
		elem[7] += dgFloat64(2.0f) * plane.m_y * plane.m_z;

		elem[6] += dgFloat64(2.0f) * plane.m_x * plane.m_w;
		elem[8] += dgFloat64(2.0f) * plane.m_y * plane.m_w;
		elem[9] += dgFloat64(2.0f) * plane.m_z * plane.m_w;
	}

	dgFloat64 Evalue(const dgVector &p) const {
		dgFloat64 acc = elem[0] * p.m_x * p.m_x + elem[1] * p.m_y * p.m_y
		                + elem[2] * p.m_z * p.m_z + elem[4] * p.m_x * p.m_y
		                + elem[5] * p.m_x * p.m_z + elem[7] * p.m_y * p.m_z + elem[6] * p.m_x
		                + elem[8] * p.m_y + elem[9] * p.m_z + elem[3];
		return fabs(acc);
	}
};

#if 0
namespace InternalPolyhedra {

struct VertexCache: public dgList<dgEdge *> {
	dgInt32 size;

	VertexCache(dgInt32 t, dgMemoryAllocator *const allocator)
		: dgList<dgEdge*>(allocator) {
		size = t;
	}

	dgInt32 IsInCache(dgEdge *edge) const {
		dgInt32 score;
		dgEdge *ptr;

		score = GetCount() + 2;
		Iterator iter(*this);
		for (iter.End(); iter; iter --) {
			ptr = *iter;
			if (ptr->m_incidentVertex == edge->m_incidentVertex) {
				return score;
			}
			score --;
		}
		return 0;
	}

	dgInt32 AddEdge(dgEdge *edge) {
		if (IsInCache(edge) == 0) {
			Addtop(edge);
			if (GetCount() > size) {
				Remove(GetLast());
			}
			return 1;
		}
		return 0;
	}

	dgEdge *GetEdge(dgInt32 mark) const {
		dgEdge *ptr;
		dgEdge *edge;

		if (GetCount()) {
			Iterator iter(*this);
			for (iter.End(); iter; iter --) {
				ptr = *iter;
				edge = ptr;
				do {
					if (edge->m_incidentFace > 0) {
						if (edge->m_mark != mark) {
							return edge;
						}
					}
					edge = edge->m_twin->m_next;
				} while (ptr != edge);
			}
		}
		return NULL;
	}
};

/*
 static bool CheckIfCoplanar (
 const dgBigPlane& plane,
 dgEdge *face,
 const dgFloat32* const pool,
 dgInt32 stride)
 {
 dgEdge* ptr;
 dgFloat64 dist;

 ptr = face;
 do {
 dgBigVector p (&pool[ptr->m_incidentVertex * stride]);
 dist = fabs (plane.Evalue (p));
 if (dist > dgFloat64(0.08)) {
 return false;
 }
 ptr = ptr->m_next;
 } while (ptr != face);

 return true;
 }
 */

static void GetAdjacentCoplanarFacesPerimeter(
    dgPolyhedra &perimeter,
    const dgPolyhedra &polyhedra,
    dgEdge *const face,
    const dgFloat32 *const pool,
    dgInt32 strideInBytes,
    dgEdge **const stack,
    dgInt32 *const faceIndex) {
	const dgFloat32 normalDeviation = dgFloat32(0.9999f);
	dgStack<dgInt32>facesIndex(4096);

	NEWTON_ASSERT(face->m_incidentFace > 0);

	polyhedra.IncLRU();
	dgInt32 faceMark = polyhedra.IncLRU();

	dgVector normal(polyhedra.FaceNormal(face, pool, strideInBytes));
	dgFloat32 dot = normal % normal;
	if (dot < dgFloat32(1.0e-12f)) {
		dgEdge *ptr = face;
		dgInt32 faceIndexCount = 0;
		do {
			faceIndex[faceIndexCount] = ptr->m_incidentVertex;
			faceIndexCount ++;
			ptr->m_mark = faceMark;
			ptr = ptr->m_next;
		} while (ptr != face);
		perimeter.AddFace(faceIndexCount, faceIndex);
		return;
	}
	normal = normal.Scale(dgFloat32(1.0f) / dgSqrt(dot));

	stack[0] = face;
	dgInt32 index = 1;
	perimeter.BeginFace();
	while (index) {
		index --;
		dgEdge *edge = stack[index];

		if (edge->m_mark == faceMark) {
			continue;
		}

		dgVector normal1(polyhedra.FaceNormal(edge, pool, strideInBytes));
		dot = normal1 % normal1;
		if (dot < dgFloat32(1.0e-12f)) {
			dot = dgFloat32(1.0e-12f);
		}
		normal1 = normal1.Scale(dgFloat32(1.0f) / dgSqrt(dot));

		dot = normal1 % normal;
		if (dot >= normalDeviation) {
			dgEdge *ptr = edge;
			dgInt32 faceIndexCount = 0;
			do {
				faceIndex[faceIndexCount] = ptr->m_incidentVertex;
				faceIndexCount ++;
				ptr->m_mark = faceMark;
				if ((ptr->m_twin->m_incidentFace > 0) && (ptr->m_twin->m_mark != faceMark)) {
					stack[index] = ptr->m_twin;
					index ++;
					NEWTON_ASSERT(index < polyhedra.GetCount() / 2);
				}
				ptr = ptr->m_next;
			} while (ptr != edge);
			perimeter.AddFace(faceIndexCount, faceIndex);
		}
	}
	perimeter.EndFace();

	dgPolyhedra::Iterator iter(perimeter);
	for (iter.Begin(); iter;) {
		dgEdge *edge = &(*iter);
		iter ++;
		if ((edge->m_incidentFace > 0) && (edge->m_twin->m_incidentFace > 0)) {
			if (perimeter.GetNodeFromInfo(*edge->m_twin) == iter.GetNode()) {
				iter ++;
			}
			perimeter.DeleteEdge(edge);
		}
	}
}
}

dgPolyhedraDescriptor::dgPolyhedraDescriptor(const dgPolyhedra &Polyhedra)
	: m_unboundedLoops(Polyhedra.GetAllocator()) {
	Update(Polyhedra);
}

dgPolyhedraDescriptor::~dgPolyhedraDescriptor() {
}

void dgPolyhedraDescriptor::Update(const dgPolyhedra &srcPolyhedra) {
	dgInt32 saveMark;
	dgInt32 faceCountLocal;
	dgInt32 edgeCountLocal;
	dgInt32 vertexCountLocal;
	dgInt32 maxVertexIndexLocal;
	dgEdge *ptr;
	dgEdge *edge;
	dgPolyhedra *polyhedra;

	polyhedra = (dgPolyhedra *) &srcPolyhedra;

	faceCountLocal = 0;
	edgeCountLocal = 0;
	vertexCountLocal = 0;
	maxVertexIndexLocal = -1;

	saveMark = polyhedra->m_edgeMark;
	if (saveMark < 8) {
		saveMark = 8;
	}
	polyhedra->m_edgeMark = 8;
	dgPolyhedra::Iterator iter(*polyhedra);
	for (iter.Begin(); iter; iter ++) {
		edge = &(*iter);
		edge->m_mark = 0;
		edgeCountLocal ++;
		if (edge->m_incidentVertex > maxVertexIndexLocal) {
			maxVertexIndexLocal = edge->m_incidentVertex;
		}
	}

	m_unboundedLoops.RemoveAll();
	for (iter.Begin(); iter; iter ++) {
		edge = &(*iter);

		if (edge->m_incidentFace < 0) {
			if (~edge->m_mark & 1) {
				m_unboundedLoops.Append(edge);
				ptr = edge;
				do {
					ptr->m_mark |= 1;
					ptr = ptr->m_next;
				} while (ptr != edge);
			}
		}

		if (~edge->m_mark & 2) {
			vertexCountLocal ++;
			ptr = edge;
			do {
				ptr->m_mark |= 2;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge);
		}

		if (~edge->m_mark & 4) {
			ptr = edge;
			faceCountLocal ++;
			do {
				ptr->m_mark |= 4;
				ptr = ptr->m_next;
			} while (ptr != edge);
		}
	}

	m_edgeCount = edgeCountLocal;
	m_faceCount = faceCountLocal;
	m_vertexCount = vertexCountLocal;
	m_maxVertexIndex = maxVertexIndexLocal + 1;

	polyhedra->m_edgeMark = saveMark;
}

void dgPolyhedra::DeleteAllFace() {
	m_baseMark = 0;
	m_edgeMark = 0;
	m_faceSecuence = 0;
	RemoveAll();
}

bool dgPolyhedra::SanityCheck() const {
	//NEWTON_ASSERT (0);
	return true;
	/*
	 dgInt32 i;
	 dgInt32 coorCount;
	 dgEdge *ptr;
	 dgEdge *edge;
	 dgTreeNode *node;
	 dgStack<dgInt32> coor(65536);

	 Iterator iter (*this);
	 for (iter.Begin(); iter; iter ++) {
	 node = iter.GetNode();
	 if (!node->IsAlive()) {
	 return false;
	 }

	 edge = &(*iter);

	 if ((edge->m_incidentFace < 0) && (edge->m_twin->m_incidentFace < 0)) {
	 return false;
	 }


	 if (edge->m_mark > m_edgeMark) {
	 return false;
	 }

	 node = iter.GetNode();
	 dgPairKey key (edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
	 if (key.GetVal() != node->GetKey()) {
	 return false;
	 }

	 ptr = edge;
	 do {
	 if (ptr->m_incidentVertex != edge->m_incidentVertex) {
	 return false;
	 }
	 ptr = ptr->m_twin->m_next;
	 } while (ptr != edge);

	 ptr = edge;
	 coorCount = 0;
	 do {
	 if (coorCount  * sizeof (dgInt32) > coor.GetSizeInBytes()) {
	 return false;
	 }
	 if (ptr->m_incidentFace != edge->m_incidentFace) {
	 return false;
	 }
	 coor [coorCount] = ptr->m_incidentVertex;
	 coorCount ++;

	 ptr = ptr->m_next;
	 } while (ptr != edge);

	 ptr = edge->m_prev;
	 i = 0;
	 do {
	 if (i * sizeof (dgInt32) > coor.GetSizeInBytes()) {
	 return false;
	 }
	 if (ptr->m_incidentFace != edge->m_incidentFace) {
	 return false;
	 }

	 if (coor [coorCount - i - 1] != ptr->m_incidentVertex) {
	 return false;
	 }

	 i ++;
	 ptr = ptr->m_prev;
	 } while (ptr != edge->m_prev);

	 if (edge->m_twin->m_twin != edge) {
	 return false;
	 }

	 if (edge->m_prev->m_next != edge) {
	 return false;
	 }

	 if (edge->m_next->m_prev != edge) {
	 return false;
	 }
	 }

	 return dgTree <dgEdge, dgInt64>::SanityCheck();
	 */
}

dgEdge *dgPolyhedra::FindVertexNode(dgInt32 v) const {
	dgEdge *edge;
	dgTreeNode *node;

	dgPairKey key(0, v);
	node = FindGreaterEqual(key.GetVal());
	edge = NULL;
	if (node) {
		dgEdge *ptr;
		ptr = node->GetInfo().m_twin;
		if (ptr->m_incidentVertex == v) {
			edge = ptr;
		}
	}

	return edge;
}

dgEdge *dgPolyhedra::SpliteEdgeAndTriangulate(dgInt32 newIndex, dgEdge *srcEdge) {
	dgEdge *ankle = srcEdge->m_next;
	dgEdge *edge = SpliteEdge(newIndex, srcEdge);
	NEWTON_ASSERT(edge == ankle->m_prev);
	edge = ankle->m_prev;
	ankle = edge;

	do {
		dgEdge *const faceEdge = edge->m_twin;
		if (faceEdge->m_incidentFace > 0) {
			if (faceEdge->m_next->m_next->m_next != faceEdge) {
				dgEdge *const edge0 = AddHalfEdge(newIndex, faceEdge->m_prev->m_incidentVertex);
				dgEdge *const twin0 = AddHalfEdge(faceEdge->m_prev->m_incidentVertex, newIndex);

				twin0->m_incidentFace = faceEdge->m_incidentFace;
				faceEdge->m_prev->m_incidentFace = m_faceSecuence;
				faceEdge->m_incidentFace = m_faceSecuence;
				edge0->m_incidentFace = m_faceSecuence;
				m_faceSecuence ++;

				edge0->m_twin = twin0;
				twin0->m_twin = edge0;

				twin0->m_next = faceEdge->m_next;
				faceEdge->m_next->m_prev = twin0;

				twin0->m_prev = faceEdge->m_prev->m_prev;
				faceEdge->m_prev->m_prev->m_next = twin0;

				edge0->m_prev = faceEdge;
				faceEdge->m_next = edge0;

				edge0->m_next = faceEdge->m_prev;
				faceEdge->m_prev->m_prev = edge0;
			}
		}

		edge = edge->m_twin->m_next;
	} while (edge != ankle);

#ifdef __ENABLE_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif

	return ankle;
}

dgInt32 dgPolyhedra::GetMaxIndex() const {
	dgInt32 maxIndex;
	dgEdge *edge;

	maxIndex = -1;

	dgPolyhedra::Iterator iter(*this);
	for (iter.Begin(); iter; iter ++) {
		edge = &(*iter);
		if (edge->m_incidentVertex > maxIndex) {
			maxIndex = edge->m_incidentVertex;
		}
	}
	return (maxIndex + 1);
}

dgInt32 dgPolyhedra::GetUnboundedFaceCount() const {
	dgInt32 count;
	dgInt32 mark;
	dgEdge *ptr;
	dgEdge *edge;
	Iterator iter(*this);

	count = 0;
	mark = IncLRU();
	for (iter.Begin(); iter; iter ++) {
		edge = &(*iter);
		if (edge->m_mark == mark) {
			continue;
		}

		if (edge->m_incidentFace > 0) {
			continue;
		}

		count ++;
		ptr = edge;
		do {
			ptr->m_mark = mark;
			ptr = ptr->m_next;
		} while (ptr != edge);
	}
	return count;
}

dgInt32 dgPolyhedra::PackVertex(dgFloat32 *const destArray, const dgFloat32 *const unpackArray, dgInt32 strideInBytes) {
	NEWTON_ASSERT(0);
	return 0;
	/*
	 dgInt32 i;
	 dgInt32 mark;
	 dgInt32 stride;
	 dgInt32 maxCount;
	 dgInt32 edgeCount;
	 dgInt32 vertexCount;
	 dgEdge *ptr;
	 dgEdge *edge;
	 dgTreeNode *node;
	 dgTreeNode **tree;

	 mark = IncLRU();

	 stride = strideInBytes / sizeof (dgFloat32);

	 maxCount = GetCount();
	 dgStack<dgTreeNode *> treePool(GetCount());
	 tree = &treePool[0];

	 edgeCount = 0;
	 vertexCount = 0;
	 Iterator iter (*this);
	 for (iter.Begin(); iter; ) {
	 node = iter.GetNode();
	 iter ++;

	 tree[edgeCount] = node;
	 node->AddRef();

	 NEWTON_ASSERT (edgeCount < maxCount);
	 edgeCount ++;

	 edge = &node->GetInfo();
	 if (edge->m_mark != mark) {
	 dgInt32 index;
	 ptr = edge;
	 index = edge->m_incidentVertex;
	 memcpy (&destArray[vertexCount * stride], &unpackArray[index * stride], stride * sizeof (dgFloat32));
	 do {
	 ptr->m_mark = mark;
	 ptr->m_incidentVertex = vertexCount;
	 ptr = ptr->m_twin->m_next;

	 } while (ptr != edge);
	 vertexCount ++;
	 }
	 }

	 RemoveAll ();
	 for (i = 0; i < edgeCount; i ++) {
	 node = tree[i];
	 node->Unkill();
	 edge = &node->GetInfo();
	 dgPairKey key(edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
	 Insert (node, key.GetVal());
	 node->Release();
	 }

	 return vertexCount;
	 */
}

void dgPolyhedra::GetBadEdges(dgList<dgEdge *> &faceList, const dgFloat32 *const pool, dgInt32 strideInBytes) const {
	dgStack<char> memPool((4096 + 256) * (sizeof(dgFloat32) + sizeof(dgEdge)));
	dgDownHeap<dgEdge *, dgFloat32> heap(&memPool[0], memPool.GetSizeInBytes());

	dgPolyhedra tmp(*this);
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat32));

	dgInt32 mark = tmp.IncLRU();
	Iterator iter(tmp);
	for (iter.Begin(); iter; iter ++) {
		dgEdge *const edge = &(*iter);

		if (edge->m_mark == mark) {
			continue;
		}
		if (edge->m_incidentFace < 0) {
			continue;
		}

		dgInt32 count = 0;
		dgEdge *ptr = edge;
		do {
			count ++;
			ptr->m_mark = mark;
			ptr = ptr->m_next;
		} while (ptr != edge);

		if (count > 3) {
			dgEdge *const badEdge = InternalPolyhedra::TriangulateFace(tmp, edge, pool, stride, heap, NULL);
			if (badEdge) {
				dgEdge *const edge = FindEdge(badEdge->m_incidentVertex, badEdge->m_twin->m_incidentVertex);
				dgEdge *ptr = edge;
				do {
					ptr->m_mark = mark;
					ptr = ptr->m_next;
				} while (ptr != edge);

				NEWTON_ASSERT(edge);
				NEWTON_ASSERT(0);
				faceList.Append(edge);
			}
		}
	}
}

/*
 void dgPolyhedra::CollapseDegenerateFaces (
 dgPolyhedraDescriptor &desc,
 const dgFloat32* const pool,
 dgInt32 strideInBytes,
 dgFloat32 area)
 {
 dgInt32 i0;
 dgInt32 i1;
 dgInt32 i2;
 dgInt32 mark;
 dgInt32 stride;
 dgFloat32 cost;
 dgFloat32 area2;
 dgFloat32 e10Len;
 dgFloat32 e21Len;
 dgFloat32 e02Len;
 dgFloat32 faceArea;
 bool someChanges;
 dgEdge *ptr;
 dgEdge *face;
 dgEdge *edge;


 #ifdef __ENABLE_SANITY_CHECK
 NEWTON_ASSERT (SanityCheck ());
 #endif

 stride = strideInBytes / sizeof (dgFloat32);
 area2 = area * area;
 dgStack<char> heapPool (desc.m_faceCount * (sizeof (dgFloat32) + sizeof (dgPairKey) + sizeof (dgInt32)));
 NEWTON_ASSERT (0);
 dgDownHeap<dgPairKey, dgFloat32> bigHeapArray(&heapPool[0], heapPool.GetSizeInBytes());

 Iterator iter (*this);
 do {
 someChanges    = false;
 mark = IncLRU();
 for (iter.Begin(); iter; iter ++) {
 edge = &(*iter);

 if ((edge->m_mark != mark) && (edge->m_incidentFace > 0)) {
 NEWTON_ASSERT (edge->m_next->m_next->m_next == edge);

 edge->m_mark = mark;
 edge->m_next->m_mark = mark;
 edge->m_prev->m_mark = mark;

 i0 = edge->m_incidentVertex * stride;
 i1 = edge->m_next->m_incidentVertex * stride;
 i2 = edge->m_prev->m_incidentVertex * stride;

 dgVector p0 (&pool[i0]);
 dgVector p1 (&pool[i1]);
 dgVector p2 (&pool[i2]);

 dgVector normal ((p2 - p0) * (p1 - p0));
 faceArea = normal % normal;
 if (faceArea < area2) {
 someChanges    = true;
 dgPairKey key (edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
 bigHeapArray.Push (key, area2 - faceArea);
 }
 }
 }

 while (bigHeapArray.GetCount()) {
 // collapse this edge
 cost = area2 - bigHeapArray.Value();
 dgPairKey key (bigHeapArray[0]);
 bigHeapArray.Pop();

 //         face = FindEdge (key.i0, key.i1);
 face = FindEdge (key.GetLowKey(), key.GetHighKey());
 if (face) {
 i0 = face->m_incidentVertex * stride;
 i1 = face->m_next->m_incidentVertex * stride;
 i2 = face->m_prev->m_incidentVertex * stride;

 dgVector p0 (&pool[i0]);
 dgVector p1 (&pool[i1]);
 dgVector p2 (&pool[i2]);

 dgVector e10 (p1 - p0);
 dgVector e21 (p2 - p1);
 dgVector e02 (p0 - p2);

 e10Len = e10 % e10;
 e21Len = e21 % e21;
 e02Len = e02 % e02;


 edge = face;
 if ((e21Len < e10Len) && (e21Len < e02Len)){
 edge = face->m_next;
 }
 if ((e02Len < e10Len) && (e02Len < e21Len)){
 edge = face->m_prev;
 }
 ptr = CollapseEdge(edge);
 if (!ptr) {
 ptr = CollapseEdge(edge->m_twin);
 if (!ptr) {
 ptr = CollapseEdge(edge->m_next);
 if (!ptr) {
 ptr = CollapseEdge(edge->m_next->m_twin);
 if (!ptr) {
 ptr = CollapseEdge(edge->m_prev->m_next);
 if (!ptr) {
 ptr = CollapseEdge(edge->m_prev->m_twin);
 if (!ptr) {
 DeleteFace (edge);
 }
 }
 }
 }
 }
 }

 #ifdef __ENABLE_SANITY_CHECK
 NEWTON_ASSERT (SanityCheck ());
 #endif

 }
 }
 } while (someChanges);

 desc.Update(*this);
 }
 */

/*
 void dgPolyhedra::GetCoplanarFaces (dgList<dgEdge*>& faceList, dgEdge *startFace, const dgFloat32* const pool, dgInt32 strideInBytes, dgFloat32 normalDeviation) const
 {
 dgInt32 mark;
 dgInt32 index;
 // dgFloat64 dot;

 if (!GetCount()) {
 return;
 }

 dgStack<dgEdge*> stackPool (GetCount() / 2);
 dgEdge **const stack = &stackPool[0];

 if (startFace->m_incidentFace < 0) {
 startFace = startFace->m_twin;
 }

 NEWTON_ASSERT (startFace->m_incidentFace > 0);
 mark = IncLRU();

 dgBigVector normal (FaceNormal (startFace, pool, strideInBytes));
 dgFloat64 dot = normal % normal;
 if (dot < dgFloat64 (1.0e-12f)) {
 dgEdge* ptr = startFace;
 do {
 ptr->m_mark = mark;
 ptr = ptr->m_next;
 } while (ptr != startFace);

 NEWTON_ASSERT (0);
 faceList.Append (startFace);
 return;
 }
 normal = normal.Scale (dgFloat64 (1.0) / sqrt (dot));


 stack[0] = startFace;
 index = 1;
 while (index) {
 index --;
 dgEdge* const edge = stack[index];

 if (edge->m_mark == mark) {
 NEWTON_ASSERT (0u);
 continue;
 }

 dgBigVector normal1 (FaceNormal (edge, pool, strideInBytes));
 dot = normal1 % normal1;
 if (dot > dgFloat64 (1.0e-12f)) {
 normal1 = normal1.Scale (dgFloat64 (1.0) / sqrt (dot));
 }

 dot = normal1 % normal;
 if (dot >= normalDeviation) {
 dgEdge* ptr = edge;
 do {
 ptr->m_mark = mark;

 if ((ptr->m_twin->m_incidentFace > 0) && (ptr->m_twin->m_mark != mark)) {
 stack[index] = ptr->m_twin;
 index ++;
 NEWTON_ASSERT (index < GetCount() / 2);
 }
 ptr = ptr->m_next;
 } while (ptr != edge);

 NEWTON_ASSERT (0);
 faceList.Append (edge);
 }
 }
 }



 void dgPolyhedra::DeleteOverlapingFaces (
 const dgFloat32* const pool,
 dgInt32 strideInBytes,
 dgFloat32 distTol__)
 {
 dgInt32 i;
 dgInt32 mark;
 dgInt32 perimeterCount;
 dgEdge *edge;
 dgPolyhedra *perimeters;

 if (!GetCount()) {
 return;
 }

 dgStack<dgInt32>faceIndexPool (4096);
 dgStack<dgEdge*> stackPool (GetCount() / 2 + 100);
 dgStack<dgPolyhedra> flatPerimetersPool (GetCount() / 3 + 100);

 perimeterCount = 0;
 perimeters = &flatPerimetersPool[0];

 mark = IncLRU();
 Iterator iter (*this);
 for (iter.Begin(); iter; iter++) {
 edge = &(*iter);
 if (edge->m_incidentFace < 0) {
 continue;
 }

 if (edge->m_mark >= mark) {
 continue;
 }

 dgPolyhedra dommy(GetAllocator());
 perimeters[perimeterCount] = dommy;
 InternalPolyhedra::GetAdjacentCoplanarFacesPerimeter (perimeters[perimeterCount], *this, edge, pool, strideInBytes, &stackPool[0], &faceIndexPool[0]);
 perimeterCount ++;
 }

 // write code here


 for (i = 0 ; i < perimeterCount; i ++) {
 perimeters[i].DeleteAllFace();
 }
 }


 void dgPolyhedra::InvertWinding ()
 {
 dgStack<dgInt32> vertexData(1024 * 4);
 dgStack<dgInt64> userData(1024 * 4);

 dgPolyhedra tmp (*this);

 RemoveAll();

 BeginFace();
 dgInt32 mark = tmp.IncLRU();
 Iterator iter (tmp);
 for (iter.Begin(); iter; iter ++) {
 dgEdge* const edge = &(*iter);

 if (edge->m_incidentFace < 0) {
 continue;
 }
 if (edge->m_mark == mark) {
 continue;
 }

 dgInt32 count = 0;
 dgEdge* ptr = edge;
 do {
 userData[count] = dgInt32 (ptr->m_userData);
 vertexData[count] = ptr->m_incidentVertex;
 count ++;
 NEWTON_ASSERT (count < 1024 * 4);

 ptr->m_mark = mark;
 ptr = ptr->m_prev;
 } while (ptr != edge);

 AddFace(count, &vertexData[0], &userData[0]);
 }
 EndFace();

 NEWTON_ASSERT (SanityCheck());

 }
 */

/*

 void dgPolyhedra::Quadrangulate (const dgFloat32* const vertex, dgInt32 strideInBytes)
 {
 dgInt32 mark;
 dgInt32 stride;
 dgUnsigned32 cost;
 dgUnsigned32 maxCost;
 dgTree<dgEdge*, dgInt64> essensialEdge;

 Iterator iter (*this);
 for (iter.Begin(); iter; iter ++) {
 dgEdge *edge;
 edge = &(*iter);
 dgPairKey code (edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
 essensialEdge.Insert (edge, code.GetVal());
 }

 Triangulate (vertex, strideInBytes);

 stride = strideInBytes / sizeof (dgFloat32);
 dgHeap<dgEdge*, dgUnsigned32> heapCost (GetCount(), 0xffffffff);
 maxCost = 1<<30;
 mark = IncLRU();
 for (iter.Begin(); iter; iter ++) {
 dgEdge *edge;
 dgEdge *twin;

 dgUnsigned32 edgeCost;
 dgUnsigned32 twinCost;
 dgUnsigned32 shapeCost;
 dgUnsigned32 normalCost;
 dgFloat32 normalDot;
 dgFloat32 edgeAngle0;
 dgFloat32 edgeAngle1;
 dgFloat32 twinAngle0;
 dgFloat32 twinAngle1;
 dgFloat32 shapeFactor;
 dgFloat32 medianAngle;

 dgTree<dgEdge*, dgInt64>::dgTreeNode *essencial;

 edge = &(*iter);
 twin = edge->m_twin;

 if (edge->m_mark == mark) {
 continue;
 }

 if ((edge->m_incidentFace < 0) || (twin->m_incidentFace < 0)) {
 continue;
 }

 edge->m_mark = mark;
 twin->m_mark = mark;

 dgVector edgeNormal (FaceNormal (edge, vertex, strideInBytes));
 if ((edgeNormal % edgeNormal) < 1.0e-8) {
 continue;
 }
 dgVector twinNormal (FaceNormal (twin, vertex, strideInBytes));

 if ((twinNormal % twinNormal) < 1.0e-8) {
 continue;
 }

 edgeNormal = edgeNormal.Scale (dgRsqrt (edgeNormal % edgeNormal));
 twinNormal = twinNormal.Scale (dgRsqrt (twinNormal % twinNormal));

 normalDot = edgeNormal % twinNormal;
 normalCost = ClampValue (2048 - (dgInt32)(2048.0f * normalDot), 0, 2048);
 if (normalCost > 600) {
 normalCost = 4000000;
 }

 dgVector p0 (&vertex[edge->m_incidentVertex * stride]);
 dgVector p1 (&vertex[edge->m_twin->m_incidentVertex * stride]);
 dgVector p2 (&vertex[edge->m_prev->m_incidentVertex * stride]);
 dgVector p3 (&vertex[twin->m_prev->m_incidentVertex * stride]);

 dgVector e10 (p1 - p0);
 dgVector e20 (p2 - p0);
 dgVector e30 (p3 - p0);

 e10 = e10.Scale (dgRsqrt ((e10 % e10) + 1.e-10f));
 e20 = e20.Scale (dgRsqrt ((e20 % e20) + 1.e-10f));
 e30 = e30.Scale (dgRsqrt ((e30 % e30) + 1.e-10f));

 edgeAngle0 = dgRAD2DEG * dgAtan2 (edgeNormal % (e10 * e20), e20 % e10);
 edgeAngle1 = dgRAD2DEG * dgAtan2 (twinNormal % (e30 * e10), e10 % e30);

 if ((edgeAngle0 + edgeAngle1) < 160.0f) {
 NEWTON_ASSERT ((edgeAngle0 + edgeAngle1) > 0.0f);
 medianAngle = 4.0f * edgeAngle0 * edgeAngle1 / (edgeAngle0 + edgeAngle1);

 NEWTON_ASSERT (medianAngle > 0.0f);
 NEWTON_ASSERT (medianAngle < 360.0f);
 edgeCost = abs (ClampValue (90 - (dgInt32)medianAngle, -90, 90));
 } else {
 edgeCost   = 4000000;
 }

 dgVector t10 (p0 - p1);
 dgVector t20 (p3 - p1);
 dgVector t30 (p2 - p1);

 t10 = t10.Scale (dgRsqrt ((t10 % t10) + 1.e-10f));
 t20 = t20.Scale (dgRsqrt ((t20 % t20) + 1.e-10f));
 t30 = t30.Scale (dgRsqrt ((t30 % t30) + 1.e-10f));

 twinAngle0 = dgRAD2DEG * dgAtan2 (twinNormal % (t10 * t20), t20 % t10);
 twinAngle1 = dgRAD2DEG * dgAtan2 (edgeNormal % (t30 * t10), t10 % t30);

 if ((twinAngle0 + twinAngle1) < 160.0f) {
 NEWTON_ASSERT ((twinAngle0 + twinAngle1) > 0.0f);
 medianAngle = 4.0f * twinAngle0 * twinAngle1 / (twinAngle0 + twinAngle1);

 NEWTON_ASSERT (medianAngle > 0.0f);
 NEWTON_ASSERT (medianAngle < 360.0f);
 twinCost = abs (ClampValue (90 - (dgInt32)medianAngle, -90, 90));
 } else {
 twinCost   = 4000000;
 }


 dgFloat32 a0;
 dgFloat32 a1;
 dgFloat32 a2;
 dgFloat32 a3;
 dgFloat32 angleSum;
 dgFloat32 angleSum2;

 a0 = edgeAngle0 + edgeAngle1;
 a1 = twinAngle0 + twinAngle1;

 dgVector oe10 (p0 - p2);
 dgVector oe20 (p1 - p2);

 oe10 = oe10.Scale (dgRsqrt ((oe10 % oe10) + 1.e-10f));
 oe20 = oe20.Scale (dgRsqrt ((oe20 % oe20) + 1.e-10f));
 a2 = dgRAD2DEG * dgAtan2 (edgeNormal % (oe10 * oe20), oe20 % oe10);

 dgVector ot10 (p1 - p3);
 dgVector ot20 (p0 - p3);

 ot10 = ot10.Scale (dgRsqrt ((ot10 % ot10) + 1.e-10f));
 ot20 = ot20.Scale (dgRsqrt ((ot20 % ot20) + 1.e-10f));
 a3 = dgRAD2DEG * dgAtan2 (twinNormal % (ot10 * ot20), ot20 % ot10);

 angleSum   = (a0 + a1 + a2 + a3) * 0.25f;
 angleSum2 = (a0 * a0 + a1 * a1 + a2 * a2 + a3 * a3) * 0.25f;
 shapeFactor = powf (dgSqrt (angleSum2 - angleSum * angleSum), 1.25f);

 shapeCost = ClampValue ((dgInt32)(shapeFactor * 4.0f), 0, 4096);

 cost = normalCost + edgeCost + twinCost + shapeCost;
 dgPairKey code (edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
 essencial = essensialEdge.Find(code.GetVal());
 if (essencial) {
 cost += 1000;
 }
 heapCost.Push (edge, maxCost - cost);
 }


 mark = IncLRU();
 while (heapCost.GetCount ()) {
 dgUnsigned32 cost;
 dgEdge *edge;

 edge = heapCost[0];
 cost = maxCost - heapCost.Value ();
 if (cost   > 100000) {
 break;
 }

 heapCost.Pop();

 if (edge->m_mark != mark) {
 edge->m_mark = mark;
 edge->m_twin->m_mark = mark;

 edge->m_next->m_mark = mark;
 edge->m_next->m_twin->m_mark = mark;

 edge->m_prev->m_mark = mark;
 edge->m_prev->m_twin->m_mark = mark;

 edge->m_twin->m_next->m_mark = mark;
 edge->m_twin->m_next->m_twin->m_mark = mark;

 edge->m_twin->m_prev->m_mark = mark;
 edge->m_twin->m_prev->m_twin->m_mark = mark;

 DeleteEdge (edge);
 }

 }

 //#ifdef _DEBUG
 //mark = IncLRU();
 //for (iter.Begin(); iter; iter ++) {
 // dgEdge *edge;
 // edge = &(*iter);
 //
 // if (edge->m_incidentFace > 0) {
 //     if (edge->m_mark != mark) {
 //         dgEdge *ptr;
 //         ptr = edge;
 //         do {
 //             ptr->m_mark = mark;
 //             dgTrace (("%d ", ptr->m_incidentVertex));
 //             ptr = ptr->m_next;
 //         } while(ptr != edge);
 //         dgTrace (("\n"));
 //
 //     }
 // }
 //}
 //NEWTON_ASSERT (0);
 //#endif

 }

 void dgPolyhedra::OptimalTriangulation (const dgFloat32* const vertex, dgInt32 strideInBytes)
 {
 dgInt32 color;
 dgEdge *edge;
 dgList<dgEdge*> edgeStart;

 Quadrangulate (vertex, strideInBytes);

 color = IncLRU();
 dgTree<dgEdge*, dgInt64> faceList;
 Iterator iter (*this);
 for (iter.Begin(); iter; iter ++) {
 dgInt32 min;
 dgInt32 count;
 dgEdge *ptr;
 dgEdge *start;

 edge = &(*iter);
 if (edge->m_mark == color) {
 continue;
 }
 if (edge->m_incidentFace < 0) {
 edge->m_mark = color + 1;
 continue;
 }

 count  = 0;
 min = 0x7fffffff;
 start  = edge;
 ptr = edge;
 do {
 count  ++;
 ptr->m_mark = color;
 if (ptr->m_incidentVertex < min) {
 start  = ptr;
 min = ptr->m_incidentVertex;
 }
 ptr = ptr->m_next;
 } while (ptr != edge);
 if (count == 4) {
 dgPairKey  key (start->m_incidentVertex, start->m_twin->m_incidentVertex);
 faceList.Insert (start, key.GetVal());
 }
 }

 color = IncLRU();
 for (edge = InternalPolyhedra::FindQuadStart(faceList, color); edge; edge = InternalPolyhedra::FindQuadStart(faceList, color)) {
 InternalPolyhedra::TriangleQuadStrips (*this, faceList, edge, color);
 }

 #ifdef _DEBUG
 for (iter.Begin(); iter; iter ++) {
 edge = &(*iter);
 if (edge->m_incidentFace > 0)
 NEWTON_ASSERT (edge->m_next->m_next->m_next == edge);
 }
 #endif
 }


 dgInt32 dgPolyhedra::TriangleStrips (
 dgUnsigned32 outputBuffer[],
 dgInt32 maxBufferSize,
 dgInt32 vertexCacheSize) const
 {
 dgInt32 setMark;
 dgInt32 indexCount;
 dgInt32 stripsIndex;
 dgInt32 faceColorMark;
 dgInt32 debugFaceCount;
 dgInt32 debugIndexCount;

 dgEdge *edge;
 InternalPolyhedra::VertexCache vertexCache(vertexCacheSize);

 dgPolyhedra tmp(*this);

 tmp.IncLRU();
 faceColorMark = tmp.IncLRU();
 tmp.IncLRU();
 setMark = tmp.IncLRU();

 debugFaceCount = 0;
 debugIndexCount = 0;

 indexCount = 0;

 for (edge = InternalPolyhedra::FindStripStart(tmp, faceColorMark, vertexCache); edge; edge = InternalPolyhedra::FindStripStart(tmp, faceColorMark, vertexCache)) {
 stripsIndex = InternalPolyhedra::TriangleStrips (edge, setMark, &outputBuffer[indexCount + 1], vertexCache);

 debugFaceCount += stripsIndex  - 2;
 debugIndexCount += stripsIndex;

 if (stripsIndex > 0)   {
 outputBuffer[indexCount] = stripsIndex;
 indexCount += stripsIndex + 1;
 if (indexCount > maxBufferSize) {
 break;
 }
 }
 }

 dgTrace (("index per triangles %f\n", dgFloat32 (debugIndexCount) / dgFloat32 (debugFaceCount)));

 return indexCount;
 }
 */

/*
 dgInt32 dgPolyhedra::TriangleList (
 dgUnsigned32 outputBuffer[],
 dgInt32 maxSize,
 dgInt32 vertexCacheSize) const
 {
 dgInt32 mark;
 dgInt32 cost;
 dgInt32 count;
 dgInt32 vertex;
 dgInt32 cacheHit;
 dgInt32 tmpVertex;
 dgInt32 cacheMiss;
 dgInt32 twinVertex;
 dgInt32 lowestPrize;
 dgInt64 key;
 dgEdge *ptr;
 dgEdge *edge;
 dgList<dgInt32> vertexCache;
 dgTree<dgEdge*, dgInt64> edgeList;
 dgTree<dgEdge*, dgInt64>::dgTreeNode *node;
 dgTree<dgEdge*, dgInt64>::dgTreeNode *bestEdge;


 cacheHit = 0;
 cacheMiss = 0;

 Iterator iter (*this);
 for (iter.Begin(); iter; iter ++) {
 edge = &(*iter);
 if (edge->m_incidentFace > 0) {
 edgeList.Insert (edge, iter.GetNode()->GetKey());
 }
 }

 count = 0;
 mark = IncLRU();
 while (edgeList) {

 node = edgeList.Minimum();
 edge = node->GetInfo();
 ptr = edge;
 do {
 ptr->m_mark = mark;

 vertex = ptr->m_incidentVertex;
 if (count < maxSize) {
 outputBuffer[count] = vertex;
 count ++;
 }
 cacheMiss ++;
 vertexCache.Append (vertex);
 if (vertexCache.GetCount() > vertexCacheSize) {
 vertexCache.Remove (vertexCache.GetFirst());
 }
 edgeList.Remove(GetNodeFromInfo(*ptr)->GetKey());

 ptr = ptr->m_next;
 } while (ptr != edge);

 dgList<dgInt32>::Iterator vertexIter(vertexCache);
 for (vertexIter.Begin(); vertexIter; ) {

 vertex = *vertexIter;
 vertexIter ++;

 key = vertex;
 key <<= 32;

 bestEdge   = NULL;
 lowestPrize = 100000;

 node = edgeList.FindGreaterEqual (key);
 dgTree<dgEdge *, dgInt64>::Iterator iter(edgeList);
 for (iter.Set(node); iter; iter ++) {
 node = iter.GetNode();
 key = node->GetKey();
 key >>= 32;
 if (key > vertex) {
 break;
 }

 ptr = node->GetInfo();

 NEWTON_ASSERT (ptr->m_mark != mark);
 NEWTON_ASSERT (ptr->m_twin->m_incidentVertex == vertex);


 twinVertex = ptr->m_twin->m_incidentVertex;
 dgList<dgInt32>::Iterator vertexIter(vertexCache);
 cost = 0;
 for (vertexIter.Begin(); vertexIter; vertexIter ++) {
 tmpVertex = *vertexIter;
 if (twinVertex == tmpVertex) {
 break;
 };
 cost ++;
 }
 if (cost < lowestPrize) {
 lowestPrize    = cost;
 bestEdge   = node;
 }
 }

 if (bestEdge) {
 edge = bestEdge->GetInfo();

 ptr = edge;
 do {
 ptr->m_mark = mark;
 vertex = ptr->m_incidentVertex;
 if (count < maxSize) {
 outputBuffer[count] = vertex;
 count ++;
 }

 edgeList.Remove(GetNodeFromInfo(*ptr)->GetKey());

 dgList<dgInt32>::Iterator vertexIter(vertexCache);
 for (vertexIter.Begin(); vertexIter; vertexIter++) {
 tmpVertex = *vertexIter;
 if (vertex == tmpVertex) {
 cacheHit ++;
 break;
 }
 }

 if (!vertexIter) {
 cacheMiss ++;
 vertexCache.Append (vertex);
 if (vertexCache.GetCount() > vertexCacheSize) {
 vertexCache.Remove (vertexCache.GetFirst());
 }
 }

 ptr = ptr->m_next;
 } while (ptr != edge);

 vertexIter.Begin();
 }
 }
 }

 // dgTrace ("cacheHit = %d, cacheMiss = %d, total = %d\n", cacheHit, cacheMiss, cacheMiss + cacheHit);

 return count;
 }

 */

dgInt32 dgPolyhedra::TriangleList(dgUnsigned32 outputBuffer[], dgInt32 maxSize__, dgInt32 vertexCacheSize__) const {
	dgInt32 mark;
	dgInt32 count;
	dgInt32 cacheMiss;
	dgInt32 score;
	dgInt32 bestScore;
	dgEdge *ptr;
	dgEdge *edge;
	dgEdge *face;
	dgTree<dgEdge *, dgInt32> vertexIndex(GetAllocator());
	InternalPolyhedra::VertexCache vertexCache(16, GetAllocator());

	Iterator iter(*this);
	for (iter.Begin(); iter; iter ++) {
		edge = &(*iter);
		vertexIndex.Insert(edge, edge->m_incidentVertex);
	}
	count = 0;
	cacheMiss = 0;;

	mark = IncLRU();
	while (vertexIndex.GetCount()) {
		edge = vertexCache.GetEdge(mark);
		if (!edge) {
			dgTree<dgEdge *, dgInt32>::dgTreeNode *node;
			dgTree<dgEdge *, dgInt32>::Iterator iter(vertexIndex);
			for (iter.Begin(); iter;) {
				node = iter.GetNode();
				iter ++;
				ptr = node->GetInfo();;
				edge = ptr;

				do {
					if (edge->m_incidentFace > 0) {
						if (edge->m_mark != mark) {
							goto newEdge;
						}
					}
					edge = edge->m_twin->m_next;
				} while (edge != ptr);
				vertexIndex.Remove(node);
			}
			edge = NULL;
		}
newEdge:

		face = NULL;
		bestScore = -1;
		if (edge) {
			ptr = edge;
			do {
				if (ptr->m_incidentFace > 0) {
					if (ptr->m_mark != mark) {
						score = vertexCache.IsInCache(ptr->m_next) + vertexCache.IsInCache(ptr->m_prev);
						if (score > bestScore) {
							bestScore = score;
							face = ptr;
						}
					}
				}

				ptr = ptr->m_twin->m_next;
			} while (ptr != edge);

			NEWTON_ASSERT(face);
			ptr = face;
			do {
				outputBuffer[count] = dgUnsigned32(ptr->m_incidentVertex);
				count ++;
				cacheMiss += vertexCache.AddEdge(ptr);
				ptr->m_mark = mark;
				ptr = ptr->m_next;
			} while (ptr != face);
		}
	}

	// add all legacy faces
	for (iter.Begin(); iter; iter ++) {
		edge = &(*iter);
		if (edge->m_mark != mark) {
			if (edge->m_incidentFace > 0) {
				ptr = edge;
				do {
					outputBuffer[count] = dgUnsigned32(ptr->m_incidentVertex);
					count ++;
					cacheMiss ++;
					ptr->m_mark = mark;
					ptr = ptr->m_next;
				} while (ptr != edge);
			}
		}
	}

	dgTrace(("fifo efficiency %f\n", dgFloat32(cacheMiss) * 3.0f / dgFloat32(count)));

	return count;
}

void dgPolyhedra::SwapInfo(dgPolyhedra &source) {
	dgTree<dgEdge, dgEdgeKey>::SwapInfo((dgTree<dgEdge, dgEdgeKey> &)source);

	Swap(m_baseMark, source.m_baseMark);
	Swap(m_edgeMark, source.m_edgeMark);
	Swap(m_faceSecuence, source.m_faceSecuence);

}

void dgPolyhedra::GetOpenFaces(dgList<dgEdge *> &faceList) const {
	dgInt32 mark = IncLRU();
	//    dgList<dgEdge*> openfaces (GetAllocator());
	Iterator iter(*this);
	for (iter.Begin(); iter; iter ++) {
		dgEdge *edge = &(*iter);
		if ((edge->m_mark != mark) && (edge->m_incidentFace < 0)) {
			dgEdge *ptr = edge;
			faceList.Append(edge);
			do {
				ptr->m_mark = mark;

				ptr = ptr->m_next;
			} while (ptr != edge);
		}
	}
}

/*
 bool dgPolyhedra::TriangulateFace (dgEdge* face, const dgFloat32* const vertex, dgInt32 strideInBytes, dgVector& normal)
 {
 dgInt32 memPool [2048];
 dgDownHeap<dgEdge*, dgFloat32> heap(&memPool[0], sizeof (memPool));


 dgInt32 stride = dgInt32 (strideInBytes / sizeof (dgFloat32));
 return InternalPolyhedra::TriangulateFace (*this, face, vertex, stride, heap, &normal) ? false : true;
 }
 */

#endif

dgPolyhedra::dgPolyhedra(dgMemoryAllocator *const allocator) :
	dgTree<dgEdge, dgInt64>(allocator), m_baseMark(0), m_edgeMark(0), m_faceSecuence(
	    0) {
}

dgPolyhedra::dgPolyhedra(const dgPolyhedra &polyhedra) :
	dgTree<dgEdge, dgInt64>(polyhedra.GetAllocator()), m_baseMark(0), m_edgeMark(
	    0), m_faceSecuence(0) {
	dgStack<dgInt32> indexPool(1024 * 16);
	dgStack<dgUnsigned64> userPool(1024 * 16);
	dgInt32 *const index = &indexPool[0];
	dgUnsigned64 *const user = &userPool[0];

	BeginFace();
	Iterator iter(polyhedra);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_incidentFace < 0) {
			continue;
		}

		if (!FindEdge(edge->m_incidentVertex, edge->m_twin->m_incidentVertex)) {
			dgInt32 indexCount = 0;
			dgEdge *ptr = edge;
			do {
				user[indexCount] = ptr->m_userData;
				index[indexCount] = ptr->m_incidentVertex;
				indexCount++;
				ptr = ptr->m_next;
			} while (ptr != edge);


			dgEdge *const face = AddFace(indexCount, index, (dgInt64 *) user);
			ptr = face;
			do {
				ptr->m_incidentFace = edge->m_incidentFace;
				ptr = ptr->m_next;
			} while (ptr != face);
		}
	}
	EndFace();

	m_faceSecuence = polyhedra.m_faceSecuence;

#ifdef __ENABLE_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
}

dgPolyhedra::~dgPolyhedra() {
}

dgInt32 dgPolyhedra::GetFaceCount() const {
	Iterator iter(*this);
	dgInt32 count = 0;
	dgInt32 mark = IncLRU();
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_mark == mark) {
			continue;
		}

		if (edge->m_incidentFace < 0) {
			continue;
		}

		count++;
		dgEdge *ptr = edge;
		do {
			ptr->m_mark = mark;
			ptr = ptr->m_next;
		} while (ptr != edge);
	}
	return count;
}

dgEdge *dgPolyhedra::AddFace(dgInt32 count, const dgInt32 *const index,
                             const dgInt64 *const userdata) {
	class IntersectionFilter {
	public:
		IntersectionFilter() {
			m_count = 0;
		}

		bool Insert(dgInt32 dummy, dgInt64 value) {
			dgInt32 i;
			for (i = 0; i < m_count; i++) {
				if (m_array[i] == value) {
					return false;
				}
			}
			m_array[i] = value;
			m_count++;
			return true;
		}

		dgInt32 m_count;
		dgInt64 m_array[2048];
	};

	IntersectionFilter selfIntersectingFaceFilter;

	dgInt32 dummyValues = 0;
	dgInt32 i0 = index[count - 1];
	for (dgInt32 i = 0; i < count; i++) {
		dgInt32 i1 = index[i];
		dgPairKey code0(i0, i1);

		if (!selfIntersectingFaceFilter.Insert(dummyValues, code0.GetVal())) {
			return NULL;
		}

		dgPairKey code1(i1, i0);
		if (!selfIntersectingFaceFilter.Insert(dummyValues, code1.GetVal())) {
			return NULL;
		}

		if (i0 == i1) {
			return NULL;
		}
		if (FindEdge(i0, i1)) {
			return NULL;
		}
		i0 = i1;
	}

	m_faceSecuence++;

	i0 = index[count - 1];
	dgInt32 i1 = index[0];
	dgUnsigned64 udata0 = 0;
	dgUnsigned64 udata1 = 0;
	if (userdata) {
		udata0 = dgUnsigned64(userdata[count - 1]);
		udata1 = dgUnsigned64(userdata[0]);
	}

	bool state;
	dgPairKey code(i0, i1);
	dgEdge tmpEdge(i0, m_faceSecuence, udata0);
	dgTreeNode *node = Insert(tmpEdge, code.GetVal(), state);
	NEWTON_ASSERT(!state);
	dgEdge *edge0 = &node->GetInfo();
	dgEdge *const first = edge0;

	for (dgInt32 i = 1; i < count; i++) {
		i0 = i1;
		i1 = index[i];
		udata0 = udata1;
		udata1 = dgUnsigned64(userdata ? userdata[i] : 0);

		dgPairKey codeI(i0, i1);
		dgEdge tmpEdgeI(i0, m_faceSecuence, udata0);
		node = Insert(tmpEdgeI, codeI.GetVal(), state);
		NEWTON_ASSERT(!state);

		dgEdge *const edge1 = &node->GetInfo();
		edge0->m_next = edge1;
		edge1->m_prev = edge0;
		edge0 = edge1;
	}

	first->m_prev = edge0;
	edge0->m_next = first;

	return first->m_next;
}

void dgPolyhedra::EndFace() {
	dgPolyhedra::Iterator iter(*this);

	// Connect all twin edge
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (!edge->m_twin) {
			edge->m_twin = FindEdge(edge->m_next->m_incidentVertex,
			                        edge->m_incidentVertex);
			if (edge->m_twin) {
				edge->m_twin->m_twin = edge;
			}
		}
	}

#ifdef __ENABLE_SANITY_CHECK
	//NEWTON_ASSERT (polyhedra->SanityCheck());
	NEWTON_ASSERT(SanityCheck());
#endif
	dgStack<dgEdge *> edgeArrayPool(GetCount() * 2 + 256);

	dgInt32 edgeCount = 0;
	dgEdge **const edgeArray = &edgeArrayPool[0];
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (!edge->m_twin) {
			bool state;
			dgPolyhedra::dgPairKey code(edge->m_next->m_incidentVertex,
			                            edge->m_incidentVertex);
			dgEdge tmpEdge(edge->m_next->m_incidentVertex, -1);
			tmpEdge.m_incidentFace = -1;
			dgPolyhedra::dgTreeNode *const node = Insert(tmpEdge, code.GetVal(),
			                                      state);
			NEWTON_ASSERT(!state);
			edge->m_twin = &node->GetInfo();
			edge->m_twin->m_twin = edge;
			edgeArray[edgeCount] = edge->m_twin;
			edgeCount++;
		}
	}

	for (dgInt32 i = 0; i < edgeCount; i++) {
		dgEdge *const edge = edgeArray[i];
		NEWTON_ASSERT(!edge->m_prev);
		dgEdge *ptr = edge->m_twin;
		for (; ptr->m_next; ptr = ptr->m_next->m_twin) {
		}
		ptr->m_next = edge;
		edge->m_prev = ptr;
	}

#ifdef __ENABLE_SANITY_CHECK
// NEWTON_ASSERT (polyhedra->SanityCheck ());
	NEWTON_ASSERT(SanityCheck());
#endif
}

void dgPolyhedra::DeleteFace(dgEdge *const face) {
	dgEdge *edgeList[1024 * 16];

	if (face->m_incidentFace > 0) {
		dgInt32 count = 0;
		dgEdge *ptr = face;
		do {
			ptr->m_incidentFace = -1;
			dgInt32 i = 0;
			for (; i < count; i++) {
				if ((edgeList[i] == ptr) || (edgeList[i]->m_twin == ptr)) {
					break;
				}
			}
			if (i == count) {
				edgeList[count] = ptr;
				count++;
			}
			ptr = ptr->m_next;
		} while (ptr != face);

		for (dgInt32 i = 0; i < count; i++) {
			dgEdge *const ptrI = edgeList[i];
			if (ptrI->m_twin->m_incidentFace < 0) {
				DeleteEdge(ptrI);
			}
		}
	}
}

dgBigVector dgPolyhedra::FaceNormal(dgEdge *const face,
                                    const dgFloat64 *const pool, dgInt32 strideInBytes) const {
	dgInt32 stride = strideInBytes / sizeof(dgFloat64);
	dgEdge *edge = face;
	dgBigVector p0(&pool[edge->m_incidentVertex * stride]);
	edge = edge->m_next;
	dgBigVector p1(&pool[edge->m_incidentVertex * stride]);
	dgBigVector e1(p1 - p0);

	dgBigVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
	                   dgFloat32(0.0f));
	for (edge = edge->m_next; edge != face; edge = edge->m_next) {
		dgBigVector p2(&pool[edge->m_incidentVertex * stride]);
		dgBigVector e2(p2 - p0);
		normal += e1 * e2;
		e1 = e2;
	}
	return normal;
}

dgEdge *dgPolyhedra::AddHalfEdge(dgInt32 v0, dgInt32 v1) {
	if (v0 != v1) {
		dgPairKey pairKey(v0, v1);
		dgEdge tmpEdge(v0, -1);

		dgTreeNode *node = Insert(tmpEdge, pairKey.GetVal());
		return node ? &node->GetInfo() : NULL;
	} else {
		return NULL;
	}
}

void dgPolyhedra::DeleteEdge(dgEdge *const edge) {
	dgEdge *const twin = edge->m_twin;

	edge->m_prev->m_next = twin->m_next;
	twin->m_next->m_prev = edge->m_prev;
	edge->m_next->m_prev = twin->m_prev;
	twin->m_prev->m_next = edge->m_next;

	dgTreeNode *const nodeA = GetNodeFromInfo(*edge);
	dgTreeNode *const nodeB = GetNodeFromInfo(*twin);

	NEWTON_ASSERT(&nodeA->GetInfo() == edge);
	NEWTON_ASSERT(&nodeB->GetInfo() == twin);

	Remove(nodeA);
	Remove(nodeB);
}

dgEdge *dgPolyhedra::SpliteEdge(dgInt32 newIndex, dgEdge *const edge) {
	dgEdge *const edge00 = edge->m_prev;
	dgEdge *const edge01 = edge->m_next;
	dgEdge *const twin00 = edge->m_twin->m_next;
	dgEdge *const twin01 = edge->m_twin->m_prev;

	dgInt32 i0 = edge->m_incidentVertex;
	dgInt32 i1 = edge->m_twin->m_incidentVertex;

	dgInt32 f0 = edge->m_incidentFace;
	dgInt32 f1 = edge->m_twin->m_incidentFace;

	DeleteEdge(edge);

	dgEdge *const edge0 = AddHalfEdge(i0, newIndex);
	dgEdge *const edge1 = AddHalfEdge(newIndex, i1);

	dgEdge *const twin0 = AddHalfEdge(newIndex, i0);
	dgEdge *const twin1 = AddHalfEdge(i1, newIndex);
	NEWTON_ASSERT(edge0);
	NEWTON_ASSERT(edge1);
	NEWTON_ASSERT(twin0);
	NEWTON_ASSERT(twin1);

	edge0->m_twin = twin0;
	twin0->m_twin = edge0;

	edge1->m_twin = twin1;
	twin1->m_twin = edge1;

	edge0->m_next = edge1;
	edge1->m_prev = edge0;

	twin1->m_next = twin0;
	twin0->m_prev = twin1;

	edge0->m_prev = edge00;
	edge00->m_next = edge0;

	edge1->m_next = edge01;
	edge01->m_prev = edge1;

	twin0->m_next = twin00;
	twin00->m_prev = twin0;

	twin1->m_prev = twin01;
	twin01->m_next = twin1;

	edge0->m_incidentFace = f0;
	edge1->m_incidentFace = f0;

	twin0->m_incidentFace = f1;
	twin1->m_incidentFace = f1;

#ifdef __ENABLE_SANITY_CHECK
	//    NEWTON_ASSERT (SanityCheck ());
#endif

	return edge0;
}

bool dgPolyhedra::FlipEdge(dgEdge *const edge) {
	//    dgTreeNode *node;
	if (edge->m_next->m_next->m_next != edge) {
		return false;
	}

	if (edge->m_twin->m_next->m_next->m_next != edge->m_twin) {
		return false;
	}

	if (FindEdge(edge->m_prev->m_incidentVertex,
	             edge->m_twin->m_prev->m_incidentVertex)) {
		return false;
	}

	dgEdge *const prevEdge = edge->m_prev;
	dgEdge *const prevTwin = edge->m_twin->m_prev;

	dgPairKey edgeKey(prevTwin->m_incidentVertex, prevEdge->m_incidentVertex);
	dgPairKey twinKey(prevEdge->m_incidentVertex, prevTwin->m_incidentVertex);

	ReplaceKey(GetNodeFromInfo(*edge), edgeKey.GetVal());
	//    NEWTON_ASSERT (node);

	ReplaceKey(GetNodeFromInfo(*edge->m_twin), twinKey.GetVal());
	//    NEWTON_ASSERT (node);

	edge->m_incidentVertex = prevTwin->m_incidentVertex;
	edge->m_twin->m_incidentVertex = prevEdge->m_incidentVertex;

	edge->m_userData = prevTwin->m_userData;
	edge->m_twin->m_userData = prevEdge->m_userData;

	prevEdge->m_next = edge->m_twin->m_next;
	prevTwin->m_prev->m_prev = edge->m_prev;

	prevTwin->m_next = edge->m_next;
	prevEdge->m_prev->m_prev = edge->m_twin->m_prev;

	edge->m_prev = prevTwin->m_prev;
	edge->m_next = prevEdge;

	edge->m_twin->m_prev = prevEdge->m_prev;
	edge->m_twin->m_next = prevTwin;

	prevTwin->m_prev->m_next = edge;
	prevTwin->m_prev = edge->m_twin;

	prevEdge->m_prev->m_next = edge->m_twin;
	prevEdge->m_prev = edge;

	edge->m_next->m_incidentFace = edge->m_incidentFace;
	edge->m_prev->m_incidentFace = edge->m_incidentFace;

	edge->m_twin->m_next->m_incidentFace = edge->m_twin->m_incidentFace;
	edge->m_twin->m_prev->m_incidentFace = edge->m_twin->m_incidentFace;

#ifdef __ENABLE_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif

	return true;
}

bool dgPolyhedra::GetConectedSurface(dgPolyhedra &polyhedra) const {
	if (!GetCount()) {
		return false;
	}

	dgEdge *edge = NULL;
	Iterator iter(*this);
	for (iter.Begin(); iter; iter++) {
		edge = &(*iter);
		if ((edge->m_mark < m_baseMark) && (edge->m_incidentFace > 0)) {
			break;
		}
	}

	if (!iter) {
		return false;
	}

	dgInt32 faceIndex[4096];
	dgInt64 faceDataIndex[4096];
	dgStack<dgEdge *> stackPool(GetCount());
	dgEdge **const stack = &stackPool[0];

	dgInt32 mark = IncLRU();

	polyhedra.BeginFace();
	stack[0] = edge;
	dgInt32 index = 1;
	while (index) {
		index--;
		dgEdge *const edgeI = stack[index];

		if (edgeI->m_mark == mark) {
			continue;
		}

		dgInt32 count = 0;
		dgEdge *ptr = edgeI;
		do {
			ptr->m_mark = mark;
			faceIndex[count] = ptr->m_incidentVertex;
			faceDataIndex[count] = dgInt64(ptr->m_userData);
			count++;
			NEWTON_ASSERT(count < dgInt32(sizeof(faceIndex) / sizeof(faceIndex[0])));

			if ((ptr->m_twin->m_incidentFace > 0) && (ptr->m_twin->m_mark != mark)) {
				stack[index] = ptr->m_twin;
				index++;
				NEWTON_ASSERT(index < GetCount());
			}

			ptr = ptr->m_next;
		} while (ptr != edgeI);

		polyhedra.AddFace(count, &faceIndex[0], &faceDataIndex[0]);
	}

	polyhedra.EndFace();

	return true;
}

void dgPolyhedra::ChangeEdgeIncidentVertex(dgEdge *const edge, dgInt32 newIndex) {
	dgEdge *ptr = edge;
	do {
		dgTreeNode *node = GetNodeFromInfo(*ptr);
		dgPairKey Key0(newIndex, ptr->m_twin->m_incidentVertex);
		ReplaceKey(node, Key0.GetVal());

		node = GetNodeFromInfo(*ptr->m_twin);
		dgPairKey Key1(ptr->m_twin->m_incidentVertex, newIndex);
		ReplaceKey(node, Key1.GetVal());

		ptr->m_incidentVertex = newIndex;

		ptr = ptr->m_twin->m_next;
	} while (ptr != edge);
}

void dgPolyhedra::DeleteDegenerateFaces(const dgFloat64 *const pool,
                                        dgInt32 strideInBytes, dgFloat64 area) {
	if (!GetCount()) {
		return;
	}

#ifdef __ENABLE_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif
	dgStack<dgPolyhedra::dgTreeNode *> faceArrayPool(GetCount() / 2 + 100);

	dgInt32 count = 0;
	dgPolyhedra::dgTreeNode **const faceArray = &faceArrayPool[0];
	dgInt32 mark = IncLRU();
	Iterator iter(*this);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);

		if ((edge->m_mark != mark) && (edge->m_incidentFace > 0)) {
			faceArray[count] = iter.GetNode();
			count++;
			dgEdge *ptr = edge;
			do {
				ptr->m_mark = mark;
				ptr = ptr->m_next;
			} while (ptr != edge);
		}
	}

	dgFloat64 area2 = area * area;
	area2 *= dgFloat64(4.0f);

	for (dgInt32 i = 0; i < count; i++) {
		dgPolyhedra::dgTreeNode *const faceNode = faceArray[i];
		dgEdge *const edge = &faceNode->GetInfo();

		dgBigVector normal(FaceNormal(edge, pool, strideInBytes));

		dgFloat64 faceArea = normal % normal;
		if (faceArea < area2) {
			DeleteFace(edge);
		}
	}

#ifdef __ENABLE_SANITY_CHECK
	mark = IncLRU();
	for (iter.Begin(); iter; iter ++)  {
		dgEdge *const edge = &(*iter);
		if ((edge->m_mark != mark) && (edge->m_incidentFace > 0)) {
			//NEWTON_ASSERT (edge->m_next->m_next->m_next == edge);
			dgEdge *ptr = edge;
			do {
				ptr->m_mark = mark;
				ptr = ptr->m_next;
			} while (ptr != edge);

			dgBigVector normal(FaceNormal(edge, pool, strideInBytes));

			dgFloat64 faceArea = normal % normal;
			NEWTON_ASSERT(faceArea >= area2);
		}
	}
	NEWTON_ASSERT(SanityCheck());
#endif
}

static void NormalizeVertex(dgInt32 count, dgBigVector *const dst,
                            const dgFloat64 *const src, dgInt32 stride) {
//	dgBigVector min;
//	dgBigVector max;
//	GetMinMax (min, max, src, count, dgInt32 (stride * sizeof (dgFloat64)));
//	dgBigVector centre (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	for (dgInt32 i = 0; i < count; i++) {
//		dst[i].m_x = centre.m_x + src[i * stride + 0];
//		dst[i].m_y = centre.m_y + src[i * stride + 1];
//		dst[i].m_z = centre.m_z + src[i * stride + 2];
		dst[i].m_x = src[i * stride + 0];
		dst[i].m_y = src[i * stride + 1];
		dst[i].m_z = src[i * stride + 2];
		dst[i].m_w = dgFloat64(0.0f);
	}
}

static dgBigPlane EdgePlane(dgInt32 i0, dgInt32 i1, dgInt32 i2,
                            const dgBigVector *const pool) {
	const dgBigVector &p0 = pool[i0];
	const dgBigVector &p1 = pool[i1];
	const dgBigVector &p2 = pool[i2];

	dgBigPlane plane(p0, p1, p2);
	dgFloat64 mag = sqrt(plane % plane);
	if (mag < dgFloat64(1.0e-12f)) {
		mag = dgFloat64(1.0e-12f);
	}
	mag = dgFloat64(1.0f) / mag;

	plane.m_x *= mag;
	plane.m_y *= mag;
	plane.m_z *= mag;
	plane.m_w *= mag;

	return plane;
}

static dgBigPlane UnboundedLoopPlane(dgInt32 i0, dgInt32 i1, dgInt32 i2,
                                     const dgBigVector *const pool) {
	const dgBigVector p0 = pool[i0];
	const dgBigVector p1 = pool[i1];
	const dgBigVector p2 = pool[i2];
	dgBigVector E0(p1 - p0);
	dgBigVector E1(p2 - p0);

	dgBigVector N((E0 * E1) * E0);
	dgFloat64 dist = -(N % p0);
	dgBigPlane plane(N, dist);

	dgFloat64 mag = sqrt(plane % plane);
	if (mag < dgFloat64(1.0e-12f)) {
		mag = dgFloat64(1.0e-12f);
	}
	mag = dgFloat64(10.0f) / mag;

	plane.m_x *= mag;
	plane.m_y *= mag;
	plane.m_z *= mag;
	plane.m_w *= mag;

	return plane;
}

static void CalculateAllMetrics(const dgPolyhedra *const polyhedra,
                                dgVertexCollapseVertexMetric *const table, const dgBigVector *const pool) {
	dgInt32 edgeMark = polyhedra->IncLRU();
	dgPolyhedra::Iterator iter(*polyhedra);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);

		NEWTON_ASSERT(edge);
		if (edge->m_mark != edgeMark) {

			if (edge->m_incidentFace > 0) {
				dgInt32 i0 = edge->m_incidentVertex;
				dgInt32 i1 = edge->m_next->m_incidentVertex;
				dgInt32 i2 = edge->m_prev->m_incidentVertex;

				dgBigPlane constrainPlane(EdgePlane(i0, i1, i2, pool));
				dgVertexCollapseVertexMetric tmp(constrainPlane);

				dgEdge *ptr = edge;
				do {
					ptr->m_mark = edgeMark;
					i0 = ptr->m_incidentVertex;
					table[i0].Accumulate(tmp);

					ptr = ptr->m_next;
				} while (ptr != edge);

			} else {
				NEWTON_ASSERT(edge->m_twin->m_incidentFace > 0);
				dgInt32 i0 = edge->m_twin->m_incidentVertex;
				dgInt32 i1 = edge->m_twin->m_next->m_incidentVertex;
				dgInt32 i2 = edge->m_twin->m_prev->m_incidentVertex;

				edge->m_mark = edgeMark;
				dgBigPlane constrainPlane(UnboundedLoopPlane(i0, i1, i2, pool));
				dgVertexCollapseVertexMetric tmp(constrainPlane);

				i0 = edge->m_incidentVertex;
				table[i0].Accumulate(tmp);

				i0 = edge->m_twin->m_incidentVertex;
				table[i0].Accumulate(tmp);
			}
		}
	}
}

dgFloat64 dgPolyhedra::EdgePenalty(const dgBigVector *const pool,
                                   dgEdge *const edge) const {
	dgInt32 i0 = edge->m_incidentVertex;
	dgInt32 i1 = edge->m_next->m_incidentVertex;

	const dgBigVector &p0 = pool[i0];
	const dgBigVector &p1 = pool[i1];
	dgBigVector dp(p1 - p0);

	dgFloat64 dot = dp % dp;
	if (dot < dgFloat64(1.0e-6f)) {
		return dgFloat64(-1.0f);
	}

	if ((edge->m_incidentFace > 0) && (edge->m_twin->m_incidentFace > 0)) {
		dgBigVector edgeNormal(FaceNormal(edge, &pool[0].m_x, sizeof(dgBigVector)));
		dgBigVector twinNormal(
		    FaceNormal(edge->m_twin, &pool[0].m_x, sizeof(dgBigVector)));

		dgFloat64 mag0 = edgeNormal % edgeNormal;
		dgFloat64 mag1 = twinNormal % twinNormal;
		if ((mag0 < dgFloat64(1.0e-24f)) || (mag1 < dgFloat64(1.0e-24f))) {
			return dgFloat64(-1.0f);
		}

		edgeNormal = edgeNormal.Scale(dgFloat64(1.0f) / sqrt(mag0));
		twinNormal = twinNormal.Scale(dgFloat64(1.0f) / sqrt(mag1));

		dot = edgeNormal % twinNormal;
		if (dot < dgFloat64(-0.9f)) {
			return dgFloat32(-1.0f);
		}

		dgEdge *ptr = edge;
		do {
			if ((ptr->m_incidentFace <= 0) || (ptr->m_twin->m_incidentFace <= 0)) {
				dgEdge *const adj = edge->m_twin;
				ptr = edge;
				do {
					if ((ptr->m_incidentFace <= 0) || (ptr->m_twin->m_incidentFace <= 0)) {
						return dgFloat32(-1.0);
					}
					ptr = ptr->m_twin->m_next;
				} while (ptr != adj);
			}
			ptr = ptr->m_twin->m_next;
		} while (ptr != edge);
	}

	dgInt32 faceA = edge->m_incidentFace;
	dgInt32 faceB = edge->m_twin->m_incidentFace;

	i0 = edge->m_twin->m_incidentVertex;
	dgBigVector p(pool[i0].m_x, pool[i0].m_y, pool[i0].m_z, dgFloat32(0.0f));

	bool penalty = false;
	dgEdge *ptr = edge;
	do {
		dgEdge *const adj = ptr->m_twin;

		dgInt32 face = adj->m_incidentFace;
		if ((face != faceB) && (face != faceA) && (face >= 0)
		        && (adj->m_next->m_incidentFace == face)
		        && (adj->m_prev->m_incidentFace == face)) {

			dgInt32 iv0 = adj->m_next->m_incidentVertex;
			const dgBigVector &pv0 = pool[iv0];

			dgInt32 iv1 = adj->m_incidentVertex;
			const dgBigVector &pv1 = pool[iv1];

			dgInt32 iv2 = adj->m_prev->m_incidentVertex;
			const dgBigVector &pv2 = pool[iv2];

			dgBigVector n0((pv1 - pv0) * (pv2 - pv0));
			dgBigVector n1((pv1 - p) * (pv2 - p));

//			dgFloat64 mag0 = n0 % n0;
//			NEWTON_ASSERT (mag0 > dgFloat64(1.0e-16f));
//			mag0 = sqrt (mag0);

//			dgFloat64 mag1 = n1 % n1;
//			mag1 = sqrt (mag1);

			dgFloat64 dotn = n0 % n1;
			if (dotn < dgFloat64(0.0f)) {
//			if (dotn <= (mag0 * mag1 * dgFloat32 (0.707f)) || (mag0 > (dgFloat64(16.0f) * mag1))) {
				penalty = true;
				break;
			}
		}

		ptr = ptr->m_twin->m_next;
	} while (ptr != edge);

	dgFloat64 aspect = dgFloat32(-1.0f);
	if (!penalty) {
		dgInt32 iv0 = edge->m_twin->m_incidentVertex;
		dgBigVector pv0(pool[iv0]);

		aspect = dgFloat32(1.0f);
		for (dgEdge *ptrn = edge->m_twin->m_next->m_twin->m_next; ptrn != edge; ptrn = ptr->m_twin->m_next) {
			if (ptrn->m_incidentFace > 0) {
				dgInt32 iv1 = ptrn->m_next->m_incidentVertex;
				const dgBigVector &pv1 = pool[iv1];

				dgInt32 iv2 = ptrn->m_prev->m_incidentVertex;
				const dgBigVector &pv2 = pool[iv2];

				dgBigVector e0(pv1 - pv0);
				dgBigVector e1(pv2 - pv1);
				dgBigVector e2(pv0 - pv2);

				dgFloat64 mag0 = e0 % e0;
				dgFloat64 mag1 = e1 % e1;
				dgFloat64 mag2 = e2 % e2;
				dgFloat64 maxMag = GetMax(mag0, mag1, mag2);
				dgFloat64 minMag = GetMin(mag0, mag1, mag2);
				dgFloat64 ratio = minMag / maxMag;

				if (ratio < aspect) {
					aspect = ratio;
				}
			}
		}
		aspect = sqrt(aspect);
		//aspect = 1.0f;
	}

	return aspect;
}

static void CalculateVertexMetrics(dgVertexCollapseVertexMetric table[],
                                   const dgBigVector *const pool, dgEdge *const edge) {
	dgInt32 i0 = edge->m_incidentVertex;

//	const dgBigVector& p0 = pool[i0];
	table[i0].Clear();
	dgEdge *ptr = edge;
	do {

		if (ptr->m_incidentFace > 0) {
			dgInt32 i1 = ptr->m_next->m_incidentVertex;
			dgInt32 i2 = ptr->m_prev->m_incidentVertex;
			dgBigPlane constrainPlane(EdgePlane(i0, i1, i2, pool));
			table[i0].Accumulate(constrainPlane);

		} else {
			dgInt32 i1 = ptr->m_twin->m_incidentVertex;
			dgInt32 i2 = ptr->m_twin->m_prev->m_incidentVertex;
			dgBigPlane constrainPlane(UnboundedLoopPlane(i0, i1, i2, pool));
			table[i0].Accumulate(constrainPlane);

			i1 = ptr->m_prev->m_incidentVertex;
			i2 = ptr->m_prev->m_twin->m_prev->m_incidentVertex;
			constrainPlane = UnboundedLoopPlane(i0, i1, i2, pool);
			table[i0].Accumulate(constrainPlane);
		}

		ptr = ptr->m_twin->m_next;
	} while (ptr != edge);
}

static void RemoveHalfEdge(dgPolyhedra *const polyhedra, dgEdge *const edge) {
	dgEdgeCollapseEdgeHandle *const handle =
	    (dgEdgeCollapseEdgeHandle *) IntToPointer(edge->m_userData);
	if (handle) {
		handle->m_edge = NULL;
	}

	dgPolyhedra::dgTreeNode *const node = polyhedra->GetNodeFromInfo(*edge);
	NEWTON_ASSERT(node);
	polyhedra->Remove(node);
}

static dgEdge *CollapseEdge(dgPolyhedra *const polyhedra, dgEdge *const edge) {
	dgInt32 v0 = edge->m_incidentVertex;
	dgInt32 v1 = edge->m_twin->m_incidentVertex;

#ifdef __ENABLE_SANITY_CHECK
	//dgPairKey TwinKey (v1, v0);
	dgPolyhedra::dgPairKey TwinKey(v1, v0);
	dgPolyhedra::dgTreeNode *const node = polyhedra->Find(TwinKey.GetVal());
	dgEdge *const twin1 = node ? &node->GetInfo() : NULL;
	NEWTON_ASSERT(twin1);
	NEWTON_ASSERT(edge->m_twin == twin1);
	NEWTON_ASSERT(twin1->m_twin == edge);
	NEWTON_ASSERT(edge->m_incidentFace != 0);
	NEWTON_ASSERT(twin1->m_incidentFace != 0);
#endif

	dgEdge *retEdge = edge->m_twin->m_prev->m_twin;
	if (retEdge == edge->m_twin->m_next) {
		return NULL;
	}
	if (retEdge == edge->m_twin) {
		return NULL;
	}
	if (retEdge == edge->m_next) {
		retEdge = edge->m_prev->m_twin;
		if (retEdge == edge->m_twin->m_next) {
			return NULL;
		}
		if (retEdge == edge->m_twin) {
			return NULL;
		}
	}

	dgEdge *lastEdge = NULL;
	dgEdge *firstEdge = NULL;
	if ((edge->m_incidentFace >= 0) && (edge->m_twin->m_incidentFace >= 0)) {
		lastEdge = edge->m_prev->m_twin;
		firstEdge = edge->m_twin->m_next->m_twin->m_next;
	} else if (edge->m_twin->m_incidentFace >= 0) {
		firstEdge = edge->m_twin->m_next->m_twin->m_next;
		lastEdge = edge;
	} else {
		lastEdge = edge->m_prev->m_twin;
		firstEdge = edge->m_twin->m_next;
	}

	for (dgEdge *ptr = firstEdge; ptr != lastEdge; ptr = ptr->m_twin->m_next) {
		dgEdge *badEdge = polyhedra->FindEdge(edge->m_twin->m_incidentVertex,
		                                      ptr->m_twin->m_incidentVertex);
		if (badEdge) {
			return NULL;
		}
	}

	dgEdge *const twin = edge->m_twin;
	if (twin->m_next == twin->m_prev->m_prev) {
		twin->m_prev->m_twin->m_twin = twin->m_next->m_twin;
		twin->m_next->m_twin->m_twin = twin->m_prev->m_twin;

		RemoveHalfEdge(polyhedra, twin->m_prev);
		RemoveHalfEdge(polyhedra, twin->m_next);
	} else {
		twin->m_next->m_prev = twin->m_prev;
		twin->m_prev->m_next = twin->m_next;
	}

	if (edge->m_next == edge->m_prev->m_prev) {
		edge->m_next->m_twin->m_twin = edge->m_prev->m_twin;
		edge->m_prev->m_twin->m_twin = edge->m_next->m_twin;
		RemoveHalfEdge(polyhedra, edge->m_next);
		RemoveHalfEdge(polyhedra, edge->m_prev);
	} else {
		edge->m_next->m_prev = edge->m_prev;
		edge->m_prev->m_next = edge->m_next;
	}

	NEWTON_ASSERT(twin->m_twin->m_incidentVertex == v0);
	NEWTON_ASSERT(edge->m_twin->m_incidentVertex == v1);
	RemoveHalfEdge(polyhedra, twin);
	RemoveHalfEdge(polyhedra, edge);

	dgEdge *ptr = retEdge;
	do {
		dgPolyhedra::dgPairKey pairKey(v0, ptr->m_twin->m_incidentVertex);

		dgPolyhedra::dgTreeNode *node = polyhedra->Find(pairKey.GetVal());
		if (node) {
			if (&node->GetInfo() == ptr) {
				dgPolyhedra::dgPairKey key(v1, ptr->m_twin->m_incidentVertex);
				ptr->m_incidentVertex = v1;
				node = polyhedra->ReplaceKey(node, key.GetVal());
				NEWTON_ASSERT(node);
			}
		}

		dgPolyhedra::dgPairKey TwinKey(ptr->m_twin->m_incidentVertex, v0);
		node = polyhedra->Find(TwinKey.GetVal());
		if (node) {
			if (&node->GetInfo() == ptr->m_twin) {
				dgPolyhedra::dgPairKey key(ptr->m_twin->m_incidentVertex, v1);
				node = polyhedra->ReplaceKey(node, key.GetVal());
				NEWTON_ASSERT(node);
			}
		}

		ptr = ptr->m_twin->m_next;
	} while (ptr != retEdge);

	return retEdge;
}

void dgPolyhedra::Optimize(const dgFloat64 *const array, dgInt32 strideInBytes,
                           dgFloat64 tol) {
	dgList<dgEdgeCollapseEdgeHandle>::dgListNode *handleNodePtr;

	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));

#ifdef __ENABLE_SANITY_CHECK
	NEWTON_ASSERT(SanityCheck());
#endif

	dgInt32 edgeCount = GetEdgeCount() * 4 + 1024 * 16;
	dgInt32 maxVertexIndex = GetLastVertexIndex();

	dgStack<dgBigVector> vertexPool(maxVertexIndex);
	dgStack<dgVertexCollapseVertexMetric> vertexMetrics(maxVertexIndex + 512);

	dgList<dgEdgeCollapseEdgeHandle> edgeHandleList(GetAllocator());
	dgStack<char> heapPool(
	    2 * edgeCount
	    * dgInt32(
	        sizeof(dgFloat64) + sizeof(dgEdgeCollapseEdgeHandle *)
	        + sizeof(dgInt32)));
	dgDownHeap<dgList<dgEdgeCollapseEdgeHandle>::dgListNode *, dgFloat64> bigHeapArray(
	    &heapPool[0], heapPool.GetSizeInBytes());

	NormalizeVertex(maxVertexIndex, &vertexPool[0], array, stride);
	for (uint i = 0; i < maxVertexIndex; i++) vertexMetrics[i].Clear();
	CalculateAllMetrics(this, &vertexMetrics[0], &vertexPool[0]);

	dgFloat64 tol2 = tol * tol;
	Iterator iter(*this);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);

		edge->m_userData = 0;
		dgInt32 index0 = edge->m_incidentVertex;
		dgInt32 index1 = edge->m_twin->m_incidentVertex;

		dgVertexCollapseVertexMetric &metric = vertexMetrics[index0];
		dgVector p(&vertexPool[index1].m_x);
		dgFloat64 cost = metric.Evalue(p);
		if (cost < tol2) {
			cost = EdgePenalty(&vertexPool[0], edge);

			if (cost > dgFloat64(0.0f)) {
				dgEdgeCollapseEdgeHandle handle(edge);
				handleNodePtr = edgeHandleList.Addtop(handle);
				bigHeapArray.Push(handleNodePtr, cost);
			}
		}
	}

	while (bigHeapArray.GetCount()) {
		handleNodePtr = bigHeapArray[0];

		dgEdge *edge = handleNodePtr->GetInfo().m_edge;
		bigHeapArray.Pop();
		edgeHandleList.Remove(handleNodePtr);

		if (edge) {
			CalculateVertexMetrics(&vertexMetrics[0], &vertexPool[0], edge);

			dgInt32 index0 = edge->m_incidentVertex;
			dgInt32 index1 = edge->m_twin->m_incidentVertex;
			dgVertexCollapseVertexMetric &metric = vertexMetrics[index0];
			dgBigVector p(vertexPool[index1]);

			if ((metric.Evalue(p) < tol2)
			        && (EdgePenalty(&vertexPool[0], edge) > dgFloat64(0.0f))) {

#ifdef __ENABLE_SANITY_CHECK
				NEWTON_ASSERT(SanityCheck());
#endif

				edge = CollapseEdge(this, edge);

#ifdef __ENABLE_SANITY_CHECK
				NEWTON_ASSERT(SanityCheck());
#endif
				if (edge) {
					// Update vertex metrics
					CalculateVertexMetrics(&vertexMetrics[0], &vertexPool[0], edge);

					// Update metrics for all surrounding vertex
					dgEdge *ptr = edge;
					do {
						CalculateVertexMetrics(&vertexMetrics[0], &vertexPool[0],
						                       ptr->m_twin);
						ptr = ptr->m_twin->m_next;
					} while (ptr != edge);

					// calculate edge cost of all incident edges
					dgInt32 mark = IncLRU();
					ptr = edge;
					do {
						NEWTON_ASSERT(ptr->m_mark != mark);
						ptr->m_mark = mark;

						index0 = ptr->m_incidentVertex;
						index1 = ptr->m_twin->m_incidentVertex;

						dgVertexCollapseVertexMetric &metricZero = vertexMetrics[index0];
						dgBigVector pv(vertexPool[index1]);

						dgFloat64 cost = dgFloat32(-1.0f);
						if (metricZero.Evalue(pv) < tol2) {
							cost = EdgePenalty(&vertexPool[0], ptr);
						}

						if (cost > dgFloat64(0.0f)) {
							dgEdgeCollapseEdgeHandle handle(ptr);
							handleNodePtr = edgeHandleList.Addtop(handle);
							bigHeapArray.Push(handleNodePtr, cost);
						} else {
							dgEdgeCollapseEdgeHandle *const handle =
							    (dgEdgeCollapseEdgeHandle *) IntToPointer(ptr->m_userData);
							if (handle) {
								handle->m_edge = NULL;
							}
							ptr->m_userData = PointerToInt(NULL);

						}

						ptr = ptr->m_twin->m_next;
					} while (ptr != edge);

					// calculate edge cost of all incident edges to a surrounding vertex
					ptr = edge;
					do {
						dgEdge *const incidentEdge = ptr->m_twin;

						dgEdge *ptr1 = incidentEdge;
						do {
							index0 = ptr1->m_incidentVertex;
							index1 = ptr1->m_twin->m_incidentVertex;

							if (ptr1->m_mark != mark) {
								ptr1->m_mark = mark;
								dgVertexCollapseVertexMetric &metricZero = vertexMetrics[index0];
								dgBigVector pv(vertexPool[index1]);

								dgFloat64 cost = dgFloat32(-1.0f);
								if (metricZero.Evalue(pv) < tol2) {
									cost = EdgePenalty(&vertexPool[0], ptr1);
								}

								if (cost > dgFloat64(0.0f)) {
									NEWTON_ASSERT(cost > dgFloat64(0.0f));
									dgEdgeCollapseEdgeHandle handle(ptr1);
									handleNodePtr = edgeHandleList.Addtop(handle);
									bigHeapArray.Push(handleNodePtr, cost);
								} else {
									dgEdgeCollapseEdgeHandle *handle;
									handle =
									    (dgEdgeCollapseEdgeHandle *) IntToPointer(ptr1->m_userData);
									if (handle) {
										handle->m_edge = NULL;
									}
									ptr1->m_userData = PointerToInt(NULL);

								}
							}

							if (ptr1->m_twin->m_mark != mark) {
								ptr1->m_twin->m_mark = mark;
								dgVertexCollapseVertexMetric &metricOne = vertexMetrics[index1];
								dgBigVector pv(vertexPool[index0]);

								dgFloat64 cost = dgFloat32(-1.0f);
								if (metricOne.Evalue(pv) < tol2) {
									cost = EdgePenalty(&vertexPool[0], ptr1->m_twin);
								}

								if (cost > dgFloat64(0.0f)) {
									NEWTON_ASSERT(cost > dgFloat64(0.0f));
									dgEdgeCollapseEdgeHandle handle(ptr1->m_twin);
									handleNodePtr = edgeHandleList.Addtop(handle);
									bigHeapArray.Push(handleNodePtr, cost);
								} else {
									dgEdgeCollapseEdgeHandle *handle;
									handle =
									    (dgEdgeCollapseEdgeHandle *) IntToPointer(ptr1->m_twin->m_userData);
									if (handle) {
										handle->m_edge = NULL;
									}
									ptr1->m_twin->m_userData = PointerToInt(NULL);

								}
							}

							ptr1 = ptr1->m_twin->m_next;
						} while (ptr1 != incidentEdge);

						ptr = ptr->m_twin->m_next;
					} while (ptr != edge);
				}
			}
		}
	}
}

dgEdge *dgPolyhedra::FindEarTip(dgEdge *const face, const dgFloat64 *const pool,
                                dgInt32 stride, dgDownHeap<dgEdge *, dgFloat64> &heap
                                , const dgBigVector &normal) const {
	dgEdge *ptr = face;
	dgBigVector p0(&pool[ptr->m_prev->m_incidentVertex * stride]);
	dgBigVector p1(&pool[ptr->m_incidentVertex * stride]);
	dgBigVector d0(p1 - p0);
	dgFloat64 f0 = sqrt(d0 % d0);
	if (f0 < dgFloat64(1.0e-10f)) {
		f0 = dgFloat64(1.0e-10f);
	}
	d0 = d0.Scale(dgFloat64(1.0f) / f0);

	dgFloat64 minAngle = dgFloat32(10.0f);
	do {
		dgBigVector p2(&pool[ptr->m_next->m_incidentVertex * stride]);
		dgBigVector d1(p2 - p1);
		dgFloat32 f1 = dgSqrt(d1 % d1);
		if (f1 < dgFloat32(1.0e-10f)) {
			f1 = dgFloat32(1.0e-10f);
		}
		d1 = d1.Scale(dgFloat32(1.0f) / f1);
		dgBigVector n(d0 * d1);

		dgFloat64 angle = normal % n;
		if (angle >= dgFloat64(0.0f)) {
			heap.Push(ptr, angle);
		}

		if (angle < minAngle) {
			minAngle = angle;
		}

		d0 = d1;
		p1 = p2;
		ptr = ptr->m_next;
	} while (ptr != face);

	if (minAngle > dgFloat32(0.1f)) {
		return heap[0];
	}

	dgEdge *ear = NULL;
	while (heap.GetCount()) {
		ear = heap[0];
		heap.Pop();

		if (FindEdge(ear->m_prev->m_incidentVertex, ear->m_next->m_incidentVertex)) {
			continue;
		}

		dgBigVector pv0(&pool[ear->m_prev->m_incidentVertex * stride]);
		dgBigVector pv1(&pool[ear->m_incidentVertex * stride]);
		dgBigVector pv2(&pool[ear->m_next->m_incidentVertex * stride]);

		dgBigVector pv10(pv1 - pv0);
		dgBigVector pv21(pv2 - pv1);
		dgBigVector pv02(pv0 - pv2);

		for (ptr = ear->m_next->m_next; ptr != ear->m_prev; ptr = ptr->m_next) {
			dgBigVector p(&pool[ptr->m_incidentVertex * stride]);

			dgFloat64 side = ((p - pv0) * pv10) % normal;
			if (side < dgFloat64(0.05f)) {
				side = ((p - pv1) * pv21) % normal;
				if (side < dgFloat64(0.05f)) {
					side = ((p - pv2) * pv02) % normal;
					if (side < dgFloat32(0.05f)) {
						break;
					}
				}
			}
		}

		if (ptr == ear->m_prev) {
			break;
		}
	}

	return ear;
}

//dgEdge* TriangulateFace (dgPolyhedra& polyhedra, dgEdge* face, const dgFloat32* const pool, dgInt32 stride, dgDownHeap<dgEdge*, dgFloat32>& heap, dgVector* const faceNormalOut)
dgEdge *dgPolyhedra::TriangulateFace(dgEdge *face, const dgFloat64 *const pool,
                                     dgInt32 stride, dgDownHeap<dgEdge *, dgFloat64> &heap
                                     , dgBigVector *const faceNormalOut) {
#if 0
	dgEdge *perimeter[1024 * 16];
	dgEdge *ptr = face;
	dgInt32 perimeterCount = 0;
	do {
		perimeter[perimeterCount] = ptr;
		perimeterCount++;
		NEWTON_ASSERT(perimeterCount < dgInt32(sizeof(perimeter) / sizeof(perimeter[0])));
		ptr = ptr->m_next;
	} while (ptr != face);
	perimeter[perimeterCount] = face;
	NEWTON_ASSERT((perimeterCount + 1) < dgInt32(sizeof(perimeter) / sizeof(perimeter[0])));
#endif

	dgBigVector normal(
	    FaceNormal(face, pool, dgInt32(stride * sizeof(dgFloat64))));

	dgFloat64 dot = normal % normal;
	if (dot < dgFloat64(1.0e-12f)) {
		if (faceNormalOut) {
			*faceNormalOut = dgBigVector(dgFloat32(0.0f), dgFloat32(0.0f),
			                             dgFloat32(0.0f), dgFloat32(0.0f));
		}
		return face;
	}
	normal = normal.Scale(dgFloat64(1.0f) / sqrt(dot));
	if (faceNormalOut) {
		*faceNormalOut = normal;
	}

	while (face->m_next->m_next->m_next != face) {
		dgEdge *const ear = FindEarTip(face, pool, stride, heap, normal);
		if (!ear) {
			return face;
		}
		if ((face == ear) || (face == ear->m_prev)) {
			face = ear->m_prev->m_prev;
		}
		dgEdge *const edge = AddHalfEdge(ear->m_next->m_incidentVertex,
		                                 ear->m_prev->m_incidentVertex);
		if (!edge) {
			return face;
		}
		dgEdge *const twin = AddHalfEdge(ear->m_prev->m_incidentVertex,
		                                 ear->m_next->m_incidentVertex);
		if (!twin) {
			return face;
		}
		NEWTON_ASSERT(twin);

		edge->m_mark = ear->m_mark;
		edge->m_userData = ear->m_next->m_userData;
		edge->m_incidentFace = ear->m_incidentFace;

		twin->m_mark = ear->m_mark;
		twin->m_userData = ear->m_prev->m_userData;
		twin->m_incidentFace = ear->m_incidentFace;

		edge->m_twin = twin;
		twin->m_twin = edge;

		twin->m_prev = ear->m_prev->m_prev;
		twin->m_next = ear->m_next;
		ear->m_prev->m_prev->m_next = twin;
		ear->m_next->m_prev = twin;

		edge->m_next = ear->m_prev;
		edge->m_prev = ear;
		ear->m_prev->m_prev = edge;
		ear->m_next = edge;

		heap.Flush();
	}
	return NULL;
}

void dgPolyhedra::MarkAdjacentCoplanarFaces(dgPolyhedra &polyhedraOut,
        dgEdge *const face, const dgFloat64 *const pool, dgInt32 strideInBytes) {
	const dgFloat64 normalDeviation = dgFloat64(0.9999f);
	const dgFloat64 distanceFromPlane = dgFloat64(1.0f / 128.0f);

	dgInt32 faceIndex[1024 * 4];
	dgEdge *stack[1024 * 4];
	dgEdge *deleteEdge[1024 * 4];

	dgInt32 deleteCount = 1;
	deleteEdge[0] = face;
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));

	NEWTON_ASSERT(face->m_incidentFace > 0);

	dgBigVector normalAverage(FaceNormal(face, pool, strideInBytes));
	dgFloat64 dot = normalAverage % normalAverage;
	if (dot > dgFloat64(1.0e-12f)) {
		dgInt32 testPointsCount = 1;
		dot = dgFloat64(1.0f) / sqrt(dot);
		dgBigVector normal(normalAverage.Scale(dot));

		dgBigVector averageTestPoint(&pool[face->m_incidentVertex * stride]);
		dgBigPlane testPlane(normal, -(averageTestPoint % normal));

		polyhedraOut.BeginFace();

		IncLRU();
		dgInt32 faceMark = IncLRU();

		dgInt32 faceIndexCount = 0;
		{
			dgEdge *ptr = face;
			do {
				ptr->m_mark = faceMark;
				faceIndex[faceIndexCount] = ptr->m_incidentVertex;
				faceIndexCount++;
				NEWTON_ASSERT(faceIndexCount < dgInt32(sizeof(faceIndex) / sizeof(faceIndex[0])));
				ptr = ptr->m_next;
			} while (ptr != face);
		}
		polyhedraOut.AddFace(faceIndexCount, faceIndex);

		dgInt32 index = 1;
		deleteCount = 0;
		stack[0] = face;
		while (index) {
			index--;
			dgEdge *const stackFace = stack[index];
			deleteEdge[deleteCount] = stackFace;
			deleteCount++;
			NEWTON_ASSERT(deleteCount < dgInt32(sizeof(deleteEdge) / sizeof(deleteEdge[0])));
			NEWTON_ASSERT(stackFace->m_next->m_next->m_next == stackFace);

			dgEdge *edge = stackFace;
			do {
				dgEdge *const ptr = edge->m_twin;
				if (ptr->m_incidentFace > 0) {
					if (ptr->m_mark != faceMark) {
						dgEdge *ptr1 = ptr;
						faceIndexCount = 0;
						do {
							ptr1->m_mark = faceMark;
							faceIndex[faceIndexCount] = ptr1->m_incidentVertex;
							NEWTON_ASSERT(
							    faceIndexCount < dgInt32(sizeof(faceIndex) / sizeof(faceIndex[0])));
							faceIndexCount++;
							ptr1 = ptr1->m_next;
						} while (ptr1 != ptr);

						dgBigVector normal1(FaceNormal(ptr, pool, strideInBytes));
						dot = normal1 % normal1;
						if (dot < dgFloat64(1.0e-12f)) {
							deleteEdge[deleteCount] = ptr;
							deleteCount++;
							NEWTON_ASSERT(
							    deleteCount < dgInt32(sizeof(deleteEdge) / sizeof(deleteEdge[0])));
						} else {
							//normal1 = normal1.Scale (dgFloat64 (1.0f) / sqrt (dot));
							dgBigVector testNormal(
							    normal1.Scale(dgFloat64(1.0f) / sqrt(dot)));
							dot = normal % testNormal;
							if (dot >= normalDeviation) {
								dgBigVector testPoint(
								    &pool[ptr->m_prev->m_incidentVertex * stride]);
								dgFloat64 dist = fabs(testPlane.Evalue(testPoint));
								if (dist < distanceFromPlane) {
									testPointsCount++;

									averageTestPoint += testPoint;
									testPoint = averageTestPoint.Scale(
									                dgFloat64(1.0f) / dgFloat64(testPointsCount));

									normalAverage += normal1;
									testNormal = normalAverage.Scale(
									                 dgFloat64(1.0f) / sqrt(normalAverage % normalAverage));
									testPlane = dgBigPlane(testNormal, -(testPoint % testNormal));

									polyhedraOut.AddFace(faceIndexCount, faceIndex);
									;
									stack[index] = ptr;
									index++;
									NEWTON_ASSERT(index < dgInt32(sizeof(stack) / sizeof(stack[0])));
								}
							}
						}
					}
				}

				edge = edge->m_next;
			} while (edge != stackFace);
		}
		polyhedraOut.EndFace();
	}

	for (dgInt32 index = 0; index < deleteCount; index++) {
		DeleteFace(deleteEdge[index]);
	}
}

void dgPolyhedra::RefineTriangulation(const dgFloat64 *const vertex,
                                      dgInt32 stride, dgBigVector *const normal, dgInt32 perimeterCount,
                                      dgEdge **const perimeter) {
	dgList<dgDiagonalEdge> dignonals(GetAllocator());

	for (dgInt32 i = 1; i <= perimeterCount; i++) {
		dgEdge *const last = perimeter[i - 1];
		for (dgEdge *ptr = perimeter[i]->m_prev; ptr != last;
		        ptr = ptr->m_twin->m_prev) {
			dgList<dgDiagonalEdge>::dgListNode *node = dignonals.GetFirst();
			for (; node; node = node->GetNext()) {
				const dgDiagonalEdge &key = node->GetInfo();
				if (((key.m_i0 == ptr->m_incidentVertex)
				        && (key.m_i1 == ptr->m_twin->m_incidentVertex))
				        || ((key.m_i1 == ptr->m_incidentVertex)
				            && (key.m_i0 == ptr->m_twin->m_incidentVertex))) {
					break;
				}
			}
			if (!node) {
				dgDiagonalEdge key(ptr);
				dignonals.Append(key);
			}
		}
	}

	dgEdge *const face = perimeter[0];
	dgInt32 i0 = face->m_incidentVertex * stride;
	dgInt32 i1 = face->m_next->m_incidentVertex * stride;
	dgBigVector p0(vertex[i0], vertex[i0 + 1], vertex[i0 + 2], dgFloat32(0.0f));
	dgBigVector p1(vertex[i1], vertex[i1 + 1], vertex[i1 + 2], dgFloat32(0.0f));

	dgBigVector p1p0(p1 - p0);
	dgFloat64 mag2 = p1p0 % p1p0;
	for (dgEdge *ptr = face->m_next->m_next; mag2 < dgFloat32(1.0e-12f);
	        ptr = ptr->m_next) {
		dgInt32 ii1 = ptr->m_incidentVertex * stride;
		dgBigVector pp1(vertex[ii1], vertex[ii1 + 1], vertex[ii1 + 2], dgFloat32(0.0f));
		p1p0 = pp1 - p0;
		mag2 = p1p0 % p1p0;
	}

	dgMatrix matrix(dgGetIdentityMatrix());
	matrix.m_posit = p0;
	matrix.m_front = dgVector(p1p0.Scale(dgFloat64(1.0f) / sqrt(mag2)));
	matrix.m_right = dgVector(
	                     normal->Scale(dgFloat64(1.0f) / sqrt(*normal % *normal)));
	matrix.m_up = matrix.m_right * matrix.m_front;
	matrix = matrix.Inverse();
	matrix.m_posit.m_w = dgFloat32(1.0f);

	dgInt32 maxCount = dignonals.GetCount() * dignonals.GetCount();
	while (dignonals.GetCount() && maxCount) {
		maxCount--;
		dgList<dgDiagonalEdge>::dgListNode *const node = dignonals.GetFirst();
		dgDiagonalEdge key(node->GetInfo());
		dignonals.Remove(node);
		dgEdge *const edge = FindEdge(key.m_i0, key.m_i1);
		if (edge) {
			dgInt32 ti0 = edge->m_incidentVertex * stride;
			dgInt32 ti1 = edge->m_next->m_incidentVertex * stride;
			dgInt32 ti2 = edge->m_next->m_next->m_incidentVertex * stride;
			dgInt32 ti3 = edge->m_twin->m_prev->m_incidentVertex * stride;

			dgBigVector tp0(vertex[ti0], vertex[ti0 + 1], vertex[ti0 + 2],
			                dgFloat64(0.0f));
			dgBigVector tp1(vertex[ti1], vertex[ti1 + 1], vertex[ti1 + 2],
			                dgFloat64(0.0f));
			dgBigVector tp2(vertex[ti2], vertex[ti2 + 1], vertex[ti2 + 2],
			                dgFloat64(0.0f));
			dgBigVector tp3(vertex[ti3], vertex[ti3 + 1], vertex[ti3 + 2],
			                dgFloat64(0.0f));

			tp0 = matrix.TransformVector(tp0);
			tp1 = matrix.TransformVector(tp1);
			tp2 = matrix.TransformVector(tp2);
			tp3 = matrix.TransformVector(tp3);

			dgFloat64 circleTest[3][3];
			circleTest[0][0] = tp0[0] - tp3[0];
			circleTest[0][1] = tp0[1] - tp3[1];
			circleTest[0][2] = circleTest[0][0] * circleTest[0][0]
			                   + circleTest[0][1] * circleTest[0][1];

			circleTest[1][0] = tp1[0] - tp3[0];
			circleTest[1][1] = tp1[1] - tp3[1];
			circleTest[1][2] = circleTest[1][0] * circleTest[1][0]
			                   + circleTest[1][1] * circleTest[1][1];

			circleTest[2][0] = tp2[0] - tp3[0];
			circleTest[2][1] = tp2[1] - tp3[1];
			circleTest[2][2] = circleTest[2][0] * circleTest[2][0]
			                   + circleTest[2][1] * circleTest[2][1];

			dgFloat64 error;
			dgFloat64 det = Determinant3x3(circleTest, &error);
			if (det < dgFloat32(0.0f)) {
				dgEdge *frontFace0 = edge->m_prev;
				dgEdge *backFace0 = edge->m_twin->m_prev;

				FlipEdge(edge);

				if (perimeterCount > 4) {
					dgEdge *backFace1 = backFace0->m_next;
					dgEdge *frontFace1 = frontFace0->m_next;
					for (dgInt32 i = 0; i < perimeterCount; i++) {
						if (frontFace0 == perimeter[i]) {
							frontFace0 = NULL;
						}
						if (frontFace1 == perimeter[i]) {
							frontFace1 = NULL;
						}

						if (backFace0 == perimeter[i]) {
							backFace0 = NULL;
						}
						if (backFace1 == perimeter[i]) {
							backFace1 = NULL;
						}
					}

					if (backFace0 && (backFace0->m_incidentFace > 0)
					        && (backFace0->m_twin->m_incidentFace > 0)) {
						dgDiagonalEdge keyb0(backFace0);
						dignonals.Append(keyb0);
					}
					if (backFace1 && (backFace1->m_incidentFace > 0)
					        && (backFace1->m_twin->m_incidentFace > 0)) {
						dgDiagonalEdge keyb1(backFace1);
						dignonals.Append(keyb1);
					}

					if (frontFace0 && (frontFace0->m_incidentFace > 0)
					        && (frontFace0->m_twin->m_incidentFace > 0)) {
						dgDiagonalEdge keyf0(frontFace0);
						dignonals.Append(keyf0);
					}

					if (frontFace1 && (frontFace1->m_incidentFace > 0)
					        && (frontFace1->m_twin->m_incidentFace > 0)) {
						dgDiagonalEdge keyf1(frontFace1);
						dignonals.Append(keyf1);
					}
				}
			}
		}
	}
}

void dgPolyhedra::RefineTriangulation(const dgFloat64 *const vertex,
                                      dgInt32 stride) {
	dgEdge *edgePerimeters[1024 * 16];
	dgInt32 perimeterCount = 0;

	dgPolyhedra::Iterator iter(*this);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_incidentFace < 0) {
			dgEdge *ptr = edge;
			do {
				edgePerimeters[perimeterCount] = ptr->m_twin;
				perimeterCount++;
				NEWTON_ASSERT(perimeterCount < dgInt32(sizeof(edgePerimeters) / sizeof(edgePerimeters[0])));
				ptr = ptr->m_prev;
			} while (ptr != edge);
			break;
		}
	}
	NEWTON_ASSERT(perimeterCount);
	NEWTON_ASSERT(perimeterCount < dgInt32(sizeof(edgePerimeters) / sizeof(edgePerimeters[0])));
	edgePerimeters[perimeterCount] = edgePerimeters[0];

	dgBigVector normal(FaceNormal(edgePerimeters[0], vertex, dgInt32(stride * sizeof(dgFloat64))));
	if ((normal % normal) > dgFloat32(1.0e-12f)) {
		RefineTriangulation(vertex, stride, &normal, perimeterCount,
		                    edgePerimeters);
	}
}

void dgPolyhedra::OptimizeTriangulation(const dgFloat64 *const vertex,
                                        dgInt32 strideInBytes) {
	dgInt32 polygon[1024 * 8];
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));

	dgPolyhedra leftOver(GetAllocator());
	dgPolyhedra buildConvex(GetAllocator());

	buildConvex.BeginFace();
	dgPolyhedra::Iterator iter(*this);

	for (iter.Begin(); iter;) {
		dgEdge *const edge = &(*iter);
		iter++;

		if (edge->m_incidentFace > 0) {
			dgPolyhedra flatFace(GetAllocator());
			MarkAdjacentCoplanarFaces(flatFace, edge, vertex, strideInBytes);
			//NEWTON_ASSERT (flatFace.GetCount());

			if (flatFace.GetCount()) {
				//flatFace.Triangulate (vertex, strideInBytes, &leftOver);
				//NEWTON_ASSERT (!leftOver.GetCount());
				flatFace.RefineTriangulation(vertex, stride);

				dgInt32 mark = flatFace.IncLRU();
				dgPolyhedra::Iterator iterFF(flatFace);
				for (iterFF.Begin(); iterFF; iterFF++) {
					dgEdge *const edgeFF = &(*iterFF);
					if (edgeFF->m_mark != mark) {
						if (edgeFF->m_incidentFace > 0) {
							dgEdge *ptr = edgeFF;
							dgInt32 vertexCount = 0;
							do {
								polygon[vertexCount] = ptr->m_incidentVertex;
								vertexCount++;
								NEWTON_ASSERT(vertexCount < dgInt32(sizeof(polygon) / sizeof(polygon[0])));
								ptr->m_mark = mark;
								ptr = ptr->m_next;
							} while (ptr != edgeFF);
							if (vertexCount >= 3) {
								buildConvex.AddFace(vertexCount, polygon);
							}
						}
					}
				}
			}
			iter.Begin();
		}
	}
	buildConvex.EndFace();
	NEWTON_ASSERT(GetCount() == 0);
	SwapInfo(buildConvex);
}

void dgPolyhedra::Triangulate(const dgFloat64 *const vertex,
                              dgInt32 strideInBytes, dgPolyhedra *const leftOver) {
	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));

	dgInt32 count = GetCount() / 2;
	dgStack<char> memPool(dgInt32((count + 512) * (2 * sizeof(dgFloat64))));
	dgDownHeap<dgEdge *, dgFloat64> heap(&memPool[0], memPool.GetSizeInBytes());

	dgInt32 mark = IncLRU();
	Iterator iter(*this);
	for (iter.Begin(); iter;) {
		dgEdge *const thisEdge = &(*iter);
		iter++;

		if (thisEdge->m_mark == mark) {
			continue;
		}
		if (thisEdge->m_incidentFace < 0) {
			continue;
		}

		count = 0;
		dgEdge *ptr = thisEdge;
		do {
			count++;
			ptr->m_mark = mark;
			ptr = ptr->m_next;
		} while (ptr != thisEdge);

		if (count > 3) {
			dgEdge *const edge = TriangulateFace(thisEdge, vertex, stride, heap,
			                                     NULL);
			heap.Flush();

			if (edge) {
				NEWTON_ASSERT(edge->m_incidentFace > 0);

				if (leftOver) {
					dgInt32 *const index = (dgInt32 *) &heap[0];
					dgInt64 *const data = (dgInt64 *) &index[count];
					dgInt32 i = 0;
					dgEdge *ptrE = edge;
					do {
						index[i] = ptrE->m_incidentVertex;
						data[i] = dgInt64(ptrE->m_userData);
						i++;
						ptrE = ptrE->m_next;
					} while (ptrE != edge);
					leftOver->AddFace(i, index, data);

				} else {
					dgTrace(("Deleting face:"));
					ptr = edge;
					do {
						dgTrace(("%d ", ptr->m_incidentVertex));
					} while (ptr != edge);
					dgTrace(("\n"));
				}

				DeleteFace(edge);
				iter.Begin();
			}
		}
	}

	OptimizeTriangulation(vertex, strideInBytes);

	mark = IncLRU();
	m_faceSecuence = 1;
	for (iter.Begin(); iter; iter++) {
		dgEdge *edge = &(*iter);
		if (edge->m_mark == mark) {
			continue;
		}
		if (edge->m_incidentFace < 0) {
			continue;
		}
		NEWTON_ASSERT(edge == edge->m_next->m_next->m_next);

		for (dgInt32 i = 0; i < 3; i++) {
			edge->m_incidentFace = m_faceSecuence;
			edge->m_mark = mark;
			edge = edge->m_next;
		}
		m_faceSecuence++;
	}
}

static void RemoveColinearVertices(dgPolyhedra &flatFace,
                                   const dgFloat64 *const vertex, dgInt32 stride) {
	dgEdge *edgePerimeters[1024];

	dgInt32 perimeterCount = 0;
	dgInt32 mark = flatFace.IncLRU();
	dgPolyhedra::Iterator iter(flatFace);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if ((edge->m_incidentFace < 0) && (edge->m_mark != mark)) {
			dgEdge *ptr = edge;
			do {
				ptr->m_mark = mark;
				ptr = ptr->m_next;
			} while (ptr != edge);
			edgePerimeters[perimeterCount] = edge;
			perimeterCount++;
			NEWTON_ASSERT(perimeterCount < dgInt32(sizeof(edgePerimeters) / sizeof(edgePerimeters[0])));
		}
	}

	for (dgInt32 i = 0; i < perimeterCount; i++) {
		dgEdge *edge = edgePerimeters[i];
		dgEdge *ptr = edge;
		dgVector p0(&vertex[ptr->m_incidentVertex * stride]);
		dgVector p1(&vertex[ptr->m_next->m_incidentVertex * stride]);
		dgVector e0(p1 - p0);
		e0 = e0.Scale(dgFloat32(1.0f) / (dgSqrt(e0 % e0) + dgFloat32(1.0e-12f)));
		dgInt32 ignoreTest = 1;
		do {
			ignoreTest = 0;
			dgVector p2(&vertex[ptr->m_next->m_next->m_incidentVertex * stride]);
			dgVector e1(p2 - p1);
			e1 = e1.Scale(dgFloat32(1.0f) / (dgSqrt(e1 % e1) + dgFloat32(1.0e-12f)));
			dgFloat32 dot = e1 % e0;
			if (dot > dgFloat32(dgFloat32(0.9999f))) {

				for (dgEdge *interiorEdge = ptr->m_next->m_twin->m_next;
				        interiorEdge != ptr->m_twin;
				        interiorEdge = ptr->m_next->m_twin->m_next) {
					flatFace.DeleteEdge(interiorEdge);
				}

				if (ptr->m_twin->m_next->m_next->m_next == ptr->m_twin) {
					NEWTON_ASSERT(ptr->m_twin->m_next->m_incidentFace > 0);
					flatFace.DeleteEdge(ptr->m_twin->m_next);
				}

				NEWTON_ASSERT(ptr->m_next->m_twin->m_next->m_twin == ptr);
				edge = ptr->m_next;

				if (!flatFace.FindEdge(ptr->m_incidentVertex,
				                       edge->m_twin->m_incidentVertex)
				        && !flatFace.FindEdge(edge->m_twin->m_incidentVertex,
				                              ptr->m_incidentVertex)) {
					ptr->m_twin->m_prev = edge->m_twin->m_prev;
					edge->m_twin->m_prev->m_next = ptr->m_twin;

					edge->m_next->m_prev = ptr;
					ptr->m_next = edge->m_next;

					edge->m_next = edge->m_twin;
					edge->m_prev = edge->m_twin;
					edge->m_twin->m_next = edge;
					edge->m_twin->m_prev = edge;
					flatFace.DeleteEdge(edge);
					flatFace.ChangeEdgeIncidentVertex(ptr->m_twin,
					                                  ptr->m_next->m_incidentVertex);

					e1 = e0;
					p1 = p2;
					edge = ptr;
					ignoreTest = 1;
					continue;
				}
			}

			e0 = e1;
			p1 = p2;
			ptr = ptr->m_next;
		} while ((ptr != edge) || ignoreTest);
	}
}

static dgInt32 GetInteriorDiagonals(dgPolyhedra &polyhedra,
                                    dgEdge **const diagonals, dgInt32 maxCount) {
	dgInt32 count = 0;
	dgInt32 mark = polyhedra.IncLRU();
	dgPolyhedra::Iterator iter(polyhedra);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_mark != mark) {
			if (edge->m_incidentFace > 0) {
				if (edge->m_twin->m_incidentFace > 0) {
					edge->m_twin->m_mark = mark;
					if (count < maxCount) {
						diagonals[count] = edge;
						count++;
					}
					NEWTON_ASSERT(count <= maxCount);
				}
			}
		}
		edge->m_mark = mark;
	}

	return count;
}

static bool IsEssensialPointDiagonal(dgEdge *const diagonal,
                                     const dgBigVector &normal, const dgFloat64 *const pool, dgInt32 stride) {
	dgFloat64 dot;
	dgBigVector p0(&pool[diagonal->m_incidentVertex * stride]);
	dgBigVector p1(
	    &pool[diagonal->m_twin->m_next->m_twin->m_incidentVertex * stride]);
	dgBigVector p2(&pool[diagonal->m_prev->m_incidentVertex * stride]);

	dgBigVector e1(p1 - p0);
	dot = e1 % e1;
	if (dot < dgFloat64(1.0e-12f)) {
		return false;
	}
	e1 = e1.Scale(dgFloat64(1.0f) / sqrt(dot));

	dgBigVector e2(p2 - p0);
	dot = e2 % e2;
	if (dot < dgFloat64(1.0e-12f)) {
		return false;
	}
	e2 = e2.Scale(dgFloat64(1.0f) / sqrt(dot));

	dgBigVector n1(e1 * e2);

	dot = normal % n1;
	//if (dot > dgFloat64 (dgFloat32 (0.1f)f)) {
	//if (dot >= dgFloat64 (-1.0e-6f)) {
	if (dot >= dgFloat64(0.0f)) {
		return false;
	}
	return true;
}

static bool IsEssensialDiagonal(dgEdge *const diagonal,
                                const dgBigVector &normal, const dgFloat64 *const pool, dgInt32 stride) {
	return IsEssensialPointDiagonal(diagonal, normal, pool, stride)
	       || IsEssensialPointDiagonal(diagonal->m_twin, normal, pool, stride);
}

void dgPolyhedra::ConvexPartition(const dgFloat64 *const vertex,
                                  dgInt32 strideInBytes, dgPolyhedra *const leftOversOut) {
	if (GetCount()) {
		Triangulate(vertex, strideInBytes, leftOversOut);
		DeleteDegenerateFaces(vertex, strideInBytes, dgFloat32(1.0e-5f));
		Optimize(vertex, strideInBytes, dgFloat32(1.0e-4f));
		DeleteDegenerateFaces(vertex, strideInBytes, dgFloat32(1.0e-5f));

		if (GetCount()) {
			dgInt32 removeCount = 0;
			dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));

			dgInt32 polygon[1024 * 8];
			dgEdge *diagonalsPool[1024 * 8];
			dgPolyhedra buildConvex(GetAllocator());

			buildConvex.BeginFace();
			dgPolyhedra::Iterator iter(*this);
			for (iter.Begin(); iter;) {
				dgEdge *edge = &(*iter);
				iter++;
				if (edge->m_incidentFace > 0) {

					dgPolyhedra flatFace(GetAllocator());
					MarkAdjacentCoplanarFaces(flatFace, edge, vertex, strideInBytes);

					if (flatFace.GetCount()) {
						flatFace.RefineTriangulation(vertex, stride);
						RemoveColinearVertices(flatFace, vertex, stride);

						dgInt32 diagonalCount = GetInteriorDiagonals(flatFace, diagonalsPool, sizeof(diagonalsPool) / sizeof(diagonalsPool[0]));
						if (diagonalCount) {
							edge = &flatFace.GetRoot()->GetInfo();
							if (edge->m_incidentFace < 0) {
								edge = edge->m_twin;
							}
							NEWTON_ASSERT(edge->m_incidentFace > 0);

							dgBigVector normal(FaceNormal(edge, vertex, strideInBytes));
							normal = normal.Scale(dgFloat64(1.0f) / sqrt(normal % normal));

							edge = NULL;
							dgPolyhedra::Iterator iterFF(flatFace);
							for (iterFF.Begin(); iterFF; iterFF++) {
								edge = &(*iterFF);
								if (edge->m_incidentFace < 0) {
									break;
								}
							}
							NEWTON_ASSERT(edge);

							dgInt32 isConvex = 1;
							dgEdge *ptr = edge;
							dgInt32 mark = flatFace.IncLRU();

							dgBigVector normal2(normal);
							dgBigVector p0(&vertex[ptr->m_prev->m_incidentVertex * stride]);
							dgBigVector p1(&vertex[ptr->m_incidentVertex * stride]);
							dgBigVector e0(p1 - p0);
							e0 = e0.Scale(
							         dgFloat32(1.0f) / (dgSqrt(e0 % e0) + dgFloat32(1.0e-14f)));
							do {
								dgBigVector p2(&vertex[ptr->m_next->m_incidentVertex * stride]);
								dgBigVector e1(p2 - p1);
								e1 = e1.Scale(dgFloat32(1.0f) / (sqrt(e1 % e1) + dgFloat32(1.0e-14f)));
								dgFloat64 dot = (e0 * e1) % normal2;
								//if (dot > dgFloat32 (0.0f)) {
								if (dot > dgFloat32(5.0e-3f)) {
									isConvex = 0;
									break;
								}
								ptr->m_mark = mark;
								e0 = e1;
								p1 = p2;
								ptr = ptr->m_next;
							} while (ptr != edge);

							if (isConvex) {
								dgPolyhedra::Iterator iterF(flatFace);
								for (iterF.Begin(); iterF; iterF++) {
									ptr = &(*iterF);
									if (ptr->m_incidentFace < 0) {
										if (ptr->m_mark < mark) {
											isConvex = 0;
											break;
										}
									}
								}
							}

							if (isConvex) {
								if (diagonalCount > 2) {
									dgInt32 count = 0;
									ptr = edge;
									do {
										polygon[count] = ptr->m_incidentVertex;
										count++;
										NEWTON_ASSERT(count < dgInt32(sizeof(polygon) / sizeof(polygon[0])));
										ptr = ptr->m_next;
									} while (ptr != edge);

									for (dgInt32 i = 0; i < count - 1; i++) {
										for (dgInt32 j = i + 1; j < count; j++) {
											if (polygon[i] == polygon[j]) {
												i = count;
												isConvex = 0;
												break;
											}
										}
									}
								}
							}

							if (isConvex) {
								for (dgInt32 j = 0; j < diagonalCount; j++) {
									dgEdge *const diagonal = diagonalsPool[j];
									removeCount++;
									flatFace.DeleteEdge(diagonal);
								}
							} else {
								for (dgInt32 j = 0; j < diagonalCount; j++) {
									dgEdge *const diagonal = diagonalsPool[j];
									if (!IsEssensialDiagonal(diagonal, normal, vertex, stride)) {
										removeCount++;
										flatFace.DeleteEdge(diagonal);
									}
								}
							}
						}

						dgInt32 mark = flatFace.IncLRU();
						dgPolyhedra::Iterator iterFLF(flatFace);
						for (iterFLF.Begin(); iterFLF; iterFLF++) {
							dgEdge *const edgeFLF = &(*iterFLF);
							if (edgeFLF->m_mark != mark) {
								if (edgeFLF->m_incidentFace > 0) {
									dgEdge *ptr = edgeFLF;
									dgInt32 diagonalCountTemp = 0;
									do {
										polygon[diagonalCountTemp] = ptr->m_incidentVertex;
										diagonalCountTemp++;
										NEWTON_ASSERT(diagonalCountTemp < dgInt32(sizeof(polygon) / sizeof(polygon[0])));
										ptr->m_mark = mark;
										ptr = ptr->m_next;
									} while (ptr != edgeFLF);
									if (diagonalCountTemp >= 3) {
										buildConvex.AddFace(diagonalCountTemp, polygon);
									}
								}
							}
						}
					}
					iter.Begin();
				}
			}

			buildConvex.EndFace();
			NEWTON_ASSERT(GetCount() == 0);
			SwapInfo(buildConvex);
		}
	}
}

dgSphere dgPolyhedra::CalculateSphere(const dgFloat64 *const vertex,
                                      dgInt32 strideInBytes, const dgMatrix *const basis) const {
	dgStack<dgInt32> pool(GetCount() * 3 + 6);
	dgInt32 *const indexList = &pool[0];

	dgMatrix axis(dgGetIdentityMatrix());
	dgBigVector p0(dgFloat32(1.0e10f), dgFloat32(1.0e10f), dgFloat32(1.0e10f),
	               dgFloat32(0.0f));
	dgBigVector p1(dgFloat32(-1.0e10f), dgFloat32(-1.0e10f), dgFloat32(-1.0e10f),
	               dgFloat32(0.0f));

	dgInt32 stride = dgInt32(strideInBytes / sizeof(dgFloat64));
	dgInt32 indexCount = 0;
	dgInt32 mark = IncLRU();
	dgPolyhedra::Iterator iter(*this);
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		if (edge->m_mark != mark) {
			dgEdge *ptr = edge;
			do {
				ptr->m_mark = mark;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge);
			dgInt32 index = edge->m_incidentVertex;
			indexList[indexCount + 6] = edge->m_incidentVertex;
			dgBigVector point(vertex[index * stride + 0], vertex[index * stride + 1],
			                  vertex[index * stride + 2], dgFloat32(0.0f));
			for (dgInt32 i = 0; i < 3; i++) {
				if (point[i] < p0[i]) {
					p0[i] = point[i];
					indexList[i * 2 + 0] = index;
				}
				if (point[i] > p1[i]) {
					p1[i] = point[i];
					indexList[i * 2 + 1] = index;
				}
			}
			indexCount++;
		}
	}
	indexCount += 6;

	dgBigVector size(p1 - p0);
	dgFloat64 volume = size.m_x * size.m_y * size.m_z;

	for (dgFloat32 pitch = dgFloat32(0.0f); pitch < dgFloat32(90.0f); pitch +=
	            dgFloat32(10.0f)) {
		dgMatrix pitchMatrix(
		    dgPitchMatrix(pitch * dgFloat32(3.1416f) / dgFloat32(180.0f)));
		for (dgFloat32 yaw = dgFloat32(0.0f); yaw < dgFloat32(90.0f); yaw +=
		            dgFloat32(10.0f)) {
			dgMatrix yawMatrix(
			    dgYawMatrix(yaw * dgFloat32(3.1416f) / dgFloat32(180.0f)));
			for (dgFloat32 roll = dgFloat32(0.0f); roll < dgFloat32(90.0f); roll +=
			            dgFloat32(10.0f)) {
				dgInt32 tmpIndex[6];
				dgMatrix rollMatrix(
				    dgRollMatrix(roll * dgFloat32(3.1416f) / dgFloat32(180.0f)));
				dgMatrix tmp(pitchMatrix * yawMatrix * rollMatrix);
				dgBigVector q0(dgFloat32(1.0e10f), dgFloat32(1.0e10f),
				               dgFloat32(1.0e10f), dgFloat32(0.0f));
				dgBigVector q1(dgFloat32(-1.0e10f), dgFloat32(-1.0e10f),
				               dgFloat32(-1.0e10f), dgFloat32(0.0f));

				dgFloat32 volume1 = dgFloat32(1.0e10f);
				for (dgInt32 i = 0; i < indexCount; i++) {
					dgInt32 index = indexList[i];
					dgBigVector point(vertex[index * stride + 0],
					                  vertex[index * stride + 1], vertex[index * stride + 2],
					                  dgFloat32(0.0f));
					point = tmp.UnrotateVector(point);

					for (dgInt32 j = 0; j < 3; j++) {
						if (point[j] < q0[j]) {
							q0[j] = point[j];
							tmpIndex[j * 2 + 0] = index;
						}
						if (point[j] > q1[j]) {
							q1[j] = point[j];
							tmpIndex[j * 2 + 1] = index;
						}
					}

					dgVector size1(q1 - q0);
					volume1 = size1.m_x * size1.m_y * size1.m_z;
					if (volume1 >= volume) {
						break;
					}
				}

				if (volume1 < volume) {
					p0 = q0;
					p1 = q1;
					axis = tmp;
					volume = volume1;
					memcpy(indexList, tmpIndex, sizeof(tmpIndex));
				}
			}
		}
	}

	dgSphere sphere(axis);
	dgVector q0(p0);
	dgVector q1(p1);
	sphere.m_posit = axis.RotateVector((q1 + q0).Scale(dgFloat32(0.5f)));
	sphere.m_size = (q1 - q0).Scale(dgFloat32(0.5f));
	return sphere;
}
