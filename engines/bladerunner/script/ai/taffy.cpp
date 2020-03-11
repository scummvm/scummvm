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

AIScriptTaffy::AIScriptTaffy(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptTaffy::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptTaffy::Update() {
	return false;
}

void AIScriptTaffy::TimerExpired(int timer) {
	//return false;
}

void AIScriptTaffy::CompletedMovementTrack() {
	//return false;
}

void AIScriptTaffy::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTaffy::ClickedByPlayer() {
	//return false;
}

void AIScriptTaffy::EnteredSet(int setId) {
	// return false;
}

void AIScriptTaffy::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptTaffy::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptTaffy::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTaffy::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTaffy::ShotAtAndHit() {
	return false;
}

void AIScriptTaffy::Retired(int byActorId) {
	// return false;
}

int AIScriptTaffy::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTaffy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptTaffy::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptTaffy::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptTaffy::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTaffy::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTaffy::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTaffy::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
