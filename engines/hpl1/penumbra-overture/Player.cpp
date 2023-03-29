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

#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/GameEntity.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/PlayerHelper.h"
#include "hpl1/penumbra-overture/PlayerMoveStates.h"

#include "hpl1/penumbra-overture/PlayerState_Interact.h"
#include "hpl1/penumbra-overture/PlayerState_Misc.h"
#include "hpl1/penumbra-overture/PlayerState_Weapon.h"

#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/SaveHandler.h"

#include "hpl1/penumbra-overture/TriggerHandler.h"
#include "hpl1/penumbra-overture/Triggers.h"

#include "hpl1/penumbra-overture/GlobalInit.h"

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cPlayer::cPlayer(cInit *apInit) : iUpdateable("Player") {
	mpInit = apInit;

	mpScene = apInit->mpGame->GetScene();
	mpGraphics = apInit->mpGame->GetGraphics();
	mpGfxDrawer = mpGraphics->GetDrawer();
	mpResources = apInit->mpGame->GetResources();

	// Create and setup camera
	mpCamera = mpScene->CreateCamera3D(eCameraMoveMode_Walk);
	mpScene->SetCamera(mpCamera);

	// Get Debug variables
	mbShowHealth = mpInit->mpConfig->GetBool("Debug", "ShowHealth", false);
	mbShowSoundsPlaying = mpInit->mpConfig->GetBool("Debug", "ShowSoundsPlaying", false);

	mvSize.x = mpInit->mpGameConfig->GetFloat("Player", "Width", 1);
	mvSize.y = mpInit->mpGameConfig->GetFloat("Player", "Height", 1);
	mvSize.z = mvSize.x;

	mpPushBody = nullptr;

	mfCameraHeightAdd = mpInit->mpGameConfig->GetFloat("Player", "CameraHeightAdd", 0);

	mfDefaultMass = mpInit->mpGameConfig->GetFloat("Player", "Mass", 1);
	mfMass = mfDefaultMass;

	mfCrouchHeight = mpInit->mpGameConfig->GetFloat("Player", "CrouchHeight", 1);
	;

	mfSpeedMul = 1.0f;

	mfHeadMoveSizeMul = 1.0f;
	mfHeadMoveSpeedMul = 1.0f;

	mItemFlash.SetUp(0.3f, 1, 0, 1, 1);

	// jump properties
	mbJumpButtonDown = false;
	mfJumpCount = 0;
	mfMaxJumpCount = mpInit->mpGameConfig->GetFloat("Player", "MaxJumpCount", 1);

	// Create and init player states
	mState = ePlayerState_Normal;

	mvStates.resize(ePlayerState_LastEnum);
	mvStates[ePlayerState_Normal] = hplNew(cPlayerState_Normal, (mpInit, this));
	mvStates[ePlayerState_Push] = hplNew(cPlayerState_Push, (mpInit, this));
	mvStates[ePlayerState_Move] = hplNew(cPlayerState_Move, (mpInit, this));
	mvStates[ePlayerState_InteractMode] = hplNew(cPlayerState_InteractMode, (mpInit, this));
	mvStates[ePlayerState_Grab] = hplNew(cPlayerState_Grab, (mpInit, this));
	mvStates[ePlayerState_WeaponMelee] = hplNew(cPlayerState_WeaponMelee, (mpInit, this));
	mvStates[ePlayerState_UseItem] = hplNew(cPlayerState_UseItem, (mpInit, this));
	mvStates[ePlayerState_Message] = hplNew(cPlayerState_Message, (mpInit, this));
	mvStates[ePlayerState_Throw] = hplNew(cPlayerState_Throw, (mpInit, this));
	mvStates[ePlayerState_Climb] = hplNew(cPlayerState_Climb, (mpInit, this));

	// The max distance you can be from something to grab it.
	mfMaxGrabDist = mpInit->mpGameConfig->GetFloat("Player", "MaxGrabDist", 0);

	// The max distance you can be from something to move it.
	mfMaxMoveDist = mpInit->mpGameConfig->GetFloat("Player", "MaxMoveDist", 0);

	// The max distance you can be from something to push it.
	mfMaxPushDist = mpInit->mpGameConfig->GetFloat("Player", "MaxPushDist", 0);
	// The maximum speed you can push something with
	mfMaxPushSpeed = mpInit->mpGameConfig->GetFloat("Player", "MaxPushSpeed", 0);
	mvMaxPushHeadMovement = cVector2f(cMath::ToRad(5), cMath::ToRad(15));
	mvMinPushHeadMovement = cVector2f(cMath::ToRad(-5), cMath::ToRad(-10));

	// This is the maximum distance on which an item can be used.
	mfMaxUseItemDist = mpInit->mpGameConfig->GetFloat("Player", "MaxUseItemDist", 0);

	// Set the maximum time the jumpbutton can be held and make the jump longer.
	mfJumpCount = 0;
	mfMaxJumpCount = mpInit->mpGameConfig->GetFloat("Player", "MaxJumpCount", 1);

	// The border that decides when the mouse moves the screen in interact mode.
	mvInteractMoveBorder = cVector2f(130, 95);

	// Create head movement
	mpHeadMove = hplNew(cPlayerHeadMove, (this));

	// create damage effect
	mpDamage = hplNew(cPlayerDamage, (mpInit));

	// Create death effect
	mpDeath = hplNew(cPlayerDeath, (mpInit));

	// Create flashlight
	mpFlashLight = hplNew(cPlayerFlashLight, (mpInit));

	// Create Glowstick
	mpGlowStick = hplNew(cPlayerGlowStick, (mpInit));

	// Create Flare
	mpFlare = hplNew(cPlayerFlare, (mpInit));

	// Create leaner
	mpLean = hplNew(cPlayerLean, (mpInit, this));

	// Create ear ringer
	mpEarRing = hplNew(cPlayerEarRing, (mpInit, this));

	// Health
	mpHealth = hplNew(cPlayerHealth, (mpInit));

	// NOise Filter
	mpNoiseFilter = hplNew(cPlayerNoiseFilter, (mpInit));

	// Fear Filter
	mpFearFilter = hplNew(cPlayerFearFilter, (mpInit));

	// Look at
	mpLookAt = hplNew(cPlayerLookAt, (this));

	// Hidden
	mpHidden = hplNew(cPlayerHidden, (mpInit));

	// Create ray callbacks
	mpGroundRayCallback = hplNew(cPlayerGroundRayCallback, ());
	mpPickRayCallback = hplNew(cPlayerPickRayCallback, ());

	// Create body callback
	mpBodyCallback = hplNew(cPlayerBodyCallback, (this));

	// Load font
	mpFont = mpResources->GetFontManager()->CreateFontData("verdana.fnt");

	// Create and init move states
	// This must be called after head move is created!
	mMoveState = ePlayerMoveState_Walk;

	mvMoveStates.resize(ePlayerMoveState_LastEnum);
	mvMoveStates[ePlayerMoveState_Walk] = hplNew(cPlayerMoveState_Walk, (this, mpInit));
	mvMoveStates[ePlayerMoveState_Run] = hplNew(cPlayerMoveState_Run, (this, mpInit));
	mvMoveStates[ePlayerMoveState_Still] = hplNew(cPlayerMoveState_Still, (this, mpInit));
	mvMoveStates[ePlayerMoveState_Jump] = hplNew(cPlayerMoveState_Jump, (this, mpInit));
	mvMoveStates[ePlayerMoveState_Crouch] = hplNew(cPlayerMoveState_Crouch, (this, mpInit));

	/////////////////////////
	// Create player gui stuff

	// Cross hair
	mCrossHairState = eCrossHairState_None;
	mvCrossHairPos = cVector2f(400, 300);

	mvCrossHairs.resize(eCrossHairState_LastEnum);

	for (size_t i = 0; i < mvCrossHairs.size(); i++)
		mvCrossHairs[i] = NULL;

	mvCrossHairs[eCrossHairState_Inactive] = mpGfxDrawer->CreateGfxObject("player_crosshair_inactive", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Active] = mpGfxDrawer->CreateGfxObject("player_crosshair_active", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Invalid] = mpGfxDrawer->CreateGfxObject("player_crosshair_invalid", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Grab] = mpGfxDrawer->CreateGfxObject("player_crosshair_grab", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Examine] = mpGfxDrawer->CreateGfxObject("player_crosshair_examine", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Pointer] = mpGfxDrawer->CreateGfxObject("player_crosshair_pointer", "diffalpha2d");
	mvCrossHairs[eCrossHairState_DoorLink] = mpGfxDrawer->CreateGfxObject("player_crosshair_doorlink", "diffalpha2d");
	mvCrossHairs[eCrossHairState_PickUp] = mpGfxDrawer->CreateGfxObject("player_crosshair_pickup", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Ladder] = mpGfxDrawer->CreateGfxObject("player_crosshair_ladder", "diffalpha2d");
	mvCrossHairs[eCrossHairState_Cross] = mpGfxDrawer->CreateGfxObject("player_crosshair_cross", "diffalpha2d");

	// Set up variable values
	Reset();
}

//-----------------------------------------------------------------------

cPlayer::~cPlayer(void) {
	hplDelete(mpGroundRayCallback);
	hplDelete(mpPickRayCallback);
	hplDelete(mpHeadMove);
	hplDelete(mpBodyCallback);
	hplDelete(mpDamage);
	hplDelete(mpDeath);
	hplDelete(mpFlashLight);
	hplDelete(mpLean);
	hplDelete(mpEarRing);
	hplDelete(mpGlowStick);
	hplDelete(mpFlare);
	hplDelete(mpHealth);
	hplDelete(mpNoiseFilter);
	hplDelete(mpFearFilter);
	hplDelete(mpLookAt);
	hplDelete(mpHidden);

	/*mpInit->mpConfig->SetFloat("Game","PlayerWidth",mvSize.x);
	mpInit->mpConfig->SetFloat("Game","PlayerHeight",mvSize.y);
	mpInit->mpConfig->SetFloat("Game","PlayerCrouchHeight",mfCrouchHeight);*/

	mpInit->mpConfig->SetBool("Debug", "ShowHealth", mbShowHealth);
	mpInit->mpConfig->SetBool("Debug", "ShowSoundsPlaying", mbShowSoundsPlaying);

	STLDeleteAll(mvMoveStates);
	STLDeleteAll(mvStates);

	for (size_t i = 0; i < mvCrossHairs.size(); i++)
		if (mvCrossHairs[i])
			mpGfxDrawer->DestroyGfxObject(mvCrossHairs[i]);

	STLMapDeleteAll(m_mapCollideCallbacks);
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS - PROPERTIES
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPlayer::SetMass(float afX) {
	mfMass = afX;
	if (mpCharBody)
		mpCharBody->SetMass(afX);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS - ACTION
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPlayer::SetActive(bool abActive) {
	mbActive = abActive;

	if (mbActive == false) {
		ChangeState(ePlayerState_Normal);
	}
}

//-----------------------------------------------------------------------

void cPlayer::SetPower(float afX) {
	mfPower = afX;
}

void cPlayer::AddPower(float afX) {
	mfPower += afX;
	if (mfPower > 100)
		mfPower = 100;
	if (mfPower < 0)
		mfPower = 0;
}

//-----------------------------------------------------------------------

void cPlayer::SetPrevMoveState(ePlayerMoveState aState) {
	mvMoveStates[mMoveState]->mPrevMoveState = aState;
}

//-----------------------------------------------------------------------

void cPlayer::SetStartPos(const tString &asName) {
	ChangeState(ePlayerState_Normal);

	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	if (pWorld) {
		cStartPosEntity *pStart = pWorld->GetStartPosEntity(asName);
		if (pStart == NULL) {
			Warning("Couldn't find start position '%s'\n", asName.c_str());

			pStart = pWorld->GetFirstStartPosEntity();
		}

		cVector3f vPosition(0, 0, 0), vCamRotation(0, 0, 0), vBodyRotation(0, 0, 0);

		if (pStart) {
			vPosition = pStart->GetWorldMatrix().GetTranslation();

			cMatrixf mtxInv = cMath::MatrixInverse(pStart->GetWorldMatrix());
			vCamRotation = cMath::GetAngleFromPoints3D(cVector3f(0, 0, 0), mtxInv.GetForward() * -1);
			vBodyRotation = cMath::GetAngleFromPoints3D(cVector3f(0, 0, 0), mtxInv.GetForward());
		}

		// Set position of the player, this should include body and stuff:
		// mpCamera->SetPosition(vPosition + cVector3f(0,mvSize.y/2.0f,0));
		mpCharBody->SetPosition(vPosition + cVector3f(0, mpCharBody->GetSize().y / 2.0f, 0));
		// mpCharBody->SetPosition(cVector3f(0,0.43f,0));
		mpCamera->SetYaw(vCamRotation.y);
		mpCamera->SetPitch(vCamRotation.x);
	}
}

//-----------------------------------------------------------------------

void cPlayer::ChangeMoveState(ePlayerMoveState aState, bool abSetHeadHeightDirectly) {
	if (mMoveState == aState)
		return;

	// Log("Change movestate from %d to: %d\n",(int)mMoveState,(int)aState);

	ePlayerMoveState PrevState = mMoveState;
	mMoveState = aState;

	mvMoveStates[aState]->InitState(mvMoveStates[PrevState]);

	if (abSetHeadHeightDirectly) {
		SetHeightAdd(mvMoveStates[aState]->mfHeightAdd);
	}
}

//-----------------------------------------------------------------------

void cPlayer::FootStep(float afMul, const tString &asType, bool abSkipCount) {
	if (mlGroundCount <= 0 && abSkipCount == false)
		return;

	iPhysicsMaterial *pMaterial = mpGroundRayCallback->mpMaterial;
	if (pMaterial == NULL)
		return;

	cSurfaceData *pData = pMaterial->GetSurfaceData();
	if (pData == NULL) {
		Error("No surface data in material '%s'!\n", pMaterial->GetName().c_str());
		return;
	}

	tString sMatStepType = pMaterial->GetSurfaceData()->GetStepType();
	if (sMatStepType == "")
		return;

	const tString &sType = asType != "" ? asType : mvMoveStates[mMoveState]->msStepType;

	tString sSoundName = "player_step_" + sType + "_" + sMatStepType;

	cResources *pResources = gpInit->mpGame->GetResources();

	cSoundEntityData *pSoundData = pResources->GetSoundEntityManager()->CreateSoundEntity(sSoundName);
	if (pSoundData) {
		cSoundHandler *pSoundHandler = gpInit->mpGame->GetSound()->GetSoundHandler();

		pSoundHandler->PlayGui(pSoundData->GetMainSoundName(), false,
							   afMul * pSoundData->GetVolume());

		cGameTrigger_Sound *pSound = hplNew(cGameTrigger_Sound, ());
		pSound->mpSound = pSoundData;
		mpInit->mpTriggerHandler->Add(pSound, eGameTriggerType_Sound,
									  mpCharBody->GetFeetPosition() + cVector3f(0, 0.2f, 0),
									  10, 1.0f / 60.0f, pSoundData->GetMaxDistance());
	}

	/*cWorld3D *pWorld = mpScene->GetWorld3D();

	cSoundEntity *pSound = pWorld->CreateSoundEntity("Step",sSoundName,true);
	if(pSound)
	{
		pSound->SetVolume(afMul * pSound->GetVolume());
		pSound->SetPosition(cVector3f(0,0.2f,0.4f));

		mFeetNode.AddEntity(pSound);
		mFeetNode.SetPosition(mFeetNode.GetLocalPosition());
	}*/
}

//-----------------------------------------------------------------------

void cPlayer::ChangeState(ePlayerState aState) {
	if (aState == mState)
		return;

	// Log("State %d --> %d\n",(int)mState, (int)aState);

	mvStates[aState]->InitState(mvStates[mState]);

	mState = aState;
}

//-----------------------------------------------------------------------

bool cPlayer::AddCrossHairPos(const cVector2f &avPos) {
	bool abEdge = false;

	mvCrossHairPos += avPos;
	if (mvCrossHairPos.x < mvInteractMoveBorder.x) {
		mvCrossHairPos.x = mvInteractMoveBorder.x;
		abEdge = true;
	}
	if (mvCrossHairPos.y < mvInteractMoveBorder.y) {
		mvCrossHairPos.y = mvInteractMoveBorder.y;
		abEdge = true;
	}
	if (mvCrossHairPos.x > (799 - mvInteractMoveBorder.x)) {
		mvCrossHairPos.x = (799 - mvInteractMoveBorder.x);
		abEdge = true;
	}
	if (mvCrossHairPos.y > (599 - mvInteractMoveBorder.y)) {
		mvCrossHairPos.y = (599 - mvInteractMoveBorder.y);
		abEdge = true;
	}

	return abEdge;
}

//-----------------------------------------------------------------------

void cPlayer::AddCollideScript(eGameCollideScriptType aType, const tString &asFunc, const tString &asEntity) {
	cGameCollideScript *pCallback;

	// Check if the function already exist
	tGameCollideScriptMapIt it = m_mapCollideCallbacks.find(asEntity);
	if (it != m_mapCollideCallbacks.end()) {
		pCallback = it->second;
	} else {
		pCallback = hplNew(cGameCollideScript, ());

		// Get the entity
		iGameEntity *pEntity = mpInit->mpMapHandler->GetGameEntity(asEntity);
		if (pEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", asEntity.c_str());
			hplDelete(pCallback);
			return;
		}

		// Set the entity
		pCallback->mpEntity = pEntity;

		// Add to container
		m_mapCollideCallbacks.insert(tGameCollideScriptMap::value_type(asEntity, pCallback));
	}

	pCallback->msFuncName[aType] = asFunc;
}

//-----------------------------------------------------------------------

void cPlayer::RemoveCollideScriptWithChildEntity(iGameEntity *apEntity) {
	tGameCollideScriptMapIt it = m_mapCollideCallbacks.begin();
	for (; it != m_mapCollideCallbacks.end();) {
		cGameCollideScript *pCallback = it->second;
		tGameCollideScriptMapIt currentIt = it;
		++it;

		if (pCallback && pCallback->mpEntity == apEntity) {
			if (mbUpdatingCollisionCallbacks) {
				pCallback->mbDeleteMe = true;
			} else {
				hplDelete(pCallback);
				m_mapCollideCallbacks.erase(currentIt);
			}
		}
	}
}

//-----------------------------------------------------------------------

void cPlayer::RemoveCollideScript(eGameCollideScriptType aType, const tString &asEntity) {
	tGameCollideScriptMapIt it = m_mapCollideCallbacks.find(asEntity);
	if (it != m_mapCollideCallbacks.end()) {
		cGameCollideScript *pCallback = it->second;

		pCallback->msFuncName[aType] = "";
		// if there are no functions left, erase
		if (pCallback->msFuncName[0] == "" && pCallback->msFuncName[1] == "" && pCallback->msFuncName[2] == "") {
			if (mbUpdatingCollisionCallbacks) {
				pCallback->mbDeleteMe = true;
			} else {
				hplDelete(pCallback);
				m_mapCollideCallbacks.erase(it);
			}
		}

	} else {
		Warning("Entity '%s' callback doesn't exist in 'Player'\n", asEntity.c_str());
	}
}

//-----------------------------------------------------------------------

void cPlayer::ClearCollideScripts() {
	STLMapDeleteAll(m_mapCollideCallbacks);
}

//-----------------------------------------------------------------------

void cPlayer::SetSpeedMul(float afSpeedMul) {
	mfSpeedMul = afSpeedMul;

	mvMoveStates[mMoveState]->SetupBody();
}

//-----------------------------------------------------------------------

void cPlayer::SetHealthSpeedMul(float afHealthSpeedMul) {
	mfHealthSpeedMul = afHealthSpeedMul;

	mvMoveStates[mMoveState]->SetupBody();
}

//-----------------------------------------------------------------------

void cPlayer::SetHealth(float afX) {
	mfHealth = afX;

	if (mfHealth > 100) {
		mfHealth = 100;
	} else if (mfHealth <= 0) {
		mpDeath->Start();
	}
}

void cPlayer::AddHealth(float afX) {
	SetHealth(mfHealth + afX);
}

void cPlayer::Damage(float afDamage, ePlayerDamageType aType) {
	if (afDamage <= 0)
		return;
	if (mpInit->mpMapHandler->IsChangingMap())
		return;
	if (mfHealth <= 0)
		return;

	if (mpInit->mDifficulty == eGameDifficulty_Easy)
		afDamage /= 2.0f;
	if (mpInit->mDifficulty == eGameDifficulty_Hard)
		afDamage *= 2.0f;

	if (mpDeath->IsActive())
		return;

	float fSize = 0.5f;
	if (afDamage > 10)
		fSize = 1.5f;
	if (afDamage > 20)
		fSize = 2.0f;
	if (afDamage > 50)
		fSize = 3.0f;
	if (afDamage > 80)
		fSize = 4.0f;
	mpDamage->Start(fSize, aType);

	AddHealth(-afDamage);
}

//-----------------------------------------------------------------------

bool cPlayer::IsDead() {
	return mpDeath->IsActive();
}

//-----------------------------------------------------------------------

iPhysicsBody *cPlayer::GetPickedBody() {
	return mpPickRayCallback->mpPickedBody;
}
void cPlayer::SetPickedBody(iPhysicsBody *apBody) {
	mpPickRayCallback->mpPickedBody = apBody;
}

//-----------------------------------------------------------------------

float cPlayer::GetPickedDist() {
	// return mpPickRayCallback->mfPickedDist;
	return cMath::Vector3Dist(mpCharBody->GetPosition(), mpPickRayCallback->mvPickedPos);
}

const cVector3f &cPlayer::GetPickedPos() {
	return mpPickRayCallback->mvPickedPos;
}
cPlayerPickRayCallback *cPlayer::GetPickRay() {
	return mpPickRayCallback;
}

//-----------------------------------------------------------------------

void cPlayer::DestroyWorldObjects() {
	// Body
	if (mpCharBody)
		mpScene->GetWorld3D()->GetPhysicsWorld()->DestroyCharacterBody(mpCharBody);

	// mpFlashLight->Destroy();
	// mpGlowStick->Destroy();
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS - INTERACTIONS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPlayer::MoveForwards(float afMul, float afTimeStep) {
	if (mvStates[mState]->OnMoveForwards(afMul, afTimeStep)) {
		// Only move if on ground
		if (mlGroundCount <= 0 || afMul == 0)
			return;

		mpCharBody->Move(eCharDir_Forward, afMul, afTimeStep);
		mbMoving = true;
		mvMoveStates[mMoveState]->Start();
	}
}

//-----------------------------------------------------------------------

void cPlayer::MoveSideways(float afMul, float afTimeStep) {
	if (mvStates[mState]->OnMoveSideways(afMul, afTimeStep)) {
		// Only move if on ground
		if (mlGroundCount <= 0 || afMul == 0)
			return;

		mpCharBody->Move(eCharDir_Right, afMul, afTimeStep);
		mbMoving = true;
		mvMoveStates[mMoveState]->Start();
	}
}

//-----------------------------------------------------------------------

void cPlayer::Lean(float afMul, float afTimeStep) {
	mpLean->Lean(afMul, afTimeStep);
}

//-----------------------------------------------------------------------

void cPlayer::AddYaw(float afVal) {
	if (mvStates[mState]->OnAddYaw(afVal)) {
		mpCamera->AddYaw(-afVal * mfLookSpeed);
		mpCharBody->SetYaw(mpCamera->GetYaw());
	}
}

//-----------------------------------------------------------------------

void cPlayer::AddPitch(float afVal) {
	if (mvStates[mState]->OnAddPitch(afVal)) {
		float fInvert = mpInit->mpButtonHandler->GetInvertMouseY() ? -1.0f : 1.0f;
		mpCamera->AddPitch(-afVal * mfLookSpeed * fInvert);
	}
}

//-----------------------------------------------------------------------

void cPlayer::StartInteract() {
	mvStates[mState]->OnStartInteract();
}

void cPlayer::StopInteract() {
	mvStates[mState]->OnStopInteract();
}

//-----------------------------------------------------------------------

void cPlayer::StartExamine() {
	mvStates[mState]->OnStartExamine();
}

void cPlayer::StopExamine() {
	mvStates[mState]->OnStopExamine();
}

//-----------------------------------------------------------------------

void cPlayer::StartHolster() {
	mvStates[mState]->OnStartHolster();
}

//-----------------------------------------------------------------------

void cPlayer::Jump() {
	if (mvStates[mState]->OnJump() && mlGroundCount > 0) {
		if (mvMoveStates[mMoveState]->mType != ePlayerMoveState_Jump) {
			ChangeMoveState(ePlayerMoveState_Jump);
		}
	}
	mfJumpCount = 0;
}

void cPlayer::SetJumpButtonDown(bool abX) {
	mbJumpButtonDown = abX;

	if (mbJumpButtonDown) {
		mfJumpCount += mpInit->mpGame->GetStepSize();
	} else {
		mfJumpCount = mfMaxJumpCount;
	}
}

//-----------------------------------------------------------------------

void cPlayer::StartRun() {
	mvStates[mState]->OnStartRun();
}

//-----------------------------------------------------------------------

void cPlayer::StopRun() {
	mvStates[mState]->OnStopRun();
}

//-----------------------------------------------------------------------

void cPlayer::StartCrouch() {
	mvStates[mState]->OnStartCrouch();
}

//-----------------------------------------------------------------------

void cPlayer::StopCrouch() {
	mvStates[mState]->OnStopCrouch();
}

//-----------------------------------------------------------------------

void cPlayer::StartInteractMode() {
	mvStates[mState]->OnStartInteractMode();
}

//-----------------------------------------------------------------------

void cPlayer::StartInventory() {
	if (mvStates[mState]->OnStartInventory()) {
		mpInit->mpInventory->SetActive(true);
	}
}

//-----------------------------------------------------------------------

void cPlayer::StartInventoryShortCut(int alNum) {
	if (mvStates[mState]->OnStartInventoryShortCut(alNum)) {
		mpInit->mpInventory->OnShortcutDown(alNum);
	}
}

//-----------------------------------------------------------------------

void cPlayer::StartFlashLightButton() {
	if (mpInit->mpInventory->GetItem("flashlight") != NULL) {
		mpFlashLight->SetActive(!mpFlashLight->IsActive());

		if (mpFlashLight->IsActive()) {
			mpGlowStick->SetActive(false);
			mpFlare->SetActive(false);
		}
	}
}

void cPlayer::StartGlowStickButton() {
	if (mpInit->mpInventory->GetItem("glowstick") != NULL ||
		mpInit->mpInventory->GetItem("glowst1") != NULL) {
		mpGlowStick->SetActive(!mpGlowStick->IsActive());

		if (mpGlowStick->IsActive()) {
			mpFlashLight->SetActive(false);
			mpFlare->SetActive(false);
		}
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS - EVENTS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cPlayer::OnWorldLoad() {
	/////////////////////////////////////////////////////////
	// Create body
	mpCharBody = mpScene->GetWorld3D()->GetPhysicsWorld()->CreateCharacterBody("Player", mvSize);

	mpCharBody->SetCamera(mpCamera);
	mpCharBody->SetMass(mfMass);
	// mpCamera->SetPosition(cVector3f(1,1.2f,-2));

	mpCharBody->SetCameraPosAdd(cVector3f(0, mfCameraHeightAdd, 0));
	mpCharBody->SetCameraSmoothPosNum(6);
	mpCharBody->SetCallback(mpBodyCallback);

	mpCharBody->SetMaxGravitySpeed(40.0f);
	mpCharBody->SetCustomGravityActive(true);
	mpCharBody->SetCustomGravity(cVector3f(0, -18.0f, 0));

	mpCharBody->SetMaxPushMass(mpInit->mpGameConfig->GetFloat("Player", "MaxPushMass", 1));
	mpCharBody->SetPushForce(mpInit->mpGameConfig->GetFloat("Player", "PushForce", 1));

	mpCharBody->SetMaxStepSize(mpInit->mpGameConfig->GetFloat("Player", "MaxStepSize", 0));
	mpCharBody->SetStepClimbSpeed(mpInit->mpGameConfig->GetFloat("Player", "StepClimbSpeed", 0));
	mpCharBody->SetClimbForwardMul(1.75f);
	mpCharBody->SetClimbHeightAdd(0.02f);
	mpCharBody->SetAccurateClimbing(true);

	mpCharBody->SetGroundFriction(mpInit->mpGameConfig->GetFloat("Player", "GroundFriction", 0));
	mpCharBody->SetAirFriction(mpInit->mpGameConfig->GetFloat("Player", "AirFriction", 0));

	// Add the crouch size
	mpCharBody->AddExtraSize(cVector3f(mvSize.x, mfCrouchHeight, mvSize.z));

	// Set so it is not saved:
	mpCharBody->SetIsSaved(false);
	mpCharBody->GetExtraBody(0)->SetIsSaved(false);
	mpCharBody->GetExtraBody(1)->SetIsSaved(false);

	mvMoveStates[mMoveState]->EnterState(NULL);
	mvMoveStates[mMoveState]->Start();

	mpFlashLight->OnWorldLoad();
	// mpGlowStick->OnWorldLoad();
	mpFlare->OnWorldLoad();
	mpHidden->OnWorldLoad();
	mpLean->OnWorldLoad();
}

//-----------------------------------------------------------------------

void cPlayer::OnWorldExit() {
	DestroyWorldObjects();

	mpGroundRayCallback->OnWorldExit();
	mpPickRayCallback->OnWorldExit();
	mpHidden->OnWorldExit();
}

//-----------------------------------------------------------------------

void cPlayer::OnStart() {
}

//-----------------------------------------------------------------------

static inline tString GetCollideCommand(const tString &asFuncName, const tString &asParent,
										const tString &asChild) {
	return asFuncName + "(\"" + asParent + "\", \"" + asChild + "\")";
}

class cTempCheckProxy : public iPhysicsRayCallback {
public:
	bool mbCollided;
	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams) {
		// if(apParams->mfT <0) return true;
		if (pBody->GetCollide() == false || pBody->IsCharacter())
			return true;

		mbCollided = true;
		return false;
	}
};

void cPlayer::Update(float afTimeStep) {
	// cSystem *pSystem = mpInit->mpGame->GetSystem();
	// unsigned int lTime = pSystem->GetLowLevel()->getTime();
	iPhysicsWorld *pPhysicsWorld = mpScene->GetWorld3D()->GetPhysicsWorld();

	// LogUpdate("  Death\n");
	////////////////////////////////////////
	// Make sure player is dead if he should be
	if (mfHealth <= 0 && mpDeath->IsActive() == false) {
		mpDeath->Start();
	}

	////////////////////////////////////////
	// Update Node and Footstep sounds
	/*//LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	lTime = pSystem->GetLowLevel()->GetTime();
	//LogUpdate("  Footstep sounds\n");
	cMatrixf mtxChar = mpInit->mpGame->GetSound()->GetLowLevel()->GetListenerMatrix();
	mtxChar.SetTranslation(mtxChar.GetTranslation() -
							cVector3f(0,mpCharBody->GetSize().y/2,0));
	mFeetNode.SetMatrix(mtxChar);
	cSoundHandler *pSoundHandler = mpInit->mpGame->GetSound()->GetSoundHandler();
	cEntityIterator entIt = mFeetNode.GetEntityInterator();
	while(entIt.HasNext())
	{
		cSoundEntity *pSound = static_cast<cSoundEntity*>(entIt.Next());
		cSoundEntry *pEntry = pSound->GetSoundEntry(eSoundEntityType_Main);
		if(	pEntry && pSoundHandler->IsValid(pEntry->mpSound))
		{
			pEntry->mpSound->SetPosition(pSound->GetWorldPosition());
		}
	}
	//LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);*/

	// lTime = pSystem->GetLowLevel()->getTime();
	//  LogUpdate("  misc\n");
	//////////////////////
	// Reset roll
	mpInit->mpPlayer->GetCamera()->SetRoll(0);

	/////////////////////////////////////////////////
	// Misc
	mItemFlash.Update(afTimeStep);

	/////////////////////////////////////////////////
	// Damage
	mpDamage->Update(afTimeStep);

	/////////////////////////////////////////////////
	// Death
	mpDeath->Update(afTimeStep);

	/////////////////////////////////////////////////
	// Flashlight
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  flashlight");
	mpFlashLight->Update(afTimeStep);

	/////////////////////////////////////////////////
	// Glowstick
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  glowstick\n");
	mpGlowStick->Update(afTimeStep);

	/////////////////////////////////////////////////
	// Flare
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  flare\n");
	mpFlare->Update(afTimeStep);

	/////////////////////////////////////////////////
	// Lean
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  more misc\n");
	mpLean->Update(afTimeStep);

	/////////////////////////////////////////////////
	// Ear ring
	mpEarRing->Update(afTimeStep);

	//////////////////////////////////////////////////
	// health
	mpHealth->Update(afTimeStep);

	////////////////////////////////////////
	// Noise filter
	mpNoiseFilter->Update(afTimeStep);

	////////////////////////////////////////
	// Fear filter
	mpFearFilter->Update(afTimeStep);

	////////////////////////////////////////
	// Look at
	mpLookAt->Update(afTimeStep);

	////////////////////////////////////////
	// Hidden
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  hidden\n");
	mpHidden->Update(afTimeStep);

	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  collide scripts\n");
	/////////////////////////////////////////////////
	// Collide script
	pPhysicsWorld = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld();
	/*cWorld3D *pWorld = */ mpInit->mpGame->GetScene()->GetWorld3D();
	cCollideData collideData;
	collideData.SetMaxSize(1);

	mbUpdatingCollisionCallbacks = true;
	tGameCollideScriptMapIt CollideIt = m_mapCollideCallbacks.begin();
	for (; CollideIt != m_mapCollideCallbacks.end(); ++CollideIt) {
		cGameCollideScript *pCallback = CollideIt->second;

		if (pCallback == NULL)
			continue;
		if (pCallback->mpEntity == NULL)
			continue;

		iGameEntity *pEntity = pCallback->mpEntity;

		if (pEntity->IsActive() == false)
			continue;

		// LogUpdate("  callback %s %s %s\n",pCallback->msFuncName[0].c_str(),pCallback->msFuncName[1].c_str(),pCallback->msFuncName[2].c_str());

		bool bCollide = false;

		for (size_t j = 0; j < pEntity->mvBodies.size(); ++j) {
			iPhysicsBody *pParentBody = mpCharBody->GetBody();
			iPhysicsBody *pChildBody = pEntity->mvBodies[j];

			if (cMath::CheckCollisionBV(*pParentBody->GetBV(), *pChildBody->GetBV())) {
				bCollide = pPhysicsWorld->CheckShapeCollision(pParentBody->GetShape(),
															  pParentBody->GetLocalMatrix(),
															  pChildBody->GetShape(),
															  pChildBody->GetLocalMatrix(), collideData, 1);
			}

			if (bCollide)
				break;
		}

		// Run Collide scripts
		// LogUpdate("  running script");
		if (bCollide) {
			if (pCallback->mbCollides) {
				if (pCallback->msFuncName[eGameCollideScriptType_During] != "") {
					tString sCommand = GetCollideCommand(
						pCallback->msFuncName[eGameCollideScriptType_During],
						"Player", CollideIt->first);
					mpInit->RunScriptCommand(sCommand);
				}
			} else {
				if (pCallback->msFuncName[eGameCollideScriptType_Enter] != "") {
					tString sCommand = GetCollideCommand(
						pCallback->msFuncName[eGameCollideScriptType_Enter],
						"Player", CollideIt->first);
					mpInit->RunScriptCommand(sCommand);
				}

				pCallback->mbCollides = true;
			}
		} else {
			if (pCallback->mbCollides) {
				if (pCallback->msFuncName[eGameCollideScriptType_Leave] != "") {
					tString sCommand = GetCollideCommand(
						pCallback->msFuncName[eGameCollideScriptType_Leave],
						"Player", CollideIt->first);
					mpInit->RunScriptCommand(sCommand);
				}

				pCallback->mbCollides = false;
			}
		}
	}
	mbUpdatingCollisionCallbacks = false;

	//////////////////////////////////////////////////
	// Check if any collide script should be deleted.
	// LogUpdate("  Check collide script deleting\n");

	CollideIt = m_mapCollideCallbacks.begin();
	for (; CollideIt != m_mapCollideCallbacks.end();) {
		cGameCollideScript *pCallback = CollideIt->second;
		if (pCallback->mbDeleteMe) {
			hplDelete(pCallback);
			m_mapCollideCallbacks.erase(CollideIt++);
		} else {
			++CollideIt;
		}
	}

	/////////////////////////////////////////////////
	// Update ground count, this is so that a little air born time still counts as on ground
	if (mpCharBody->IsOnGround())
		mlGroundCount = 25;
	else if (mlGroundCount > 0)
		mlGroundCount--;

	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  Check For ground\n");
	//////////////////////////////
	// Cast ray and check for ground.
	cVector3f vStart, vEnd;

	iCollideShape *pBodyShape = mpCharBody->GetShape();
	vStart = mpCharBody->GetPosition() - cVector3f(0, pBodyShape->GetSize().y / 2 - 0.3f, 0);
	vEnd = vStart + cVector3f(0, -0.6f, 0);

	mpGroundRayCallback->Clear();
	pPhysicsWorld->CastRay(mpGroundRayCallback, vStart, vEnd, true, false, false);

	//////////////////////////////
	// Update movement
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  Movement\n");

	if (mbMoving == false)
		mvMoveStates[mMoveState]->Stop();

	mvMoveStates[mMoveState]->Update(afTimeStep);
	mpHeadMove->Update(afTimeStep);

	mbMoving = false;

	//////////////////////////////
	// Update camera pos add
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  Camera pos\n");
	if (mpCharBody) {
		float fYAdd = mfCameraHeightAdd + mpHeadMove->GetPos() + mfHeightAdd + mpDeath->GetHeighAdd() +
					  mpInit->mpEffectHandler->GetShakeScreen()->GetScreenAdd().y;

		cVector3f vRight = mpCharBody->GetRight();
		float fXAdd = mpLean->mfMovement * vRight.x +
					  mpInit->mpEffectHandler->GetShakeScreen()->GetScreenAdd().x;

		float fZAdd = mpLean->mfMovement * vRight.z +
					  mpInit->mpEffectHandler->GetShakeScreen()->GetScreenAdd().x;

		// Log("HEadMove: %f HeightAdd %f Death: %f\n",mpHeadMove->GetPos(),mfHeightAdd,mpDeath->GetHeighAdd());

		mpCharBody->SetCameraPosAdd(cVector3f(fXAdd, fYAdd, fZAdd));
	}

	///////////////////////////
	// Update state

	// Clear picked body
	// mpPushBody = NULL;
	SetPickedBody(NULL);

	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
	// lTime = pSystem->GetLowLevel()->getTime();
	// LogUpdate("  state %d\n",mState);
	if (mpInit->mpInventory->IsActive() == false &&
		mpInit->mpNotebook->IsActive() == false) {
		mvStates[mState]->OnUpdate(afTimeStep);
	}
	// LogUpdate("  took %d ms\n",pSystem->GetLowLevel()->GetTime() - lTime);
}

//-----------------------------------------------------------------------

void cPlayer::Reset() {
	// Camera
	mpCamera->SetPitchLimits(cVector2f(cMath::ToRad(70), cMath::ToRad(-70)));
	mpCamera->SetYawLimits(0);
	mpCamera->SetFOV(cMath::ToRad(70));

	cVector2f vScreenSize = mpInit->mpGame->GetGraphics()->GetLowLevel()->GetScreenSize();
	mpCamera->SetAspect(vScreenSize.x / vScreenSize.y);

	// Properties
	mbItemFlash = false;
	mfHealth = 100;
	mbMoving = false;
	mfMass = mfDefaultMass;
	mbLandedFromJump = false;
	mfSpeedMul = 1.0f;
	mfHealthSpeedMul = 1.0f;
	mbActive = true;
	mfPower = 0;

	mfHeightAdd = 0;

	mfLookSpeed = 1.0f;
	mpCharBody = NULL;
	mpWeaponCallback = NULL;
	mbUpdatingCollisionCallbacks = false;

	mbDamageFromPos = false;

	// jump properties
	mbJumpButtonDown = false;
	mfJumpCount = 0;

	// States
	mMoveState = ePlayerMoveState_Walk;
	mState = ePlayerState_Normal;

	// Crosshair
	mCrossHairState = eCrossHairState_None;
	mvCrossHairPos = cVector2f(400, 300);

	// Stats
	mlStat_NumOfSaves = 0;

	// Callbacks
	STLMapDeleteAll(m_mapCollideCallbacks);

	// Helpers
	mpDeath->Reset();
	mpFlashLight->Reset();
	mpEarRing->Stop(true);
	mpGlowStick->Reset();
	mpFlare->Reset();
	mpLookAt->Reset();
	mpFearFilter->Reset();
	mpLean->Reset();
	mpEarRing->Reset();
	mpHealth->Reset();
	mpHidden->Reset();
	mpGroundRayCallback->Reset();
}

//-----------------------------------------------------------------------

void cPlayer::OnDraw() {
	/////////////////////////////////
	// Damage
	mpDamage->Draw();

	/////////////////////////////////
	// Death
	mpDeath->Draw();

	////////////////////////////////////////
	// Noise filter
	mpNoiseFilter->Draw();

	////////////////////////////////////////
	// Fear filter
	mpFearFilter->Draw();

	////////////////////////////////////////
	// Hidden
	mpHidden->Draw();

	mpHealth->Draw();

	////////////////////////////////
	// Cross hair
	if (IsActive() == false) {
		// Do noting...
	} else if (mCrossHairState == eCrossHairState_Item) {
		cGfxObject *pObject = mpCurrentItem->GetGfxObject();
		cGfxObject *pAdditive = mpCurrentItem->GetGfxObjectAdditive();
		if (pObject) {
			cVector2l vIntSize = pObject->GetMaterial()->GetImage(eMaterialTexture_Diffuse)->GetSize();
			cVector2f vSize((float)vIntSize.x, (float)vIntSize.y);

			cVector2f vPosAdd(((float)vSize.x) / 2.0f, ((float)vSize.y) / 2.0f);

			if (mbItemFlash) {
				mpGfxDrawer->DrawGfxObject(pObject, cVector3f(0, 0, 100) + (mvCrossHairPos - vPosAdd), vSize,
										   cColor(1, 1, 1, 1));
				for (int i = 0; i < 2; ++i)
					mpGfxDrawer->DrawGfxObject(pAdditive, cVector3f(0, 0, 101) + (mvCrossHairPos - vPosAdd), vSize,
											   cColor(1, 1, 1, mItemFlash.val));

				/*mpGfxDrawer->DrawGfxObject(pAdditive,cVector3f(3,3,99)+(mvCrossHairPos - vPosAdd),vSize,
											cColor(0,1,0,mItemFlash.val*0.8f));
				mpGfxDrawer->DrawGfxObject(pAdditive,cVector3f(-3,-3,99)+(mvCrossHairPos - vPosAdd),vSize,
											cColor(0,1,0,mItemFlash.val*0.8f));
				mpGfxDrawer->DrawGfxObject(pAdditive,cVector3f(-3,3,99)+(mvCrossHairPos - vPosAdd),vSize,
											cColor(0,1,0,mItemFlash.val*0.8f));
				mpGfxDrawer->DrawGfxObject(pAdditive,cVector3f(3,-3,99)+(mvCrossHairPos - vPosAdd),vSize,
											cColor(0,1,0,mItemFlash.val*0.8f));*/
			} else {
				mpGfxDrawer->DrawGfxObject(pObject, cVector3f(0, 0, 100) + (mvCrossHairPos - vPosAdd), vSize,
										   cColor(1, 0.3f, 0.3f, 1.0f));
			}
		}
	} else if (mCrossHairState != eCrossHairState_None) {
		cResourceImage *pImage = mvCrossHairs[mCrossHairState]->GetMaterial()->GetImage(eMaterialTexture_Diffuse);
		cVector2l vSize = pImage->GetSize();
		cVector2f vPosAdd(((float)vSize.x) / 2.0f, ((float)vSize.y) / 2.0f);
		mpGfxDrawer->DrawGfxObject(mvCrossHairs[mCrossHairState], cVector3f(0, 0, 100) + (mvCrossHairPos - vPosAdd));
	} else if (mpInit->mbShowCrossHair) {
		cVector3f vPos = cVector3f(400, 300, 0);
		cResourceImage *pImage = mvCrossHairs[eCrossHairState_Cross]->GetMaterial()->GetImage(eMaterialTexture_Diffuse);
		cVector2l vSize = pImage->GetSize();
		cVector2f vPosAdd(((float)vSize.x) / 2.0f, ((float)vSize.y) / 2.0f);
		mpGfxDrawer->DrawGfxObject(mvCrossHairs[eCrossHairState_Cross], cVector3f(0, 0, 100) + (vPos - vPosAdd));
	}

	// DEBUG: Memory
	/*float fMbMem = ((float)cMemoryManager::mlTotalMemoryUsage) / (1024.0f * 1024.0f);
	mpFont->Draw(cVector3f(5,5,0),12,cColor(1,1,1,1),eFontAlign_Left,_W("Memory used: %d (%.2f mb)"),
											cMemoryManager::mlTotalMemoryUsage, fMbMem);
	*/

	// DEBUG: Mouse
	// iMouse *pMouse = mpInit->mpGame->GetInput()->GetMouse();
	// mpFont->Draw(cVector3f(5,5,0),12,cColor(1,1,1,1),eFontAlign_Left,_W("Left: %d Right: %d"),
	//													pMouse->ButtonIsDown(eMButton_Left),
	//													pMouse->ButtonIsDown(eMButton_Right));

	// DEBUG: MoveState
	/*tString sState ="";
	if(mMoveState == ePlayerMoveState_Jump) sState = "Jump";
	else if(mMoveState == ePlayerMoveState_Crouch) sState = "Crouch";
	else if(mMoveState == ePlayerMoveState_Walk) sState = "Walk";
	else if(mMoveState == ePlayerMoveState_Run) sState = "Run";
	else if(mMoveState == ePlayerMoveState_Still) sState = "Still";

	mpFont->Draw(cVector3f(5,5,0),12,cColor(1,1,1,1),eFontAlign_Left,"MoveState: %s",
					sState.c_str());*/

	// DEBUG: Picked body
	/*if(mpInit->mbHasHaptics)
	{
		if(mpPickRayCallback->mpPickedBody){
			mpFont->Draw(cVector3f(5,35,0),12,cColor(1,1,1,1),eFontAlign_Left,_W("Body: %s"),
				cString::To16Char(mpPickRayCallback->mpPickedBody->GetName()).c_str());
		}
		else{
			mpFont->Draw(cVector3f(5,35,0),12,cColor(1,1,1,1),eFontAlign_Left,_W("Body: NULL"));
		}
		mpFont->Draw(cVector3f(5,46,0),12,cColor(1,1,1,1),eFontAlign_Left,_W("Dist: %f"),
			mpPickRayCallback->mfPickedDist);

		tWString sCState = _W("Unknown");
		if(mCrossHairState == eCrossHairState_Inactive)sCState = _W("Inactive");
		if(mCrossHairState == eCrossHairState_Active)sCState = _W("Active");
		if(mCrossHairState == eCrossHairState_Invalid)sCState = _W("Invalid");
		if(mCrossHairState == eCrossHairState_Grab)sCState = _W("Grab");
		if(mCrossHairState == eCrossHairState_Examine)sCState = _W("Examine");
		if(mCrossHairState == eCrossHairState_Pointer)sCState = _W("Pointer");
		if(mCrossHairState == eCrossHairState_Item)sCState = _W("Item");
		if(mCrossHairState == eCrossHairState_DoorLink)sCState = _W("DoorLink");
		if(mCrossHairState == eCrossHairState_PickUp)sCState = _W("PickUp");
		if(mCrossHairState == eCrossHairState_Ladder)sCState = _W("Ladder");
		if(mCrossHairState == eCrossHairState_None)sCState = _W("None");
		mpFont->Draw(	cVector3f(5,66,0),12,cColor(1,1,1,1),eFontAlign_Left,
						_W("CState: %s"),sCState.c_str());

	}*/

	// DEBUG: On ground and step material
	/*mpFont->Draw(cVector3f(5,17,0),12,cColor(1,1,1,1),eFontAlign_Left,"Position: %f ClimbPos: %f ClimbCount: %f\n",
											mpHeadMove->GetPosition(),
											mpHeadMove->GetClimbPosition(),
											mpHeadMove->GetClimbCount());*/
	// mpFont->Draw(cVector3f(5,29,0),12,cColor(1,1,1,1),eFontAlign_Left,"Gravity: %f",
	// mpCharBody->GetForceVelocity().y);
	// mpFont->Draw(cVector3f(5,29,0),12,cColor(1,1,1,1),eFontAlign_Left,"CameraPos: %s",
	//	mpCamera->GetPosition().ToString().c_str());
	// mpFont->Draw(cVector3f(5,43,0),12,cColor(1,1,1,1),eFontAlign_Left,"CharPos: %s",
	//	GetCharacterBody()->GetPosition().ToString().c_str());

	/*if(mpRayCallback->mpMaterial)
	mpFont->Draw(cVector3f(5,17,0),12,cColor(1,1,1,1),eFontAlign_Left,"Material: %s",
	mpGroundRayCallback->mpMaterial->GetName().c_str());*/

	// DEBUG: health
	if (mbShowHealth) {
		mpFont->draw(cVector3f(5, 5, 0), 12, cColor(1, 1, 1, 1), eFontAlign_Left, Common::U32String::format("Health: %.0f", mfHealth));
	}

	// DEBUG: misc
	// mpFont->Draw(cVector3f(5,20,0),12,cColor(1,1,1,1),eFontAlign_Left,
	//			_W("Ground: %d Speed: %f ForceSpeed: %f"),
	//													mpCharBody->IsOnGround()?1:0,
	//													mpCharBody->GetMoveSpeed(eCharDir_Forward),
	//													mpCharBody->GetForceVelocity().Length()
	//												);
	// cVector3f vGravity = mpInit->mpGame->GetScene()->GetWorld3D()->GetPhysicsWorld()->GetGravity();
	// mpFont->Draw(cVector3f(5,20,0),12,cColor(1,1,1,1),eFontAlign_Left,"Gravity: %s",
	//														vGravity.ToString().c_str());

	// DEBUG: sounds playing
	if (mbShowSoundsPlaying) {
		tStringVec vSoundNames;
		Common::Array<cSoundEntry *> vEntries;

		//////////////////////////////
		// Sound channels
		tSoundEntryList *pEntryList = mpInit->mpGame->GetSound()->GetSoundHandler()->GetWorldEntryList();
		for (tSoundEntryListIt it = pEntryList->begin(); it != pEntryList->end(); ++it) {
			iSoundChannel *pSound = it->mpSound;
			vSoundNames.push_back(pSound->GetData()->GetName());
			vEntries.push_back(&(*it));
		}

		vSoundNames.push_back("");
		vEntries.push_back(NULL);

		pEntryList = mpInit->mpGame->GetSound()->GetSoundHandler()->GetGuiEntryList();
		for (tSoundEntryListIt it = pEntryList->begin(); it != pEntryList->end(); ++it) {
			iSoundChannel *pSound = it->mpSound;
			vSoundNames.push_back(pSound->GetData()->GetName());
			vEntries.push_back(&(*it));
		}

		mpFont->draw(cVector3f(5, 18, 0), 10, cColor(1, 1, 1, 1), eFontAlign_Left, Common::U32String::format("Num of sounds: %d", vSoundNames.size() - 1));

		int lRow = 0, lCol = 0;
		for (int i = 0; i < (int)vSoundNames.size(); i++) {
			cSoundEntry *pEntry = vEntries[i];
			if (pEntry == NULL) {
				lRow = 4;
				lCol = 0;
				continue;
			}
			mpFont->draw(cVector3f((float)lCol * 250, 26 + (float)lRow * 11, 0), 10, cColor(1, 1, 1, 1), eFontAlign_Left,
						 Common::U32String::format("%S(%.2f (%.2f %.2f)->%.2f", cString::To16Char(vSoundNames[i]).c_str(),
												   pEntry->mpSound->GetVolume(),
												   pEntry->mfNormalVolumeMul,
												   pEntry->mfNormalVolumeFadeSpeed,
												   pEntry->mfNormalVolumeFadeDest,
												   pEntry->mpSound->GetPriority(),
												   pEntry->mpSound->GetElapsedTime(),
												   pEntry->mpSound->GetTotalTime()

													   ));
			//								pEntry->mpSound->GetPriority(),
			//								pEntry->mpSound->IsBufferUnderrun()?1:0);

			lCol++;
			if (lCol == 3) {
				lCol = 0;
				lRow++;
			}
		}

		//////////////////////////////
		// Music
		cMusicEntry *pMusic = mpInit->mpGame->GetSound()->GetMusicHandler()->GetCurrentSong();
		if (pMusic) {
			iSoundChannel *pChannel = pMusic->mpStream;
			mpFont->draw(cVector3f(5, 18 + 70, 0), 10, cColor(1, 1, 1, 1), eFontAlign_Left,
						 Common::U32String::format("Music: '%S' vol: %.2f playing: %d prio: %d elapsed: %.2f total time: %.2f",
												   cString::To16Char(pChannel->GetData()->GetName()).c_str(),
												   pChannel->GetVolume(),
												   pChannel->IsPlaying(),
												   pChannel->GetPriority(),
												   pChannel->GetElapsedTime(),
												   pChannel->GetTotalTime()));
		}
	}

	// DEBUG: Portals
	/*tString sPortals = "Portals: ";
	cPortalContainer *pContainer = mpInit->mpGame->GetScene()->GetWorld3D()->GetPortalContainer();
	tStringList* pStringList = pContainer->GetVisibleSectorsList();
	for(tStringListIt it=pStringList->begin(); it != pStringList->end(); ++it)
	{
		sPortals += *it + ", ";
	}

	mpFont->Draw(cVector3f(5,5,0),12,cColor(1,1,1,1),eFontAlign_Left,"%s",
															sPortals.c_str());*/

	mvStates[mState]->OnDraw();
}

void cPlayer::OnPostSceneDraw() {
	cCamera3D *pCam = static_cast<cCamera3D *>(mpScene->GetCamera());
	iLowLevelGraphics *pLowGfx = mpInit->mpGame->GetGraphics()->GetLowLevel();
	pLowGfx->SetMatrix(eMatrix_ModelView, pCam->GetViewMatrix());

	pLowGfx->SetTexture(0, NULL);
	pLowGfx->SetBlendActive(false);

	/*mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawLine(mvLineStart,mvLineEnd,cColor(1,1,1,1));
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawSphere(mvLineStart,0.1f,cColor(1,0,1,1));
	mpInit->mpGame->GetGraphics()->GetLowLevel()->DrawSphere(mvLineEnd,0.1f,cColor(1,0,1,1));*/

	mpFlashLight->OnPostSceneDraw();

	mvStates[mState]->OnPostSceneDraw();
}

//////////////////////////////////////////////////////////////////////////
// SAVING
//////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------

void cPlayer::AddSaveData(cSavedWorld *apSavedWorld) {
	// Collide callbacks
	tGameCollideScriptMapIt colIt = m_mapCollideCallbacks.begin();
	for (; colIt != m_mapCollideCallbacks.end(); ++colIt) {
		cGameCollideScript *pScript = colIt->second;
		cSaveGame_cGameCollideScript savedScript;
		savedScript.LoadFrom(pScript);

		apSavedWorld->mlstCollideCallbacks.Add(savedScript);
	}
}

//-------------------------------------------------------------------

void cPlayer::LoadSaveData(cSavedWorld *apSavedWorld) {
	// Collide callbacks
	// Collide scripts
	cContainerListIterator<cSaveGame_cGameCollideScript> colIt = apSavedWorld->mlstCollideCallbacks.GetIterator();
	while (colIt.HasNext()) {
		cSaveGame_cGameCollideScript &savedScript = colIt.Next();
		cGameCollideScript *pCallback = hplNew(cGameCollideScript, ());

		pCallback->mpEntity = mpInit->mpMapHandler->GetGameEntity(savedScript.msEntity);
		if (pCallback->mpEntity == NULL) {
			Warning("Couldn't find entity '%s'\n", savedScript.msEntity.c_str());
			hplDelete(pCallback);
			continue;
		}
		savedScript.SaveTo(pCallback);

		m_mapCollideCallbacks.insert(tGameCollideScriptMap::value_type(savedScript.msEntity, pCallback));
	}
}

//-------------------------------------------------------------------

void cPlayer::SaveToGlobal(cPlayer_GlobalSave *apSave) {
	cPlayer_GlobalSave *pData = apSave;

	//////////////////////////////
	// Stats
	kSaveData_SaveTo(mlStat_NumOfSaves);

	//////////////////////////////
	// Global
	kSaveData_SaveTo(mfForwardUpMul);
	kSaveData_SaveTo(mfForwardRightMul);

	kSaveData_SaveTo(mfUpMul);
	kSaveData_SaveTo(mfRightMul);

	kSaveData_SaveTo(mbPickAtPoint);
	kSaveData_SaveTo(mbRotateWithPlayer);
	kSaveData_SaveTo(mbUseNormalMass);

	kSaveData_SaveTo(mfGrabMassMul);

	//////////////////////////////
	// Private
	kSaveData_SaveTo(mbActive);

	kSaveData_SaveTo(mfHeightAdd);

	kSaveData_SaveTo(mfSpeedMul);
	kSaveData_SaveTo(mfHealthSpeedMul);
	kSaveData_SaveTo(mfHeadMoveSizeMul);
	kSaveData_SaveTo(mfHeadMoveSpeedMul);

	kSaveData_SaveTo(mState);
	kSaveData_SaveTo(mMoveState);
	kSaveData_SaveTo(mCrossHairState);

	kSaveData_SaveTo(mbItemFlash);
	kSaveData_SaveTo(mfHealth);
	kSaveData_SaveTo(mfPower);
	kSaveData_SaveTo(mfMass);

	///////////////////////////////////////
	// Particles on camera
	cNode3D *pNode = mpCamera->GetAttachmentNode();

	pData->mvOnCameraPS.Clear();
	cEntityIterator it = pNode->GetEntityIterator();
	while (it.HasNext()) {
		iEntity3D *pEntity3D = static_cast<iEntity3D *>(it.Next());
		if (pEntity3D->GetEntityType() == "ParticleSystem3D") {
			cParticleSystem3D *pPS = static_cast<cParticleSystem3D *>(pEntity3D);

			cPlayer_GlobalSave_CameraPS cameraPS;
			cameraPS.msName = pPS->GetName();
			cameraPS.msFile = pPS->GetDataName();
			pData->mvOnCameraPS.Add(cameraPS);
		}
	}

	//////////////////////////////
	// Lights
	apSave->mbFlashlightActive = mpFlashLight->IsActive();
	apSave->mbFlashlightDisabled = mpFlashLight->IsDisabled();
	apSave->mbGlowstickActive = mpGlowStick->IsActive();
	apSave->mbFlareActive = mpFlare->IsActive();
	apSave->mfFlareTime = mpFlare->GetTime();

	//////////////////////////////
	// Body and Camera Specific
	pData->mvPosition = mpCharBody->GetPosition();
	pData->mfYaw = mpCharBody->GetYaw();
	pData->mfPitch = mpCamera->GetPitch();
}

//-------------------------------------------------------------------

void cPlayer::LoadFromGlobal(cPlayer_GlobalSave *apSave) {
	cPlayer_GlobalSave *pData = apSave;

	//////////////////////////////
	// Stats
	kSaveData_LoadFrom(mlStat_NumOfSaves);

	//////////////////////////////
	// Global
	kSaveData_LoadFrom(mfForwardUpMul);
	kSaveData_LoadFrom(mfForwardRightMul);

	kSaveData_LoadFrom(mfUpMul);
	kSaveData_LoadFrom(mfRightMul);

	kSaveData_LoadFrom(mbPickAtPoint);
	kSaveData_LoadFrom(mbRotateWithPlayer);
	kSaveData_LoadFrom(mbUseNormalMass);

	kSaveData_LoadFrom(mfGrabMassMul);

	//////////////////////////////
	// Private
	SetActive(pData->mbActive);

	kSaveData_LoadFrom(mfHeightAdd);

	// Probably better of skipping these:
	/*kSaveData_LoadFrom(mfSpeedMul);
	kSaveData_LoadFrom(mfHealthSpeedMul);
	kSaveData_LoadFrom(mfHeadMoveSizeMul);
	kSaveData_LoadFrom(mfHeadMoveSpeedMul);*/

	// Skip these for now.
	ChangeMoveState(apSave->mMoveState, true);
	// kSaveData_LoadFrom(mState);
	// kSaveData_LoadFrom(mMoveState);
	// kSaveData_LoadFrom(mCrossHairState);

	kSaveData_LoadFrom(mbItemFlash);
	kSaveData_LoadFrom(mfHealth);
	kSaveData_LoadFrom(mfPower);
	// kSaveData_LoadFrom(mfMass); //Skip this

	///////////////////////////////////////
	// Particles on camera
	cWorld3D *pWorld = mpInit->mpGame->GetScene()->GetWorld3D();
	for (size_t i = 0; i < pData->mvOnCameraPS.Size(); ++i) {
		cParticleSystem3D *pPS = pWorld->CreateParticleSystem(
			pData->mvOnCameraPS[i].msName,
			pData->mvOnCameraPS[i].msFile,
			1, cMatrixf::Identity);
		if (pPS) {
			mpCamera->AttachEntity(pPS);
		}
	}

	//////////////////////////////
	// Lights
	if (apSave->mbFlashlightActive)
		mpFlashLight->SetActive(true);
	if (apSave->mbFlashlightDisabled)
		mpFlashLight->SetDisabled(true);
	if (apSave->mbGlowstickActive)
		mpGlowStick->SetActive(true);
	if (apSave->mbFlareActive) {
		mpFlare->SetActive(true);
		mpFlare->SetTime(apSave->mfFlareTime);
	}

	//////////////////////////////
	// Body and Camera Specific
	mpCharBody->SetPosition(pData->mvPosition);

	mpCharBody->SetYaw(pData->mfYaw);
	mpCamera->SetYaw(pData->mfYaw);
	mpCamera->SetPitch(pData->mfPitch);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE OBJECT STUFF
//////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------

kBeginSerializeBase(cSaveData_cPlayer)
	kSerializeVar(mlStat_NumOfSaves, eSerializeType_Int32)
		kSerializeClassContainer(mlstCollideCallbacks, cSaveGame_cGameCollideScript, eSerializeType_Class)
			kEndSerialize()

	//-----------------------------------------------------------------------

	iSaveObject *cSaveData_cPlayer::CreateSaveObject(cSaveObjectHandler *apSaveObjectHandler, cGame *apGame) {
	cContainerListIterator<cSaveGame_cGameCollideScript> it = mlstCollideCallbacks.GetIterator();
	while (it.HasNext()) {
		cSaveGame_cGameCollideScript &saveScript = it.Next();

		iGameEntity *pEntity = gpInit->mpMapHandler->GetGameEntity(saveScript.msEntity);
		if (pEntity == NULL) {
			Error("Couldn't find game entity '%s'\n", saveScript.msEntity.c_str());
			continue;
		}

		cGameCollideScript *pCallback = hplNew(cGameCollideScript, ());
		pCallback->mpEntity = pEntity;
		saveScript.SaveTo(pCallback);

		gpInit->mpPlayer->m_mapCollideCallbacks.insert(tGameCollideScriptMap::value_type(saveScript.msEntity, pCallback));
	}

	return NULL;
}

//-----------------------------------------------------------------------

int cSaveData_cPlayer::GetSaveCreatePrio() {
	return 10;
}

//-----------------------------------------------------------------------

iSaveData *cPlayer::CreateSaveData() {
	cSaveData_cPlayer *pData = hplNew(cSaveData_cPlayer, ());

	// Collide callbacks
	{
		tGameCollideScriptMapIt it = m_mapCollideCallbacks.begin();
		for (; it != m_mapCollideCallbacks.end(); ++it) {
			cGameCollideScript *pScript = it->second;
			cSaveGame_cGameCollideScript saveScript;
			saveScript.LoadFrom(pScript);
			pData->mlstCollideCallbacks.Add(saveScript);
		}
	}

	return pData;
}

//-----------------------------------------------------------------------
