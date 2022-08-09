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

#include "hpl1/penumbra-overture/PlayerState_WeaponHaptX.h"
#include "hpl1/engine/engine.h"

#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/HudModel_Throw.h"
#include "hpl1/penumbra-overture/HudModel_Weapon.h"

#include "hpl1/penumbra-overture/HapticGameCamera.h"

#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/MapHandler.h"

#include <list>

//////////////////////////////////////////////////////////////////////////
// THROW STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_ThrowHaptX::cPlayerState_ThrowHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_Throw) {
	mfLastForward = 1.0f;
	mfLastSideways = 1.0f;

	mpHudObject = NULL;
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnUpdate(float afTimeStep) {
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

void cPlayerState_ThrowHaptX::OnDraw() {
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnPostSceneDraw() {
	// mpHudObject->PostSceneDraw();
}

//-----------------------------------------------------------------------

bool cPlayerState_ThrowHaptX::OnJump() {
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnStartInteractMode() {
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnStartInteract() {
	mpHudObject->OnAttackDown();
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnStopInteract() {
	mpHudObject->OnAttackUp();
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnStartExamine() {
	// mpInit->mpPlayerHands->SetCurrentModel(1,"");
	// mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
	if (mpPlayer->GetPickedBody()) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnStartHolster() {
	mpInit->mpPlayerHands->SetCurrentModel(1, "");
	mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
}

//-----------------------------------------------------------------------

bool cPlayerState_ThrowHaptX::OnAddYaw(float afVal) {
	return true; // mpHudObject->OnMouseMove(cVector2f(afVal,0));
}

bool cPlayerState_ThrowHaptX::OnAddPitch(float afVal) {
	return true; // return mpHudObject->OnMouseMove(cVector2f(0,afVal));
}

//-----------------------------------------------------------------------

bool cPlayerState_ThrowHaptX::OnMoveForwards(float afMul, float afTimeStep) {
	mfLastForward = afMul;
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_ThrowHaptX::OnMoveSideways(float afMul, float afTimeStep) {
	mfLastSideways = afMul;
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::EnterState(iPlayerState *apPrevState) {
	mpHudObject->OnStart();

	// Haptic
	mpPlayer->GetHapticCamera()->SetRenderActive(false);
	mpPlayer->GetHapticCamera()->SetType(eHapticGameCameraType_Centre);
	mpPlayer->GetHapticCamera()->SetHandVisible(false);
}

//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::LeaveState(iPlayerState *apNextState) {
	if (apNextState->mType != ePlayerState_Message &&
		apNextState->mType != ePlayerState_WeaponMelee &&
		mpInit->mpPlayerHands->GetCurrentModel(1) == mpHudObject) {
		mpInit->mpPlayerHands->SetCurrentModel(1, "");
	}

	// Haptic
	mpPlayer->GetHapticCamera()->SetRenderActive(true);
	mpPlayer->GetHapticCamera()->SetType(eHapticGameCameraType_Frame);
	mpPlayer->GetHapticCamera()->SetHandVisible(true);
}

//-----------------------------------------------------------------------

// No running when you have a weapon:
void cPlayerState_ThrowHaptX::OnStartRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Walk)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Run);
	}
}
void cPlayerState_ThrowHaptX::OnStopRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	else if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}
//-----------------------------------------------------------------------

void cPlayerState_ThrowHaptX::OnStartCrouch() {
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

void cPlayerState_ThrowHaptX::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_ThrowHaptX::OnStartInventory() {
	// mpPlayer->ChangeState(ePlayerState_Normal);
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_ThrowHaptX::OnStartInventoryShortCut(int alNum) {
	return true;
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// WEAPON MELEE STATE
//////////////////////////////////////////////////////////////////////////

cPlayerState_WeaponMeleeHaptX::cPlayerState_WeaponMeleeHaptX(cInit *apInit, cPlayer *apPlayer) : iPlayerState(apInit, apPlayer, ePlayerState_WeaponMelee) {
	mfLastForward = 1.0f;
	mfLastSideways = 1.0f;

	mpLowLevelHaptic = mpInit->mpGame->GetHaptic()->GetLowLevel();

	mpSlowdownForce = mpLowLevelHaptic->CreateViscosityForce(0, 6, 0.5f, 50.0f);
	mpSlowdownForce->SetActive(false);

	mbInteractDown = false;

	mpFeedbackForce = mpLowLevelHaptic->CreateImpulseForce(0);
	mpFeedbackForce->SetActive(false);

	mpImpulseForce = mpLowLevelHaptic->CreateImpulseForce(0);
	mpImpulseForce->SetActive(false);

	mbCollided = false;
}

//-----------------------------------------------------------------------

static cVector3f gvPrevPos;
static bool gbFirstTime = true;

static std::list<cVector3f> glsVelList;

void cPlayerState_WeaponMeleeHaptX::OnUpdate(float afTimeStep) {
	/////////////////////////////////////
	// Set crosshair to centre of screen
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

	///////////////////////////////////////////////
	// Calculate proxy movement speed
	mpLowLevelHaptic = mpInit->mpGame->GetHaptic()->GetLowLevel();

	cVector3f vMovement = mpLowLevelHaptic->GetHardwarePosition();
	if (gbFirstTime) {
		gvPrevPos = vMovement;
		gbFirstTime = false;
	}

	cVector3f vVelocity = (vMovement - gvPrevPos);
	vVelocity = cVector3f(vVelocity.x, vVelocity.y, vVelocity.z);

	glsVelList.push_back(vVelocity);
	// Change here to let the average speed consist of more samples
	if (glsVelList.size() > 10)
		glsVelList.pop_front();

	cVector3f vAvg(0);
	for (std::list<cVector3f>::iterator it = glsVelList.begin(); it != glsVelList.end(); ++it) {
		vAvg += *it;
	}
	vAvg = vAvg / (float)glsVelList.size();

	///////////////////////////////////////////////
	// If button is pressed then turn weapon according to speed of proxy
	if (mbInteractDown) {
		// Log("Avg: %s\n",vAvg.ToString().c_str());

		// mvGoalRotation.z =cMath::ToRad(0);
		bool bXSet = true;
		if (vAvg.x > 0.75f) {
			mvGoalRotation.y = cMath::ToRad(65.0f);
			mvGoalRotation.z = cMath::ToRad(90.0f);
		} else if (vAvg.x < -0.75f) {
			mvGoalRotation.y = cMath::ToRad(-65.0f);
			mvGoalRotation.z = cMath::ToRad(90.0f);
		} else {
			mvGoalRotation.y = cMath::ToRad(0.0f);
			bXSet = false;
		}

		if (vAvg.y > 0.75f) {
			mvGoalRotation.x = cMath::ToRad(-90.0f - 50.0f);
			if (bXSet) {
				if (mvGoalRotation.y > 0)
					mvGoalRotation.z = cMath::ToRad(45.0f);
				if (mvGoalRotation.y < 0)
					mvGoalRotation.z = cMath::ToRad(135.0f);
			} else {
				mvGoalRotation.z = 0;
			}
		} else if (vAvg.y < -0.75f) {
			mvGoalRotation.x = cMath::ToRad(-90.0f + 50.0f);
			if (bXSet) {
				if (mvGoalRotation.y < 0)
					mvGoalRotation.z = cMath::ToRad(45.0f);
				if (mvGoalRotation.y > 0)
					mvGoalRotation.z = cMath::ToRad(135.0f);
			} else {
				mvGoalRotation.z = 0;
			}
		} else {
			mvGoalRotation.x = cMath::ToRad(-90.0f);
		}
	}

	///////////////////////////////////////////////
	// Calculate weapon and collider matrix
	cVector3f vProxyPos = mpPlayer->GetHapticCamera()->GetHandEntity()->GetWorldPosition();
	float fYaw = mpPlayer->GetCamera()->GetYaw();

	cMatrixf mtxWeapon = cMath::MatrixScale(mpHudWeapon->GetHapticScale());

	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotate(mpHudWeapon->GetHapticRot(), eEulerRotationOrder_XYZ), mtxWeapon);

	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotate(cVector3f(0, mvRotation.z, 0), eEulerRotationOrder_YXZ), mtxWeapon);
	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotate(cVector3f(mvRotation.x, mvRotation.y, 0), eEulerRotationOrder_XZY), mtxWeapon);

	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotateY(fYaw), mtxWeapon);
	mtxWeapon = cMath::MatrixMul(cMath::MatrixTranslate(vProxyPos), mtxWeapon);

	mpHudWeapon->GetEntity()->SetMatrix(mtxWeapon);

	cMatrixf mtxCollider = cMatrixf::Identity;

	mtxCollider = cMath::MatrixMul(cMath::MatrixRotate(cVector3f(0, mvRotation.z, 0), eEulerRotationOrder_YXZ), mtxCollider);
	mtxCollider = cMath::MatrixMul(cMath::MatrixRotate(cVector3f(mvRotation.x, mvRotation.y, 0), eEulerRotationOrder_XZY), mtxCollider);

	mtxCollider = cMath::MatrixMul(cMath::MatrixRotateY(fYaw), mtxCollider);
	mtxCollider = cMath::MatrixMul(cMath::MatrixTranslate(vProxyPos), mtxCollider);

	// mpBody->SetMatrix(mtxCollider);

	///////////////////////////////////////////////
	// Update angles and turn towards goal
	// Y
	float fAngleStep = cMath::ToRad(90.0f * afTimeStep * 1.2f);
	if (mvGoalRotation.y != mvRotation.y) {
		mvRotation.y = cMath::TurnAngleRad(mvRotation.y, mvGoalRotation.y, fAngleStep);
	}
	// X
	fAngleStep = cMath::ToRad(90.0f * afTimeStep * 1.2f);
	if (mvGoalRotation.x != mvRotation.x) {
		mvRotation.x = cMath::TurnAngleRad(mvRotation.x, mvGoalRotation.x, fAngleStep);
	}
	// Z
	fAngleStep = cMath::ToRad(90.0f * afTimeStep * 5.2f);
	if (mvGoalRotation.z != mvRotation.z) {
		mvRotation.z = cMath::TurnAngleRad(mvRotation.z, mvGoalRotation.z, fAngleStep);
	}

	///////////////////////////////////////////////
	// Make an attack
	float fSpeed = vAvg.Length();          // This speed is used to
	if (mbInteractDown &&                  // Interaction must be down
		(fSpeed > 0.5 || vAvg.z > 0.3f) && // The minimum speed required for attack (z needs to be a bit lower)
		mfAttackCount <= 0 &&              // Do not allow a newer attack too soon after
		mbCollided == false                // Makes sure that something that just collided can not make an attack, this removes double hits, but
							// may skips some hits too.
	) {
		if (CheckAttack(mtxCollider, cMath::Min(fSpeed * 0.3f, 1.0f))) {
			mfAttackCount = 0.5f;

			// Force at hit, removed for now.
			// if(mpFeedbackForce->IsActive())	mpFeedbackForce->SetActive(false);
			// cVector3f vDir = cMath::Vector3Normalize(vAvg);
			// mpFeedbackForce->SetForce(vDir * -1.0f * fSpeed * 0.08f);
			// mpFeedbackForce->SetTimeControl(false, 0.1f,0,0,0.1f);
			// mpFeedbackForce->SetActive(true);
		}
	}

	if (mfAttackCount > 0)
		mfAttackCount -= afTimeStep;

	///////////////////////////////////////////////
	// Check of collsion with any body and add force if there is.
	// this so weapon does not go into objects.
	cCamera3D *pCamera = mpInit->mpPlayer->GetCamera();
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	cMatrixf mtxProxy = cMath::MatrixRotate(cVector3f(-pCamera->GetPitch(), -pCamera->GetYaw(),
													  -pCamera->GetRoll()),
											eEulerRotationOrder_YXZ);

	if (mbInteractDown) {
		cVector3f vNewPos = mtxCollider.GetTranslation();
		bool bCollide = pPhysicsWorld->CheckShapeWorldCollision(&vNewPos, mpCollider, mtxCollider, NULL, false, false, NULL, false);
		if (bCollide && vNewPos != mtxWeapon.GetTranslation()) {
			cVector3f vDiff = vNewPos - mtxCollider.GetTranslation();

			float fMaxDiffLength = 0.1f;
			if (vDiff.Length() > fMaxDiffLength) {
				vDiff.Normalise();
				vDiff = vDiff * fMaxDiffLength;
			}

			mvPushBackVec = vDiff;
			mvPushBackPos = mtxCollider.GetTranslation();

			vDiff = cMath::MatrixMul(mtxProxy, vDiff);
			mpImpulseForce->SetForce(cMath::Vector3Normalize(vDiff) * 2);

			// Log("Collide %s\n",vDiff.ToString().c_str());
			mbCollided = true; // Set so that we must be out of collision before making an attack.
		} else {
			mbCollided = false;

			mvPushBackVec = 0;
			mpImpulseForce->SetForce(0);
			// mpImpulseForce->SetActive(false);
		}
	} else {
		mvPushBackVec = 0;
		mpImpulseForce->SetForce(0);
		mbCollided = false;
	}

	/////////////////////////////////////////////////
	// Cast Ray, To see if anything is picked. So you can still look at stuff
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

	//////////////////////////////////////
	// Set the previous movement
	gvPrevPos = vMovement;

	// Quick fix but works, the OnLeave of the previous states sets an old cross hair state.
	// mpPlayer->SetCrossHairState(eCrossHairState_None);
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnDraw() {
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnPostSceneDraw() {
	// Comment out this to see debug info!
	return;

	cCamera3D *pCamera = static_cast<cCamera3D *>(mpInit->mpGame->GetScene()->GetCamera());

	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawLine(mvPushBackPos, mvPushBackPos + mvPushBackVec, cColor(1, 1));

	cVector3f vProxyPos = mpPlayer->GetHapticCamera()->GetHandEntity()->GetWorldPosition();
	float fYaw = mpPlayer->GetCamera()->GetYaw();
	cVector3f vSize = mpCollider->GetSize();

	cMatrixf mtxWeapon = cMatrixf::Identity; // cMath::MatrixTranslate(cVector3f(0,vSize.y*0.5f,0));

	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotate(cVector3f(0, mvRotation.z, 0), eEulerRotationOrder_YXZ), mtxWeapon);
	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotate(cVector3f(mvRotation.x, mvRotation.y, 0), eEulerRotationOrder_XZY), mtxWeapon);

	mtxWeapon = cMath::MatrixMul(cMath::MatrixRotateY(fYaw), mtxWeapon);
	mtxWeapon = cMath::MatrixMul(cMath::MatrixTranslate(vProxyPos), mtxWeapon);

	cMatrixf mtxCollider = cMath::MatrixMul(pCamera->GetViewMatrix(), mtxWeapon);

	cBoundingVolume boundingVolume = mpCollider->GetBoundingVolume();
	boundingVolume.SetTransform(cMath::MatrixMul(mtxWeapon, boundingVolume.GetTransform()));

	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(boundingVolume.GetMax(), boundingVolume.GetMin(), cColor(0, 0, 1, 1));

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, mtxCollider);
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawBoxMaxMin(vSize * 0.5f, vSize * -0.5f, cColor(0, 1, 0, 1));

	mpInit->mpGame->GetGraphics()->GetLowLevel()->SetMatrix(eMatrix_ModelView, pCamera->GetViewMatrix());

	// mpBody->RenderDebugGeometry(mpInit->mpGame->GetGraphics()->GetLowLevel(),cColor(0,1,0,1));
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMeleeHaptX::OnJump() {
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnStartInteractMode() {
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnStartInteract() {
	mbInteractDown = true;
	mpPlayer->GetHapticCamera()->SetUseFrame(false);
	mpImpulseForce->SetActive(true);
	mpImpulseForce->SetForce(0);
	mpSlowdownForce->SetActive(true);
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnStopInteract() {
	mbInteractDown = false;
	mvGoalRotation = 0;
	mpPlayer->GetHapticCamera()->SetUseFrame(true);
	mpImpulseForce->SetActive(false);
	mpSlowdownForce->SetActive(false);
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnStartExamine() {
	// mpPlayer->ChangeState(ePlayerState_Normal);
	if (mpPlayer->GetPickedBody() && mpHudWeapon->IsAttacking() == false) {
		iGameEntity *pEntity = (iGameEntity *)mpPlayer->GetPickedBody()->GetUserData();

		pEntity->PlayerExamine();
	}
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnStartHolster() {
	mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
	// mpHudWeapon = NULL;
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMeleeHaptX::OnAddYaw(float afVal) {
	return true;
}

bool cPlayerState_WeaponMeleeHaptX::OnAddPitch(float afVal) {
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMeleeHaptX::OnMoveForwards(float afMul, float afTimeStep) {
	return true;
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMeleeHaptX::OnMoveSideways(float afMul, float afTimeStep) {
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::EnterState(iPlayerState *apPrevState) {
	gbFirstTime = false;

	// Show the darn weapon
	mpHudWeapon->LoadEntities();
	mpHudWeapon->GetEntity()->SetVisible(true);
	mpHudWeapon->GetEntity()->SetCastsShadows(true);

	// Create collider
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	cVector3f vSize = mpHudWeapon->GetHapticSize();
	cMatrixf mtxOffset = cMath::MatrixTranslate(cVector3f(0, vSize.y * 0.5f, 0));
	mpCollider = pPhysicsWorld->CreateBoxShape(vSize, &mtxOffset);

	// mpBody = pPhysicsWorld->CreateBody("",mpCollider);
	// mpBody->SetActive(false);

	mvRotation = 0;
	mfAttackCount = 0;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::LeaveState(iPlayerState *apNextState) {
	mpSlowdownForce->SetActive(false);
	mpImpulseForce->SetActive(false);

	mpHudWeapon->GetEntity()->SetVisible(false);
	mpHudWeapon->DestroyEntities();

	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	pPhysicsWorld->DestroyShape(mpCollider);
	// pPhysicsWorld->DestroyBody(mpBody);

	mpPlayer->GetHapticCamera()->SetUseFrame(true);

	// mpHudWeapon = NULL;
}

//-----------------------------------------------------------------------

// No running when you have a weapon:
void cPlayerState_WeaponMeleeHaptX::OnStartRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Walk)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Run);
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Run);
	}
}
void cPlayerState_WeaponMeleeHaptX::OnStopRun() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Run)
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	else if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump)
		mpPlayer->SetPrevMoveState(ePlayerMoveState_Walk);
}
//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::OnStartCrouch() {
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

void cPlayerState_WeaponMeleeHaptX::OnStopCrouch() {
	if (mpPlayer->GetMoveState() == ePlayerMoveState_Crouch &&
		mpInit->mpButtonHandler->GetToggleCrouch() == false) {
		mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
	}
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMeleeHaptX::OnStartInventory() {
	// mpPlayer->ChangeState(ePlayerState_Normal);
	return true;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::PlaySound(const tString &asSound) {
	cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

	pSoundHandler->PlayGui(asSound, false, 1.0f);
}

//-----------------------------------------------------------------------

bool cPlayerState_WeaponMeleeHaptX::CheckAttack(const cMatrixf &a_mtxTransform, float afSpeed) {
	mpInit->mbWeaponAttacking = true;
	// Log("----------------- BEGIN ATTACK WITH WEAPON ------------ \n");

	cMeleeWeaponAttack *pAttack = mpHudWeapon->GetAttack(0);

	float fMaxImpulse = pAttack->mfMaxImpulse * afSpeed;
	float fMinImpulse = pAttack->mfMinImpulse * afSpeed;

	float fMinMass = pAttack->mfMinMass * afSpeed;
	float fMaxMass = pAttack->mfMaxMass * afSpeed;

	float fMinDamage = pAttack->mfMinDamage * afSpeed;
	float fMaxDamage = pAttack->mfMaxDamage * afSpeed;

	int lStrength = pAttack->mlAttackStrength;

	cCamera3D *pCamera = mpInit->mpPlayer->GetCamera();

	cBoundingVolume tempBV = mpCollider->GetBoundingVolume();
	tempBV.SetTransform(cMath::MatrixMul(a_mtxTransform, tempBV.GetTransform()));

	cCollideData collideData;
	collideData.SetMaxSize(1);

	bool bHit = false;

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();

	tVector3fList lstPostions;

	////////////////////////////////
	// Iterate Enemies
	tGameEnemyIterator enemyIt = mpInit->mpMapHandler->GetGameEnemyIterator();
	while (enemyIt.HasNext()) {
		iGameEnemy *pEnemy = enemyIt.Next();
		iPhysicsBody *pBody = pEnemy->GetMover()->GetCharBody()->GetBody();
		float fMass = pBody->GetMass();

		if (pEnemy->GetMover()->GetCharBody()->IsActive() == false)
			continue;

		if (cMath::CheckCollisionBV(tempBV, *pBody->GetBV())) {
			if (pEnemy->GetMeshEntity()->CheckColliderShapeCollision(pPhysicsWorld,
																	 mpCollider, a_mtxTransform, &lstPostions, NULL) == false) {
				continue;
			}

			// Calculate force
			float fForceSize = 0;
			if (fMass > fMaxMass)
				fForceSize = 0;
			else if (fMass <= fMinMass)
				fForceSize = fMaxImpulse;
			else {
				float fT = (fMass - fMinMass) / (fMaxMass - fMinMass);
				fForceSize = fMinImpulse * fT + fMaxImpulse * (1 - fT);
			}

			cVector3f vForceDir = pCamera->GetForward();
			vForceDir.Normalise();

			// Add force to bodies
			for (int i = 0; i < pEnemy->GetBodyNum(); ++i) {
				iPhysicsBody *pBody2 = pEnemy->GetBody(i);

				pBody2->AddImpulse(vForceDir * fForceSize * 0.5f);

				cVector3f vTorque = cVector3f(0, 1, 0) * fMass * fForceSize * 0.5f;
				pBody2->AddTorque(vTorque);
			}

			// Calculate damage
			float fDamage = cMath::RandRectf(fMinDamage, fMaxDamage);

			pEnemy->Damage(fDamage, lStrength);

			// Get closest position
			float fClosestDist = 9999.0f;
			cVector3f vClosestPostion = a_mtxTransform.GetTranslation();
			for (tVector3fListIt it = lstPostions.begin(); it != lstPostions.end(); ++it) {
				cVector3f &vPos = *it;

				float fDist = cMath::Vector3DistSqr(pCamera->GetPosition(), vPos);
				if (fDist < fClosestDist) {
					fClosestDist = fDist;
					vClosestPostion = vPos;
				}
			}

			// Particle system
			if (pEnemy->GetHitPS() != "") {
				pWorld->CreateParticleSystem("Hit", pEnemy->GetHitPS(), 1, cMath::MatrixTranslate(vClosestPostion));
			}

			lstPostions.clear();

			bHit = true;
		}
	}

	std::set<iPhysicsBody *> m_setHitBodies;

	////////////////////////////////
	// Iterate bodies
	float fClosestHitDist = 9999.0f;
	cVector3f vClosestHitPos;
	iPhysicsMaterial *pClosestHitMat = NULL;
	cPhysicsBodyIterator it = pPhysicsWorld->GetBodyIterator();
	while (it.HasNext()) {
		iPhysicsBody *pBody = it.Next();

		/*float fMass = */pBody->GetMass();

		if (pBody->IsActive() == false)
			continue;
		if (pBody->GetCollide() == false)
			continue;
		if (pBody->IsCharacter())
			continue;

		if (cMath::CheckCollisionBV(tempBV, *pBody->GetBV())) {
			if (pPhysicsWorld->CheckShapeCollision(pBody->GetShape(), pBody->GetLocalMatrix(),
												   mpCollider, a_mtxTransform, collideData, 1) == false) {
				continue;
			}

			cVector3f vHitPos = collideData.mvContactPoints[0].mvPoint;

			m_setHitBodies.insert(pBody);

			// Deal damage and force
			HitBody(pBody, fMinImpulse, fMaxImpulse, fMinMass, fMaxMass, fMinDamage, fMaxDamage, lStrength);

			// Check if this is the closest hit body
			float fDist = cMath::Vector3DistSqr(vHitPos, pCamera->GetPosition());
			if (fDist < fClosestHitDist) {
				fClosestHitDist = fDist;
				vClosestHitPos = collideData.mvContactPoints[0].mvPoint;

				pClosestHitMat = pBody->GetMaterial();
			}

			bHit = true;
		}
	}

	////////////////////////////////////////////
	// Check the closest material and play sounds and effects depending on it.
	if (pClosestHitMat) {
		bHit = true;

		cMatrixf mtxPosition = cMath::MatrixTranslate(vClosestHitPos);

		cSurfaceData *pData = pClosestHitMat->GetSurfaceData();
		cSurfaceImpactData *pImpact = pData->GetHitDataFromSpeed(pAttack->mfAttackSpeed);
		if (pImpact) {
			cSoundEntity *pSound = pWorld->CreateSoundEntity("Hit", pImpact->GetSoundName(), true);
			if (pSound)
				pSound->SetPosition(vClosestHitPos);

			if (pAttack->mlHitPSPrio <= pImpact->GetPSPrio()) {
				if (pImpact->GetPSName() != "")
					pWorld->CreateParticleSystem("Hit", pImpact->GetPSName(), 1, mtxPosition);
			} else {
				if (pAttack->msHitPS != "")
					pWorld->CreateParticleSystem("Hit", pAttack->msHitPS, 1, mtxPosition);
			}
		}
	}
	// Log("----------------- END ATTACK WITH WEAPON ------------ \n");

	/////////////////////////
	// Play hit sound
	if (bHit) {
		PlaySound(pAttack->msHitSound);
	}

	mpInit->mbWeaponAttacking = false;

	return bHit;
}

//-----------------------------------------------------------------------

void cPlayerState_WeaponMeleeHaptX::HitBody(iPhysicsBody *apBody, float afMinImpulse, float afMaxImpulse,
											float afMinMass, float afMaxMass,
											float afMinDamage, float afMaxDamage,
											int alStrength) {
	iGameEntity *pEntity = (iGameEntity *)apBody->GetUserData();

	if (pEntity && pEntity->GetType() == eGameEntityType_Enemy)
		return;

	cCamera3D *pCamera = mpInit->mpPlayer->GetCamera();

	cVector3f vSpinMul = cVector3f(0, 1, 0);
	vSpinMul = pCamera->GetRight() * vSpinMul.x +
			   pCamera->GetUp() * vSpinMul.y +
			   pCamera->GetForward() * vSpinMul.z;

	float fMass = apBody->GetMass();

	// Calculate force
	float fForceSize = 0;
	if (fMass > afMaxMass)
		fForceSize = 0;
	else if (fMass <= afMinMass)
		fForceSize = afMaxImpulse;
	else {
		float fT = (fMass - afMinMass) / (afMaxMass - afMinMass);
		fForceSize = afMinImpulse * fT + afMaxImpulse * (1 - fT);
	}

	// Calculate damage
	float fDamage = cMath::RandRectf(afMinDamage, afMaxDamage);

	cVector3f vForceDir = pCamera->GetForward();

	if (fMass > 0 && fForceSize > 0) {
		vForceDir.Normalise();

		// pBody->AddForce(vForceDir * fForceSize);
		apBody->AddImpulse(vForceDir * fForceSize);

		cVector3f vTorque = vSpinMul * fMass * fForceSize;

		// vTorque = cMath::MatrixMul(pBody->GetInertiaMatrix(),vTorque);

		apBody->AddTorque(vTorque);
	}

	if (pEntity) {
		pEntity->SetLastImpulse(vForceDir * fForceSize);
		pEntity->Damage(fDamage, alStrength);
	}
}

//-----------------------------------------------------------------------
