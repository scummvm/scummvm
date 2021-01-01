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

AIScriptHawkersBarkeep::AIScriptHawkersBarkeep(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1, 2s
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var3 = 1;
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptHawkersBarkeep::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var3 = 1;
	_resumeIdleAfterFramesetCompletesFlag = false;

	Actor_Put_In_Set(kActorHawkersBarkeep, kSetHC01_HC02_HC03_HC04);
	Actor_Set_At_XYZ(kActorHawkersBarkeep, -225.0f, 0.14f, 39.0f, 284);
}

bool AIScriptHawkersBarkeep::Update() {
	return false;
}

void AIScriptHawkersBarkeep::TimerExpired(int timer) {
	//return false;
}

void AIScriptHawkersBarkeep::CompletedMovementTrack() {
	//return false;
}

void AIScriptHawkersBarkeep::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHawkersBarkeep::ClickedByPlayer() {
	//return false;
}

void AIScriptHawkersBarkeep::EnteredSet(int setId) {
	// return false;
}

void AIScriptHawkersBarkeep::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptHawkersBarkeep::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptHawkersBarkeep::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHawkersBarkeep::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHawkersBarkeep::ShotAtAndHit() {
	return false;
}

void AIScriptHawkersBarkeep::Retired(int byActorId) {
	// return false;
}

int AIScriptHawkersBarkeep::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHawkersBarkeep::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 1)  {
		_animationState = 8;
		_animationFrame = -1;
		Actor_Set_Goal_Number(kActorHawkersBarkeep, 0);

		return true;
	} else if (newGoalNumber == 2) {
		_animationState = 9;
		_animationFrame = -1;
		Actor_Set_Goal_Number(kActorHawkersBarkeep, 0);

		return true;
	}

	return false;
}

bool AIScriptHawkersBarkeep::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationHawkersBarkeepIdle;

			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;

				if (Random_Query(0, 6) == 0) {
					_var3 = -_var3;
				}
			} else {
				_animationFrame += _var3;

				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepIdle))
					_animationFrame = 0;

				if (_animationFrame < 0)
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepIdle) - 1;

				if (!Random_Query(0, 4))
					_varNumOfTimesToHoldCurrentFrame = 1;

				if (_animationFrame == 13 || _animationFrame == 5 || _animationFrame == 9)
					_varNumOfTimesToHoldCurrentFrame = Random_Query(2, 8);

				if (!Random_Query(0, 5)) {
					if (_animationFrame == 0 || _animationFrame == 11) {
						_animationFrame = 0;

						if (Random_Query(0, 1)) {
							*animation = kModelAnimationHawkersBarkeepCleaningBar;
							_varChooseIdleAnimation = 1;
						} else {
							*animation = kModelAnimationHawkersBarkeepWipingGlasses;
							_varChooseIdleAnimation = 2;
						}
					}
				}
			}
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationHawkersBarkeepCleaningBar;
			if (_animationFrame <= 3)
				_var3 = 1;

			if (_animationFrame == 11) {
				if (Random_Query(0, 2))
					_var3 = -1;
			}

			_animationFrame += _var3;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepCleaningBar)) {
				*animation = kModelAnimationHawkersBarkeepIdle;
				_animationFrame = 0;
				_varChooseIdleAnimation = 0;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationHawkersBarkeepWipingGlasses;

			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;

				if (_varNumOfTimesToHoldCurrentFrame == 0)
					_var3 = 2 * Random_Query(0, 1) - 1;
			} else {
				if (_animationFrame <= 11)
					_var3 = 1;

				if (_animationFrame == 14) {
					if (Random_Query(0, 2) != 0) {
						_var3 = -1;
					}
				}

				if (_animationFrame == 18) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(5, 15);
				}

				++_animationFrame;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepWipingGlasses)) {
					*animation = kModelAnimationHawkersBarkeepIdle;
					_animationFrame = 0;
					_varChooseIdleAnimation = 0;
					_varNumOfTimesToHoldCurrentFrame = 0;
				}
			}
		}
		break;

	case 1:
		if (_varChooseIdleAnimation == 0) {
			_animationFrame = 0;
			_animationState = _animationStateNext;
			*animation = _animationNext;
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationHawkersBarkeepCleaningBar;
			++_animationFrame;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepCleaningBar)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationHawkersBarkeepWipingGlasses;
			_animationFrame += 2;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepWipingGlasses)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		}

		break;

	case 2:
		*animation = kModelAnimationHawkersBarkeepCalmTalk;

		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			_animationState = 0;
			_varChooseIdleAnimation = 0;
		} else {
			++_animationFrame;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepCalmTalk)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = kModelAnimationHawkersBarkeepExplainTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepExplainTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationHawkersBarkeepCalmTalk;
		}
		break;

	case 4:
		*animation = kModelAnimationHawkersBarkeepBentsAndWipesTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepBentsAndWipesTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationHawkersBarkeepCalmTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationHawkersBarkeepAltGestureGiveTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepAltGestureGiveTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationHawkersBarkeepCalmTalk;
		}
		break;

	case 6:
		*animation = kModelAnimationHawkersBarkeepBentingTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepBentingTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationHawkersBarkeepCalmTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationHawkersBarkeepBentingAndDismissTalk;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepBentingAndDismissTalk)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationHawkersBarkeepCalmTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationHawkersBarkeepGestureGive;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepGestureGive)) {
			*animation = kModelAnimationHawkersBarkeepIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 9:
		*animation = kModelAnimationHawkersBarkeepGiveMoonshine;
		++_animationFrame;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationHawkersBarkeepGiveMoonshine)) {
			*animation = kModelAnimationHawkersBarkeepIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	default:
		break;
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptHawkersBarkeep::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 2 && _animationState <= 7) {
			_resumeIdleAfterFramesetCompletesFlag = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 3:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 2;
			_animationNext = kModelAnimationHawkersBarkeepCalmTalk;
		}
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 12:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 3;
			_animationNext = kModelAnimationHawkersBarkeepExplainTalk;
		}
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 4;
			_animationNext = kModelAnimationHawkersBarkeepBentsAndWipesTalk;
		}
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 5;
			_animationNext = kModelAnimationHawkersBarkeepAltGestureGiveTalk;
		}
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 15:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 6;
			_animationNext = kModelAnimationHawkersBarkeepBentingTalk;
		}
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 16:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 7;
			_animationNext = kModelAnimationHawkersBarkeepBentingAndDismissTalk;
		}
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptHawkersBarkeep::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHawkersBarkeep::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHawkersBarkeep::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHawkersBarkeep::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
