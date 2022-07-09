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

/****************************************************************************
 *
 *  Visual C++ 6.0 created by: Julio Jerez
 *
 ****************************************************************************/
#include "dgStdafx.h"
#include "dgStack.h"
#include "dgMatrix.h"
#include "dgMemory.h"
#include "dgPolyhedra.h"
#include "dgPolygonSoupBuilder.h"

#define DG_POINTS_RUN (512 * 1024)

//class dgPolySoupFilterAllocator: public dgMemoryAllocator
class dgPolySoupFilterAllocator: public dgPolyhedra
{
public:
  dgPolySoupFilterAllocator(dgMemoryAllocator* const allocator) :
      dgPolyhedra(allocator)
  {
  }

  ~dgPolySoupFilterAllocator()
  {
  }

  dgInt32 AddFilterFace(dgUnsigned32 count, dgInt32* const pool)
  {
    BeginFace();
    _ASSERTE(count);
    bool reduction = true;
    while (reduction && !AddFace(dgInt32(count), pool))
    {
      reduction = false;
      if (count > 3)
      {
        for (dgUnsigned32 i = 0; i < count; i++)
        {
          for (dgUnsigned32 j = i + 1; j < count; j++)
          {
            if (pool[j] == pool[i])
            {
              for (i = j; i < count - 1; i++)
              {
                pool[i] = pool[i + 1];
              }
              count--;
              i = count;
              reduction = true;
              break;
            }
          }
        }
      }
    }
    EndFace();

    _ASSERTE(reduction);
    return reduction ? dgInt32(count) : 0;
  }
};

dgPolygonSoupDatabaseBuilder::dgPolygonSoupDatabaseBuilder(
    dgMemoryAllocator* const allocator) :
    m_faceVertexCount(allocator), m_vertexIndex(allocator), m_normalIndex(
        allocator), m_vertexPoints(allocator), m_normalPoints(allocator)
{
  m_run = DG_POINTS_RUN;
  m_faceCount = 0;
  m_indexCount = 0;
  m_vertexCount = 0;
  m_normalCount = 0;
  m_allocator = allocator;
}

dgPolygonSoupDatabaseBuilder::~dgPolygonSoupDatabaseBuilder()
{
}

void dgPolygonSoupDatabaseBuilder::Begin()
{
  m_run = DG_POINTS_RUN;
  m_faceCount = 0;
  m_indexCount = 0;
  m_vertexCount = 0;
  m_normalCount = 0;
}

void dgPolygonSoupDatabaseBuilder::AddMesh(const dgFloat32* const vertex,
    dgInt32 vertexCount, dgInt32 strideInBytes, dgInt32 faceCount,
    const dgInt32* const faceArray, const dgInt32* const indexArray,
    const dgInt32* const faceTagsData, const dgMatrix& worldMatrix)
{
  dgInt32 faces[256];
  dgInt32 pool[2048];

  m_vertexPoints[m_vertexCount + vertexCount].m_x = dgFloat64(0.0f);
  dgBigVector* const vertexPool = &m_vertexPoints[m_vertexCount];

  worldMatrix.TransformTriplex(&vertexPool[0].m_x, sizeof(dgBigVector), vertex,
      strideInBytes, vertexCount);
  for (dgInt32 i = 0; i < vertexCount; i++)
  {
    vertexPool[i].m_w = dgFloat64(0.0f);
  }

  dgInt32 totalIndexCount = faceCount;
  for (dgInt32 i = 0; i < faceCount; i++)
  {
    totalIndexCount += faceArray[i];
  }

  m_vertexIndex[m_indexCount + totalIndexCount] = 0;
  m_faceVertexCount[m_faceCount + faceCount] = 0;

  dgInt32 k = 0;
  for (dgInt32 i = 0; i < faceCount; i++)
  {
    dgInt32 count = faceArray[i];
    for (dgInt32 j = 0; j < count; j++)
    {
      dgInt32 index = indexArray[k];
      pool[j] = index + m_vertexCount;
      k++;
    }

    dgInt32 convexFaces = 0;
    if (count == 3)
    {
      convexFaces = 1;
      dgBigVector p0(m_vertexPoints[pool[2]]);
      for (dgInt32 i = 0; i < 3; i++)
      {
        dgBigVector p1(m_vertexPoints[pool[i]]);
        dgBigVector edge(p1 - p0);
        dgFloat64 mag2 = edge % edge;
        if (mag2 < dgFloat32(1.0e-6f))
        {
          convexFaces = 0;
        }
        p0 = p1;
      }

      if (convexFaces)
      {
        dgBigVector edge0(m_vertexPoints[pool[2]] - m_vertexPoints[pool[0]]);
        dgBigVector edge1(m_vertexPoints[pool[1]] - m_vertexPoints[pool[0]]);
        dgBigVector normal(edge0 * edge1);
        dgFloat64 mag2 = normal % normal;
        if (mag2 < dgFloat32(1.0e-8f))
        {
          convexFaces = 0;
        }
      }

      if (convexFaces)
      {
        faces[0] = 3;
      }

    }
    else
    {
      convexFaces = AddConvexFace(count, pool, faces);
    }

    dgInt32 index = 0;
    for (dgInt32 k = 0; k < convexFaces; k++)
    {
      dgInt32 count = faces[k];
      m_vertexIndex[m_indexCount] = faceTagsData[i];
      m_indexCount++;
      for (dgInt32 j = 0; j < count; j++)
      {
        m_vertexIndex[m_indexCount] = pool[index];
        index++;
        m_indexCount++;
      }
      m_faceVertexCount[m_faceCount] = count + 1;
      m_faceCount++;
    }
  }
  m_vertexCount += vertexCount;
  m_run -= vertexCount;
  if (m_run <= 0)
  {
    PackArray();
  }
}

void dgPolygonSoupDatabaseBuilder::PackArray()
{
  dgStack<dgInt32> indexMapPool(m_vertexCount);
  dgInt32* const indexMap = &indexMapPool[0];
  m_vertexCount = dgVertexListToIndexList(&m_vertexPoints[0].m_x,
      sizeof(dgBigVector), 3, m_vertexCount, &indexMap[0], dgFloat32(1.0e-6f));

  dgInt32 k = 0;
  for (dgInt32 i = 0; i < m_faceCount; i++)
  {
    k++;

    dgInt32 count = m_faceVertexCount[i];
    for (dgInt32 j = 1; j < count; j++)
    {
      dgInt32 index = m_vertexIndex[k];
      index = indexMap[index];
      m_vertexIndex[k] = index;
      k++;
    }
  }

  m_run = DG_POINTS_RUN;
}

void dgPolygonSoupDatabaseBuilder::SingleFaceFixup()
{
  if (m_faceCount == 1)
  {
    dgInt32 index = 0;
    dgInt32 count = m_faceVertexCount[0];
    for (dgInt32 j = 0; j < count; j++)
    {
      m_vertexIndex[m_indexCount] = m_vertexIndex[index];
      index++;
      m_indexCount++;
    }
    m_faceVertexCount[m_faceCount] = count;
    m_faceCount++;
  }
}

void dgPolygonSoupDatabaseBuilder::EndAndOptimize(bool optimize)
{
  if (m_faceCount)
  {
    dgStack<dgInt32> indexMapPool(m_indexCount + m_vertexCount);

    dgInt32* const indexMap = &indexMapPool[0];
    m_vertexCount = dgVertexListToIndexList(&m_vertexPoints[0].m_x,
        sizeof(dgBigVector), 3, m_vertexCount, &indexMap[0],
        dgFloat32(1.0e-4f));

    dgInt32 k = 0;
    for (dgInt32 i = 0; i < m_faceCount; i++)
    {
      k++;
      dgInt32 count = m_faceVertexCount[i];
      for (dgInt32 j = 1; j < count; j++)
      {
        dgInt32 index = m_vertexIndex[k];
        index = indexMap[index];
        m_vertexIndex[k] = index;
        k++;
      }
    }

    OptimizeByIndividualFaces();
    if (optimize)
    {
      OptimizeByGroupID();
      OptimizeByIndividualFaces();
    }
  }
}

void dgPolygonSoupDatabaseBuilder::OptimizeByGroupID()
{
  dgTree<int, int> attribFilter(m_allocator);
  dgPolygonSoupDatabaseBuilder builder(m_allocator);
  dgPolygonSoupDatabaseBuilder builderAux(m_allocator);
  dgPolygonSoupDatabaseBuilder builderLeftOver(m_allocator);

  builder.Begin();
  dgInt32 polygonIndex = 0;
  for (dgInt32 i = 0; i < m_faceCount; i++)
  {
    dgInt32 attribute = m_vertexIndex[polygonIndex];
    if (!attribFilter.Find(attribute))
    {
      attribFilter.Insert(attribute, attribute);
      builder.OptimizeByGroupID(*this, i, polygonIndex, builderLeftOver);
      for (dgInt32 j = 0; builderLeftOver.m_faceCount && (j < 64); j++)
      {
        builderAux.m_faceVertexCount[builderLeftOver.m_faceCount] = 0;
        builderAux.m_vertexIndex[builderLeftOver.m_indexCount] = 0;
        builderAux.m_vertexPoints[builderLeftOver.m_vertexCount].m_x =
            dgFloat32(0.0f);

        memcpy(&builderAux.m_faceVertexCount[0],
            &builderLeftOver.m_faceVertexCount[0],
            sizeof(dgInt32) * builderLeftOver.m_faceCount);
        memcpy(&builderAux.m_vertexIndex[0], &builderLeftOver.m_vertexIndex[0],
            sizeof(dgInt32) * builderLeftOver.m_indexCount);
        memcpy(&builderAux.m_vertexPoints[0],
            &builderLeftOver.m_vertexPoints[0],
            sizeof(dgBigVector) * builderLeftOver.m_vertexCount);

        builderAux.m_faceCount = builderLeftOver.m_faceCount;
        builderAux.m_indexCount = builderLeftOver.m_indexCount;
        builderAux.m_vertexCount = builderLeftOver.m_vertexCount;

        dgInt32 prevFaceCount = builderLeftOver.m_faceCount;
        builderLeftOver.m_faceCount = 0;
        builderLeftOver.m_indexCount = 0;
        builderLeftOver.m_vertexCount = 0;

        builder.OptimizeByGroupID(builderAux, 0, 0, builderLeftOver);
        if (prevFaceCount == builderLeftOver.m_faceCount)
        {
          break;
        }
      }
      _ASSERTE(builderLeftOver.m_faceCount == 0);
    }
    polygonIndex += m_faceVertexCount[i];
  }
//	builder.End();
  builder.Optimize(false);

  m_faceVertexCount[builder.m_faceCount] = 0;
  m_vertexIndex[builder.m_indexCount] = 0;
  m_vertexPoints[builder.m_vertexCount].m_x = dgFloat32(0.0f);

  memcpy(&m_faceVertexCount[0], &builder.m_faceVertexCount[0],
      sizeof(dgInt32) * builder.m_faceCount);
  memcpy(&m_vertexIndex[0], &builder.m_vertexIndex[0],
      sizeof(dgInt32) * builder.m_indexCount);
  memcpy(&m_vertexPoints[0], &builder.m_vertexPoints[0],
      sizeof(dgBigVector) * builder.m_vertexCount);

  m_faceCount = builder.m_faceCount;
  m_indexCount = builder.m_indexCount;
  m_vertexCount = builder.m_vertexCount;
  m_normalCount = builder.m_normalCount;
}

void dgPolygonSoupDatabaseBuilder::OptimizeByGroupID(
    dgPolygonSoupDatabaseBuilder& source, dgInt32 faceNumber,
    dgInt32 faceIndexNumber, dgPolygonSoupDatabaseBuilder& leftOver)
{
  dgInt32 indexPool[1024 * 1];
  dgInt32 atributeData[1024 * 1];
  dgVector vertexPool[1024 * 1];
  dgPolyhedra polyhedra(m_allocator);

  dgInt32 attribute = source.m_vertexIndex[faceIndexNumber];
  for (dgInt32 i = 0; i < dgInt32(sizeof(atributeData) / sizeof(dgInt32)); i++)
  {
    indexPool[i] = i;
    atributeData[i] = attribute;
  }

  leftOver.Begin();
  polyhedra.BeginFace();
  for (dgInt32 i = faceNumber; i < source.m_faceCount; i++)
  {
    dgInt32 indexCount;
    indexCount = source.m_faceVertexCount[i];
    _ASSERTE(indexCount < 1024);

    if (source.m_vertexIndex[faceIndexNumber] == attribute)
    {
      dgEdge* const face = polyhedra.AddFace(indexCount - 1,
          &source.m_vertexIndex[faceIndexNumber + 1]);
      if (!face)
      {
        dgInt32 faceArray;
        for (dgInt32 j = 0; j < indexCount - 1; j++)
        {
          dgInt32 index;
          index = source.m_vertexIndex[faceIndexNumber + j + 1];
          vertexPool[j] = source.m_vertexPoints[index];
        }
        faceArray = indexCount - 1;
        leftOver.AddMesh(&vertexPool[0].m_x, indexCount - 1, sizeof(dgVector),
            1, &faceArray, indexPool, atributeData, dgGetIdentityMatrix());
      }
      else
      {
        // set the attribute
        dgEdge* ptr = face;
        do
        {
          ptr->m_userData = dgUnsigned64(attribute);
          ptr = ptr->m_next;
        } while (ptr != face);
      }
    }
    faceIndexNumber += indexCount;
  }

  leftOver.Optimize(false);
  polyhedra.EndFace();

  dgPolyhedra facesLeft(m_allocator);
  facesLeft.BeginFace();
  polyhedra.ConvexPartition(&source.m_vertexPoints[0].m_x, sizeof(dgBigVector),
      &facesLeft);
  facesLeft.EndFace();

  dgInt32 mark = polyhedra.IncLRU();
  dgPolyhedra::Iterator iter(polyhedra);
  for (iter.Begin(); iter; iter++)
  {
    dgEdge* const edge = &(*iter);
    if (edge->m_incidentFace < 0)
    {
      continue;
    }
    if (edge->m_mark == mark)
    {
      continue;
    }

    dgEdge* ptr = edge;
    dgInt32 indexCount = 0;
    do
    {
      ptr->m_mark = mark;
      vertexPool[indexCount] = source.m_vertexPoints[ptr->m_incidentVertex];
      indexCount++;
      ptr = ptr->m_next;
    } while (ptr != edge);

    if (indexCount >= 3)
    {
      AddMesh(&vertexPool[0].m_x, indexCount, sizeof(dgVector), 1, &indexCount,
          indexPool, atributeData, dgGetIdentityMatrix());
    }
  }

  mark = facesLeft.IncLRU();
  dgPolyhedra::Iterator iter1(facesLeft);
  for (iter1.Begin(); iter1; iter1++)
  {
    dgEdge* const edge = &(*iter1);
    if (edge->m_incidentFace < 0)
    {
      continue;
    }
    if (edge->m_mark == mark)
    {
      continue;
    }

    dgEdge* ptr = edge;
    dgInt32 indexCount = 0;
    do
    {
      ptr->m_mark = mark;
      vertexPool[indexCount] = source.m_vertexPoints[ptr->m_incidentVertex];
      indexCount++;
      ptr = ptr->m_next;
    } while (ptr != edge);
    if (indexCount >= 3)
    {
      AddMesh(&vertexPool[0].m_x, indexCount, sizeof(dgVector), 1, &indexCount, indexPool, atributeData, dgGetIdentityMatrix());
    }
  }
}

void dgPolygonSoupDatabaseBuilder::OptimizeByIndividualFaces()
{
  dgInt32* const faceArray = &m_faceVertexCount[0];
  dgInt32* const indexArray = &m_vertexIndex[0];

  dgInt32* const oldFaceArray = &m_faceVertexCount[0];
  dgInt32* const oldIndexArray = &m_vertexIndex[0];

  dgInt32 polygonIndex = 0;
  dgInt32 newFaceCount = 0;
  dgInt32 newIndexCount = 0;
  for (dgInt32 i = 0; i < m_faceCount; i++)
  {
    dgInt32 oldCount = oldFaceArray[i];
    dgInt32 count = FilterFace(oldCount - 1, &oldIndexArray[polygonIndex + 1]);
    if (count)
    {
      faceArray[newFaceCount] = count + 1;
      for (dgInt32 j = 0; j < count + 1; j++)
      {
        indexArray[newIndexCount + j] = oldIndexArray[polygonIndex + j];
      }
      newFaceCount++;
      newIndexCount += (count + 1);
    }
    polygonIndex += oldCount;
  }_ASSERTE(polygonIndex == m_indexCount);
  m_faceCount = newFaceCount;
  m_indexCount = newIndexCount;
}

void dgPolygonSoupDatabaseBuilder::End(bool optimize)
{
  Optimize(optimize);

  // build the normal array and adjacency array
  // calculate all face the normals
  dgInt32 indexCount = 0;
  m_normalPoints[m_faceCount].m_x = dgFloat64(0.0f);
  for (dgInt32 i = 0; i < m_faceCount; i++)
  {
    dgInt32 faceIndexCount = m_faceVertexCount[i];

    dgInt32* const ptr = &m_vertexIndex[indexCount + 1];
    dgBigVector v0(&m_vertexPoints[ptr[0]].m_x);
    dgBigVector v1(&m_vertexPoints[ptr[1]].m_x);
    dgBigVector e0(v1 - v0);
    dgBigVector normal(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(0.0f));
    for (dgInt32 j = 2; j < faceIndexCount - 1; j++)
    {
      dgBigVector v2(&m_vertexPoints[ptr[j]].m_x);
      dgBigVector e1(v2 - v0);
      normal += e0 * e1;
      e0 = e1;
    }
    normal = normal.Scale(dgRsqrt (normal % normal));

    m_normalPoints[i].m_x = normal.m_x;
    m_normalPoints[i].m_y = normal.m_y;
    m_normalPoints[i].m_z = normal.m_z;
    indexCount += faceIndexCount;
  }
  // compress normals array
  m_normalIndex[m_faceCount] = 0;
  m_normalCount = dgVertexListToIndexList(&m_normalPoints[0].m_x,
      sizeof(dgBigVector), 3, m_faceCount, &m_normalIndex[0],
      dgFloat32(1.0e-4f));
}

void dgPolygonSoupDatabaseBuilder::Optimize(bool optimize)
{
#define DG_PATITION_SIZE (1024 * 4)
  if (optimize && (m_faceCount > DG_PATITION_SIZE))
  {

    dgBigVector median(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(0.0f));
    dgBigVector varian(dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f),
        dgFloat32(0.0f));

    dgStack<dgVector> pool(1024 * 2);
    dgStack<dgInt32> indexArray(1024 * 2);
    dgInt32 polygonIndex = 0;
    for (dgInt32 i = 0; i < m_faceCount; i++)
    {

      dgBigVector p0(dgFloat32(1.0e10f), dgFloat32(1.0e10f), dgFloat32(1.0e10f),
          dgFloat32(0.0f));
      dgBigVector p1(dgFloat32(-1.0e10f), dgFloat32(-1.0e10f),
          dgFloat32(-1.0e10f), dgFloat32(0.0f));
      dgInt32 count = m_faceVertexCount[i];

      for (dgInt32 j = 1; j < count; j++)
      {
        dgInt32 k = m_vertexIndex[polygonIndex + j];
        p0.m_x = GetMin(p0.m_x, dgFloat64(m_vertexPoints[k].m_x));
        p0.m_y = GetMin(p0.m_y, dgFloat64(m_vertexPoints[k].m_y));
        p0.m_z = GetMin(p0.m_z, dgFloat64(m_vertexPoints[k].m_z));
        p1.m_x = GetMax(p1.m_x, dgFloat64(m_vertexPoints[k].m_x));
        p1.m_y = GetMax(p1.m_y, dgFloat64(m_vertexPoints[k].m_y));
        p1.m_z = GetMax(p1.m_z, dgFloat64(m_vertexPoints[k].m_z));
      }

      dgBigVector p((p0 + p1).Scale(0.5f));
      median += p;
      varian += p.CompProduct(p);
      polygonIndex += count;
    }

    varian = varian.Scale(dgFloat32(m_faceCount)) - median.CompProduct(median);

    dgInt32 axis = 0;
    dgFloat32 maxVarian = dgFloat32(-1.0e10f);
    for (dgInt32 i = 0; i < 3; i++)
    {
      if (varian[i] > maxVarian)
      {
        axis = i;
        maxVarian = dgFloat32(varian[i]);
      }
    }
    dgBigVector center = median.Scale(dgFloat32(1.0f) / dgFloat32(m_faceCount));
    dgFloat64 axisVal = center[axis];

    dgPolygonSoupDatabaseBuilder left(m_allocator);
    dgPolygonSoupDatabaseBuilder right(m_allocator);

    left.Begin();
    right.Begin();
    polygonIndex = 0;
    for (dgInt32 i = 0; i < m_faceCount; i++)
    {
      dgInt32 side = 0;
      dgInt32 count = m_faceVertexCount[i];
      for (dgInt32 j = 1; j < count; j++)
      {
        dgInt32 k;
        k = m_vertexIndex[polygonIndex + j];
        dgVector p(&m_vertexPoints[k].m_x);
        if (p[axis] > axisVal)
        {
          side = 1;
          break;
        }
      }

      dgInt32 faceArray = count - 1;
      dgInt32 faceTagsData = m_vertexIndex[polygonIndex];
      for (dgInt32 j = 1; j < count; j++)
      {
        dgInt32 k = m_vertexIndex[polygonIndex + j];
        pool[j - 1] = m_vertexPoints[k];
        indexArray[j - 1] = j - 1;
      }

      if (!side)
      {
        left.AddMesh(&pool[0].m_x, count - 1, sizeof(dgVector), 1, &faceArray,
            &indexArray[0], &faceTagsData, dgGetIdentityMatrix());
      }
      else
      {
        right.AddMesh(&pool[0].m_x, count - 1, sizeof(dgVector), 1, &faceArray,
            &indexArray[0], &faceTagsData, dgGetIdentityMatrix());
      }
      polygonIndex += count;
    }

    left.Optimize(optimize);
    right.Optimize(optimize);

    m_faceCount = 0;
    m_indexCount = 0;
    m_vertexCount = 0;
    m_normalCount = 0;
    polygonIndex = 0;
    for (dgInt32 i = 0; i < left.m_faceCount; i++)
    {
      dgInt32 count = left.m_faceVertexCount[i];
      dgInt32 faceArray = count - 1;
      dgInt32 faceTagsData = left.m_vertexIndex[polygonIndex];
      for (dgInt32 j = 1; j < count; j++)
      {
        dgInt32 k = left.m_vertexIndex[polygonIndex + j];
        pool[j - 1] = left.m_vertexPoints[k];
        indexArray[j - 1] = j - 1;
      }
      AddMesh(&pool[0].m_x, count - 1, sizeof(dgVector), 1, &faceArray,
          &indexArray[0], &faceTagsData, dgGetIdentityMatrix());
      polygonIndex += count;
    }

    polygonIndex = 0;
    for (dgInt32 i = 0; i < right.m_faceCount; i++)
    {
      dgInt32 count = right.m_faceVertexCount[i];
      dgInt32 faceArray = count - 1;
      dgInt32 faceTagsData = right.m_vertexIndex[polygonIndex];
      for (dgInt32 j = 1; j < count; j++)
      {
        dgInt32 k = right.m_vertexIndex[polygonIndex + j];
        pool[j - 1] = right.m_vertexPoints[k];
        indexArray[j - 1] = j - 1;
      }
      AddMesh(&pool[0].m_x, count - 1, sizeof(dgVector), 1, &faceArray,
          &indexArray[0], &faceTagsData, dgGetIdentityMatrix());
      polygonIndex += count;
    }

    if (m_faceCount < DG_PATITION_SIZE)
    {
      EndAndOptimize(optimize);
    }
    else
    {
      EndAndOptimize(false);
    }

  }
  else
  {
    EndAndOptimize(optimize);
  }
}

dgInt32 dgPolygonSoupDatabaseBuilder::FilterFace(dgInt32 count,
    dgInt32* const pool)
{
  if (count == 3)
  {
    dgBigVector p0(m_vertexPoints[pool[2]]);
    for (dgInt32 i = 0; i < 3; i++)
    {
      dgBigVector p1(m_vertexPoints[pool[i]]);
      dgBigVector edge(p1 - p0);
      dgFloat64 mag2 = edge % edge;
      if (mag2 < dgFloat32(1.0e-6f))
      {
        count = 0;
      }
      p0 = p1;
    }

    if (count == 3)
    {
      dgBigVector edge0(m_vertexPoints[pool[2]] - m_vertexPoints[pool[0]]);
      dgBigVector edge1(m_vertexPoints[pool[1]] - m_vertexPoints[pool[0]]);
      dgBigVector normal(edge0 * edge1);
      dgFloat64 mag2 = normal % normal;
      if (mag2 < dgFloat32(1.0e-8f))
      {
        count = 0;
      }
    }
  }
  else
  {
    dgPolySoupFilterAllocator polyhedra(m_allocator);

    count = polyhedra.AddFilterFace(dgUnsigned32(count), pool);

    if (!count)
    {
      return 0;
    }

    dgEdge* edge = &polyhedra.GetRoot()->GetInfo();
    if (edge->m_incidentFace < 0)
    {
      edge = edge->m_twin;
    }

    bool flag = true;
    while (flag)
    {
      flag = false;
      if (count >= 3)
      {
        dgEdge* ptr = edge;

        dgBigVector p0(&m_vertexPoints[ptr->m_incidentVertex].m_x);
        do
        {
          dgBigVector p1(&m_vertexPoints[ptr->m_next->m_incidentVertex].m_x);
          dgBigVector e0(p1 - p0);
          dgFloat64 mag2 = e0 % e0;
          if (mag2 < dgFloat32(1.0e-6f))
          {
            count--;
            flag = true;
            edge = ptr->m_next;
            ptr->m_prev->m_next = ptr->m_next;
            ptr->m_next->m_prev = ptr->m_prev;
            ptr->m_twin->m_next->m_prev = ptr->m_twin->m_prev;
            ptr->m_twin->m_prev->m_next = ptr->m_twin->m_next;
            break;
          }
          p0 = p1;
          ptr = ptr->m_next;
        } while (ptr != edge);
      }
    }
    if (count >= 3)
    {
      flag = true;
      dgBigVector normal(
          polyhedra.FaceNormal(edge, &m_vertexPoints[0].m_x,
              sizeof(dgBigVector)));

      _ASSERTE((normal % normal) > dgFloat32 (1.0e-10f));
      normal = normal.Scale(dgRsqrt (normal % normal + dgFloat32 (1.0e-20f)));

      while (flag)
      {
        flag = false;
        if (count >= 3)
        {
          dgEdge* ptr = edge;

          dgBigVector p0(&m_vertexPoints[ptr->m_prev->m_incidentVertex].m_x);
          dgBigVector p1(&m_vertexPoints[ptr->m_incidentVertex].m_x);
          dgBigVector e0(p1 - p0);
          e0 = e0.Scale(dgRsqrt (e0 % e0 + dgFloat32(1.0e-10f)));
          do
          {
            dgBigVector p2(&m_vertexPoints[ptr->m_next->m_incidentVertex].m_x);
            dgBigVector e1(p2 - p1);

            e1 = e1.Scale(dgRsqrt (e1 % e1 + dgFloat32(1.0e-10f)));
            dgFloat64 mag2 = e1 % e0;
            if (mag2 > dgFloat32(0.9999f))
            {
              count--;
              flag = true;
              edge = ptr->m_next;
              ptr->m_prev->m_next = ptr->m_next;
              ptr->m_next->m_prev = ptr->m_prev;
              ptr->m_twin->m_next->m_prev = ptr->m_twin->m_prev;
              ptr->m_twin->m_prev->m_next = ptr->m_twin->m_next;
              break;
            }

            dgBigVector n(e0 * e1);
            mag2 = n % normal;
            if (mag2 < dgFloat32(1.0e-5f))
            {
              count--;
              flag = true;
              edge = ptr->m_next;
              ptr->m_prev->m_next = ptr->m_next;
              ptr->m_next->m_prev = ptr->m_prev;
              ptr->m_twin->m_next->m_prev = ptr->m_twin->m_prev;
              ptr->m_twin->m_prev->m_next = ptr->m_twin->m_next;
              break;
            }

            e0 = e1;
            p1 = p2;
            ptr = ptr->m_next;
          } while (ptr != edge);
        }
      }
    }

    dgEdge* first = edge;
    if (count >= 3)
    {
      dgFloat64 best = dgFloat32(2.0f);
      dgEdge* ptr = edge;

      dgBigVector p0(&m_vertexPoints[ptr->m_incidentVertex].m_x);
      dgBigVector p1(&m_vertexPoints[ptr->m_next->m_incidentVertex].m_x);
      dgBigVector e0(p1 - p0);
      e0 = e0.Scale(dgRsqrt (e0 % e0 + dgFloat32(1.0e-10f)));
      do
      {
        dgBigVector p2(
            &m_vertexPoints[ptr->m_next->m_next->m_incidentVertex].m_x);
        dgBigVector e1(p2 - p1);

        e1 = e1.Scale(dgRsqrt (e1 % e1 + dgFloat32(1.0e-10f)));
        dgFloat64 mag2 = fabs(e1 % e0);
        if (mag2 < best)
        {
          best = mag2;
          first = ptr;
        }

        e0 = e1;
        p1 = p2;
        ptr = ptr->m_next;
      } while (ptr != edge);

      count = 0;
      ptr = first;
      do
      {
        pool[count] = ptr->m_incidentVertex;
        count++;
        ptr = ptr->m_next;
      } while (ptr != first);
    }

#ifdef _DEBUG
    if (count >= 3)
    {
      dgInt32 j0 = count - 2;
      dgInt32 j1 = count - 1;
      dgBigVector normal(
          polyhedra.FaceNormal(edge, &m_vertexPoints[0].m_x,
              sizeof(dgBigVector)));
      for (dgInt32 j2 = 0; j2 < count; j2++)
      {
        dgBigVector p0(&m_vertexPoints[pool[j0]].m_x);
        dgBigVector p1(&m_vertexPoints[pool[j1]].m_x);
        dgBigVector p2(&m_vertexPoints[pool[j2]].m_x);
        dgBigVector e0((p0 - p1));
        dgBigVector e1((p2 - p1));

        dgBigVector n(e1 * e0);
        _ASSERTE((n % normal) > dgFloat32 (0.0f));
        j0 = j1;
        j1 = j2;
      }
    }
#endif
  }

  return (count >= 3) ? count : 0;
}

dgInt32 dgPolygonSoupDatabaseBuilder::AddConvexFace(dgInt32 count,
    dgInt32* const pool, dgInt32* const facesArray)
{
  dgPolySoupFilterAllocator polyhedra(m_allocator);

  count = polyhedra.AddFilterFace(dgUnsigned32(count), pool);

  dgEdge* edge = &polyhedra.GetRoot()->GetInfo();
  if (edge->m_incidentFace < 0)
  {
    edge = edge->m_twin;
  }

  dgInt32 isconvex = 1;
  dgInt32 facesCount = 0;

  dgInt32 flag = 1;
  while (flag)
  {
    flag = 0;
    if (count >= 3)
    {
      dgEdge* ptr = edge;

      dgBigVector p0(&m_vertexPoints[ptr->m_incidentVertex].m_x);
      do
      {
        dgBigVector p1(&m_vertexPoints[ptr->m_next->m_incidentVertex].m_x);
        dgBigVector e0(p1 - p0);
        dgFloat64 mag2 = e0 % e0;
        if (mag2 < dgFloat32(1.0e-6f))
        {
          count--;
          flag = 1;
          edge = ptr->m_next;
          ptr->m_prev->m_next = ptr->m_next;
          ptr->m_next->m_prev = ptr->m_prev;
          ptr->m_twin->m_next->m_prev = ptr->m_twin->m_prev;
          ptr->m_twin->m_prev->m_next = ptr->m_twin->m_next;
          break;
        }
        p0 = p1;
        ptr = ptr->m_next;
      } while (ptr != edge);
    }
  }
  if (count >= 3)
  {
    flag = 1;

    while (flag)
    {
      flag = 0;
      if (count >= 3)
      {
        dgEdge* ptr = edge;

        dgBigVector p0(&m_vertexPoints[ptr->m_prev->m_incidentVertex].m_x);
        dgBigVector p1(&m_vertexPoints[ptr->m_incidentVertex].m_x);
        dgBigVector e0(p1 - p0);
        e0 = e0.Scale(dgRsqrt (e0 % e0 + dgFloat32(1.0e-10f)));
        do
        {
          dgBigVector p2(&m_vertexPoints[ptr->m_next->m_incidentVertex].m_x);
          dgBigVector e1(p2 - p1);

          e1 = e1.Scale(dgRsqrt (e1 % e1 + dgFloat32(1.0e-10f)));
          dgFloat64 mag2 = e1 % e0;
          if (mag2 > dgFloat32(0.9999f))
          {
            count--;
            flag = 1;
            edge = ptr->m_next;
            ptr->m_prev->m_next = ptr->m_next;
            ptr->m_next->m_prev = ptr->m_prev;
            ptr->m_twin->m_next->m_prev = ptr->m_twin->m_prev;
            ptr->m_twin->m_prev->m_next = ptr->m_twin->m_next;
            break;
          }

          e0 = e1;
          p1 = p2;
          ptr = ptr->m_next;
        } while (ptr != edge);
      }
    }

    dgBigVector normal(
        polyhedra.FaceNormal(edge, &m_vertexPoints[0].m_x,
            sizeof(dgBigVector)));
    dgFloat64 mag2 = normal % normal;
    if (mag2 < dgFloat32(1.0e-8f))
    {
      return 0;
    }
    normal = normal.Scale(dgRsqrt (mag2));

    if (count >= 3)
    {
      dgEdge* ptr = edge;
      dgBigVector p0(&m_vertexPoints[ptr->m_prev->m_incidentVertex].m_x);
      dgBigVector p1(&m_vertexPoints[ptr->m_incidentVertex].m_x);
      dgBigVector e0(p1 - p0);
      e0 = e0.Scale(dgRsqrt (e0 % e0 + dgFloat32(1.0e-10f)));
      do
      {
        dgBigVector p2(&m_vertexPoints[ptr->m_next->m_incidentVertex].m_x);
        dgBigVector e1(p2 - p1);

        e1 = e1.Scale(dgRsqrt (e1 % e1 + dgFloat32(1.0e-10f)));

        dgBigVector n(e0 * e1);
        dgFloat64 mag2 = n % normal;
        if (mag2 < dgFloat32(1.0e-5f))
        {
          isconvex = 0;
          break;
        }

        e0 = e1;
        p1 = p2;
        ptr = ptr->m_next;
      } while (ptr != edge);
    }
  }

  if (isconvex)
  {
    dgEdge* const first = edge;
    if (count >= 3)
    {
      count = 0;
      dgEdge* ptr = first;
      do
      {
        pool[count] = ptr->m_incidentVertex;
        count++;
        ptr = ptr->m_next;
      } while (ptr != first);
      facesArray[facesCount] = count;
      facesCount = 1;
    }
  }
  else
  {
    dgPolyhedra leftOver(m_allocator);
    dgPolyhedra polyhedra2(m_allocator);
    dgEdge* ptr = edge;
    count = 0;
    do
    {
      pool[count] = ptr->m_incidentVertex;
      count++;
      ptr = ptr->m_next;
    } while (ptr != edge);

    polyhedra2.BeginFace();
    polyhedra2.AddFace(count, pool);
    polyhedra2.EndFace();
    leftOver.BeginFace();
    polyhedra2.ConvexPartition(&m_vertexPoints[0].m_x, sizeof(dgTriplex),
        &leftOver);
    leftOver.EndFace();
    _ASSERTE(leftOver.GetCount() == 0);

    dgInt32 mark = polyhedra2.IncLRU();
    dgInt32 index = 0;
    dgPolyhedra::Iterator iter(polyhedra2);
    for (iter.Begin(); iter; iter++)
    {
      dgEdge* const edge = &(*iter);
      if (edge->m_incidentFace < 0)
      {
        continue;
      }
      if (edge->m_mark == mark)
      {
        continue;
      }

      ptr = edge;
      count = 0;
      do
      {
        ptr->m_mark = mark;
        pool[index] = ptr->m_incidentVertex;
        index++;
        count++;
        ptr = ptr->m_next;
      } while (ptr != edge);

      facesArray[facesCount] = count;
      facesCount++;
    }
  }

  return facesCount;
}
