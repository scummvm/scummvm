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

#include "hpl1/penumbra-overture/GameForceArea.h"
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

cAreaLoader_GameForceArea::cAreaLoader_GameForceArea(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameForceArea::~cAreaLoader_GameForceArea() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameForceArea::Load(const tString &asName, const cVector3f &avSize,
										   const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameForceArea *pArea = hplNew(cGameForceArea, (mpInit, asName));

	pArea->m_mtxOnLoadTransform = a_mtxTransform;

	// Create physics data
	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();
	iCollideShape *pShape = pPhysicsWorld->CreateBoxShape(avSize, NULL);
	Common::Array<iPhysicsBody *> vBodies;
	vBodies.push_back(pPhysicsWorld->CreateBody(asName, pShape));

	vBodies[0]->SetCollide(false);
	vBodies[0]->SetCollideCharacter(false);
	vBodies[0]->SetMatrix(a_mtxTransform);

	vBodies[0]->SetUserData(pArea);
	pArea->SetBodies(vBodies);

	mpInit->mpMapHandler->AddGameEntity(pArea);

	pArea->Setup();

	// Return something else later perhaps.
	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

cGameForceArea::cGameForceArea(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_ForceArea;

	mfMaxForce = 1;
	mfConstant = 1;
	mfDestSpeed = 1;
	mfMaxMass = 0;

	mbMulWithMass = false;
	mbAffectCharacters = true;
	mbAffectBodies = true;
	mbForceAtPoint = false;
}

//-----------------------------------------------------------------------

cGameForceArea::~cGameForceArea(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameForceArea::Setup() {
	mvUp = cMath::MatrixInverse(mvBodies[0]->GetWorldMatrix()).GetUp();
	mvUp.Normalise();
}

//-----------------------------------------------------------------------

void cGameForceArea::Update(float afTimeStep) {
	iPhysicsBody *pAreaBody = mvBodies[0];
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();

	cCollideData collideData;
	collideData.SetMaxSize(4);

	// Log("---------- UPDATE -------------\n");

	////////////////////////////////////////////////////////
	// Iterate all bodies in world and check for intersection
	cPortalContainerEntityIterator bodyIt = pWorld->GetPortalContainer()->GetEntityIterator(
		pAreaBody->GetBoundingVolume());
	while (bodyIt.HasNext()) {
		iPhysicsBody *pBody = static_cast<iPhysicsBody *>(bodyIt.Next());

		// Log("Checking: %s\n",pBody->GetName().c_str());

		iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();

		if (pBody->GetCollide() && pBody->IsActive()) {
			if (pEntity) {
				if (pEntity->IsActive() == false)
					continue;
				if (pBody->GetMass() == 0 && pEntity->GetType() != eGameEntityType_Enemy)
					continue;
			} else if ((pBody->GetMass() == 0 && pBody->IsCharacter() == false) ||
					   pBody->GetMass() > mfMaxMass) {
				continue;
			}

			/////////////////////////
			// Bounding volume check
			if (cMath::CheckCollisionBV(*pBody->GetBV(), *pAreaBody->GetBV()) == false)
				continue;

			///////////////////////////////
			// Check for collision
			// int lCollideNum = mbForceAtPoint ? 4 : 1;

			if (pPhysicsWorld->CheckShapeCollision(pBody->GetShape(), pBody->GetLocalMatrix(),
												   pAreaBody->GetShape(), pAreaBody->GetLocalMatrix(), collideData, 1) == false) {
				continue;
			}

			///////////////////////////////
			// Add Character Force
			if (pBody->IsCharacter() && mbAffectCharacters) {
				iCharacterBody *pCharBody = pBody->GetCharacterBody();

				float fRelSpeed = cMath::Vector3Dot(mvUp, pCharBody->GetForceVelocity());
				float fDiff = mfDestSpeed - fRelSpeed;

				// If speed is above wanted, do nothing.
				if (fDiff <= 0)
					continue;

				float fForce = fDiff * mfConstant;
				if (mfMaxForce != 0 && fForce > mfMaxForce)
					fForce = mfMaxForce;
				if (mbMulWithMass)
					fForce *= pCharBody->GetMass();

				fForce *= 10; // Character needs to extra force.

				// Log("Vel: %s Speed: %f Force: %f\n",pCharBody->GetForceVelocity().ToString().c_str(),
				//									fRelSpeed,fForce);

				pCharBody->AddForce(mvUp * fForce);
			}
			///////////////////////////////
			// Add Body Force
			else if (mbAffectBodies) {
				cVector3f vPos = 0;
				if (mbForceAtPoint) {
					for (int i = 0; i < collideData.mlNumOfPoints; ++i) {
						vPos += collideData.mvContactPoints[i].mvPoint;
					}
					vPos = vPos / (float)collideData.mlNumOfPoints;
				}

				// Get relative speed
				cVector3f vVel = mbForceAtPoint ? pBody->GetVelocityAtPosition(vPos) : pBody->GetLinearVelocity();

				float fRelSpeed = cMath::Vector3Dot(mvUp, vVel);
				float fDiff = mfDestSpeed - fRelSpeed;

				// If speed is above wanted, do nothing.
				if (fDiff <= 0)
					continue;

				float fForce = fDiff * mfConstant;
				if (mfMaxForce != 0 && fForce > mfMaxForce)
					fForce = mfMaxForce;
				if (mbMulWithMass)
					fForce *= pBody->GetMass();

				if (mbForceAtPoint) {
					pBody->AddForceAtPosition(mvUp * fForce, vPos);
				} else {
					pBody->AddForce(mvUp * fForce);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void cGameForceArea::OnPostSceneDraw() {
	return;
	iLowLevelGraphics *pLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();

	mvBodies[0]->RenderDebugGeometry(pLowGfx, cColor(1, 1, 1, 1));
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

kBeginSerialize(cGameForceArea_SaveData, iGameEntity_SaveData)
	kSerializeVar(mvSize, eSerializeType_Vector3f)

		kSerializeVar(mfMaxForce, eSerializeType_Float32)
			kSerializeVar(mfConstant, eSerializeType_Float32)
				kSerializeVar(mfDestSpeed, eSerializeType_Float32)
					kSerializeVar(mfMaxMass, eSerializeType_Float32)

						kSerializeVar(mbMulWithMass, eSerializeType_Bool)
							kSerializeVar(mbAffectCharacters, eSerializeType_Bool)
								kSerializeVar(mbAffectBodies, eSerializeType_Bool)
									kSerializeVar(mbForceAtPoint, eSerializeType_Bool)
										kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameForceArea_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameForceArea::CreateSaveData() {
	return hplNew(cGameForceArea_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameForceArea::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameForceArea_SaveData *pData = static_cast<cGameForceArea_SaveData *>(apSaveData);

	kCopyToVar(pData, mfMaxForce);
	kCopyToVar(pData, mfConstant);
	kCopyToVar(pData, mfDestSpeed);
	kCopyToVar(pData, mfMaxMass);

	kCopyToVar(pData, mbMulWithMass);
	kCopyToVar(pData, mbAffectCharacters);
	kCopyToVar(pData, mbAffectBodies);
	kCopyToVar(pData, mbForceAtPoint);

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();
}

//-----------------------------------------------------------------------

void cGameForceArea::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameForceArea_SaveData *pData = static_cast<cGameForceArea_SaveData *>(apSaveData);

	kCopyFromVar(pData, mfMaxForce);
	kCopyFromVar(pData, mfConstant);
	kCopyFromVar(pData, mfDestSpeed);
	kCopyFromVar(pData, mfMaxMass);

	kCopyFromVar(pData, mbMulWithMass);
	kCopyFromVar(pData, mbAffectCharacters);
	kCopyFromVar(pData, mbAffectBodies);
	kCopyFromVar(pData, mbForceAtPoint);
}

//-----------------------------------------------------------------------

void cGameForceArea::SetupSaveData(iGameEntity_SaveData *apSaveData) {
	super::SetupSaveData(apSaveData);
}

//-----------------------------------------------------------------------
