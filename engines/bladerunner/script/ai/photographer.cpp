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

AIScriptPhotographer::AIScriptPhotographer(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var2 = 0; // is always set to 0, never checked, unused
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptPhotographer::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_varNumOfTimesToHoldCurrentFrame = 0;
	_var2 = 0;
	_resumeIdleAfterFramesetCompletesFlag = false;
}

bool AIScriptPhotographer::Update() {
	if ( Game_Flag_Query(kFlagTB02ElevatorToTB05)
	 && !Game_Flag_Query(kFlagTB06Photographer)
	) {
		Actor_Put_In_Set(kActorPhotographer, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorPhotographer, 35, 0);
		Game_Flag_Set(kFlagTB06Photographer);
		Actor_Set_Goal_Number(kActorPhotographer, 100);

		return true;
	}

	return false;
}

void AIScriptPhotographer::TimerExpired(int timer) {
	//return false;
}

void AIScriptPhotographer::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorPhotographer)) {
	case 100:
		Actor_Set_Goal_Number(kActorPhotographer, 101);
		break;

	case 101:
		Actor_Set_Goal_Number(kActorPhotographer, 102);
		break;

	case 102:
		Actor_Set_Goal_Number(kActorPhotographer, 101);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptPhotographer::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptPhotographer::ClickedByPlayer() {
	if ( Actor_Clue_Query(kActorMcCoy, kClueDragonflyEarring)
	 && !Actor_Clue_Query(kActorMcCoy, kClueVictimInformation)
	 && !Game_Flag_Query(kFlagTB06PhotographTalk1)
	) {
		AI_Movement_Track_Pause(kActorPhotographer);
		Actor_Face_Actor(kActorMcCoy, kActorPhotographer, true);
		Actor_Says(kActorMcCoy, 5300, 14);
		Actor_Face_Actor(kActorPhotographer, kActorMcCoy, true);
		Actor_Says(kActorPhotographer, 20, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 5305, 15);
		Game_Flag_Set(kFlagTB06PhotographTalk1);
		AI_Movement_Track_Unpause(kActorPhotographer);
	} else {
		AI_Movement_Track_Pause(kActorPhotographer);
		Actor_Face_Actor(kActorMcCoy, kActorPhotographer, true);
		Actor_Face_Actor(kActorPhotographer, kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 5310, 11);
		Actor_Says(kActorPhotographer, 40, kAnimationModeTalk);
		AI_Movement_Track_Unpause(kActorPhotographer);
	}
}

void AIScriptPhotographer::EnteredSet(int setId) {
	// return false;
}

void AIScriptPhotographer::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptPhotographer::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptPhotographer::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptPhotographer::ShotAtAndMissed() {
	// return false;
}

bool AIScriptPhotographer::ShotAtAndHit() {
	return false;
}

void AIScriptPhotographer::Retired(int byActorId) {
	// return false;
}

int AIScriptPhotographer::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptPhotographer::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 100:
		AI_Movement_Track_Flush(kActorPhotographer);
		AI_Movement_Track_Append(kActorPhotographer, 35, 0);
		AI_Movement_Track_Repeat(kActorPhotographer);
		return true;

	case 101:
		AI_Movement_Track_Flush(kActorPhotographer);
		switch (Random_Query(1, 3)) {
		case 1:
			AI_Movement_Track_Append(kActorPhotographer, 280, 4);
			break;

		case 2:
			AI_Movement_Track_Append(kActorPhotographer, 279, 8);
			break;

		case 3:
			AI_Movement_Track_Append(kActorPhotographer, 280, 3);
			break;
		}
		AI_Movement_Track_Repeat(kActorPhotographer);
		return false;

	case 102:
		AI_Movement_Track_Flush(kActorPhotographer);
		AI_Movement_Track_Append(kActorPhotographer, 279, 5);
		AI_Movement_Track_Repeat(kActorPhotographer);
		return true;

	case 199:
		Actor_Put_In_Set(kActorPhotographer, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorPhotographer, 35, 0);
		Actor_Put_In_Set(kActorMarcus, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorMarcus, 41, 0);
		if (Game_Flag_Query(kFlagTB06Visited)) {
			Item_Remove_From_World(kItemDeadDogA);
			Item_Remove_From_World(kItemDeadDogB);
			Item_Remove_From_World(kItemDeadDogC);
		}
		if (!Actor_Clue_Query(kActorMcCoy, kClueDogCollar1)) {
			Actor_Clue_Acquire(kActorSteele, kClueDogCollar1, true, -1);
			if (Game_Flag_Query(kFlagTB06Visited)) {
				Item_Remove_From_World(kItemDogCollar);
			}
			Global_Variable_Increment(kVariableMcCoyEvidenceMissed, 1);
		}
		return true;

	default:
		return true;
	}
}

bool AIScriptPhotographer::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationPhotographerIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationPhotographerWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerWalking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationPhotographerCalmTalk;
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationPhotographerIdle;
			_animationState = 0;
			_var2 = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerCalmTalk)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
#if BLADERUNNER_ORIGINAL_BUGS
		// TODO A bug? This is identical to case 4 for animation 749, but 748 talk animation is left unused
		*animation = kModelAnimationPhotographerExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerExplainTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationPhotographerCalmTalk;
		}
#else
		*animation = kModelAnimationPhotographerMoreHeadMoveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerMoreHeadMoveTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationPhotographerCalmTalk;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case 4:
		*animation = kModelAnimationPhotographerExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerExplainTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationPhotographerCalmTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationPhotographerSuggestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerSuggestTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationPhotographerCalmTalk;
		}
		break;

	case 6:
		*animation = kModelAnimationPhotographerTakingAPhoto;
		if (_animationFrame == 11) {
			Ambient_Sounds_Play_Sound(kSfxCAMCOP1, 80, -20, -20, 20);
		}
		if (_varNumOfTimesToHoldCurrentFrame) {
			--_varNumOfTimesToHoldCurrentFrame;
		} else {
			++_animationFrame;
			if (_animationFrame == 10) {
				_varNumOfTimesToHoldCurrentFrame = 5;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationPhotographerTakingAPhoto)) {
				*animation = kModelAnimationPhotographerIdle;
				_animationState = 0;
				_animationFrame = 0;
				Actor_Change_Animation_Mode(kActorPhotographer, kAnimationModeIdle);
			}
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptPhotographer::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState < 2 || _animationState > 5) {
			_animationState = 0;
			_var2 = 0;
			_animationFrame = 0;
		} else {
			_resumeIdleAfterFramesetCompletesFlag = true;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 1;
		_var2 = 0;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		_animationState = 2;
		_var2 = 0;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 12:
		_animationState = 3;
		_var2 = 0;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		_animationState = 4;
		_var2 = 0;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		_animationState = 5;
		_var2 = 0;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 43:
		_animationState = 6;
		_animationFrame = 0;
		break;

	default:
		break;
	}
	return true;
}

void AIScriptPhotographer::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptPhotographer::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptPhotographer::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 276
	 || waypointId == 278
	 || waypointId == 280
	) {
		ChangeAnimationMode(43);
	}

	return true;
}

void AIScriptPhotographer::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
