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

#include "bladerunner/script/aiscript_officer_leroy.h"

namespace BladeRunner {

AIScript_Officer_Leroy::AIScript_Officer_Leroy(BladeRunnerEngine *vm)
	: AIScriptBase(vm) {
}

void AIScript_Officer_Leroy::Initialize() {
	var_45D5B0_animation_state = 0;
	var_45D5B4_frame = 0;
	var_45D5B8 = 0;

	Actor_Put_In_Set(23, 69);
	Actor_Set_At_XYZ(23, -261.80f, 6.00f, 79.58f, 512);
	// Actor_Set_Goal_Number(23, 0);
	// Actor_Set_Frame_Rate_FPS(23, 8);
}

bool AIScript_Officer_Leroy::Update() {
	return false;
}

void AIScript_Officer_Leroy::TimerExpired(int timer) {
}

void AIScript_Officer_Leroy::CompletedMovementTrack() {
}

void AIScript_Officer_Leroy::ReceivedClue(int clueId, int fromActorId) {
}

void AIScript_Officer_Leroy::ClickedByPlayer() {
}

void AIScript_Officer_Leroy::EnteredScene(int sceneId) {
}

void AIScript_Officer_Leroy::OtherAgentEnteredThisScene() {
}

void AIScript_Officer_Leroy::OtherAgentExitedThisScene() {
}

void AIScript_Officer_Leroy::OtherAgentEnteredCombatMode() {
}

void AIScript_Officer_Leroy::ShotAtAndMissed() {
}

void AIScript_Officer_Leroy::ShotAtAndHit() {
}

void AIScript_Officer_Leroy::Retired(int byActorId) {
}

void AIScript_Officer_Leroy::GetFriendlinessModifierIfGetsClue() {
}

bool AIScript_Officer_Leroy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScript_Officer_Leroy::UpdateAnimation(int *animation, int *frame) {
	if (var_45D5B8 == 0) {
		*animation = 589;
		var_45D5B4_frame++;

		if (var_45D5B4_frame >= Slice_Animation_Query_Number_Of_Frames(589)) {
			var_45D5B4_frame = 0;
			var_45D5B8 = Random_Query(0, 2);
		}
	} else if (var_45D5B8 == 1) {
		*animation = 590;
		var_45D5B4_frame++;

		if (var_45D5B4_frame >= Slice_Animation_Query_Number_Of_Frames(590)) {
			var_45D5B4_frame = 0;
			var_45D5B8 = Random_Query(0, 2);
		}
	} else if (var_45D5B8 == 2) {
		*animation = 591;
		var_45D5B4_frame++;

		if (var_45D5B4_frame >= Slice_Animation_Query_Number_Of_Frames(591)) {
			var_45D5B4_frame = 0;
			var_45D5B8 = Random_Query(0, 2);
		}
	}
	*frame = var_45D5B4_frame;
	return true;
}

bool AIScript_Officer_Leroy::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 1:
		var_45D5B0_animation_state = 32;
		break;
	}
	return true;
}

void AIScript_Officer_Leroy::QueryAnimationState(int *animationState, int *a2, int *a3, int *a4) {
}

void AIScript_Officer_Leroy::SetAnimationState(int animationState, int a2, int a3, int a4) {
}

bool AIScript_Officer_Leroy::ReachedMovementTrackWaypoint() {
	return false;
}
} // End of namespace BladeRunner
