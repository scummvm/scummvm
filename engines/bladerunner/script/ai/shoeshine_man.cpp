/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bladerunner/script/ai_script.h"

namespace BladeRunner {

AIScriptShoeshineMan::AIScriptShoeshineMan(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_state = false;
}

void AIScriptShoeshineMan::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_state = false;
}

bool AIScriptShoeshineMan::Update() {
	return false;
}

void AIScriptShoeshineMan::TimerExpired(int timer) {
	//return false;
}

void AIScriptShoeshineMan::CompletedMovementTrack() {
	//return false;
}

void AIScriptShoeshineMan::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptShoeshineMan::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorShoeshineMan) != 100) {
		return;
	}

	if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -1098.15f, -0.04f, -201.53f, 0, true, false, false)) {
		Actor_Set_Goal_Number(kActorShoeshineMan, 102);
		Actor_Face_Actor(kActorMcCoy, kActorShoeshineMan, true);
		Actor_Says_With_Pause(kActorShoeshineMan, 0, 1.2f, 13);
		Actor_Says(kActorMcCoy, 730, 17);
		Actor_Says(kActorShoeshineMan, 10, 13);
		Actor_Says(kActorShoeshineMan, 20, 12);
		Actor_Says_With_Pause(kActorMcCoy, 735, 1.0f, 14);
		Actor_Change_Animation_Mode(kActorShoeshineMan, 29);
	}
}

void AIScriptShoeshineMan::EnteredSet(int setId) {
	// return false;
}

void AIScriptShoeshineMan::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptShoeshineMan::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptShoeshineMan::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptShoeshineMan::ShotAtAndMissed() {
	// return false;
}

bool AIScriptShoeshineMan::ShotAtAndHit() {
	return false;
}

void AIScriptShoeshineMan::Retired(int byActorId) {
	// return false;
}

int AIScriptShoeshineMan::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptShoeshineMan::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 100:
		Actor_Put_In_Set(kActorShoeshineMan, kSetDR01_DR02_DR04);
		Actor_Set_At_XYZ(kActorShoeshineMan, -1160.0f, -0.04f, -235.0f, 524);
		break;

	case 101:
		AI_Movement_Track_Flush(kActorShoeshineMan);
		AI_Movement_Track_Append(kActorShoeshineMan, 281, 0);
		AI_Movement_Track_Append(kActorShoeshineMan, 40, 0);
		AI_Movement_Track_Repeat(kActorShoeshineMan);
		break;
	}
	return false;
}

bool AIScriptShoeshineMan::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptShoeshineMan::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 2 && _animationState <= 4) {
			_state = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 1:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 2;
		_animationFrame = 0;
		_state = false;
		break;

	case 12:
		_animationState = 3;
		_animationFrame = 0;
		_state = false;
		break;

	case 13:
		_animationState = 4;
		_animationFrame = 0;
		_state = false;
		break;

	case 23:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 29:
		_animationState = 6;
		_animationFrame = 0;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptShoeshineMan::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptShoeshineMan::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptShoeshineMan::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptShoeshineMan::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
