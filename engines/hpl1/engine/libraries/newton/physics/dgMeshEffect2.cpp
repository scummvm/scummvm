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

// create a convex hull
dgMeshEffect::dgMeshEffect(dgMemoryAllocator *const allocator,
                           const dgFloat64 *const vertexCloud, dgInt32 count, dgInt32 strideInByte,
                           dgFloat64 distTol) : dgPolyhedra(allocator) {
	Init(true);
	if (count >= 4) {
		dgConvexHull3d convexHull(allocator, vertexCloud, strideInByte, count,
		                          distTol);
		if (convexHull.GetCount()) {

			dgInt32 vertexCount = convexHull.GetVertexCount();
			dgStack<dgVector> pointsPool(convexHull.GetVertexCount());
			dgVector *const points = &pointsPool[0];
			for (dgInt32 i = 0; i < vertexCount; i++) {
				points[i] = convexHull.GetVertex(i);
			}
			dgVector uv(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
			            dgFloat32(0.0f));
			dgVector normal(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
			                dgFloat32(0.0f));

			dgInt32 triangleCount = convexHull.GetCount();
			dgStack<dgInt32> faceCountPool(triangleCount);
			dgStack<dgInt32> materialsPool(triangleCount);
			dgStack<dgInt32> vertexIndexListPool(triangleCount * 3);
			dgStack<dgInt32> normalIndexListPool(triangleCount * 3);

			memset(&materialsPool[0], 0, triangleCount * sizeof(dgInt32));
			memset(&normalIndexListPool[0], 0, 3 * triangleCount * sizeof(dgInt32));

			dgInt32 index = 0;
			dgInt32 *const faceCount = &faceCountPool[0];
			dgInt32 *const vertexIndexList = &vertexIndexListPool[0];
			for (dgConvexHull3d::dgListNode *faceNode = convexHull.GetFirst();
			        faceNode; faceNode = faceNode->GetNext()) {
				dgConvexHull3DFace &face = faceNode->GetInfo();
				faceCount[index] = 3;
				vertexIndexList[index * 3 + 0] = face.m_index[0];
				vertexIndexList[index * 3 + 1] = face.m_index[1];
				vertexIndexList[index * 3 + 2] = face.m_index[2];
				index++;
			}

			BuildFromVertexListIndexList(triangleCount, faceCount, &materialsPool[0],
			                             &points[0].m_x, sizeof(dgVector), vertexIndexList, &normal.m_x,
			                             sizeof(dgVector), &normalIndexListPool[0], &uv.m_x, sizeof(dgVector),
			                             &normalIndexListPool[0], &uv.m_x, sizeof(dgVector),
			                             &normalIndexListPool[0]);
			RepairTJoints(true);
		}
	}
}


dgMeshEffect *dgMeshEffect::CreateDelanayTretrahedralization(
    dgInt32 interionMaterial, dgMatrix &matrix) const {
	NEWTON_ASSERT(0);
	return NULL;
}

dgMeshEffect *dgMeshEffect::CreateVoronoiPartition(dgInt32 pointsCount,
        dgInt32 pointStrideInBytes, const dgFloat32 *const pointCloud,
        dgInt32 interiorMaterial, dgMatrix &textureProjectionMatrix) const {
	// return new (GetAllocator()) dgMeshEffect (*this);

	dgMeshEffectSolidTree *const tree = CreateSolidTree();
	NEWTON_ASSERT(tree);

	dgStack<dgBigVector> buffer(pointsCount);
	dgBigVector *const pool = &buffer[0];

	dgInt32 count = 0;
	dgFloat64 quantizeFactor = dgFloat64(16.0f);
	dgFloat64 invQuantizeFactor = dgFloat64(1.0f) / quantizeFactor;
	dgInt32 stride = pointStrideInBytes / sizeof(dgFloat32);
	for (dgInt32 i = 0; i < pointsCount; i++) {
		dgFloat64 x = pointCloud[i * stride + 0];
		dgFloat64 y = pointCloud[i * stride + 1];
		dgFloat64 z = pointCloud[i * stride + 2];
		x = floor(x * quantizeFactor) * invQuantizeFactor;
		y = floor(y * quantizeFactor) * invQuantizeFactor;
		z = floor(z * quantizeFactor) * invQuantizeFactor;
		dgBigVector p(x, y, z, dgFloat64(0.0f));

		if (tree->GetPointSide(p) == dgMeshEffectSolidTree::m_solid) {
			pool[count] = p;
			count++;
		}
	}

	NEWTON_ASSERT(count >= 4);
	dgStack<dgInt32> indexList(count);
	count = dgVertexListToIndexList(&pool[0].m_x, sizeof(dgBigVector), 3, count,
	                                &indexList[0], dgFloat64(1.0e-5f));
	NEWTON_ASSERT(count >= 4);

	dgDelaunayTetrahedralization delaunayTetrahedras(GetAllocator(), &pool[0].m_x,
	        count, sizeof(dgBigVector), 0.0f);
	delaunayTetrahedras.RemoveUpperHull();

	dgBigVector minBox;
	dgBigVector maxBox;
	CalculateAABB(minBox, maxBox);
	maxBox -= minBox;
	dgFloat32 bboxDiagnalFactor = 4.0f;
	dgFloat64 perimeterConvexBound = bboxDiagnalFactor * sqrt(maxBox % maxBox);

	dgInt32 tetraCount = delaunayTetrahedras.GetCount();
	dgStack<dgBigVector> voronoiPoints(tetraCount);
	dgStack<dgDelaunayTetrahedralization::dgListNode *> tetradrumNode(tetraCount);
	dgTree<dgList<dgInt32>, dgInt32> delanayNodes(GetAllocator());

	dgInt32 index = 0;
	const dgHullVector *const delanayPoints =
	    delaunayTetrahedras.GetHullVertexArray();
	for (dgDelaunayTetrahedralization::dgListNode *node =
	            delaunayTetrahedras.GetFirst();
	        node; node = node->GetNext()) {
		dgConvexHull4dTetraherum &tetra = node->GetInfo();
		dgBigVector origin(tetra.CircumSphereCenter(delanayPoints));
		voronoiPoints[index] = dgBigVector(dgFloat64(origin.m_x),
		                                   dgFloat64(origin.m_y), dgFloat64(origin.m_z), dgFloat64(0.0f));
		tetradrumNode[index] = node;

		for (dgInt32 i = 0; i < 3; i++) {
			dgTree<dgList<dgInt32>, dgInt32>::dgTreeNode *header = delanayNodes.Find(
			            tetra.m_faces[0].m_index[i]);
			if (!header) {
				dgList<dgInt32> list(GetAllocator());
				header = delanayNodes.Insert(list, tetra.m_faces[0].m_index[i]);
			}
			header->GetInfo().Append(index);
		}

		dgTree<dgList<dgInt32>, dgInt32>::dgTreeNode *header = delanayNodes.Find(
		            tetra.m_faces[0].m_otherVertex);
		if (!header) {
			dgList<dgInt32> list(GetAllocator());
			header = delanayNodes.Insert(list, tetra.m_faces[0].m_otherVertex);
		}
		header->GetInfo().Append(index);
		index++;
	}

	dgMeshEffect *const voronoiPartition = new (GetAllocator()) dgMeshEffect(
	    GetAllocator(), true);
	voronoiPartition->BeginPolygon();
	dgFloat64 layer = dgFloat64(0.0f);

	dgTree<dgList<dgInt32>, dgInt32>::Iterator iter(delanayNodes);
	for (iter.Begin(); iter; iter++) {

		dgInt32 countI = 0;
		dgBigVector pointArray[256];
		dgTree<dgList<dgInt32>, dgInt32>::dgTreeNode *const nodeNode =
		    iter.GetNode();

		dgList<dgInt32> &list = nodeNode->GetInfo();

		dgInt32 key = nodeNode->GetKey();

		for (dgList<dgInt32>::dgListNode *ptr = list.GetFirst(); ptr;
		        ptr = ptr->GetNext()) {
			dgInt32 i = ptr->GetInfo();
			dgConvexHull4dTetraherum *const tetrahedrum =
			    &tetradrumNode[i]->GetInfo();
			for (dgInt32 j = 0; j < 4; j++) {
				if (!tetrahedrum->m_faces[j].m_twin) {
					if ((tetrahedrum->m_faces[j].m_index[0] == key) || (tetrahedrum->m_faces[j].m_index[1] == key) || (tetrahedrum->m_faces[j].m_index[2] == key)) {
						dgBigVector p0(
						    delaunayTetrahedras.GetVertex(
						        tetrahedrum->m_faces[j].m_index[0]));
						dgBigVector p1(
						    delaunayTetrahedras.GetVertex(
						        tetrahedrum->m_faces[j].m_index[1]));
						dgBigVector p2(
						    delaunayTetrahedras.GetVertex(
						        tetrahedrum->m_faces[j].m_index[2]));
						dgBigVector n((p1 - p0) * (p2 - p0));
						n = n.Scale(dgFloat64(1.0f) / sqrt(n % n));
						dgBigVector normal(dgFloat64(n.m_x), dgFloat64(n.m_y),
						                   dgFloat64(n.m_z), dgFloat64(0.0f));
						pointArray[countI] = voronoiPoints[i] + normal.Scale(perimeterConvexBound);

						countI++;
						NEWTON_ASSERT(countI < dgInt32(sizeof(pointArray) / sizeof(pointArray[0])));
					}
				}
			}

			pointArray[countI] = voronoiPoints[i];
			countI++;
			NEWTON_ASSERT(countI < dgInt32(sizeof(pointArray) / sizeof(pointArray[0])));
		}

		dgMeshEffect *const convexMesh = MakeDelanayIntersection(tree,
		                                 &pointArray[0], countI, interiorMaterial, textureProjectionMatrix,
		                                 dgFloat64(45.0f * 3.1416f / 180.0f));
		if (convexMesh) {
			for (dgInt32 i = 0; i < convexMesh->m_pointCount; i++) {
				convexMesh->m_points[i].m_w = layer;
			}
			for (dgInt32 i = 0; i < convexMesh->m_atribCount; i++) {
				convexMesh->m_attib[i].m_vertex.m_w = layer;
			}

			voronoiPartition->MergeFaces(convexMesh);
			layer += dgFloat64(1.0f);

			convexMesh->Release();
		}
	}

	voronoiPartition->EndPolygon(dgFloat64(1.0e-5f));

	voronoiPartition->ConvertToPolygons();

	delete tree;
	return voronoiPartition;
}

dgMeshEffect *dgMeshEffect::MakeDelanayIntersection(
    dgMeshEffectSolidTree *const tree, dgBigVector *const points, dgInt32 count,
    dgInt32 materialId, const dgMatrix &textureProjectionMatrix,
    dgFloat32 normalAngleInRadians) const {
	for (dgInt32 i = 0; i < count; i++) {
		points[i].m_x = QuantizeCordinade(points[i].m_x);
		points[i].m_y = QuantizeCordinade(points[i].m_y);
		points[i].m_z = QuantizeCordinade(points[i].m_z);
		points[i].m_w = dgFloat64(0.0f);
	}

	dgMeshEffect *intersection = NULL;
	dgMeshEffect convexMesh(GetAllocator(), &points[0].m_x, count,
	                        sizeof(dgBigVector), dgFloat64(0.0f));

	if (convexMesh.GetCount()) {
		convexMesh.CalculateNormals(normalAngleInRadians);
		convexMesh.UniformBoxMapping(materialId, textureProjectionMatrix);

#if 0
		intersection = new (GetAllocator()) dgMeshEffect(convexMesh);
#else

		DG_MESG_EFFECT_BOOLEAN_INIT();

		ClipMesh(&convexMesh, &leftMeshSource, &rightMeshSource, &sourceCoplanar);
		convexMesh.ClipMesh(tree, &leftMeshClipper, &rightMeshClipper,
		                    &clipperCoplanar);
		if (leftMeshSource || leftMeshClipper) {
			result = new (GetAllocator()) dgMeshEffect(GetAllocator(), true);
			result->BeginPolygon();

			if (leftMeshSource) {
				result->MergeFaces(leftMeshSource);
			}

			if (leftMeshClipper) {
				result->MergeFaces(leftMeshClipper);
			}

			if (clipperCoplanar && sourceCoplanar) {
				sourceCoplanar->FilterCoplanarFaces(clipperCoplanar, dgFloat32(-1.0f));
				result->MergeFaces(sourceCoplanar);
			}

			result->EndPolygon(dgFloat64(1.0e-5f));
			if (!result->GetCount()) {
				result->Release();
				result = NULL;
			}
		}
		intersection = result;
		DG_MESG_EFFECT_BOOLEAN_FINISH()
#endif
	}

#if 0
	if (intersection) {
		dgBigVector xxx(0, 0, 0, 0);
		for (dgInt32 i = 0; i < intersection->m_pointCount; i ++) {
			xxx += intersection->m_points[i];
		}
		xxx = xxx.Scale(0.5f / intersection->m_pointCount);
		for (dgInt32 i = 0; i < intersection->m_pointCount; i ++) {
			intersection->m_points[i] += xxx;
		}
		for (dgInt32 i = 0; i < intersection->m_atribCount; i ++) {
			intersection->m_attib[i].m_vertex += xxx;
		}
	}
#endif

	return intersection;
}
