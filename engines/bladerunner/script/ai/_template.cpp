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

AIScriptGenericWalkerA::AIScriptGenericWalkerA(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptGenericWalkerA::Initialize() {
	_animationStateNext = 0;
	_animationNext = 0;
	_animationFrame = 0;
	_animationState = 0;
}

bool AIScriptGenericWalkerA::Update() {
	return false;
}

void AIScriptGenericWalkerA::TimerExpired(int timer) {
}

void AIScriptGenericWalkerA::CompletedMovementTrack() {
}

void AIScriptGenericWalkerA::ReceivedClue(int clueId, int fromActorId) {
}

void AIScriptGenericWalkerA::ClickedByPlayer() {
}

void AIScriptGenericWalkerA::EnteredScene(int sceneId) {
}

void AIScriptGenericWalkerA::OtherAgentEnteredThisScene(int otherActorId) {
}

void AIScriptGenericWalkerA::OtherAgentExitedThisScene(int otherActorId) {
}

void AIScriptGenericWalkerA::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
}

void AIScriptGenericWalkerA::ShotAtAndMissed() {
}

bool AIScriptGenericWalkerA::ShotAtAndHit() {
	return false;
}

void AIScriptGenericWalkerA::Retired(int byActorId) {
}

int AIScriptGenericWalkerA::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGenericWalkerA::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptGenericWalkerA::UpdateAnimation(int *animation, int *frame) {
	*animation = 0;
	*frame = _animationFrame;
	return true;
}

bool AIScriptGenericWalkerA::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptGenericWalkerA::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGenericWalkerA::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGenericWalkerA::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptGenericWalkerA::FledCombat() {}

} // End of namespace BladeRunner
