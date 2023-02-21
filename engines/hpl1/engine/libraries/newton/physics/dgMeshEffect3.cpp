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
#include "dgCollisionConvexHull.h"
#include "dgMeshEffect.h"
#include "dgMeshEffectSolidTree.h"
#include "dgWorld.h"
#include "hpl1/engine/libraries/newton/core/dg.h"

// based of the paper Hierarchical Approximate Convex Decomposition by Khaled Mamou
// with his permission to adapt his algorithm so be more efficient.
// available http://sourceforge.net/projects/hacd/
// for the details http://kmamou.blogspot.com/

class dgClusterFace {
public:
	dgClusterFace() {
	}
	~dgClusterFace() {
	}

	dgEdge *m_edge;
	dgFloat64 m_area;
	dgFloat64 m_perimeter;
	dgBigVector m_normal;
};

class dgPairProxi {
public:
	dgPairProxi()
		: m_edgeA(NULL), m_edgeB(NULL), m_area(dgFloat64(0.0f)), m_perimeter(dgFloat64(0.0f)) {
	}

	~dgPairProxi() {
	}

	dgEdge *m_edgeA;
	dgEdge *m_edgeB;
	dgFloat64 m_area;
	dgFloat64 m_perimeter;
};

class dgClusterList : public dgList<dgClusterFace> {
public:
	dgClusterList(dgMemoryAllocator *const allocator)
		: dgList<dgClusterFace>(allocator), m_area(dgFloat32(0.0f)), m_perimeter(dgFloat32(0.0f)) {
	}

	~dgClusterList() {
	}

	dgInt32 AddVertexToPool(const dgMeshEffect &mesh, dgBigVector *const vertexPool, dgInt32 *const vertexMarks, dgInt32 vertexMark) {
		dgInt32 count = 0;

		const dgBigVector *const points = (dgBigVector *)mesh.GetVertexPool();
		for (dgListNode *node = GetFirst(); node; node = node->GetNext()) {
			dgClusterFace &face = node->GetInfo();

			dgEdge *edge = face.m_edge;
			do {
				dgInt32 index = edge->m_incidentVertex;
				if (vertexMarks[index] != vertexMark) {
					vertexMarks[index] = vertexMark;
					vertexPool[count] = points[index];
					count++;
				}
				edge = edge->m_next;
			} while (edge != face.m_edge);
		}
		return count;
	}

	dgFloat64 CalculateTriangleConcavity2(const dgConvexHull3d &convexHull, dgClusterFace &info, dgInt32 i0, dgInt32 i1, dgInt32 i2, const dgBigVector *const points) const {
		dgUnsigned32 head = 1;
		dgUnsigned32 tail = 0;
		dgBigVector pool[1 << 8][3];

		pool[0][0] = points[i0];
		pool[0][1] = points[i1];
		pool[0][2] = points[i2];

		const dgBigVector step(info.m_normal.Scale(dgFloat64(4.0f) * convexHull.GetDiagonal()));

		dgFloat64 concavity = dgFloat32(0.0f);
		dgFloat64 minArea = dgFloat32(0.125f);
		dgFloat64 minArea2 = minArea * minArea * 0.5f;

		// weight the area by the area of the face
		// dgBigVector edge10(pool[0][1] - pool[0][0]);
		// dgBigVector edge20(pool[0][2] - pool[0][0]);
		// dgBigVector triangleArea = edge10 * edge20;
		// dgFloat64 triangleArea2 = triangleArea % triangleArea;
		// if ((triangleArea2 / minArea2)> dgFloat32 (64.0f)) {
		// minArea2 = triangleArea2 / dgFloat32 (64.0f);
		//}

		dgInt32 maxCount = 4;
		dgUnsigned32 mask = (sizeof(pool) / (3 * sizeof(pool[0][0]))) - 1;
		while ((tail != head) && (maxCount >= 0)) {
			// stack--;
			maxCount--;
			dgBigVector p0(pool[tail][0]);
			dgBigVector p1(pool[tail][1]);
			dgBigVector p2(pool[tail][2]);
			tail = (tail + 1) & mask;

			dgBigVector q1((p0 + p1 + p2).Scale(dgFloat64(1.0f / 3.0f)));
			dgBigVector q0(q1 + step);

			dgFloat64 param = convexHull.RayCast(q0, q1);
			if (param > dgFloat64(1.0f)) {
				param = dgFloat64(1.0f);
			}
			dgBigVector dq(step.Scale(dgFloat32(1.0f) - param));
			dgFloat64 lenght2 = dq % dq;
			if (lenght2 > concavity) {
				concavity = lenght2;
			}

			if (((head + 1) & mask) != tail) {
				dgBigVector edge10(p1 - p0);
				dgBigVector edge20(p2 - p0);
				dgBigVector n(edge10 * edge20);
				dgFloat64 area2 = n % n;
				if (area2 > minArea2) {
					dgBigVector p01((p0 + p1).Scale(dgFloat64(0.5f)));
					dgBigVector p12((p1 + p2).Scale(dgFloat64(0.5f)));
					dgBigVector p20((p2 + p0).Scale(dgFloat64(0.5f)));

					pool[head][0] = p0;
					pool[head][1] = p01;
					pool[head][2] = p20;
					head = (head + 1) & mask;

					if (((head + 1) & mask) != tail) {
						pool[head][0] = p1;
						pool[head][1] = p12;
						pool[head][2] = p01;
						head = (head + 1) & mask;

						if (((head + 1) & mask) != tail) {
							pool[head][0] = p2;
							pool[head][1] = p20;
							pool[head][2] = p12;
							head = (head + 1) & mask;
						}
					}
				}
			}
		}
		return concavity;
	}

	dgFloat64 CalculateConcavity2(const dgConvexHull3d &convexHull, const dgMeshEffect &mesh) {
		dgFloat64 concavity = dgFloat32(0.0f);

		const dgBigVector *const points = (dgBigVector *)mesh.GetVertexPool();

		for (dgListNode *node = GetFirst(); node; node = node->GetNext()) {
			dgClusterFace &info = node->GetInfo();
			dgInt32 i0 = info.m_edge->m_incidentVertex;
			dgInt32 i1 = info.m_edge->m_next->m_incidentVertex;
			for (dgEdge *edge = info.m_edge->m_next->m_next; edge != info.m_edge; edge = edge->m_next) {
				dgInt32 i2 = edge->m_incidentVertex;
				dgFloat64 val = CalculateTriangleConcavity2(convexHull, info, i0, i1, i2, points);
				if (val > concavity) {
					concavity = val;
				}
				i1 = i2;
			}
		}

		return concavity;
	}

	bool IsClusterCoplanar(const dgBigPlane &plane,
	                       const dgMeshEffect &mesh) const {
		const dgBigVector *const points = (dgBigVector *)mesh.GetVertexPool();
		for (dgListNode *node = GetFirst(); node; node = node->GetNext()) {
			dgClusterFace &info = node->GetInfo();

			dgEdge *ptr = info.m_edge;
			do {
				const dgBigVector &p = points[ptr->m_incidentVertex];
				dgFloat64 dist = fabs(plane.Evalue(p));
				if (dist > dgFloat64(1.0e-5f)) {
					return false;
				}
				ptr = ptr->m_next;
			} while (ptr != info.m_edge);
		}

		return true;
	}

	bool IsEdgeConvex(const dgBigPlane &plane, const dgMeshEffect &mesh,
	                  dgEdge *const edge) const {
		const dgBigVector *const points = (dgBigVector *)mesh.GetVertexPool();
		dgEdge *const edge0 = edge->m_next;
		dgEdge *ptr = edge0->m_twin->m_next;
		do {
			if (ptr->m_twin->m_incidentFace == edge->m_twin->m_incidentFace) {
				NEWTON_ASSERT(edge0->m_incidentVertex == ptr->m_incidentVertex);
				dgBigVector e0(points[edge0->m_twin->m_incidentVertex] - points[edge0->m_incidentVertex]);
				dgBigVector e1(points[ptr->m_twin->m_incidentVertex] - points[edge0->m_incidentVertex]);
				dgBigVector normal(e0 * e1);
				return (normal % plane) > dgFloat64(0.0f);
			}
			ptr = ptr->m_twin->m_next;
		} while (ptr != edge->m_twin);

		NEWTON_ASSERT(0);
		return true;
	}

	// calculate the convex hull of a conched group of faces,
	// and measure the concavity, according to Khaled convexity criteria, which is basically
	// has two components,
	// the first is ratio  between the the perimeter of the group of faces
	// and the second the largest distance from any of the face to the surface of the hull
	// when the faces are are a strip of a convex hull the perimeter ratio components is 1.0 and the distance to the hull is zero
	// this is the ideal concavity.
	// when the face are no part of the hull, then the worse distance to the hull is dominate the the metric
	// this matrix is used to place all possible combination of this cluster with any adjacent cluster into a priority heap and determine
	// which pair of two adjacent cluster is the best selection for combining the into a larger cluster.
	void CalculateNodeCost(dgMeshEffect &mesh, dgInt32 meshMask,
	                       dgBigVector *const vertexPool, dgInt32 *const vertexMarks,
	                       dgInt32 &vertexMark, dgClusterList *const clusters, dgFloat64 diagonalInv,
	                       dgFloat64 aspectRatioCoeficent, dgList<dgPairProxi> &proxyList,
	                       dgUpHeap<dgList<dgPairProxi>::dgListNode *, dgFloat64> &heap) {
		dgInt32 faceIndex = GetFirst()->GetInfo().m_edge->m_incidentFace;

		const dgBigVector *const points = (dgBigVector *)mesh.GetVertexPool();

		bool flatStrip = true;
		dgBigPlane plane(GetFirst()->GetInfo().m_normal, -(points[GetFirst()->GetInfo().m_edge->m_incidentVertex] % GetFirst()->GetInfo().m_normal));
		if (GetCount() > 1) {
			flatStrip = IsClusterCoplanar(plane, mesh);
		}

		vertexMark++;
		dgInt32 vertexCount = AddVertexToPool(mesh, vertexPool, vertexMarks, vertexMark);
		for (dgListNode *node = GetFirst(); node; node = node->GetNext()) {
			// dgClusterFace& clusterFaceA = GetFirst()->GetInfo();
			dgClusterFace &clusterFaceA = node->GetInfo();

			dgEdge *edge = clusterFaceA.m_edge;
			do {
				dgInt32 twinFaceIndex = edge->m_twin->m_incidentFace;
				if ((edge->m_mark != meshMask) && (twinFaceIndex != faceIndex) && (twinFaceIndex > 0)) {

					dgClusterList &clusterListB = clusters[twinFaceIndex];

					vertexMark++;
					dgInt32 extraCount = clusterListB.AddVertexToPool(mesh, &vertexPool[vertexCount], &vertexMarks[0], vertexMark);

					dgInt32 count = vertexCount + extraCount;
					dgConvexHull3d convexHull(mesh.GetAllocator(), &vertexPool[0].m_x, sizeof(dgBigVector), count, 0.0);

					dgFloat64 concavity = dgFloat64(0.0f);
					if (convexHull.GetVertexCount()) {
						concavity = sqrt(GetMax(CalculateConcavity2(convexHull, mesh), clusterListB.CalculateConcavity2(convexHull, mesh)));
						if (concavity < dgFloat64(1.0e-3f)) {
							concavity = dgFloat64(0.0f);
						}
					}

					if ((concavity == dgFloat64(0.0f)) && flatStrip) {
						if (clusterListB.IsClusterCoplanar(plane, mesh)) {
							bool concaveEdge = !(IsEdgeConvex(plane, mesh, edge) && IsEdgeConvex(plane, mesh, edge->m_twin));
							if (concaveEdge) {
								concavity += 1000.0f;
							}
						}
					}

					dgBigVector p1p0(points[edge->m_twin->m_incidentVertex] - points[edge->m_incidentVertex]);
					dgFloat64 edgeLength = dgFloat64(2.0f) * sqrt(p1p0 % p1p0);

					dgFloat64 area = m_area + clusterListB.m_area;
					dgFloat64 perimeter = m_perimeter + clusterListB.m_perimeter - edgeLength;
					dgFloat64 edgeCost = perimeter * perimeter / (dgFloat64(4.0f * 3.141592f) * area);
					dgFloat64 cost = diagonalInv * (concavity + edgeCost * aspectRatioCoeficent);

					if ((heap.GetCount() + 20) > heap.GetMaxCount()) {
						for (dgInt32 i = heap.GetCount() - 1; i >= 0; i--) {
							dgList<dgPairProxi>::dgListNode *emptyNode = heap[i];
							dgPairProxi &emptyPair = emptyNode->GetInfo();
							if ((emptyPair.m_edgeA == NULL) && (emptyPair.m_edgeB == NULL)) {
								heap.Remove(i);
							}
						}
					}

					dgList<dgPairProxi>::dgListNode *pairNode = proxyList.Append();
					dgPairProxi &pair = pairNode->GetInfo();
					pair.m_edgeA = edge;
					pair.m_edgeB = edge->m_twin;
					pair.m_area = area;
					pair.m_perimeter = perimeter;
					edge->m_userData = dgUnsigned64(pairNode);
					edge->m_twin->m_userData = dgUnsigned64(pairNode);
					heap.Push(pairNode, cost);
				}

				edge->m_mark = meshMask;
				edge->m_twin->m_mark = meshMask;
				edge = edge->m_next;
			} while (edge != clusterFaceA.m_edge);
		}
	}

	dgFloat64 m_area;
	dgFloat64 m_perimeter;
};

dgMeshEffect::dgMeshEffect(const dgMeshEffect &source, dgFloat32 absoluteconcavity, dgInt32 maxCount)
	: dgPolyhedra(source.GetAllocator()) {
	Init(true);

	dgMeshEffect mesh(source);
	dgInt32 faceCount = mesh.GetTotalFaceCount() + 1;
	dgStack<dgClusterList> clusterPool(faceCount);
	dgClusterList *const clusters = &clusterPool[0];

	for (dgInt32 i = 0; i < faceCount; i++) {
		clusters[i] = dgClusterList(mesh.GetAllocator());
	}

	dgInt32 meshMask = mesh.IncLRU();
	const dgBigVector *const points = mesh.m_points;

	// enumerate all faces, and initialize cluster pool
	dgMeshEffect::Iterator iter(mesh);

	dgInt32 clusterIndex = 1;
	for (iter.Begin(); iter; iter++) {
		dgEdge *const edge = &(*iter);
		edge->m_userData = dgUnsigned64(-1);
		if ((edge->m_mark != meshMask) && (edge->m_incidentFace > 0)) {
			dgFloat64 perimeter = dgFloat64(0.0f);
			dgEdge *ptr = edge;
			do {
				dgBigVector p1p0(points[ptr->m_incidentVertex] - points[ptr->m_prev->m_incidentVertex]);
				perimeter += sqrt(p1p0 % p1p0);
				ptr->m_incidentFace = clusterIndex;

				ptr->m_mark = meshMask;
				ptr = ptr->m_next;
			} while (ptr != edge);

			dgBigVector normal = mesh.FaceNormal(edge, &points[0][0], sizeof(dgBigVector));
			dgFloat64 mag = sqrt(normal % normal);

			dgClusterFace &faceInfo = clusters[clusterIndex].Append()->GetInfo();

			faceInfo.m_edge = edge;
			faceInfo.m_perimeter = perimeter;
			faceInfo.m_area = dgFloat64(0.5f) * mag;
			faceInfo.m_normal = normal.Scale(dgFloat64(1.0f) / mag);

			clusters[clusterIndex].m_perimeter = perimeter;
			clusters[clusterIndex].m_area = faceInfo.m_area;

			clusterIndex++;
		}
	}

	NEWTON_ASSERT(faceCount == clusterIndex);

	// recalculate all edge cost
	dgStack<dgInt32> vertexMarksArray(mesh.GetVertexCount());
	dgStack<dgBigVector> vertexArray(mesh.GetVertexCount() * 2);

	dgBigVector *const vertexPool = &vertexArray[0];
	dgInt32 *const vertexMarks = &vertexMarksArray[0];
	memset(&vertexMarks[0], 0, vertexMarksArray.GetSizeInBytes());

	dgList<dgPairProxi> proxyList(mesh.GetAllocator());
	dgUpHeap<dgList<dgPairProxi>::dgListNode *, dgFloat64> heap(mesh.GetCount() + 1000, mesh.GetAllocator());

	dgInt32 vertexMark = 0;

	dgFloat64 diagonalInv = dgFloat32(1.0f);
	dgFloat64 aspectRatioCoeficent = absoluteconcavity / dgFloat32(10.0f);
	meshMask = mesh.IncLRU();

	// calculate all the initial cost of all clusters, which at this time are all a single faces
	for (dgInt32 faceIndex = 1; faceIndex < faceCount; faceIndex++) {
		vertexMark++;
		dgClusterList &clusterList = clusters[faceIndex];
		NEWTON_ASSERT(clusterList.GetFirst()->GetInfo().m_edge->m_incidentFace == faceIndex);
		clusterList.CalculateNodeCost(mesh, meshMask, &vertexPool[0], &vertexMarks[0], vertexMark, &clusters[0], diagonalInv, aspectRatioCoeficent, proxyList, heap);
	}

	// calculate all essential convex clusters by merging the all possible clusters according
	// which combined concavity es lower that the max absolute concavity
	// select the pair with the smaller concavity and fuse then into a larger cluster
	dgInt32 essencialClustersCount = faceCount - 1;
	while (heap.GetCount() && ((heap.Value() < absoluteconcavity) || (essencialClustersCount > maxCount))) {
		dgList<dgPairProxi>::dgListNode *const pairNode = heap[0];
		heap.Pop();
		dgPairProxi &pair = pairNode->GetInfo();

		NEWTON_ASSERT((pair.m_edgeA && pair.m_edgeA) || (!pair.m_edgeA && !pair.m_edgeA));
		if (pair.m_edgeA && pair.m_edgeB) {

			NEWTON_ASSERT(pair.m_edgeA->m_incidentFace != pair.m_edgeB->m_incidentFace);

			// merge two clusters
			dgInt32 faceIndexA = pair.m_edgeA->m_incidentFace;
			dgInt32 faceIndexB = pair.m_edgeB->m_incidentFace;
			dgClusterList *listA = &clusters[faceIndexA];
			dgClusterList *listB = &clusters[faceIndexB];
			if (pair.m_edgeA->m_incidentFace > pair.m_edgeB->m_incidentFace) {
				Swap(faceIndexA, faceIndexB);
				Swap(listA, listB);
			}

			while (listB->GetFirst()) {
				dgClusterList::dgListNode *const nodeB = listB->GetFirst();
				listB->Unlink(nodeB);
				dgClusterFace &faceB = nodeB->GetInfo();

				dgEdge *ptr = faceB.m_edge;
				do {
					ptr->m_incidentFace = faceIndexA;
					ptr = ptr->m_next;
				} while (ptr != faceB.m_edge);
				listA->Append(nodeB);
			}
			essencialClustersCount--;

			listB->m_area = dgFloat32(0.0f);
			listB->m_perimeter = dgFloat32(0.0f);
			listA->m_area = pair.m_area;
			listA->m_perimeter = pair.m_perimeter;

			// recalculated the new metric for the new cluster, and tag the used cluster as invalid, so that
			// other potential selection do not try merge with this this one, producing convex that re use a face more than once
			dgInt32 mark = mesh.IncLRU();
			for (dgClusterList::dgListNode *node = listA->GetFirst(); node; node = node->GetNext()) {
				dgClusterFace &face = node->GetInfo();
				dgEdge *ptr = face.m_edge;
				do {
					if (ptr->m_userData != dgUnsigned64(-1)) {
						dgList<dgPairProxi>::dgListNode *const pairNodeT = (dgList<dgPairProxi>::dgListNode *)ptr->m_userData;
						dgPairProxi &pairProxy = pairNodeT->GetInfo();
						pairProxy.m_edgeA = NULL;
						pairProxy.m_edgeB = NULL;
					}
					ptr->m_userData = dgUnsigned64(-1);
					ptr->m_twin->m_userData = dgUnsigned64(-1);

					if ((ptr->m_twin->m_incidentFace == faceIndexA) || (ptr->m_twin->m_incidentFace < 0)) {
						ptr->m_mark = mark;
						ptr->m_twin->m_mark = mark;
					}

					if (ptr->m_mark != mark) {
						dgClusterList &adjacentList = clusters[ptr->m_twin->m_incidentFace];
						for (dgClusterList::dgListNode *adjacentNode = adjacentList.GetFirst(); adjacentNode; adjacentNode = adjacentNode->GetNext()) {
							dgClusterFace &adjacentFace = adjacentNode->GetInfo();
							dgEdge *adjacentEdge = adjacentFace.m_edge;
							do {
								if (adjacentEdge->m_twin->m_incidentFace == faceIndexA) {
									adjacentEdge->m_twin->m_mark = mark;
								}
								adjacentEdge = adjacentEdge->m_next;
							} while (adjacentEdge != adjacentFace.m_edge);
						}
						ptr->m_mark = mark - 1;
					}
					ptr = ptr->m_next;
				} while (ptr != face.m_edge);
			}

			// re generated the cost of merging this new all its adjacent clusters, that are still alive.
			vertexMark++;
			listA->CalculateNodeCost(mesh, mark, &vertexPool[0], &vertexMarks[0], vertexMark, &clusters[0], diagonalInv, aspectRatioCoeficent, proxyList, heap);
		}

		proxyList.Remove(pairNode);
	}

	// if the essential convex cluster count is larger than the the maximum specified by the user
	// then resuming the cluster again to the heap and start merging then by the the worse merging criteria
	// also at this time add the distance heuristic to combine disjoint cluster as well.
	// this is where I disagree with Khaled Mamore, he uses a brute force approach by adding extra points.
	// I do this to the first partition and then connect disjoint face only on the perimeter
	/*
	maxCount = 1;
	    while (essencialClustersCount > maxCount) {

	        heap.Flush();
	        meshMask = mesh.IncLRU();

	        // color code each face on each cluster with it cluster index
	        for (dgInt32 faceIndex = 0; faceIndex < faceCount; faceIndex++) {
	            dgClusterList& clusterList = clusters[faceIndex];
	            if (clusterList.GetCount()) {
	                for (dgClusterList::dgListNode* node = clusterList.GetFirst(); node; node = node->GetNext()) {
	                    dgClusterFace& face = node->GetInfo();
	                    dgEdge* ptr = face.m_edge;
	                    do {
	                        ptr->m_incidentFace = faceIndex;
	                        ptr = ptr->m_next;
	                    } while (ptr != face.m_edge);
	                }
	            }
	        }

	        for (dgInt32 faceIndex = 0; faceIndex < faceCount; faceIndex++) {
	            dgClusterList& clusterList = clusters[faceIndex];

	            // note: add the disjoint cluster criteria here, but for now just ignore

	            // calculate the cost again
	            if (clusterList.GetCount()) {

	                // note: something is wrong with my color coding that is not marking the perimeter corrently

	                vertexMark++;
	                clusterList.CalculateNodeCost(mesh, meshMask, &vertexPool[0], &vertexMarks[0], vertexMark, &clusters[0], diagonalInv, aspectRatioCoeficent, proxyList, heap);
	            }

	        }
	    }
	*/

	BeginPolygon();
	dgFloat32 layer = dgFloat32(0.0f);

	dgVertexAtribute polygon[256];
	for (uint i = 0; i < ARRAYSIZE(polygon); i++) polygon[i].clear();
	dgArray<dgBigVector> convexVertexBuffer(1024, GetAllocator());
	for (dgInt32 i = 0; i < faceCount; i++) {
		dgClusterList &clusterList = clusters[i];

		if (clusterList.GetCount()) {
			dgInt32 count = 0;
			for (dgClusterList::dgListNode *node = clusterList.GetFirst(); node; node = node->GetNext()) {
				dgClusterFace &face = node->GetInfo();
				dgEdge *edge = face.m_edge;

				dgEdge *sourceEdge = source.FindEdge(edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
				do {
					dgInt32 index = edge->m_incidentVertex;
					convexVertexBuffer[count] = points[index];

					count++;
					sourceEdge = sourceEdge->m_next;
					edge = edge->m_next;
				} while (edge != face.m_edge);
			}

			dgConvexHull3d convexHull(mesh.GetAllocator(), &convexVertexBuffer[0].m_x, sizeof(dgBigVector), count, 0.0);

			if (convexHull.GetCount()) {
				const dgBigVector *const vertex = convexHull.GetVertexPool();
				for (dgConvexHull3d::dgListNode *node = convexHull.GetFirst(); node; node = node->GetNext()) {
					const dgConvexHull3DFace *const face = &node->GetInfo();

					dgInt32 i0 = face->m_index[0];
					dgInt32 i1 = face->m_index[1];
					dgInt32 i2 = face->m_index[2];

					polygon[0].m_vertex = vertex[i0];
					polygon[0].m_vertex.m_w = layer;

					polygon[1].m_vertex = vertex[i1];
					polygon[1].m_vertex.m_w = layer;

					polygon[2].m_vertex = vertex[i2];
					polygon[2].m_vertex.m_w = layer;

					AddPolygon(3, &polygon[0].m_vertex.m_x, sizeof(dgVertexAtribute), 0);
				}

				layer += dgFloat32(1.0f);
				// break;
			}
		}
	}
	EndPolygon(1.0e-5f);

	for (dgInt32 i = 0; i < faceCount; i++) {
		clusters[i].RemoveAll();
	}
}

dgMeshEffect *dgMeshEffect::CreateConvexApproximation(dgFloat32 maxConcavity, dgInt32 maxCount) const {
	dgMeshEffect triangleMesh(*this);
	if (maxCount <= 1) {
		maxCount = 1;
	}
	if (maxConcavity <= dgFloat32(1.0e-5f)) {
		maxConcavity = dgFloat32(1.0e-5f);
	}
	dgMeshEffect *const convexPartion = new (GetAllocator()) dgMeshEffect(triangleMesh, maxConcavity, maxCount);
	return convexPartion;
}
