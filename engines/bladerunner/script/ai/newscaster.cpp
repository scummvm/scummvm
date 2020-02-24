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

AIScriptNewscaster::AIScriptNewscaster(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptNewscaster::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptNewscaster::Update() {
	return false;
}

void AIScriptNewscaster::TimerExpired(int timer) {
	//return false;
}

void AIScriptNewscaster::CompletedMovementTrack() {
	//return false;
}

void AIScriptNewscaster::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptNewscaster::ClickedByPlayer() {
	//return false;
}

void AIScriptNewscaster::EnteredSet(int setId) {
	// return false;
}

void AIScriptNewscaster::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptNewscaster::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptNewscaster::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptNewscaster::ShotAtAndMissed() {
	// return false;
}

bool AIScriptNewscaster::ShotAtAndHit() {
	return false;
}

void AIScriptNewscaster::Retired(int byActorId) {
	// return false;
}

int AIScriptNewscaster::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptNewscaster::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptNewscaster::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptNewscaster::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptNewscaster::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptNewscaster::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptNewscaster::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptNewscaster::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
