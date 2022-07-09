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
#include "dgStack.h"
#include "dgGoogol.h"
#include "dgSmallDeterminant.h"
#include "dgDelaunayTetrahedralization.h"

dgDelaunayTetrahedralization::dgDelaunayTetrahedralization(
    dgMemoryAllocator* const allocator, const dgFloat64* const vertexCloud,
    dgInt32 count, dgInt32 strideInByte, dgFloat64 distTol) :
    dgConvexHull4d(allocator)
{
#ifdef _WIN32
  dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
  dgControlFP(_PC_53, _MCW_PC);
#endif

  dgStack<dgBigVector> pool(count);

  dgBigVector* const points = &pool[0];
  dgInt32 stride = dgInt32(strideInByte / sizeof(dgFloat64));
  for (dgInt32 i = 0; i < count; i++)
  {
    volatile float x = float(vertexCloud[i * stride + 0]);
    volatile float y = float(vertexCloud[i * stride + 1]);
    volatile float z = float(vertexCloud[i * stride + 2]);
    points[i] = dgBigVector(x, y, z, x * x + y * y + z * z);
  }

  dgInt32 oldCount = count;
  BuildHull(allocator, &pool[0], count, distTol);
#if 1
//	if ((oldCount > m_count) && (m_count >= 4)) {
  if (oldCount > m_count)
  {
    // this is probably a regular convex solid, which will have a zero volume hull
    // add the rest of the points by incremental insertion with small perturbation
    dgInt32 hullCount = m_count;

    for (dgInt32 i = 0; i < count; i++)
    {
      bool inHull = false;
      const dgHullVector* const hullPoints = &m_points[0];
      for (dgInt32 j = 0; j < hullCount; j++)
      {
        if (hullPoints[j].m_index == i)
        {
          inHull = true;
          break;
        }
      }
      if (!inHull)
      {
        dgBigVector q(points[i]);
        dgInt32 index = AddVertex(q);
        if (index == -1)
        {
          q.m_x += dgFloat64(1.0e-3f);
          q.m_y += dgFloat64(1.0e-3f);
          q.m_z += dgFloat64(1.0e-3f);
          index = AddVertex(q);
          _ASSERTE(index != -1);
        }_ASSERTE(index != -1);
//				m_points[index] = points[i];
        m_points[index].m_index = i;
      }
    }
  }
#else
  if (oldCount > m_count)
  {
    // this is probably a regular convex solid, which will have a zero volume hull
    // perturbate a point and try again
    dgBigVector p (points[0]);
    points[0].m_x += dgFloat64 (1.0e-0f);
    points[0].m_y += dgFloat64 (1.0e-0f);
    points[0].m_z += dgFloat64 (1.0e-0f);
    points[0].m_w = points[0].m_x * points[0].m_x + points[0].m_y * points[0].m_y + points[0].m_z * points[0].m_z;
    BuildHull (allocator, &pool[0], oldCount, distTol);
    _ASSERTE (oldCount == m_count);
    // restore the old point
    //points[0].m_w = points[0].m_x * points[0].m_x + points[0].m_y * points[0].m_y + points[0].m_z * points[0].m_z;
  }
#endif

#ifdef _DEBUG
  SortVertexArray();
#endif

#ifdef _WIN32
  dgControlFP(controlWorld, _MCW_PC);
#endif
}

dgDelaunayTetrahedralization::~dgDelaunayTetrahedralization()
{
}

dgInt32 dgDelaunayTetrahedralization::AddVertex(const dgBigVector& vertex)
{
#ifdef _WIN32
  dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
  dgControlFP(_PC_53, _MCW_PC);
#endif

  dgBigVector p(vertex);
  p.m_w = p % p;
  dgInt32 index = dgConvexHull4d::AddVertex(p);

#ifdef _WIN32
  dgControlFP(controlWorld, _MCW_PC);
#endif
  return index;
}

#ifdef _DEBUG
dgInt32 dgDelaunayTetrahedralization::CompareVertexByIndex(
    const dgHullVector* const A, const dgHullVector* const B,
    void* const context)
{
  if (A->m_index < B->m_index)
  {
    return -1;
  }
  else if (A->m_index > B->m_index)
  {
    return 1;
  }
  return 0;
}

void dgDelaunayTetrahedralization::SortVertexArray()
{
  dgHullVector* const points = &m_points[0];
  for (dgListNode* node = GetFirst(); node; node = node->GetNext())
  {
    dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
    for (dgInt32 i = 0; i < 4; i++)
    {
      dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
      dgInt32 index = face.m_otherVertex;
      face.m_otherVertex = points[index].m_index;
      for (dgInt32 j = 0; j < 3; j++)
      {
        dgInt32 index = face.m_index[j];
        face.m_index[j] = points[index].m_index;
      }
    }
  }

  dgSort(points, m_count, CompareVertexByIndex);
}

#endif

void dgDelaunayTetrahedralization::RemoveUpperHull()
{
#ifdef _WIN32
  dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
  dgControlFP(_PC_53, _MCW_PC);
#endif

  dgListNode* nextNode = NULL;
//	const dgHullVector* const points = &m_points[0];
  for (dgListNode* node = GetFirst(); node; node = nextNode)
  {
    nextNode = node->GetNext();

    dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
    tetra->SetMark(0);

//		const dgBigVector &p0 = points[tetra->m_faces[0].m_index[0]];
//		const dgBigVector &p1 = points[tetra->m_faces[0].m_index[1]];
//		const dgBigVector &p2 = points[tetra->m_faces[0].m_index[2]];
//		const dgBigVector &p3 = points[tetra->m_faces[0].m_otherVertex];
//		dgFloat64 w = GetTetraVolume (p0, p1, p2, p3);
    dgFloat64 w = GetTetraVolume(tetra);
    if (w >= dgFloat64(0.0f))
    {
      DeleteFace(node);
    }
  }

#ifdef _WIN32
  dgControlFP(controlWorld, _MCW_PC);
#endif
}

void dgDelaunayTetrahedralization::DeleteFace(dgListNode* const node)
{
  dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
  for (dgInt32 i = 0; i < 4; i++)
  {
    dgListNode* const twinNode = tetra->m_faces[i].m_twin;
    if (twinNode)
    {
      dgConvexHull4dTetraherum* const twinTetra = &twinNode->GetInfo();
      for (dgInt32 j = 0; j < 4; j++)
      {
        if (twinTetra->m_faces[j].m_twin == node)
        {
          twinTetra->m_faces[j].m_twin = NULL;
          break;
        }
      }
    }
  }
  dgConvexHull4d::DeleteFace(node);
}

//dgFloat64 dgDelaunayTetrahedralization::GetTetraVolume (const dgBigVector& p0, const dgBigVector& p1, const dgBigVector& p2, const dgBigVector& p3) const
dgFloat64 dgDelaunayTetrahedralization::GetTetraVolume(
    const dgConvexHull4dTetraherum* const tetra) const
{
  //	dgBigVector p1p0 (p1.Sub4(p0));
  //	dgBigVector p2p0 (p2.Sub4(p0));
  //	dgBigVector p3p0 (p3.Sub4(p0));
  //	dgBigVector normal (p1p0.CrossProduct4 (p2p0, p3p0));
  //  dgFloat64 det = normal.m_w;

  const dgHullVector* const points = &m_points[0];
  const dgBigVector &p0 = points[tetra->m_faces[0].m_index[0]];
  const dgBigVector &p1 = points[tetra->m_faces[0].m_index[1]];
  const dgBigVector &p2 = points[tetra->m_faces[0].m_index[2]];
  const dgBigVector &p3 = points[tetra->m_faces[0].m_otherVertex];

  dgFloat64 matrix[3][3];
  for (dgInt32 i = 0; i < 3; i++)
  {
    matrix[0][i] = p2[i] - p0[i];
    matrix[1][i] = p1[i] - p0[i];
    matrix[2][i] = p3[i] - p0[i];
  }

  dgFloat64 error;
  dgFloat64 det = Determinant3x3(matrix, &error);

  dgFloat64 precision = dgFloat64(1.0f) / dgFloat64(1 << 24);
  dgFloat64 errbound = error * precision;
  if (fabs(det) > errbound)
  {
    return det;
  }

  dgGoogol exactMatrix[3][3];
  for (dgInt32 i = 0; i < 3; i++)
  {
    exactMatrix[0][i] = dgGoogol(p2[i]) - dgGoogol(p0[i]);
    exactMatrix[1][i] = dgGoogol(p1[i]) - dgGoogol(p0[i]);
    exactMatrix[2][i] = dgGoogol(p3[i]) - dgGoogol(p0[i]);
  }

  dgGoogol exactDet(Determinant3x3(exactMatrix));
  det = exactDet.GetAproximateValue();
  return det;
}

