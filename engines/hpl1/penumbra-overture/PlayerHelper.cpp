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

#include "hpl1/penumbra-overture/PlayerHelper.h"

#include "hpl1/penumbra-overture/DeathMenu.h"
#include "hpl1/penumbra-overture/GameArea.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GameObject.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/NumericalPanel.h"
#include "hpl1/penumbra-overture/Player.h"
#include "hpl1/penumbra-overture/PlayerHands.h"

//////////////////////////////////////////////////////////////////////////
// HIT GROUND CALLBACK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerBodyCallback::cPlayerBodyCallback(cPlayer *apPlayer) {
	mpPlayer = apPlayer;
	cInit *pInit = apPlayer->GetInit();

	mfFallSpeed_Min = pInit->mpGameConfig->GetFloat("Player", "FallSpeed_Min", 0);
	mfFallDamage_Min = pInit->mpGameConfig->GetFloat("Player", "FallDamage_Min", 0);
	msFallSound_Min = pInit->mpGameConfig->GetString("Player", "FallSound_Min", "");

	mfFallSpeed_Med = pInit->mpGameConfig->GetFloat("Player", "FallSpeed_Med", 0);
	mfFallDamage_Med = pInit->mpGameConfig->GetFloat("Player", "FallDamage_Med", 0);
	msFallSound_Med = pInit->mpGameConfig->GetString("Player", "FallSound_Med", "");

	mfFallSpeed_Max = pInit->mpGameConfig->GetFloat("Player", "FallSpeed_Max", 0);
	mfFallDamage_Max = pInit->mpGameConfig->GetFloat("Player", "FallDamage_Max", 0);
	msFallSound_Max = pInit->mpGameConfig->GetString("Player", "FallSound_Max", "");
}

//-----------------------------------------------------------------------

void cPlayerBodyCallback::OnHitGround(iCharacterBody *apCharBody, const cVector3f &avVel) {
	// if(avVel.y < -5.5f)
	//	mpPlayer->FootStep(1.0f,"run",true);

	if (mpPlayer->GetMoveState() == ePlayerMoveState_Jump) {
		if (avVel.y < 0) {
			if (avVel.y < -3.5f) {
				mpPlayer->FootStep(1.0f, "run", true);
			}
			mpPlayer->SetLandedFromJump(true);
		}
	} else if (avVel.y < -8.5f) {
		mpPlayer->FootStep(1.0f, "run", true);
	}

	tString sSound = "";
	float fVol = 1.0f;
	cSoundHandler *pSoundHandler = mpPlayer->GetInit()->mpGame->GetSound()->GetSoundHandler();
	// Log("Speed: %f\n",avVel.y);
	/////////////////////
	// Fall damage
	// Hard
	if (avVel.y < mfFallSpeed_Max) {
		sSound = msFallSound_Max;
		fVol = 1.0f;
		mpPlayer->Damage(mfFallDamage_Max, ePlayerDamageType_BloodSplash);
		pSoundHandler->PlayGui("player_fall_damage", false, 1);
		// Log("Hard\n");
	}
	// Medium
	else if (avVel.y < mfFallSpeed_Med) {
		sSound = msFallSound_Med;
		fVol = 1.0f;
		mpPlayer->Damage(mfFallDamage_Med, ePlayerDamageType_BloodSplash);
		pSoundHandler->PlayGui("player_fall_damage", false, 0.75f);
		// Log("Medium\n");
	}
	// Light
	else if (avVel.y < mfFallSpeed_Min) {
		sSound = msFallSound_Min;
		fVol = 1.0f;
		mpPlayer->Damage(mfFallDamage_Min, ePlayerDamageType_BloodSplash);
		pSoundHandler->PlayGui("player_fall_damage", false, 0.5f);
		// Log("Light\n");
	}

	if (sSound != "") {
		cWorld3D *pWorld = mpPlayer->GetInit()->mpGame->GetScene()->GetWorld3D();
		cSoundEntity *pSound = pWorld->CreateSoundEntity("HitGround", sSound, true);
		if (pSound) {
			iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
			pSound->SetVolume(fVol * pSound->GetVolume());
			pSound->SetPosition(pCharBody->GetPosition() - cVector3f(0, pCharBody->GetSize().y / 2 - 0.2f, 0));
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GROUND RAY CALLBACK
//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------

cPlayerGroundRayCallback::cPlayerGroundRayCallback() {
	mpMaterial = NULL;
	mbOnGround = false;
}

//-----------------------------------------------------------------------

void cPlayerGroundRayCallback::Clear() {
	mfMinDist = 1000;
	mpMaterial = NULL;
}

//-----------------------------------------------------------------------

void cPlayerGroundRayCallback::OnWorldExit() {
	mpMaterial = NULL;
}

void cPlayerGroundRayCallback::Reset() {
	mpMaterial = NULL;
}

//-----------------------------------------------------------------------

bool cPlayerGroundRayCallback::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	mbOnGround = true;

	if (apParams->mfDist < mfMinDist && pBody->GetMaterial()->GetSurfaceData() &&
		pBody->GetMaterial()->GetSurfaceData()->GetStepType() != "") {
		mpMaterial = pBody->GetMaterial();
		mfMinDist = apParams->mfDist;
	}

	return true;
}

//-----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
// PICK RAY CALLBACK
//////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------

cPlayerPickRayCallback::cPlayerPickRayCallback() {
	mfMaxDistance = 10.0f;
	mpPickedBody = NULL;

	mLastEntityType = eGameEntityType_LastEnum;
}

//-----------------------------------------------------------------------

void cPlayerPickRayCallback::Clear() {
	mfPickedDist = mfMaxDistance + 10.0f;
	mfPickedAreaDist = mfMaxDistance + 10.0f;

	mpPickedBody = NULL;
	mpPickedAreaBody = NULL;

	mfMinStaticDist = mfMaxDistance + 10.0f;
}

//-----------------------------------------------------------------------

void cPlayerPickRayCallback::OnWorldExit() {
	mpPickedBody = NULL;
}

//-----------------------------------------------------------------------

bool cPlayerPickRayCallback::OnIntersect(iPhysicsBody *apBody, cPhysicsRayParams *apParams) {
	float &fDist = apParams->mfDist;

	// Must be positive
	if (fDist < 0)
		return true;

	// Must be below max distance
	if (fDist > mfMaxDistance)
		return true;

	// No characters allowed
	if (apBody->IsCharacter())
		return true;

	// Check if is below current picked dist
	iGameEntity *pEntity = NULL;
	if (apBody->GetUserData())
		pEntity = (iGameEntity *)apBody->GetUserData();

	//////////////////////////////
	// Entity
	if (pEntity) {
		// Skip stick area
		if (pEntity->GetType() == eGameEntityType_StickArea)
			return true;

		// Area
		if (pEntity->GetType() == eGameEntityType_Area ||
			pEntity->GetType() == eGameEntityType_StickArea ||
			pEntity->GetType() == eGameEntityType_DamageArea ||
			pEntity->GetType() == eGameEntityType_ForceArea) {
			if (mfPickedAreaDist > fDist &&
				(pEntity->GetDescription() != _W("") ||
				 pEntity->GetCallbackScript(eGameEntityScriptType_PlayerInteract) != NULL ||
				 pEntity->GetCallbackScript(eGameEntityScriptType_PlayerExamine) != NULL ||
				 pEntity->GetHasInteraction())) {
				mfPickedAreaDist = fDist;
				mvPickedAreaPos = apParams->mvPoint;
				mpPickedAreaBody = apBody;
			}
		}
		// Other entity
		else {
			if (mfPickedDist > fDist) {
				mfPickedDist = fDist;
				mvPickedPos = apParams->mvPoint;
				mpPickedBody = apBody;
			}
		}
	}
	///////////////////////////////
	// Non Entity
	else {
		if (mfPickedDist > fDist) {
			mfPickedDist = fDist;
			mpPickedBody = NULL;
		}
	}

	return true;

	/*float &fDist = apParams->mfDist;

	iGameEntity *pEntity = NULL;
	if(apBody->GetUserData()) pEntity = (iGameEntity*)apBody->GetUserData();

	//Don't wanna pick characters
	if(apBody->IsCharacter() && apBody->GetUserData()==NULL) return true;

	if(pEntity && pEntity->IsActive()==false) return true;

	//If it is outside of the examine distance skip
	if(pEntity && pEntity->GetMaxExamineDist() < fDist) return true;

	////////////////////////////
	//Not picked
	bool bStatic = false;
	if(pEntity && pEntity->GetType()== eGameEntityType_Object)
	{
		cGameObject* pObject = static_cast<cGameObject*>(pEntity);
		if(pObject->GetInteractMode() == eObjectInteractMode_Static)
		{
			bStatic = true;
		}
	}

	if(	 pEntity==NULL || bStatic)
	{
		if(fDist < mfPickedDist)
		{
			mfMinStaticDist = fDist;
			mfPickedDist = fDist;
			mpPickedBody = NULL;
			mvPickedPos = apParams->mvPoint;
			mLastEntityType = eGameEntityType_LastEnum;
		}
	}
	////////////////////////////
	//Picked
	else
	{
		if(pEntity->GetType() == eGameEntityType_Area)
		{
			if(fDist < mfPickedDist)
			{
				mpPickedBody = apBody;
				mLastEntityType = pEntity->GetType();
				mvPickedPos = apParams->mvPoint;
				mfPickedDist = fDist;
			}
		}
		else
		{
			if	(fDist < mfPickedDist ||
				(mLastEntityType == eGameEntityType_Area))
			{
				mpPickedBody = apBody;
				mLastEntityType = pEntity->GetType();
				mvPickedPos = apParams->mvPoint;
				mfPickedDist = fDist;
			}
		}

	}

	return true;*/
}

void cPlayerPickRayCallback::CalculateResults() {
	// Check if an area is closer than the closest normal body
	if (mpPickedAreaBody && mfPickedAreaDist < mfPickedDist) {
		bool bUseArea = false;

		if (mpPickedBody) {
			iGameEntity *pEntity = (iGameEntity *)mpPickedBody->GetUserData();
			cGameArea *pArea = (cGameArea *)mpPickedAreaBody->GetUserData();

			if (pEntity) {
				// Too for from object
				if (pEntity->GetMaxExamineDist() < mfPickedDist)
					bUseArea = true;

				// No description for object and it is not possible to interact with it
				else if (pEntity->GetDescription() == _W("")) {
					if (mpPickedBody->GetMass() == 0)
						bUseArea = true;

					else if (pEntity->GetType() == eGameEntityType_Object) {
						cGameObject *pObject = static_cast<cGameObject *>(pEntity);

						if (pObject->GetInteractMode() == eObjectInteractMode_Static)
							bUseArea = true;
					}
				}
			} else {
				bUseArea = true;
			}
		} else {
			bUseArea = true;
		}

		if (bUseArea) {
			mpPickedBody = mpPickedAreaBody;
			mfPickedDist = mfPickedAreaDist;
			mvPickedPos = mvPickedAreaPos;
		}
	}
}

//-----------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////
// HEAD MOVE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerHeadMove::cPlayerHeadMove(cPlayer *apPlayer) {
	mfAdd = 0;
	mfPos = 0;
	mbActive = false;
	mpPlayer = apPlayer;

	mbFootStepped = false;
	mbWasActive = false;

	mfClimbCount = 0;
	mfClimbPos = 0;
}

//-----------------------------------------------------------------------

void cPlayerHeadMove::Update(float afTimeStep) {
	///////////////////////////////
	// Climb count
	iCharacterBody *pBody = mpPlayer->GetCharacterBody();
	/*float fUpVel = (pBody->GetPosition().y - pBody->GetLastPosition().y) / afTimeStep;*/

	if (pBody->IsClimbing() && mfClimbCount == 0) {
		mfClimbCount = 0.5f;
		mfClimbPos = mfPos;
	} else if (mfClimbCount > 0) {
		mfClimbCount -= afTimeStep;
		if (mfClimbCount < 0) {
			mfClimbCount = 0;
			mfPos = mfClimbPos;
			mfAdd = mfHeadMoveSpeed;
		}
	}

	///////////////////////////////
	// Head movement
	if (mbActive) {
		mbWasActive = true;

		mfPos += mfAdd * afTimeStep * mpPlayer->GetHeadMoveSpeedMul();
		if (mfAdd < 0) {
			if (mfPos <= mfMinHeadMove * mpPlayer->GetHeadMoveSpeedMul()) {
				mfPos = mfMinHeadMove * mpPlayer->GetHeadMoveSpeedMul();
				mfAdd = mfHeadMoveSpeed;

				// Create a footstep.
				if (mfFootStepMul > 0)
					mpPlayer->FootStep(mfFootStepMul);
				mbFootStepped = true;
			}
		} else {
			if (mfPos >= mfMaxHeadMove * mpPlayer->GetHeadMoveSpeedMul()) {
				mfPos = mfMaxHeadMove * mpPlayer->GetHeadMoveSpeedMul();
				mfAdd = -mfHeadMoveSpeed;
			}
		}
	} else {
		if (mbWasActive == true && (mbFootStepped == false || (mfPos < 0 && mfAdd < 0))) {
			if (mfFootStepMul > 0)
				mpPlayer->FootStep(mfFootStepMul * 0.6f);
		}
		mbFootStepped = false;
		mbWasActive = false;

		if (mfPos == 0)
			return;

		if (mfPos < 0) {
			mfPos += mfHeadMoveBackSpeed * afTimeStep;
			if (mfPos > 0) {
				mfPos = 0;
			}
		} else {
			mfPos -= mfHeadMoveBackSpeed * afTimeStep;
			if (mfPos < 0) {
				mfPos = 0;
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerHeadMove::Start() {
	if (mbActive == false) {
		mbActive = true;
	}

	if (fabs(fabs(mfAdd) - mfHeadMoveSpeed) > 0.001f) {
		mfAdd = mfHeadMoveSpeed;
	}
}

void cPlayerHeadMove::Stop() {
	mbActive = false;
}
//-----------------------------------------------------------------------

float cPlayerHeadMove::GetPos() {
	if (mfClimbCount > 0)
		return mfClimbPos;

	return mfPos;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// HEALTH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerHealth::cPlayerHealth(cInit *apInit) {
	mpInit = apInit;
	mfTimeCount = 0;

	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mpDamageGfx = mpDrawer->CreateGfxObject("player_hurt.bmp", "diffalpha2d");

	mfGfxAlpha = 0;
	mfGfxAlphaAdd = 0;

	mfGfxGlobalAlpha = 0;

	mfTerrorCheckCount = 0;

	mpSoundEntry = NULL;
}

//-----------------------------------------------------------------------

void cPlayerHealth::Reset() {
	mfTerrorCheckCount = 0;
	if (mpSoundEntry)
		mpSoundEntry->mpSound->Stop();
	mpSoundEntry = NULL;

	mlTerrorLevel = 0;
}

//-----------------------------------------------------------------------

void cPlayerHealth::Update(float afTimeStep) {
	cPlayer *pPlayer = mpInit->mpPlayer;

	if (pPlayer->GetHealth() <= 0) {
		if (mpSoundEntry)
			mpSoundEntry->mpSound->Stop();
		mpSoundEntry = NULL;

		mfGfxAlpha -= afTimeStep * 2.0f;
		return;
	}

	////////////////////////////
	// ALpha
	mfGfxAlpha += mfGfxAlphaAdd * afTimeStep;
	if (mfGfxAlphaAdd < 0 && mfGfxAlpha < 0)
		mfGfxAlpha = 0;
	if (mfGfxAlphaAdd > 0 && mfGfxAlpha >= 1.0f) {
		mfGfxAlpha = 1.0f;
		mfGfxAlphaAdd = -1.5;
	}

	////////////////////////////
	// Heart timer
	if (mfTimeCount <= 0) {
		if (pPlayer->GetHealth() > 50) {
			pPlayer->SetHealthSpeedMul(1.0f);
			mfGfxGlobalAlpha = 0;
		} else {
			float fVolume = 0.5;
			if (pPlayer->GetHealth() > 25) {
				mfTimeCount = 1.7f;
				pPlayer->SetHealthSpeedMul(0.8f);
				mfGfxGlobalAlpha = 0.5f;
			} else if (pPlayer->GetHealth() > 10) {
				mfTimeCount = 1.0;
				fVolume = 0.75f;
				pPlayer->SetHealthSpeedMul(0.6f);
				mfGfxGlobalAlpha = 0.75f;
			} else {
				mfTimeCount = 0.7f;
				fVolume = 1.0f;
				pPlayer->SetHealthSpeedMul(0.4f);
				mfGfxGlobalAlpha = 1.0f;
			}

			mpInit->mpGame->GetSound()->GetSoundHandler()->PlayGui("player_heartbeat", false, fVolume);
			mfGfxAlphaAdd = 3.5f;
		}
	} else {
		mfTimeCount -= afTimeStep;
	}

	////////////////////////////
	// Increase health over time:
	switch (mpInit->mDifficulty) {
	case eGameDifficulty_Easy:
		pPlayer->AddHealth(afTimeStep * 1.4f);
		break;
	case eGameDifficulty_Normal:
		pPlayer->AddHealth(afTimeStep * 0.4f);
		break;
	case eGameDifficulty_Hard:
		pPlayer->AddHealth(afTimeStep * 0.14f);
		break;
	}

	////////////////////////////
	// Check for enemies.
	/*if(mfTerrorCheckCount <=0)
	{
		//Check the current terror level.
		int lTerrorLevel =0;
		tGameEnemyIterator it = mpInit->mpMapHandler->GetGameEnemyIterator();
		while(it.HasNext())
		{
			iGameEnemy *pEnemy = it.Next();

			if(pEnemy->IsActive()==false || pEnemy->GetHealth() <=0) continue;

			int lState = pEnemy->GetStateMachine()->CurrentState()->GetId();

			if(lState == eGameEnemyState_Hunt || lState == eGameEnemyState_Attack)
			{
				lTerrorLevel = 3;
			}
			else if(lTerrorLevel < 2 && lState == eGameEnemyState_Investigate)
			{
				lTerrorLevel = 2;
			}
			else if(lTerrorLevel < 1)
			{
				lTerrorLevel = 1;
			}
		}

		if(mlTerrorLevel != lTerrorLevel)
		{
			mlTerrorLevel = lTerrorLevel;

			cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

			//Fade out the current sound if there is any.
			if(mpSoundEntry)
			{
				mpSoundEntry->mfNormalVolumeFadeDest = 0;
				mpSoundEntry->mfNormalVolumeFadeSpeed = -0.25f;
				mpSoundEntry = NULL;
			}
			mpSoundEntry = NULL;

			tString sSound="";
			float fVolume;

			if(mlTerrorLevel==0)
			{
			}
			else if(mlTerrorLevel==1)
			{
				sSound = "horror_roach_idle";
				fVolume = 0.3f;
			}
			else if(mlTerrorLevel==2)
			{
				sSound = "horror_roach_notice";
				fVolume = 0.45f;
			}
			else if(mlTerrorLevel==3)
			{
				sSound = "horror_roach_attack";
				fVolume = 0.6f;
			}

			if(sSound != "")
			{
				iSoundChannel *pChannel = pSoundHandler->PlayGui(sSound, true, fVolume);
				pChannel->SetPriority(200);
				mpSoundEntry = pSoundHandler->GetEntryFromSound(pChannel);

				mpSoundEntry->mfNormalVolumeMul =0;
				mpSoundEntry->mfNormalVolumeFadeDest =1;
				mpSoundEntry->mfNormalVolumeFadeSpeed = 0.55f;
			}
		}

		mfTerrorCheckCount = 0.2f;
	}
	else
	{
		mfTerrorCheckCount -= afTimeStep;
	}*/
}

void cPlayerHealth::Draw() {
	if (mfGfxAlpha <= 0)
		return;

	mpDrawer->DrawGfxObject(mpDamageGfx, cVector3f(0, 0, 50), cVector2f(800, 600),
							cColor(1, mfGfxGlobalAlpha * mfGfxAlpha));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// EAR RING
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerEarRing::cPlayerEarRing(cInit *apInit, cPlayer *apPlayer) {
	mpInit = apInit;
	mpPlayer = apPlayer;

	mpSoundEntry = NULL;
	mfTimeCount = 0;

	mbDeaf = false;
}

//-----------------------------------------------------------------------

void cPlayerEarRing::Reset() {
	cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

	if (mpSoundEntry)
		mpSoundEntry->mpSound->Stop();
	pSoundHandler->SetVolume(1.0f, 0.1f, eSoundDest_World);

	mpSoundEntry = NULL;
	mfTimeCount = 0;

	mbDeaf = false;
}

//-----------------------------------------------------------------------

void cPlayerEarRing::Update(float afTimeStep) {
	if (mfTimeCount > 0) {
		cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

		mfTimeCount -= afTimeStep;
		mfDeafTime -= afTimeStep;

		if (mbDeaf && mfDeafTime <= 0) {
			mbDeaf = false;
			pSoundHandler->SetVolume(0.2f, 1.5f, eSoundDest_World);
		}

		if (mfTimeCount <= 0) {
			mfTimeCount = 0;
			pSoundHandler->SetVolume(1.0f, 3.5f, eSoundDest_World);

			if (mpSoundEntry) {
				mpSoundEntry->mfNormalVolumeFadeDest = 0;
				mpSoundEntry->mfNormalVolumeFadeSpeed = -0.25f;
				mpSoundEntry = NULL;
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerEarRing::Start(float afTime) {
	if (mpInit->mpPlayer->GetHealth() <= 0)
		return;

	if (mfTimeCount > 0) {
	} else {
		cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

		iSoundChannel *pChannel = pSoundHandler->PlayGui("player_ear_ring", true, 0.5f);
		pChannel->SetPriority(200);
		mpSoundEntry = pSoundHandler->GetEntryFromSound(pChannel);

		mpSoundEntry->mfNormalVolumeMul = 0;
		mpSoundEntry->mfNormalVolumeFadeDest = 1;
		mpSoundEntry->mfNormalVolumeFadeSpeed = 0.55f;

		pSoundHandler->SetVolume(0.0f, 1.2f, eSoundDest_World);
	}

	mfTimeCount = afTime;

	mbDeaf = true;
	mfDeafTime = afTime * 0.3f;
}

//-----------------------------------------------------------------------

void cPlayerEarRing::Stop(bool abStopDirectly) {
	cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

	if (abStopDirectly) {
		if (mpSoundEntry)
			mpSoundEntry->mpSound->Stop();
		mpSoundEntry = NULL;

		pSoundHandler->SetVolume(1, 5, eSoundDest_World);
	} else {
		mfTimeCount = 0;
		pSoundHandler->SetVolume(1.0f, 3.5f, eSoundDest_World);

		if (mpSoundEntry) {
			mpSoundEntry->mfNormalVolumeFadeDest = 0;
			mpSoundEntry->mfNormalVolumeFadeSpeed = -0.25f;
			mpSoundEntry = NULL;
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LEAN
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerLean::cPlayerLean(cInit *apInit, cPlayer *apPlayer) {
	mpPlayer = apPlayer;
	mpInit = apInit;

	mfMaxMovement = 0.5f;
	mfMaxRotation = cMath::ToRad(15);

	mpHeadShape = NULL;
}

void cPlayerLean::Reset() {
	mfDir = 0;
	mfMaxTime = 0.8f;
	mfMovement = 0;
	mfRotation = 0;

	mfMoveSpeed = 0;

	mbPressed = false;
}

//-----------------------------------------------------------------------

void cPlayerLean::OnWorldLoad() {
	iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

	float fRadius = mpInit->mpPlayer->GetSize().x / 2 * 0.63f;
	float fHeight = mpInit->mpPlayer->GetCameraHeightAdd() * 2;
	if (fHeight < 0)
		fHeight = fHeight * -1;
	cMatrixf mtxOffset = cMath::MatrixRotateZ(kPi2f);
	mpHeadShape = pPhysicsWorld->CreateCylinderShape(fRadius, fHeight, &mtxOffset);
}

//-----------------------------------------------------------------------

void cPlayerLean::Update(float afTimeStep) {
	////////////////////////////////
	// If pressed move in direction
	if (mbPressed) {
		mbPressed = false;

		float fGoalPos = mfMaxMovement * mfDir;
		float fGoalRot = mfMaxRotation * -mfDir;

		//////////////
		// Position
		float fPrevMovement = mfMovement;
		float fMoveSpeed = (fGoalPos - mfMovement);
		if (fabsf(fMoveSpeed) < 0.1f)
			fMoveSpeed = 0.1f * mfDir;
		mfMovement += fMoveSpeed * afTimeStep * 3;

		if (fGoalPos < 0 && mfMovement < fGoalPos)
			mfMovement = fGoalPos;
		if (fGoalPos > 0 && mfMovement > fGoalPos)
			mfMovement = fGoalPos;

		//////////////
		// Rotation
		float fPrevRotation = mfRotation;
		float fRotSpeed = fGoalRot - mfRotation;
		if (fabsf(fRotSpeed) < 0.13f)
			fRotSpeed = 0.13f * -mfDir;

		mfRotation += fRotSpeed * afTimeStep * 2;

		if (fGoalRot < 0 && mfRotation < fGoalRot)
			mfRotation = fGoalRot;
		if (fGoalRot > 0 && mfRotation > fGoalRot)
			mfRotation = fGoalRot;

		////////////////////
		// Check collision
		cCamera3D *pCam = mpInit->mpPlayer->GetCamera();
		iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

		float fReverseMov = fPrevMovement - mfMovement;
		float fReverseRot = fPrevRotation - mfRotation;

		iCharacterBody *pCharBody = mpPlayer->GetCharacterBody();
		float fHeightAdd = mpPlayer->GetSize().y + mpPlayer->GetHeightAdd() + mpPlayer->GetCameraHeightAdd();
		cVector3f vStartPos = pCharBody->GetFeetPosition() + cVector3f(0, fHeightAdd, 0);

		cVector3f vPos = vStartPos + pCam->GetRight() * mfMovement;

		int lCount = 0;
		while (pPhysicsWorld->CheckShapeWorldCollision(NULL, mpHeadShape,
													   cMath::MatrixTranslate(vPos), NULL, false, true, NULL, false)) {
			mfMovement += fReverseMov;
			mfRotation += fReverseRot;

			if (fReverseMov < 0 && mfMovement < 0) {
				mfMovement = 0;
				mfRotation = 0;
				break;
			}
			if (fReverseMov > 0 && mfMovement > 0) {
				mfMovement = 0;
				mfRotation = 0;
				break;
			}

			vPos = vStartPos + pCam->GetRight() * mfMovement;
			lCount++;
			if (lCount > 10) {
				mfMovement = 0;
				mfRotation = 0;
				break;
			}
		}
	}
	////////////////////////////
	// Not pressed move back
	else {
		float fMoveSpeed = (0 - mfMovement);
		mfMovement += fMoveSpeed * afTimeStep * 6;

		float fRotSpeed = 0 - mfRotation;
		mfRotation += fRotSpeed * afTimeStep * 3;
	}

	mpPlayer->GetCamera()->AddRoll(mfRotation);
}

//-----------------------------------------------------------------------

bool cPlayerLean::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	if (pBody == mpPlayer->GetCharacterBody()->GetBody())
		return true;

	mbIntersect = true;
	return false;
}

//-----------------------------------------------------------------------

void cPlayerLean::Lean(float afMul, float afTimeStep) {
	mbPressed = true;
	mfDir = afMul;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// DAMAGE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerDamage::cPlayerDamage(cInit *apInit) {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpPostEffects = mpInit->mpGame->GetGraphics()->GetRendererPostEffects();

	mbActive = false;

	mType = ePlayerDamageType_BloodSplash;

	mvHitGfx[ePlayerDamageType_BloodSplash] = mpDrawer->CreateGfxObject("player_hit_blood_splash.bmp", "diffalpha2d");
	mvHitGfx[ePlayerDamageType_Ice] = mpDrawer->CreateGfxObject("player_hit_ice.bmp", "diffalpha2d");

	mvHeadSwingAcc = cVector2f(17.0f, 17.0f);
}

cPlayerDamage::~cPlayerDamage() {
	for (int i = 0; i < ePlayerDamageType_LastEnum; ++i)
		mpDrawer->DestroyGfxObject(mvHitGfx[i]);
}

//-----------------------------------------------------------------------

void cPlayerDamage::Start(float afSize, ePlayerDamageType aType) {
	if (mbActive)
		return;

	mfSize = afSize;

	mType = aType;

	mfAlpha = 0;
	mfAlphaAdd = 4.3f;

	mbActive = true;

	if (!mpInit->mpPlayer->GetFearFilter()->IsActive()) {
		mpPostEffects->SetImageTrailActive(true);
		mpPostEffects->SetImageTrailAmount(0);
	}

	mvHeadSwingSpeed = cVector2f(cMath::RandRectf(-1, 1), cMath::RandRectf(0, 0.5f));
	if (mvHeadSwingSpeed.x == 0 && mvHeadSwingSpeed.y == 0)
		mvHeadSwingSpeed.x = 1;
	else
		mvHeadSwingSpeed.Normalise();

	mvHeadSwingSpeed = mvHeadSwingSpeed * afSize * 1.5f;

	mpInit->mpPlayer->GetCharacterBody()->SetMoveSpeed(eCharDir_Forward, 0);
	mpInit->mpPlayer->GetCharacterBody()->SetMoveSpeed(eCharDir_Right, 0);
}

//-----------------------------------------------------------------------

void cPlayerDamage::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	// Alpha update
	mfAlpha += mfAlphaAdd * afTimeStep;
	if (mfAlphaAdd > 0) {
		if (mfAlpha >= 1) {
			mfAlpha = 1;
			mfAlphaAdd = -2.2f;
		}
	} else if (mfAlphaAdd < 0) {
		if (mfAlpha <= 0) {
			mfAlpha = 0;
		}
	}

	// Update swing
	mpInit->mpPlayer->GetCamera()->AddYaw(mvHeadSwingSpeed.x * afTimeStep);
	mpInit->mpPlayer->GetCamera()->AddPitch(mvHeadSwingSpeed.y * afTimeStep);

	if (mvHeadSwingSpeed.x > 0) {
		mvHeadSwingSpeed.x -= (mvHeadSwingAcc.x / mfSize) * afTimeStep * 2.2f;
		if (mvHeadSwingSpeed.x < 0)
			mvHeadSwingSpeed.x = 0;
	} else {
		mvHeadSwingSpeed.x += (mvHeadSwingAcc.x / mfSize) * afTimeStep * 2.2f;
		if (mvHeadSwingSpeed.x > 0)
			mvHeadSwingSpeed.x = 0;
	}
	if (mvHeadSwingSpeed.y > 0) {
		mvHeadSwingSpeed.y -= (mvHeadSwingAcc.x / mfSize) * afTimeStep * 2.2f;
		if (mvHeadSwingSpeed.y < 0)
			mvHeadSwingSpeed.y = 0;
	} else {
		mvHeadSwingSpeed.y += (mvHeadSwingAcc.x / mfSize) * afTimeStep * 2.2f;
		if (mvHeadSwingSpeed.y > 0)
			mvHeadSwingSpeed.y = 0;
	}

	// Blur update
	if (mpInit->mpPlayer->IsDead() == false) {
		if (!mpInit->mpPlayer->GetFearFilter()->IsActive()) {
			mpPostEffects->SetImageTrailAmount(mfAlpha * 0.92f);
		}
	}

	// Has it ended?
	if (mvHeadSwingSpeed.x == 0 && mvHeadSwingSpeed.y == 0 && mfAlpha == 0) {
		mbActive = false;
		if (mpInit->mpPlayer->IsDead() == false) {
			if (!mpInit->mpPlayer->GetFearFilter()->IsActive()) {
				mpPostEffects->SetImageTrailActive(false);
				mpPostEffects->SetImageTrailAmount(0);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerDamage::Draw() {
	if (mbActive == false)
		return;

	mpDrawer->DrawGfxObject(mvHitGfx[mType], cVector3f(0, 0, 3), cVector2f(800, 600), cColor(1, mfAlpha * 0.6f));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// DEATH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerDeath::cPlayerDeath(cInit *apInit) {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();
	mpPostEffects = mpInit->mpGame->GetGraphics()->GetRendererPostEffects();

	mpFadeGfx = mpDrawer->CreateGfxObject("player_death_fade.bmp", "smoke2d");
	mpBlackGfx = mpDrawer->CreateGfxObject("player_death_black.bmp", "smoke2d");
}

cPlayerDeath::~cPlayerDeath() {
	mpDrawer->DestroyGfxObject(mpFadeGfx);
}

//-----------------------------------------------------------------------

void cPlayerDeath::Reset() {
	mbActive = false;

	mfHeightAdd = 0;
	mfRoll = 0;

	mpPostEffects->SetImageTrailActive(false);
}

//-----------------------------------------------------------------------

void cPlayerDeath::Start() {
	if (mbActive)
		return;

	if (mpInit->mpInventory->IsActive())
		mpInit->mpInventory->SetActive(false);
	if (mpInit->mpNotebook->IsActive())
		mpInit->mpNotebook->SetActive(false);
	if (mpInit->mpNumericalPanel->IsActive())
		mpInit->mpNumericalPanel->SetActive(false);
	mpInit->mpPlayer->GetFlashLight()->SetActive(false);
	mpInit->mpPlayer->GetGlowStick()->SetActive(false);
	mpInit->mpPlayer->GetFlare()->SetActive(false);
	mpInit->mpPlayerHands->SetCurrentModel(0, "");
	mpInit->mpPlayerHands->SetCurrentModel(1, "");

	mpInit->mpPlayer->ChangeState(ePlayerState_Normal);

	mfHeightAdd = 0;
	mfRoll = 0;

	mfMinHeightAdd = -1.3f;
	if (mpInit->mpPlayer->GetMoveState() == ePlayerMoveState_Crouch)
		mfMinHeightAdd = -0.7f;

	mbActive = true;
	mbStartFade = false;

	mfFadeAlpha = 0;
	mfBlackAlpha = 0;

	mpPostEffects->SetImageTrailActive(true);
	mpPostEffects->SetImageTrailAmount(0.7f);

	mpInit->mpPlayer->GetEarRing()->Stop(false);

	for (int i = 0; i <= 10; ++i) {
		mpInit->mpMusicHandler->Stop(0.2f, i);
	}
}

//-----------------------------------------------------------------------

void cPlayerDeath::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	mfHeightAdd -= 0.95f * afTimeStep;
	if (mfHeightAdd < mfMinHeightAdd) {
		mfHeightAdd = mfMinHeightAdd;
		mbStartFade = true;
		mpInit->mpDeathMenu->SetActive(true);
	}

	mfRoll += cMath::ToRad(40.0f) * afTimeStep;
	if (mfRoll > cMath::ToRad(65.0f))
		mfRoll = cMath::ToRad(65.0f);

	mpInit->mpPlayer->GetCamera()->AddRoll(mfRoll);

	if (mbStartFade) {
		mfFadeAlpha += 0.7f * afTimeStep;
		if (mfFadeAlpha > 1)
			mfFadeAlpha = 1;

		if (mfFadeAlpha > 0.6f) {
			mfBlackAlpha += 0.45f * afTimeStep;
			if (mfBlackAlpha > 1)
				mfBlackAlpha = 1;
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerDeath::Draw() {
	if (mbActive == false)
		return;

	cVector3f vPos(-mfFadeAlpha * 400 - mfBlackAlpha * 200, -mfFadeAlpha * 400 - mfBlackAlpha * 200, 4);
	cVector2f vSize(800 + mfFadeAlpha * 800 + mfBlackAlpha * 400, 600 + mfFadeAlpha * 800 + mfBlackAlpha * 400);
	mpDrawer->DrawGfxObject(mpFadeGfx, vPos, vSize, cColor(mfFadeAlpha, 0));

	mpDrawer->DrawGfxObject(mpBlackGfx, cVector3f(0, 0, 5), cVector2f(800, 600), cColor(mfBlackAlpha, 0));
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// FLASHLIGHT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerFlashLight::cPlayerFlashLight(cInit *apInit) {
	mpInit = apInit;

	mpInit->mpPlayerHands->AddModelFromFile("hud_object_flashlight.hud");

	Reset();
}

//-----------------------------------------------------------------------

cPlayerFlashLight::~cPlayerFlashLight() {
}

//-----------------------------------------------------------------------

void cPlayerFlashLight::Reset() {
	mbActive = false;
	mfAlpha = 0;
	mfFlickerTime = 0;

	mfRayCastTime = 0;
	mpClosestBody = NULL;
	mfClosestDist = 10000.0f;

	mbDisabled = false;
}

//-----------------------------------------------------------------------

bool cPlayerFlashLight::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	if (pBody->GetCollide() == false)
		return true;
	if (pBody->IsCharacter() && pBody == mpInit->mpPlayer->GetCharacterBody()->GetBody())
		return true;

	if (apParams->mfDist < mfClosestDist) {
		mfClosestDist = apParams->mfDist;
		mpClosestBody = pBody;
	}

	return true;
}

//-----------------------------------------------------------------------

void cPlayerFlashLight::OnWorldLoad() {
}

//-----------------------------------------------------------------------

void cPlayerFlashLight::Update(float afTimeStep) {
	if (mbActive) {
		//////////////////////////
		// Hits enemy
		if (mfRayCastTime >= 1) // 1.0f/20.0f)
		{
			mfRayCastTime = 0;

			cCamera3D *pCam = mpInit->mpPlayer->GetCamera();
			iPhysicsWorld *pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();

			iHudModel *pHudModel = mpInit->mpPlayerHands->GetModel("Flashlight");

			cVector3f vForward = pCam->GetForward();
			if (pHudModel && pHudModel->GetEntity()) {
				vForward = cMath::MatrixMul(
					pHudModel->GetEntity()->GetWorldMatrix().GetRotation(),
					cVector3f(0, -1, 0));
			}

			if (pHudModel == mpInit->mpPlayerHands->GetCurrentModel(0)) {
				tGameEnemyIterator it = mpInit->mpMapHandler->GetGameEnemyIterator();
				while (it.HasNext()) {
					iGameEnemy *pEnemy = it.Next();

					if (pEnemy->GetHealth() <= 0 || pEnemy->IsActive() == false)
						continue;

					cVector3f vToEnemy = pEnemy->GetMover()->GetCharBody()->GetPosition() -
										 mpInit->mpPlayer->GetCharacterBody()->GetPosition();
					float fSqrDist = vToEnemy.SqrLength();
					vToEnemy.Normalise();

					if (fSqrDist < 100.0f) {
						float fAngle = cMath::Vector3Angle(vToEnemy, vForward);

						if (fAngle < cMath::ToRad(25)) {
							cVector3f vStart = pCam->GetPosition();
							cVector3f vEnd = pEnemy->GetMover()->GetCharBody()->GetPosition();

							mvStart = vStart;
							mvEnd = vEnd;

							mpClosestBody = NULL;
							mfClosestDist = 10000.0f;
							pPhysicsWorld->CastRay(this, vStart, vEnd, true, false, false, false);

							if (mpClosestBody == NULL || mpClosestBody->IsCharacter() == true) {
								if (pEnemy->GetUsesTriggers()) {
									pEnemy->OnFlashlight(pCam->GetPosition());
								}
							}
						}
					}
				}

				//////////////////////////////
				// Get forward
				// cVector3f vForward = pCam->GetForward();

				/*float fAngleX = cMath::RandRectf(cMath::ToRad(-3),cMath::ToRad(3));
				float fAngleY = cMath::RandRectf(0,k2Pif);
				cVector3f vForward = cMath::MatrixMul(
									cMath::MatrixRotate(cVector3f(fAngleX,fAngleY,0),eEulerRotationOrder_XYZ),
									cVector3f(0,1,0));

				vForward =	cMath::MatrixMul(
							pHudModel->GetEntity()->GetWorldMatrix().GetRotation(),
							//cMath::MatrixInverse(pCam->GetViewMatrix().GetRotation()),
							//cMath::MatrixRotate(cVector3f(pCam->GetPitch(),pCam->GetYaw(),pCam->GetRoll()),
							//					eEulerRotationOrder_YXZ),
											vForward*-1);

				//////////////////////////////
				//Get start and end
				cVector3f vStart = pCam->GetPosition();
				cVector3f vEnd = pCam->GetPosition() +  vForward* 20.0f;

				mvStart =vStart;
				mvEnd = vEnd;

				mpClosestBody = NULL; mfClosestDist = 10000.0f;
				pPhysicsWorld->CastRay(this,vStart,vEnd,true,false,false,false);

				if(mpClosestBody)
				{
					iGameEntity *pEntity = (iGameEntity*)mpClosestBody->GetUserData();

					if( pEntity && pEntity->GetType() == eGameEntityType_Enemy &&
						pEntity->IsActive() && pEntity->GetHealth() >0)
					{
						iGameEnemy *pEnemy = static_cast<iGameEnemy*>(pEntity);

						if(pEnemy->GetUsesTriggers())
							pEnemy->OnFlashlight(vStart);
					}
				}*/

				// mpInit->mpEffectHandler->GetSubTitle()->Add("Cast rays!\n",0.5f);
			}
		} else {
			mfRayCastTime += afTimeStep;
		}
	}

	iHudModel *pHudModel = mpInit->mpPlayerHands->GetCurrentModel(0);

	//////////////////////////
	// Player power
	if (mbActive &&
		mpInit->mpInventory->IsActive() == false &&
		mpInit->mpNotebook->IsActive() == false &&
		mpInit->mpNumericalPanel->IsActive() == false) {
		mpInit->mpPlayer->AddPower(-0.1f * afTimeStep);
		if (mpInit->mpPlayer->GetPower() == 0) {
			SetActive(false);
		}
	}

	//////////////////////////
	// Flicker at low energy
	if (mbActive && mpInit->mpInventory->IsActive() == false &&
		pHudModel->msName == "Flashlight") {
		if (mpInit->mpPlayer->GetPower() < 3.0f) {
			if (mfFlickerTime <= 0) {
				if (pHudModel->mvLights[0]->IsVisible()) {
					pHudModel->mvLights[0]->SetVisible(false);
					pHudModel->mvBillboards[0]->SetVisible(false);

					mfFlickerTime = cMath::RandRectf(0.04f, 0.15f);

					cSoundHandler *pSoundHanlder = mpInit->mpGame->GetSound()->GetSoundHandler();

					pSoundHanlder->PlayGui("item_flashlight_flicker", false, 1);
				} else {
					pHudModel->mvLights[0]->SetVisible(true);
					pHudModel->mvBillboards[0]->SetVisible(true);
					mfFlickerTime = cMath::RandRectf(0.05f, 1.5f);
				}
			} else {
				mfFlickerTime -= afTimeStep;
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerFlashLight::SetActive(bool abX) {
	cSoundHandler *pSoundHanlder = mpInit->mpGame->GetSound()->GetSoundHandler();

	if (abX && (mpInit->mpPlayer->GetPower() == 0 || mbDisabled)) {
		pSoundHanlder->PlayGui("item_flashlight_nopower", false, 1);
		return;
	}

	if (mbActive == abX)
		return;
	mbActive = abX;

	/////////////////////////////
	// Active
	if (mbActive) {
		mpInit->mpPlayerHands->SetCurrentModel(0, "Flashlight");

		// pSoundHanlder->PlayGui("item_flashlight_on",false,1);
	}
	/////////////////////////////
	// Not active
	else if (mpInit->mpPlayerHands->GetCurrentModel(0) &&
			 mpInit->mpPlayerHands->GetCurrentModel(0)->msName == "Flashlight" &&
			 mpInit->mpPlayerHands->GetCurrentModel(0)->GetState() != eHudModelState_Unequip) {
		mpInit->mpPlayerHands->SetCurrentModel(0, "");

		if (mpInit->mpPlayer->GetPower() == 0)
			pSoundHanlder->PlayGui("item_flashlight_nopower", false, 1);
		// else
		//	pSoundHanlder->PlayGui("item_flashlight_off",false,1);
	}
}

//-----------------------------------------------------------------------

void cPlayerFlashLight::SetDisabled(bool abX) {
	if (abX == true)
		SetActive(false);

	mbDisabled = abX;
}

//-----------------------------------------------------------------------

void cPlayerFlashLight::OnPostSceneDraw() {
	/*iLowLevelGraphics *mpLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();

	mpLowGfx->DrawLine(mvStart,mvEnd,cColor(1,0,1));*/
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// GLOW STICK
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerGlowStick::cPlayerGlowStick(cInit *apInit) {
	mpInit = apInit;

	mpInit->mpPlayerHands->AddModelFromFile("hud_object_glowstick.hud");

	Reset();
}

//-----------------------------------------------------------------------

cPlayerGlowStick::~cPlayerGlowStick() {
}

//-----------------------------------------------------------------------

void cPlayerGlowStick::Reset() {
	mbActive = false;
}

//-----------------------------------------------------------------------

void cPlayerGlowStick::OnWorldLoad() {
}

//-----------------------------------------------------------------------

void cPlayerGlowStick::Destroy() {
}

//-----------------------------------------------------------------------

void cPlayerGlowStick::Update(float afTimeStep) {
}

//-----------------------------------------------------------------------

void cPlayerGlowStick::SetActive(bool abX) {
	if (mbActive == abX)
		return;
	mbActive = abX;

	cSoundHandler *pSoundHanlder = mpInit->mpGame->GetSound()->GetSoundHandler();

	if (mbActive) {
		// Log("Setting the glowstick to TRUE\n");
		mpInit->mpPlayerHands->SetCurrentModel(0, "Glowstick");
		// pSoundHanlder->PlayGui("item_glowstick_on",false,1);
	} else if (mpInit->mpPlayerHands->GetCurrentModel(0) &&
			   mpInit->mpPlayerHands->GetCurrentModel(0)->msName == "Glowstick" &&
			   mpInit->mpPlayerHands->GetCurrentModel(0)->GetState() != eHudModelState_Unequip) {
		// Log("Setting the glowstick to FALSE\n");
		mpInit->mpPlayerHands->SetCurrentModel(0, "");
		// pSoundHanlder->PlayGui("item_glowstick_off",false,1);
	}
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// FLARE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerFlare::cPlayerFlare(cInit *apInit) {
	mpInit = apInit;

	mpInit->mpPlayerHands->AddModelFromFile("hud_object_flare.hud");

	mfTime = mpInit->mpGameConfig->GetFloat("Items", "FlareGlowTime", 1);

	mfLightPulse = 1.0f;
	mfLightPulseAdd = 0.88f;

	mpLight = NULL;

	Reset();
}

//-----------------------------------------------------------------------

cPlayerFlare::~cPlayerFlare() {
}

//-----------------------------------------------------------------------

void cPlayerFlare::Reset() {
	mbActive = false;
	mpLight = NULL;
}

//-----------------------------------------------------------------------

void cPlayerFlare::OnWorldLoad() {
	mpLight = NULL;
}

//-----------------------------------------------------------------------

void cPlayerFlare::Update(float afTimeStep) {
	if (mbActive) {
		//////////////////////////////////////
		// Model entities are loaded and light gotten
		if (mpLight) {
			//////////////////////
			// Fade at end
			if (mfTime <= 30) {
				mfLightRadius = (mfTime / 30.0f) * mfMaxLightRadius;
			}

			float fRadius = mfLightRadius;

			//////////////////////
			// Pulse
			mfLightPulse += mfLightPulseAdd * afTimeStep;
			if (mfLightPulseAdd > 0 && mfLightPulse >= 1.0f) {
				mfLightPulse = 1.0f;
				mfLightPulseAdd = -mfLightPulseAdd;
			}
			if (mfLightPulseAdd < 0 && mfLightPulse <= 0.0f) {
				mfLightPulse = 0.0f;
				mfLightPulseAdd = -mfLightPulseAdd;
			}

			fRadius = (mfLightPulse * 0.4f + 0.8f) * mfLightRadius;

			if (mpLight)
				mpLight->SetFarAttenuation(fRadius);

			//////////////////////
			// Time
			if (mpInit->mpInventory->IsActive() == false &&
				mpInit->mpNotebook->IsActive() == false &&
				mpInit->mpNumericalPanel->IsActive() == false) {
				mfTime -= afTimeStep;
				if (mfTime <= 0) {
					SetActive(false);
				}
			}
		}
		//////////////////////////////////////
		// Model entities not loaded yet
		else {
			if (mpModel == mpInit->mpPlayerHands->GetCurrentModel(0)) {
				if (mpModel->mvLights.empty() == false) {
					mpLight = mpModel->mvLights[0];
					mfMaxLightRadius = mpLight->GetFarAttenuation();
					mfLightRadius = mfMaxLightRadius;
				} else {
					Warning("No lights in flare!\n");
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerFlare::SetActive(bool abX) {
	if (mbActive == abX)
		return;
	mbActive = abX;

	cSoundHandler *pSoundHanlder = mpInit->mpGame->GetSound()->GetSoundHandler();

	if (mbActive) {
		mpInit->mpPlayerHands->SetCurrentModel(0, "Flare");

		if (mpInit->mpPlayer->GetFlashLight()->IsActive())
			mpInit->mpPlayer->GetFlashLight()->SetActive(false);
		if (mpInit->mpPlayer->GetGlowStick()->IsActive())
			mpInit->mpPlayer->GetGlowStick()->SetActive(false);

		mfTime = mpInit->mpGameConfig->GetFloat("Items", "FlareGlowTime", 1);

		mpModel = mpInit->mpPlayerHands->GetModel("Flare");
		mpLight = NULL;
	} else {
		///////////////////////////////
		// Check if hud model should be put down.
		if (mpInit->mpPlayerHands->GetCurrentModel(0) &&
			mpInit->mpPlayerHands->GetCurrentModel(0)->msName == "Flare" &&
			mpInit->mpPlayerHands->GetCurrentModel(0)->GetState() != eHudModelState_Unequip) {
			mpInit->mpPlayerHands->SetCurrentModel(0, "");
		}

		///////////////////////////////
		// Create entity
		cCamera3D *pCam = mpInit->mpPlayer->GetCamera();

		cVector3f vRot = cVector3f(pCam->GetPitch(), pCam->GetYaw(), pCam->GetRoll());
		cMatrixf mtxStart = cMath::MatrixRotate(vRot, eEulerRotationOrder_XYZ);
		mtxStart.SetTranslation(pCam->GetPosition());

		iEntity3D *pEntity = mpInit->mpGame->GetScene()->GetWorld3D()->CreateEntity("Throw", mtxStart,
																					"items_flare_thrown.ent", true);
		if (pEntity) {
			iGameEntity *pEntity = mpInit->mpMapHandler->GetLatestEntity();

			cVector3f vRot = cMath::MatrixMul(mtxStart.GetRotation(), cVector3f(1, 0.3f, 0));

			for (int i = 0; i < pEntity->GetBodyNum(); ++i) {
				iPhysicsBody *pBody = pEntity->GetBody(i);
				pBody->AddImpulse(pCam->GetForward() * 3.0f);
				pBody->AddTorque(vRot);
			}

			// setup light
			if (mfTime <= 30) {
				float fRadius = (mfTime / 30.0f);
				if (fRadius < 0)
					fRadius = 0;

				for (int i = 0; i < pEntity->GetLightNum(); ++i) {
					iLight3D *pLight = pEntity->GetLight(i);
					pLight->SetFarAttenuation(pLight->GetFarAttenuation() * fRadius);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// NOISE FILTER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerNoiseFilter::cPlayerNoiseFilter(cInit *apInit) {
	mpInit = apInit;
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mlAmount = 6;

	for (int i = 0; i < mlAmount; ++i) {
		tString sFileName = "effect_noise0" + cString::ToString(i);
		cGfxObject *pObject = mpDrawer->CreateGfxObject(sFileName, "smoke2d");
		if (pObject == NULL) {
			error("Error loading noise filter");
		}

		mvGfxNoise.push_back(pObject);
	}

	mvCurrentGfx.resize(20 * 15, cMath::RandRectl(0, mlAmount - 1));

	mfAlpha = 0.3f;
	mfTimeCount = 0;

	mbActive = mpInit->mpConfig->GetBool("Graphics", "NoiseFilter", true);
}

//-----------------------------------------------------------------------

cPlayerNoiseFilter::~cPlayerNoiseFilter() {
	mpInit->mpConfig->SetBool("Graphics", "NoiseFilter", mbActive);

	for (size_t i = 0; i < mvGfxNoise.size(); ++i) {
		mpDrawer->DestroyGfxObject(mvGfxNoise[i]);
	}
}

//-----------------------------------------------------------------------

void cPlayerNoiseFilter::Draw() {
	if (mbActive == false)
		return;

	for (int x = 0; x < 20; ++x)
		for (int y = 0; y < 15; ++y) {
			cVector3f vPos((float)x * 40, (float)y * 40, 0);

			mpDrawer->DrawGfxObject(mvGfxNoise[mvCurrentGfx[x * y]], vPos, 40, cColor(mfAlpha, 0));
			// mpDrawer->DrawGfxObject(mvGfxNoise[0],vPos,40,cColor(mfAlpha,mfAlpha));
		}
}

//-----------------------------------------------------------------------

void cPlayerNoiseFilter::Update(float afTimeStep) {
	if (mbActive == false)
		return;
	if (mfTimeCount <= 0) {
		for (int x = 0; x < 20; ++x)
			for (int y = 0; y < 15; ++y) {
				// Change the image
				int lCurrent = mvCurrentGfx[x * y];
				int lRand = cMath::RandRectl(0, mlAmount - 1);
				if (lRand == lCurrent) {
					++lRand;
					if (lRand >= mlAmount)
						lRand = 0;
				}
				mvCurrentGfx[x * y] = lRand;
			}

		mfTimeCount = 1 / 15.0f;
	} else {
		mfTimeCount -= afTimeStep;
	}
}

//-----------------------------------------------------------------------

void cPlayerNoiseFilter::SetActive(bool abX) {
	mbActive = abX;
}

//-----------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
// FEAR FILTER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerFearFilter::cPlayerFearFilter(cInit *apInit) {
	mpInit = apInit;

	Reset();
}

//-----------------------------------------------------------------------

cPlayerFearFilter::~cPlayerFearFilter() {
}

//-----------------------------------------------------------------------

void cPlayerFearFilter::Draw() {
}

//-----------------------------------------------------------------------

void cPlayerFearFilter::Update(float afTimeStep) {
	if (mpInit->mpPlayer->IsDead())
		return;

	if (mbActive) {
		mfAlpha += afTimeStep * 0.5f;
		if (mfAlpha > mfMaxAlpha)
			mfAlpha = mfMaxAlpha;
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailAmount(mfAlpha);
	} else if (mfAlpha > 0) {
		mfAlpha -= afTimeStep * 0.15f;
		if (mfAlpha < 0) {
			mfAlpha = 0;
			mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(false);
		} else {
			mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailAmount(mfAlpha);
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerFearFilter::Reset() {
	mbActive = false;
	mfAlpha = 0;
	mfMaxAlpha = 0.58f;
}

//-----------------------------------------------------------------------

void cPlayerFearFilter::SetActive(bool abX) {
	if (mbActive == abX)
		return;

	mbActive = abX;
	mfAlpha = 0;

	mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(true);
	mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailAmount(0);
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// LOOK AT
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerLookAt::cPlayerLookAt(cPlayer *apPlayer) {
	mpPlayer = apPlayer;

	Reset();
}

//-----------------------------------------------------------------------

cPlayerLookAt::~cPlayerLookAt() {
}

//-----------------------------------------------------------------------

void cPlayerLookAt::Update(float afTimeStep) {
	if (mbActive == false)
		return;

	cCamera3D *pCam = mpPlayer->GetCamera();
	cVector3f vGoalAngle = cMath::GetAngleFromPoints3D(pCam->GetPosition(), mvTargetPos);

	// Get distance to goal
	cVector3f vDist;
	vDist.x = cMath::GetAngleDistanceRad(pCam->GetPitch(), vGoalAngle.x);
	vDist.y = cMath::GetAngleDistanceRad(pCam->GetYaw(), vGoalAngle.y);

	// Get the Speed
	cVector3f vSpeed;
	vSpeed.x = cMath::Min(vDist.x * mfSpeedMul, mfMaxSpeed);
	vSpeed.y = cMath::Min(vDist.y * mfSpeedMul, mfMaxSpeed);

	// Add Pitch
	pCam->AddPitch(vSpeed.x * afTimeStep);

	// Add yaw
	pCam->AddYaw(vSpeed.y * afTimeStep);
	mpPlayer->GetCharacterBody()->SetYaw(pCam->GetYaw());
}

void cPlayerLookAt::Reset() {
	mbActive = false;
	mfMaxSpeed = 9999.0f;
	mfSpeedMul = 1.0f;
	mvTargetPos = cVector3f(0, 1, 0);
}

void cPlayerLookAt::SetTarget(const cVector3f &avTargetPos, float afSpeedMul, float afMaxSpeed) {
	mvTargetPos = avTargetPos;
	mfSpeedMul = afSpeedMul;
	mfMaxSpeed = afMaxSpeed;
}

void cPlayerLookAt::SetActive(bool abX) {
	mbActive = abX;
}

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////
// HIDDEN
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayerHidden::cPlayerHidden(cInit *apInit) {
	mpInit = apInit;
	mpRenderer = mpInit->mpGame->GetGraphics()->GetRenderer3D();
	mpDrawer = mpInit->mpGame->GetGraphics()->GetDrawer();

	mpSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();

	mfUpdateCount = 1.0f / 3.0f;

	cVector2f vScreenSize = mpInit->mpGame->GetGraphics()->GetLowLevel()->GetScreenSize();

	mfStartEffectOffset = 40;
	mfHiddenEffectOffset = 0;
	mfEffectOffsetAdd = mfStartEffectOffset - mfHiddenEffectOffset;

	mfStartAspect = vScreenSize.x / vScreenSize.y;
	mfHiddenAspect = mfStartAspect * 0.82f;
	mfAspectAdd = mfStartAspect - mfHiddenAspect;

	mfStartFov = cMath::ToRad(70);
	mfHiddenFov = cMath::ToRad(78);
	mfFovAdd = mfHiddenFov - mfStartFov;

	mfEnemyTooCloseMax = 4.5f;

	mfCloseEffectFovMax = cMath::ToRad(2);
	mfCloseEffectFovMin = cMath::ToRad(-11);

	Reset();

	// Get images
	mpInShadowGfx = mpDrawer->CreateGfxObject("player_in_shadow.jpg", "diffadditive2d");

	// Get font
	mpFont = mpInit->mpGame->GetResources()->GetFontManager()->CreateFontData("verdana.fnt");
}

//-----------------------------------------------------------------------

cPlayerHidden::~cPlayerHidden() {
}

//-----------------------------------------------------------------------

void cPlayerHidden::OnWorldLoad() {
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	mpLight = pWorld->CreateLightPoint("HiddenLight");

	mpLight->SetFarAttenuation(13);
	mpLight->SetDiffuseColor(cColor(0, 0, 0, 0));
	mpLight->SetVisible(false);
	mpLight->SetCastShadows(false);
}

void cPlayerHidden::OnWorldExit() {
}

//-----------------------------------------------------------------------

void cPlayerHidden::Draw() {
	/*if(mfLight <= mfMaxLight)
	{
		mpFont->Draw(5,12,cColor(0.3f,1,0.3f),eFontAlign_Left,_W("Light: %f\n"),mfLight);
		if(!mbHidden)
			mpFont->Draw(cVector3f(5,19,5),12,cColor(1,1,1),eFontAlign_Left,_W("Hidden count: %f"),mfHiddenCount);
	}
	else
	{
		mpFont->Draw(5,12,cColor(1,0.3f,0.3f),eFontAlign_Left,_W("Light: %f\n"),mfLight);
	}

	if(mbHidden)
		mpFont->Draw(cVector3f(5,19,5),12,cColor(1,1,1),eFontAlign_Left,_W("Hidden\n"));
	*/
	/*float fAdd =0;
	if(mfCloseEffectFov >= 0)
	{
		float fT = mfCloseEffectFov / mfCloseEffectFovMax;
		fAdd = sin(fT * kPi2f) * mfCloseEffectFovMax;
	}
	else
	{
		float fT = mfCloseEffectFov / mfCloseEffectFovMin;
		fAdd = sin(fT * kPi2f) * mfCloseEffectFovMin;
	}

	mpFont->Draw(5,12,cColor(1,1,1),eFontAlign_Left,"CloseCount: %f TooClose: %d\n",
													mfEnemyTooCloseCount,
													mbEnemyTooClose);

	mpFont->Draw(cVector3f(5,19,5),12,cColor(1,1,1),eFontAlign_Left,"Fov: %f Add: %f\n",
													mfCloseEffectFov,fAdd);*/

	// Draw in shadow effect
	if (mfInShadowAlpha > 0) {
		float fAlpha = (mfInShadowPulse * 0.5f + 0.5f) * mfInShadowAlpha * 0.85f;
		mpDrawer->DrawGfxObject(mpInShadowGfx, cVector3f(-mfEffectOffset, -mfEffectOffset, 0),
								cVector2f(800 + mfEffectOffset * 2, 600 + mfEffectOffset * 2),
								cColor(fAlpha, fAlpha));
	}
}

//-----------------------------------------------------------------------

void cPlayerHidden::UnHide() {
	if (mbHidden) {
		mpLight->FadeTo(cColor(0, 0, 0, 0), mpLight->GetFarAttenuation(), mfHiddenOffTime);

		if (mfHiddenOnCount > 2)
			mpSoundHandler->PlayGui("player_hidden_off", false, 1);
	}
	mbHidden = false;
	mfHiddenCount = 0;
	mfHiddenOnCount = 0;

	if (mbEnemyTooClose) {
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(false);
	}

	mfEnemyTooCloseCount = 0;
	mbEnemyTooClose = false;
}

//-----------------------------------------------------------------------

bool cPlayerHidden::InShadows() {
	if (mfLight <= mfMaxLight)
		return true;

	return false;
}

//-----------------------------------------------------------------------

static float GetMaxRGB(const cColor &aCol) {
	float fAmount = aCol.r;
	if (fAmount < aCol.g)
		fAmount = aCol.g;
	if (fAmount < aCol.b)
		fAmount = aCol.b;

	return fAmount;
}

void cPlayerHidden::Update(float afTimeStep) {
	iCharacterBody *pCharBody = mpInit->mpPlayer->GetCharacterBody();

	bool bIsCrouching = false;
	if (mpInit->mpPlayer->GetMoveState() == ePlayerMoveState_Crouch)
		bIsCrouching = true;

	///////////////////////////////////
	// Update Hidden effect
	if (mbHidden) {
		// Aspect
		if (mfAspect > mfHiddenAspect) {
			mfAspect -= afTimeStep * mfAspectAdd * (1 / mfHiddenOnTime);
			if (mfAspect < mfHiddenAspect)
				mfAspect = mfHiddenAspect;

			mpInit->mpPlayer->GetCamera()->SetAspect(mfAspect);
		}

		// FOV
		if (mfFov < mfHiddenFov) {
			mfFov += afTimeStep * mfFovAdd * (1 / mfHiddenOnTime);
			if (mfFov > mfHiddenFov)
				mfFov = mfHiddenFov;

			mpInit->mpPlayer->GetCamera()->SetFOV(mfFov);
		}

		// Effect offset
		if (mfEffectOffset > mfHiddenEffectOffset) {
			mfEffectOffset -= afTimeStep * mfEffectOffsetAdd * (1 / mfHiddenOnTime);
			if (mfEffectOffset < mfHiddenEffectOffset)
				mfEffectOffset = mfHiddenEffectOffset;
		}
	} else {
		// Aspect
		if (mfAspect < mfStartAspect) {
			mfAspect += afTimeStep * mfAspectAdd * (1 / mfHiddenOffTime);
			if (mfAspect > mfStartAspect)
				mfAspect = mfStartAspect;

			mpInit->mpPlayer->GetCamera()->SetAspect(mfAspect);
		}

		// FOV
		if (mfFov > mfStartFov) {
			mfFov -= afTimeStep * mfFovAdd * (1 / mfHiddenOffTime);
			if (mfFov < mfStartFov)
				mfFov = mfStartFov;

			mpInit->mpPlayer->GetCamera()->SetFOV(mfFov);
		}

		// Effect offset
		if (mfEffectOffset < mfStartEffectOffset) {
			mfEffectOffset += afTimeStep * mfEffectOffsetAdd * (1 / mfHiddenOffTime);
			if (mfEffectOffset > mfStartEffectOffset)
				mfEffectOffset = mfStartEffectOffset;
		}
	}

	///////////////////////////////////
	// Update In Shadow effect

	// Alpha
	if (mfLight <= mfMaxLight && bIsCrouching) {
		mfInShadowAlpha += afTimeStep;
		if (mfInShadowAlpha > 1)
			mfInShadowAlpha = 1;
	} else {
		mfInShadowAlpha -= afTimeStep;
		if (mfInShadowAlpha < 0)
			mfInShadowAlpha = 0;
	}

	// Pulse
	mfInShadowPulse += afTimeStep * mfInShadowPulseAdd;
	if (mfInShadowPulseAdd < 0 && mfInShadowPulse < 0) {
		mfInShadowPulse = 0;
		mfInShadowPulseAdd = -mfInShadowPulseAdd;
	} else if (mfInShadowPulseAdd > 0 && mfInShadowPulse > 1) {
		mfInShadowPulse = 1;
		mfInShadowPulseAdd = -mfInShadowPulseAdd;
	}

	if (mbInShadows)
		mpLight->SetPosition(mpInit->mpPlayer->GetCamera()->GetPosition());

	//////////////////////////////////////////
	// Check if the player is hidden
	if (mfLight <= mfMaxLight && bIsCrouching && mpInit->mpPlayer->GetHealth() > 0) {
		cVector3f vDiff = pCharBody->GetLastPosition() - pCharBody->GetPosition();
		if (fabsf(vDiff.x) < 0.01f && fabsf(vDiff.y) < 0.01f && fabsf(vDiff.z) < 0.01f) {
			if (!mbHidden) {
				if (mfHiddenCount >= mfHiddenTime) {
					mbInShadows = false;
					mbHidden = true;
					mpLight->SetVisible(true);
					mpLight->SetPosition(mpInit->mpPlayer->GetCamera()->GetPosition());
					mpLight->FadeTo(cColor(0.1f, 0.1f, 0.6f, 0), 12, mfHiddenOnTime);

					mpSoundHandler->PlayGui("player_hidden_on", false, 1);
				} else {
					mfHiddenCount += afTimeStep;
				}
			} else {
				mfHiddenOnCount += afTimeStep;
			}
		} else {
			UnHide();
		}
		if (!mbHidden) {
			if (!mbInShadows) {
				mpLight->FadeTo(cColor(0.09f, 0.09f, 0.28f, 0), 6, 1);
				mpLight->SetVisible(true);
			}
			mbInShadows = true;
		}
	} else {
		if (mbInShadows) {
			mbInShadows = false;
			mpLight->FadeTo(cColor(0, 0, 0, 0), 8, 1);
		}

		UnHide();
	}

	//////////////////////////////////////////7
	// Update the current light level
	if (mfUpdateCount <= 0) {
		mfUpdateCount = 1.0f / 2.0f;

		mfLight = GetMaxRGB(mpInit->mpGame->GetGraphics()->GetRenderer3D()->GetAmbientColor());
		cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
		if (pWorld == NULL)
			return;
		iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();
		cBoundingVolume *pPlayerBV = pCharBody->GetBody()->GetBV();

		// Just to be sure...
		if (mpInit->mpPlayer->GetFlashLight()->IsActive())
			mfLight += 0.5;

		// Iterate lights
		cLight3DListIterator lightIt = pWorld->GetLightIterator();
		while (lightIt.HasNext()) {
			iLight3D *pLight = lightIt.Next();
			if (pLight == mpLight)
				continue;
			if (pLight->IsActive() == false || pLight->IsVisible() == false)
				continue;

			if (cMath::CheckCollisionBV(*pPlayerBV, *pLight->GetBoundingVolume())) {
				// Check line of sight
				if (HasLineOfSight(pLight, pPhysicsWorld) == false)
					continue;

				// Get highest value of rg b
				float fAmount = GetMaxRGB(pLight->GetDiffuseColor());

				// Get distance to the light
				float fDist = cMath::Vector3Dist(pLight->GetWorldPosition(),
												 pPlayerBV->GetWorldCenter());

				// Calculate attenuation
				float fT = 1 - fDist / pLight->GetFarAttenuation();
				if (fT < 0)
					fT = 0;
				fAmount *= fT;

				mfLight += fAmount;
			}
		}

	} else {
		mfUpdateCount -= afTimeStep;
	}

	////////////////////////////////////////////////////
	// Check if enemy is too close and act accordingly.
	UpdateEnemyTooClose(afTimeStep);
}

//-----------------------------------------------------------------------

bool cPlayerHidden::OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
	// bool bDebug = pBody->GetName() == "polySurface76";

	if (pBody->IsCharacter() || pBody->GetCollide() == false || pBody->GetBlocksLight() == false) {
		// if(bDebug)Log("Out on first!\n");
		return true;
	}

	iGameEntity *pEntity = (iGameEntity *)pBody->GetUserData();
	if (pEntity && pEntity->GetMeshEntity()) {
		cMeshEntity *pMeshEntity = pEntity->GetMeshEntity();
		if (pMeshEntity->IsShadowCaster() == false)
			return true;

		bool bFoundSolid = false;
		for (int i = 0; i < pMeshEntity->GetSubMeshEntityNum(); ++i) {
			iMaterial *pMaterial = pMeshEntity->GetSubMeshEntity(i)->GetMaterial();
			if (pMaterial &&
				(pMaterial->IsTransperant() == false && pMaterial->HasAlpha() == false)) {
				bFoundSolid = true;
				break;
			}
		}
		if (bFoundSolid == false) {
			// if(bDebug)Log("Out on non solid!\n");
			return true;
		}
	}

	mbIntersected = true;
	// if(bDebug)Log("Intersection!!\n");
	return false;
}

//-----------------------------------------------------------------------

bool cPlayerHidden::HasLineOfSight(iLight3D *pLight, iPhysicsWorld *pPhysicsWorld) {
	if (pLight->GetCastShadows() == false)
		return true;

	if (mpRenderer->GetShowShadows() == eRendererShowShadows_None)
		return true;

	iCharacterBody *pCharBody = mpInit->mpPlayer->GetCharacterBody();
	cVector3f vPlayerPos = pCharBody->GetPosition();

	// Check frustum of spotlight
	if (pLight->GetLightType() == eLight3DType_Spot) {
		cLight3DSpot *pSpotLight = static_cast<cLight3DSpot *>(pLight);

		if (pSpotLight->GetFrustum()->CollideBoundingVolume(pLight->GetBoundingVolume()) ==
			eFrustumCollision_Outside) {
			return false;
		}
	}

	int lCount = 0;

	cVector3f vPosAdd[5];
	int lPosAddNum = 5;

	// Centre
	vPosAdd[0] = cVector3f(0, 0, 0);
	// Head
	vPosAdd[1] = cVector3f(0, pCharBody->GetSize().y / 2 - 0.1f, 0);
	// Feet
	vPosAdd[2] = cVector3f(0, -(pCharBody->GetSize().y / 2 + 0.1f), 0);

	cVector3f vRight = cMath::Vector3Cross(cMath::Vector3Normalize(pLight->GetWorldPosition() - vPlayerPos),
										   cVector3f(0, 1, 0));
	// Right
	vPosAdd[3] = vRight * (pCharBody->GetSize().x * 0.4f);
	// Left
	vPosAdd[4] = vRight * (pCharBody->GetSize().x * -0.4f);

	for (int i = 0; i < lPosAddNum; ++i) {
		mbIntersected = false;
		pPhysicsWorld->CastRay(this, vPlayerPos + vPosAdd[i], pLight->GetWorldPosition(),
							   false, false, false, false);

		if (mbIntersected)
			++lCount;
		else {
			break;
		}
	}

	return lCount < lPosAddNum;
}

//-----------------------------------------------------------------------

void cPlayerHidden::UpdateEnemyTooClose(float afTimeStep) {
	cCamera3D *pCam = mpInit->mpPlayer->GetCamera();
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	if (pWorld == NULL)
		return;

	/////////////////////////////
	// Update too close effect
	if (mbEnemyTooClose && mfEnemyTooCloseCount > 1.2f) {
		if (mfCloseEffectFovAdd > 0) {
			mfCloseEffectFov += mfCloseEffectFovAdd * afTimeStep;
			if (mfCloseEffectFov >= mfCloseEffectFovMax) {
				mfCloseEffectFovAdd = -mfCloseEffectFovAdd;
				mfCloseEffectFov = mfCloseEffectFovMax;

				if (mfEnemyTooCloseCount > mfEnemyTooCloseMax * 0.4f)
					mpSoundHandler->PlayGui("player_scare_mid", false, 1);
				else
					mpSoundHandler->PlayGui("player_scare_low", false, 1);
			}
		} else {
			mfCloseEffectFov += mfCloseEffectFovAdd * afTimeStep;
			if (mfCloseEffectFov <= mfCloseEffectFovMin) {
				mfCloseEffectFovAdd = -mfCloseEffectFovAdd;
				mfCloseEffectFov = mfCloseEffectFovMin;

				// if(mfEnemyTooCloseCount > mfEnemyTooCloseMax*0.6f)
				//	mpSoundHandler->PlayGui("player_scare_mid",false,1);
				// else
				//	mpSoundHandler->PlayGui("player_scare_low",false,1);
			}
		}

		float fAdd = 0;
		if (mfCloseEffectFov >= 0) {
			float fT = mfCloseEffectFov / mfCloseEffectFovMax;
			fAdd = sin(fT * kPi2f) * mfCloseEffectFovMax;
		} else {
			float fT = mfCloseEffectFov / mfCloseEffectFovMin;
			fAdd = sin(fT * kPi2f) * mfCloseEffectFovMin;
		}

		pCam->SetFOV(mfFov + fAdd);

		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(true);
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailAmount(0.8f);
	} else if (mfCloseEffectFov != 0) {
		mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(false);

		if (mfCloseEffectFov < 0) {
			mfCloseEffectFov += afTimeStep * cMath::ToRad(7);
			if (mfCloseEffectFov > 0)
				mfCloseEffectFov = 0;
		} else {
			mfCloseEffectFov -= afTimeStep * cMath::ToRad(7);
			if (mfCloseEffectFov < 0)
				mfCloseEffectFov = 0;
		}

		float fAdd = 0;
		if (mfCloseEffectFov >= 0) {
			float fT = mfCloseEffectFov / mfCloseEffectFovMax;
			fAdd = sin(fT * kPi2f) * mfCloseEffectFovMax;
		} else {
			float fT = mfCloseEffectFov / mfCloseEffectFovMin;
			fAdd = sin(fT * kPi2f) * mfCloseEffectFovMin;
		}

		pCam->SetFOV(mfFov + fAdd);
	}

	/////////////////////////////
	// Update the count
	if (InShadows()) // mbHidden)
	{
		if (mfCheckEnemyCloseCount <= 0) {
			float fMaxDist = mbHidden ? 11.0f : 11.0f;
			float fMaxDistSqr = fMaxDist * fMaxDist;

			// Reset variables
			mbEnemyTooClose = false;
			mfCheckEnemyCloseCount = 0.3f;

			/// Get needed data
			iPhysicsWorld *pPhysicsWorld = pWorld->GetPhysicsWorld();

			// Iterate enemies
			tGameEnemyIterator enemyIt = mpInit->mpMapHandler->GetGameEnemyIterator();
			while (enemyIt.HasNext()) {
				iGameEnemy *pEnemy = enemyIt.Next();

				if (pEnemy->GetHealth() <= 0 || pEnemy->IsActive() == false)
					continue;

				iCharacterBody *pCharBody = pEnemy->GetMover()->GetCharBody();

				// Check if the enemy is close enough
				float fDistSqr = cMath::Vector3DistSqr(pCam->GetPosition(), pCharBody->GetPosition());
				if (fDistSqr > fMaxDistSqr)
					continue;

				// Check if enemy is in FOV
				cVector3f vDir = pCharBody->GetPosition() - pCam->GetPosition();
				vDir.Normalise();

				float fAngle = cMath::Vector3Angle(pCam->GetForward(), vDir);
				if (fAngle > pCam->GetFOV() * 0.5f)
					continue;

				// Check if ther is a line of sight
				mbIntersected = false;
				pPhysicsWorld->CastRay(this, pCam->GetPosition(), pCharBody->GetPosition(),
									   false, false, false, false);
				if (mbIntersected == false) {
					mbEnemyTooClose = true;
					break;
				}
			}
		} else {
			mfCheckEnemyCloseCount -= afTimeStep;
		}

		////////////////////////////////
		// Update the count if enemy is near
		if (mbEnemyTooClose) {
			mfEnemyTooCloseCount += afTimeStep;

			// Do some effect

			// Check if limit is reached
			if (mfEnemyTooCloseCount > mfEnemyTooCloseMax) {
				UnHide();
				mpInit->mpPlayer->ChangeMoveState(ePlayerMoveState_Walk);
				mpSoundHandler->PlayGui("player_scare_high", false, 1);

				cSoundEntity *pSound = pWorld->CreateSoundEntity("Scare", "player_scare_high", true);
				if (pSound) {
					pSound->SetPosition(pCam->GetPosition());
				}

				// Show player to nearby enemies.
				tGameEnemyIterator enemyIt = mpInit->mpMapHandler->GetGameEnemyIterator();
				while (enemyIt.HasNext()) {
					iGameEnemy *pEnemy = enemyIt.Next();

					if (pEnemy->GetHealth() <= 0 || pEnemy->IsActive() == false)
						continue;
					iCharacterBody *pCharBody = pEnemy->GetMover()->GetCharBody();

					// Check if the enemy is close enough
					float fDist = cMath::Vector3Dist(pCam->GetPosition(), pCharBody->GetPosition());

					if (fDist <= 8) {
						pEnemy->ShowPlayer(mpInit->mpPlayer->GetCharacterBody()->GetFeetPosition());
					}
				}
			}
		} else if (mfEnemyTooCloseCount > 0) {
			mfEnemyTooCloseCount -= afTimeStep;
			mpInit->mpGame->GetGraphics()->GetRendererPostEffects()->SetImageTrailActive(false);
		}
	}
}

//-----------------------------------------------------------------------

void cPlayerHidden::Reset() {
	mpLight = NULL;

	mfHiddenOnTime = 1;
	mfHiddenOffTime = 0.5f;

	mfHiddenOnCount = 0;

	mfEffectOffset = mfStartEffectOffset;
	mfAspect = mfStartAspect;
	mfFov = mfStartFov;

	mbInShadows = false;

	mfLight = 1.0f;
	mfMaxLight = 0.25f;

	mfHiddenCount = 0;
	mfHiddenTime = 1.4f;
	mbHidden = false;

	mfInShadowAlpha = 0;
	mfInShadowPulse = 0;
	mfInShadowPulseAdd = 1;

	mfCheckEnemyCloseCount = 0;
	mfEnemyTooCloseCount = 0;
	mbEnemyTooClose = false;

	mfCloseEffectFovAdd = cMath::ToRad(15);
	mfCloseEffectFov = 0;
}

//-----------------------------------------------------------------------
