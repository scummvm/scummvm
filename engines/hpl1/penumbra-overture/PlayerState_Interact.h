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

#ifndef GAME_PLAYER_STATE_INTERACT_H
#define GAME_PLAYER_STATE_INTERACT_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/PlayerState.h"

using namespace hpl;

//-----------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// GRAB STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------

class cPlayerState_Grab : public iPlayerState {
public:
	cPlayerState_Grab(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void OnDraw();

	void OnPostSceneDraw();

	bool OnJump();

	void OnStartInteractMode();

	void OnStartInteract();
	void OnStopInteract();
	void OnStartExamine();

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnStartCrouch();
	void OnStopCrouch();
	bool OnStartInventory();
	bool OnStartInventoryShortCut(int alNum);

	static float mfMassDiv;

private:
	cVector3f mvRelPickPoint;
	iPhysicsBody *mpPushBody;

	ePlayerMoveState mPrevMoveState;
	ePlayerState mPrevState;

	cSpringVec3 mGrabSpring;
	cPidControllerVec3 mGrabPid;
	cSpringVec3 mRotateSpring;
	cPidControllerVec3 mRotatePid;

	bool mbHasGravity;

	bool mbMoveHand;

	float mfGrabDist;

	float mfDefaultMass;

	bool mbHasPlayerGravityPush;

	bool mbPickAtPoint;
	bool mbRotateWithPlayer;

	cVector3f mvObjectUp;
	cVector3f mvObjectRight;

	cVector3f mvCurrentUp;
	cVector3f mvCurrentUpAxis;

	float mfStartYaw;

	cVector3f mvCurrentDisered;

	float mfSpeedMul;

	float mfYRotation;

	float mfMaxPidForce;

	float mfMinThrowMass;
	float mfMaxThrowMass;

	float mfMinThrowImpulse;
	float mfMaxThrowImpulse;

	iFontData *mpFont;
};

//-----------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MOVE STATE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------

class cPlayerState_Move_BodyCallback : public iPhysicsBodyCallback {
public:
	cPlayerState_Move_BodyCallback(cPlayer *apPlayer, float afTimeStep);

	bool OnBeginCollision(iPhysicsBody *apBody, iPhysicsBody *apCollideBody);
	void OnCollide(iPhysicsBody *apBody, iPhysicsBody *apCollideBody, cPhysicsContactData *apContactData);

	float mfTimeStep;
	int mlBackCount;
	cPlayer *mpPlayer;

private:
};

//-----------------------------------------------------------------

class cPlayerState_Move : public iPlayerState {
public:
	cPlayerState_Move(cInit *apInit, cPlayer *apPlayer);
	~cPlayerState_Move();

	void OnUpdate(float afTimeStep);

	void OnStartInteract();
	void OnStopInteract();

	bool OnJump();

	void OnStartExamine();

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnPostSceneDraw();

	bool OnStartInventory();
	bool OnStartInventoryShortCut(int alNum);

private:
	cVector3f mvForward;
	cVector3f mvRight;
	cVector3f mvUp;

	cVector3f mvRelPickPoint;
	cVector3f mvPickPoint;

	bool bPausedGravity;

	iPhysicsBody *mpPushBody;

	int mlMoveCount;

	ePlayerMoveState mPrevMoveState;
	ePlayerState mPrevState;

	cPlayerState_Move_BodyCallback *mpCallback;
};

//////////////////////////////////////////////////////////////////////////
// PUSH STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerState_Push : public iPlayerState {
public:
	cPlayerState_Push(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);
	bool OnJump();

	void OnStartInteract();
	void OnStopInteract();

	void OnStartExamine();

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnPostSceneDraw();

	bool OnStartInventory();
	bool OnStartInventoryShortCut(int alNum);

private:
	cVector3f mvForward;
	cVector3f mvRight;

	cVector3f mvRelPickPoint;

	cVector3f mvLocalPickPoint;

	iPhysicsBody *mpPushBody;
	cVector3f mvLastBodyPos;

	cVector2f mvPrevPitchLimits;

	bool mbHasPlayerGravityPush;

	float mfMaxSpeed;

	int mlForward;
	int mlSideways;

	ePlayerMoveState mPrevMoveState;
	ePlayerState mPrevState;
};

#endif // GAME_PLAYER_STATE_INTERACT_H
