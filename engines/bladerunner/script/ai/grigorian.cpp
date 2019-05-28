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
	var_45CA10 = 0;
	var_45CA14 = 0;
}

void AIScriptGrigorian::Initialize() {
	var_45CA10 = 0;
	var_45CA14 = 0;
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

void AIScriptGrigorian::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptGrigorian::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptGrigorian::OtherAgentExitedThisScene(int otherActorId) {
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
		if (var_45CA10 == 0) {
			*animation = 478;
			if (var_45CA14) {
				var_45CA14--;
			} else {
				_animationFrame++;
				if (_animationFrame == 5 || _animationFrame == 13) {
					var_45CA14 = Random_Query(2, 4);
				}
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(478)) {
					_animationFrame = 0;
					var_45CA10 = Random_Query(0, 2);
				}
			}
		} else if (var_45CA10 == 1) {
			*animation = 479;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(479)) {
				*animation = 478;
				_animationFrame = 0;
				var_45CA10 = 0;
			}
		} else if (var_45CA10 == 2) {
			*animation = 480;
			if (var_45CA14) {
				var_45CA14--;
			} else {
				_animationFrame++;
				if (_animationFrame >= 8 && _animationFrame <= 10) {
					var_45CA14 = Random_Query(2, 4);
				}
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(478)) {
					*animation = 478;
					_animationFrame = 0;
					var_45CA10 = 0;
				}
			}
		}
		break;
	case 1:
		*animation = 479;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(479)) {
			_animationFrame = 0;
		}
		break;
	case 2:
		*animation = 481;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(481)) {
			_animationFrame = 0;
		}
		break;
	case 3:
		*animation = 482;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(482)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 481;
		}
		break;
	case 4:
		*animation = 483;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(483)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 481;
		}
		break;
	case 5:
		*animation = 484;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(484)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 481;
		}
		break;
	case 6:
		*animation = 485;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(485)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 481;
		}
		break;
	case 7:
		*animation = 486;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(486)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 481;
		}
		break;
	default:
		*animation = 399;
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
