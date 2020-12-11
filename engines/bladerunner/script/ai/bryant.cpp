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

AIScriptBryant::AIScriptBryant(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptBryant::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	World_Waypoint_Set(324, kSetBB05,       267.0f,  -60.3f,    203.0f);
	World_Waypoint_Set(325, kSetBB05,        84.0f,  -60.3f,    337.0f);
	World_Waypoint_Set(326, kSetBB06_BB07,  -36.0f,    0.0f,    185.0f);
	World_Waypoint_Set(327, kSetBB06_BB07, -166.0f,    0.0f,   -103.0f);
	World_Waypoint_Set(328, kSetBB07,      -556.0f, 252.59f, -1018.11f);
	World_Waypoint_Set(329, kSetBB07,      -655.0f,  252.6f,  -1012.0f);
	World_Waypoint_Set(330, kSetBB07,      -657.0f,  253.0f,  -1127.0f);
	World_Waypoint_Set(331, kSetBB12,       163.8f,    0.0f,     67.0f);
	World_Waypoint_Set(332, kSetBB06_BB07,  -39.0f,    0.0f,     11.5f);
	World_Waypoint_Set(333, kSetBB12,       -34.0f,    0.0f,     33.0f);
	World_Waypoint_Set(334, kSetBB05,         3.0f, -60.30f,   -144.0f);
	World_Waypoint_Set(335, kSetBB12,       -50.0f,    0.0f,    212.0f);

	Actor_Put_In_Set(kActorBryant, kSetBB05);

	Actor_Set_Goal_Number(kActorBryant, 100);
}

bool AIScriptBryant::Update() {
	if (Global_Variable_Query(kVariableChapter) == 2
	 && Actor_Query_Goal_Number(kActorBryant) <= 101
	 && Player_Query_Current_Scene() == kSceneBB05
	) {
		Actor_Set_Goal_Number(kActorBryant, 101);
		return true;
	}

	if ( Global_Variable_Query(kVariableChapter) == 3
	 &&  Player_Query_Current_Scene() == kSceneBB05
	 && !Game_Flag_Query(kFlagNotUsed686)
	) {
		Game_Flag_Set(kFlagNotUsed686);
		return true;
	}
	return false;
}

void AIScriptBryant::TimerExpired(int timer) {
	//return false;
}

void AIScriptBryant::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorBryant) == 101) {
		Actor_Set_Goal_Number(kActorBryant, 102);
		// return true;
	} else if (Actor_Query_Goal_Number(kActorBryant) == 102) {
		Actor_Set_Goal_Number(kActorBryant, 101);
		// return true;
	}
	// return false;
}

void AIScriptBryant::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptBryant::ClickedByPlayer() {
	Actor_Face_Actor(kActorMcCoy, kActorBryant, true);
	Actor_Voice_Over(30, kActorVoiceOver);
	Actor_Voice_Over(40, kActorVoiceOver);
}

void AIScriptBryant::EnteredSet(int setId) {
	// return false;
}

void AIScriptBryant::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptBryant::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptBryant::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptBryant::ShotAtAndMissed() {
	// return false;
}

bool AIScriptBryant::ShotAtAndHit() {
	AI_Movement_Track_Flush(kActorBryant);
	Sound_Play(kSfxFEMHURT1, 100, 0, 0, 50);  // a bug?

	return false;
}

void AIScriptBryant::Retired(int byActorId) {
	// return false;
}

int AIScriptBryant::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptBryant::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 101:
		AI_Movement_Track_Flush(kActorBryant);

		switch (Random_Query(0, 9)) {
		case 0:
			AI_Movement_Track_Append(kActorBryant, 134, 0);
			AI_Movement_Track_Append(kActorBryant, 135, 0);
			AI_Movement_Track_Append(kActorBryant, 136, 0);
			AI_Movement_Track_Append(kActorBryant, 137, 0);
			AI_Movement_Track_Append(kActorBryant, 138, 0);
			AI_Movement_Track_Append(kActorBryant, 139, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 1:
			AI_Movement_Track_Append(kActorBryant, 139, 0);
			AI_Movement_Track_Append(kActorBryant, 138, 0);
			AI_Movement_Track_Append(kActorBryant, 137, 0);
			AI_Movement_Track_Append(kActorBryant, 136, 0);
			AI_Movement_Track_Append(kActorBryant, 135, 0);
			AI_Movement_Track_Append(kActorBryant, 134, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 2:
			AI_Movement_Track_Append(kActorBryant, 196, 0);
			AI_Movement_Track_Append(kActorBryant, 197, 0);
			AI_Movement_Track_Append(kActorBryant, 198, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 3:
			AI_Movement_Track_Append(kActorBryant, 198, 0);
			AI_Movement_Track_Append(kActorBryant, 197, 0);
			AI_Movement_Track_Append(kActorBryant, 196, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 4:
			AI_Movement_Track_Append(kActorBryant, 326, 0);
			AI_Movement_Track_Append(kActorBryant, 332, 0);
			AI_Movement_Track_Append(kActorBryant, 327, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 5:
			AI_Movement_Track_Append(kActorBryant, 327, 0);
			AI_Movement_Track_Append(kActorBryant, 332, 0);
			AI_Movement_Track_Append(kActorBryant, 326, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 6:
			AI_Movement_Track_Append(kActorBryant, 329, 0);
			AI_Movement_Track_Append(kActorBryant, 328, 0);
			AI_Movement_Track_Append(kActorBryant, 330, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 7:
			AI_Movement_Track_Append(kActorBryant, 330, 0);
			AI_Movement_Track_Append(kActorBryant, 328, 0);
			AI_Movement_Track_Append(kActorBryant, 329, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 8:
			AI_Movement_Track_Append(kActorBryant, 331, 0);
			AI_Movement_Track_Append(kActorBryant, 333, 0);
			AI_Movement_Track_Append(kActorBryant, 335, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		case 9:
			AI_Movement_Track_Append(kActorBryant, 335, 0);
			AI_Movement_Track_Append(kActorBryant, 333, 0);
			AI_Movement_Track_Append(kActorBryant, 331, 0);
			AI_Movement_Track_Repeat(kActorBryant);
			break;

		default:
			break;
		}
		// fallthrough
	case 102:
		AI_Movement_Track_Append(kActorBryant, 39, 0);
		AI_Movement_Track_Repeat(kActorBryant);
		break;

	default:
		break;
	}

	return false;
}

bool AIScriptBryant::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationBearToyOscillatingIdle;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBearToyOscillatingIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationBearToyWalking;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBearToyWalking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationBearToyShotDead;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBearToyShotDead) - 1) {
			_animationState = 3;
		}
		break;

	case 3:
		*animation = kModelAnimationBearToyShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationBearToyShotDead) - 1;
		break;

	default:
		break;
	}

	*frame = _animationFrame;
	return true;
}

bool AIScriptBryant::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = 0;
		_animationFrame = 0;
		break;

	case kAnimationModeWalk:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 2;
		_animationFrame = 0;
		Actor_Set_Goal_Number(kActorBryant, 0);
		break;

	default:
		break;
	}
	return true;
}

void AIScriptBryant::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptBryant::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptBryant::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptBryant::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
