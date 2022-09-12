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
#include "dgTree.h"
#include "dgHeap.h"
#include "dgGoogol.h"
#include "dgConvexHull4d.h"
#include "dgSmallDeterminant.h"

#define DG_VERTEX_CLUMP_SIZE_4D		8

class dgAABBPointTree4d
{
public:
#ifdef _DEBUG
  dgAABBPointTree4d()
  {
    static dgInt32 id = 0;
    m_id = id;
    id++;
  }
  dgInt32 m_id;
#endif

  dgBigVector m_box[2];
  dgAABBPointTree4d* m_left;
  dgAABBPointTree4d* m_right;
  dgAABBPointTree4d* m_parent;

};

class dgAABBPointTree4dClump: public dgAABBPointTree4d
{
public:
  dgInt32 m_count;
  dgInt32 m_indices[DG_VERTEX_CLUMP_SIZE_4D];
};

dgConvexHull4dTetraherum::dgTetrahedrumPlane::dgTetrahedrumPlane(
    const dgBigVector& p0, const dgBigVector& p1, const dgBigVector& p2,
    const dgBigVector& p3) :
    dgBigVector(p1.Sub4(p0).CrossProduct4(p2.Sub4(p0), p3.Sub4(p0)))
{
  dgBigVector& me = *this;
//	_ASSERTE (me.DotProduct4(me) > dgFloat64 (1.0e-64f));
  dgFloat64 invMag2 = dgFloat32(0.0f);
//	if (me.DotProduct4(me) > dgFloat64 (1.0e-64)) {
  if (me.DotProduct4(me) > dgFloat64(1.0e-38))
  {
    invMag2 = dgFloat64(1.0f) / sqrt(me.DotProduct4(me));
  }
  else
  {
    invMag2 = dgFloat32(0.0f);
  }
  me.m_x *= invMag2;
  me.m_y *= invMag2;
  me.m_z *= invMag2;
  me.m_w *= invMag2;
  m_dist = -me.DotProduct4(p0);
}

dgFloat64 dgConvexHull4dTetraherum::dgTetrahedrumPlane::Evalue(
    const dgBigVector& point) const
{
  const dgBigVector& me = *this;
  return me.DotProduct4(point) + m_dist;
}

dgConvexHull4dTetraherum::dgConvexHull4dTetraherum()
{
#ifdef _DEBUG
  static dgInt32 debugID;
  m_debugID = debugID;
  debugID++;
#endif

}

void dgConvexHull4dTetraherum::Init(const dgHullVector* const points,
    dgInt32 v0, dgInt32 v1, dgInt32 v2, dgInt32 v3)
{
  //{0, 1, 2, 3},
  //{3, 0, 2, 1},
  //{3, 2, 1, 0},
  //{3, 1, 0, 2}

  m_faces[0].m_index[0] = v0;
  m_faces[0].m_index[1] = v1;
  m_faces[0].m_index[2] = v2;
  m_faces[0].m_otherVertex = v3;

  m_faces[1].m_index[0] = v3;
  m_faces[1].m_index[1] = v0;
  m_faces[1].m_index[2] = v2;
  m_faces[1].m_otherVertex = v1;

  m_faces[2].m_index[0] = v3;
  m_faces[2].m_index[1] = v2;
  m_faces[2].m_index[2] = v1;
  m_faces[2].m_otherVertex = v0;

  m_faces[3].m_index[0] = v3;
  m_faces[3].m_index[1] = v1;
  m_faces[3].m_index[2] = v0;
  m_faces[3].m_otherVertex = v2;

  SetMark(0);
  for (dgInt32 i = 0; i < 4; i++)
  {
    m_faces[i].m_twin = NULL;
  }

#ifdef _DEBUG
  dgBigVector p1p0(points[v1].Sub4(points[v0]));
  dgBigVector p2p0(points[v2].Sub4(points[v0]));
  dgBigVector p3p0(points[v3].Sub4(points[v0]));
  dgBigVector normal(p1p0.CrossProduct4(p2p0, p3p0));
  dgFloat64 volume = normal.DotProduct4(normal);
  _ASSERTE(volume > dgFloat64 (0.0f));
#endif
}

dgFloat64 dgConvexHull4dTetraherum::Evalue(const dgHullVector* const pointArray,
    const dgBigVector& point) const
{
  const dgBigVector &p0 = pointArray[m_faces[0].m_index[0]];
  const dgBigVector &p1 = pointArray[m_faces[0].m_index[1]];
  const dgBigVector &p2 = pointArray[m_faces[0].m_index[2]];
  const dgBigVector &p3 = pointArray[m_faces[0].m_otherVertex];

  dgFloat64 matrix[4][4];
  for (dgInt32 i = 0; i < 4; i++)
  {
    matrix[0][i] = p1[i] - p0[i];
    matrix[1][i] = p2[i] - p0[i];
    matrix[2][i] = p3[i] - p0[i];
    matrix[3][i] = point[i] - p0[i];
  }

  dgFloat64 error;
  dgFloat64 det = Determinant4x4(matrix, &error);
  dgFloat64 precision = dgFloat64(1.0f) / dgFloat64(1 << 24);
  dgFloat64 errbound = error * precision;
  if (fabs(det) > errbound)
  {
    return det;
  }

  dgGoogol exactMatrix[4][4];
  for (dgInt32 i = 0; i < 4; i++)
  {
    exactMatrix[0][i] = dgGoogol(p1[i]) - dgGoogol(p0[i]);
    exactMatrix[1][i] = dgGoogol(p2[i]) - dgGoogol(p0[i]);
    exactMatrix[2][i] = dgGoogol(p3[i]) - dgGoogol(p0[i]);
    exactMatrix[3][i] = dgGoogol(point[i]) - dgGoogol(p0[i]);
  }

  dgGoogol exactDet(Determinant4x4(exactMatrix));
  det = exactDet.GetAproximateValue();

  return det;
}

dgBigVector dgConvexHull4dTetraherum::CircumSphereCenter(
    const dgHullVector* const pointArray) const
{
  dgGoogol matrix[4][4];

  dgBigVector points[4];
  points[0] = pointArray[m_faces[0].m_index[0]];
  points[1] = pointArray[m_faces[0].m_index[1]];
  points[2] = pointArray[m_faces[0].m_index[2]];
  points[3] = pointArray[m_faces[0].m_otherVertex];

  for (dgInt32 i = 0; i < 4; i++)
  {
    for (dgInt32 j = 0; j < 3; j++)
    {
      matrix[i][j] = dgGoogol(points[i][j]);
    }
    matrix[i][3] = dgGoogol(1.0f);
  }
  dgGoogol det(Determinant4x4(matrix));
  dgFloat64 invDen = dgFloat32(1.0f)
      / (det.GetAproximateValue() * dgFloat32(2.0f));

  dgBigVector centerOut;
  dgFloat64 sign = dgFloat64(1.0f);
  for (dgInt32 k = 0; k < 3; k++)
  {
    for (dgInt32 i = 0; i < 4; i++)
    {
      matrix[i][0] = dgGoogol(points[i][3]);
      for (dgInt32 j = 0; j < 2; j++)
      {
        dgInt32 j1 = (j < k) ? j : j + 1;
        matrix[i][j + 1] = dgGoogol(points[i][j1]);
      }
      matrix[i][3] = dgGoogol(1.0f);
    }
    dgGoogol detN(Determinant4x4(matrix));
    dgFloat64 val = detN.GetAproximateValue() * sign;
    sign *= dgFloat64(-1.0f);
    centerOut[k] = val * invDen;
  }
  centerOut[3] = dgFloat32(0.0f);
//	dgBigVector radius (points[0] - centerOut);
//dgFloat32 a = (points[0] - centerOut) % (points[0] - centerOut);
//dgFloat32 b = (points[1] - centerOut) % (points[1] - centerOut);
//dgFloat32 c = (points[2] - centerOut) % (points[2] - centerOut);
//dgFloat32 d = (points[3] - centerOut) % (points[3] - centerOut);
  return centerOut;
}

dgConvexHull4dTetraherum::dgTetrahedrumPlane dgConvexHull4dTetraherum::GetPlaneEquation(
    const dgHullVector* const points) const
{
  const dgBigVector &p0 = points[m_faces[0].m_index[0]];
  const dgBigVector &p1 = points[m_faces[0].m_index[1]];
  const dgBigVector &p2 = points[m_faces[0].m_index[2]];
  const dgBigVector &p3 = points[m_faces[0].m_otherVertex];
  return dgTetrahedrumPlane(p0, p1, p2, p3);
}

dgConvexHull4d::dgConvexHull4d(dgMemoryAllocator* const allocator) :
    dgList<dgConvexHull4dTetraherum>(allocator), m_mark(0), m_count(0), m_diag(), m_points(
        1024, allocator)
{
}

dgConvexHull4d::dgConvexHull4d(dgMemoryAllocator* const allocator,
    const dgBigVector* const vertexCloud, dgInt32 count, dgFloat32 distTol) :
    dgList<dgConvexHull4dTetraherum>(allocator), m_mark(0), m_count(0), m_diag(), m_points(
        count, allocator)
{
  /*
   #ifdef _WIN32
   dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
   dgControlFP (_PC_53, _MCW_PC);
   #endif

   //	InitVertexArray(vertexCloud, count);

   dgInt32 treeCount = count / (DG_VERTEX_CLUMP_SIZE_4D>>1);
   if (treeCount < 4) {
   treeCount = 4;
   }
   treeCount *= 2;

   dgStack<dgHullVector> points (count);
   dgStack<dgHullVector> convexPoints (count);
   dgStack<dgAABBPointTree4dClump> treePool (treeCount);

   count = InitVertexArray(&convexPoints[0], &points[0], vertexCloud, count);

   if (m_count >= 4) {
   _ASSERTE (0);
   //		CalculateConvexHull (distTol);
   }

   #ifdef _WIN32
   dgControlFP (controlWorld, _MCW_PC);
   #endif
   */
  BuildHull(allocator, vertexCloud, count, distTol);
}

dgConvexHull4d::~dgConvexHull4d(void)
{
}

void dgConvexHull4d::BuildHull(dgMemoryAllocator* const allocator,
    const dgBigVector* const vertexCloud, dgInt32 count, dgFloat64 distTol)
{
#ifdef _WIN32
  dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
  dgControlFP(_PC_53, _MCW_PC);
#endif

  //	InitVertexArray(vertexCloud, count);

  dgInt32 treeCount = count / (DG_VERTEX_CLUMP_SIZE_4D >> 1);
  if (treeCount < 4)
  {
    treeCount = 4;
  }
  treeCount *= 2;

  dgStack<dgHullVector> points(count);
  dgStack<dgAABBPointTree4dClump> treePool(treeCount + 256);

  count = InitVertexArray(&points[0], vertexCloud, count, &treePool[0],
      treePool.GetSizeInBytes());
  if (m_count >= 4)
  {
    CalculateConvexHull(&treePool[0], &points[0], count, distTol);
  }

#ifdef _WIN32
  dgControlFP(controlWorld, _MCW_PC);
#endif
}

void dgConvexHull4d::TessellateTriangle(dgInt32 level, const dgVector& p0,
    const dgVector& p1, const dgVector& p2, dgInt32& count,
    dgBigVector* const ouput, dgInt32& start) const
{
  if (level)
  {
    _ASSERTE(dgAbsf (p0 % p0 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
    _ASSERTE(dgAbsf (p1 % p1 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
    _ASSERTE(dgAbsf (p2 % p2 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
    dgVector p01(p0 + p1);
    dgVector p12(p1 + p2);
    dgVector p20(p2 + p0);

    p01 = p01.Scale(dgFloat32(1.0f) / dgSqrt(p01 % p01));
    p12 = p12.Scale(dgFloat32(1.0f) / dgSqrt(p12 % p12));
    p20 = p20.Scale(dgFloat32(1.0f) / dgSqrt(p20 % p20));

    _ASSERTE(dgAbsf (p01 % p01 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
    _ASSERTE(dgAbsf (p12 % p12 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
    _ASSERTE(dgAbsf (p20 % p20 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));

    TessellateTriangle(level - 1, p0, p01, p20, count, ouput, start);
    TessellateTriangle(level - 1, p1, p12, p01, count, ouput, start);
    TessellateTriangle(level - 1, p2, p20, p12, count, ouput, start);
    TessellateTriangle(level - 1, p01, p12, p20, count, ouput, start);

  }
  else
  {
    dgBigPlane n(p0, p1, p2);
    n = n.Scale(dgFloat64(1.0f) / sqrt(n % n));
    n.m_w = dgFloat64(0.0f);
    ouput[start] = n;
    start += 8;
    count++;
  }
}

dgInt32 dgConvexHull4d::BuildNormalList(dgBigVector* const normalArray) const
{
  dgVector p0(dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  dgVector p1(-dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  dgVector p2(dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  dgVector p3(dgFloat32(0.0f), -dgFloat32(1.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  dgVector p4(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f),
      dgFloat32(0.0f));
  dgVector p5(dgFloat32(0.0f), dgFloat32(0.0f), -dgFloat32(1.0f),
      dgFloat32(0.0f));

  dgInt32 count = 0;
  dgInt32 subdivitions = 1;

  dgInt32 start = 0;
  TessellateTriangle(subdivitions, p4, p0, p2, count, normalArray, start);
  start = 1;
  TessellateTriangle(subdivitions, p5, p3, p1, count, normalArray, start);
  start = 2;
  TessellateTriangle(subdivitions, p5, p1, p2, count, normalArray, start);
  start = 3;
  TessellateTriangle(subdivitions, p4, p3, p0, count, normalArray, start);
  start = 4;
  TessellateTriangle(subdivitions, p4, p2, p1, count, normalArray, start);
  start = 5;
  TessellateTriangle(subdivitions, p5, p0, p3, count, normalArray, start);
  start = 6;
  TessellateTriangle(subdivitions, p5, p2, p0, count, normalArray, start);
  start = 7;
  TessellateTriangle(subdivitions, p4, p1, p3, count, normalArray, start);

  /*
   for (dgInt32 i = 0; i < count; i ++) {
   dgBigVector p = normalArray[i];
   p.m_w = 1.0f;
   p = p.Scale4 (1.0f / sqrt (p.DotProduct4(p)));
   normalArray[count + i * 2] = p;
   p = p.Scale4(dgFloat64 (-1.0f));
   normalArray[count + i * 2 + 1] = p;
   }
   count *= 3;
   */
  return count;
}

//dgInt32 dgConvexHull4d::SupportVertex (int count, const dgHullVector* const points, const dgBigVector& dir) const
dgInt32 dgConvexHull4d::SupportVertex(dgAABBPointTree4d** const treePointer,
    const dgHullVector* const points, const dgBigVector& dir) const
{
  /*
   dgFloat64 dist = dgFloat32 (-1.0e10f);
   dgInt32 index = -1;
   for (dgInt32 i = 0; i < count; i ++) {
   dgFloat64 dist1 = dir.DotProduct4(points[i]);
   if (dist1 > dist) {
   dist = dist1;
   index = i;
   }
   }
   _ASSERTE (index != -1);
   return index;
   */

#define DG_STACK_DEPTH_4D	64
  dgFloat64 aabbProjection[DG_STACK_DEPTH_4D];
  const dgAABBPointTree4d *stackPool[DG_STACK_DEPTH_4D];

  dgInt32 index = -1;
  dgInt32 stack = 1;
  stackPool[0] = *treePointer;
  aabbProjection[0] = dgFloat32(1.0e20f);
  dgFloat64 maxProj = dgFloat64(-1.0e20f);
  dgInt32 ix = (dir[0] > dgFloat64(0.0f)) ? 1 : 0;
  dgInt32 iy = (dir[1] > dgFloat64(0.0f)) ? 1 : 0;
  dgInt32 iz = (dir[2] > dgFloat64(0.0f)) ? 1 : 0;
  dgInt32 iw = (dir[3] > dgFloat64(0.0f)) ? 1 : 0;
  while (stack)
  {
    stack--;
    dgFloat64 boxSupportValue = aabbProjection[stack];
    if (boxSupportValue > maxProj)
    {
      const dgAABBPointTree4d* const me = stackPool[stack];

      if (me->m_left && me->m_right)
      {
        dgBigVector leftSupportPoint(me->m_left->m_box[ix].m_x,
            me->m_left->m_box[iy].m_y, me->m_left->m_box[iz].m_z,
            me->m_left->m_box[iw].m_w);
        dgFloat64 leftSupportDist = leftSupportPoint.DotProduct4(dir);

        dgBigVector rightSupportPoint(me->m_right->m_box[ix].m_x,
            me->m_right->m_box[iy].m_y, me->m_right->m_box[iz].m_z,
            me->m_right->m_box[iw].m_w);
        dgFloat64 rightSupportDist = rightSupportPoint.DotProduct4(dir);

        if (rightSupportDist >= leftSupportDist)
        {
          aabbProjection[stack] = leftSupportDist;
          stackPool[stack] = me->m_left;
          stack++;
          _ASSERTE(stack < DG_STACK_DEPTH_4D);
          aabbProjection[stack] = rightSupportDist;
          stackPool[stack] = me->m_right;
          stack++;
          _ASSERTE(stack < DG_STACK_DEPTH_4D);
        }
        else
        {
          aabbProjection[stack] = rightSupportDist;
          stackPool[stack] = me->m_right;
          stack++;
          _ASSERTE(stack < DG_STACK_DEPTH_4D);
          aabbProjection[stack] = leftSupportDist;
          stackPool[stack] = me->m_left;
          stack++;
          _ASSERTE(stack < DG_STACK_DEPTH_4D);
        }

      }
      else
      {
        dgAABBPointTree4dClump* const clump = (dgAABBPointTree4dClump*) me;
        for (dgInt32 i = 0; i < clump->m_count; i++)
        {
          const dgHullVector& p = points[clump->m_indices[i]];
          _ASSERTE(p.m_x >= clump->m_box[0].m_x);
          _ASSERTE(p.m_x <= clump->m_box[1].m_x);
          _ASSERTE(p.m_y >= clump->m_box[0].m_y);
          _ASSERTE(p.m_y <= clump->m_box[1].m_y);
          _ASSERTE(p.m_z >= clump->m_box[0].m_z);
          _ASSERTE(p.m_z <= clump->m_box[1].m_z);
          _ASSERTE(p.m_w >= clump->m_box[0].m_w);
          _ASSERTE(p.m_w <= clump->m_box[1].m_w);
          if (!p.m_mark)
          {
            dgFloat64 dist = p.DotProduct4(dir);
            if (dist > maxProj)
            {
              maxProj = dist;
              index = clump->m_indices[i];
            }
          }
          else
          {
            clump->m_indices[i] = clump->m_indices[clump->m_count - 1];
            clump->m_count = clump->m_count - 1;
            i--;
          }
        }

        if (clump->m_count == 0)
        {
          dgAABBPointTree4d* const parent = clump->m_parent;
          if (parent)
          {
            dgAABBPointTree4d* const sibling =
                (parent->m_left != clump) ? parent->m_left : parent->m_right;
            _ASSERTE(sibling != clump);
            dgAABBPointTree4d* const grandParent = parent->m_parent;
            if (grandParent)
            {
              sibling->m_parent = grandParent;
              if (grandParent->m_right == parent)
              {
                grandParent->m_right = sibling;
              }
              else
              {
                grandParent->m_left = sibling;
              }
            }
            else
            {
              sibling->m_parent = NULL;
              *treePointer = sibling;
            }
          }
        }
      }
    }
  }

  _ASSERTE(index != -1);
  return index;
}

dgInt32 dgConvexHull4d::ConvexCompareVertex(const dgHullVector* const A,
    const dgHullVector* const B, void* const context)
{
  for (dgInt32 i = 0; i < 4; i++)
  {
    if ((*A)[i] < (*B)[i])
    {
      return -1;
    }
    else if ((*A)[i] > (*B)[i])
    {
      return 1;
    }
  }
  return 0;
}

dgAABBPointTree4d* dgConvexHull4d::BuildTree(dgAABBPointTree4d* const parent,
    dgHullVector* const points, dgInt32 count, dgInt32 baseIndex,
    dgInt8** memoryPool, dgInt32& maxMemSize) const
{
  dgAABBPointTree4d* tree = NULL;

  _ASSERTE(count);
  dgBigVector minP(dgFloat32(1.0e15f), dgFloat32(1.0e15f), dgFloat32(1.0e15f),
      dgFloat32(1.0e15f));
  dgBigVector maxP(-dgFloat32(1.0e15f), -dgFloat32(1.0e15f),
      -dgFloat32(1.0e15f), -dgFloat32(1.0e15f));
  if (count <= DG_VERTEX_CLUMP_SIZE_4D)
  {

    dgAABBPointTree4dClump* const clump =
        new (*memoryPool) dgAABBPointTree4dClump;
    *memoryPool += sizeof(dgAABBPointTree4dClump);
    maxMemSize -= sizeof(dgAABBPointTree4dClump);
    _ASSERTE(maxMemSize >= 0);

    clump->m_count = count;
    for (dgInt32 i = 0; i < count; i++)
    {
      clump->m_indices[i] = i + baseIndex;

      const dgBigVector& p = points[i];
      minP.m_x = GetMin(p.m_x, minP.m_x);
      minP.m_y = GetMin(p.m_y, minP.m_y);
      minP.m_z = GetMin(p.m_z, minP.m_z);
      minP.m_w = GetMin(p.m_w, minP.m_w);

      maxP.m_x = GetMax(p.m_x, maxP.m_x);
      maxP.m_y = GetMax(p.m_y, maxP.m_y);
      maxP.m_z = GetMax(p.m_z, maxP.m_z);
      maxP.m_w = GetMax(p.m_w, maxP.m_w);
    }

    clump->m_left = NULL;
    clump->m_right = NULL;
    tree = clump;

  }
  else
  {
    dgBigVector median(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(0.0f));
    dgBigVector varian(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(0.0f));
    for (dgInt32 i = 0; i < count; i++)
    {

      const dgBigVector& p = points[i];
      minP.m_x = GetMin(p.m_x, minP.m_x);
      minP.m_y = GetMin(p.m_y, minP.m_y);
      minP.m_z = GetMin(p.m_z, minP.m_z);
      minP.m_w = GetMin(p.m_w, minP.m_w);

      maxP.m_x = GetMax(p.m_x, maxP.m_x);
      maxP.m_y = GetMax(p.m_y, maxP.m_y);
      maxP.m_z = GetMax(p.m_z, maxP.m_z);
      maxP.m_w = GetMax(p.m_w, maxP.m_w);

      median = median.Add4(p);
      varian = varian.Add4(p.CompProduct4(p));
    }

    varian = varian.Scale4(dgFloat32(count)).Sub4(median.CompProduct4(median));
    dgInt32 index = 0;
    dgFloat64 maxVarian = dgFloat64(-1.0e10f);
    for (dgInt32 i = 0; i < 4; i++)
    {
      if (varian[i] > maxVarian)
      {
        index = i;
        maxVarian = varian[i];
      }
    }
    dgBigVector center = median.Scale4(dgFloat64(1.0f) / dgFloat64(count));

    dgFloat64 test = center[index];

    dgInt32 i0 = 0;
    dgInt32 i1 = count - 1;
    do
    {
      for (; i0 <= i1; i0++)
      {
        dgFloat64 val = points[i0][index];
        if (val > test)
        {
          break;
        }
      }

      for (; i1 >= i0; i1--)
      {
        dgFloat64 val = points[i1][index];
        if (val < test)
        {
          break;
        }
      }

      if (i0 < i1)
      {
        Swap(points[i0], points[i1]);
        i0++;
        i1--;
      }
    } while (i0 <= i1);

    if (i0 == 0)
    {
      i0 = count / 2;
    }
    if (i0 == (count - 1))
    {
      i0 = count / 2;
    }

    tree = new (*memoryPool) dgAABBPointTree4d;
    *memoryPool += sizeof(dgAABBPointTree4d);
    maxMemSize -= sizeof(dgAABBPointTree4d);
    _ASSERTE(maxMemSize >= 0);

    _ASSERTE(i0);
    _ASSERTE(count - i0);

    tree->m_left = BuildTree(tree, points, i0, baseIndex, memoryPool,
        maxMemSize);
    tree->m_right = BuildTree(tree, &points[i0], count - i0, i0 + baseIndex,
        memoryPool, maxMemSize);
  }

  _ASSERTE(tree);
  tree->m_parent = parent;
  tree->m_box[0] = minP
      - dgBigVector(dgFloat64(1.0e-3f), dgFloat64(1.0e-3f), dgFloat64(1.0e-3f),
          dgFloat64(1.0e-3f));
  tree->m_box[1] = maxP
      + dgBigVector(dgFloat64(1.0e-3f), dgFloat64(1.0e-3f), dgFloat64(1.0e-3f),
          dgFloat64(1.0e-3f));
  return tree;
}

dgInt32 dgConvexHull4d::InitVertexArray(dgHullVector* const points,
    const dgBigVector* const vertexCloud, dgInt32 count, void* const memoryPool,
    dgInt32 maxMemSize)
{
  for (dgInt32 i = 0; i < count; i++)
  {
    points[i] = vertexCloud[i];
    points[i].m_index = i;
    points[i].m_mark = 0;
  }

  dgSort(points, count, ConvexCompareVertex);
  dgInt32 indexCount = 0;
  for (int i = 1; i < count; i++)
  {
    for (; i < count; i++)
    {
      if (ConvexCompareVertex(&points[indexCount], &points[i], NULL))
      {
        indexCount++;
        points[indexCount] = points[i];
        break;
      }
    }
  }
  count = indexCount + 1;
  if (count < 4)
  {
    m_count = 0;
    return count;
  }

  dgAABBPointTree4d* tree = BuildTree(NULL, points, count, 0,
      (dgInt8**) &memoryPool, maxMemSize);

  dgBigVector boxSize(tree->m_box[1].Sub4(tree->m_box[0]));
  m_diag = dgFloat32(sqrt(boxSize.DotProduct4(boxSize)));

  m_points[4].m_x = dgFloat64(0.0f);
  dgHullVector* const convexPoints = &m_points[0];
  dgStack<dgBigVector> normalArrayPool(256);
  dgBigVector* const normalArray = &normalArrayPool[0];
  dgInt32 normalCount = BuildNormalList(&normalArray[0]);

  dgInt32 index = SupportVertex(&tree, points, normalArray[0]);
  convexPoints[0] = points[index];
  points[index].m_mark = 1;

  bool validTetrahedrum = false;
  dgBigVector e1(dgFloat64(0.0f), dgFloat64(0.0f), dgFloat64(0.0f),
      dgFloat64(0.0f));
  for (dgInt32 i = 1; i < normalCount; i++)
  {
    dgInt32 indexSV = SupportVertex(&tree, points, normalArray[i]);
    _ASSERTE(indexSV >= 0);
    e1 = points[indexSV].Sub4(convexPoints[0]);
    e1.m_w = dgFloat64(0.0f);
    dgFloat64 error2 = e1.DotProduct4(e1);
    if (error2 > (dgFloat32(1.0e-4f) * m_diag * m_diag))
    {
      convexPoints[1] = points[indexSV];
      points[indexSV].m_mark = 1;
      validTetrahedrum = true;
      break;
    }
  }
  if (!validTetrahedrum)
  {
    m_count = 0;
    return count;
  }

  dgInt32 bestIndex = -1;
  dgFloat64 bestValue = dgFloat64(1.0f);
  validTetrahedrum = false;
  dgFloat64 lenght2 = e1.DotProduct4(e1);
  dgBigVector e2(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  ;
  for (dgInt32 i = 2; i < normalCount; i++)
  {
    dgInt32 indexSV = SupportVertex(&tree, points, normalArray[i]);
    _ASSERTE(indexSV >= 0);
    _ASSERTE(indexSV < count);
    e2 = points[indexSV].Sub4(convexPoints[0]);
    e2.m_w = dgFloat64(0.0f);
    dgFloat64 den = e2.DotProduct4(e2);
    if (fabs(den) > (dgFloat64(1.0e-6f) * m_diag))
    {
      den = sqrt(lenght2 * den);
      dgFloat64 num = e2.DotProduct4(e1);
      dgFloat64 cosAngle = fabs(num / den);
      if (cosAngle < bestValue)
      {
        bestValue = cosAngle;
        bestIndex = indexSV;
      }

      if (cosAngle < 0.9f)
      {
//				convexPoints[2] = points[indexSV];
//				points[indexSV].m_mark = 1;
//				validTetrahedrum = true;
        break;
      }
    }
  }

  if (bestValue < dgFloat64(0.999f))
  {
    convexPoints[2] = points[bestIndex];
    points[bestIndex].m_mark = 1;
    validTetrahedrum = true;
  }

  if (!validTetrahedrum)
  {
    m_count = 0;
    return count;
  }

  validTetrahedrum = false;
  dgBigVector e3(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
      dgFloat32(0.0f));
  ;
  for (dgInt32 i = 3; i < normalCount; i++)
  {
    dgInt32 indexSV = SupportVertex(&tree, points, normalArray[i]);
    _ASSERTE(indexSV >= 0);
    _ASSERTE(indexSV < count);

    e3 = points[indexSV].Sub4(convexPoints[0]);
    e3.m_w = dgFloat64(0.0f);
    dgFloat64 volume = (e1 * e2) % e3;
    if (fabs(volume) > (dgFloat64(1.0e-4f) * m_diag * m_diag * m_diag))
    {
      convexPoints[3] = points[indexSV];
      points[indexSV].m_mark = 1;
      validTetrahedrum = true;
      break;
    }
  }

  m_count = 4;
  if (!validTetrahedrum)
  {
    m_count = 0;
  }

  return count;
}

dgConvexHull4d::dgListNode* dgConvexHull4d::AddFace(dgInt32 i0, dgInt32 i1,
    dgInt32 i2, dgInt32 i3)
{
  dgListNode* const node = Append();
  dgConvexHull4dTetraherum& face = node->GetInfo();
  face.Init(&m_points[0], i0, i1, i2, i3);
  return node;
}

void dgConvexHull4d::DeleteFace(dgListNode* const node)
{
  Remove(node);
}

bool dgConvexHull4d::Sanity() const
{
  for (dgListNode* node = GetFirst(); node; node = node->GetNext())
  {
    dgConvexHull4dTetraherum* const tetra = &node->GetInfo();

    for (dgInt32 i = 0; i < 4; i++)
    {
      dgConvexHull4dTetraherum::dgTetrahedrumFace* const face =
          &tetra->m_faces[i];
      dgListNode* const twinNode = face->m_twin;
      if (!twinNode)
      {
        return false;
      }
    }
  }

  /*
   dgList<dgListNode*> tetraList(GetAllocator());
   const dgHullVector* const points = &m_points[0];
   for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
   dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
   const dgBigVector &p0 = points[tetra->m_faces[0].m_index[0]];
   const dgBigVector &p1 = points[tetra->m_faces[0].m_index[1]];
   const dgBigVector &p2 = points[tetra->m_faces[0].m_index[2]];
   const dgBigVector &p3 = points[tetra->m_faces[0].m_otherVertex];

   dgBigVector p1p0 (p1.Sub4(p0));
   dgBigVector p2p0 (p2.Sub4(p0));
   dgBigVector p3p0 (p3.Sub4(p0));
   dgBigVector normal (p1p0.CrossProduct4 (p2p0, p3p0));

   if (normal.m_w < dgFloat64 (0.0f)) {
   tetraList.Append(node);
   }
   }

   for (dgList<dgListNode*>::dgListNode* node0 = tetraList.GetFirst(); node0; node0 = node0->GetNext()) {
   dgListNode* const tetraNode0 = node0->GetInfo();
   dgConvexHull4dTetraherum* const tetra0 = &tetraNode0->GetInfo();

   dgInt32 index0[4];
   index0[0] = tetra0->m_faces[0].m_index[0];
   index0[1] = tetra0->m_faces[0].m_index[1];
   index0[2] = tetra0->m_faces[0].m_index[2];
   index0[3] = tetra0->m_faces[0].m_otherVertex;

   const dgBigVector &p0 = points[index0[0]];
   const dgBigVector &p1 = points[index0[1]];
   const dgBigVector &p2 = points[index0[2]];
   const dgBigVector &p3 = points[index0[3]];
   for (dgList<dgListNode*>::dgListNode* node1 = node0->GetNext(); node1; node1 = node1->GetNext()) {
   dgListNode* const tetraNode1 = node1->GetInfo();
   dgConvexHull4dTetraherum* const tetra1 = &tetraNode1->GetInfo();

   dgInt32 index1[4];
   index1[0] = tetra1->m_faces[0].m_index[0];
   index1[1] = tetra1->m_faces[0].m_index[1];
   index1[2] = tetra1->m_faces[0].m_index[2];
   index1[3] = tetra1->m_faces[0].m_otherVertex;

   for (dgInt32 i = 0; i < 4; i ++) {
   dgInt32 count = 0;
   dgInt32 k = index1[i];
   for (dgInt32 j = 0; j < 4; j ++) {
   count += (k == index0[j]);
   }
   if (!count){
   //					const dgBigVector &p = points[k];
   //					dgFloat64 size = -insphere(&p0.m_x, &p1.m_x, &p2.m_x, &p3.m_x, &p.m_x);
   //					if (size < dgFloat64 (0.0f)) {
   //						return false;
   //					}
   }
   }
   }
   }
   */

  //dgTrace (("\n"));
  return true;
}

void dgConvexHull4d::LinkSibling(dgListNode* node0, dgListNode* node1) const
{
  dgConvexHull4dTetraherum* const tetra0 = &node0->GetInfo();
  dgConvexHull4dTetraherum* const tetra1 = &node1->GetInfo();
  for (int i = 0; i < 4; i++)
  {
    dgConvexHull4dTetraherum::dgTetrahedrumFace* const face0 =
        &tetra0->m_faces[i];
    if (!face0->m_twin)
    {
      dgInt32 i0 = face0->m_index[0];
      dgInt32 i1 = face0->m_index[1];
      dgInt32 i2 = face0->m_index[2];
      for (int j = 0; j < 4; j++)
      {
        dgConvexHull4dTetraherum::dgTetrahedrumFace* const face1 =
            &tetra1->m_faces[j];
        if (!face1->m_twin)
        {
          dgInt32 j2 = face1->m_index[0];
          dgInt32 j1 = face1->m_index[1];
          dgInt32 j0 = face1->m_index[2];

          if (((i0 == j0) && (i1 == j1) && (i2 == j2))
              || ((i1 == j0) && (i2 == j1) && (i0 == j2))
              || ((i2 == j0) && (i0 == j1) && (i1 == j2)))
          {
            face0->m_twin = node1;
            face1->m_twin = node0;
            return;
          }
        }
      }
    }
  }
}

void dgConvexHull4d::InsertNewVertex(dgInt32 vertexIndex,
    dgListNode* const frontFace, dgList<dgListNode*>& deletedFaces,
    dgList<dgListNode*>& newFaces)
{
  _ASSERTE(Sanity());
  dgList<dgListNode*> stack(GetAllocator());

  dgInt32 mark = IncMark();
  stack.Append(frontFace);
  dgHullVector* const hullVertexArray = &m_points[0];
  const dgBigVector& p = hullVertexArray[vertexIndex];
  while (stack.GetCount())
  {
    dgList<dgListNode*>::dgListNode* const stackNode = stack.GetLast();
    dgListNode* const node = stackNode->GetInfo();
    stack.Remove(stackNode);
    dgConvexHull4dTetraherum* const face = &node->GetInfo();
    if ((face->GetMark() != mark)
        && (face->Evalue(hullVertexArray, p) > dgFloat64(0.0f)))
    {
#ifdef _DEBUG
      for (dgList<dgListNode*>::dgListNode* deleteNode =
          deletedFaces.GetFirst(); deleteNode; deleteNode =
          deleteNode->GetNext())
      {
        _ASSERTE(deleteNode->GetInfo() != node);
      }
#endif
      deletedFaces.Append(node);

      face->SetMark(mark);
      for (dgInt32 i = 0; i < 4; i++)
      {
        dgListNode* const twinNode = (dgListNode*) face->m_faces[i].m_twin;
        _ASSERTE(twinNode);
        dgConvexHull4dTetraherum* const twinFace = &twinNode->GetInfo();

        if (twinFace->GetMark() != mark)
        {
          stack.Append(twinNode);
        }
      }
    }
  }

  dgTree<dgListNode*, dgListNode*> perimeter(GetAllocator());
  for (dgList<dgListNode*>::dgListNode* deleteNode = deletedFaces.GetFirst();
      deleteNode; deleteNode = deleteNode->GetNext())
  {
    dgListNode* const deleteTetraNode = deleteNode->GetInfo();
    dgConvexHull4dTetraherum* const deletedTetra = &deleteTetraNode->GetInfo();
    _ASSERTE(deletedTetra->GetMark() == mark);
    for (dgInt32 i = 0; i < 4; i++)
    {
      dgListNode* const twinNode = deletedTetra->m_faces[i].m_twin;
      dgConvexHull4dTetraherum* const twinTetra = &twinNode->GetInfo();

      if (twinTetra->GetMark() != mark)
      {
        if (!perimeter.Find(twinNode))
        {
          perimeter.Insert(twinNode, twinNode);
        }
      }
      deletedTetra->m_faces[i].m_twin = NULL;
    }
  }

  dgList<dgListNode*> coneList(GetAllocator());
  dgTree<dgListNode*, dgListNode*>::Iterator iter(perimeter);
  for (iter.Begin(); iter; iter++)
  {
    dgListNode* const perimeterNode = iter.GetNode()->GetInfo();
    dgConvexHull4dTetraherum* const perimeterTetra = &perimeterNode->GetInfo();
    for (dgInt32 i = 0; i < 4; i++)
    {
      dgConvexHull4dTetraherum::dgTetrahedrumFace* const perimeterFace =
          &perimeterTetra->m_faces[i];

      if (perimeterFace->m_twin->GetInfo().GetMark() == mark)
      {
        dgListNode* const newNode = AddFace(vertexIndex,
            perimeterFace->m_index[0], perimeterFace->m_index[1],
            perimeterFace->m_index[2]);
        newFaces.Addtop(newNode);

        dgConvexHull4dTetraherum* const newTetra = &newNode->GetInfo();
        newTetra->m_faces[2].m_twin = perimeterNode;
        perimeterFace->m_twin = newNode;
        coneList.Append(newNode);
      }
    }
  }

  for (dgList<dgListNode*>::dgListNode* coneNode = coneList.GetFirst();
      coneNode->GetNext(); coneNode = coneNode->GetNext())
  {
    dgListNode* const coneNodeA = coneNode->GetInfo();
    for (dgList<dgListNode*>::dgListNode* nextConeNode = coneNode->GetNext();
        nextConeNode; nextConeNode = nextConeNode->GetNext())
    {
      dgListNode* const coneNodeB = nextConeNode->GetInfo();
      LinkSibling(coneNodeA, coneNodeB);
    }
  }
}

void dgConvexHull4d::CalculateConvexHull(dgAABBPointTree4d* vertexTree,
    dgHullVector* const points, dgInt32 count, dgFloat64 distTol)
{
  distTol = fabs(distTol) * m_diag;
  dgListNode* const nodes0 = AddFace(0, 1, 2, 3);
  dgListNode* const nodes1 = AddFace(0, 1, 3, 2);
  //	nodes[2] = AddFace (3, 2, 0, 4);
  //	nodes[3] = AddFace (3, 1, 2, 4);
  //	nodes[4] = AddFace (3, 0, 1, 4);

  dgList<dgListNode*> boundaryFaces(GetAllocator());
  boundaryFaces.Append(nodes0);
  boundaryFaces.Append(nodes1);

  LinkSibling(nodes0, nodes1);
  LinkSibling(nodes0, nodes1);
  LinkSibling(nodes0, nodes1);
  LinkSibling(nodes0, nodes1);

  IncMark();
  count -= 4;
  dgInt32 currentIndex = 4;
  while (boundaryFaces.GetCount() && count)
  {

    dgHullVector* const hullVertexArray = &m_points[0];
    dgListNode* const faceNode = boundaryFaces.GetFirst()->GetInfo();
    dgConvexHull4dTetraherum* const face = &faceNode->GetInfo();
    dgConvexHull4dTetraherum::dgTetrahedrumPlane planeEquation(
        face->GetPlaneEquation(hullVertexArray));

    //dgInt32 index = SupportVertex (count, &hullVertexArray[currentIndex], planeEquation) + currentIndex;
    dgInt32 index = SupportVertex(&vertexTree, points, planeEquation);

    const dgHullVector& p = points[index];
    dgFloat64 dist = planeEquation.Evalue(p);

    if ((dist > distTol)
        && (face->Evalue(hullVertexArray, p) > dgFloat64(0.0f)))
    {

      m_points[currentIndex] = p;
      points[index].m_mark = 1;

      dgList<dgListNode*> deleteList(GetAllocator());
      InsertNewVertex(currentIndex, faceNode, deleteList, boundaryFaces);

      for (dgList<dgListNode*>::dgListNode* deleteNode = deleteList.GetFirst();
          deleteNode; deleteNode = deleteNode->GetNext())
      {
        dgListNode* const node = deleteNode->GetInfo();
        boundaryFaces.Remove(node);
        DeleteFace(node);
      }

      currentIndex++;
      count--;
    }
    else
    {
      boundaryFaces.Remove(faceNode);
    }
  }
  m_count = currentIndex;
}

dgConvexHull4d::dgListNode* dgConvexHull4d::FindFacingNode(
    const dgBigVector& vertex)
{
  const dgHullVector* const hullVertexArray = &m_points[0];

  dgListNode* bestNode = GetFirst();
  dgConvexHull4dTetraherum* const tetra = &bestNode->GetInfo();
  dgConvexHull4dTetraherum::dgTetrahedrumPlane plane(
      tetra->GetPlaneEquation(hullVertexArray));
  dgFloat64 dist = plane.Evalue(vertex);
  dgInt32 mark = IncMark();
  tetra->SetMark(mark);

  dgInt8 buffer[1024 * 2 * sizeof(dgFloat64)];
  dgDownHeap<dgListNode*, dgFloat64> heap(buffer, sizeof(buffer));

  heap.Push(bestNode, dist);
  dgInt32 maxCount = heap.GetMaxCount() - 1;
  dgInt32 releafCount = maxCount >> 3;
  while (heap.GetCount())
  {
    dgListNode* const node = heap[0];
    dgFloat64 distN = heap.Value();
    if (distN > dgFloat64(1.0e-5f))
    {
      return node;
    }
    heap.Pop();
    dgConvexHull4dTetraherum* const tetraN = &node->GetInfo();
    for (dgInt32 i = 0; i < 4; i++)
    {
      dgListNode* neigborghNode = tetraN->m_faces[i].m_twin;
      dgConvexHull4dTetraherum* const neighborgh = &neigborghNode->GetInfo();
      if (neighborgh->GetMark() != mark)
      {
        neighborgh->SetMark(mark);
        if (heap.GetCount() >= maxCount)
        {
          for (dgInt32 j = 0; j < releafCount; j++)
          {
            heap.Remove(heap.GetCount() - 1);
          }
        }
        dgConvexHull4dTetraherum::dgTetrahedrumPlane planeN(
            neighborgh->GetPlaneEquation(hullVertexArray));
        heap.Push(neigborghNode, planeN.Evalue(vertex));
      }
    }
  }

  for (dgListNode* node = GetFirst(); node; node = node->GetNext())
  {
    dgConvexHull4dTetraherum* const tetraN = &node->GetInfo();
    dgFloat64 distN = tetraN->Evalue(hullVertexArray, vertex);
    if (distN > dgFloat64(0.0f))
    {
      return node;
    }
  }

  return NULL;
}

dgInt32 dgConvexHull4d::AddVertex(const dgBigVector& vertex)
{
#ifdef _WIN32
  dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
  dgControlFP(_PC_53, _MCW_PC);
#endif

  dgInt32 index = -1;
  dgListNode* const faceNode = FindFacingNode(vertex);
  if (faceNode)
  {
    index = m_count;
    m_points[index] = vertex;
    m_points[index].m_index = index;
    m_count++;

    dgList<dgListNode*> newFaces(GetAllocator());
    dgList<dgListNode*> deleteList(GetAllocator());

    InsertNewVertex(index, faceNode, deleteList, newFaces);
    for (dgList<dgListNode*>::dgListNode* deleteNode = deleteList.GetFirst();
        deleteNode; deleteNode = deleteNode->GetNext())
    {
      dgListNode* const node = deleteNode->GetInfo();
      DeleteFace(node);
    }
  }

#ifdef _WIN32
  dgControlFP(controlWorld, _MCW_PC);
#endif

  return index;
}

