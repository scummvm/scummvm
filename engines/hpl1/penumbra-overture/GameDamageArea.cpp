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
 * This file is part of Penumbra Overture.
 */

#include "hpl1/penumbra-overture/GameDamageArea.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerState.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAreaLoader_GameDamageArea::cAreaLoader_GameDamageArea(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameDamageArea::~cAreaLoader_GameDamageArea() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameDamageArea::Load(const tString &asName, const cVector3f &avSize,
											const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameDamageArea *pArea = hplNew(cGameDamageArea, (mpInit, asName));

	pArea->m_mtxOnLoadTransform = a_mtxTransform;

	// Create physics data
	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();
	iCollideShape *pShape = pPhysicsWorld->CreateBoxShape(avSize, NULL);
	std::vector<iPhysicsBody *> vBodies;
	vBodies.push_back(pPhysicsWorld->CreateBody(asName, pShape));

	vBodies[0]->SetCollide(false);
	vBodies[0]->SetCollideCharacter(false);
	vBodies[0]->SetMatrix(a_mtxTransform);

	vBodies[0]->SetUserData(pArea);
	pArea->SetBodies(vBodies);

	mpInit->mpMapHandler->AddGameEntity(pArea);

	// Return something else later perhaps.
	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

cGameDamageArea::cGameDamageArea(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_DamageArea;

	mfDamage = 1;
	mfUpdatesPerSec = 1;
	mlStrength = 1;
	mbDisableObjects = false;
	mbDisableEnemies = false;

	mbHasInteraction = false;

	mfUpdateCount = 0;
}

//-----------------------------------------------------------------------

cGameDamageArea::~cGameDamageArea(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameDamageArea::OnPlayerPick() {
}

//-----------------------------------------------------------------------

void cGameDamageArea::Update(float afTimeStep) {
	if (mfUpdateCount <= 0) {
		mfUpdateCount = 1 / mfUpdatesPerSec;

		iPhysicsBody *pAreaBody = mvBodies[0];
		cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
		iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();

		cCollideData collideData;
		collideData.SetMaxSize(1);

		// Get bodies and add to list, this incase the portal contaniner gets changed.
		std::list<iPhysicsBody *> lstBodies;
		cPortalContainerEntityIterator bodyIt = pWorld->GetPortalContainer()->GetEntityIterator(
			pAreaBody->GetBoundingVolume());
		while (bodyIt.HasNext()) {
			iPhysicsBody *pBody = static_cast<iPhysicsBody *>(bodyIt.Next());
			lstBodies.push_back(pBody);
		}

		////////////////////////////////////////////////////////
		// Iterate all bodies in world and check for intersection
		std::list<iPhysicsBody *>::iterator it = lstBodies.begin();
		for (; it != lstBodies.end(); ++it) {
			iPhysicsBody *pBody = *it;
			iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();

			if (pBody->GetCollide() && pBody->IsActive()) {
				if (pEntity) {
					if (pEntity->GetHealth() <= 0 || pEntity->IsActive() == false)
						continue;
				} else if (mpInit->mpPlayer->GetCharacterBody()->GetBody() != pBody) {
					continue;
				}

				/////////////////////////
				// Bounding volume check
				if (cMath::CheckCollisionBV(*pBody->GetBV(), *pAreaBody->GetBV()) == false)
					continue;

				///////////////////////////////
				// Check for collision
				if (pPhysicsWorld->CheckShapeCollision(pBody->GetShape(), pBody->GetLocalMatrix(),
													   pAreaBody->GetShape(), pAreaBody->GetLocalMatrix(), collideData, 1) == false) {
					continue;
				}

				////////////////////
				// Player
				if (mpInit->mpPlayer->GetCharacterBody()->GetBody() == pBody) {
					mpInit->mpPlayer->Damage(mfDamage, ePlayerDamageType_BloodSplash);
				} else if (pEntity) {
					if ((pEntity->GetType() == eGameEntityType_Object && mbDisableObjects) ||
						(pEntity->GetType() == eGameEntityType_Enemy && mbDisableEnemies)) {
						if (pEntity->GetType() == eGameEntityType_Enemy &&
							pEntity->GetSubType() == "Worm") {
							return;
						}

						pEntity->SetActive(false);
					} else {
						pEntity->Damage(mfDamage, mlStrength);
					}
				}
			}
		}
	} else {
		mfUpdateCount -= afTimeStep;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cGameDamageArea_SaveData, iGameEntity_SaveData)
	kSerializeVar(mvSize, eSerializeType_Vector3f)
		kSerializeVar(mfDamage, eSerializeType_Float32)
			kSerializeVar(mfUpdatesPerSec, eSerializeType_Float32)
				kSerializeVar(mlStrength, eSerializeType_Int32)
					kSerializeVar(mbDisableObjects, eSerializeType_Bool)
						kSerializeVar(mbDisableEnemies, eSerializeType_Bool)
							kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameDamageArea_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameDamageArea::CreateSaveData() {
	return hplNew(cGameDamageArea_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameDamageArea::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameDamageArea_SaveData *pData = static_cast<cGameDamageArea_SaveData *>(apSaveData);

	kCopyToVar(pData, mfDamage);
	kCopyToVar(pData, mfUpdatesPerSec);
	kCopyToVar(pData, mlStrength);
	kCopyToVar(pData, mbDisableObjects);
	kCopyToVar(pData, mbDisableEnemies);

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();
}

//-----------------------------------------------------------------------

void cGameDamageArea::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameDamageArea_SaveData *pData = static_cast<cGameDamageArea_SaveData *>(apSaveData);

	kCopyFromVar(pData, mfDamage);
	kCopyFromVar(pData, mfUpdatesPerSec);
	kCopyFromVar(pData, mlStrength);
	kCopyFromVar(pData, mbDisableObjects);
	kCopyFromVar(pData, mbDisableEnemies);
}

//-----------------------------------------------------------------------

void cGameDamageArea::SetupSaveData(iGameEntity_SaveData *apSaveData) {
	super::SetupSaveData(apSaveData);
}
//-----------------------------------------------------------------------
