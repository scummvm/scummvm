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

#include "hpl1/penumbra-overture/PlayerState_Weapon.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/HudModel_Throw.h"
#include "hpl1/penumbra-overture/HudModel_Weapon.h"

//////////////////////////////////////////////////////////////////////////
// THROW STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_Throw::cPlayerState_Throw(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Throw) {
	mfLastForward = 1.0f;
	mfLastSideways = 1.0f;

	mpHudObject = NULL;
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnUpdate(float afTimeStep) {
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

	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		// Set cross hair state
		eCrossHairState CrossState = pEntity->GetPickCrossHairState(mpPlayer->GetPickedBody());
		if (CrossState != eCrossHairState_None && pEntity->GetDescription() != _W("") &&
			!pEntity->GetHasBeenExamined()) {
			mpPlayer->SetCrossHairState(eCrossHairState_Examine);
		} else {
			mpPlayer->SetCrossHairState(eCrossHairState_None);
		}

		// Call entity
		// pEntity->PlayerPick();
	} else {
		mpPlayer->SetCrossHairState(eCrossHairState_None);
	}

	// Quick fix but works, the OnLeave of the previous states sets an old cross hair state.
	// mpPlayer->SetCrossHairState(eCrossHairState_None);
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnDraw() {
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnPostSceneDraw() {
	// mpHudObject->PostSceneDraw();
}

//-----------------------------------------------------------------------

bool cPlayerState_Throw::OnJump() {
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnStartInteractMode() {
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnStartInteract() {
	mpHudObject->OnAttackDown();
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnStopInteract() {
	mpHudObject->OnAttackUp();
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnStartExamine() {
	// mpInit->mpPlayerHands->SetCurrentModel(1,"");
	// mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::OnStartHolster() {
	mpInit->mpPlayerHands->SetCurrentModel(1, "");
	mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

bool cPlayerState_Throw::OnAddYaw(float afVal) {
	return true; // mpHudObject->OnMouseMove(cVector2f(afVal,0));
}

bool cPlayerState_Throw::OnAddPitch(float afVal) {
	return true; // return mpHudObject->OnMouseMove(cVector2f(0,afVal));
}

//-----------------------------------------------------------------------

bool cPlayerState_Throw::OnMoveForwards(float afMul, float afTimeStep) {
	mfLastForward = afMul;
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_Throw::OnMoveSideways(float afMul, float afTimeStep) {
	mfLastSideways = afMul;
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::EnterState(iPlayerState *apPrevState) {
	mpHudObject->OnStart();
}

//-----------------------------------------------------------------------

void cPlayerState_Throw::LeaveState(iPlayerState *apNextState) {
	if (apNextState->mType != ePlayerState_Message &&
		apNextState->mType != ePlayerState_WeaponMelee &&
		mpInit->mpPlayerHands->GetCurrentModel(1) == mpHudObject) {
		mpInit->mpPlayerHands->SetCurrentModel(1, "");
	}
}

//-----------------------------------------------------------------------

// No running when you have a weapon:
void cPlayerState_Throw::OnStartRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Walk)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Run);
	}
}
void cPlayerState_Throw::OnStopRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	else if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}
//-----------------------------------------------------------------------

void cPlayerState_Throw::OnStartCrouch() {
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

void cPlayerState_Throw::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_Throw::OnStartInventory() {
	// mpPlayer->ChangeState(ePlayerState_Normal);
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_Throw::OnStartInventoryShortCut(int alNum) {
	return true;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WEAPON MELEE STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_WeaponMelee::cPlayerState_WeaponMelee(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_WeaponMelee) {
	mfLastForward = 1.0f;
	mfLastSideways = 1.0f;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnUpdate(float afTimeStep) {
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

	if (mpPlayer->GetPickedBody() && mpHudWeapon->IsAttacking() == false) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		// Set cross hair state
		eCrossHairState CrossState = pEntity->GetPickCrossHairState(mpPlayer->GetPickedBody());
		if (CrossState != eCrossHairState_None && pEntity->GetDescription() != _W("") &&
			!pEntity->GetHasBeenExamined()) {
			mpPlayer->SetCrossHairState(eCrossHairState_Examine);
		} else {
			mpPlayer->SetCrossHairState(eCrossHairState_None);
		}

		// Call entity
		// pEntity->PlayerPick();
	} else {
		mpPlayer->SetCrossHairState(eCrossHairState_None);
	}

	// Quick fix but works, the OnLeave of the previous states sets an old cross hair state.
	// mpPlayer->SetCrossHairState(eCrossHairState_None);
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnDraw() {
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnPostSceneDraw() {
	mpHudWeapon->PostSceneDraw();
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMelee::OnJump() {
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnStartInteractMode() {
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnStartInteract() {
	mpHudWeapon->OnAttackDown();
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnStopInteract() {
	mpHudWeapon->OnAttackUp();
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnStartExamine() {
	// mpPlayer->ChangeState(ePlayerState_Normal);
	if (mpPlayer->GetPickedBody() && mpHudWeapon->IsAttacking() == false) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnStartHolster() {
	mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMelee::OnAddYaw(float afVal) {
	return mpHudWeapon->OnMouseMove(cVector2f(afVal, 0));
}

bool cPlayerState_WeaponMelee::OnAddPitch(float afVal) {
	return mpHudWeapon->OnMouseMove(cVector2f(0, afVal));
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMelee::OnMoveForwards(float afMul, float afTimeStep) {
	/*cInput *pInput = mpInit->mpGame->GetInput();
	if(pInput->IsTriggerd("Run"))
	{
		if(mfLastForward == 0.0f && afMul != 0.0f && mpPlayer->GetCharacterBody()->IsOnGround())
		{
			cVector3f vFwd = mpPlayer->GetCharacterBody()->GetForward();
			cVector3f vDir = afMul>0 ? vFwd : vFwd * -1.0f;

			mpPlayer->GetCharacterBody()->AddForce( vDir * 300 * mpPlayer->GetDefaultMass() +
				cVector3f(0,200 * mpPlayer->GetDefaultMass() ,0));
		}

		mfLastForward = afMul;
		return false;
	}
	else*/
	{
		mfLastForward = afMul;
		return true;
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMelee::OnMoveSideways(float afMul, float afTimeStep) {
	/*cInput *pInput = mpInit->mpGame->GetInput();
	if(pInput->IsTriggerd("Run"))
	{
		if(mfLastSideways == 0.0f && afMul != 0.0f && mpPlayer->GetCharacterBody()->IsOnGround())
		{
			cVector3f vRight = mpPlayer->GetCharacterBody()->GetRight();
			cVector3f vDir = afMul>0 ? vRight : vRight * -1.0f;

			mpPlayer->GetCharacterBody()->AddForce( vDir * 400 * mpPlayer->GetDefaultMass() +
													cVector3f(0,200 * mpPlayer->GetDefaultMass() ,0));
		}
		mfLastSideways = afMul;
		return false;
	}
	else*/
	{
		mfLastSideways = afMul;
		return true;
	}
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::EnterState(iPlayerState *apPrevState) {
	// get the callback
	/*mpWeaponCallback = mpPlayer->GetWeaponCallback();
	if(mpWeaponCallback==NULL)
	{
	Error("No weapon callback for player weapon state!\n");
	mpPlayer->ChangeState(ePlayerState_Normal);
	}

	//Load the data
	cCamera3D *pCamera = mpPlayer->GetCamera();
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	cResources *pResources = mpInit->mpGame->GetResources();

	cMesh* pMesh = pResources->GetMeshManager()->CreateMesh(mpWeaponCallback->GetModelFile().c_str());
	if(pMesh==NULL)
	{
	Error("Couldn't load mesh %s for player weapon state!\n",mpWeaponCallback->GetModelFile().c_str());
	mpPlayer->ChangeState(ePlayerState_Normal);
	}

	mpMeshEntity = pWorld->CreateMeshEntity("Weapon",pMesh);

	m_mtxOffset = mpWeaponCallback->GetOffsetMatrix();

	mpMeshEntity->SetMatrix(cMath::MatrixMul(cMath::MatrixInverse(pCamera->GetViewMatrix()),m_mtxOffset));
	mpMeshEntity->SetPosition(pCamera->GetPosition() + pCamera->GetForward());

	mpPlayer->SetCrossHairState(eCrossHairState_None);*/

	// Try not doing this.
	// mpPlayer->GetGlowStick()->SetActive(false);
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::LeaveState(iPlayerState *apNextState) {
	if (apNextState->mType != ePlayerState_Message &&
		apNextState->mType != ePlayerState_Throw &&
		mpInit->mpPlayerHands->GetCurrentModel(1) == mpHudWeapon) {
		mpInit->mpPlayerHands->SetCurrentModel(1, "");
	}
}

//-----------------------------------------------------------------------

// No running when you have a weapon:
void cPlayerState_WeaponMelee::OnStartRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Walk)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Run);
	}
}
void cPlayerState_WeaponMelee::OnStopRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	else if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}
//-----------------------------------------------------------------------

void cPlayerState_WeaponMelee::OnStartCrouch() {
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

void cPlayerState_WeaponMelee::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMelee::OnStartInventory() {
	// mpPlayer->ChangeState(ePlayerState_Normal);
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMelee::OnStartInventoryShortCut(int alNum) {
	return true;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WEAPON STATE
//////////////////////////////////////////////////////////////////////////

/*class cPlayerState_Weapon : public iPlayerState
{
public:
iPlayerWeaponCallback *mpWeaponCallback;
cMatrixf m_mtxOffset;
cMeshEntity *mpMeshEntity;

cVector3f mvPosition;

cPlayerState_Weapon(cInit *apInit,cPlayer *apPlayer) : iPlayerState(apInit,apPlayer,ePlayerState_Weapon)
{
mvPosition = cVector3f(0.13f, -0.16f, 0.2f);
}

//-----------------------------------------------------------------------


void OnUpdate(float afTimeStep)
{
//Quick fix but works, the OnLeave of the previous states sets an old cross hair state.
mpPlayer->SetCrossHairState(eCrossHairState_None);

cCamera3D *pCamera = mpPlayer->GetCamera();

cMatrixf mtxInv = cMath::MatrixInverse(pCamera->GetViewMatrix());
mpMeshEntity->SetMatrix(cMath::MatrixMul(mtxInv,m_mtxOffset));

cVector3f vLocalPos = mvPosition + cVector3f(0,-mpPlayer->GetHeadMove()->GetPos()*0.1f,0);

cVector3f vPos =	pCamera->GetForward() * vLocalPos.z +
pCamera->GetUp() * vLocalPos.y +
pCamera->GetRight() * vLocalPos.x;

mpMeshEntity->SetPosition(pCamera->GetPosition() + vPos);
}

//-----------------------------------------------------------------------

void OnDraw()
{

}

//-----------------------------------------------------------------------


void OnPostSceneDraw()
{
}

//-----------------------------------------------------------------------

bool OnJump()
{
return true;
}

//-----------------------------------------------------------------------

void OnStartInteractMode()
{
//Do nothing
}

//-----------------------------------------------------------------------

void OnStartInteract()
{
mpWeaponCallback->OnAttackDown();
}

//-----------------------------------------------------------------------

void OnStopInteract()
{
mpWeaponCallback->OnAttackUp();
}

//-----------------------------------------------------------------------

void OnStartExamine()
{
mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

bool OnAddYaw(float afVal)
{
return true;
}

bool OnAddPitch(float afVal)
{
return true;
}

//-----------------------------------------------------------------------

bool OnMoveForwards(float afMul, float afTimeStep)
{
return true;
}

//-----------------------------------------------------------------------

bool OnMoveSideways(float afMul, float afTimeStep)
{
return true;
}

//-----------------------------------------------------------------------

void EnterState(iPlayerState* apPrevState)
{
//get the callback
mpWeaponCallback = mpPlayer->GetWeaponCallback();
if(mpWeaponCallback==NULL)
{
Error("No weapon callback for player weapon state!\n");
mpPlayer->ChangeState(ePlayerState_Normal);
}

//Load the data
cCamera3D *pCamera = mpPlayer->GetCamera();
cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
cResources *pResources = mpInit->mpGame->GetResources();

cMesh* pMesh = pResources->GetMeshManager()->CreateMesh(mpWeaponCallback->GetModelFile().c_str());
if(pMesh==NULL)
{
Error("Couldn't load mesh %s for player weapon state!\n",mpWeaponCallback->GetModelFile().c_str());
mpPlayer->ChangeState(ePlayerState_Normal);
}

mpMeshEntity = pWorld->CreateMeshEntity("Weapon",pMesh);

m_mtxOffset = mpWeaponCallback->GetOffsetMatrix();

mpMeshEntity->SetMatrix(cMath::MatrixMul(cMath::MatrixInverse(pCamera->GetViewMatrix()),m_mtxOffset));
mpMeshEntity->SetPosition(pCamera->GetPosition() + pCamera->GetForward());

mpPlayer->SetCrossHairState(eCrossHairState_None);

//Try not doing this.
//mpPlayer->GetGlowStick()->SetActive(false);
}

//-----------------------------------------------------------------------

void LeaveState(iPlayerState* apNextState)
{
cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();

pWorld->DestroyMeshEntity(mpMeshEntity);

//Skipping this...
//mpPlayer->SetWeaponCallback(NULL);
}

//-----------------------------------------------------------------------

// No running when you have a weapon:
void OnStartRun()
{
//if(mpPlayer->GetMoveState() == ePlayerMoveState_Walk)
//	mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
}
void OnStopRun()
{
//if(mpPlayer->GetMoveState() == ePlayerMoveState_Run)
//	mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
//else if(mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
//	mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}
//-----------------------------------------------------------------------

void OnStartCrouch()
{
if(mpPlayer->GetMoveState() == ePlayerMoveState_Jump)return;

if(mpInit->mpButtonHandler->GetToggleCrouch())
{
if(mpPlayer->GetMoveState() == ePlayerMoveState_Crouch)
mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
else
mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
}
else
{
mpPlayer->ChangeMoveState(ePlayerMoveState_Crouch);
}
}

void OnStopCrouch()
{
if(mpInit->mpButtonHandler->GetToggleCrouch()==false)
{
mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
}
}

//-----------------------------------------------------------------------

bool OnStartInventory()
{
mpPlayer->ChangeState(ePlayerState_Normal);
return true;
}

//-----------------------------------------------------------------------

bool OnStartInventoryShortCut(int alNum)
{
return false;
}
//-----------------------------------------------------------------------
};*/
