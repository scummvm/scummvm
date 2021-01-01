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

AIScriptMia::AIScriptMia(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptMia::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;
	Actor_Put_In_Set(kActorMia, kSetHF01);
	Actor_Set_At_XYZ(kActorMia, 606.77f, -0.01f, -214.3f, 511);
	Actor_Set_Goal_Number(kActorMia, 0);
}

bool AIScriptMia::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4
	 && Actor_Query_Goal_Number(kActorMia) != 300
	) {
		Actor_Set_Goal_Number(kActorMia, 300);
	}

	return false;
}

void AIScriptMia::TimerExpired(int timer) {
	//return false;
}

void AIScriptMia::CompletedMovementTrack() {
	//return false;
}

void AIScriptMia::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptMia::ClickedByPlayer() {
	//return false;
}

void AIScriptMia::EnteredSet(int setId) {
	// return false;
}

void AIScriptMia::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptMia::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptMia::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptMia::ShotAtAndMissed() {
	// return false;
}

bool AIScriptMia::ShotAtAndHit() {
	return false;
}

void AIScriptMia::Retired(int byActorId) {
	// return false;
}

int AIScriptMia::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMia::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 300) {
		Actor_Put_In_Set(kActorMia, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorMia, 40, 0);
	}

	return false;
}

bool AIScriptMia::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationMiaIdle;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMiaIdle))
			_animationFrame = 0;

		break;

	case 1:
		*animation = kModelAnimationMiaGestureGive;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMiaGestureGive)) {
			*animation = kModelAnimationMiaIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationMiaHeadNodTalk;

		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationMiaIdle;
			_animationFrame = 0;
			_animationState = 0;
		} else {
			++_animationFrame;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = kModelAnimationMiaMoreCalmTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMiaMoreCalmTalk)) {
			*animation = kModelAnimationMiaHeadNodTalk;
			_animationFrame = 0;
			_animationState = 2;
		}
		break;

	case 4:
		*animation = kModelAnimationMiaHandsOnWaistTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMiaHandsOnWaistTalk)) {
			*animation = kModelAnimationMiaHeadNodTalk;
			_animationFrame = 0;
			_animationState = 2;
		}
		break;
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptMia::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState >= 2
		 && _animationState <= 4
		) {
			_resumeIdleAfterFramesetCompletesFlag = false;
		} else {
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case kAnimationModeTalk:
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		_animationState = 2;
		break;

	case 12:
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		_animationState = 3;
		break;

	case 13:
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		_animationState = 4;
		break;

	case 23:
		_animationFrame = 0;
		_animationState = 1;
		break;
	}

	return true;
}

void AIScriptMia::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMia::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMia::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMia::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
