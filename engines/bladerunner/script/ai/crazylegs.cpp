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


enum kCrazylegsStates {
	kCrazylegsStateIdle       = 0,
	kCrazylegsStateHandsUp    = 1,
	kCrazylegsStateMobileCall = 2
	// TODO fill in the rest of the animationStates
};

AIScriptCrazylegs::AIScriptCrazylegs(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptCrazylegs::Initialize() {
	_animationFrame = 0;
	_animationState = kCrazylegsStateIdle;
	_animationStateNext = kCrazylegsStateIdle;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;

	World_Waypoint_Set(360, kSetHF05, -103.0f, 40.63f, -53.0f);
	Actor_Put_In_Set(kActorCrazylegs, kSetHF05);
	Actor_Set_At_XYZ(kActorCrazylegs, -33.0f, 40.63f, 16.0f, 845);
}

bool AIScriptCrazylegs::Update() {
	if (Actor_Query_Goal_Number(kActorCrazylegs) == 10)
		Actor_Set_Goal_Number(kActorCrazylegs, 11);

	return false;
}

void AIScriptCrazylegs::TimerExpired(int timer) {
	//return false;
}

void AIScriptCrazylegs::CompletedMovementTrack() {
	//return false;
}

void AIScriptCrazylegs::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptCrazylegs::ClickedByPlayer() {
	//return false;
}

void AIScriptCrazylegs::EnteredSet(int setId) {
	// return false;
}

void AIScriptCrazylegs::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptCrazylegs::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptCrazylegs::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Goal_Number(kActorCrazylegs) != kGoalCrazyLegsLeavesShowroom && otherActorId == kActorMcCoy) {
		if (combatMode && Global_Variable_Query(kVariableChapter) < 5) {
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
			Actor_Says(kActorCrazylegs, 430, 3);
			Actor_Says_With_Pause(kActorCrazylegs, 440, 0.0f, 3);
			Actor_Says(kActorMcCoy, 1870, -1);
			Actor_Says(kActorCrazylegs, 450, 3);
			Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsMcCoyDrewHisGun);
		} else if (Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsMcCoyDrewHisGun) {
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
			Actor_Says(kActorCrazylegs, 460, 3);
			Actor_Says(kActorCrazylegs, 470, 3);
			Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsLeavesShowroom);
		}
	}
}

void AIScriptCrazylegs::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsLeavesShowroom)
		return;

	Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsLeavesShowroom);
}

bool AIScriptCrazylegs::ShotAtAndHit() {
	Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsShotAndHit);
	Actor_Says(kActorMcCoy, 1875, 4);  // I wouldn't drag that bucket of bolts if you paid me.
	return false;
}

void AIScriptCrazylegs::Retired(int byActorId) {
	// return false;
}

int AIScriptCrazylegs::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptCrazylegs::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalCrazyLegsDefault) {
		return true;
	}

	if (newGoalNumber == kGoalCrazyLegsShotAndHit) {
		AI_Movement_Track_Flush(kActorCrazylegs);
		Actor_Set_Targetable(kActorCrazylegs, false);
		return true;
	}

	if (newGoalNumber == kGoalCrazyLegsLeavesShowroom) {
		AI_Movement_Track_Flush(kActorCrazylegs);
		AI_Movement_Track_Append(kActorCrazylegs, 360, 0);
		AI_Movement_Track_Append(kActorCrazylegs, 40, 0);
		AI_Movement_Track_Repeat(kActorCrazylegs);
		return true;
	}

	return false;
}

bool AIScriptCrazylegs::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kCrazylegsStateIdle:
		*animation = kModelAnimationCrazylegsIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsIdle))
			_animationFrame = 0;
		break;

	case kCrazylegsStateHandsUp:
		*animation = kModelAnimationCrazylegsHandsUpIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHandsUpIdle))
			_animationFrame = 0;
		break;

	case kCrazylegsStateMobileCall:
		*animation = kModelAnimationCrazylegsMobileIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsMobileIdle))
			_animationFrame = 0;
		break;

	case 3:
		*animation = kModelAnimationCrazylegsGestureGive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsGestureGive)) {
			*animation = kModelAnimationCrazylegsIdle;
			_animationFrame = 0;
			_animationState = kCrazylegsStateIdle;
		}
		break;

	case 4:
		*animation = kModelAnimationCrazylegsRollSlow;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsRollSlow))
			_animationFrame = 0;
		break;

	case 5:
		*animation = kModelAnimationCrazylegsRollFast;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsRollFast))
			_animationFrame = 0;
		break;

	case 6:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationCrazylegsIdle;
			_animationState = kCrazylegsStateIdle;
			_resumeIdleAfterFramesetCompletesFlag = false;
		} else {
			*animation = kModelAnimationCrazylegsCalmTalk;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsCalmTalk))
				_animationFrame = 0;
		}
		break;

	case 7:
		*animation = kModelAnimationCrazylegsSmallHandMoveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsSmallHandMoveTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationCrazylegsPointingAtSelfTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsPointingAtSelfTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 9:
		*animation = kModelAnimationCrazylegsDisagreeTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsDisagreeTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 10:
		*animation = kModelAnimationCrazylegsFastTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsFastTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 11:
		*animation = kModelAnimationCrazylegsProtestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsProtestTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 12:
		*animation = kModelAnimationCrazylegsMobileCalmTalk;
		++_animationFrame;
		// This is probably an untriggered animation
		// TODO This animation has a faulty last frame which "breaks" its looping consistency
		// TODO It should not lead to kModelAnimationCrazylegsCalmTalk but to one of Crazylegs mobile animations
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsMobileCalmTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 13:
		*animation = kModelAnimationCrazylegsMobileIntenseTalk;
		++_animationFrame;
		// This is probably an untriggered animation
		// TODO It should not lead to kModelAnimationCrazylegsCalmTalk but to one of Crazylegs mobile animations
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsMobileIntenseTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 14:
		*animation = kModelAnimationCrazylegsHandsUpTalk;
		++_animationFrame;
		// This is probably an untriggered animation
		// TODO It should not lead to kModelAnimationCrazylegsCalmTalk but to one of Crazylegs hands-up animations
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHandsUpTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationCrazylegsCalmTalk;
		}
		break;

	case 15:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			Actor_Change_Animation_Mode(kActorCrazylegs, 43);
			_animationState = kCrazylegsStateMobileCall;
			_resumeIdleAfterFramesetCompletesFlag = false;
			*animation = kModelAnimationCrazylegsMobileIdle;
		} else {
			*animation = kModelAnimationCrazylegsMobileIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsMobileIdle))
				_animationFrame = 0;
		}
		break;

	case 16:
		*animation = kModelAnimationCrazylegsHandsUpRaisesHands;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHandsUpRaisesHands)) {
			_animationFrame = 0;
			_animationState = kCrazylegsStateHandsUp;
			*animation = kModelAnimationCrazylegsHandsUpIdle;
		}
		break;

	case 17:
		*animation = kModelAnimationCrazylegsHandsUpLowersHands;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHandsUpLowersHands)) {
			*animation = kModelAnimationCrazylegsIdle;
			_animationFrame = 0;
			_animationState = kCrazylegsStateIdle;
		}
		break;

	case 18:
		// This animation plays in reverse
		// TODO Code seems wrong (especially the if clause condition).
		//      Probably left like this because the animation is untriggered
		*animation = kModelAnimationCrazylegsHangsUpMobile;
		--_animationFrame;
		if (_animationFrame <= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHangsUpMobile)) { // matches original
			_animationFrame = 0;
			_animationState = kCrazylegsStateMobileCall;
			*animation = kModelAnimationCrazylegsIdle;
		}
		break;

	case 19:
		*animation = kModelAnimationCrazylegsHangsUpMobile;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHangsUpMobile)) {
			*animation = kModelAnimationCrazylegsIdle;
			_animationFrame = 0;
			_animationState = kCrazylegsStateIdle;
			if (Actor_Query_Goal_Number(kActorCrazylegs) == 10) {
				Actor_Set_Goal_Number(kActorCrazylegs, 11);
			}
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptCrazylegs::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case kCrazylegsStateIdle:
			// fall through
		case 3:
			return true;

		case kCrazylegsStateHandsUp:
			_animationState = 17;
			_animationFrame = 0;
			break;

		case kCrazylegsStateMobileCall:
			_animationState = 19;
			_animationFrame = 0;
			break;

		case 4:
			// fall through
		case 5:
			_animationState = kCrazylegsStateIdle;
			_animationFrame = 0;
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
		case 11:
			// fall through
		case 12:
			// fall through
		case 13:
			// fall through
		case 14:
			// fall through
		case 15:
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		default:
			break;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		if (_animationState == kCrazylegsStateMobileCall) {
			_animationState = 15;
		} else {
			_animationState = 6;
		}
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 12:
		_animationState = 7;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		_animationState = 8;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		_animationState = 9;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 15:
		_animationState = 10;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 16:
		_animationState = 11;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 17:
		_animationState = 12;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 18:
		_animationState = 13;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 19:
		_animationState = 14;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 23:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 43:
		// picks up mobile phone call
		if (_animationState != kCrazylegsStateMobileCall) {
			_animationState = 18;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationCrazylegsHangsUpMobile) - 1;
		}
		break;

	default:
		break;
	}

	return true;
}

void AIScriptCrazylegs::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptCrazylegs::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptCrazylegs::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptCrazylegs::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
