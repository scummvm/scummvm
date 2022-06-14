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

#include "hpl1/engine/scene/TileMap.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/graphics/RenderObject2D.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/resources/ResourceImage.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/TileSetManager.h"
#include "hpl1/engine/scene/TileMapLineIt.h"
#include "hpl1/engine/scene/TileMapRectIt.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTileMap::cTileMap(cVector2l avSize, float afTileSize, cGraphics *apGraphics, cResources *apResources) {
	mfTileSize = 1;
	mvSize = avSize;
	mfTileSize = afTileSize;
	mpGraphics = apGraphics;
	mpResources = apResources;
	mlShadowLayer = 0;
}

//-----------------------------------------------------------------------

cTileMap::~cTileMap() {
	Log(" Deleting tilemap.\n");
	for (tTileLayerVecIt it = mvTileLayer.begin(); it != mvTileLayer.end(); it++)
		hplDelete(*it);
	for (tTileSetVecIt it = mvTileSet.begin(); it != mvTileSet.end(); it++)
		mpResources->GetTileSetManager()->Destroy(*it);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cTileMap::Render(cCamera2D *apCam) {
	cRect2f Rect;
	apCam->GetClipRect(Rect);

	iTileMapIt *TileIt = GetRectIterator(Rect, -1);

	while (TileIt->HasNext()) {
		RenderTileData(TileIt->Next(), TileIt->GetCurrentLayer());
	}

	hplDelete(TileIt);
}

//-----------------------------------------------------------------------

iTileMapIt *cTileMap::GetRectIterator(const cRect2f &aRect, int alLayer) {
	cVector2l vPos = cVector2l((int)floor(aRect.x / mfTileSize),
							   (int)floor(aRect.y / mfTileSize));

	cVector2l vSize = cVector2l((int)(aRect.w / mfTileSize) + 1,
								(int)(aRect.h / mfTileSize) + 1);

	// Check if we need yet another grid for x and y
	if (aRect.x + aRect.w >= (vPos.x + vSize.x) * mfTileSize)
		vSize.x++;
	if (aRect.y + aRect.h >= (vPos.y + vSize.y) * mfTileSize)
		vSize.y++;

	// Log("\nPos: %d:%d\n",vPos.x,vPos.y);
	// Log("Size: %d:%d\n\n",vSize.x,vSize.y);

	return hplNew(cTileMapRectIt, (vPos, vSize, this, alLayer));
}

//-----------------------------------------------------------------------

iTileMapIt *cTileMap::GetLineIterator(const cVector2f &avStart, const cVector2f &avEnd, int alLayer) {
	return hplNew(cTileMapLineIt, (avStart, avEnd, this, alLayer));
}

//-----------------------------------------------------------------------

cVector2f cTileMap::GetWorldPos(cVector2f avScreenPos, cCamera2D *apCam) {
	cVector2f vWorldPos;
	cRect2f Rect;
	apCam->GetClipRect(Rect);
	cVector2f vVirtSize = mpGraphics->GetLowLevel()->GetVirtualSize();

	vWorldPos.x = Rect.x + Rect.w * (avScreenPos.x / ((float)vVirtSize.x));
	vWorldPos.y = Rect.y + Rect.h * (avScreenPos.y / ((float)vVirtSize.y));

	return vWorldPos;
}

//-----------------------------------------------------------------------

cTile *cTileMap::GetScreenTile(cVector2f avPos, int alLayer, cCamera2D *apCam) {
	return GetWorldTile(GetWorldPos(avPos, apCam), alLayer);
}

//-----------------------------------------------------------------------

void cTileMap::SetScreenTileData(cVector2f avPos, int alLayer, cCamera2D *apCam,
								 int alTileSet, int alTileNum) {
	cVector2f vWorldPos = GetWorldPos(avPos, apCam);

	cTile *pOldTile = GetWorldTile(vWorldPos, alLayer);

	iTileData *pData = NULL;
	if (alTileSet >= 0) {
		cTileSet *pSet = GetTileSet(alTileSet);
		if (pSet == NULL)
			return;
		pData = pSet->Get(alTileNum);
		if (pData == NULL)
			return;
	}

	cVector2l vTilePos = cVector2l((int)floor(vWorldPos.x / mfTileSize), (int)floor(vWorldPos.y / mfTileSize));
	if (vTilePos.x < 0 || vTilePos.y < 0 || vTilePos.x >= mvSize.x || vTilePos.y >= mvSize.y)
		return;

	if (alLayer < 0 || alLayer >= (int)mvTileLayer.size())
		return;

	cVector3f vTileWorldPos(vTilePos.x * mfTileSize, vTilePos.y * mfTileSize, mvTileLayer[alLayer]->GetZ());

	int lAngle = 0;
	if (pOldTile)
		lAngle = pOldTile->GetAngle();

	cTile *pTile = NULL;
	if (pData)
		pTile = hplNew(cTile, (pData, (eTileRotation)lAngle, vTileWorldPos, mfTileSize, NULL));

	mvTileLayer[alLayer]->SetTile(vTilePos.x, vTilePos.y, pTile);
}

//-----------------------------------------------------------------------

void cTileMap::SetScreenTileAngle(cVector2f avPos, int alLayer, cCamera2D *apCam, int alAngle) {
	cVector2f vWorldPos = GetWorldPos(avPos, apCam);
	cTile *pTile = GetWorldTile(vWorldPos, alLayer);
	if (pTile == NULL)
		return;

	cVector2l vTilePos = cVector2l((int)floor(vWorldPos.x / mfTileSize), (int)floor(vWorldPos.y / mfTileSize));
	if (vTilePos.x < 0 || vTilePos.y < 0 || vTilePos.x >= mvSize.x || vTilePos.y >= mvSize.y)
		return;

	cVector3f vTileWorldPos(vTilePos.x * mfTileSize, vTilePos.y * mfTileSize, mvTileLayer[alLayer]->GetZ());

	pTile->SetAngle((eTileRotation)alAngle);
}

//-----------------------------------------------------------------------

cTile *cTileMap::GetWorldTile(cVector2f avPos, int alLayer) {
	if (alLayer < 0 || alLayer >= (int)mvTileLayer.size())
		return NULL;

	return mvTileLayer[alLayer]->GetAt((int)floor(avPos.x / mfTileSize),
									   (int)floor(avPos.y / mfTileSize));
}

//-----------------------------------------------------------------------

/**
 *
 * \param alTileNum
 * \param alLayer
 * \param avDir 0=left, 1=right, 2=up, 3 = down
 * \return num of neighbours
 */
int cTileMap::GetTileNeighbours4Dir(int alTileNum, int alLayer, bool *avDir) {
	int lCount = 0;
	cTileLayer *pLayer = mvTileLayer[alLayer];
	int lX = alTileNum % mvSize.x;
	int lY = alTileNum / mvSize.x;

	/*avDir[0] = pLayer->GetAt(lX-1,lY)!=NULL?true:false;
	avDir[1] = pLayer->GetAt(lX+1,lY)!=NULL?true:false;
	avDir[2] = pLayer->GetAt(lX,lY-1)!=NULL?true:false;
	avDir[3] = pLayer->GetAt(lX,lY+1)!=NULL?true:false;*/

	cTile *pTile;
	cTileDataNormal *pData;

	cVector2l vAdd[4] = {cVector2l(-1, 0), cVector2l(+1, 0), cVector2l(0, -1), cVector2l(0, +1)};

	for (int i = 0; i < 4; i++) {
		pTile = pLayer->GetAt(lX + vAdd[i].x, lY + vAdd[i].y);
		if (pTile != NULL) {
			pData = static_cast<cTileDataNormal *>(pTile->GetTileData());
			if (pData->IsSolid())
				avDir[i] = true;
			else
				avDir[i] = false;
		} else {
			avDir[i] = false;
		}
	}

	for (int i = 0; i < 4; i++)
		if (avDir[i] == true)
			lCount++;

	return lCount;
}

//-----------------------------------------------------------------------

void cTileMap::AddTileLayerFront(cTileLayer *apLayer) {
	if (mvTileLayer.size() < 1) {
		mvTileLayer.push_back(apLayer);
	} else {
		mvTileLayer.push_back(NULL);
		for (int i = (int)mvTileLayer.size() - 2; i >= 0; i--) {
			mvTileLayer[i + 1] = mvTileLayer[i];
		}
		mvTileLayer[0] = apLayer;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cTileMap::RenderTileData(cTile *apTile, int alLayer) {
	static int count = 0;

	cTileDataNormal *pData = static_cast<cTileDataNormal *>(apTile->GetTileData());
	if (pData == NULL)
		return;
	cRect2f _obj = cRect2f(apTile->GetPosition().x - mfTileSize / 2,
						   apTile->GetPosition().y - mfTileSize / 2, mfTileSize, mfTileSize);
	cRenderObject2D _obj2 = cRenderObject2D(pData->GetMaterial(),
											pData->GetVertexVec(apTile->GetAngle()),
											pData->GetIndexVec(apTile->GetAngle()),
											ePrimitiveType_Quad, apTile->GetPosition().z,
											_obj,
											NULL, apTile->GetPositionPtr());
	mpGraphics->GetRenderer2D()->AddObject(_obj2);

	count++;
}

//-----------------------------------------------------------------------

} // namespace hpl
