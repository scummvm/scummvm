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

AIScriptGrigorian::AIScriptGrigorian(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1, 2
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
}

void AIScriptGrigorian::Initialize() {
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_animationStateNext = 0;
	_animationFrame = 0;
	_animationState = 0;
}

bool AIScriptGrigorian::Update() {
	if ( Actor_Query_Friendliness_To_Other(kActorGrigorian, kActorMcCoy) < 30
	 && !Game_Flag_Query(kFlagGrigorianDislikeMcCoy)
	) {
		Game_Flag_Set(kFlagGrigorianDislikeMcCoy);
		return true;
	}
	return false;
}

void AIScriptGrigorian::TimerExpired(int timer) {
	//return false;
}

void AIScriptGrigorian::CompletedMovementTrack() {
	//return false;
}

void AIScriptGrigorian::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptGrigorian::ClickedByPlayer() {
	//return false;
}

void AIScriptGrigorian::EnteredSet(int setId) {
	// return false;
}

void AIScriptGrigorian::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptGrigorian::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptGrigorian::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptGrigorian::ShotAtAndMissed() {
	// return false;
}

bool AIScriptGrigorian::ShotAtAndHit() {
	return false;
}

void AIScriptGrigorian::Retired(int byActorId) {
	// return false;
}

int AIScriptGrigorian::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGrigorian::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptGrigorian::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationGrigorianStandIdle;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				++_animationFrame;
				if (_animationFrame == 5 || _animationFrame == 13) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(2, 4);
				}
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandIdle)) {
					_animationFrame = 0;
					_varChooseIdleAnimation = Random_Query(0, 2);
				}
			}
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationGrigorianStandAnnoyedTalk;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandAnnoyedTalk)) {
				*animation = kModelAnimationGrigorianStandIdle;
				_animationFrame = 0;
				_varChooseIdleAnimation = 0;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationGrigorianStandArmsCrossedTalk;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				++_animationFrame;
				if (_animationFrame >= 8 && _animationFrame <= 10) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(2, 4);
				}
#if BLADERUNNER_ORIGINAL_BUGS
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandIdle)) {
					*animation = kModelAnimationGrigorianStandIdle;
					_animationFrame = 0;
					_varChooseIdleAnimation = 0;
				}
#else
				// bugfix set proper current animation here to get frameset number of frames
				//        (could also use *animation)
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandArmsCrossedTalk)) {
					*animation = kModelAnimationGrigorianStandIdle;
					_animationFrame = 0;
					_varChooseIdleAnimation = 0;
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
			}
		}
		break;

	case 1:
		*animation = kModelAnimationGrigorianStandAnnoyedTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandAnnoyedTalk)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationGrigorianStandProtestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandProtestTalk)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationGrigorianStandProtestMoreTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandProtestMoreTalk)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationGrigorianStandProtestTalk;
		}
		break;

	case 4:
		*animation = kModelAnimationGrigorianStandProtestEvenMoreTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandProtestEvenMoreTalk)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationGrigorianStandProtestTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationGrigorianStandInsistentTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandInsistentTalk)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationGrigorianStandProtestTalk;
		}
		break;

	case 6:
		*animation = kModelAnimationGrigorianStandDismissOrAccuseTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandDismissOrAccuseTalk)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationGrigorianStandProtestTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationGrigorianStandBegOrMockingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGrigorianStandBegOrMockingTalk)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationGrigorianStandProtestTalk;
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

bool AIScriptGrigorian::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = 0;
		_animationFrame = 0;
		break;

	case kAnimationModeWalk:
		// TODO A bug? This animation state is not for walking. It is for kModelAnimationGrigorianStandAnnoyedTalk.
		if (_animationState != 1) {
			_animationState = 1;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeTalk:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 13:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case 14:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 15:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 16:
		_animationState = 7;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptGrigorian::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGrigorian::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGrigorian::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptGrigorian::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
