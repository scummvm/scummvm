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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hpl1/engine/scene/TileMapLineIt.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cTileMapLineIt::cTileMapLineIt(cVector2f avStartPos, cVector2f avEndPos, cTileMap *apTileMap, int alLayer) {
	mpTileMap = apTileMap;
	mpTile = NULL;

	mvPos = avStartPos;

	mlLayer = alLayer;
	mlLayerCount = 0;
	mlCurrentLayer = 0;

	mbAtLastTile = false;
	mbAddNext = true;

	float fAngle = cMath::GetAngleFromPoints2D(avStartPos, avEndPos);
	float fDist = sqrt(mpTileMap->GetTileSize() * mpTileMap->GetTileSize());

	mvPosAdd = cMath::GetVectorFromAngle2D(fAngle, fDist);
	mvPos = avStartPos;

	// Get the current tile
	mvTilePos = cVector2l((int)floor(avStartPos.x / apTileMap->GetTileSize()),
						  (int)floor(avStartPos.y / apTileMap->GetTileSize()));
	mlTileNum = mvTilePos.x + mvTilePos.y * mpTileMap->mvSize.x;

	mvEndPos = cVector2l((int)floor(avEndPos.x / apTileMap->GetTileSize()),
						 (int)floor(avEndPos.y / apTileMap->GetTileSize()));

	if (mvEndPos == mvTilePos)
		mbAtLastTile = true;

	/*Log("Start: %d %d\n", mvTilePos.x,mvTilePos.y);
	Log("End: %d %d\n", mvEndPos.x,mvEndPos.y);
	Log("End: %f : %f\n",avEndPos.x,avEndPos.y);
	Log("Pos: %s\n",mvPos.ToString().c_str());
	Log("Add: %s\n",mvPosAdd.ToString().c_str());
	Log("Angle: %f\n\n",(fAngle/k2Pif)*360);
	Log("%f : %f\n", mvPosAdd.x / (avEndPos.x - avStartPos.x), mvPosAdd.y / (avEndPos.y - avStartPos.y));
	Log("-------------\n");*/

	/*Check if the tilepos is outside of the map*/
	if (mvTilePos.x < 0 || mvTilePos.y < 0 || mvTilePos.x >= mpTileMap->mvSize.x ||
		mvTilePos.y >= mpTileMap->mvSize.y) {
		mlLayerCount = (int)mpTileMap->mvTileLayer.size();
	}

	mbUpdated = false;
}

//-----------------------------------------------------------------------

cTileMapLineIt::~cTileMapLineIt() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cTileMapLineIt::HasNext() {
	GetTile();

	return mpTile != NULL;
}

//-----------------------------------------------------------------------

cTile *cTileMapLineIt::Next() {
	GetTile();

	mbUpdated = false;
	return mpTile;
}

//-----------------------------------------------------------------------

cTile *cTileMapLineIt::PeekNext() {
	GetTile();

	return mpTile;
}

//-----------------------------------------------------------------------

int cTileMapLineIt::GetNum() {
	return mlTileNum;
}

//-----------------------------------------------------------------------

int cTileMapLineIt::GetCurrentLayer() {
	return mlCurrentLayer;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cTileMapLineIt::GetTile() {
	if (mbUpdated)
		return;
	mbUpdated = true;

	// We are gonna check till we find a non NULL value or the end.
	while (true) {
		// Check if end of the this tile pos
		if ((mlLayer >= 0 && mlLayerCount > 0) || (mlLayer == -1 && mlLayerCount >= (int)mpTileMap->mvTileLayer.size())) {
			if (mbAtLastTile) {
				mpTile = NULL;
				break;
			}

			// add pos so we go to the next tile.
			if (mbAddNext) {
				mvPos += mvPosAdd;

				// Get the current tile
				cVector2l vLastTilePos = mvTilePos;
				mvTilePos = cVector2l((int)floor(mvPos.x / mpTileMap->GetTileSize()),
									  (int)floor(mvPos.y / mpTileMap->GetTileSize()));

				// if there has been a change on both x and y then I tile has been missed
				if (mvTilePos.x != vLastTilePos.x && mvTilePos.y != vLastTilePos.y) {
					cVector2l vAdd = mvTilePos - vLastTilePos;

					// Log("Too big jump!\n");
					cVector2f vIntersectX, vIntersectY;
					cVector2f vOldPos = mvPos - mvPosAdd;

					GetXYIntersection(vOldPos, &vIntersectX, &vIntersectY);

					if (cMath::SqrDist2D(vOldPos, vIntersectX) < cMath::SqrDist2D(vOldPos, vIntersectY))
						mvTilePos = cVector2l(vLastTilePos.x, vLastTilePos.y + vAdd.y);
					else
						mvTilePos = cVector2l(vLastTilePos.x + vAdd.x, vLastTilePos.y);

					mbAddNext = false;
				}
			} else {
				mbAddNext = true;
				mvTilePos = cVector2l((int)floor(mvPos.x / mpTileMap->GetTileSize()),
									  (int)floor(mvPos.y / mpTileMap->GetTileSize()));
			}

			/*Check if the tilepos is outside of the map*/
			if (mvTilePos.x < 0 || mvTilePos.y < 0 || mvTilePos.x >= mpTileMap->mvSize.x ||
				mvTilePos.y >= mpTileMap->mvSize.y) {
				mpTile = NULL;
				Error("Outside of bounds!\n");
				// should just not set mlLayer count to 0. SO that the start can be soutside of the map.
				break;
			} else {
				mlLayerCount = 0;
			}

			mlTileNum = mvTilePos.x + mvTilePos.y * mpTileMap->mvSize.x;

			// Log("Next: %d %d\n", mvTilePos.x,mvTilePos.y);
			// Log("Pos: %s\n",mvPos.ToString().c_str());

			if (mvTilePos == mvEndPos) {
				mbAtLastTile = true;
			}
		} else {
			if (mlLayer < 0) {
				mpTile = mpTileMap->mvTileLayer[mlLayerCount]->mvTile[mlTileNum];
				mlCurrentLayer = mlLayerCount;
			} else {
				mpTile = mpTileMap->mvTileLayer[mlLayer]->mvTile[mlTileNum];
				mlCurrentLayer = mlLayer;
			}

			mlLayerCount++;

			if (mpTile != NULL) {
				iTileData *pData = mpTile->GetTileData();
				if (pData && pData->IsSolid()) {
					mlLayerCount = (int)mpTileMap->mvTileLayer.size();
				}
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------

void cTileMapLineIt::GetXYIntersection(const cVector2f &avPosA,
									   cVector2f *avXIntersect, cVector2f *avYIntersect) {
	// Calculate DX
	float fDx;
	if (mvPosAdd.x > 0)
		fDx = ceil(avPosA.x / mpTileMap->GetTileSize()) * mpTileMap->GetTileSize();
	else
		fDx = floor(avPosA.x / mpTileMap->GetTileSize()) * mpTileMap->GetTileSize();
	fDx = fDx - avPosA.x;

	// Calculate DY
	float fDy;
	if (mvPosAdd.y > 0)
		fDy = ceil(avPosA.y / mpTileMap->GetTileSize()) * mpTileMap->GetTileSize();
	else
		fDy = floor(avPosA.y / mpTileMap->GetTileSize()) * mpTileMap->GetTileSize();
	fDy = fDy - avPosA.y;

	// Get Y Intersection
	float fDiv = mvPosAdd.x == 0 ? 0.00001f : mvPosAdd.x; // Handle div by 0
	float fInterY = (fDx / fDiv) * mvPosAdd.y;
	avYIntersect->x = avPosA.x + fDx;
	avYIntersect->y = avPosA.y + fInterY;

	// Get X Intersection
	fDiv = mvPosAdd.y == 0 ? 0.00001f : mvPosAdd.y; // Handle div by 0
	float fInterX = (fDy / fDiv) * mvPosAdd.x;
	avXIntersect->y = avPosA.y + fDy;
	avXIntersect->x = avPosA.x + fInterX;

	// Log("fDx: %0.2f  fDy: %0.2f\n",fDx,fDy);
	// Log("Intersections: X: %0.2f %0.2f  Y: %0.2f %0.2f\n",avXIntersect->x,avXIntersect->y,
	//			avYIntersect->x,avYIntersect->y);
}

//-----------------------------------------------------------------------

} // namespace hpl
