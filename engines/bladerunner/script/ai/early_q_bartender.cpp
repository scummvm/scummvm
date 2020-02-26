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

AIScriptEarlyQBartender::AIScriptEarlyQBartender(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
	_var1 = 0;
	_var2 = 1;
}

void AIScriptEarlyQBartender::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;
	_var1 = 0;
	_var2 = 1;

	Actor_Put_In_Set(kActorEarlyQBartender, kSetNR05_NR08);
	Actor_Set_At_XYZ(kActorEarlyQBartender, -717.0f, 0.0f, -468.0f, 416);
}

bool AIScriptEarlyQBartender::Update() {
	return false;
}

void AIScriptEarlyQBartender::TimerExpired(int timer) {
	//return false;
}

void AIScriptEarlyQBartender::CompletedMovementTrack() {
	//return false;
}

void AIScriptEarlyQBartender::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptEarlyQBartender::ClickedByPlayer() {
	//return false;
}

void AIScriptEarlyQBartender::EnteredSet(int setId) {
	// return false;
}

void AIScriptEarlyQBartender::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptEarlyQBartender::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptEarlyQBartender::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptEarlyQBartender::ShotAtAndMissed() {
	// return false;
}

bool AIScriptEarlyQBartender::ShotAtAndHit() {
	return false;
}

void AIScriptEarlyQBartender::Retired(int byActorId) {
	// return false;
}

int AIScriptEarlyQBartender::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptEarlyQBartender::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptEarlyQBartender::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_var1 == 1) {
			*animation = 753;
			if (_animationFrame <= 5) {
				_var2 = 1;
			}
			_animationFrame += _var2;
			if (_animationFrame == 14 && !Random_Query(0, 1)) {
				_var2 = -1;
			}

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(753)) {
				_animationFrame = 0;
				_var1 = 0;
			}
		} else if (_var1 == 0) {
			*animation = 752;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(752)) {
				_animationFrame = 0;

				if (!Random_Query(0, 6)) {
					_var1 = 1;
				}
			}
		}
		break;

	case 1:
		*animation = 755;

		if (!_animationFrame && _flag) {
			_animationState = 0;
			_var1 = 0;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(755)) {
				_animationFrame = 0;
			}
		}
		break;

	case 2:
		*animation = 757;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(757)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 755;
		}
		break;

	case 3:
		*animation = 757;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(757)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 755;
		}
		break;

	case 4:
		*animation = 754;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(754)) {
			Actor_Change_Animation_Mode(kActorEarlyQBartender, kAnimationModeIdle);
			*animation = 752;
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

bool AIScriptEarlyQBartender::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		_animationState = 0;
		_animationFrame = 0;
		_var1 = 0;
		_var2 = 1;
		break;

	case 3:
		_animationState = 1;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 12:
		_animationState = 2;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 13:
		_animationState = 3;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 23:
		_animationState = 4;
		_animationFrame = 0;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptEarlyQBartender::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptEarlyQBartender::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptEarlyQBartender::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptEarlyQBartender::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
