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

AIScriptBryant::AIScriptBryant(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptBryant::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	World_Waypoint_Set(324, 22, 267.0, -60.30, 203.0);
	World_Waypoint_Set(325, 22, 84.0, -60.30, 337.0);
	World_Waypoint_Set(326, 2, -36.0, 0.0, 185.0);
	World_Waypoint_Set(327, 2, -166.0, 0.0, -103.0);
	World_Waypoint_Set(328, 3, -556.0, 252.59, -1018.11);
	World_Waypoint_Set(329, 3, -655.0, 252.60, -1012.0);
	World_Waypoint_Set(330, 3, -657.0, 253.0, -1127.0);
	World_Waypoint_Set(331, 102, 163.8, 0.0, 67.0);
	World_Waypoint_Set(332, 2, -39.0, 0.0, 11.5);
	World_Waypoint_Set(333, 102, -34.0, 0.0, 33.0);
	World_Waypoint_Set(334, 22, 3.0, -60.30, -144.0);
	World_Waypoint_Set(335, 102, -50.0, 0.0, 212.0);
	Actor_Put_In_Set(kActorBryant, 22);
	Actor_Set_Goal_Number(kActorBryant, 100);
}

bool AIScriptBryant::Update() {
	if (Global_Variable_Query(kVariableChapter) == 2 && Actor_Query_Goal_Number(kActorBryant) <= 101 && Player_Query_Current_Scene() == kSceneBB05) {
		Actor_Set_Goal_Number(kActorBryant, 101);
		return true;
	} else if (Global_Variable_Query(kVariableChapter) != 3 || Game_Flag_Query(686) || Player_Query_Current_Scene() != kSceneBB05) {
		return false;
	} else {
		Game_Flag_Set(686);
		return true;
	}
}

void AIScriptBryant::TimerExpired(int timer) {
	//return false;
}

void AIScriptBryant::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorBryant) == 101) {
		Actor_Set_Goal_Number(kActorBryant, 102);
		//result = true;
	} else if (Actor_Query_Goal_Number(kActorBryant) == 102) {
		Actor_Set_Goal_Number(kActorBryant, 101);
		//result = true;
	} else {
		//result = false;
	}
}

void AIScriptBryant::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptBryant::ClickedByPlayer() {
	Actor_Face_Actor(kActorMcCoy, kActorBryant, true);
	Actor_Voice_Over(30, kActorVoiceOver);
	Actor_Voice_Over(40, kActorVoiceOver);
}

void AIScriptBryant::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptBryant::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptBryant::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptBryant::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptBryant::ShotAtAndMissed() {
	// return false;
}

bool AIScriptBryant::ShotAtAndHit() {
	AI_Movement_Track_Flush(kActorBryant);
	Sound_Play(3, 100, 0, 0, 50);

	return false;
}

void AIScriptBryant::Retired(int byActorId) {
	// return false;
}

int AIScriptBryant::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptBryant::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if ((newGoalNumber - 101) <= 1) {
		if (newGoalNumber == 101) {
			AI_Movement_Track_Flush(kActorBryant);

			switch (Random_Query(0, 9)) {
			case 0:
				AI_Movement_Track_Append(kActorBryant, 134, 0);
				AI_Movement_Track_Append(kActorBryant, 135, 0);
				AI_Movement_Track_Append(kActorBryant, 136, 0);
				AI_Movement_Track_Append(kActorBryant, 137, 0);
				AI_Movement_Track_Append(kActorBryant, 138, 0);
				AI_Movement_Track_Append(kActorBryant, 139, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 1:
				AI_Movement_Track_Append(kActorBryant, 139, 0);
				AI_Movement_Track_Append(kActorBryant, 138, 0);
				AI_Movement_Track_Append(kActorBryant, 137, 0);
				AI_Movement_Track_Append(kActorBryant, 136, 0);
				AI_Movement_Track_Append(kActorBryant, 135, 0);
				AI_Movement_Track_Append(kActorBryant, 134, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 2:
				AI_Movement_Track_Append(kActorBryant, 196, 0);
				AI_Movement_Track_Append(kActorBryant, 197, 0);
				AI_Movement_Track_Append(kActorBryant, 198, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 3:
				AI_Movement_Track_Append(kActorBryant, 198, 0);
				AI_Movement_Track_Append(kActorBryant, 197, 0);
				AI_Movement_Track_Append(kActorBryant, 196, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 4:
				AI_Movement_Track_Append(kActorBryant, 326, 0);
				AI_Movement_Track_Append(kActorBryant, 332, 0);
				AI_Movement_Track_Append(kActorBryant, 327, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 5:
				AI_Movement_Track_Append(kActorBryant, 327, 0);
				AI_Movement_Track_Append(kActorBryant, 332, 0);
				AI_Movement_Track_Append(kActorBryant, 326, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 6:
				AI_Movement_Track_Append(kActorBryant, 329, 0);
				AI_Movement_Track_Append(kActorBryant, 328, 0);
				AI_Movement_Track_Append(kActorBryant, 330, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 7:
				AI_Movement_Track_Append(kActorBryant, 330, 0);
				AI_Movement_Track_Append(kActorBryant, 328, 0);
				AI_Movement_Track_Append(kActorBryant, 329, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 8:
				AI_Movement_Track_Append(kActorBryant, 331, 0);
				AI_Movement_Track_Append(kActorBryant, 333, 0);
				AI_Movement_Track_Append(kActorBryant, 335, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			case 9:
				AI_Movement_Track_Append(kActorBryant, 335, 0);
				AI_Movement_Track_Append(kActorBryant, 333, 0);
				AI_Movement_Track_Append(kActorBryant, 331, 0);
				AI_Movement_Track_Repeat(kActorBryant);
				break;
			default:
				break;
			}
		}

		AI_Movement_Track_Append(kActorBryant, 39, 0);
		AI_Movement_Track_Repeat(kActorBryant);
	}

	return false;
}

bool AIScriptBryant::UpdateAnimation(int *animation, int *frame) {
	if (_animationState <= 3) {
		if (_animationState) {
			if (_animationState == 1) {
				*animation = 805;
				_animationFrame++;

				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(805)) {
					_animationFrame = 0;
				}
			} else if (_animationState == 2) {
				*animation = 808;
				_animationFrame++;

				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(808) - 1) {
					_animationState = 3;
				}
			} else {
				*animation = 808;
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(808) - 1;
			}
		} else {
			*animation = 806;
			_animationFrame++;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(806)) {
				_animationFrame = 0;
			}
		}
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptBryant::ChangeAnimationMode(int mode) {
	if (mode > 1) {
		if (mode == 48) {
			_animationState = 2;
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorBryant, 0);
		}
	} else {
		_animationState = (mode != 0);
		_animationFrame = 0;
	}

	return true;
}

void AIScriptBryant::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptBryant::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptBryant::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptBryant::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
