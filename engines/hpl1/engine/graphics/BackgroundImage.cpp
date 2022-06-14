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

#include "hpl1/engine/graphics/BackgroundImage.h"
#include "hpl1/engine/math/Math.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cBackgroundImage::cBackgroundImage(iMaterial *apMat, const cVector3f &avPos, bool abTile,
								   const cVector2f &avSize, const cVector2f &avPosPercent, const cVector2f &avVel) {
	mpMaterial = apMat;

	mvPos = avPos;
	mbTile = abTile;
	mvSize = avSize;
	mvPosPercent = avPosPercent;
	mvVel = avVel;

	mvVtx = apMat->GetImage(eMaterialTexture_Diffuse)->GetVertexVecCopy(0, mvSize);
	for (int i = 0; i < (int)mvVtx.size(); i++) {
		mvVtx[i].pos.z = mvPos.z;
	}
}

//-----------------------------------------------------------------------

cBackgroundImage::~cBackgroundImage() {
	hplDelete(mpMaterial);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cBackgroundImage::Draw(const cRect2f &aCollideRect, iLowLevelGraphics *apLowLevelGraphics) {
	tVector3fList lstPositions;
	cVector3f vScreenPos(aCollideRect.x * mvPosPercent.x - mvPos.x,
						 aCollideRect.y * mvPosPercent.y, mvPos.z - mvPos.y);

	// Calulate at what positions(s) the background is to be drawn.
	if (mbTile) {
		// The number of images needed.
		cVector2l vNum;
		vNum.x = (int)(aCollideRect.w / mvSize.x) + 1;
		vNum.y = (int)(aCollideRect.h / mvSize.y) + 1;

		cVector2f vStartPos;
		if (vScreenPos.x >= 0)
			vStartPos.x = mvSize.x - cMath::Modulus(vScreenPos.x, mvSize.x);
		else
			vStartPos.x = cMath::Modulus(vScreenPos.x, mvSize.x);

		if (vScreenPos.y >= 0)
			vStartPos.y = mvSize.y - cMath::Modulus(vScreenPos.y, mvSize.y);
		else
			vStartPos.y = cMath::Modulus(vScreenPos.y, mvSize.y);

		// Log("Screen: %f : %f\n",vScreenPos.x, vScreenPos.y);
		// Log("Start: %f : %f\n",vStartPos.x, vStartPos.y);
		// Log("Pos: %f : %f\n",mvPos.x, mvPos.y);

		if (vStartPos.x > 0) {
			vStartPos.x -= mvSize.x;
			vNum.x++;
		}
		if (vStartPos.y > 0) {
			vStartPos.y -= mvSize.y;
			vNum.y++;
		}

		for (int x = 0; x < vNum.x; x++)
			for (int y = 0; y < vNum.y; y++) {
				lstPositions.push_back(cVector3f(vStartPos.x + mvSize.x * x,
												 vStartPos.y + mvSize.y * y, vScreenPos.z));
			}
	} else {
		cRect2f Rect(vScreenPos.x, vScreenPos.y, mvSize.x, mvSize.y);

		if (cMath::BoxCollision(aCollideRect, Rect)) {
			lstPositions.push_back(vScreenPos);
		}
	}

	// Draw the images
	mpMaterial->StartRendering(eMaterialRenderType_Diffuse, NULL, NULL);

	int lIdxAdd = 0;

	tVector3fListIt it = lstPositions.begin();
	for (; it != lstPositions.end(); it++) {
		mvVtx[0].pos.x = it->x;
		mvVtx[0].pos.y = it->y;
		apLowLevelGraphics->AddVertexToBatch(mvVtx[0]);

		mvVtx[1].pos.x = it->x + mvSize.x;
		mvVtx[1].pos.y = it->y;
		apLowLevelGraphics->AddVertexToBatch(mvVtx[1]);

		mvVtx[2].pos.x = it->x + mvSize.x;
		mvVtx[2].pos.y = it->y + mvSize.y;
		apLowLevelGraphics->AddVertexToBatch(mvVtx[2]);

		mvVtx[3].pos.x = it->x;
		mvVtx[3].pos.y = it->y + mvSize.y;
		apLowLevelGraphics->AddVertexToBatch(mvVtx[3]);

		apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 0);
		apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 1);
		apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 2);
		apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 3);

		/*apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 2);
		apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 3);
		apLowLevelGraphics->AddIndexToBatch(lIdxAdd + 0);*/

		lIdxAdd += 4;
	}

	do {
		apLowLevelGraphics->FlushQuadBatch(mpMaterial->GetBatchFlags(eMaterialRenderType_Diffuse), false);
	} while (mpMaterial->NextPass(eMaterialRenderType_Diffuse));

	apLowLevelGraphics->ClearBatch();

	mpMaterial->EndRendering(eMaterialRenderType_Diffuse);
}
//-----------------------------------------------------------------------

void cBackgroundImage::Update() {
	mvPos += mvVel;

	if (mbTile) {
		if (mvPos.x >= mvSize.x)
			mvPos.x = mvPos.x - mvSize.x;
		if (mvPos.y >= mvSize.y)
			mvPos.y = mvPos.y - mvSize.y;
	}
}

//-----------------------------------------------------------------------
} // namespace hpl
