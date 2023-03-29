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

#ifndef GAME_PLAYER_HELPER_H
#define GAME_PLAYER_HELPER_H

#include "hpl1/engine/engine.h"

using namespace hpl;

#include "hpl1/penumbra-overture/GameTypes.h"

class cInit;
class cPlayer;

//---------------------------------------------

class iPlayerWeaponCallback {
public:
	virtual ~iPlayerWeaponCallback() = default;
	virtual tString GetModelFile() = 0;
	virtual cMatrixf GetOffsetMatrix() = 0;

	virtual void OnAttackDown() = 0;
	virtual void OnAttackUp() = 0;
};

//---------------------------------------------

class cPlayerGroundRayCallback : public iPhysicsRayCallback {
public:
	cPlayerGroundRayCallback();

	void Clear();
	void Reset();

	void OnWorldExit();

	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	bool mbOnGround;
	iPhysicsMaterial *mpMaterial;
	float mfMinDist;
};

//---------------------------------------------

class cPlayerPickRayCallback : public iPhysicsRayCallback {
public:
	cPlayerPickRayCallback();

	void OnWorldExit();

	void Clear();

	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	void CalculateResults();

	iPhysicsBody *mpPickedBody;
	iPhysicsBody *mpPickedAreaBody;

	float mfMaxDistance;

	float mfPickedDist;
	float mfPickedAreaDist;

	eGameEntityType mLastEntityType;

	cVector3f mvPickedPos;
	cVector3f mvPickedAreaPos;

	float mfMinStaticDist;
};

//---------------------------------------------

class cPlayerHeadMove {
public:
	cPlayerHeadMove(cPlayer *apPlayer);

	void Update(float afTimeStep);

	void Start();
	void Stop();

	float GetPos();

	bool IsActive() { return mbActive; }
	float GetPosition() { return mfPos; }
	float GetClimbPosition() { return mfClimbPos; }
	float GetClimbCount() { return mfClimbCount; }

	float mfMaxHeadMove;
	float mfMinHeadMove;
	float mfHeadMoveSpeed;
	float mfHeadMoveBackSpeed;

	float mfFootStepMul;

private:
	float mfAdd;
	float mfPos;
	bool mbActive;
	bool mbFootStepped;
	bool mbWasActive;

	float mfClimbCount;
	float mfClimbPos;

	cPlayer *mpPlayer;
};

//---------------------------------------------

class cPlayerHealth {
public:
	cPlayerHealth(cInit *apInit);

	void Reset();

	void Update(float afTimeStep);

	void Draw();

private:
	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	float mfTimeCount;
	float mfGfxAlpha;
	float mfGfxAlphaAdd;
	float mfGfxGlobalAlpha;

	cSoundEntry *mpSoundEntry;
	float mfTerrorCheckCount;
	int mlTerrorLevel;

	cGfxObject *mpDamageGfx;
};

//---------------------------------------------

class cPlayerEarRing {
public:
	cPlayerEarRing(cInit *apInit, cPlayer *apPlayer);

	void Update(float afTimeStep);

	void Start(float afTime);
	void Stop(bool abStopDirectly);

	void Reset();

private:
	cInit *mpInit;
	cPlayer *mpPlayer;

	cSoundEntry *mpSoundEntry;

	float mfTimeCount;
	float mfDeafTime;
	bool mbDeaf;
};

//---------------------------------------------

class cPlayerLean : public iPhysicsRayCallback {
public:
	cPlayerLean(cInit *apInit, cPlayer *apPlayer);

	void OnWorldLoad();

	void Update(float afTimeStep);

	void Lean(float afMul, float afTimeStep);

	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	void Reset();

	float mfDir;
	float mfMovement;
	float mfRotation;

private:
	cPlayer *mpPlayer;
	cInit *mpInit;

	iCollideShape *mpHeadShape;

	float mfMaxMovement;
	float mfMaxRotation;
	float mfMaxTime;

	float mfMoveSpeed;

	bool mbIntersect;

	bool mbPressed;
};

//---------------------------------------------

class cPlayerDamage {
public:
	cPlayerDamage(cInit *apInit);
	~cPlayerDamage();

	void Start(float afSize, ePlayerDamageType aType);

	void Update(float afTimeStep);

	void Draw();

private:
	bool mbActive;

	float mfAlpha;
	float mfAlphaAdd;

	cVector2f mvHeadSwingAcc;
	cVector2f mvHeadSwingSpeed;

	float mfSize;

	cGfxObject *mvHitGfx[ePlayerDamageType_LastEnum];

	ePlayerDamageType mType;

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;
	cRendererPostEffects *mpPostEffects;
};

//---------------------------------------------

class cPlayerDeath {
public:
	cPlayerDeath(cInit *apInit);
	~cPlayerDeath();

	void Reset();

	void Start();

	void Update(float afTimeStep);

	void Draw();

	bool IsActive() { return mbActive; }

	float GetHeighAdd() { return mfHeightAdd; }

private:
	bool mbActive;

	float mfHeightAdd;
	float mfRoll;

	float mfMinHeightAdd;

	cGfxObject *mpFadeGfx;
	cGfxObject *mpBlackGfx;

	bool mbStartFade;
	float mfFadeAlpha;
	float mfBlackAlpha;

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;
	cRendererPostEffects *mpPostEffects;
};

//---------------------------------------------

class cPlayerFlashLight : iPhysicsRayCallback {
public:
	cPlayerFlashLight(cInit *apInit);
	~cPlayerFlashLight();

	void Reset();

	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	void OnWorldLoad();

	void Update(float afTimeStep);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	void SetDisabled(bool abX);
	bool IsDisabled() { return mbDisabled; }

	void OnPostSceneDraw();

private:
	bool mbActive;

	bool mbDisabled;

	float mfAlpha;

	float mfFlickerTime;

	float mfRayCastTime;
	float mfClosestDist;
	iPhysicsBody *mpClosestBody;

	// iTexture *mpTextureCache;

	cInit *mpInit;

	cVector3f mvStart;
	cVector3f mvEnd;
};

//---------------------------------------------

class cPlayerGlowStick {
public:
	cPlayerGlowStick(cInit *apInit);
	~cPlayerGlowStick();

	void Reset();

	void OnWorldLoad();
	void Destroy();

	void Update(float afTimeStep);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

private:
	bool mbActive;

	cInit *mpInit;
};

//---------------------------------------------

class iHudModel;
class cPlayerFlare {
public:
	cPlayerFlare(cInit *apInit);
	~cPlayerFlare();

	void Reset();

	void OnWorldLoad();

	void Update(float afTimeStep);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	float GetTime() { return mfTime; }
	void SetTime(float afX) { mfTime = afX; }

private:
	bool mbActive;

	float mfTime;

	iLight3D *mpLight;
	iHudModel *mpModel;

	float mfMaxLightRadius;
	float mfLightRadius;
	float mfLightPulse;
	float mfLightPulseAdd;

	cInit *mpInit;
};

//---------------------------------------------

class cPlayerNoiseFilter {
public:
	cPlayerNoiseFilter(cInit *apInit);
	~cPlayerNoiseFilter();

	void Draw();

	void Update(float afTimeStep);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

private:
	bool mbActive;

	float mfAlpha;
	float mfTimeCount;

	int mlAmount;

	// cLight3DPoint *mpLight;
	// cMeshEntity *mpMeshEntity;

	cInit *mpInit;
	cGraphicsDrawer *mpDrawer;

	Common::Array<cGfxObject *> mvGfxNoise;
	tIntVec mvCurrentGfx;
};

//---------------------------------------------

class cPlayerFearFilter {
public:
	cPlayerFearFilter(cInit *apInit);
	~cPlayerFearFilter();

	void Draw();

	void Update(float afTimeStep);

	void Reset();

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

private:
	bool mbActive;

	float mfAlpha;
	float mfMaxAlpha;

	cInit *mpInit;
};

//---------------------------------------------

class cPlayerLookAt {
public:
	cPlayerLookAt(cPlayer *apPlayer);
	~cPlayerLookAt();

	void Update(float afTimeStep);

	void Reset();

	void SetTarget(const cVector3f &avTargetPos, float afSpeedMul, float afMaxSpeed);

	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

private:
	bool mbActive;
	cVector3f mvTargetPos;

	float mfSpeedMul;
	float mfMaxSpeed;

	cPlayer *mpPlayer;
};

//---------------------------------------------

class cPlayerHidden : public iPhysicsRayCallback {
public:
	cPlayerHidden(cInit *apInit);
	~cPlayerHidden();

	void Draw();

	void Update(float afTimeStep);

	void Reset();

	void OnWorldLoad();
	void OnWorldExit();

	bool InShadows();
	bool IsHidden() { return mbHidden; }
	void UnHide();

	bool OnIntersect(iPhysicsBody *pBody, cPhysicsRayParams *apParams);

	bool HasLineOfSight(iLight3D *pLight, iPhysicsWorld *pPhysicsWorld);

private:
	void UpdateEnemyTooClose(float afTimeStep);

	cInit *mpInit;
	cRenderer3D *mpRenderer;
	cGraphicsDrawer *mpDrawer;
	cSoundHandler *mpSoundHandler;

	cGfxObject *mpInShadowGfx;

	iLight3D *mpLight;

	float mfInShadowAlpha;
	float mfInShadowPulse;
	float mfInShadowPulseAdd;

	float mfStartAspect;
	float mfHiddenAspect;
	float mfAspect;
	float mfAspectAdd;

	float mfStartFov;
	float mfHiddenFov;
	float mfFov;
	float mfFovAdd;

	float mfStartEffectOffset;
	float mfHiddenEffectOffset;
	float mfEffectOffset;
	float mfEffectOffsetAdd;

	float mfHiddenOnTime;
	float mfHiddenOffTime;

	float mfCheckEnemyCloseCount;
	float mfEnemyTooCloseCount;
	float mfEnemyTooCloseMax;
	bool mbEnemyTooClose;

	float mfCloseEffectFov;
	float mfCloseEffectFovAdd;
	float mfCloseEffectFovMax;
	float mfCloseEffectFovMin;

	float mbInShadows;

	float mfUpdateCount;

	float mfLight;
	float mfMaxLight;

	float mfHiddenCount;
	float mfHiddenTime;
	bool mbHidden;
	float mfHiddenOnCount;

	bool mbIntersected;

	FontData *mpFont;
};

//---------------------------------------------

class cPlayerBodyCallback : public iCharacterBodyCallback {
public:
	cPlayerBodyCallback(cPlayer *apPlayer);
	virtual ~cPlayerBodyCallback() = default;

	void OnHitGround(iCharacterBody *apCharBody, const cVector3f &avVel);
	void OnGravityCollide(iCharacterBody *apCharBody, iPhysicsBody *apCollideBody,
						  cCollideData *apCollideData) {}

private:
	cPlayer *mpPlayer;

	float mfFallSpeed_Min;
	float mfFallDamage_Min;
	tString msFallSound_Min;

	float mfFallSpeed_Med;
	float mfFallDamage_Med;
	tString msFallSound_Med;

	float mfFallSpeed_Max;
	float mfFallDamage_Max;
	tString msFallSound_Max;
};

//---------------------------------------------

#endif // GAME_PLAYER_HELPER_H
