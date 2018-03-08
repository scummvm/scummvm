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

AIScriptHysteriaPatron3::AIScriptHysteriaPatron3(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptHysteriaPatron3::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptHysteriaPatron3::Update() {
	return false;
}

void AIScriptHysteriaPatron3::TimerExpired(int timer) {
	//return false;
}

void AIScriptHysteriaPatron3::CompletedMovementTrack() {
	//return false;
}

void AIScriptHysteriaPatron3::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHysteriaPatron3::ClickedByPlayer() {
	//return false;
}

void AIScriptHysteriaPatron3::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptHysteriaPatron3::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHysteriaPatron3::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHysteriaPatron3::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHysteriaPatron3::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHysteriaPatron3::ShotAtAndHit() {
	return false;
}

void AIScriptHysteriaPatron3::Retired(int byActorId) {
	// return false;
}

int AIScriptHysteriaPatron3::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHysteriaPatron3::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptHysteriaPatron3::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptHysteriaPatron3::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptHysteriaPatron3::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHysteriaPatron3::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHysteriaPatron3::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHysteriaPatron3::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
