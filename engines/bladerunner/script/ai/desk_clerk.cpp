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

AIScriptDeskClerk::AIScriptDeskClerk(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1
	_varChooseIdleAnimation = 0;
	_resumeIdleAfterFramesetCompletesFlag = false;
	_varNumOfTimesToHoldCurrentFrame = 75;
}

void AIScriptDeskClerk::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_varChooseIdleAnimation = 0;
	_resumeIdleAfterFramesetCompletesFlag = false;
	_varNumOfTimesToHoldCurrentFrame = 75;
	Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkDefault);
}

bool AIScriptDeskClerk::Update() {
	if (Actor_Query_Goal_Number(kActorDeskClerk) == kGoalDeskClerkKnockedOut
	 && Player_Query_Current_Set() != kSetCT01_CT12
	 && Player_Query_Current_Set() != kSetCT03_CT04
	 && Player_Query_Current_Set() != kSetCT08_CT51_UG12
	 && Player_Query_Current_Set() != kSetCT02
	 && Player_Query_Current_Set() != kSetCT05
	 && Player_Query_Current_Set() != kSetCT06
	 && Player_Query_Current_Set() != kSetCT07
	 && Player_Query_Current_Set() != kSetCT09
	 && Player_Query_Current_Set() != kSetCT10
	 && Player_Query_Current_Set() != kSetCT11
	) {
		Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkRecovered);
	}

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorDeskClerk) < kGoalDeskClerkGone
	) {
		Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkGone);
	}

	return false;
}

void AIScriptDeskClerk::TimerExpired(int timer) {
	//return false;
}

void AIScriptDeskClerk::CompletedMovementTrack() {
	//return false;
}

void AIScriptDeskClerk::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptDeskClerk::ClickedByPlayer() {
	//return false;
}

void AIScriptDeskClerk::EnteredSet(int setId) {
	// return false;
}

void AIScriptDeskClerk::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptDeskClerk::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptDeskClerk::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptDeskClerk::ShotAtAndMissed() {
	// return false;
}

bool AIScriptDeskClerk::ShotAtAndHit() {
	return false;
}

void AIScriptDeskClerk::Retired(int byActorId) {
	// return false;
}

int AIScriptDeskClerk::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptDeskClerk::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalDeskClerkDefault:
		// fall through
	case kGoalDeskClerkRecovered:
		Actor_Put_In_Set(kActorDeskClerk, kSetCT09);
		Actor_Set_At_XYZ(kActorDeskClerk, 282.0f, 360.52f, 743.0f, 513);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Change_Animation_Mode(kActorDeskClerk, kAnimationModeIdle);
		_animationFrame = 0;
		_animationState = 0;
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case kGoalDeskClerkKnockedOut:
		// fall through
	case kGoalDeskClerkGone:
		Actor_Put_In_Set(kActorDeskClerk, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorDeskClerk, 40, 0);
		break;
	}
	return false;
}

bool AIScriptDeskClerk::UpdateAnimation(int *animation, int *frame) {

#if BLADERUNNER_ORIGINAL_BUGS
#else
	// Fixing a bug for when the Clerk gets stuck in animation id kModelAnimationDeskClerkIsHeldUpByLeonIdle (668), after Act 3:
	//	- when using HDFRAMES, the clerk will briefly be in the choking animation when McCoy re-enters
	//	- when using CDFRAMES, the game would crash with a message:
	//   "Unable to locate page 2214 for animation 668 frame 4!"
	// This occurs when:
	//	 The player walks out too fast from the scene where Leon is choking the clerk in Act 3.
	//   Hence, Leon's AI script's OtherAgentExitedThisSet() is triggered, Leon is gone,
	//   and DeskClerk goal is set to kGoalDeskClerkKnockedOut which puts him in kSetFreeSlotH without changing his animation id.
	// Thus later on, when the player leaves Chinatown and returns, DeskClerk's (update()) will set his goal to kGoalDeskClerkRecovered
	// In Act 4, the CDFRAMES#.DAT method loads a reduced number of animations for DeskClerk causing the crash when McCoy visits the Yukon lobby.
	//
	// The following fix will work with awry saved games too (even from the original game in theory),
	// that have this buggy state stored.
	// We also include the rest of the problematic states that are missing animations in Act 4
	// (ie. all _animationState >= 6)
	if (Global_Variable_Query(kVariableChapter) > 3
	    && _animationState >= 6
	) {
		Actor_Change_Animation_Mode(kActorDeskClerk, kAnimationModeIdle);
		*animation = kModelAnimationDeskClerkReadPaperIdle;
		_animationFrame = 0;
		_animationState = 0;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation > 0) {
			*animation = kModelAnimationDeskClerkReadPaperChangePageIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkReadPaperChangePageIdle)) {
				_animationFrame = 0;
				_varChooseIdleAnimation = 0;
				*animation = kModelAnimationDeskClerkReadPaperIdle;
				_varNumOfTimesToHoldCurrentFrame = Random_Query(50, 100);
			}
		} else {
			if (_varNumOfTimesToHoldCurrentFrame != 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			}

			*animation = kModelAnimationDeskClerkReadPaperIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkReadPaperIdle)) {
				_animationFrame = 0;

				if (_varNumOfTimesToHoldCurrentFrame == 0) {
					*animation = kModelAnimationDeskClerkReadPaperChangePageIdle;
					_varChooseIdleAnimation = 1;
				}
			}
		}
		break;

	case 1:
		*animation = kModelAnimationDeskClerkReadPaperCalmTalk;

		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationDeskClerkReadPaperIdle;
			_animationState = 0;
			_varChooseIdleAnimation = 0;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 2:
		*animation = kModelAnimationDeskClerkReadPaperMoreCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkReadPaperMoreCalmTalk)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationDeskClerkReadPaperCalmTalk;
		}
		break;

	case 3:
		*animation = kModelAnimationDeskClerkReadPaperMoveLeftTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkReadPaperMoveLeftTalk)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationDeskClerkReadPaperCalmTalk;
		}
		break;

	case 4:
		*animation = kModelAnimationDeskClerkReadPaperSlightClosePaperTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkReadPaperSlightClosePaperTalk)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationDeskClerkReadPaperCalmTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationDeskClerkReadPaperMoreClosePaperTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkReadPaperMoreClosePaperTalk)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationDeskClerkReadPaperCalmTalk;
		}
		break;

	case 6:
		*animation = kModelAnimationDeskClerkIsHeldUpByLeonIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkIsHeldUpByLeonIdle)) {
			_animationFrame = 0;
		}
		break;

	case 7:
		*animation = kModelAnimationDeskClerkFallingAfterLeonLetsGo;

		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			Actor_Change_Animation_Mode(kActorDeskClerk, 72);
			*animation = kModelAnimationDeskClerkIsHeldUpByLeonIdle;
			_animationState = 6;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 8:
		*animation = kModelAnimationDeskClerkHeadSmashedOnCounter;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationDeskClerkHeadSmashedOnCounter) - 2) {
			Ambient_Sounds_Play_Sound(kSfxZUBLAND1, 40, 30, 30, 99);
			Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkKnockedOut);
			Actor_Change_Animation_Mode(kActorDeskClerk, kAnimationModeIdle);
			*animation = kModelAnimationDeskClerkReadPaperIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	}

	*frame = _animationFrame;
	return true;
}

bool AIScriptDeskClerk::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 0:
			_animationState = 8;
			_animationFrame = 0;
			break;

		case 1:
			// fall through
		case 2:
			// fall through
		case 3:
			// fall through
		case 4:
			// fall through
		case 5:
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		case 6:
			Actor_Change_Animation_Mode(kActorDeskClerk, 72);
			break;

		default:
			_animationState = 0;
			_animationFrame = 0;
			_varChooseIdleAnimation = 0;
			_varNumOfTimesToHoldCurrentFrame = Random_Query(70, 140);
			break;
		}
		break;

	case kAnimationModeTalk:
		_animationState = 1;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 12:
		_animationState = 2;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		_animationState = 3;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		_animationState = 4;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 15:
		_animationState = 5;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 26:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case 58:
		_animationState = 7;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 72:
		if (_animationState != 6) {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;
	}

	return true;
}

void AIScriptDeskClerk::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptDeskClerk::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptDeskClerk::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptDeskClerk::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
