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

#ifndef GAME_PLAYER_STATE_MISC_H
#define GAME_PLAYER_STATE_MISC_H

#include "hpl1/engine/engine.h"
#include "hpl1/penumbra-overture/PlayerState.h"

using namespace hpl;

//////////////////////////////////////////////////////////////////////////
// NORMAL STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerState_Normal : public iPlayerState {
public:
	cPlayerState_Normal(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void OnStartInteract();
	void OnStartExamine();

	void OnStartRun();
	void OnStopRun();
	void OnStartCrouch();
	void OnStopCrouch();

	void OnStartInteractMode();

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);
};
//////////////////////////////////////////////////////////////////////////
// INTERACT MODE STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerState_InteractMode : public iPlayerState {
public:
	cPlayerState_InteractMode(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void OnStartInteract();
	void OnStartExamine();

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	void OnStartInteractMode();

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnStartCrouch();
	void OnStopCrouch();

private:
	ePlayerMoveState mPrevMoveState;

	cVector2f mvLookSpeed;
	float mfRange;
};

//////////////////////////////////////////////////////////////////////////
// USE ITEM STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerState_UseItem : public iPlayerState {
private:
	ePlayerMoveState mPrevMoveState;
	ePlayerState mPrevState;

public:
	cPlayerState_UseItem(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void OnStartInteract();
	void OnStartExamine();

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnStartCrouch();
	void OnStopCrouch();

	bool OnStartInventory();
};

//////////////////////////////////////////////////////////////////////////
// MESSAGE STATE
//////////////////////////////////////////////////////////////////////////

class cPlayerState_Message : public iPlayerState {
private:
	ePlayerMoveState mPrevMoveState;
	// ePlayerState mPrevState;

public:
	cPlayerState_Message(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	bool OnJump();

	void OnStartInteract();
	void OnStopInteract();
	void OnStartExamine();

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	bool OnStartInventory();
	bool OnStartInventoryShortCut(int alNum);
};

//////////////////////////////////////////////////////////////////////////
// CLIMB
//////////////////////////////////////////////////////////////////////////
class cGameLadder;
class cPlayerState_Climb : public iPlayerState {
private:
	int mlState;

	float mfTimeCount;

	cVector3f mvCharPosition;

	cVector3f mvGoalPos;
	cVector3f mvGoalRot;

	cVector3f mvPosAdd;
	cVector3f mvRotAdd;

	float mfUpSpeed;
	float mfDownSpeed;

	float mfStepLength;
	float mfStepCount;
	bool mbPlayedSound;

	cVector2f mvPrevPitchLimits;

	float mfLeaveAtTopCount;

	void PlaySound(const tString &asSound);

public:
	cPlayerState_Climb(cInit *apInit, cPlayer *apPlayer);

	void OnUpdate(float afTimeStep);

	void EnterState(iPlayerState *apPrevState);
	void LeaveState(iPlayerState *apNextState);

	void OnStartInteract();
	void OnStartExamine();

	bool OnAddYaw(float afVal);
	bool OnAddPitch(float afVal);

	bool OnMoveForwards(float afMul, float afTimeStep);
	bool OnMoveSideways(float afMul, float afTimeStep);

	void OnStartCrouch();
	void OnStopCrouch();

	bool OnJump();

	bool OnStartInventory();

	cGameLadder *mpLadder;
	cVector3f mvStartPosition;
};

#endif // GAME_PLAYER_STATE_MISC_H
