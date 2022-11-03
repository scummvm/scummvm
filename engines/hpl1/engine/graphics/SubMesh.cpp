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

#include "hpl1/engine/graphics/SubMesh.h"

#include "hpl1/engine/graphics/Bone.h"
#include "hpl1/engine/graphics/Material.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/Skeleton.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/MaterialManager.h"

#include "hpl1/engine/system/MemoryManager.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSubMesh::cSubMesh(const tString &asName, cMaterialManager *apMaterialManager) {
	mpMaterialManager = apMaterialManager;

	msName = asName;
	msNodeName = "";

	mpMaterial = NULL;
	mpVtxBuffer = NULL;

	mbDoubleSided = false;

	mpVertexWeights = NULL;
	mpVertexBones = NULL;

	m_mtxLocalTransform = cMatrixf::Identity;

	mbIsOneSided = false;
	mvOneSidedNormal = 0;
}

//-----------------------------------------------------------------------

cSubMesh::~cSubMesh() {
	if (mpMaterial)
		mpMaterialManager->Destroy(mpMaterial);
	if (mpVtxBuffer)
		hplDelete(mpVtxBuffer);

	if (mpVertexBones)
		hplDeleteArray(mpVertexBones);
	if (mpVertexWeights)
		hplDeleteArray(mpVertexWeights);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cSubMesh::SetMaterial(iMaterial *apMaterial) {
	if (mpMaterial)
		mpMaterialManager->Destroy(mpMaterial);
	mpMaterial = apMaterial;
}

//-----------------------------------------------------------------------

void cSubMesh::SetVertexBuffer(iVertexBuffer *apVtxBuffer) {
	if (mpVtxBuffer == apVtxBuffer)
		return;

	mpVtxBuffer = apVtxBuffer;
}

//-----------------------------------------------------------------------

iMaterial *cSubMesh::GetMaterial() {
	return mpMaterial;
}

//-----------------------------------------------------------------------

iVertexBuffer *cSubMesh::GetVertexBuffer() {
	return mpVtxBuffer;
}

//-----------------------------------------------------------------------

void cSubMesh::ResizeVertexBonePairs(int alSize) {
	mvVtxBonePairs.reserve(alSize);
}

int cSubMesh::GetVertexBonePairNum() {
	return (int)mvVtxBonePairs.size();
}
cVertexBonePair &cSubMesh::GetVertexBonePair(int alNum) {
	return mvVtxBonePairs[alNum];
}

void cSubMesh::AddVertexBonePair(const cVertexBonePair &aPair) {
	mvVtxBonePairs.push_back(aPair);
}

void cSubMesh::ClearVertexBonePairs() {
	mvVtxBonePairs.clear();
}

//-----------------------------------------------------------------------

/// normalize weights here?
void cSubMesh::CompileBonePairs() {
	mpVertexWeights = hplNewArray(float, 4 * mpVtxBuffer->GetVertexNum());
	mpVertexBones = hplNewArray(unsigned char, 4 * mpVtxBuffer->GetVertexNum());
	memset(mpVertexWeights, 0, 4 * mpVtxBuffer->GetVertexNum() * sizeof(float));

	for (int i = 0; i < (int)mvVtxBonePairs.size(); i++) {
		cVertexBonePair &Pair = mvVtxBonePairs[i];

		float *pWeight = &mpVertexWeights[Pair.vtxIdx * 4];
		unsigned char *pBoneIdx = &mpVertexBones[Pair.vtxIdx * 4];
		int lPos = -1;
		// Find out where to add the next weight.
		for (int j = 0; j < 4; j++) {
			if (pWeight[j] == 0) {
				lPos = j;
				break;
			}
		}
		// If no place was found there are too many bones on the vertex.
		if (lPos == -1) {
			Warning("More than 4 bones on a vertex!\n");
			continue;
		}

		pWeight[lPos] = Pair.weight;
		pBoneIdx[lPos] = Pair.boneIdx;
	}

	bool bUnconnectedVertexes = false;

	// Normalize the weights
	for (int vtx = 0; vtx < mpVtxBuffer->GetVertexNum(); ++vtx) {
		float *pWeight = &mpVertexWeights[vtx * 4];

		// check if the vertex is missing bone connection
		if (pWeight[0] == 0) {
			bUnconnectedVertexes = true;
			continue;
		}

		float fTotal = 0;
		int lNum = 0;
		while (pWeight[lNum] != 0 && lNum <= 4) {
			fTotal += pWeight[lNum];
			lNum++;
		}
		for (int i = 0; i < lNum; ++i) {
			pWeight[lNum] = pWeight[lNum] / fTotal;
		}
	}

	if (bUnconnectedVertexes) {
		Warning("Some vertices in sub mesh '%s' in mesh '%s' are not connected to a bone!\n", GetName().c_str(), mpParent->GetName().c_str());
	}
}

//-----------------------------------------------------------------------

void cSubMesh::Compile() {
	CheckOneSided();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIAVTE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cSubMesh::CheckOneSided() {
	// Log("--- %s\n",GetName().c_str());

	if (mpVtxBuffer == NULL)
		return;

	int lIdxNum = mpVtxBuffer->GetIndexNum();

	if (lIdxNum > 400 * 3)
		return; // Just skip larger buffers for now, they should never be planes.

	unsigned int *pIndices = mpVtxBuffer->GetIndices();
	float *pPositions = mpVtxBuffer->GetArray(eVertexFlag_Position);

	bool bFirst = true;
	cVector3f vNormalSum;
	cVector3f vFirstNormal;
	int vTri[3];
	const int lVtxStride = kvVertexElements[cMath::Log2ToInt(eVertexFlag_Position)];
	float fCount = 0;

	for (int i = 0; i < lIdxNum; i += 3) {
		// Log("%d \n",i);

		vTri[0] = pIndices[i + 0];
		vTri[1] = pIndices[i + 1];
		vTri[2] = pIndices[i + 2];

		const float *pVtx0 = &pPositions[vTri[0] * lVtxStride];
		const float *pVtx1 = &pPositions[vTri[1] * lVtxStride];
		const float *pVtx2 = &pPositions[vTri[2] * lVtxStride];

		cVector3f vEdge1(pVtx1[0] - pVtx0[0], pVtx1[1] - pVtx0[1], pVtx1[2] - pVtx0[2]);
		cVector3f vEdge2(pVtx2[0] - pVtx0[0], pVtx2[1] - pVtx0[1], pVtx2[2] - pVtx0[2]);

		cVector3f vNormal = cMath::Vector3Normalize(cMath::Vector3Cross(vEdge2, vEdge1));

		// Log(" normal: %s\n",vNormal.ToString().c_str());

		if (bFirst) {
			bFirst = false;
			vFirstNormal = vNormal;
			vNormalSum = vNormal;
		} else {
			if (cMath::Vector3Dot(vFirstNormal, vNormal) < 0.9f)
				return;
			vNormalSum += vNormal;
		}

		fCount += 1;
	}

	mbIsOneSided = true;
	mvOneSidedNormal = cMath::Vector3Normalize(vNormalSum / fCount);
}

//-----------------------------------------------------------------------
} // namespace hpl
