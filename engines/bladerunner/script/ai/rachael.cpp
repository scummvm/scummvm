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

AIScriptRachael::AIScriptRachael(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = true;
}

void AIScriptRachael::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = true;

	Actor_Set_Goal_Number(kActorRachael, 0);
}

bool AIScriptRachael::Update() {
	return false;
}

void AIScriptRachael::TimerExpired(int timer) {
	//return false;
}

void AIScriptRachael::CompletedMovementTrack() {
	//return false;
}

void AIScriptRachael::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptRachael::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorRachael) == 305) {
		Actor_Face_Actor(kActorMcCoy, kActorRachael, 1);
		Actor_Says(kActorMcCoy, 2730, 12);
		AI_Movement_Track_Pause(57);
		dialogue_start();

		if (Player_Query_Agenda() == kPlayerAgendaSurly || Player_Query_Agenda() == kPlayerAgendaErratic) {
			dialogue_agenda2();
		} else if (Player_Query_Agenda()) {
			if (Actor_Query_Friendliness_To_Other(kActorSteele, kActorMcCoy) > Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy)) {
				dialogue_agenda2();
			} else {
				dialogue_agenda1();
			}
		} else {
			dialogue_agenda1();
		}

		Actor_Set_Goal_Number(kActorRachael, 306);
		AI_Movement_Track_Unpause(57);
	}
}

void AIScriptRachael::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptRachael::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptRachael::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptRachael::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptRachael::ShotAtAndMissed() {
	// return false;
}

bool AIScriptRachael::ShotAtAndHit() {
	return false;
}

void AIScriptRachael::Retired(int byActorId) {
	// return false;
}

int AIScriptRachael::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptRachael::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 200:
		AI_Movement_Track_Flush(kActorRachael);
		AI_Movement_Track_Append(kActorRachael, 379, 0);
		AI_Movement_Track_Append(kActorRachael, 39, 0);
		AI_Movement_Track_Repeat(kActorRachael);
		break;

	case 300:
		Actor_Put_In_Set(kActorRachael, kSetMA07);
		Actor_Set_At_XYZ(kActorRachael, -8.09f, -162.8f, 135.33f, 544);
		break;

	case 305:
		AI_Movement_Track_Flush(kActorRachael);
		AI_Movement_Track_Append(kActorRachael, 468, 0);
		AI_Movement_Track_Append(kActorRachael, 39, 0);
		AI_Movement_Track_Repeat(kActorRachael);
		break;

	case 400:
		Actor_Put_In_Set(kActorRachael, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorRachael, 39, 0);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptRachael::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 823;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(823) - 1) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 822;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(822) - 1) {
			_animationFrame = 0;
		}
		break;

	case 2:
		if (!_animationFrame && _flag) {
			*animation = 823;
			_animationState = 0;
		} else {
			*animation = 825;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(825) - 1) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 826;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(826) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	case 4:
		*animation = 827;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(827) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	case 5:
		*animation = 828;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(828) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	case 6:
		*animation = 829;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(829) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	case 7:
		*animation = 830;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(830) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	case 8:
		*animation = 831;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(831) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	case 9:
		*animation = 832;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(832) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 825;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptRachael::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 2 && _animationState <= 9) {
			_flag = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;
	case 1:
		_animationState = 1;
		_animationFrame = 0;
		break;
	case 3:
		_animationState = 2;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 12:
		_animationState = 3;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 13:
		_animationState = 4;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 14:
		_animationState = 5;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 15:
		_animationState = 6;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 16:
		_animationState = 7;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 17:
		_animationState = 8;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 18:
		_animationState = 9;
		_animationFrame = 0;
		_flag = 0;
		break;
	default:
		break;
	}

	return true;
}

void AIScriptRachael::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptRachael::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptRachael::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptRachael::FledCombat() {
	// return false;
}

void AIScriptRachael::dialogue_start() {
	Actor_Face_Actor(kActorRachael, kActorMcCoy, 1);
	Loop_Actor_Walk_To_Actor(kActorRachael, kActorMcCoy, 84, 0, 0);
	Actor_Says(kActorRachael, 0, 15);
	Actor_Says(kActorMcCoy, 2740, 13);
	Actor_Says(kActorRachael, 10, 14);
	Actor_Says(kActorMcCoy, 2745, 13);
	Actor_Says(kActorRachael, 20, 12);
	Actor_Says_With_Pause(kActorMcCoy, 2750, 1.5f, 3);
	Actor_Says(kActorRachael, 30, 13);
	Actor_Says(kActorRachael, 40, 15);
}

void AIScriptRachael::dialogue_agenda1() {
	Actor_Says(kActorMcCoy, 2795, 13);
	Actor_Says(kActorRachael, 140, 15);
	Actor_Says(kActorMcCoy, 2800, 12);
	Actor_Says(kActorRachael, 150, 16);
	Actor_Says(kActorMcCoy, 2805, 18);
	Actor_Says(kActorRachael, 160, 14);
	Actor_Says(kActorRachael, 170, 13);
	Actor_Says(kActorMcCoy, 2810, 14);
	Actor_Says(kActorRachael, 180, 13);
	Actor_Says(kActorMcCoy, 2815, 12);
	Actor_Says(kActorMcCoy, 2820, 13);
	Actor_Says(kActorRachael, 190, 15);
	Actor_Says(kActorMcCoy, 2825, 12);
	Actor_Says(kActorRachael, 200, 15);
	Actor_Says(kActorMcCoy, 2830, 14);
	Actor_Says(kActorRachael, 210, 15);
	Actor_Says(kActorRachael, 220, 16);
	Actor_Says(kActorMcCoy, 2835, 13);
	Actor_Says(kActorRachael, 230, 14);
	Actor_Says(kActorMcCoy, 2840, 12);
	Actor_Says(kActorRachael, 240, 13);
	Actor_Says(kActorRachael, 250, 15);
	Actor_Says(kActorRachael, 260, 16);
	Actor_Says(kActorMcCoy, 2845, 13);
	Actor_Says(kActorRachael, 270, 13);
	Actor_Says(kActorRachael, 280, 14);
	Actor_Says(kActorMcCoy, 2850, 13);
	Actor_Says(kActorRachael, 290, 14);
	Actor_Says_With_Pause(kActorRachael, 300, 1.0f, 3);
	Actor_Says(kActorMcCoy, 2860, 14);
}

void AIScriptRachael::dialogue_agenda2() {
	Actor_Says(kActorRachael, 50, 15);
	Actor_Says(kActorMcCoy, 2765, 16);
	Actor_Says(kActorMcCoy, 2770, 17);
	Actor_Says(kActorRachael, 60, 14);
	Actor_Says(kActorMcCoy, 2775, 16);
	Actor_Says(kActorRachael, 70, 13);
	Actor_Says(kActorRachael, 80, 14);
	Actor_Says(kActorRachael, 90, 15);
	Actor_Says(kActorMcCoy, 2780, 17);
	Actor_Says(kActorRachael, 100, 16);
	Actor_Says(kActorRachael, 110, 15);
	Actor_Says(kActorMcCoy, 2785, 17);
	Actor_Says(kActorRachael, 120, 13);
	Actor_Says(kActorMcCoy, 2790, 16);
	Actor_Says(kActorRachael, 130, 14);
	Actor_Says_With_Pause(kActorRachael, 300, 1.0f, 3);
	Actor_Says(0, 2860, 14);
}

// Not used in the game
void AIScriptRachael::dialogue_agenda3() {
	Actor_Says(kActorMcCoy, 2865, 3);
	Actor_Says(kActorRachael, 320, 3);
	Actor_Says(kActorRachael, 330, 3);
	Actor_Says(kActorMcCoy, 2870, 3);
	Actor_Says(kActorRachael, 340, 3);
	Actor_Says(kActorMcCoy, 2875, 3);
	Actor_Says(kActorRachael, 350, 3);
	Actor_Says(kActorMcCoy, 2880, 3);
	Actor_Says(kActorMcCoy, 2885, 3);
	Actor_Says(kActorRachael, 360, 3);
	Actor_Says(kActorRachael, 370, 3);
	Actor_Says(kActorMcCoy, 2890, 3);
	Actor_Says(kActorRachael, 380, 3);
	Actor_Says(kActorRachael, 390, 3);
	Actor_Says(kActorRachael, 400, 3);
	Actor_Says(kActorMcCoy, 2895, 3);
	Actor_Says(kActorRachael, 410, 3);
	Actor_Says(kActorMcCoy, 2900, 3);
	Actor_Says(kActorRachael, 420, 3);
	Actor_Says(kActorMcCoy, 2905, 3);
	Actor_Says(kActorRachael, 430, 3);
	Actor_Says(kActorRachael, 440, 3);
	Actor_Says(kActorMcCoy, 2910, 3);
	Actor_Says(kActorMcCoy, 2920, 3);
	Actor_Says(kActorRachael, 450, 3);
	Actor_Says(kActorMcCoy, 2925, 3);
	Actor_Says(kActorMcCoy, 2930, 3);
	Actor_Says(kActorRachael, 460, 3);
	Actor_Says(kActorMcCoy, 2935, 3);
	Actor_Says(kActorRachael, 470, 3);
}

} // End of namespace BladeRunner
