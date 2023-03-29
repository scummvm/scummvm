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

#include "hpl1/penumbra-overture/PlayerState_Misc.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// NORMAL STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_Normal::cPlayerState_Normal(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Normal) {
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::OnUpdate(float afTimeStep) {
	if (mpInit->mpNotebook->IsActive() == false &&
		mpInit->mpInventory->IsActive() == false &&
		mpInit->mpNumericalPanel->IsActive() == false &&
		mpInit->mpDeathMenu->IsActive() == false) {
		mpPlayer->ResetCrossHairPos();
	}

	/////////////////////////////////////
	// If run is down, run!!
	cInput *pInput = mpInit->mpGame->GetInput();
	if (pInput->IsTriggerd("Run") &&
		mpPlayer->GetMoveState() == ePlayerMoveState_Walk) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	}

	/////////////////////////////////////////////////
	// Cast ray to see if anything is picked.
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	cVector3f vStart, vEnd;

	vStart = mpPlayer->GetCamera()->GetPosition();
	vEnd = vStart + mpPlayer->GetCamera()->GetForward() * mpPlayer->GetPickRay()->mfMaxDistance;

	mpPlayer->GetPickRay()->Clear();
	pPhysicsWorld->CastRay(mpPlayer->GetPickRay(), vStart, vEnd, true, false, true);
	mpPlayer->GetPickRay()->CalculateResults();

	// Log("Picked body: %d\n",(size_t)mpPlayer->GetPickedBody());

	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		// Set cross hair state
		eCrossHairState CrossState = pEntity->GetPickCrossHairState(mpPlayer->GetPickedBody());

		if (CrossState == eCrossHairState_Active ||
			(CrossState == eCrossHairState_Examine && !pEntity->GetHasBeenExamined())) {
			mpPlayer->SetCrossHairState(CrossState);
		} else {
			mpPlayer->SetCrossHairState(eCrossHairState_None);
		}

		// Call entity
		pEntity->PlayerPick();
	} else {
		mpPlayer->SetCrossHairState(eCrossHairState_None);
	}
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::OnStartInteract() {
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerInteract();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::OnStartExamine() {
	// Log("Picked body: %d\n",(size_t)mpPlayer->GetPickedBody());
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::OnStartRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Walk ||
		mpPlayer->GetMoveState() == ePlayerMoveState_Crouch) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	}
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Run);
	}
}
void cPlayerState_Normal::OnStopRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	else if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::OnStartCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		return;

	if (mpInit->mpButtonHandler->GetToggleCrouch()) {
		if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch)
			mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
		else
			mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
	} else {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
	}
}

void cPlayerState_Normal::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::OnStartInteractMode() {
	mpPlayer->ChangeState(ePlayerState_InteractMode);
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::EnterState(iPlayerState *apPrevState) {
	mpPlayer->ResetCrossHairPos();
}

//-----------------------------------------------------------------------

void cPlayerState_Normal::LeaveState(iPlayerState *apNextState) {
	// Can cause crashes!!
	// mpPlayer->GetPickRay()->mpPickedBody = NULL;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INTERACT MODE STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_InteractMode::cPlayerState_InteractMode(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_InteractMode) {
	mvLookSpeed = 0;
	mfRange = 15.0f;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::OnUpdate(float afTimeStep) {
	/////////////////////////////////////////////////
	// Move viewport.
	/*if(mvLookSpeed.x != 0)
	{
		mpPlayer->GetCamera()->AddYaw(mvLookSpeed.x * afTimeStep * 1.0f);
		mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
	}
	if(mvLookSpeed.y != 0)
	{
		mpPlayer->GetCamera()->AddPitch(mvLookSpeed.y * afTimeStep * 1.0f);
	}*/

	// LogUpdate("  Casting ray\n");
	/////////////////////////////////////////////////
	// Cast ray to see if anything is picked.
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	cVector3f vStart, vEnd;

	cVector3f vDir = mpPlayer->GetCamera()->UnProject(
		mpPlayer->GetCrossHairPos(),
		mpInit->mpGame->GetGraphics()->GetLowLevel());
	vStart = mpPlayer->GetCamera()->GetPosition();
	vEnd = vStart + vDir * mpPlayer->GetPickRay()->mfMaxDistance;

	mpPlayer->GetPickRay()->Clear();
	pPhysicsWorld->CastRay(mpPlayer->GetPickRay(), vStart, vEnd, true, false, true);

	// LogUpdate("  Calc pickray results\n");
	mpPlayer->GetPickRay()->CalculateResults();

	// LogUpdate("  Use picked body\n");
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		eCrossHairState CrossState = pEntity->GetPickCrossHairState(mpPlayer->GetPickedBody());

		if (CrossState == eCrossHairState_None)
			mpPlayer->SetCrossHairState(eCrossHairState_Inactive);
		else
			mpPlayer->SetCrossHairState(CrossState);

		pEntity->PlayerPick();
	} else {
		mpPlayer->SetCrossHairState(eCrossHairState_Inactive);
	}
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::OnStartInteract() {
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerInteract();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::OnStartExamine() {
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_InteractMode::OnAddYaw(float afVal) {
	cInput *pInput = mpInit->mpGame->GetInput();

	if (pInput->IsTriggerd("LookMode")) {
		mpPlayer->GetCamera()->AddYaw(-afVal * 2.0f * mpPlayer->GetLookSpeed());
		mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
	} else {
		if (mpPlayer->AddCrossHairPos(cVector2f(afVal * 800.0f, 0))) {
			mpPlayer->GetCamera()->AddYaw(-afVal * mpPlayer->GetLookSpeed());
			mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
		}

		cVector2f vBorder = mpPlayer->GetInteractMoveBorder();
		cVector2f vPos = mpPlayer->GetCrossHairPos();

		if (vPos.x < vBorder.x + mfRange || vPos.x > (799 - vBorder.x - mfRange)) {
			float fDist;
			if (vPos.x < vBorder.x + mfRange) {
				fDist = (vPos.x - vBorder.x);
				mvLookSpeed.x = 1 - (fDist / mfRange) * 1;
			} else {
				fDist = ((799 - vBorder.x) - vPos.x);
				mvLookSpeed.x = (1 - (fDist / mfRange)) * -1;
			}
		} else {
			mvLookSpeed.x = 0;
		}
	}

	return false;
}

bool cPlayerState_InteractMode::OnAddPitch(float afVal) {
	cInput *pInput = mpInit->mpGame->GetInput();

	if (pInput->IsTriggerd("LookMode")) {
		float fInvert = mpInit->mpButtonHandler->GetInvertMouseY() ? -1.0f : 1.0f;
		mpPlayer->GetCamera()->AddPitch(-afVal * 2.0f * fInvert * mpPlayer->GetLookSpeed());
	} else {
		if (mpPlayer->AddCrossHairPos(cVector2f(0, afVal * 600.0f))) {
			mpPlayer->GetCamera()->AddPitch(-afVal * mpPlayer->GetLookSpeed());
		}

		cVector2f vBorder = mpPlayer->GetInteractMoveBorder();
		cVector2f vPos = mpPlayer->GetCrossHairPos();

		if (vPos.y < vBorder.y + mfRange || vPos.y > (599 - vBorder.y - mfRange)) {
			float fDist;
			if (vPos.y < vBorder.y + mfRange) {
				fDist = (vPos.y - vBorder.y);
				mvLookSpeed.y = 1 - (fDist / mfRange) * 1;
			} else {
				fDist = ((599 - vBorder.y) - vPos.y);
				mvLookSpeed.y = (1 - (fDist / mfRange)) * -1;
			}
		} else {
			mvLookSpeed.y = 0;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::OnStartInteractMode() {
	mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::EnterState(iPlayerState *apPrevState) {
	mPrevMoveState = mpPlayer->GetMoveState();

	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);

	mvLookSpeed = 0;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::LeaveState(iPlayerState *apNextState) {
	/*if(mPrevMoveState != ePlayerMoveState_Run)
	mpPlayer->ChangeMoveState(mPrevMoveState);
	else
	mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);*/

	if (apNextState->mType == ePlayerState_Normal)
		mpPlayer->ResetCrossHairPos();

	// Can cause crashes!!
	// mpPlayer->GetPickRay()->mpPickedBody = NULL;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractMode::OnStartCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		return;

	if (mpInit->mpButtonHandler->GetToggleCrouch()) {
		if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch)
			mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
		else
			mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
	} else {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
	}
}

void cPlayerState_InteractMode::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// USE ITEM STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_UseItem::cPlayerState_UseItem(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_UseItem) {
}

//-----------------------------------------------------------------------

void cPlayerState_UseItem::OnUpdate(float afTimeStep) {
	/////////////////////////////////////////////////
	// Cast ray to see if anything is picked.
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	cVector3f vStart, vEnd;

	cVector3f vDir = mpPlayer->GetCamera()->UnProject(
		mpPlayer->GetCrossHairPos(),
		mpInit->mpGame->GetGraphics()->GetLowLevel());
	vStart = mpPlayer->GetCamera()->GetPosition();
	vEnd = vStart + vDir * mpPlayer->GetPickRay()->mfMaxDistance;

	mpPlayer->GetPickRay()->Clear();
	pPhysicsWorld->CastRay(mpPlayer->GetPickRay(), vStart, vEnd, true, false, true);
	mpPlayer->GetPickRay()->CalculateResults();

	iPhysicsBody *pBody = mpPlayer->GetPickedBody();
	iGameEntity *pEntity = NULL;
	if (pBody)
		pEntity = (iGameEntity *)pBody->GetUserData();

	if (pEntity && mpPlayer->GetPickedDist() <= pEntity->GetMaxInteractDist()) {
		mpInit->mpPlayer->SetItemFlash(true);
	} else {
		mpInit->mpPlayer->SetItemFlash(false);
	}
}

//-----------------------------------------------------------------------

void cPlayerState_UseItem::OnStartInteract() {
	iPhysicsBody *pBody = mpPlayer->GetPickedBody();
	iGameEntity *pEntity = NULL;
	if (pBody)
		pEntity = (iGameEntity *)pBody->GetUserData();

	if (pEntity && mpPlayer->GetPickedDist() <= pEntity->GetMaxExamineDist()) {
		if (mpPlayer->GetPickedDist() <= pEntity->GetMaxInteractDist()) {
			iGameEntity *pEntity2 = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();
			cGameItemType *pType = mpInit->mpInventory->GetItemType(mpPlayer->GetCurrentItem()->GetItemType());

			if (mPrevState == ePlayerState_WeaponMelee ||
				mPrevState == ePlayerState_Throw) {
				mpPlayer->ChangeState(ePlayerState_Normal);
			} else {
				mpPlayer->ChangeState(mPrevState);
			}

			pType->OnUse(mpPlayer->GetCurrentItem(), pEntity2);
		} else {
			mpInit->mpEffectHandler->GetSubTitle()->Add(kTranslate("Player", "UseItemTooFar"), 2.0f, true);
			return;
		}
	} else {
		if (mPrevState == ePlayerState_WeaponMelee ||
			mPrevState == ePlayerState_Throw) {
			mpPlayer->ChangeState(ePlayerState_Normal);
		} else {
			mpPlayer->ChangeState(mPrevState);
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerState_UseItem::OnStartExamine() {
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		if (mpPlayer->GetPickedDist() <= pEntity->GetMaxExamineDist()) {
			pEntity->PlayerExamine();
		}
	} else {
		if (mPrevState == ePlayerState_WeaponMelee ||
			mPrevState == ePlayerState_Throw) {
			mpPlayer->ChangeState(ePlayerState_Normal);
		} else {
			mpPlayer->ChangeState(mPrevState);
		}
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_UseItem::OnAddYaw(float afVal) {
	cInput *pInput = mpInit->mpGame->GetInput();

	if (pInput->IsTriggerd("LookMode")) {
		mpPlayer->GetCamera()->AddYaw(-afVal * 2.0f * mpPlayer->GetLookSpeed());
		mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
	} else if (mpPlayer->AddCrossHairPos(cVector2f(afVal * 800.0f, 0))) {
		mpPlayer->GetCamera()->AddYaw(-afVal * mpPlayer->GetLookSpeed());
		mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
	}

	return false;
}

bool cPlayerState_UseItem::OnAddPitch(float afVal) {
	cInput *pInput = mpInit->mpGame->GetInput();

	if (pInput->IsTriggerd("LookMode")) {
		float fInvert = mpInit->mpButtonHandler->GetInvertMouseY() ? -1.0f : 1.0f;
		mpPlayer->GetCamera()->AddPitch(-afVal * 2.0f * fInvert * mpPlayer->GetLookSpeed());
	} else if (mpPlayer->AddCrossHairPos(cVector2f(0, afVal * 600.0f))) {
		mpPlayer->GetCamera()->AddPitch(-afVal * mpPlayer->GetLookSpeed());
	}

	return false;
}

//-----------------------------------------------------------------------

void cPlayerState_UseItem::EnterState(iPlayerState *apPrevState) {
	mPrevMoveState = mpPlayer->GetMoveState();
	mPrevState = apPrevState->mType;

	mpPlayer->SetCrossHairState(eCrossHairState_Item);
}

//-----------------------------------------------------------------------

void cPlayerState_UseItem::LeaveState(iPlayerState *apNextState) {
	mpPlayer->SetCrossHairState(eCrossHairState_None);
}

//-----------------------------------------------------------------------

void cPlayerState_UseItem::OnStartCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		return;

	if (mpInit->mpButtonHandler->GetToggleCrouch()) {
		if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch)
			mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
		else
			mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
	} else {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
	}
}

void cPlayerState_UseItem::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_UseItem::OnStartInventory() {
	mpPlayer->ChangeState(mPrevState);
	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MESSAGE STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_Message::cPlayerState_Message(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Message) {
}

//-----------------------------------------------------------------------

void cPlayerState_Message::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

bool cPlayerState_Message::OnJump() {
	return false;
}

//-----------------------------------------------------------------------

void cPlayerState_Message::OnStartInteract() {
	mpInit->mpGameMessageHandler->ShowNext();
}

void cPlayerState_Message::OnStopInteract() {
}

//-----------------------------------------------------------------------

void cPlayerState_Message::OnStartExamine() {
	mpInit->mpGameMessageHandler->ShowNext();
}

//-----------------------------------------------------------------------

bool cPlayerState_Message::OnMoveForwards(float afMul, float afTimeStep) { return false; }
bool cPlayerState_Message::OnMoveSideways(float afMul, float afTimeStep) { return false; }

//-----------------------------------------------------------------------

bool cPlayerState_Message::OnAddYaw(float afVal) { return false; }
bool cPlayerState_Message::OnAddPitch(float afVal) { return false; }

//-----------------------------------------------------------------------

void cPlayerState_Message::EnterState(iPlayerState *apPrevState) {
	// Change move state so the player is still
	mPrevMoveState = mpPlayer->GetMoveState();
	// mpPlayer->ChangeMoveState(ePlayerMoveState_Still);

	mpPlayer->SetCrossHairState(eCrossHairState_None);
}

//-----------------------------------------------------------------------

void cPlayerState_Message::LeaveState(iPlayerState *apNextState) {
	if (mPrevMoveState != ePlayerMoveState_Run && mPrevMoveState != ePlayerMoveState_Jump)
		mpPlayer->ChangeMoveState(mPrevMoveState);
	else
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
}

//-----------------------------------------------------------------------

bool cPlayerState_Message::OnStartInventory() {
	return false;
}

//-----------------------------------------------------------------------

bool cPlayerState_Message::OnStartInventoryShortCut(int alNum) {
	return false;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CLIMB STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_Climb::cPlayerState_Climb(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Climb) {
	mpLadder = NULL;

	mfUpSpeed = mpInit->mpGameConfig->GetFloat("Movement_Climb", "UpSpeed", 0);
	mfDownSpeed = mpInit->mpGameConfig->GetFloat("Movement_Climb", "DownSpeed", 0);

	mfStepLength = mpInit->mpGameConfig->GetFloat("Movement_Climb", "StepLength", 0);

	mfStepCount = 0;

	mlState = 0;
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::PlaySound(const tString &asSound) {
	if (asSound == "")
		return;

	iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	cSoundEntity *pSound = pWorld->CreateSoundEntity("LadderStep", asSound, true);
	if (pSound)
		pSound->SetPosition(pCharBody->GetPosition());
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::EnterState(iPlayerState *apPrevState) {
	mpPlayer->SetCrossHairState(eCrossHairState_None);

	mvPrevPitchLimits = mpPlayer->GetCamera()->GetPitchLimits();

	mlState = 0;
	mpPlayer->GetCharacterBody()->SetGravityActive(false);
	mpPlayer->GetCharacterBody()->SetTestCollision(false);

	mvGoalPos = mvStartPosition;
	mvGoalRot.x = 0;
	mvGoalRot.y = mpLadder->GetStartRotation().y;

	// Different time if you are above the ladder.
	float fTime = 0.5f;
	if (mpPlayer->GetCharacterBody()->GetPosition().y > mpLadder->GetMaxY())
		fTime = 1.2f;

	cVector3f vStartRot;
	vStartRot.x = mpPlayer->GetCamera()->GetPitch();
	vStartRot.y = mpPlayer->GetCamera()->GetYaw();

	mvPosAdd = (mvGoalPos - mpPlayer->GetCharacterBody()->GetPosition()) / fTime;

	mvRotAdd.x = cMath::GetAngleDistance(vStartRot.x, mvGoalRot.x, k2Pif) / fTime;
	mvRotAdd.y = cMath::GetAngleDistance(vStartRot.y, mvGoalRot.y, k2Pif) / fTime;

	mvCharPosition = mpPlayer->GetCharacterBody()->GetPosition();

	mfTimeCount = fTime;

	mfStepCount = 0;

	mbPlayedSound = false;

	// Play Sound
	PlaySound(mpLadder->GetAttachSound());
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::LeaveState(iPlayerState *apNextState) {
	mpPlayer->SetCrossHairState(eCrossHairState_None);

	mpPlayer->GetCharacterBody()->SetGravityActive(true);

	mpPlayer->GetCamera()->SetPitchLimits(mvPrevPitchLimits);
	mpPlayer->GetCamera()->SetYawLimits(cVector2f(0, 0));
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::OnUpdate(float afTimeStep) {
	iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
	cCamera3D *pCam = mpPlayer->GetCamera();

	//////////////////////////////////
	// Attach To Ladder
	if (mlState == 0) {
		mfTimeCount -= afTimeStep;

		mvCharPosition += mvPosAdd * afTimeStep;
		pCharBody->SetPosition(mvCharPosition);
		mvCharPosition = pCharBody->GetPosition();

		pCam->AddPitch(mvRotAdd.x * afTimeStep);
		pCam->AddYaw(mvRotAdd.y * afTimeStep);
		pCharBody->SetYaw(pCam->GetYaw());

		if (mfTimeCount <= 0) {
			mpPlayer->GetCharacterBody()->SetTestCollision(true);

			////////////////////////////
			// Set turn head limits
			cVector2f vMaxHeadLimits = cVector2f(cMath::ToRad(120), cMath::ToRad(79));
			cVector2f vMinHeadLimits = cVector2f(cMath::ToRad(-120), cMath::ToRad(-60));
			float fXmax = pCam->GetYaw() + vMaxHeadLimits.x;
			float fYmax = pCam->GetPitch() + vMaxHeadLimits.y;

			float fXmin = pCam->GetYaw() + vMinHeadLimits.x;
			float fYmin = pCam->GetPitch() + vMinHeadLimits.y;

			pCam->SetPitchLimits(cVector2f(fYmax, fYmin));
			pCam->SetYawLimits(cVector2f(fXmax, fXmin));

			mlState++;
		}
	}
	//////////////////////////////////
	// Move On Ladder
	else if (mlState == 1) {

	}
	//////////////////////////////////
	// On the top of the ladder
	else if (mlState == 2) {
		mfLeaveAtTopCount -= afTimeStep;
		pCharBody->Move(eCharDir_Forward, 1, afTimeStep);

		if (pCharBody->IsOnGround()) {
			mfLeaveAtTopCount -= afTimeStep;
		}

		if (mfLeaveAtTopCount <= 0) {
			mpPlayer->ChangeState(ePlayerState_Normal);
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::OnStartInteract() {
	if (mlState != 0)
		mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::OnStartExamine() {
	if (mlState != 0)
		mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

bool cPlayerState_Climb::OnAddYaw(float afVal) {
	if (mlState != 0)
		mpPlayer->GetCamera()->AddYaw(-afVal * mpPlayer->GetLookSpeed());

	return false;
}

bool cPlayerState_Climb::OnAddPitch(float afVal) {
	// if(mlState!=0)
	//	mpPlayer->GetCamera()->AddPitch( -afVal * mpPlayer->GetLookSpeed());
	if (mlState == 0)
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_Climb::OnMoveForwards(float afMul, float afTimeStep) {
	if (mlState == 0 || mlState == 2)
		return false;

	iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
	iPhysicsWorld *pWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	mvCharPosition = pCharBody->GetPosition();
	if (afMul > 0) {
		mvCharPosition.y += afMul * mfUpSpeed * afTimeStep;

		if (mfStepCount < 0)
			mfStepCount = 0;
	} else if (afMul < 0) {
		mvCharPosition.y += afMul * mfDownSpeed * afTimeStep;

		if (mfStepCount > 0)
			mfStepCount = 0;
	} else {
		if (!mbPlayedSound) {
			if (mfStepCount > 0)
				PlaySound(mpLadder->GetClimbUpSound());
			if (mfStepCount < 0)
				PlaySound(mpLadder->GetClimbDownSound());
		}

		mfStepCount = 0;
		mbPlayedSound = false;
	}

	// Check collision
	cMatrixf mtxPos = cMath::MatrixTranslate(mvCharPosition);
	cVector3f vNewPos;
	pWorld->CheckShapeWorldCollision(&vNewPos, pCharBody->GetShape(), mtxPos,
									 pCharBody->GetBody(), false, true, NULL, true);
	if (vNewPos != mtxPos.GetTranslation()) {
		return false;
	}

	if (afMul > 0) {
		mfStepCount += afMul * mfUpSpeed * afTimeStep;
		if (mfStepCount >= mfStepLength) {
			mfStepCount = 0;
			PlaySound(mpLadder->GetClimbUpSound());
			mbPlayedSound = true;
		}
	} else if (afMul < 0) {
		mfStepCount += afMul * mfDownSpeed * afTimeStep;
		if (mfStepCount <= -mfStepLength) {
			mfStepCount = 0;
			PlaySound(mpLadder->GetClimbDownSound());
			mbPlayedSound = true;
		}
	} else {
	}

	pCharBody->SetPosition(mvCharPosition);

	if ((mvCharPosition.y - pCharBody->GetSize().y * 0.5f) > mpLadder->GetMaxY()) {
		// mpPlayer->ChangeState(ePlayerState_Normal);
		mlState = 2;
		mfLeaveAtTopCount = 2;
		mpPlayer->GetCharacterBody()->SetGravityActive(true);
	} else if ((mvCharPosition.y - pCharBody->GetSize().y / 2) < mpLadder->GetMinY()) {
		mpPlayer->ChangeState(ePlayerState_Normal);
	}

	return false;
}
bool cPlayerState_Climb::OnMoveSideways(float afMul, float afTimeStep) {
	/*iCharacterBody *pCharBody = */ mpPlayer->GetCharacterBody();

	return false;
}

//-----------------------------------------------------------------------

void cPlayerState_Climb::OnStartCrouch() {
}

void cPlayerState_Climb::OnStopCrouch() {
}

//-----------------------------------------------------------------------

bool cPlayerState_Climb::OnJump() {
	mpPlayer->ChangeState(ePlayerState_Normal);
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_Climb::OnStartInventory() {
	return false;
}

//-----------------------------------------------------------------------
