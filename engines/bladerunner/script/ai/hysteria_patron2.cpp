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

AIScriptHysteriaPatron2::AIScriptHysteriaPatron2(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptHysteriaPatron2::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	Actor_Put_In_Set(kActorHysteriaPatron2, kSetNR05_NR08);
	Actor_Set_At_XYZ(kActorHysteriaPatron2, -516.0f, 0.0f, -190.0f, 452);
}

bool AIScriptHysteriaPatron2::Update() {
	return false;
}

void AIScriptHysteriaPatron2::TimerExpired(int timer) {
	//return false;
}

void AIScriptHysteriaPatron2::CompletedMovementTrack() {
	//return false;
}

void AIScriptHysteriaPatron2::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHysteriaPatron2::ClickedByPlayer() {
	Actor_Face_Actor(kActorMcCoy, kActorHysteriaPatron2, true);
	Actor_Says(kActorMcCoy, 8935, kAnimationModeTalk);
}

void AIScriptHysteriaPatron2::EnteredSet(int setId) {
	// return false;
}

void AIScriptHysteriaPatron2::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptHysteriaPatron2::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptHysteriaPatron2::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHysteriaPatron2::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHysteriaPatron2::ShotAtAndHit() {
	return false;
}

void AIScriptHysteriaPatron2::Retired(int byActorId) {
	// return false;
}

int AIScriptHysteriaPatron2::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHysteriaPatron2::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

const int kAnimationsCount = 30;
const int animationList[kAnimationsCount] = {
	kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsUpLeftMotion,      kModelAnimationHysteriaPatron2DanceHandsUpSitAndUp,
	kModelAnimationHysteriaPatron2DanceHandsUpToHandsDown,     kModelAnimationHysteriaPatron2DanceHandsDownHipsSwirl,     kModelAnimationHysteriaPatron2DanceHandsDownLegSwirl,
	kModelAnimationHysteriaPatron2DanceHandsDownLeanBackForth, kModelAnimationHysteriaPatron2DanceHandsDownToHandsUp,     kModelAnimationHysteriaPatron2DanceHandsUpLeftMotion,
	kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsBellyMotion,
	kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsUpLeftMotion,      kModelAnimationHysteriaPatron2DanceHandsUpToHandsDown,
	kModelAnimationHysteriaPatron2DanceHandsDownLegSwirl,      kModelAnimationHysteriaPatron2DanceHandsDownHipsSwirl,     kModelAnimationHysteriaPatron2DanceHandsDownLeanBackForth,
	kModelAnimationHysteriaPatron2DanceHandsDownToHandsUp,     kModelAnimationHysteriaPatron2DanceHandsUpSitAndUp,        kModelAnimationHysteriaPatron2DanceHandsBellyMotion,
	kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsBellyMotion,
	kModelAnimationHysteriaPatron2DanceHandsBellyMotion,       kModelAnimationHysteriaPatron2DanceHandsUpToHandsDown,     kModelAnimationHysteriaPatron2DanceHandsDownHipsSwirl,
	kModelAnimationHysteriaPatron2DanceHandsDownHipsSwirl,     kModelAnimationHysteriaPatron2DanceHandsDownLeanBackForth, kModelAnimationHysteriaPatron2DanceHandsDownToHandsUp
};

bool AIScriptHysteriaPatron2::UpdateAnimation(int *animation, int *frame) {
	*animation = animationList[_animationState];

	if (++_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
		_animationFrame = 0;

		if (++_animationState >= kAnimationsCount)
			_animationState = 0;

		*animation = animationList[_animationState];
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptHysteriaPatron2::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptHysteriaPatron2::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHysteriaPatron2::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHysteriaPatron2::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHysteriaPatron2::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
