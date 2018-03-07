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

AIScriptTemplate::AIScriptTemplate(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptTemplate::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptTemplate::Update() {
	return false;
}

void AIScriptTemplate::TimerExpired(int timer) {
	//return false;
}

void AIScriptTemplate::CompletedMovementTrack() {
	//return false;
}

void AIScriptTemplate::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTemplate::ClickedByPlayer() {
	//return false;
}

void AIScriptTemplate::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptTemplate::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptTemplate::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptTemplate::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTemplate::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTemplate::ShotAtAndHit() {
	return false;
}

void AIScriptTemplate::Retired(int byActorId) {
	// return false;
}

int AIScriptTemplate::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTemplate::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptTemplate::UpdateAnimation(int *animation, int *frame) {
	*animation = 0;
	*frame = _animationFrame;
	return true;
}

bool AIScriptTemplate::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptTemplate::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTemplate::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTemplate::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTemplate::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
