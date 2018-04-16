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

AIScriptTyrell::AIScriptTyrell(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = true;
	_var = 0;
}

void AIScriptTyrell::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = true;
	_var = 0;

	Actor_Set_Goal_Number(kActorTyrell, 0);
}

bool AIScriptTyrell::Update() {
	if (Global_Variable_Query(kVariableChapter) > 3 && Actor_Query_Goal_Number(kActorTyrell) < 300)
		Actor_Set_Goal_Number(kActorTyrell, 300);

	return false;
}

void AIScriptTyrell::TimerExpired(int timer) {
	//return false;
}

void AIScriptTyrell::CompletedMovementTrack() {
	//return false;
}

void AIScriptTyrell::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTyrell::ClickedByPlayer() {
	//return false;
}

void AIScriptTyrell::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptTyrell::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptTyrell::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptTyrell::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTyrell::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTyrell::ShotAtAndHit() {
	return false;
}

void AIScriptTyrell::Retired(int byActorId) {
	// return false;
}

int AIScriptTyrell::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTyrell::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 300) {
		Actor_Put_In_Set(kActorTyrell, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorTyrell, 40, 0);
	}

	return false;
}

bool AIScriptTyrell::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_var == 1) {
			*animation = 767;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(767)) {
				*animation = 766;
				_animationFrame = 0;
				_var = 0;
			}
		} else if (_var == 0) {
			*animation = 766;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(766)) {
				_animationFrame = 0;
				if (!Random_Query(0, 3)) {
					_var = 1;
				}
			}
		}
		break;

	case 1:
		*animation = 765;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(765)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		if (!_animationFrame && _flag) {
			*animation = 766;
			_animationState = 0;
		} else {
			*animation = 768;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(768)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 769;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(769)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 768;
		}
		break;

	case 4:
		*animation = 770;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(770)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 768;
		}
		break;

	case 5:
		*animation = 771;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(771)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 768;
		}
		break;

	case 6:
		*animation = 772;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(772)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 768;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptTyrell::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 2 && _animationState <= 6) {
			_flag = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;
	case 1:
		_animationState = 1;
		_animationFrame = 0;
		break;
	case 3:
		_animationState = 2;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 12:
		_animationState = 3;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 13:
		_animationState = 4;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 14:
		_animationState = 5;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 15:
		_animationState = 6;
		_animationFrame = 0;
		_flag = 0;
		break;
	default:
		break;
	}

	return true;
}

void AIScriptTyrell::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTyrell::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTyrell::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTyrell::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
