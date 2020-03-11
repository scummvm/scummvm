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

AIScriptHysteriaPatron1::AIScriptHysteriaPatron1(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptHysteriaPatron1::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	Actor_Put_In_Set(kActorHysteriaPatron1, kSetNR03);
	Actor_Set_At_XYZ(kActorHysteriaPatron1, 50.0f, -6.59f, -1030.0f, 524);
}

bool AIScriptHysteriaPatron1::Update() {
	return false;
}

void AIScriptHysteriaPatron1::TimerExpired(int timer) {
	//return false;
}

void AIScriptHysteriaPatron1::CompletedMovementTrack() {
	//return false;
}

void AIScriptHysteriaPatron1::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHysteriaPatron1::ClickedByPlayer() {
	//return false;
}

void AIScriptHysteriaPatron1::EnteredSet(int setId) {
	// return false;
}

void AIScriptHysteriaPatron1::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptHysteriaPatron1::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptHysteriaPatron1::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHysteriaPatron1::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHysteriaPatron1::ShotAtAndHit() {
	return false;
}

void AIScriptHysteriaPatron1::Retired(int byActorId) {
	// return false;
}

int AIScriptHysteriaPatron1::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHysteriaPatron1::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

const int kAnimationsCount = 27;
const int animationList[kAnimationsCount] = {
	kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp, kModelAnimationHysteriaPatron1DanceStandingUpLeftMotion, kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp,
	kModelAnimationHysteriaPatron1DanceStandingUpToSplits,     kModelAnimationHysteriaPatron1DanceSplitsDuckAndDown,    kModelAnimationHysteriaPatron1DanceSplitsSemiUpAndDown,
	kModelAnimationHysteriaPatron1DanceSplitsBackAndForth,     kModelAnimationHysteriaPatron1DanceSplitsToStandingUp,   kModelAnimationHysteriaPatron1DanceStandingUpLeftMotion,
	kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp, kModelAnimationHysteriaPatron1DanceStandingUpToSplits,   kModelAnimationHysteriaPatron1DanceSplitsSemiUpAndDown,
	kModelAnimationHysteriaPatron1DanceSplitsDuckAndDown,      kModelAnimationHysteriaPatron1DanceSplitsToStandingUp,   kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp,
	kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp, kModelAnimationHysteriaPatron1DanceStandingUpLeftMotion, kModelAnimationHysteriaPatron1DanceStandingUpToSplits,
	kModelAnimationHysteriaPatron1DanceSplitsBackAndForth,     kModelAnimationHysteriaPatron1DanceSplitsToStandingUp,   kModelAnimationHysteriaPatron1DanceStandingUpLeftMotion,
	kModelAnimationHysteriaPatron1DanceStandingUpSemiSitAndUp, kModelAnimationHysteriaPatron1DanceStandingUpToSplits,   kModelAnimationHysteriaPatron1DanceSplitsBackAndForth,
	kModelAnimationHysteriaPatron1DanceSplitsDuckAndDown,      kModelAnimationHysteriaPatron1DanceSplitsSemiUpAndDown,  kModelAnimationHysteriaPatron1DanceSplitsToStandingUp
};

bool AIScriptHysteriaPatron1::UpdateAnimation(int *animation, int *frame) {
	if (_vm->_cutContent
	    && (_animationState == 2 || _animationState == 16 || _animationState == 21)
	) {
		// replace a few of the repeated "standing up" animations
		// with the cut animation kModelAnimationHysteriaPatron1DanceStandingUpStowingMoney
		*animation = kModelAnimationHysteriaPatron1DanceStandingUpStowingMoney;
	} else {
		*animation = animationList[_animationState];
	}

	if (++_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
		_animationFrame = 0;

		if (++_animationState >= kAnimationsCount) {
			_animationState = 0;
		}

		if (_vm->_cutContent
		    && (_animationState == 2 || _animationState == 16 || _animationState == 21)
		) {
			// replace a few of the repeated "standing up" animations
			// with the cut animation kModelAnimationHysteriaPatron1DanceStandingUpStowingMoney
			*animation = kModelAnimationHysteriaPatron1DanceStandingUpStowingMoney;
		} else {
			*animation = animationList[_animationState];
		}
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptHysteriaPatron1::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptHysteriaPatron1::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHysteriaPatron1::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHysteriaPatron1::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHysteriaPatron1::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
