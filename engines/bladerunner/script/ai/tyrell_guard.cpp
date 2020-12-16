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

AIScriptTyrellGuard::AIScriptTyrellGuard(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_frameDelta = 1;
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptTyrellGuard::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_frameDelta = 1;
	_resumeIdleAfterFramesetCompletesFlag = false;
	Actor_Set_Goal_Number(kActorTyrellGuard, 0);
}

bool AIScriptTyrellGuard::Update() {
	return false;
}

void AIScriptTyrellGuard::TimerExpired(int timer) {
	switch (timer) {
	case kActorTimerAIScriptCustomTask0:
		AI_Countdown_Timer_Reset(kActorTyrellGuard, kActorTimerAIScriptCustomTask0);
		if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetTB02_TB03) {
			Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardWakeUpAndArrestMcCoy);
		}
		break;

	case kActorTimerAIScriptCustomTask1:
		AI_Countdown_Timer_Reset(kActorTyrellGuard, kActorTimerAIScriptCustomTask1);
		Actor_Set_Goal_Number(kActorTyrellGuard, kGoalTyrellGuardArrestMcCoy);
		break;
	}
}

void AIScriptTyrellGuard::CompletedMovementTrack() {
	//return false;
}

void AIScriptTyrellGuard::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTyrellGuard::ClickedByPlayer() {
	//return false;
}

void AIScriptTyrellGuard::EnteredSet(int setId) {
	// return false;
}

void AIScriptTyrellGuard::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptTyrellGuard::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptTyrellGuard::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTyrellGuard::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTyrellGuard::ShotAtAndHit() {
	return false;
}

void AIScriptTyrellGuard::Retired(int byActorId) {
	// return false;
}

int AIScriptTyrellGuard::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTyrellGuard::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalTyrellGuardSleeping:
		if (currentGoalNumber != newGoalNumber) {
			Actor_Change_Animation_Mode(kActorTyrellGuard, 55);
			AI_Countdown_Timer_Start(kActorTyrellGuard, kActorTimerAIScriptCustomTask0, 30);
		}
		return true;

	case kGoalTyrellGuardWakeUpAndArrestMcCoy:
		Actor_Change_Animation_Mode(kActorTyrellGuard, kAnimationModeIdle);
		Delay(1000);
		Actor_Says(kActorTyrellGuard, 320, 14);
		Actor_Change_Animation_Mode(kActorTyrellGuard, 50);
		Ambient_Sounds_Play_Sound(kSfxTBALARM, 100, 0, 0, 0);
		Delay(1000);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
		return true;

	case kGoalTyrellGuardWakeUp:
		AI_Countdown_Timer_Reset(kActorTyrellGuard, kActorTimerAIScriptCustomTask0);
		Actor_Says(kActorTyrellGuard, 310, 14);
		AI_Countdown_Timer_Start(kActorTyrellGuard, kActorTimerAIScriptCustomTask1, 20);
		return true;

	case kGoalTyrellGuardArrestMcCoy:
		Actor_Change_Animation_Mode(kActorTyrellGuard, 50);
		Ambient_Sounds_Play_Sound(kSfxTBALARM, 100, 0, 0, 0);
		Delay(1000);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
		return true;

	case kGoalTyrellGuardWait:
		AI_Countdown_Timer_Reset(kActorTyrellGuard, kActorTimerAIScriptCustomTask1);
		return true;
	}
	return false;
}

bool AIScriptTyrellGuard::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationTyrellGuardSittingIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationTyrellGuardSittingSleepingWakingUp;
		if (_animationFrame <= 5) {
			_frameDelta = 1;
		} else if (_animationFrame >= 12) {
			_frameDelta = -1;
		}
		_animationFrame += _frameDelta;
		break;

	case 2:
		*animation = kModelAnimationTyrellGuardSittingSleepingWakingUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingSleepingWakingUp)) {
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationTyrellGuardSittingCalmTalk;
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationTyrellGuardSittingIdle;
			_animationState = 0;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 4:
		*animation = kModelAnimationTyrellGuardSittingExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingExplainTalk)) {
			*animation = kModelAnimationTyrellGuardSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 3;
		}
		break;

	case 5:
		*animation = kModelAnimationTyrellGuardSittingHandOverHeadTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingHandOverHeadTalk)) {
			*animation = kModelAnimationTyrellGuardSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 3;
		}
		break;

	case 6:
		*animation = kModelAnimationTyrellGuardSittingSuggestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingSuggestTalk)) {
			*animation = kModelAnimationTyrellGuardSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 3;
		}
		break;

	case 7:
		*animation = kModelAnimationTyrellGuardSittingUpsetTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingUpsetTalk)) {
			*animation = kModelAnimationTyrellGuardSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 3;
		}
		break;

	case 8:
		*animation = kModelAnimationTyrellGuardSittingGestureGive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingGestureGive)) {
			*animation = kModelAnimationTyrellGuardSittingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 9:
		*animation = kModelAnimationTyrellGuardSittingMaybeHugsMonitors;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingMaybeHugsMonitors)) {
			*animation = kModelAnimationTyrellGuardSittingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 10:
		*animation = kModelAnimationTyrellGuardSittingSleepingWakingUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingSleepingWakingUp)) {
			*animation = kModelAnimationTyrellGuardSittingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 11:
		*animation = kModelAnimationTyrellGuardSittingPressingAlertButton;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationTyrellGuardSittingPressingAlertButton)) {
			*animation = kModelAnimationTyrellGuardSittingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	}

	*frame = _animationFrame;
	return true;
}

bool AIScriptTyrellGuard::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 0:
			_animationState = 8;
			_animationFrame = 0;
			break;

		case 1:
			_animationState = 2;
			break;

		case 3:
			// fall through
		case 4:
			// fall through
		case 5:
			// fall through
		case 6:
			// fall through
		case 7:
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		case 8:
			break;

		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeTalk:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 3;
			_animationFrame = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		}
		break;

	case 12:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 4;
			_animationFrame = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		}
		break;

	case 13:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 5;
			_animationFrame = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		}
		break;

	case 14:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 6;
			_animationFrame = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		}
		break;

	case 15:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 7;
			_animationFrame = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		}
		break;

	case 23:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case 50:
		_animationState = 11;
		_animationFrame = 0;
		break;

	case 43:
		// fall through
	case 55:
		if (_animationState != 1) {
			_animationState = 1;
			_animationFrame = 0;
		}
		break;
	}
	return true;
}

void AIScriptTyrellGuard::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTyrellGuard::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTyrellGuard::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTyrellGuard::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
