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

AIScriptDispatcher::AIScriptDispatcher(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptDispatcher::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptDispatcher::Update() {
	return false;
}

void AIScriptDispatcher::TimerExpired(int timer) {
	//return false;
}

void AIScriptDispatcher::CompletedMovementTrack() {
	//return false;
}

void AIScriptDispatcher::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptDispatcher::ClickedByPlayer() {
	//return false;
}

void AIScriptDispatcher::EnteredSet(int setId) {
	// return false;
}

void AIScriptDispatcher::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptDispatcher::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptDispatcher::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptDispatcher::ShotAtAndMissed() {
	// return false;
}

bool AIScriptDispatcher::ShotAtAndHit() {
	return false;
}

void AIScriptDispatcher::Retired(int byActorId) {
	// return false;
}

int AIScriptDispatcher::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptDispatcher::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptDispatcher::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptDispatcher::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptDispatcher::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptDispatcher::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptDispatcher::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptDispatcher::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
