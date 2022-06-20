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

#include "hpl1/penumbra-overture/PlayerState_MiscHaptX.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/penumbra-overture/HapticGameCamera.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

//////////////////////////////////////////////////////////////////////////
// NORMAL STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_NormalHaptX::cPlayerState_NormalHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Normal) {
}

//-----------------------------------------------------------------------

cVector3f gvStart, gvEnd;
cVector3f gvFinalStart = 0, gvFinalEnd = 0;
void cPlayerState_NormalHaptX::OnUpdate(float afTimeStep) {
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
	mpPlayer->mbProxyTouching = false;
	mpPlayer->GetPickRay()->mpPickedBody = NULL;
	cVector3f vProxyPos = mpInit->mpGame->GetHaptic()->GetLowLevel()->GetProxyPosition();
	mpPlayer->GetPickRay()->mvPickedPos = vProxyPos;
	mpPlayer->GetPickRay()->mfPickedDist = cMath::Vector3Dist(vProxyPos, mpPlayer->GetCamera()->GetPosition());

	cPortalContainer *pCont = mpInit->mpGame->GetScene()->GetWorld3D()->GetPortalContainer();
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	cBoundingVolume bv;
	bv.SetSize(mpPlayer->GetHapticCamera()->GetHandReachRadius() * 2);
	bv.SetPosition(vProxyPos);
	cCollideData collideData;
	collideData.SetMaxSize(1);

	cPortalContainerEntityIterator it = pCont->GetEntityIterator(&bv);
	iGameEntity *pPickedEntity = NULL;
	eGameEntityType pickedType = eGameEntityType_LastEnum;

	while (it.HasNext()) {
		iPhysicsBody *pBody = static_cast<iPhysicsBody *>(it.Next());

		if (pBody->GetUserData() == NULL)
			continue;
		if (cMath::CheckCollisionBV(bv, *pBody->GetBV()) == false)
			continue;

		iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();
		eGameEntityType type = pEntity->GetType();

		//////////////////////////////////////
		// TODO: Sort out more unwanted entites here (areas etc?)

		// If Item is picked, skip all else
		if (pickedType == eGameEntityType_Item && type != eGameEntityType_Item)
			continue;

		// Non moveable
		if (pBody->GetMass() == 0)
			continue;
		// Static
		if (pEntity->GetType() == eGameEntityType_Object) {
			cGameObject *pObject = static_cast<cGameObject *>(pEntity);
			if (pObject->GetInteractMode() == eObjectInteractMode_Static)
				continue;
		}
		// more checks...

		////////////////////////////////////
		// Collision check
		if (pPhysicsWorld->CheckShapeCollision(mpPlayer->GetHapticCamera()->GetHandShape(),
											   cMath::MatrixTranslate(vProxyPos),
											   pBody->GetShape(), pBody->GetWorldMatrix(),
											   collideData, 1)) {
			pickedType = type;

			mpPlayer->mbProxyTouching = true;
			mpPlayer->GetPickRay()->mpPickedBody = pBody;
			if (pickedType == eGameEntityType_Item)
				break;
		}
	}

	////////////////////////////////////
	// Cross hair and pick check
	if (mpPlayer->GetPickRay()->mpPickedBody) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		// Set cross hair state
		eCrossHairState CrossState = pEntity->GetPickCrossHairState(mpPlayer->GetPickedBody());

		if (CrossState == eCrossHairState_Active ||
			CrossState == eCrossHairState_Examine) {
			mpPlayer->SetCrossHairState(CrossState);
		} else {
			mpPlayer->SetCrossHairState(eCrossHairState_None);
		}

		// Call entity
		pEntity->PlayerPick();
	}

	/////////////////////////////////////////////////
	// Ray Check, if a body is not yet picked
	if (mpPlayer->GetPickRay()->mpPickedBody == NULL) {
		cVector3f vStart, vEnd;

		vStart = mpPlayer->GetCamera()->GetPosition();
		vEnd = vStart + cMath::Vector3Normalize(vProxyPos - vStart) * mpPlayer->GetPickRay()->mfMaxDistance;

		// gvEnd = vEnd; gvStart = vStart;

		mpPlayer->GetPickRay()->Clear();
		pPhysicsWorld->CastRay(mpPlayer->GetPickRay(), vStart, vEnd, true, false, true);
		mpPlayer->GetPickRay()->CalculateResults();

		// Log("Picked body: %d\n",(size_t)mpPlayer->GetPickedBody());

		if (mpPlayer->GetPickedBody()) {
			iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

			// Set cross hair state
			eCrossHairState CrossState = pEntity->GetPickCrossHairState(mpPlayer->GetPickedBody());

			// TODO: Check here if it is some non physical interaction.
			if ( // CrossState == eCrossHairState_Active || Skip active here...
				CrossState == eCrossHairState_Examine) {
				mpPlayer->SetCrossHairState(CrossState);
				if (CrossState == eCrossHairState_Active) {
					if ((pEntity->GetType() == eGameEntityType_Object &&
						 pEntity->GetCallbackScript(eGameEntityScriptType_PlayerInteract) == NULL)) {
						mpPlayer->SetCrossHairState(eCrossHairState_None);
					}
				}
			} else {
				mpPlayer->SetCrossHairState(eCrossHairState_None);
			}

			// Call entity
			pEntity->PlayerPick();
		} else {
			mpPlayer->SetCrossHairState(eCrossHairState_None);
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::OnStartInteract() {
	// gvFinalEnd = gvEnd;
	// gvFinalStart = gvStart;

	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();
		pEntity->PlayerInteract();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::OnStartExamine() {
	// Log("Picked body: %d\n",(size_t)mpPlayer->GetPickedBody());
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::OnStartRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Walk ||
		mpPlayer->GetMoveState() == ePlayerMoveState_Crouch) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	}
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Run);
	}
}
void cPlayerState_NormalHaptX::OnStopRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	else if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::OnStartCrouch() {
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

void cPlayerState_NormalHaptX::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::OnStartInteractMode() {
	mpPlayer->ChangeState(ePlayerState_InteractMode);
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::EnterState(iPlayerState *apPrevState) {
	mpPlayer->ResetCrossHairPos();
}

//-----------------------------------------------------------------------

void cPlayerState_NormalHaptX::LeaveState(iPlayerState *apNextState) {
	// Can cause crashes!!
	// mpPlayer->GetPickRay()->mpPickedBody = NULL;
}

void cPlayerState_NormalHaptX::OnPostSceneDraw() {
	/*iLowLevelGraphics *pLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();
	pLowGfx->DrawLine(	gvFinalStart,gvFinalEnd,cColor(1,1));
	pLowGfx->DrawSphere(gvFinalStart,0.1f,cColor(1,0,0,1));
	pLowGfx->DrawSphere(gvFinalEnd,0.1f,cColor(0,1,0,1));*/
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// INTERACT MODE STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_InteractModeHaptX::cPlayerState_InteractModeHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_InteractMode) {
	mvLookSpeed = 0;
	mfRange = 15.0f;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractModeHaptX::OnUpdate(float afTimeStep) {
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

void cPlayerState_InteractModeHaptX::OnStartInteract() {
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerInteract();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_InteractModeHaptX::OnStartExamine() {
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_InteractModeHaptX::OnAddYaw(float afVal) {
	/*cInput *pInput = mpInit->mpGame->GetInput();

	if(pInput->IsTriggerd("LookMode"))
	{
		mpPlayer->GetCamera()->AddYaw( -afVal * 2.0f * mpPlayer->GetLookSpeed());
		mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
	}
	else
	{
		if(mpPlayer->AddCrossHairPos(cVector2f(afVal * 800.0f,0)))
		{
			mpPlayer->GetCamera()->AddYaw( -afVal * mpPlayer->GetLookSpeed());
			mpPlayer->GetCharacterBody()->SetYaw(mpPlayer->GetCamera()->GetYaw());
		}

		cVector2f vBorder = mpPlayer->GetInteractMoveBorder();
		cVector2f vPos = mpPlayer->GetCrossHairPos();

		if(vPos.x < vBorder.x + mfRange || vPos.x > (799 - vBorder.x - mfRange))
		{
			float fDist;
			if(vPos.x < vBorder.x + mfRange)
			{
				fDist = (vPos.x - vBorder.x);
				mvLookSpeed.x = 1 - (fDist / mfRange) * 1;
			}
			else
			{
				fDist = ((799 - vBorder.x) - vPos.x);
				mvLookSpeed.x = (1-(fDist / mfRange)) * -1;
			}
		}
		else
		{
			mvLookSpeed.x =0;
		}
	}

	return false;*/
	return true;
}

bool cPlayerState_InteractModeHaptX::OnAddPitch(float afVal) {
	/*cInput *pInput = mpInit->mpGame->GetInput();

	if(pInput->IsTriggerd("LookMode"))
	{
		float fInvert = mpInit->mpButtonHandler->GetInvertMouseY() ? -1.0f : 1.0f;
		mpPlayer->GetCamera()->AddPitch( -afVal *2.0f*fInvert * mpPlayer->GetLookSpeed());
	}
	else
	{
		if(mpPlayer->AddCrossHairPos(cVector2f(0,afVal * 600.0f)))
		{
			mpPlayer->GetCamera()->AddPitch( -afVal * mpPlayer->GetLookSpeed());
		}

		cVector2f vBorder = mpPlayer->GetInteractMoveBorder();
		cVector2f vPos = mpPlayer->GetCrossHairPos();

		if(vPos.y < vBorder.y + mfRange || vPos.y > (599 - vBorder.y - mfRange))
		{
			float fDist;
			if(vPos.y < vBorder.y + mfRange)
			{
				fDist = (vPos.y - vBorder.y);
				mvLookSpeed.y = 1 - (fDist / mfRange) * 1;
			}
			else
			{
				fDist = ((599 - vBorder.y) - vPos.y);
				mvLookSpeed.y = (1-(fDist / mfRange)) * -1;
			}
		}
		else
		{
			mvLookSpeed.y =0;
		}
	}

	return false;*/
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractModeHaptX::OnStartInteractMode() {
	mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

void cPlayerState_InteractModeHaptX::EnterState(iPlayerState *apPrevState) {
	mPrevMoveState = mpPlayer->GetMoveState();

	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);

	mvLookSpeed = 0;
}

//-----------------------------------------------------------------------

void cPlayerState_InteractModeHaptX::LeaveState(iPlayerState *apNextState) {
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

void cPlayerState_InteractModeHaptX::OnStartCrouch() {
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

void cPlayerState_InteractModeHaptX::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// USE ITEM STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_UseItemHaptX::cPlayerState_UseItemHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_UseItem) {
}

//-----------------------------------------------------------------------

void cPlayerState_UseItemHaptX::OnUpdate(float afTimeStep) {
	iLowLevelHaptic *pLowLevelHaptic = mpInit->mpGame->GetHaptic()->GetLowLevel();

	/////////////////////////////////////////////////
	// Move cross hair
	cVector2f vCrossPos = pLowLevelHaptic->GetProxyScreenPos(cVector2f(800, 600));
	mpPlayer->SetCrossHairPos(vCrossPos);

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

void cPlayerState_UseItemHaptX::OnStartInteract() {
	iPhysicsBody *pBody = mpPlayer->GetPickedBody();
	iGameEntity *pEntity = NULL;
	if (pBody)
		pEntity = (iGameEntity *)pBody->GetUserData();

	if (pEntity && mpPlayer->GetPickedDist() <= pEntity->GetMaxExamineDist()) {
		if (mpPlayer->GetPickedDist() <= pEntity->GetMaxInteractDist()) {
			iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();
			cGameItemType *pType = mpInit->mpInventory->GetItemType(mpPlayer->GetCurrentItem()->GetItemType());

			if (mPrevState == ePlayerState_WeaponMelee ||
				mPrevState == ePlayerState_Throw) {
				mpPlayer->ChangeState(ePlayerState_Normal);
			} else {
				mpPlayer->ChangeState(mPrevState);
			}

			pType->OnUse(mpPlayer->GetCurrentItem(), pEntity);
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

void cPlayerState_UseItemHaptX::OnStartExamine() {
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

bool cPlayerState_UseItemHaptX::OnAddYaw(float afVal) {
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

bool cPlayerState_UseItemHaptX::OnAddPitch(float afVal) {
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

void cPlayerState_UseItemHaptX::EnterState(iPlayerState *apPrevState) {
	mPrevMoveState = mpPlayer->GetMoveState();
	mPrevState = apPrevState->mType;

	mpPlayer->SetCrossHairState(eCrossHairState_Item);

	mpPlayer->GetHapticCamera()->SetHandVisible(false);
}

//-----------------------------------------------------------------------

void cPlayerState_UseItemHaptX::LeaveState(iPlayerState *apNextState) {
	mpPlayer->SetCrossHairState(eCrossHairState_None);

	mpPlayer->GetHapticCamera()->SetHandVisible(true);
}

//-----------------------------------------------------------------------

void cPlayerState_UseItemHaptX::OnStartCrouch() {
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

void cPlayerState_UseItemHaptX::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_UseItemHaptX::OnStartInventory() {
	mpPlayer->ChangeState(mPrevState);
	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MESSAGE STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_MessageHaptX::cPlayerState_MessageHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Message) {
}

//-----------------------------------------------------------------------

void cPlayerState_MessageHaptX::OnUpdate(float afTimeStep) {
}

//-----------------------------------------------------------------------

bool cPlayerState_MessageHaptX::OnJump() {
	return false;
}

//-----------------------------------------------------------------------

void cPlayerState_MessageHaptX::OnStartInteract() {
	mpInit->mpGameMessageHandler->ShowNext();
}

void cPlayerState_MessageHaptX::OnStopInteract() {
}

//-----------------------------------------------------------------------

void cPlayerState_MessageHaptX::OnStartExamine() {
	mpInit->mpGameMessageHandler->ShowNext();
}

//-----------------------------------------------------------------------

bool cPlayerState_MessageHaptX::OnMoveForwards(float afMul, float afTimeStep) { return false; }
bool cPlayerState_MessageHaptX::OnMoveSideways(float afMul, float afTimeStep) { return false; }

//-----------------------------------------------------------------------

bool cPlayerState_MessageHaptX::OnAddYaw(float afVal) { return false; }
bool cPlayerState_MessageHaptX::OnAddPitch(float afVal) { return false; }

//-----------------------------------------------------------------------

void cPlayerState_MessageHaptX::EnterState(iPlayerState *apPrevState) {
	// Change move state so the player is still
	mPrevMoveState = mpPlayer->GetMoveState();
	// mpPlayer->ChangeMoveState(ePlayerMoveState_Still);

	mpPlayer->SetCrossHairState(eCrossHairState_None);
	mpPlayer->GetHapticCamera()->SetHandVisible(false);
	mpPlayer->GetHapticCamera()->SetRenderActive(false);
}

//-----------------------------------------------------------------------

void cPlayerState_MessageHaptX::LeaveState(iPlayerState *apNextState) {
	if (mPrevMoveState != ePlayerMoveState_Run && mPrevMoveState != ePlayerMoveState_Jump)
		mpPlayer->ChangeMoveState(mPrevMoveState);
	else
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	mpPlayer->GetHapticCamera()->SetHandVisible(true);
	mpPlayer->GetHapticCamera()->SetRenderActive(true);
}

//-----------------------------------------------------------------------

bool cPlayerState_MessageHaptX::OnStartInventory() {
	return false;
}

//-----------------------------------------------------------------------

bool cPlayerState_MessageHaptX::OnStartInventoryShortCut(int alNum) {
	return false;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CLIMB STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_ClimbHaptX::cPlayerState_ClimbHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Climb) {
	mpLadder = NULL;

	mfUpSpeed = mpInit->mpGameConfig->GetFloat("Movement_Climb", "UpSpeed", 0);
	mfDownSpeed = mpInit->mpGameConfig->GetFloat("Movement_Climb", "DownSpeed", 0);

	mfStepLength = mpInit->mpGameConfig->GetFloat("Movement_Climb", "StepLength", 0);

	mfStepCount = 0;

	mlState = 0;
}

//-----------------------------------------------------------------------

void cPlayerState_ClimbHaptX::PlaySound(const tString &asSound) {
	if (asSound == "")
		return;

	iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

	cSoundEntity *pSound = pWorld->CreateSoundEntity("LadderStep", asSound, true);
	if (pSound)
		pSound->SetPosition(pCharBody->GetPosition());
}

//-----------------------------------------------------------------------

void cPlayerState_ClimbHaptX::EnterState(iPlayerState *apPrevState) {
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

	// Haptic
	mpPlayer->GetHapticCamera()->SetRenderActive(false);
	mpPlayer->GetHapticCamera()->SetType(eHapticGameCameraType_Centre);
	mpPlayer->GetHapticCamera()->SetHandVisible(false);
}

//-----------------------------------------------------------------------

void cPlayerState_ClimbHaptX::LeaveState(iPlayerState *apNextState) {
	mpPlayer->SetCrossHairState(eCrossHairState_None);

	mpPlayer->GetCharacterBody()->SetGravityActive(true);

	mpPlayer->GetCamera()->SetPitchLimits(mvPrevPitchLimits);
	mpPlayer->GetCamera()->SetYawLimits(cVector2f(0, 0));

	// Haptic
	mpPlayer->GetHapticCamera()->SetRenderActive(true);
	mpPlayer->GetHapticCamera()->SetType(eHapticGameCameraType_Frame);
	mpPlayer->GetHapticCamera()->SetHandVisible(true);
}

//-----------------------------------------------------------------------

void cPlayerState_ClimbHaptX::OnUpdate(float afTimeStep) {
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

void cPlayerState_ClimbHaptX::OnStartInteract() {
	if (mlState != 0)
		mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

void cPlayerState_ClimbHaptX::OnStartExamine() {
	if (mlState != 0)
		mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

bool cPlayerState_ClimbHaptX::OnAddYaw(float afVal) {
	if (mlState != 0)
		mpPlayer->GetCamera()->AddYaw(-afVal * mpPlayer->GetLookSpeed());

	return false;
}

bool cPlayerState_ClimbHaptX::OnAddPitch(float afVal) {
	// if(mlState!=0)
	//	mpPlayer->GetCamera()->AddPitch( -afVal * mpPlayer->GetLookSpeed());
	if (mlState == 0)
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_ClimbHaptX::OnMoveForwards(float afMul, float afTimeStep) {
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
bool cPlayerState_ClimbHaptX::OnMoveSideways(float afMul, float afTimeStep) {
	iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();

	return false;
}

//-----------------------------------------------------------------------

void cPlayerState_ClimbHaptX::OnStartCrouch() {
}

void cPlayerState_ClimbHaptX::OnStopCrouch() {
}

//-----------------------------------------------------------------------

bool cPlayerState_ClimbHaptX::OnJump() {
	mpPlayer->ChangeState(ePlayerState_Normal);
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_ClimbHaptX::OnStartInventory() {
	return false;
}

//-----------------------------------------------------------------------
