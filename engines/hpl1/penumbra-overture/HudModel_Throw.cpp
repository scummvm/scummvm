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

#include "hpl1/penumbra-overture/HudModel_Throw.h"

#include "hpl1/penumbra-overture/AttackHandler.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"

/////////////////////////////////////////////////////////////////////////
// HUD MODEL THROW
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cHudModel_Throw::cHudModel_Throw() : iHudModel(ePlayerHandType_Throw) {
	mbButtonDown = false;

	mfChargeCount = 0;
}

//-----------------------------------------------------------------------

void cHudModel_Throw::LoadData(TiXmlElement *apRootElem) {
	////////////////////////////////////////////////
	// Load the MAIN element.
	TiXmlElement *pMeleeElem = apRootElem->FirstChildElement("THROW");
	if (pMeleeElem == NULL) {
		Error("Couldn't load THROW element from XML document\n");
		return;
	}

	// mbDrawDebug = cString::ToBool(pMeleeElem->Attribute("DrawDebug"),false);
	mChargePose = GetPoseFromElem("ChargePose", pMeleeElem);

	mfChargeTime = cString::ToFloat(pMeleeElem->Attribute("ChargeTime"), 0);
	mfMinImpulse = cString::ToFloat(pMeleeElem->Attribute("MinImpulse"), 0);
	mfMaxImpulse = cString::ToFloat(pMeleeElem->Attribute("MaxImpulse"), 0);

	mfReloadTime = cString::ToFloat(pMeleeElem->Attribute("ReloadTime"), 0);

	mvTorque = cString::ToVector3f(pMeleeElem->Attribute("Torque"), 0);

	msThrowEntity = cString::ToString(pMeleeElem->Attribute("ThrowEntity"), "");

	msChargeSound = cString::ToString(pMeleeElem->Attribute("ChargeSound"), "");
	msThrowSound = cString::ToString(pMeleeElem->Attribute("ThrowSound"), "");
}

//-----------------------------------------------------------------------

void cHudModel_Throw::OnStart() {
	mbButtonDown = false;
	mfChargeCount = 0;
}

//-----------------------------------------------------------------------

void cHudModel_Throw::ResetExtraData() {
	mbButtonDown = false;
	mfChargeCount = 0;
}

//-----------------------------------------------------------------------

bool cHudModel_Throw::UpdatePoseMatrix(cMatrixf &aPoseMtx, float afTimeStep) {
	if (mbButtonDown) {
		mfChargeCount += afTimeStep / mfChargeTime;
		if (mfChargeCount > 1)
			mfChargeCount = 1;
	} else {
		mfChargeCount -= afTimeStep * 4;
		if (mfChargeCount < 0)
			mfChargeCount = 0;
	}

	cMatrixf mtxA = mEquipPose.ToMatrix();
	cMatrixf mtxB = mChargePose.ToMatrix();
	aPoseMtx = cMath::MatrixSlerp(mfChargeCount, mtxA, mtxB, true);

	return true;
}

//-----------------------------------------------------------------------

void cHudModel_Throw::OnAttackDown() {
	if (mState == eHudModelState_Idle) {
		mbButtonDown = true;

		if (msChargeSound != "") {
			cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();
			pSoundHandler->PlayGui(msChargeSound, false, 1.0f);
		}
	}
}

//-----------------------------------------------------------------------

void cHudModel_Throw::OnAttackUp() {
	if (mbButtonDown == false)
		return;

	mbButtonDown = false;

	// Play sound
	if (msThrowSound != "") {
		cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();
		pSoundHandler->PlayGui(msThrowSound, false, 1.0f);
	}

	///////////////////////////////
	// Create entity
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();

	cVector3f vRot = cVector3f(pCam->GetPitch(), pCam->GetYaw(), pCam->GetRoll());
	cMatrixf mtxStart = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
	mtxStart.SetTranslation(pCam->GetPosition());

	iEntity3D *pEntity = mpInit->mpGame->GetScene()->GetWorld3D()->CreateEntity("Throw", mtxStart,
																				msThrowEntity, true);
	if (pEntity) {
		iGameEntity *pEntity2 = mpInit->mpMapHandler->GetLatestEntity();

		float fImpulse = mfMinImpulse * (1 - mfChargeCount) + mfMaxImpulse * mfChargeCount;

		cVector3f vRot2 = cMath::MatrixMul(mtxStart.GetRotation(), mvTorque);

		for (int i = 0; i < pEntity2->GetBodyNum(); ++i) {
			iPhysicsBody *pBody = pEntity2->GetBody(i);
			pBody->AddImpulse(pCam->GetForward() * fImpulse);
			pBody->AddTorque(vRot2);
		}
	}

	mpInit->mpPlayer->GetHidden()->UnHide();

	//////////////////////
	// Reset animations

	mfChargeCount = 0;

	mfTime = -mfReloadTime;
	mState = eHudModelState_Equip;

	//////////////////////
	// Decrease item amount
	mpItem->AddCount(-1);
	if (mpItem->GetCount() <= 0) {
		mfTime = 0.0f;
		mpInit->mpInventory->RemoveItem(mpItem);
		mpInit->mpPlayerHands->SetCurrentModel(1, "");
		mpInit->mpPlayer->ChangeState(ePlayerState_Normal);
	}
}

//-----------------------------------------------------------------------

bool cHudModel_Throw::OnMouseMove(const cVector2f &avMovement) {
	return false;
}

//-----------------------------------------------------------------------
