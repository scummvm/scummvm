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

AIScriptTaffyPatron::AIScriptTaffyPatron(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptTaffyPatron::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptTaffyPatron::Update() {
	return false;
}

void AIScriptTaffyPatron::TimerExpired(int timer) {
	//return false;
}

void AIScriptTaffyPatron::CompletedMovementTrack() {
	//return false;
}

void AIScriptTaffyPatron::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTaffyPatron::ClickedByPlayer() {
	//return false;
}

void AIScriptTaffyPatron::EnteredSet(int setId) {
	// return false;
}

void AIScriptTaffyPatron::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptTaffyPatron::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptTaffyPatron::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTaffyPatron::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTaffyPatron::ShotAtAndHit() {
	return false;
}

void AIScriptTaffyPatron::Retired(int byActorId) {
	// return false;
}

int AIScriptTaffyPatron::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTaffyPatron::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 0:
		Actor_Put_In_Set(kActorTaffyPatron, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorTaffyPatron, 40, 0);
		return true;

	case 250:
		Actor_Put_In_Set(kActorTaffyPatron, kSetNR01);
		Actor_Set_At_XYZ(kActorTaffyPatron, -170.4f, 23.68f, -850.0f, 324);
		Async_Actor_Walk_To_XYZ(kActorTaffyPatron, -390.0f, 31.55f, -429.0f, 24, true);
		return true;

	case 255:
		Actor_Put_In_Set(kActorTaffyPatron, kSetNR01);
		Actor_Set_At_XYZ(kActorTaffyPatron, -170.4f, 23.68f, -850.0f, 324);
		Actor_Change_Animation_Mode(kActorTaffyPatron, 48);
		return true;

	default:
		break;
	}

	return false;
}

bool AIScriptTaffyPatron::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		// Dummy placeholder, kModelAnimationZubenIdle (406) is a Zuben animation
		*animation = kModelAnimationZubenIdle;
		_animationFrame = 0;
		break;

	case 1:
		*animation = kModelAnimationTaffyPatronRunning;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationTaffyPatronRunning) - 1) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationTaffyPatronShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationTaffyPatronShotDead) - 1) {
			++_animationFrame;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptTaffyPatron::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		_animationState = 0;
		_animationFrame = 0;
		break;

	case 2:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 2;
		_animationFrame = 0;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptTaffyPatron::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTaffyPatron::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTaffyPatron::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTaffyPatron::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
