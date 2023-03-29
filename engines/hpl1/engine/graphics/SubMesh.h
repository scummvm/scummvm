/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_SUB_MESH_H
#define HPL_SUB_MESH_H

#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/math/MathTypes.h"
#include "hpl1/engine/math/MeshTypes.h"
#include "hpl1/engine/system/SystemTypes.h"

namespace hpl {

class iMaterial;
class iVertexBuffer;

class cMesh;

class cMaterialManager;

class cSubMesh {
	friend class cMesh;
	friend class cSubMeshEntity;

public:
	cSubMesh(const tString &asName, cMaterialManager *apMaterialManager);
	~cSubMesh();

	void SetMaterial(iMaterial *apMaterial);
	void SetVertexBuffer(iVertexBuffer *apVtxBuffer);

	// Renderable implementation.
	iMaterial *GetMaterial();
	iVertexBuffer *GetVertexBuffer();

	tString GetName() { return msName; }

	// Vertex-Bone pairs
	void ResizeVertexBonePairs(int alSize);
	int GetVertexBonePairNum();
	cVertexBonePair &GetVertexBonePair(int alNum);

	void AddVertexBonePair(const cVertexBonePair &aPair);
	void ClearVertexBonePairs();

	void CompileBonePairs();

	const cTriEdge &GetEdge(int alIndex) const { return mvEdges[alIndex]; }
	int GetEdgeNum() { return (int)mvEdges.size(); }

	tTriEdgeVec *GetEdgeVecPtr() { return &mvEdges; }

	tTriangleDataVec *GetTriangleVecPtr() { return &mvTriangles; }

	void SetDoubleSided(bool abX) { mbDoubleSided = abX; }
	bool GetDoubleSided() { return mbDoubleSided; }

	const tString &GetGroup() { return msGroup; }
	void SetGroup(const tString &asGroup) { msGroup = asGroup; }

	const tString &GetNodeName() { return msNodeName; }
	void SetNodeName(const tString &asNodeName) { msNodeName = asNodeName; }

	void SetModelScale(const cVector3f &avScale) { mvModelScale = avScale; }
	cVector3f GetModelScale() { return mvModelScale; }

	const cMatrixf &GetLocalTransform() { return m_mtxLocalTransform; }
	void SetLocalTransform(const cMatrixf &a_mtxTrans) { m_mtxLocalTransform = a_mtxTrans; }

	bool GetIsOneSided() { return mbIsOneSided; }
	const cVector3f &GetOneSidedNormal() { return mvOneSidedNormal; }

	void Compile();

private:
	void CheckOneSided();

	iMaterial *mpMaterial;
	iVertexBuffer *mpVtxBuffer;

	cMatrixf m_mtxLocalTransform;

	tVertexBonePairVec mvVtxBonePairs;

	tString msGroup;
	tString msNodeName;

	float *mpVertexWeights;
	unsigned char *mpVertexBones;

	tTriEdgeVec mvEdges;
	tTriangleDataVec mvTriangles;

	cVector3f mvModelScale;

	tString msName;

	bool mbDoubleSided;

	bool mbIsOneSided;
	cVector3f mvOneSidedNormal;

	cMaterialManager *mpMaterialManager;

	cMesh *mpParent;
};

} // namespace hpl

#endif // HPL_SUB_MESH_H
