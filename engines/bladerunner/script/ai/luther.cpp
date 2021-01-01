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

AIScriptLuther::AIScriptLuther(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptLuther::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;

	Actor_Put_In_Set(kActorLuther, kSetUG16);
	Actor_Set_At_XYZ(kActorLuther, 176.91f, -40.67f, 225.92f, 486);
	Actor_Set_Goal_Number(kActorLuther, kGoalLutherDefault);
	Actor_Set_Targetable(kActorLuther, true);
}

bool AIScriptLuther::Update() {
	if ( Actor_Query_Is_In_Current_Set(kActorLuther)
	 &&  Player_Query_Combat_Mode()
	 &&  Global_Variable_Query(kVariableLutherLanceShot) == 0
	 && !Game_Flag_Query(kFlagUG16PulledGun)
	 &&  Global_Variable_Query(kVariableChapter) == 4
	) {
		Actor_Says(kActorMcCoy, 5720, 12);
		Actor_Says(kActorLuther, 80, 13);
		Actor_Says(kActorLance, 40, 12);
		Game_Flag_Set(kFlagUG16PulledGun);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorLuther) == kGoalLutherDefault
	 && Actor_Query_Goal_Number(kActorLuther) != kGoalLutherDead // A bug? this is redundant
	) {
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherMoveAround);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorLuther) == kGoalLutherShot) {
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDyingStarted);
		ChangeAnimationMode(kAnimationModeDie);
		return false;
	}

	if ( Actor_Query_Goal_Number(kActorLuther) == kGoalLutherDyingStarted
	 && !Game_Flag_Query(kFlagUG15LutherLanceStartedDying)
	) {
		AI_Countdown_Timer_Reset(kActorLuther, kActorTimerAIScriptCustomTask2);
		AI_Countdown_Timer_Start(kActorLuther, kActorTimerAIScriptCustomTask2, 5);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDyingWait);
		Game_Flag_Set(kFlagUG15LutherLanceStartedDying);
		return false;
	}

	if ( Actor_Query_Goal_Number(kActorLuther) == kGoalLutherDyingCheck
	 &&  Global_Variable_Query(kVariableLutherLanceShot) < 2
	 && !Game_Flag_Query(kFlagUG16ComputerOff)
	) {
		Game_Flag_Set(kFlagUG16ComputerOff);
		ChangeAnimationMode(50);
		ChangeAnimationMode(kAnimationModeDie);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDie);
		Actor_Set_Targetable(kActorLuther, false);
		Scene_Loop_Set_Default(5); // UG16MainLoopNoComputerLight
		Scene_Loop_Start_Special(kSceneLoopModeOnce, 4, true); // UG16SparkLoop
		Ambient_Sounds_Play_Sound(kSfxCOMPDWN4, 50, 0, 0, 99);
		Ambient_Sounds_Remove_Looping_Sound(kSfxELECLAB1, 1);
		return false;
	}

	if ( Actor_Query_Goal_Number(kActorLuther) == kGoalLutherDyingCheck
	 &&  Global_Variable_Query(kVariableLutherLanceShot) > 1
	 && !Game_Flag_Query(kFlagUG16ComputerOff)
	) {
		Actor_Set_Targetable(kActorLuther, false);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDie);
		Actor_Set_Targetable(kActorLuther, false);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorLuther) == kGoalLutherDie) {
		Game_Flag_Set(kFlagLutherLanceAreDead);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDead);
		Actor_Set_Targetable(kActorLuther, false);
		return false;
	}

	return false;
}

void AIScriptLuther::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorLuther, kActorTimerAIScriptCustomTask2);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDyingCheck);
		// return true;
	}
	// return false;
}

void AIScriptLuther::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorLuther) == kGoalLutherMoveAround) {
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherMoveAroundRestart);
		// return true;
	}
	//return false;
}

void AIScriptLuther::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptLuther::ClickedByPlayer() {
	//return false;
}

void AIScriptLuther::EnteredSet(int setId) {
	// return false;
}

void AIScriptLuther::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptLuther::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptLuther::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptLuther::ShotAtAndMissed() {
	// return false;
}

bool AIScriptLuther::ShotAtAndHit() {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Actor_Query_In_Set(kActorLuther, kSetKP07)) {
		AI_Movement_Track_Flush(kActorLuther);
		ChangeAnimationMode(kAnimationModeDie);
		Actor_Retired_Here(kActorLuther, 6, 6, true, kActorMcCoy);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherDie);
		return false;
	}
#endif
	if (Actor_Query_Which_Set_In(kActorLuther) == kSetUG16) {
		Actor_Set_Health(kActorLuther, 50, 50);
	}

	Global_Variable_Increment(kVariableLutherLanceShot, 1);
	Music_Stop(2);

	if (Global_Variable_Query(kVariableLutherLanceShot) > 0) {
		if (!Game_Flag_Query(kFlagLutherLanceIsReplicant)) {
			Game_Flag_Set(kFlagNotUsed557);
		}
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherShot);
		return true;
	}

	return false;
}

void AIScriptLuther::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorLuther, kGoalLutherGone);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	if (Actor_Query_In_Set(kActorLuther, kSetKP07)) {
		Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1);
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherGone);

		if (Global_Variable_Query(kVariableReplicantsSurvivorsAtMoonbus) == 0) {
			Player_Loses_Control();
			Delay(2000);
			Player_Set_Combat_Mode(false);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, false);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP07toKP06);
			Game_Flag_Reset(kFlagMcCoyIsHelpingReplicants);
			Set_Enter(kSetKP05_KP06, kSceneKP06);
			return; //true;
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
}

int AIScriptLuther::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptLuther::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalLutherMoveAround:
		AI_Movement_Track_Flush(kActorLuther);
		AI_Movement_Track_Append(kActorLuther, 39, 20);
		AI_Movement_Track_Append_With_Facing(kActorLuther, 368, 120, 486);
		AI_Movement_Track_Append(kActorLuther, 40, 10);
		AI_Movement_Track_Repeat(kActorLuther);
		break;

	case kGoalLutherMoveAroundRestart:
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherMoveAround);
		break;

	case kGoalLutherStop:
		AI_Movement_Track_Flush(kActorLuther);
		break;

	case kGoalLutherDead:
		Actor_Set_Goal_Number(kActorLuther, kGoalLutherGone);
		break;
	}

	return false;
}

bool AIScriptLuther::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationTwinsSitIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationTwinsSitLancePutsSomethingToTheLeft;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationTwinsSitIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorLuther, kAnimationModeIdle);
		}
		break;

	case 2:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationTwinsSitIdle;
			_animationState = 0;
		} else {
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = kModelAnimationTwinsSitLanceLongerCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
		}
		break;

	case 4:
		*animation = kModelAnimationTwinsSitLutherCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationTwinsSitLutherMoreCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
		}
		break;

	case 6:
		*animation = kModelAnimationTwinsSitLanceMoreCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationTwinsSitLutherProtestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationTwinsSitLutherGoAheadTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationTwinsSitLanceShortCalmTalk;
		}
		break;

	case 9:
		*animation = kModelAnimationTwinsSitLutherHitsOrFeedsLance;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationTwinsSitIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorLuther, kAnimationModeIdle);
		}
		break;

	case 10:
		*animation = kModelAnimationTwinsSitDropForwards;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorLuther, 50);
			*animation = kModelAnimationTwinsSitAlmostDeadLutherPushesButton;
			_animationFrame = 0;
		}
		break;

	case 11:
		*animation = kModelAnimationTwinsSitAlmostDeadLutherPushesButton;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			++_animationFrame;
		}
		break;

	case 12:
		*animation = kModelAnimationTwinsSitDieCompletely;
		if (_animationFrame == 12) {
			Ambient_Sounds_Play_Sound(kSfxHEADHIT2, 59, 0, 0, 20);
		}
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			++_animationFrame;
		}
		break;

	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptLuther::ChangeAnimationMode(int mode) {
	// these modes are differnent that other actors
	switch (mode) {
	case 0:
		if ((unsigned int)(_animationState - 2) > 6) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_resumeIdleAfterFramesetCompletesFlag = true;
		}
		break;

	case 3:
		_animationState = 2;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 6:
		_animationState = 9;
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

	case 16:
		_animationState = 7;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 17:
		_animationState = 8;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 23:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 12;
		_animationFrame = 0;
		break;

	case 50:
		_animationState = 11;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptLuther::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptLuther::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptLuther::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptLuther::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
