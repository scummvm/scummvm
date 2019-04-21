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

AIScriptSergeantWalls::AIScriptSergeantWalls(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptSergeantWalls::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptSergeantWalls::Update() {
	if (Game_Flag_Query(kFlagSergeantWallsBuzzInRequest)
	 && Game_Flag_Query(kFlagSergeantWallsBuzzInDone)
	) {
		Game_Flag_Reset(kFlagSergeantWallsBuzzInRequest);
	}
	return false;
}

void AIScriptSergeantWalls::TimerExpired(int timer) {
	//return false;
}

void AIScriptSergeantWalls::CompletedMovementTrack() {
	//return false;
}

void AIScriptSergeantWalls::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptSergeantWalls::ClickedByPlayer() {
	//return false;
}

void AIScriptSergeantWalls::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptSergeantWalls::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptSergeantWalls::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptSergeantWalls::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptSergeantWalls::ShotAtAndMissed() {
	// return false;
}

bool AIScriptSergeantWalls::ShotAtAndHit() {
	return false;
}

void AIScriptSergeantWalls::Retired(int byActorId) {
	// return false;
}

int AIScriptSergeantWalls::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptSergeantWalls::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptSergeantWalls::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		_animationFrame++;
		if ( Game_Flag_Query(kFlagSergeantWallsBuzzInRequest)
		 && !Game_Flag_Query(kFlagSergeantWallsBuzzInDone)
		) {
			*animation = 724;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(724)) {
				_animationFrame = 0;
				Game_Flag_Set(kFlagSergeantWallsBuzzInDone);
			}
		} else {
			*animation = 722;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(722)) {
				_animationFrame = 0;
			}
		}
		break;
	case 1:
		*animation = 725;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(725)) {
			_animationFrame = 0;
		}
		break;
	case 3:
		*animation = 726;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(726)) {
			_animationState = 1;
			_animationFrame = 0;
			*animation = 725;
		}
		break;
	case 4:
		*animation = 727;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(727)) {
			_animationState = 1;
			_animationFrame = 0;
			*animation = 725;
		}
		break;
	case 5:
		*animation = 728;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(728)) {
			_animationState = 1;
			_animationFrame = 0;
			*animation = 725;
		}
		break;
	case 6:
		*animation = 729;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(729)) {
			_animationState = 1;
			_animationFrame = 0;
			*animation = 725;
		}
		break;
	case 7:
		*animation = 730;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(730)) {
			_animationState = 1;
			_animationFrame = 0;
			*animation = 725;
		}
		break;
	case 8:
		*animation = 731;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(731)) {
			_animationState = 1;
			_animationFrame = 0;
			*animation = 725;
		}
		break;
	case 9:
		*animation = 724;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(724)) {
			_animationState = 9;
			_animationFrame = 0;
			*animation = 724;
		}
		break;
	default:
		*animation = 399;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptSergeantWalls::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = 0;
		_animationFrame = 0;
		break;
	case kAnimationModeTalk:
		_animationState = 1;
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
	case 17:
		_animationState = 8;
		_animationFrame = 0;
		break;
	case 23:
		_animationState = 9;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptSergeantWalls::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptSergeantWalls::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptSergeantWalls::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptSergeantWalls::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
