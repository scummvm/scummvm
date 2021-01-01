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

AIScriptHolloway::AIScriptHolloway(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptHolloway::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;

	Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayDefault);
}

bool AIScriptHolloway::Update() {
	if (Actor_Query_Goal_Number(kActorHolloway) == kGoalHollowayPrepareCaptureMcCoy) {
		Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayCaptureMcCoy);
	}

	return false;
}

void AIScriptHolloway::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		AI_Countdown_Timer_Reset(kActorHolloway, kActorTimerAIScriptCustomTask0);
		if (Global_Variable_Query(kVariableHollowayArrest) == 1) {
			Player_Gains_Control();
		}
		Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayTalkToMcCoy);
	}
}

void AIScriptHolloway::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorHolloway) < 245
	 && Actor_Query_Goal_Number(kActorHolloway) > 239
	) {
		Loop_Actor_Walk_To_Actor(kActorHolloway, kActorMcCoy, 24, false, false);
		Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayApproachMcCoy);
	}
}

void AIScriptHolloway::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHolloway::ClickedByPlayer() {
	//return false;
}

void AIScriptHolloway::EnteredSet(int setId) {
	// return false;
}

void AIScriptHolloway::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptHolloway::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptHolloway::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == kActorMcCoy
	 && Actor_Query_Goal_Number(kActorHolloway) == kGoalHollowayApproachMcCoy
	) {
		AI_Countdown_Timer_Reset(kActorHolloway, kActorTimerAIScriptCustomTask0);
		Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayKnockOutMcCoy);
	}
}

void AIScriptHolloway::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHolloway::ShotAtAndHit() {
	return false;
}

void AIScriptHolloway::Retired(int byActorId) {
	// return false;
}

int AIScriptHolloway::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHolloway::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalHollowayGoToNR07:
		Actor_Put_In_Set(kActorHolloway, kSetNR07);
		Actor_Set_At_XYZ(kActorHolloway, -102.0f, -73.5f, -233.0f, 0);
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorHolloway);
		AI_Movement_Track_Append(kActorHolloway, 336, 1);
		AI_Movement_Track_Repeat(kActorHolloway);
		break;

	case kGoalHollowayGoToNR02:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorHolloway);
		AI_Movement_Track_Append(kActorHolloway, 375, 0);
		AI_Movement_Track_Append(kActorHolloway, 376, 0);
		AI_Movement_Track_Repeat(kActorHolloway);
		break;

	case kGoalHollowayGoToHF03:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorHolloway);
		AI_Movement_Track_Append(kActorHolloway, 372, 0);
		AI_Movement_Track_Repeat(kActorHolloway);
		break;

	case kGoalHollowayApproachMcCoy:
		Scene_Exits_Disable();
		Actor_Says(kActorHolloway, 20, kAnimationModeTalk);
		Actor_Face_Actor(kActorHolloway, kActorMcCoy, true);
		if (Player_Query_Combat_Mode()) {
			Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayKnockOutMcCoy);
		} else {
			Actor_Says(kActorHolloway, 30, kAnimationModeTalk);
			Actor_Face_Actor(kActorMcCoy, kActorHolloway, true);
			AI_Countdown_Timer_Reset(kActorHolloway, kActorTimerAIScriptCustomTask0);
			AI_Countdown_Timer_Start(kActorHolloway, kActorTimerAIScriptCustomTask0, 1);
		}
		break;

	case kGoalHollowayTalkToMcCoy:
		Actor_Face_Actor(kActorMcCoy, kActorHolloway, true);
		Actor_Says(kActorMcCoy, 6130, 15);
		Actor_Says(kActorHolloway, 40, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 6135, 13);
		Actor_Says(kActorHolloway, 50, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 6140, 16);
		Actor_Says(kActorHolloway, 60, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 6145, 12);
		Actor_Says(kActorHolloway, 70, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayKnockOutMcCoy);
		break;

	case kGoalHollowayKnockOutMcCoy:
		Player_Loses_Control();
		Actor_Change_Animation_Mode(kActorHolloway, kAnimationModeCombatAttack);
		break;

	case kGoalHollowayCaptureMcCoy:
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Player_Gains_Control();

		Outtake_Play(kOuttakeInterrogation, false, 1);

		if (Global_Variable_Query(kVariableHollowayArrest) == 1) {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR08ReadyToRun);
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
		}

		Player_Gains_Control();
		Game_Flag_Set(kFlagMcCoyCapturedByHolloway);
		Scene_Exits_Enable();
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR01WaitForMcCoy);
		Actor_Put_In_Set(kActorHolloway, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorHolloway, 41, 0);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		if (Global_Variable_Query(kVariableHollowayArrest) != 1) {
			Player_Gains_Control();
		}

		Game_Flag_Set(kFlagUG03toUG04);
		Set_Enter(kSetUG04, kSceneUG04);
		break;

	default:
		break;
	}

	return false;
}

bool AIScriptHolloway::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationHollowayIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHollowayIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationHollowayGlobAttack;
		++_animationFrame;

		if (_animationFrame == 9) {
			Ambient_Sounds_Play_Sound(kSfxKICK1, 90, 99, 0, 0);
		}

		if (_animationFrame == 10) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		}

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHollowayGlobAttack)) {
			Actor_Change_Animation_Mode(kActorHolloway, kAnimationModeIdle);
			_animationFrame = 0;
			_animationState = 0;
			*animation = kModelAnimationHollowayIdle;
			Actor_Set_Goal_Number(kActorHolloway, kGoalHollowayPrepareCaptureMcCoy);
		}
		break;

	case 2:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationHollowayIdle;
			_animationState = 0;
		} else {
			*animation = kModelAnimationHollowayCalmTalk;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHollowayCalmTalk)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		// fall through
	case 4:
		// fall through
	case 5:
		// fall through
	case 6:
		*animation = kModelAnimationHollowayExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHollowayExplainTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationHollowayCalmTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationHollowayWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHollowayWalking)) {
			_animationFrame = 0;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptHolloway::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState > 6) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_resumeIdleAfterFramesetCompletesFlag = true;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		_animationState = 2;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case kAnimationModeCombatAttack:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 3;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		_animationState = 4;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		_animationState = 5;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 15:
		_animationState = 6;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptHolloway::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHolloway::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHolloway::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHolloway::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
