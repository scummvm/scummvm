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

#include "hpl1/engine/graphics/MeshCreator.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/Mesh.h"
#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/graphics/SubMesh.h"
#include "hpl1/engine/graphics/VertexBuffer.h"
#include "hpl1/engine/resources/AnimationManager.h"
#include "hpl1/engine/resources/MaterialManager.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/system/String.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMeshCreator::cMeshCreator(iLowLevelGraphics *apLowLevelGraphics, cResources *apResources) {
	mpLowLevelGraphics = apLowLevelGraphics;
	mpResources = apResources;
}

//-----------------------------------------------------------------------

cMeshCreator::~cMeshCreator() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cMesh *cMeshCreator::CreateBox(const tString &asName, cVector3f avSize, const tString &asMaterial) {
	cMesh *pMesh = hplNew(cMesh, (asName, mpResources->GetMaterialManager(),
								  mpResources->GetAnimationManager()));

	cSubMesh *pSubMesh = pMesh->CreateSubMesh("Main");

	iMaterial *pMat = mpResources->GetMaterialManager()->CreateMaterial(asMaterial);
	pSubMesh->SetMaterial(pMat);
	iVertexBuffer *pVtxBuff = CreateBoxVertexBuffer(avSize);
	pSubMesh->SetVertexBuffer(pVtxBuff);

	return pMesh;
}

//-----------------------------------------------------------------------

iVertexBuffer *cMeshCreator::CreateSkyBoxVertexBuffer(float afSize) {
	iVertexBuffer *pSkyBox = mpLowLevelGraphics->CreateVertexBuffer(
		eVertexFlag_Color0 | eVertexFlag_Position | eVertexFlag_Texture0,
		eVertexBufferDrawType_Quad, eVertexBufferUsageType_Static);

	float fSize = afSize;

	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++) {
				if (x == 0 && y == 0 && z == 0)
					continue;
				if (ABS(x) + ABS(y) + ABS(z) > 1)
					continue;

				// Direction (could say inverse normal) of the quad.
				cVector3f vDir;
				cVector3f vSide;

				cVector3f vAdd[4];
				if (ABS(x)) {
					vDir.x = (float)x;

					vAdd[0].y = 1;
					vAdd[0].z = 1;
					vAdd[1].y = -1;
					vAdd[1].z = 1;
					vAdd[2].y = -1;
					vAdd[2].z = -1;
					vAdd[3].y = 1;
					vAdd[3].z = -1;
				} else if (ABS(y)) {
					vDir.y = (float)y;

					vAdd[0].z = 1;
					vAdd[0].x = 1;
					vAdd[1].z = -1;
					vAdd[1].x = 1;
					vAdd[2].z = -1;
					vAdd[2].x = -1;
					vAdd[3].z = 1;
					vAdd[3].x = -1;
				} else if (ABS(z)) {
					vAdd[0].y = 1;
					vAdd[0].x = 1;
					vAdd[1].y = 1;
					vAdd[1].x = -1;
					vAdd[2].y = -1;
					vAdd[2].x = -1;
					vAdd[3].y = -1;
					vAdd[3].x = 1;

					vDir.z = (float)z;
				}

				// Log("Side: (%.0f : %.0f : %.0f) [ ", vDir.x,  vDir.y,vDir.z);
				for (int i = 0; i < 4; i++) {
					int idx = i;
					if (x + y + z < 0)
						idx = 3 - i;

					pSkyBox->AddColor(eVertexFlag_Color0, cColor(1, 1, 1, 1));
					pSkyBox->AddVertex(eVertexFlag_Position, (vDir + vAdd[idx]) * fSize);
					pSkyBox->AddVertex(eVertexFlag_Texture0, vDir + vAdd[idx]);

					vSide = vDir + vAdd[idx];
					// Log("%d: (%.1f : %.1f : %.1f) ", i,vSide.x,  vSide.y,vSide.z);
				}
				// Log("\n");
			}

	for (int i = 0; i < 24; i++)
		pSkyBox->AddIndex(i);

	if (!pSkyBox->Compile(0)) {
		hplDelete(pSkyBox);
		return NULL;
	}
	return pSkyBox;
}

//-----------------------------------------------------------------------

iVertexBuffer *cMeshCreator::CreateBoxVertexBuffer(cVector3f avSize) {
	iVertexBuffer *pBox = mpLowLevelGraphics->CreateVertexBuffer(
		eVertexFlag_Color0 | eVertexFlag_Position | eVertexFlag_Texture0 |
			eVertexFlag_Texture1 | eVertexFlag_Normal,
		eVertexBufferDrawType_Tri, eVertexBufferUsageType_Static);

	avSize = avSize * 0.5;

	int lVtxIdx = 0;

	for (int x = -1; x <= 1; x++)
		for (int y = -1; y <= 1; y++)
			for (int z = -1; z <= 1; z++) {
				if (x == 0 && y == 0 && z == 0)
					continue;
				if (ABS(x) + ABS(y) + ABS(z) > 1)
					continue;

				// Direction (could say inverse normal) of the quad.
				cVector3f vDir;
				cVector3f vSide;

				cVector3f vAdd[4];
				if (ABS(x)) {
					vDir.x = (float)x;

					vAdd[0].y = 1;
					vAdd[0].z = 1;
					vAdd[1].y = -1;
					vAdd[1].z = 1;
					vAdd[2].y = -1;
					vAdd[2].z = -1;
					vAdd[3].y = 1;
					vAdd[3].z = -1;
				} else if (ABS(y)) {
					vDir.y = (float)y;

					vAdd[0].z = 1;
					vAdd[0].x = 1;
					vAdd[1].z = -1;
					vAdd[1].x = 1;
					vAdd[2].z = -1;
					vAdd[2].x = -1;
					vAdd[3].z = 1;
					vAdd[3].x = -1;
				} else if (ABS(z)) {
					vAdd[0].y = 1;
					vAdd[0].x = 1;
					vAdd[1].y = 1;
					vAdd[1].x = -1;
					vAdd[2].y = -1;
					vAdd[2].x = -1;
					vAdd[3].y = -1;
					vAdd[3].x = 1;

					vDir.z = (float)z;
				}

				// Log("Side: (%.0f : %.0f : %.0f) [ ", vDir.x,  vDir.y,vDir.z);
				for (int i = 0; i < 4; i++) {
					int idx = GetBoxIdx(i, x, y, z);
					cVector3f vTex = GetBoxTex(i, x, y, z, vAdd);

					pBox->AddColor(eVertexFlag_Color0, cColor(1, 1, 1, 1));
					pBox->AddVertex(eVertexFlag_Position, (vDir + vAdd[idx]) * avSize);
					pBox->AddVertex(eVertexFlag_Normal, vDir);

					// texture coord
					cVector3f vCoord = cVector3f((vTex.x + 1) * 0.5f, (vTex.y + 1) * 0.5f, 0);
					pBox->AddVertex(eVertexFlag_Texture0, vCoord);

					vSide = vDir + vAdd[idx];
					// Log("%d: Tex: (%.1f : %.1f : %.1f) ", i,vTex.x,  vTex.y,vTex.z);
					// Log("%d: (%.1f : %.1f : %.1f) ", i,vSide.x,  vSide.y,vSide.z);
				}

				for (int i = 0; i < 3; i++)
					pBox->AddIndex(lVtxIdx + i);
				pBox->AddIndex(lVtxIdx + 2);
				pBox->AddIndex(lVtxIdx + 3);
				pBox->AddIndex(lVtxIdx + 0);

				lVtxIdx += 4;

				// Log("\n");
			}

	if (!pBox->Compile(eVertexCompileFlag_CreateTangents)) {
		hplDelete(pBox);
		return NULL;
	}
	return pBox;
}

cVector3f cMeshCreator::GetBoxTex(int i, int x, int y, int z, cVector3f *vAdd) {
	cVector3f vTex;

	if (ABS(x)) {
		vTex.x = vAdd[i].z;
		vTex.y = vAdd[i].y;
	} else if (ABS(y)) {
		vTex.x = vAdd[i].x;
		vTex.y = vAdd[i].z;
	} else if (ABS(z)) {
		vTex.x = vAdd[i].x;
		vTex.y = vAdd[i].y;
	}

	// Inverse for negative directions
	if (x + y + z < 0) {
		vTex.x = -vTex.x;
		vTex.y = -vTex.y;
	}

	return vTex;
}
int cMeshCreator::GetBoxIdx(int i, int x, int y, int z) {
	int idx = i;
	if (x + y + z > 0)
		idx = 3 - i;

	return idx;
}

//-----------------------------------------------------------------------

cMesh2D *cMeshCreator::Create2D(tString asName, cVector2f mvSize) {
	int i;
	tString sMeshName = cString::ToLowerCase(asName);
	cMesh2D *pMesh;

	/// SQUARE ///////////////////////
	if (sMeshName == "square") {
		cVector2f vPos[4] = {cVector2f(mvSize.x / 2, -mvSize.y / 2), cVector2f(mvSize.x / 2, mvSize.y / 2), cVector2f(-mvSize.x / 2, mvSize.y / 2), cVector2f(-mvSize.x / 2, -mvSize.y / 2)};

		pMesh = hplNew(cMesh2D, ());
		for (i = 0; i < 4; i++) {
			pMesh->AddVertex(vPos[i], 0, 1);
			pMesh->AddEdgeIndex(i);
		}
		for (i = 1; i < 4; i++)
			pMesh->AddIndex(i);
		for (i = 0; i < 3; i++)
			pMesh->AddIndex(i < 2 ? i : 3);

		return pMesh;
	}
	/// TRIANGLE ///////////////////////
	else if (sMeshName == "tri_1_to_1") {
		cVector2f vPos[3] = {cVector2f(mvSize.x / 2, -mvSize.y / 2), cVector2f(mvSize.x / 2, mvSize.y / 2), cVector2f(-mvSize.x / 2, mvSize.y / 2)};

		pMesh = hplNew(cMesh2D, ());
		for (i = 0; i < 3; i++) {
			pMesh->AddVertex(vPos[i], 0, 1);
			pMesh->AddEdgeIndex(i);
			pMesh->AddIndex(i);
		}

		return pMesh;
	}
	/// OTHER PRIMITIVE ///////////////////////
	else {
	}

	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

} // namespace hpl
