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

// Notes:
// kActorBlimpGuy's speech appears
// in specific scenes:
//  - AR01, AR02
//  - BB01
//  - CT01, CT02, CT03, CT04, CT06, CT07, CT08, CT12
//  - DR01, DR04
//  - MA05
//  - RC03
// and one cutscene:
//  - TB_FLY
// In the in-game scene his speech is played as ambient sound using Ambient_Sounds_Add_Speech_Sound()
// It is thus not subtitled as of yet.
// TODO: maybe if we support dual subtitles being displayed on-screen
// The Blimp Guy's speech in the TB_FLY VQA cutscene is subtitled.
//
// The FRA and ESP versions do not use the Blimp's horn sound at all during any of the announcements
//
// From the available quotes for kActorBlimpGuy, only quotes with id 0, 20, 40, 50 are used.
// Quote 10 is unused:
//    "A new life awaits you in the Off-World colonies."
//    "The chance to begin again in a golden land of opportunity and adventure."
//    - In ENG and DEU versions it is identical with the second half of quote id 0 and thus redundant
//    - In FRA, ESP and ITA versions it is the missing second half of quote id 0 and is thus "required"
//      TODO: Figure out a way to restore this quote as a continuation of the previous quote
//            given that this is ambient sound!
//
// Quote 30 is unused:
//    Roughly translates to: "What are you waiting for? Emigrate to the colonies!"
//    - In ENG version this is a *boop* sound
//    - In DEU version this is the start of a new announcement (the Blimp horn plays)
//    - In ITA version this should be a continuation of an announcement (the Blimp horn does not play)
//    - In FRA and ESP version this can be either a new announcement or a continuation of the previous announcement
//      (the Blimp horn does not play ever in FRA and ESP versions)
//      TODO: Figure out a way to restore this quote as a continuation of the previous quote
//            given that this is ambient sound!
//
AIScriptBlimpGuy::AIScriptBlimpGuy(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptBlimpGuy::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
}

bool AIScriptBlimpGuy::Update() {
	return false;
}

void AIScriptBlimpGuy::TimerExpired(int timer) {
	//return false;
}

void AIScriptBlimpGuy::CompletedMovementTrack() {
	//return false;
}

void AIScriptBlimpGuy::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptBlimpGuy::ClickedByPlayer() {
	//return false;
}

void AIScriptBlimpGuy::EnteredSet(int setId) {
	// return false;
}

void AIScriptBlimpGuy::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptBlimpGuy::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptBlimpGuy::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptBlimpGuy::ShotAtAndMissed() {
	// return false;
}

bool AIScriptBlimpGuy::ShotAtAndHit() {
	return false;
}

void AIScriptBlimpGuy::Retired(int byActorId) {
	// return false;
}

int AIScriptBlimpGuy::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptBlimpGuy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptBlimpGuy::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptBlimpGuy::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptBlimpGuy::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptBlimpGuy::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptBlimpGuy::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptBlimpGuy::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
