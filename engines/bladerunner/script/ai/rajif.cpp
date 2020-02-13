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

AIScriptRajif::AIScriptRajif(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptRajif::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	Actor_Set_Goal_Number(kActorRajif, 0);
}

bool AIScriptRajif::Update() {
	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorRajif) < 400
	)
		Actor_Set_Goal_Number(kActorRajif, 599);

	return false;
}

void AIScriptRajif::TimerExpired(int timer) {
	//return false;
}

void AIScriptRajif::CompletedMovementTrack() {
	//return false;
}

void AIScriptRajif::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptRajif::ClickedByPlayer() {
	//return false;
}

void AIScriptRajif::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptRajif::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptRajif::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptRajif::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptRajif::ShotAtAndMissed() {
	// return false;
}

bool AIScriptRajif::ShotAtAndHit() {
	return false;
}

void AIScriptRajif::Retired(int byActorId) {
	// return false;
}

int AIScriptRajif::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptRajif::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 300) {
		Actor_Put_In_Set(kActorRajif, kSetMA02_MA04);
		Actor_Set_At_XYZ(kActorRajif, -73.34f, -140.40f, 342.0f, 300);
		Actor_Change_Animation_Mode(kActorRajif, kAnimationModeIdle);
		return true;
	}

	if (newGoalNumber == 599) {
		Actor_Put_In_Set(kActorRajif, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorRajif, 41, 0);
		return true;
	}

	return false;
}

bool AIScriptRajif::UpdateAnimation(int *animation, int *frame) {
	if (_animationState <= 1) {
		if (_animationState) {
			*animation = 751;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(751)) {
				_animationFrame = 0;
			}
		} else { // bug in original. Both branches are equal
			*animation = 751;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(751)) {
				_animationFrame = 0;
			}
		}
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptRajif::ChangeAnimationMode(int mode) {
	if (!mode) {
		_animationState = 0;
		_animationFrame = 0;
	}
	return true;
}

void AIScriptRajif::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptRajif::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptRajif::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptRajif::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
