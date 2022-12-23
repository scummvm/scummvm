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

#ifndef __DG_CONVEXHULL_4D__
#define __DG_CONVEXHULL_4D__

#include "dgStdafx.h"
#include "dgList.h"
#include "dgArray.h"
#include "dgPlane.h"
#include "dgVector.h"
#include "dgMatrix.h"
#include "dgQuaternion.h"

class dgMemoryAllocator;
class dgAABBPointTree4d;

class dgHullVector: public dgBigVector {
public:
	void operator = (const dgBigVector &a) {
		m_x = a.m_x;
		m_y = a.m_y;
		m_z = a.m_z;
		m_w = a.m_w;
		m_index = 0;
		m_mark = 0;
	}

	dgInt32 m_index;
	dgInt32 m_mark;
};


class dgConvexHull4dTetraherum {
public:
	class dgTetrahedrumFace {
	public:
		dgInt32 m_index[3];
		dgInt32 m_otherVertex;
		dgList<dgConvexHull4dTetraherum>::dgListNode *m_twin;
	};


	class dgTetrahedrumPlane: public dgBigVector {
	public:
		dgTetrahedrumPlane(const dgBigVector &p0, const dgBigVector &p1, const dgBigVector &p2, const dgBigVector &p3);
		dgFloat64 Evalue(const dgBigVector &point) const;
		dgFloat64 m_dist;
	};

	dgConvexHull4dTetraherum();
	dgTetrahedrumPlane GetPlaneEquation(const dgHullVector *const points) const;
	dgFloat64 Evalue(const dgHullVector *const pointArray, const dgBigVector &point) const;

	dgBigVector CircumSphereCenter(const dgHullVector *const pointArray) const;

	dgInt32 GetMark() const {
		return m_mark;
	}
	void SetMark(dgInt32 mark) {
		m_mark = mark;
	}

	dgTetrahedrumFace m_faces[4];
#ifdef _DEBUG
	dgInt32 m_debugID;
#endif

private:
	void Init(const dgHullVector *const points, dgInt32 v0, dgInt32 v1, dgInt32 v2, dgInt32 v3);

	dgInt32 m_mark;
	friend class dgConvexHull4d;
	friend class dgDelaunayTetrahedralization;
};


class dgConvexHull4d: public dgList<dgConvexHull4dTetraherum> {
public:
	dgConvexHull4d(dgMemoryAllocator *const allocator, const dgBigVector *const vertexCloud, dgInt32 count, dgFloat32 distTol);
	virtual ~dgConvexHull4d();

	dgInt32 GetVertexCount() const;
	dgInt32 GetVertexIndex(dgInt32 i) const;
	const dgBigVector &GetVertex(dgInt32 i) const;

	const dgHullVector *GetHullVertexArray() const;

	dgInt32 IncMark();

protected:
	dgConvexHull4d(dgMemoryAllocator *const allocator);

	void BuildHull(dgMemoryAllocator *const allocator, const dgBigVector *const vertexCloud, dgInt32 count, dgFloat64 distTol);

	virtual dgInt32 AddVertex(const dgBigVector &vertex);
	virtual dgInt32 InitVertexArray(dgHullVector *const points, const dgBigVector *const vertexCloud, dgInt32 count, void *memoryPool, dgInt32 maxMemSize);
	virtual dgListNode *AddFace(dgInt32 i0, dgInt32 i1, dgInt32 i2, dgInt32 i3);
	virtual void DeleteFace(dgListNode *const node) ;

	dgListNode *FindFacingNode(const dgBigVector &vertex);

	dgInt32 BuildNormalList(dgBigVector *const normalArray) const;
	void InsertNewVertex(dgInt32 vertexIndex, dgListNode *const frontFace, dgList<dgListNode *> &deletedFaces, dgList<dgListNode *> &newFaces);
	dgInt32 SupportVertex(dgAABBPointTree4d **const tree, const dgHullVector *const points, const dgBigVector &dir) const;
	void TessellateTriangle(dgInt32 level, const dgVector &p0, const dgVector &p1, const dgVector &p2, dgInt32 &count, dgBigVector *const ouput, dgInt32 &start) const;

	void CalculateConvexHull(dgAABBPointTree4d *vertexTree, dgHullVector *const points, dgInt32 count, dgFloat64 distTol);
	void LinkSibling(dgListNode *node0, dgListNode *node1) const;
	bool Sanity() const;
	static dgInt32 ConvexCompareVertex(const dgHullVector *const  A, const dgHullVector *const B, void *const context);

	dgAABBPointTree4d *BuildTree(dgAABBPointTree4d *const parent, dgHullVector *const points, dgInt32 count, dgInt32 baseIndex, dgInt8 **const memoryPool, dgInt32 &maxMemSize) const;

	dgInt32 m_mark;
	dgInt32 m_count;
	dgFloat64 m_diag;
	dgArray<dgHullVector> m_points;
};


inline dgInt32 dgConvexHull4d::IncMark() {
	m_mark ++;
	return m_mark;
}

inline dgInt32 dgConvexHull4d::GetVertexCount() const {
	return m_count;
}

inline dgInt32 dgConvexHull4d::GetVertexIndex(dgInt32 index) const {
	NEWTON_ASSERT(index >= 0);
	NEWTON_ASSERT(index < m_count);
	return m_points[index].m_index;
}


inline const dgBigVector &dgConvexHull4d::GetVertex(dgInt32 index) const {
	NEWTON_ASSERT(index >= 0);
	NEWTON_ASSERT(index < m_count);
//	return dgVector (dgFloat32 (m_points[index].m_x), dgFloat32 (m_points[index].m_y), dgFloat32 (m_points[index].m_z), dgFloat32 (m_points[index].m_w));
	return m_points[index];
}

inline const dgHullVector *dgConvexHull4d::GetHullVertexArray() const {
	return &m_points[0];
}

#endif
