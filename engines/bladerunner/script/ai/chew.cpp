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

AIScriptChew::AIScriptChew(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
	_varNumOfTimesToHoldCurrentFrame = 0;
	// _varChooseIdleAnimation can have valid values: 0, 1, 2
	_varChooseIdleAnimation = 0;
	_var3 = 1;
}

void AIScriptChew::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_varChooseIdleAnimation = 0;
	_var3 = 1;
}

bool AIScriptChew::Update() {
	return false;
}

void AIScriptChew::TimerExpired(int timer) {
	//return false;
}

void AIScriptChew::CompletedMovementTrack() {
	//return false;
}

void AIScriptChew::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptChew::ClickedByPlayer() {
	//return false;
}

void AIScriptChew::EnteredSet(int setId) {
	// return false;
}

void AIScriptChew::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptChew::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptChew::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptChew::ShotAtAndMissed() {
	// return false;
}

bool AIScriptChew::ShotAtAndHit() {
	return false;
}

void AIScriptChew::Retired(int byActorId) {
	// return false;
}

int AIScriptChew::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptChew::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptChew::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationChewIdle;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				++_animationFrame;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewIdle)) {
					_animationFrame = 0;
					_var3 = 1;
					_varChooseIdleAnimation = Random_Query(0, 2);
				} else if (!Random_Query(0, 1)) {
					_varNumOfTimesToHoldCurrentFrame = 1;
					if (!Random_Query(0, 3)) {
						_var3 = -_var3;
					}
				}
			}
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationChewFiddlingWithInstruments;
			_animationFrame += _var3;
			if (_animationFrame <= 6) {
				_var3 = 1;
			}
			if (_animationFrame == 13) {
				if (!Random_Query(0, 1)) {
					_var3 = -1;
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewFiddlingWithInstruments)) {
				_animationFrame = 0;
				_varChooseIdleAnimation = Random_Query(0, 2);
				_var3 = 1;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationChewAdjustingInstrument;
			_animationFrame += _var3;
			if (_animationFrame <= 8) {
				_var3 = 1;
			}
			if (_animationFrame == 16) {
				if (!Random_Query(0, 1)) {
					_var3 = -1;
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewAdjustingInstrument)) {
				_animationFrame = 0;
				_varChooseIdleAnimation = Random_Query(0, 2);
				_var3 = 1;
			}
		}
		break;

	case 1:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationChewIdle;
		}
		if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationChewFiddlingWithInstruments;
		}
		if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationChewAdjustingInstrument;
		}
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame += 2;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else {
			_animationFrame -= 2;
			if (_animationFrame <= 0) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		}
		break;

	case 2:
		*animation = kModelAnimationChewProtestTalk;
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationChewIdle;
			_animationState = 0;
			_varChooseIdleAnimation = 0;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewProtestTalk)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = kModelAnimationChewAngryTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewAngryTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 4:
		*animation = kModelAnimationChewExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewExplainTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationChewGoAwayTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewGoAwayTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 6:
		*animation = kModelAnimationChewDismissiveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewDismissiveTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationChewPointingSomewhereTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewPointingSomewhereTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationChewDescribePulledTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewDescribePulledTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 9:
		*animation = kModelAnimationChewDescribePushedTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewDescribePushedTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	case 10:
		*animation = kModelAnimationChewGotHit;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewGotHit)) {
			*animation = kModelAnimationChewIdle;
			_animationFrame = 0;
			_animationState = 0;
			_varChooseIdleAnimation = 0;
		}
		break;

	case 11:
		*animation = kModelAnimationChewShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewShotDead) - 1) {
			++_animationFrame;
		}
		break;

	case 12:
		*animation = kModelAnimationChewWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewWalking)) {
			_animationFrame = 0;
		}
		break;

	case 13:
		*animation = kModelAnimationChewTakesAStepBackwards;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationChewTakesAStepBackwards)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationChewProtestTalk;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptChew::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 0:
			return true;

		case 2:
			// fall through
		case 3:
			// fall through
		case 4:
			// fall through
		case 5:
			// fall through
		case 6:
			// fall through
		case 7:
			// fall through
		case 8:
			// fall through
		case 9:
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		default:
			_animationState = 0;
			_varChooseIdleAnimation = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeWalk:
		_animationFrame = 0;
		_animationState = 12;
		break;

	case 3:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 2;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 2;
				_animationNext = kModelAnimationChewProtestTalk;
				_animationState = 1;
			}
		}
		break;

	case 12:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 3;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 3;
				_animationNext = kModelAnimationChewAngryTalk;
				_animationState = 1;
			}
		}
		break;

	case 13:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 4;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 4;
				_animationNext = kModelAnimationChewExplainTalk;
				_animationState = 1;
			}
		}
		break;

	case 14:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 5;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 5;
				_animationNext = kModelAnimationChewGoAwayTalk;
				_animationState = 1;
			}
		}
		break;

	case 15:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 6;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 6;
				_animationNext = kModelAnimationChewDismissiveTalk;
				_animationState = 1;
			}
		}
		break;

	case 16:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 7;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 7;
				_animationNext = kModelAnimationChewPointingSomewhereTalk;
				_animationState = 1;
			}
		}
		break;

	case 17:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 8;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 8;
				_animationNext = kModelAnimationChewDescribePulledTalk;
				_animationState = 1;
			}
		}
		break;

	case 18:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState > 0) {
				_animationState = 9;
				_animationFrame = 0;
				_resumeIdleAfterFramesetCompletesFlag = false;
			} else {
				_animationStateNext = 9;
				_animationNext = kModelAnimationChewDescribePushedTalk;
				_animationState = 1;
			}
		}
		break;

	case 43:
		if (_animationState > 0 || (_animationState == 0 && _varChooseIdleAnimation != 1 && _varChooseIdleAnimation != 2)) {
			Actor_Change_Animation_Mode(kActorChew, kAnimationModeIdle);
			_varChooseIdleAnimation = Random_Query(1, 2);
		}
		break;

	case kAnimationModeDie:
		_animationFrame = 0;
		_animationState = 11;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptChew::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptChew::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptChew::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptChew::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
