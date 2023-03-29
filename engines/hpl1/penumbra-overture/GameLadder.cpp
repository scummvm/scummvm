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

#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerState_Misc.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cAreaLoader_GameLadder::cAreaLoader_GameLadder(const tString &asName, cInit *apInit)
	: iArea3DLoader(asName) {
	mpInit = apInit;
}

cAreaLoader_GameLadder::~cAreaLoader_GameLadder() {
}

//-----------------------------------------------------------------------

iEntity3D *cAreaLoader_GameLadder::Load(const tString &asName, const cVector3f &avSize,
										const cMatrixf &a_mtxTransform, cWorld3D *apWorld) {
	cGameLadder *pLadder = hplNew(cGameLadder, (mpInit, asName));

	pLadder->m_mtxOnLoadTransform = a_mtxTransform;

	// Create physics data
	iPhysicsWorld *pPhysicsWorld = apWorld->GetPhysicsWorld();
	iCollideShape *pShape = pPhysicsWorld->CreateBoxShape(avSize, NULL);
	Common::Array<iPhysicsBody *> vBodies;
	vBodies.push_back(pPhysicsWorld->CreateBody(asName, pShape));

	vBodies[0]->SetCollide(false);
	vBodies[0]->SetCollideCharacter(false);
	vBodies[0]->SetMatrix(a_mtxTransform);

	vBodies[0]->SetUserData(pLadder);
	pLadder->SetBodies(vBodies);

	mpInit->mpMapHandler->AddGameEntity(pLadder);

	pLadder->Setup();

	return NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGameLadder::cGameLadder(cInit *apInit, const tString &asName) : iGameEntity(apInit, asName) {
	mType = eGameEntityType_Ladder;

	msAttachSound = mpInit->mpGameConfig->GetString("Movement_Climb", "DefaultAttachSound", "");
	msClimbUpSound = mpInit->mpGameConfig->GetString("Movement_Climb", "DefaultClimbUpSound", "");
	msClimbDownSound = mpInit->mpGameConfig->GetString("Movement_Climb", "DefaultClimbDownSound", "");
}

//-----------------------------------------------------------------------

cGameLadder::~cGameLadder(void) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGameLadder::OnPlayerPick() {
	float fHeight = GetHeight();
	if (GetDist2D() <= 1.4 && fabs(fHeight) < 1.1f) {
		mpInit->mpPlayer->SetCrossHairState(eCrossHairState_Ladder);
	}
}

void cGameLadder::OnPlayerInteract() {
	float fHeight = GetHeight();
	if (GetDist2D() <= 1.4 && fabs(fHeight) < 1.1f) {
		mpInit->mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);

		///////////////////////////////
		// Check if the pos infront of the ladder is free.
		iCharacterBody *pCharBody = mpInit->mpPlayer->GetCharacterBody();
		iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
		cMatrixf mtxPos = cMath::MatrixTranslate(GetStartPosition());
		cVector3f vNewPos;
		bool bFound = false;

		do {
			pWorld->CheckShapeWorldCollision(&vNewPos, pCharBody->GetShape(), mtxPos,
											 pCharBody->GetBody(), false, true, NULL, true);
			if (vNewPos == mtxPos.GetTranslation()) {
				bFound = true;
				break;
			}

			mtxPos.SetTranslation(mtxPos.GetTranslation() + cVector3f(0, 0.1f, 0));
		} while (mtxPos.GetTranslation().y <= mfMaxY);

		if (bFound == false) {
			// TODO: Message?
			return;
		}

		///////////////////////////////
		// Set state
		cPlayerState_Climb *pState = static_cast<cPlayerState_Climb *>(mpInit->mpPlayer->GetStateData(ePlayerState_Climb));
		pState->mpLadder = this;
		pState->mvStartPosition = mtxPos.GetTranslation();

		mpInit->mpPlayer->ChangeState(ePlayerState_Climb);
	}
}

//-----------------------------------------------------------------------

void cGameLadder::OnPostSceneDraw() {
	// mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawLine(mvBodies[0]->GetWorldPosition(),
	//											mvBodies[0]->GetWorldPosition()+mvForward,
	//											cColor(0,1,0,1));
}

//-----------------------------------------------------------------------

void cGameLadder::Setup() {
	cMatrixf mtxInv = cMath::MatrixInverse(mvBodies[0]->GetWorldMatrix());

	mvForward = mtxInv.GetForward();

	mfMaxY = mvBodies[0]->GetWorldPosition().y + mvBodies[0]->GetShape()->GetSize().y / 2.0f;
	mfMinY = mvBodies[0]->GetWorldPosition().y - mvBodies[0]->GetShape()->GetSize().y / 2.0f;
}

//-----------------------------------------------------------------------

float cGameLadder::GetDist2D() {
	iCharacterBody *pCharBody = mpInit->mpPlayer->GetCharacterBody();
	cVector3f vPosA = pCharBody->GetPosition();
	vPosA.y = 0;
	cVector3f vPosB = mvBodies[0]->GetWorldPosition();
	vPosB.y = 0;
	return cMath::Vector3Dist(vPosA, vPosB);
}

//-----------------------------------------------------------------------

float cGameLadder::GetHeight() {
	iCharacterBody *pCharBody = mpInit->mpPlayer->GetCharacterBody();
	cVector3f vPosA = pCharBody->GetPosition();
	cVector3f vPosB = mpInit->mpPlayer->GetPickedPos();

	return vPosB.y - vPosA.y;
}

//-----------------------------------------------------------------------

cVector3f cGameLadder::GetStartRotation() {
	/*iCharacterBody *pCharBody = */ mpInit->mpPlayer->GetCharacterBody();

	return cMath::GetAngleFromPoints3D(cVector3f(0, 0, 0), GetForward() * -1);
}
cVector3f cGameLadder::GetStartPosition() {
	iCharacterBody *pCharBody = mpInit->mpPlayer->GetCharacterBody();

	cVector3f vPos = pCharBody->GetPosition();
	cVector3f vLadderPos = GetBody(0)->GetWorldPosition() +
						   GetForward() * pCharBody->GetSize().x * 0.6f;
	vLadderPos.y = vPos.y + 0.05f;

	if (vLadderPos.y > mfMaxY - pCharBody->GetSize().y * 0.3f)
		vLadderPos.y = mfMaxY - pCharBody->GetSize().y * 0.3f;

	if (vLadderPos.y - pCharBody->GetSize().y / 2 < mfMinY)
		vLadderPos.y = mfMinY + pCharBody->GetSize().y / 2 + 0.1f;

	return vLadderPos;
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

kBeginSerialize(cGameLadder_SaveData, iGameEntity_SaveData)
	kSerializeVar(mvSize, eSerializeType_Vector3f)

		kSerializeVar(msAttachSound, eSerializeType_String)
			kSerializeVar(msClimbUpSound, eSerializeType_String)
				kSerializeVar(msClimbDownSound, eSerializeType_String)
					kEndSerialize()

	//-----------------------------------------------------------------------

	iGameEntity *cGameLadder_SaveData::CreateEntity() {
	return NULL;
}

//-----------------------------------------------------------------------

iGameEntity_SaveData *cGameLadder::CreateSaveData() {
	return hplNew(cGameLadder_SaveData, ());
}

//-----------------------------------------------------------------------

void cGameLadder::SaveToSaveData(iGameEntity_SaveData *apSaveData) {
	super::SaveToSaveData(apSaveData);
	cGameLadder_SaveData *pData = static_cast<cGameLadder_SaveData *>(apSaveData);

	pData->mvSize = mvBodies[0]->GetShape()->GetSize();

	kCopyToVar(pData, msAttachSound);
	kCopyToVar(pData, msClimbUpSound);
	kCopyToVar(pData, msClimbDownSound);
}

//-----------------------------------------------------------------------

void cGameLadder::LoadFromSaveData(iGameEntity_SaveData *apSaveData) {
	super::LoadFromSaveData(apSaveData);
	cGameLadder_SaveData *pData = static_cast<cGameLadder_SaveData *>(apSaveData);

	kCopyFromVar(pData, msAttachSound);
	kCopyFromVar(pData, msClimbUpSound);
	kCopyFromVar(pData, msClimbDownSound);
}
//-----------------------------------------------------------------------
