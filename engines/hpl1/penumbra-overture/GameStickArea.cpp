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

#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerState.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAreaLoader_GameStickArea::cAreaLoader_GameStickArea(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameStickArea::~cAreaLoader_GameStickArea() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameStickArea::Load(const tString &asName, const cVector3f &avSize,
										   const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameStickArea *pArea = hplNew(cGameStickArea, (mpInit, asName));

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
	mpInit->mpMapHandler->AddStickArea(pArea);

	// Return something else later perhaps.
	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

bool cGameStickArea::mbAllowAttachment = false;

//-----------------------------------------------------------------------

cGameStickArea::cGameStickArea(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_StickArea;

	mpAttachedBody = NULL;
	mpLastAttachedBody = NULL;

	msAttachFunction = "";
	msDetachFunction = "";

	msAttachSound = "";
	msDetachFunction = "";

	msAttachSound = "";
	msDetachSound = "";

	mbMoveBody = true;
	mbRotateBody = true;

	mbCheckCenterInArea = true;

	mfPoseTime = 0.2f;

	mbCanDeatch = true;

	mfSetMtxTime = 1.0f;
}

//-----------------------------------------------------------------------

cGameStickArea::~cGameStickArea(void) {
	mpInit->mpMapHandler->RemoveStickArea(this);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameStickArea::OnPlayerPick() {
}

//-----------------------------------------------------------------------

void cGameStickArea::Update(float afTimeStep) {
	iPhysicsBody *pAreaBody = mvBodies[0];
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	if (mpAttachedBody && mfSetMtxTime < 1) {
		if (mbMoveBody && mbRotateBody) {
			if (mfPoseTime == 0)
				mfSetMtxTime = 1.0f;
			else
				mfSetMtxTime += afTimeStep / mfPoseTime;

			cMatrixf mtxGoal = pAreaBody->GetWorldMatrix();
			mtxGoal.SetTranslation(pAreaBody->GetWorldPosition() - mpAttachedBody->GetMassCentre());

			cMatrixf mtxNew = cMath::MatrixSlerp(mfSetMtxTime, mtxAttachedStart, mtxGoal, true);

			mpAttachedBody->SetMatrix(mtxNew);
		} else if (mbMoveBody && !mbRotateBody) {
			if (mfPoseTime == 0)
				mfSetMtxTime = 1.0f;
			else
				mfSetMtxTime += afTimeStep / mfPoseTime;

			cVector3f vGoal = pAreaBody->GetWorldPosition() - mpAttachedBody->GetMassCentre();

			cVector3f vNew = mtxAttachedStart.GetTranslation() * (1 - mfSetMtxTime) +
							 vGoal * mfSetMtxTime;

			mpAttachedBody->SetPosition(vNew);
		} else if (!mbMoveBody && mbRotateBody) {
			if (mfPoseTime == 0)
				mfSetMtxTime = 1.0f;
			else
				mfSetMtxTime += afTimeStep / mfPoseTime;

			cMatrixf mtxGoal = pAreaBody->GetWorldMatrix();

			cMatrixf mtxNew = cMath::MatrixSlerp(mfSetMtxTime, mtxAttachedStart, mtxGoal, true);

			mtxNew.SetTranslation(mpAttachedBody->GetWorldPosition());
			mpAttachedBody->SetMatrix(mtxNew);
		} else {
			mfSetMtxTime = 1.0f;
		}

		if (mfSetMtxTime >= 1) {
			// Sound
			if (msAttachSound != "") {
				cSoundEntity *pSound = pWorld->CreateSoundEntity("AttachSound", msAttachSound, true);
				if (pSound)
					pSound->SetPosition(pAreaBody->GetWorldPosition());
			}

			// Particle System
			if (msAttachPS != "") {
				pWorld->CreateParticleSystem("AttachPS", msAttachPS, 1, pAreaBody->GetWorldMatrix());
			}
		}
	}

	if (mpAttachedBody)
		return;

	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	iPhysicsBody *pAttachBody = NULL;

	////////////////////////////////////////////////////////
	// Iterate all bodies in world and check for intersection
	cPhysicsBodyIterator bodyIt = pPhysicsWorld->GetBodyIterator();
	while (bodyIt.HasNext()) {
		iPhysicsBody *pBody = bodyIt.Next();

		/*if(	mpInit->mpPlayer->GetPickedBody() == pBody &&
			(	mpInit->mpPlayer->GetState() == ePlayerState_Grab ||
				mpInit->mpPlayer->GetState() == ePlayerState_Move ||
				mpInit->mpPlayer->GetState() == ePlayerState_Push)
		  )
		{
			continue;
		}*/

		if (pBody->IsActive() && pBody->GetCollide() && pBody->GetMass() > 0 &&
			pBody->IsCharacter() == false) {
			// Bounding volume check
			// if(cMath::CheckCollisionBV(*pBody->GetBV(), *pAreaBody->GetBV())==false) continue;

			bool bCheck = false;
			if (mbCheckCenterInArea) {
				cVector3f vPos = pBody->GetLocalPosition() +
								 cMath::MatrixMul(pBody->GetLocalMatrix().GetRotation(),
												  pBody->GetMassCentre());
				bCheck = cMath::PointBVCollision(vPos, *pAreaBody->GetBV());
			} else {
				bCheck = cMath::CheckCollisionBV(*pBody->GetBV(), *pAreaBody->GetBV());
			}

			if (bCheck == false) {
				if (pBody == mpLastAttachedBody) {
					mpLastAttachedBody = NULL;
				}
				continue;
			}

			if (pBody == mpLastAttachedBody)
				continue;

			// Shape collision check.
			cCollideData collideData;
			collideData.SetMaxSize(1);
			bool bCollide = pPhysicsWorld->CheckShapeCollision(pAreaBody->GetShape(),
															   pAreaBody->GetLocalMatrix(),
															   pBody->GetShape(),
															   pBody->GetLocalMatrix(),
															   collideData, 1);
			if (bCollide) {
				if (msAttachFunction != "") {
					mbAllowAttachment = false;
					tString sCommand = GetCallbackFunc(msAttachFunction, pBody);
					mpInit->RunScriptCommand(sCommand);

					if (mbAllowAttachment == false)
						continue;
				}

				pAttachBody = pBody;
			}
		}
	}

	////////////////////////////////////////////////////////
	// Do some stuff with the body to be attached.
	if (pAttachBody) {
		// Log("Attaching body %s\n", pAttachBody->GetName().c_str());

		/////////////////////////
		// If in an interact state, set the previous state
		if (mpInit->mpPlayer->GetPushBody() == pAttachBody &&
			(mpInit->mpPlayer->GetState() == ePlayerState_Grab ||
			 mpInit->mpPlayer->GetState() == ePlayerState_Move ||
			 mpInit->mpPlayer->GetState() == ePlayerState_Push)) {
			// Log(" Setting a prev state\n");

			ePlayerState state = mpInit->mpPlayer->GetState();
			ePlayerState prev = mpInit->mpPlayer->GetStateData(state)->mPreviuosState;
			if (prev == ePlayerState_InteractMode)
				mpInit->mpPlayer->ChangeState(ePlayerState_InteractMode);
			else
				mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
		}

		/////////////////////////
		// Snap it into place.
		// pAttachBody->SetMatrix(pAreaBody->GetWorldMatrix());
		// pAttachBody->SetPosition(pAreaBody->GetWorldPosition() - pAttachBody->GetMassCentre());
		mtxAttachedStart = pAttachBody->GetLocalMatrix();

		mfSetMtxTime = 0;

		pAttachBody->SetLinearVelocity(0);
		pAttachBody->SetAngularVelocity(0);

		mbBodyGravity = pAttachBody->GetGravity();
		pAttachBody->SetGravity(false);

		mfBodyMass = pAttachBody->GetMass();
		pAttachBody->SetMass(0);

		mpAttachedBody = pAttachBody;
		mpLastAttachedBody = mpAttachedBody;

		// Log("Attaching body!\n");
	}
}

//-----------------------------------------------------------------------

void cGameStickArea::DetachBody() {
	if (mpAttachedBody) {
		// Log("Dettached body %s\n", mpAttachedBody->GetName().c_str());

		cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
		iPhysicsBody *pAreaBody = mvBodies[0];

		// Callback function
		if (msDetachFunction != "") {
			tString sCommand = GetCallbackFunc(msDetachFunction, mpAttachedBody);
			mpInit->RunScriptCommand(sCommand);
		}

		// Sound
		if (msDetachSound != "") {
			cSoundEntity *pSound = pWorld->CreateSoundEntity("DetachSound", msDetachSound, true);
			if (pSound)
				pSound->SetPosition(pAreaBody->GetWorldPosition());
		}

		// Particle System
		if (msDetachPS != "") {
			pWorld->CreateParticleSystem("DetachPS", msDetachPS, 1, pAreaBody->GetWorldMatrix());
		}

		mpAttachedBody->SetGravity(true);
		mpAttachedBody->SetMass(mfBodyMass);
		mpAttachedBody->SetEnabled(true);

		// Log("Body mass: %f\n",mpAttachedBody->GetMass());

		mpAttachedBody = NULL;
	}
}

//-----------------------------------------------------------------------

bool cGameStickArea::GetCanDeatch() {
	if (mfSetMtxTime < 1)
		return false;

	return mbCanDeatch;
}

//-----------------------------------------------------------------------

tString cGameStickArea::GetCallbackFunc(const tString &asFunc, iPhysicsBody *apBody) {
	return asFunc + "(\"" + msName + "\",\"" + apBody->GetName() + "\")";
}

//-----------------------------------------------------------------------

void cGameStickArea::OnPostSceneDraw() {
	// iPhysicsBody *pAreaBody = mvBodies[0];

	// pAreaBody->RenderDebugGeometry(mpInit->mpGame->GetGraphics()->GetLowLevel(),cColor(1,1));
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

kBeginSerialize(cGameStickArea_SaveData, iGameEntity_SaveData)
	kSerializeVar(mvSize, eSerializeType_Vector3f)

		kSerializeVar(msAttachFunction, eSerializeType_String)
			kSerializeVar(msDetachFunction, eSerializeType_String)

				kSerializeVar(msAttachSound, eSerializeType_String)
					kSerializeVar(msDetachSound, eSerializeType_String)

						kSerializeVar(msAttachPS, eSerializeType_String)
							kSerializeVar(msDetachPS, eSerializeType_String)

								kSerializeVar(mbCanDeatch, eSerializeType_Bool)

									kSerializeVar(msAttachedBody, eSerializeType_String)

										kSerializeVar(mfSetMtxTime, eSerializeType_Float32)

											kSerializeVar(mbCheckCenterInArea, eSerializeType_Bool)

												kSerializeVar(mfPoseTime, eSerializeType_Float32)
													kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameStickArea_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameStickArea::CreateSaveData() {
	return hplNew(cGameStickArea_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameStickArea::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameStickArea_SaveData *pData = static_cast<cGameStickArea_SaveData *>(apSaveData);

	kCopyToVar(pData, msAttachFunction);
	kCopyToVar(pData, msDetachFunction);

	kCopyToVar(pData, msAttachSound);
	kCopyToVar(pData, msDetachSound);

	kCopyToVar(pData, msAttachPS);
	kCopyToVar(pData, msDetachPS);

	kCopyToVar(pData, mbCanDeatch);

	kCopyToVar(pData, mbCheckCenterInArea);

	kCopyToVar(pData, mfPoseTime);

	kCopyToVar(pData, mfSetMtxTime);

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();

	if (mpAttachedBody) {
		pData->msAttachedBody = mpAttachedBody->GetName();
	}
}

//-----------------------------------------------------------------------

void cGameStickArea::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameStickArea_SaveData *pData = static_cast<cGameStickArea_SaveData *>(apSaveData);

	kCopyFromVar(pData, msAttachFunction);
	kCopyFromVar(pData, msDetachFunction);

	kCopyFromVar(pData, msAttachSound);
	kCopyFromVar(pData, msDetachSound);

	kCopyFromVar(pData, msAttachPS);
	kCopyFromVar(pData, msDetachPS);

	kCopyFromVar(pData, mbCanDeatch);

	kCopyFromVar(pData, mbCheckCenterInArea);

	kCopyFromVar(pData, mfPoseTime);

	kCopyFromVar(pData, mfSetMtxTime);
}

//-----------------------------------------------------------------------

void cGameStickArea::SetupSaveData(iGameEntity_SaveData *apSaveData) {
	super::SetupSaveData(apSaveData);
	cGameStickArea_SaveData *pData = static_cast<cGameStickArea_SaveData *>(apSaveData);

	if (pData->msAttachedBody != "") {
		iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
		mpAttachedBody = pWorld->GetBody(pData->msAttachedBody);
		mpLastAttachedBody = mpAttachedBody;
	}
}
//-----------------------------------------------------------------------
