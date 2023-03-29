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

#include "hpl1/engine/physics/Collider2D.h"

#include "hpl1/engine/graphics/Mesh2d.h"
#include "hpl1/engine/math/Math.h"
#include "hpl1/engine/physics/Body2D.h"
#include "hpl1/engine/physics/CollideData2D.h"
#include "hpl1/engine/scene/GridMap2D.h"
#include "hpl1/engine/scene/TileMap.h"
#include "hpl1/engine/scene/World2D.h"
#include "hpl1/engine/system/low_level_system.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cCollider2D::cCollider2D() {
	mpWorld = NULL;
}

//-----------------------------------------------------------------------

cCollider2D::~cCollider2D() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

tFlag cCollider2D::CollideBody(cBody2D *apBody, cCollideData2D *apData) {
	tFlag lCollision = eFlagBit_None;

	cRect2f CollideRect = apBody->GetBoundingBox();

	cCollisionMesh2D *pCollMesh = apBody->GetCollisionMesh();

	cVector2f vPushVector;
	cVector2f vLastPushVector;

	/////// TEST COLLISION WITH TILES
	// float fTileSize = mpWorld->GetTileMap()->GetTileSize();
	// cRect2f TileRect = cRect2f(0, 0, fTileSize, fTileSize);

	for (int i = 0; i < mpWorld->GetTileMap()->GetTileLayerNum(); i++) {
		if (mpWorld->GetTileMap()->GetTileLayer(i)->HasCollision() == false)
			continue;

		iTileMapIt *pTileIt = mpWorld->GetTileMap()->GetRectIterator(CollideRect, i);

		while (pTileIt->HasNext()) {
			cTile *pTile = pTileIt->Next();
			// TileRect.x = pTile->GetPosition().x - fTileSize / 2;
			// TileRect.y = pTile->GetPosition().y - fTileSize / 2;

			// This can be used for material properties.
			// cTileDataNormal *pTData = static_cast<cTileDataNormal*>(pTile->GetTileData());

			if (pTile->GetCollisionMesh() == NULL)
				continue;

			if (Collide(pCollMesh, pTile->GetCollisionMesh(), vPushVector)) {
				if (apData)
					apData->mlstTiles.push_back(cCollidedTile(pTile, i));

				cVector3f vD;
				vD = apBody->GetPosition() - pTile->GetPosition();
				if ((vD.x * vPushVector.x + vD.y * vPushVector.y) < 0.0f)
					vPushVector = vPushVector * -1;

				cVector3f vPos = apBody->GetPosition();

				bool bAlterX = true;
				bool bAlterY = true;

				// reverse the latest push, maybe pos here instead?
				if (lCollision) {
					// If the current X push is grater use it instead.
					if (ABS(vPushVector.x) > ABS(vLastPushVector.x)) {
						vPos.x -= vLastPushVector.x;
					} else {
						bAlterX = false;
					}
					// If the current Y push is grater use it instead.
					if (ABS(vPushVector.y) > ABS(vLastPushVector.y)) {
						vPos.y -= vLastPushVector.y;
						bAlterY = true;
					} else {
						bAlterY = false;
					}
				}

				if (bAlterX)
					vPos.x += vPushVector.x;
				if (bAlterY)
					vPos.y += vPushVector.y;

				apBody->SetPosition(vPos);
				apBody->ResetLastPosition();

				apBody->UpdateCollisionMesh();
				// not really needed untill layer change
				CollideRect = apBody->GetBoundingBox();

				lCollision |= eFlagBit_0;
				vLastPushVector = vPushVector;

				// break;
			}
		}

		hplDelete(pTileIt);

		// if(bCollision)break;
	}

	/////// TEST COLLISION WITH BODIES
	iGridMap2DIt *pBodyIt = mpWorld->GetGridMapBodies()->GetRectIterator(CollideRect);

	while (pBodyIt->HasNext()) {
		cBody2D *pBody = static_cast<cBody2D *>(pBodyIt->Next());

		if (apBody == pBody)
			continue;

		// eFlagBit_0 is probably just temporary.
		if (pBody->IsActive() && pBody->GetCollideType() & apBody->GetCollideFlag()) {
			pBody->UpdateCollisionMesh(); // Temp

			if (Collide(pCollMesh, pBody->GetCollisionMesh(), vPushVector)) {
				if (apData)
					apData->mlstBodies.push_back(pBody);

				cVector3f vD;
				vD = apBody->GetPosition() - pBody->GetPosition();
				if ((vD.x * vPushVector.x + vD.y * vPushVector.y) < 0.0f)
					vPushVector = vPushVector * -1;

				cVector3f vPos = apBody->GetPosition();

				vPos += vPushVector;
				apBody->SetPosition(vPos);
				apBody->ResetLastPosition();

				// apBody->UpdateCollisionMesh();
				// not really needed untill layer change
				CollideRect = apBody->GetBoundingBox();

				lCollision |= eFlagBit_0;

				break;
			}
		}
	}

	hplDelete(pBodyIt);

	/// Do some stuff when colliding
	if (lCollision) {
		/*cVector3f vPos = apBody->GetPosition() + vPushVector;
		apBody->SetPosition(vPos);
		apBody->ResetLastPosition();*/

		mDebug.mvPushVec = vPushVector;
		// mDebug.mvPushPos = cVector2f(apBody->GetPosition().x,apBody->GetPosition().y);
	}

	return lCollision;
}

//-----------------------------------------------------------------------

tFlag cCollider2D::CollideRect(cRect2f &aRect, tFlag alCollideFlags, cCollideData2D *apData) {
	tFlag lCollision = eFlagBit_None;

	cRect2f CollideRect = aRect;

	cCollisionMesh2D *pCollMesh = hplNew(cCollisionMesh2D, ());
	pCollMesh->mvPos.resize(4);
	pCollMesh->mvNormal.resize(4);
	SetCollideMesh(pCollMesh, aRect);

	cVector2f vPushVector;
	cVector2f vLastPushVector;

	//// Check for all tiles if the flag is set
	if (alCollideFlags & eFlagBit_0) {
		// float fTileSize = mpWorld->GetTileMap()->GetTileSize();
		// cRect2f TileRect = cRect2f(0, 0, fTileSize, fTileSize);

		for (int i = 0; i < mpWorld->GetTileMap()->GetTileLayerNum(); i++) {
			if (mpWorld->GetTileMap()->GetTileLayer(i)->HasCollision() == false)
				continue;

			iTileMapIt *pTileIt = mpWorld->GetTileMap()->GetRectIterator(CollideRect, i);

			while (pTileIt->HasNext()) {
				cTile *pTile = pTileIt->Next();
				// TileRect.x = pTile->GetPosition().x - fTileSize / 2;
				// TileRect.y = pTile->GetPosition().y - fTileSize / 2;

				if (pTile->GetCollisionMesh() == NULL)
					continue;

				if (apData)
					apData->mlstTiles.push_back(cCollidedTile(pTile, i));

				if (Collide(pCollMesh, pTile->GetCollisionMesh(), vPushVector)) {
					cVector3f vD;
					vD = cVector3f(aRect.x, aRect.y, 0) - pTile->GetPosition();
					if ((vD.x * vPushVector.x + vD.y * vPushVector.y) < 0.0f)
						vPushVector = vPushVector * -1;

					cVector3f vPos;
					vPos.x = aRect.x;
					vPos.y = aRect.y;

					// reverse the latest push, maybe pos here instead?
					if (lCollision) {
						vPos -= vLastPushVector;
					}

					vPos += vPushVector;
					aRect.x = vPos.x;
					aRect.y = vPos.y;

					SetCollideMesh(pCollMesh, aRect);
					CollideRect = aRect;

					lCollision |= eFlagBit_0;
					vLastPushVector = vPushVector;
				}
			}

			hplDelete(pTileIt);
		}
	}

	iGridMap2DIt *pBodyIt = mpWorld->GetGridMapBodies()->GetRectIterator(CollideRect);

	while (pBodyIt->HasNext()) {
		cBody2D *pBody = static_cast<cBody2D *>(pBodyIt->Next());

		if (pBody->IsActive() && pBody->GetCollideType() & alCollideFlags) {
			if (cMath::BoxCollision(CollideRect, pBody->GetBoundingBox())) {
				if (Collide(pCollMesh, pBody->GetCollisionMesh(), vPushVector)) {
					if (apData)
						apData->mlstBodies.push_back(pBody);

					lCollision |= pBody->GetCollideType();

					/*Perhaps fix the push vector here?*/
				}
			}
		}
	}

	hplDelete(pBodyIt);

	/// Do some stuff when colliding
	if (lCollision) {
		if (apData)
			apData->mvPushVec = vPushVector;
	}

	hplDelete(pCollMesh);

	return lCollision;
}

//-----------------------------------------------------------------------

tFlag cCollider2D::CollideLine(const cVector2f &avStart, const cVector2f &avEnd, tFlag alCollideFlags,
							   cCollideData2D *apData) {
	tFlag lCollision = eFlagBit_None;

	//// Check for all tiles if the flag is set
	if (alCollideFlags & eFlagBit_0) {
		/*float fTileSize = */ mpWorld->GetTileMap()->GetTileSize();

		for (int i = 0; i < mpWorld->GetTileMap()->GetTileLayerNum(); i++) {
			if (mpWorld->GetTileMap()->GetTileLayer(i)->HasCollision() == false)
				continue;

			iTileMapIt *pTileIt = mpWorld->GetTileMap()->GetLineIterator(avStart, avEnd, i);

			while (pTileIt->HasNext()) {
				cTile *pTile = pTileIt->Next();

				if (pTile->GetCollisionMesh() == NULL)
					continue;
				// Log("Found tile!\n");

				if (apData)
					apData->mlstTiles.push_back(cCollidedTile(pTile, i));

				lCollision |= eFlagBit_0;
			}

			hplDelete(pTileIt);
		}
	}

	// hplDelete(pCollMesh);

	return lCollision;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cCollider2D::SetCollideMesh(cCollisionMesh2D *apMesh, cRect2f &aRect) {
	apMesh->mvPos[0] = cVector2f(aRect.x, aRect.y);
	apMesh->mvPos[1] = cVector2f(aRect.x + aRect.w, aRect.y);
	apMesh->mvPos[2] = cVector2f(aRect.x + aRect.w, aRect.y + aRect.h);
	apMesh->mvPos[3] = cVector2f(aRect.x, aRect.y + aRect.w);

	apMesh->mvNormal[0] = cVector2f(0, -1);
	apMesh->mvNormal[1] = cVector2f(1, 0);
	apMesh->mvNormal[2] = cVector2f(0, 1);
	apMesh->mvNormal[3] = cVector2f(-1, 0);
}

//-----------------------------------------------------------------------

cVector2f FindMTD(cVector2f *pPushVector, int alNumVectors) {
	cVector2f MTD = pPushVector[0];
	float mind2 = pPushVector[0].x * pPushVector[0].x + pPushVector[0].y * pPushVector[0].y;
	for (int i = 1; i < alNumVectors; i++) {
		float fD2 = pPushVector[i].x * pPushVector[i].x + pPushVector[i].y * pPushVector[i].y;
		if (fD2 < mind2) {
			mind2 = fD2;
			MTD = pPushVector[i];
		}
	}
	return MTD;
}

//-----------------------------------------------------------------------

bool cCollider2D::Collide(cCollisionMesh2D *apMeshA, cCollisionMesh2D *apMeshB, cVector2f &avMTD) {
	cVector2f vAxis[32];
	int lAxisNum = 0;

	// Check separating planes for A
	for (int i = 0; i < (int)apMeshA->mvNormal.size(); i++) {
		vAxis[lAxisNum] = apMeshA->mvNormal[i];
		if (AxisSeparateMeshes(vAxis[lAxisNum], apMeshA, apMeshB)) {
			return false;
		}
		lAxisNum++;
	}

	// Check separating planes for B
	for (int i = 0; i < (int)apMeshB->mvNormal.size(); i++) {
		vAxis[lAxisNum] = apMeshB->mvNormal[i];
		if (AxisSeparateMeshes(vAxis[lAxisNum], apMeshA, apMeshB)) {
			return false;
		}
		lAxisNum++;
	}

	avMTD = FindMTD(vAxis, lAxisNum);

	return true;
}
//-----------------------------------------------------------------------

bool cCollider2D::AxisSeparateMeshes(cVector2f &avAxis, cCollisionMesh2D *apMeshA,
									 cCollisionMesh2D *apMeshB) {
	float fMinA, fMaxA;
	float fMinB, fMaxB;

	CalculateInterval(avAxis, apMeshA, fMinA, fMaxA);
	CalculateInterval(avAxis, apMeshB, fMinB, fMaxB);

	if (fMinA >= fMaxB || fMinB >= fMaxA)
		return true;

	float fD0 = fMaxA - fMinB;
	float fD1 = fMaxB - fMinA;
	float fDepth = (fD0 < fD1) ? fD0 : fD1;

	avAxis *= fDepth;

	return false;
}

//-----------------------------------------------------------------------

void cCollider2D::CalculateInterval(const cVector2f &avAxis, cCollisionMesh2D *apMesh,
									float &afMin, float &afMax) {
	float fTemp = avAxis.x * apMesh->mvPos[0].x + avAxis.y * apMesh->mvPos[0].y;
	afMin = fTemp;
	afMax = fTemp;

	for (int i = 1; i < (int)apMesh->mvPos.size(); i++) {
		fTemp = avAxis.x * apMesh->mvPos[i].x + avAxis.y * apMesh->mvPos[i].y;

		if (fTemp < afMin) {
			afMin = fTemp;
		} else if (fTemp > afMax) {
			afMax = fTemp;
		}
	}
}

//-----------------------------------------------------------------------
} // namespace hpl
