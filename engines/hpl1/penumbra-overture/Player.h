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

#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "hpl1/engine/engine.h"

using namespace hpl;

#include "hpl1/penumbra-overture/GameTypes.h"

class cInit;
class iPlayerMoveState;
class iPlayerState;
class cPlayer;

class iPlayerWeaponCallback;
class cPlayerGroundRayCallback;
class cPlayerPickRayCallback;
class cPlayerHeadMove;
class cPlayerHealth;
class cPlayerEarRing;
class cPlayerLean;
class cPlayerDamage;
class cPlayerDeath;
class cPlayerFlashLight;
class cPlayerGlowStick;
class cPlayerFlare;
class cPlayerNoiseFilter;
class cPlayerFearFilter;
class cPlayerLookAt;
class cPlayerHidden;
class cPlayerAmbientLight;

class cPlayerBodyCallback;

typedef Common::Array<iPlayerState *> tPlayerStateVec;
typedef tPlayerStateVec::iterator tPlayerStateVecIt;

typedef Common::Array<iPlayerMoveState *> tPlayerMoveStateVec;
typedef tPlayerMoveStateVec::iterator tPlayerMoveStateVecIt;

class cInventoryItem;
class cPlayer_GlobalSave;
class cSavedWorld;

//---------------------------------------------

kSaveData_BaseClass(cPlayer) {
	kSaveData_ClassInit(cPlayer) public :

		int mlStat_NumOfSaves;

	cContainerList<cSaveGame_cGameCollideScript> mlstCollideCallbacks;

	virtual iSaveObject *CreateSaveObject(cSaveObjectHandler * apSaveObjectHandler, cGame * apGame);
	virtual int GetSaveCreatePrio();
};

//---------------------------------------------

class iPlayerMoveState {
public:
	iPlayerMoveState(cPlayer *apPlayer, cInit *apInit);
	virtual ~iPlayerMoveState() = default;

	void SetupBody();

	void InitState(iPlayerMoveState *apPrevState);

	void Update(float afTimeStep);

	void Stop();

	void Start();

	virtual void EnterState(iPlayerMoveState *apPrevState) {}
	virtual void LeaveState(iPlayerMoveState *apNextState) {}

	virtual void OnUpdate(float afTimeStep) {}

	bool mbActive;

	float mfForwardSpeed;
	float mfBackwardSpeed;
	float mfSidewaySpeed;

	float mfForwardAcc;
	float mfForwardDeacc;
	float mfSidewayAcc;
	float mfSidewayDeacc;

	float mfMaxHeadMove;
	float mfMinHeadMove;
	float mfHeadMoveSpeed;
	float mfHeadMoveBackSpeed;

	float mfFootStepMul;

	tString msStepType;

	float mfSpeedMul;

	float mfHeightAddCount;
	float mfHeightAdd;
	float mfHeightAddSpeed;

	cPlayer *mpPlayer;
	cPlayerHeadMove *mpHeadMove;

	cInit *mpInit;
	cConfigFile *mpGameConfig;

	ePlayerMoveState mPrevMoveState;

	ePlayerMoveState mType;
};
//---------------------------------------------

class cPlayer : public iUpdateable {
	friend class cSaveData_cPlayer;

public:
	cPlayer(cInit *apInit);
	~cPlayer();

	////////////////////////////////////////
	// Interaction

	void MoveForwards(float afMul, float afTimeStep);
	void MoveSideways(float afMul, float afTimeStep);
	void AddYaw(float afVal);
	void AddPitch(float afVal);
	void Lean(float afMul, float afTimeStep);

	void StartInteract();
	void StopInteract();
	void StartExamine();
	void StopExamine();
	void StartHolster();

	void Jump();

	void StartRun();
	void StopRun();

	void StartCrouch();
	void StopCrouch();

	void StartInteractMode();

	void StartInventory();

	void StartInventoryShortCut(int alNum);

	void StartFlashLightButton();

	void StartGlowStickButton();

	////////////////////////////////
	// Actions
	void SetStartPos(const tString &asName);

	void FootStep(float afMul, const tString &asType = "", bool abSkipCount = false);

	void ChangeState(ePlayerState aState);
	void ChangeMoveState(ePlayerMoveState aState, bool abSetHeadHeightDirectly = false);

	void AddCollideScript(eGameCollideScriptType aType, const tString &asFunc, const tString &asEntity);
	void RemoveCollideScript(eGameCollideScriptType aType, const tString &asFunc);
	void RemoveCollideScriptWithChildEntity(iGameEntity *apEntity);
	void ClearCollideScripts();

	void SetHealth(float afX);
	void AddHealth(float afX);
	float GetHealth() { return mfHealth; }
	void Damage(float afDamage, ePlayerDamageType aType);

	void DestroyWorldObjects();

	iSaveData *CreateSaveData();

	/////////////////////////////////////////
	// Events
	void OnWorldLoad();
	void OnWorldExit();
	void OnStart();
	void Update(float afTimeStep);
	void Reset();
	void OnDraw();
	void OnPostSceneDraw();

	/////////////////////////////////////////
	// Saving

	void AddSaveData(cSavedWorld *apSavedWorld);
	void LoadSaveData(cSavedWorld *apSavedWorld);

	void SaveToGlobal(cPlayer_GlobalSave *apSave);
	void LoadFromGlobal(cPlayer_GlobalSave *apSave);

	/////////////////////////////////////////
	// Properties
	bool IsDead();

	void SetActive(bool abActive);
	bool IsActive() { return mbActive; }

	void SetPushBody(iPhysicsBody *apBody) { mpPushBody = apBody; }
	iPhysicsBody *GetPushBody() { return mpPushBody; }

	float GetDefaultMass() { return mfDefaultMass; }

	float GetMass() { return mfMass; }
	void SetMass(float afX);

	void SetPower(float afX);
	void AddPower(float afX);
	float GetPower() { return mfPower; }

	iCharacterBody *GetCharacterBody() { return mpCharBody; }
	cCamera3D *GetCamera() { return mpCamera; }

	cVector3f GetSize() { return mvSize; }
	float GetCrouchHeight() { return mfCrouchHeight; }

	float GetMaxPushDist() { return mfMaxPushDist; }
	float GetMaxPushSpeed() { return mfMaxPushSpeed; }
	cVector2f GetMaxPushHeadMovement() { return mvMaxPushHeadMovement; }
	cVector2f GetMinPushHeadMovement() { return mvMinPushHeadMovement; }

	float GetMaxMoveDist() { return mfMaxMoveDist; }

	float GetMaxGrabDist() { return mfMaxGrabDist; }

	float GetMaxUseItemDist() { return mfMaxUseItemDist; }

	void SetPrevMoveState(ePlayerMoveState aState);
	iPlayerMoveState *GetMoveStateData(ePlayerMoveState aState) { return mvMoveStates[aState]; }
	ePlayerMoveState GetMoveState() { return mMoveState; }
	ePlayerState GetState() { return mState; }

	iPlayerState *GetStateData(ePlayerState aState) { return mvStates[aState]; }

	float GetSpeedMul() { return mfSpeedMul; }
	void SetSpeedMul(float afSpeedMul);

	float GetHealthSpeedMul() { return mfHealthSpeedMul; }
	void SetHealthSpeedMul(float afHealthSpeedMul);

	void SetHeadMoveSizeMul(float afX) { mfHeadMoveSizeMul = afX; }
	void SetHeadMoveSpeedMul(float afX) { mfHeadMoveSpeedMul = afX; }
	float GetHeadMoveSizeMul() { return mfHeadMoveSizeMul; }
	float GetHeadMoveSpeedMul() { return mfHeadMoveSpeedMul; }

	float GetLookSpeed() { return mfLookSpeed; }

	void SetCrossHairState(eCrossHairState aState) { mCrossHairState = aState; }
	eCrossHairState GetCrossHairState() { return mCrossHairState; }

	iPhysicsBody *GetPickedBody();
	void SetPickedBody(iPhysicsBody *apBody);
	float GetPickedDist();
	const cVector3f &GetPickedPos();
	cPlayerPickRayCallback *GetPickRay();

	float GetCameraHeightAdd() { return mfCameraHeightAdd; }
	bool IsMoving() { return mbMoving; }

	cPlayerFlashLight *GetFlashLight() { return mpFlashLight; }

	cPlayerHeadMove *GetHeadMove() { return mpHeadMove; }

	cPlayerEarRing *GetEarRing() { return mpEarRing; }

	cPlayerGlowStick *GetGlowStick() { return mpGlowStick; }

	cPlayerFlare *GetFlare() { return mpFlare; }

	cPlayerNoiseFilter *GetNoiseFilter() { return mpNoiseFilter; }

	cPlayerFearFilter *GetFearFilter() { return mpFearFilter; }

	cPlayerLookAt *GetLookAt() { return mpLookAt; }

	cPlayerHidden *GetHidden() { return mpHidden; }

	float GetHeightAdd() { return mfHeightAdd; }
	void SetHeightAdd(float afX) { mfHeightAdd = afX; }

	cInit *GetInit() { return mpInit; }

	cVector2f GetInteractMoveBorder() { return mvInteractMoveBorder; }

	cVector2f GetCrossHairPos() { return mvCrossHairPos; }
	void SetCrossHairPos(const cVector2f &avPos) { mvCrossHairPos = avPos; }
	bool AddCrossHairPos(const cVector2f &avPos);
	void ResetCrossHairPos() { mvCrossHairPos = cVector2f(400, 300); }

	void SetCurrentItem(cInventoryItem *apItem) { mpCurrentItem = apItem; }
	cInventoryItem *GetCurrentItem() { return mpCurrentItem; }
	void SetItemFlash(bool abX) { mbItemFlash = abX; }

	void SetWeaponCallback(iPlayerWeaponCallback *apCallback) { mpWeaponCallback = apCallback; }
	iPlayerWeaponCallback *GetWeaponCallback() { return mpWeaponCallback; }

	bool GetLandedFromJump() { return mbLandedFromJump; }
	void SetLandedFromJump(bool abX) { mbLandedFromJump = abX; }

	void SetJumpButtonDown(bool abX);
	bool GetJumpButtonDown() { return mbJumpButtonDown; }
	float GetJumpCount() { return mfJumpCount; }
	float GetMaxJumpCount() { return mfMaxJumpCount; }

	/////////////////////////////////////////
	// Stats
	int mlStat_NumOfSaves;

	/////////////////////////////////////////
	// State properties
	float mfForwardUpMul;
	float mfForwardRightMul;

	float mfUpMul;
	float mfRightMul;

	bool mbPickAtPoint;
	bool mbRotateWithPlayer;
	bool mbUseNormalMass;

	float mfGrabMassMul;

	bool mbCanBeThrown;
	bool mbCanBePulled;

	float mfCurrentMaxInteractDist;

	// Haptic
	bool mbGrabbingMoveBody;
	bool mbProxyTouching;
	float mfHapticTorqueMul;

	bool mbDamageFromPos;
	cVector3f mvDamagePos;

	// Debug
	cVector3f mvLineStart;
	cVector3f mvLineEnd;

private:
	cInit *mpInit;

	cScene *mpScene;
	cGraphics *mpGraphics;
	cResources *mpResources;
	cGraphicsDrawer *mpGfxDrawer;

	FontData *mpFont;

	cNode3D mFeetNode;

	bool mbActive;

	cCamera3D *mpCamera;
	float mfLookSpeed;
	float mfCameraHeightAdd;

	cVector3f mvSize;
	float mfCrouchHeight;

	bool mbJumpButtonDown;
	float mfJumpCount;
	float mfMaxJumpCount;

	bool mbLandedFromJump;

	cVector2f mvInteractMoveBorder;

	float mfHeightAdd;

	float mfMaxPushDist;
	float mfMaxPushSpeed;

	float mfMaxMoveDist;

	float mfMaxGrabDist;
	// float mfMaxGrabForce;

	float mfMaxUseItemDist;

	float mfSpeedMul;
	float mfHealthSpeedMul;

	float mfHeadMoveSizeMul;
	float mfHeadMoveSpeedMul;

	iPhysicsBody *mpPushBody;

	Common::Array<cGfxObject *> mvCrossHairs;

	cVector2f mvMaxPushHeadMovement;
	cVector2f mvMinPushHeadMovement;

	bool mbMoving;

	int mlGroundCount;

	iCharacterBody *mpCharBody;

	cPlayerGroundRayCallback *mpGroundRayCallback;
	cPlayerPickRayCallback *mpPickRayCallback;

	cPlayerBodyCallback *mpBodyCallback;

	cPlayerHeadMove *mpHeadMove;
	cPlayerDamage *mpDamage;
	cPlayerDeath *mpDeath;
	cPlayerFlashLight *mpFlashLight;
	cPlayerLean *mpLean;
	cPlayerEarRing *mpEarRing;
	cPlayerGlowStick *mpGlowStick;
	cPlayerFlare *mpFlare;
	cPlayerHealth *mpHealth;
	cPlayerNoiseFilter *mpNoiseFilter;
	cPlayerFearFilter *mpFearFilter;
	cPlayerLookAt *mpLookAt;
	cPlayerHidden *mpHidden;

	iPlayerWeaponCallback *mpWeaponCallback;

	ePlayerState mState;
	tPlayerStateVec mvStates;

	ePlayerMoveState mMoveState;
	tPlayerMoveStateVec mvMoveStates;

	eCrossHairState mCrossHairState;
	cVector2f mvCrossHairPos;

	cInventoryItem *mpCurrentItem;
	cLinearOscillation mItemFlash;
	bool mbItemFlash;

	float mfHealth;

	float mfPower;

	float mfDefaultMass;
	float mfMass;

	bool mbUpdatingCollisionCallbacks;

	tSoundEntityList mlstFootStepSounds;

	// Debug
	bool mbShowHealth;
	bool mbShowSoundsPlaying;

	tGameCollideScriptMap m_mapCollideCallbacks;
};

#endif // GAME_PLAYER_H
