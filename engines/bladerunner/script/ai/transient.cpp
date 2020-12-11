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

AIScriptTransient::AIScriptTransient(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptTransient::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	Actor_Put_In_Set(kActorTransient, kSetCT03_CT04);
	Actor_Set_At_XYZ(kActorTransient, -171.41f, -621.3f, 736.52f, 580);
	Actor_Set_Goal_Number(kActorTransient, kGoalTransientDefault);
	Actor_Set_Targetable(kActorTransient, true);
}

bool AIScriptTransient::Update() {
	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Which_Set_In(kActorTransient) != kSetFreeSlotG
	) {
		Actor_Put_In_Set(kActorTransient, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorTransient, 39, false);
	}

	if (Global_Variable_Query(kVariableChapter) == 2
	 && (Actor_Query_Goal_Number(kActorTransient) == kGoalTransientDefault
	  || Actor_Query_Goal_Number(kActorTransient) == 10
	 )
	) {
		Actor_Set_Goal_Number(kActorTransient, 200);
	}

	if ( Global_Variable_Query(kVariableChapter) == 3
	 &&  Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)
	 &&  Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)
	 && !Game_Flag_Query(kFlagCT04HomelessBodyFound)
	 && !Game_Flag_Query(kFlagCT04HomelessBodyThrownAway)
	) {
		Game_Flag_Set(kFlagCT04HomelessBodyThrownAway);
	}

	if (Global_Variable_Query(kVariableChapter) < 4
	 && Game_Flag_Query(kFlagCT04HomelessBodyFound)
	 && Actor_Query_Goal_Number(kActorTransient) != 6
	 && Actor_Query_Goal_Number(kActorTransient) != 599
	) {
		Actor_Set_Goal_Number(kActorTransient, 6);
	}

	if ( Player_Query_Current_Scene() == kSceneCT04
	 && !Game_Flag_Query(kFlagCT04HomelessTrashFinish)
	) {
		Game_Flag_Set(kFlagCT04HomelessTrashFinish);
		AI_Countdown_Timer_Reset(kActorTransient, kActorTimerAIScriptCustomTask1);
		AI_Countdown_Timer_Start(kActorTransient, kActorTimerAIScriptCustomTask1, 12);
	}

	return false;
}

void AIScriptTransient::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		if (Actor_Query_Goal_Number(kActorTransient) == 395 && Actor_Query_Which_Set_In(kActorMcCoy) == kSetUG13) {
			AI_Countdown_Timer_Start(kActorTransient, kActorTimerAIScriptCustomTask0, Random_Query(20, 10));
			switch (Random_Query(1, 3)) {
			case 1:
				Sound_Play(kSfxBUMSNOR1, 50, 0, 0, 50);
				break;

			case 2:
				Sound_Play(kSfxBUMSNOR2, 50, 0, 0, 50);
				break;

			case 3:
				Sound_Play(kSfxBUMSNOR3, 50, 0, 0, 50);
				break;
			}
		} else if (Actor_Query_Goal_Number(kActorTransient) != 599) {
			Actor_Set_Goal_Number(kActorTransient, 391);
			AI_Countdown_Timer_Reset(kActorTransient, kActorTimerAIScriptCustomTask0);
		}
	}
	if (timer == kActorTimerAIScriptCustomTask1) {
		if (Actor_Query_Goal_Number(kActorTransient) == kGoalTransientDefault) { // stop diggin the trash
			Actor_Set_Goal_Number(kActorTransient, 10);
			Actor_Change_Animation_Mode(kActorTransient, kAnimationModeIdle);
		}
		Actor_Set_Goal_Number(kActorTransient, 10);
		Actor_Set_Targetable(kActorTransient, false);
		AI_Countdown_Timer_Reset(kActorTransient, kActorTimerAIScriptCustomTask1);
	}
}

void AIScriptTransient::CompletedMovementTrack() {
	//return false;
}

void AIScriptTransient::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTransient::ClickedByPlayer() {
	//return false;
}

void AIScriptTransient::EnteredSet(int setId) {
	// return false;
}

void AIScriptTransient::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptTransient::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptTransient::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTransient::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTransient::ShotAtAndHit() {
	Actor_Set_Frame_Rate_FPS(kActorTransient, 8);
	if (Game_Flag_Query(kFlagUG13HomelessLayingdown)) {
		_animationState = 11;
	} else {
		_animationState = 14;
	}

	_animationFrame = 0;

	Actor_Set_Targetable(kActorTransient, false);
	if (Global_Variable_Query(kVariableChapter) < 4) {
		Actor_Set_Goal_Number(kActorTransient, 3);
	} else {
		Actor_Set_Goal_Number(kActorTransient, 599);
	}

	Game_Flag_Set(kFlagCT04HomelessKilledByMcCoy);

	return false;
}

void AIScriptTransient::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorTransient, 599);

	if (Global_Variable_Query(kVariableChapter) == 4) {
		Game_Flag_Set(kFlagMcCoyRetiredHuman);
	}
}

int AIScriptTransient::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTransient::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalTransientCT04Leave:
		AI_Movement_Track_Flush(kActorTransient);
		AI_Movement_Track_Append(kActorTransient, 51, 0);
		AI_Movement_Track_Append(kActorTransient, 105, 0);
		AI_Movement_Track_Append(kActorTransient, 42, 1);
		AI_Movement_Track_Repeat(kActorTransient);
		return true;

	case 6:
		AI_Movement_Track_Flush(kActorTransient);
		AI_Movement_Track_Append(kActorTransient, 41, 10);
		AI_Movement_Track_Repeat(kActorTransient);
		return true;

	case 200:
		Actor_Put_In_Set(kActorTransient, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorTransient, 40, 0);
		return true;

	case 390:
		// laying on the couch - not sleeping
		Actor_Put_In_Set(kActorTransient, kSetUG13);
		Actor_Set_At_XYZ(kActorTransient, -310.0, 55.0, -350.0, 400);
		Actor_Change_Animation_Mode(kActorTransient, 53);
		Actor_Set_Targetable(kActorTransient, true);
		Game_Flag_Set(kFlagUG13HomelessLayingdown);
		return true;

	case 391:
		// laying on the couch - sleeping - dialogue exhausted pre-flask - awaiting flask
		Actor_Change_Animation_Mode(kActorTransient, 53);
		return true;

	case 395:
		// laying on the couch - sleeping - post flask
		Actor_Change_Animation_Mode(kActorTransient, 55);
		AI_Countdown_Timer_Start(kActorTransient, kActorTimerAIScriptCustomTask0, Random_Query(30, 40));
		return true;

	case 599:
		AI_Countdown_Timer_Reset(kActorTransient, kActorTimerAIScriptCustomTask0);
		return true;
	}
	return false;
}

bool AIScriptTransient::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationTransientPickingNodeAndWiping;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientPickingNodeAndWiping)) {
			_animationFrame = 0;
		} else {
			*animation = kModelAnimationTransientIdle;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientIdle)) {
				_animationFrame = 0;
			}
		}
		break;

	case 1:
		*animation = kModelAnimationTransientWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientWalking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationTransientGestureGive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientGestureGive)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationTransientScratchBackOfHeadTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientScratchBackOfHeadTalk)) {
			*animation = kModelAnimationTransientGestureGive;
			_animationState = 2;
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = kModelAnimationTransientDescriptiveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientDescriptiveTalk)) {
			*animation = kModelAnimationTransientGestureGive;
			_animationState = 2;
			_animationFrame = 0;
		}
		break;

	case 5:
		*animation = kModelAnimationTransientPointingAtTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientPointingAtTalk)) {
			*animation = kModelAnimationTransientGestureGive;
			_animationState = 2;
			_animationFrame = 0;
		}
		break;

	case 6:
		*animation = kModelAnimationTransientLayingIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingIdle)) {
			_animationFrame = 0;
		}
		break;

	case 7:
		*animation = kModelAnimationTransientLayingCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingCalmTalk)) {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;

	case 8:
		*animation = kModelAnimationTransientLayingMoreCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingMoreCalmTalk)) {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;

	case 9:
		*animation = kModelAnimationTransientLayingThisAndThatTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingThisAndThatTalk)) {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;

	case 10:
		*animation = kModelAnimationTransientLayingGestureGiveOrTake;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingGestureGiveOrTake)) {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;

	case 11:
		*animation = kModelAnimationTransientLayingShotDead;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingShotDead)) {
			Actor_Set_Frame_Rate_FPS(kActorTransient, 8);
			_animationState = 12;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingShotDead) - 1;
		}
		break;

	case 12:
		*animation = kModelAnimationTransientLayingShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingShotDead) - 1;
		break;

	case 14:
		*animation = kModelAnimationTransientShotDeadCollapseInPlace;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientShotDeadCollapseInPlace)) {
			Actor_Set_Goal_Number(kActorTransient, 3);
			_animationState = 15;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientShotDeadCollapseInPlace) - 1;
			Actor_Set_Targetable(kActorTransient, false);
			Actor_Retired_Here(kActorTransient, 120, 24, 1, -1);
		}
		break;

	case 15:
		*animation = kModelAnimationTransientShotDeadCollapseInPlace;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientShotDeadCollapseInPlace) - 1;
		break;

	case 16:
		*animation = kModelAnimationTransientSearchingTrash;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientSearchingTrash) - 1) {
			_animationFrame = 0;
		}
		break;

	case 17:
		*animation = kModelAnimationTransientSearchingTrashToIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientSearchingTrashToIdle) - 1) {
			*animation = kModelAnimationTransientIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorTransient, kAnimationModeIdle);
			if (Actor_Query_Goal_Number(kActorTransient) == 10 && Player_Query_Current_Scene() == kSceneCT04) {
				Actor_Set_At_XYZ(kActorTransient, -171.41f, -621.30f, 728.52f, 530);
				Actor_Face_Actor(kActorTransient, kActorMcCoy, true);
			}
		}
		break;

	case 18:
		*animation = kModelAnimationTransientLayingIdle;
		++_animationFrame;
		if (_animationFrame - 1 == 4) {
			_animationState = 19;
		}
		break;

	case 19:
		*animation = kModelAnimationTransientLayingIdle;
		--_animationFrame;
		if (_animationFrame + 1 == 4) {
			_animationState = 18;
		}
		break;
	default:
		// Dummy placeholder, kModelAnimationZubenWalking (399) is a Zuben animation
		*animation = kModelAnimationZubenWalking;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptTransient::ChangeAnimationMode(int mode) {
	if (mode != 55) {
		Actor_Set_Frame_Rate_FPS(kActorTransient, 8);
	}

	switch (mode) {
	case 0:
		switch (_animationState) {
		case 0:
			break;

		case 6:
			// fall through
		case 7:
			// fall through
		case 8:
			// fall through
		case 9:
			// fall through
		case 10:
			// fall through
		case 18:
			// fall through
		case 19:
			_animationState = 6;
			_animationFrame = 0;
			break;

		case 16:
			_animationState = 17;
			_animationFrame = 0;
			break;

		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case 1:
		if (_animationState != 1) {
			_animationState = 1;
			_animationFrame = 0;
		}
		break;

	case 3:
		// fall through
	case 30:
		if (_animationState - 6 > 4) {
			_animationState = 2;
		} else {
			_animationState = 7;
		}
		_animationFrame = 0;
		break;

	case 12:
		// fall through
	case 31:
		if (_animationState == 6) {
			_animationState = 8;
		} else {
			_animationState = 3;
		}
		_animationFrame = 0;
		break;

	case 13:
		// fall through
	case 32:
		if (_animationState == 6) {
			_animationState = 9;
		} else {
			_animationState = 4;
		}
		_animationFrame = 0;
		break;

	case 14:
		// fall through
	case 33:
		// fall through
	case 34:
		// fall through
	case 35:
		// fall through
	case 36:
		// fall through
	case 37:
		if (_animationState == 6) {
			_animationState = 10;
		} else {
			_animationState = 5;
		}
		_animationFrame = 0;
		break;

	case 21:
		if (Game_Flag_Query(kFlagUG13HomelessLayingdown)) {
			_animationState = 11;
		} else {
			_animationState = 14;
		}
		_animationFrame = 0;
		break;

	case 38:
		_animationState = 16;
		_animationFrame = 0;
		break;

	case 53:
		// fall through
	case 54:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 55:
		if (_animationState == 6) {
			Actor_Set_Frame_Rate_FPS(kActorTransient, 4);
			_animationState = 18;
			_animationFrame = 3;
		}
		break;

	case 89:
		_animationState = 12;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationTransientLayingShotDead) - 1;
		break;
	}

	return true;
}

void AIScriptTransient::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTransient::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTransient::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTransient::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
