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

#ifndef __dgMeshEffectSolidTree_H__
#define __dgMeshEffectSolidTree_H__

#include "hpl1/engine/libraries/newton/core/dg.h"

class dgMeshEffect;
class dgMeshTreeCSGFace;
class dgMeshEffectSolidTree;

class dgMeshEffectSolidTree {
public:
	DG_CLASS_ALLOCATOR(allocator)

	enum dgPlaneType {
		m_divider = 1,
		m_empty,
		m_solid,
	};

	dgMeshEffectSolidTree(dgPlaneType type);
	dgMeshEffectSolidTree(const dgMeshEffect &mesh, dgEdge *const face);
	dgMeshEffectSolidTree(const dgHugeVector &plane, dgMemoryAllocator *const allocator);
	~dgMeshEffectSolidTree();

	dgHugeVector BuildPlane(const dgMeshEffect &mesh, dgEdge *const face) const;
	void AddFace(const dgMeshEffect &mesh, dgEdge *const face);

	dgPlaneType GetPointSide(const dgHugeVector &point) const;
	dgPlaneType GetFaceSide(const dgMeshTreeCSGFace *const face) const;

	dgPlaneType m_planeType;
	dgMeshEffectSolidTree *m_back;
	dgMeshEffectSolidTree *m_front;
	dgHugeVector m_plane;
};

class dgMeshTreeCSGFace : public dgList<dgHugeVector>, public dgRefCounter {
public:
	dgMeshTreeCSGFace(const dgMeshEffect &mesh, dgEdge *const face);
	dgMeshTreeCSGFace(dgMemoryAllocator *const allocator, dgInt32 count, const dgHugeVector *const points);

	void Clip(const dgHugeVector &plane, dgMeshTreeCSGFace **leftOut, dgMeshTreeCSGFace **rightOut);
	void MergeMissingVertex(const dgMeshTreeCSGFace *const face);
	bool IsPointOnEdge(const dgHugeVector &p0, const dgHugeVector &p1, const dgHugeVector &q) const;
	bool CheckFaceArea(dgInt32 count, const dgHugeVector *const points) const;
	dgInt32 RemoveDuplicates(dgInt32 count, dgHugeVector *const points) const;

	dgHugeVector FaceNormal() const;

	bool CheckConvex(const dgHugeVector &normal) const;

#ifdef _DEBUG
	dgMatrix DebugMatrix() const;
	void Trace(const dgMatrix &matrix) const;
#endif

	dgMeshEffectSolidTree::dgPlaneType m_side;
};

#endif
