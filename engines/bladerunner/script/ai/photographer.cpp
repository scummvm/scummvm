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

AIScriptPhotographer::AIScriptPhotographer(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 0;
	_flag = false;
}

void AIScriptPhotographer::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 0;
	_flag = false;
}

bool AIScriptPhotographer::Update() {
	if (Game_Flag_Query(450) != 1 || Game_Flag_Query(485))
		return false;

	Actor_Put_In_Set(kActorPhotographer, kSetFreeSlotC);
	Actor_Set_At_Waypoint(kActorPhotographer, 35, 0);
	Game_Flag_Set(485);
	Actor_Set_Goal_Number(kActorPhotographer, 100);

	return true;
}

void AIScriptPhotographer::TimerExpired(int timer) {
	//return false;
}

void AIScriptPhotographer::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorPhotographer)) {
	case 100:
		Actor_Set_Goal_Number(kActorPhotographer, 101);
		break;

	case 101:
		Actor_Set_Goal_Number(kActorPhotographer, 102);
		break;

	case 102:
		Actor_Set_Goal_Number(kActorPhotographer, 101);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptPhotographer::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptPhotographer::ClickedByPlayer() {
	if (Actor_Clue_Query(kActorMcCoy, 44) != 1 || Actor_Clue_Query(kActorMcCoy, 50) || Game_Flag_Query(707)) {
		AI_Movement_Track_Pause(37);
		Actor_Face_Actor(kActorMcCoy, kActorPhotographer, 1);
		Actor_Face_Actor(kActorPhotographer, kActorMcCoy, 1);
		Actor_Says(kActorMcCoy, 5310, 11);
		Actor_Says(kActorPhotographer, 40, 3);
		AI_Movement_Track_Unpause(37);
	} else {
		AI_Movement_Track_Pause(37);
		Actor_Face_Actor(kActorMcCoy, kActorPhotographer, 1);
		Actor_Says(kActorMcCoy, 5300, 14);
		Actor_Face_Actor(kActorPhotographer, kActorMcCoy, 1);
		Actor_Says(kActorPhotographer, 20, 3);
		Actor_Says(kActorMcCoy, 5305, 15);
		Game_Flag_Set(707);
		AI_Movement_Track_Unpause(37);
	}
}

void AIScriptPhotographer::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptPhotographer::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptPhotographer::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptPhotographer::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptPhotographer::ShotAtAndMissed() {
	// return false;
}

bool AIScriptPhotographer::ShotAtAndHit() {
	return false;
}

void AIScriptPhotographer::Retired(int byActorId) {
	// return false;
}

int AIScriptPhotographer::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptPhotographer::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 100:
		AI_Movement_Track_Flush(kActorPhotographer);
		AI_Movement_Track_Append(kActorPhotographer, 35, 0);
		AI_Movement_Track_Repeat(kActorPhotographer);

		return true;

	case 101:
		AI_Movement_Track_Flush(kActorPhotographer);
		switch (Random_Query(1, 3) - 1) {
		case 0:
			AI_Movement_Track_Append(kActorPhotographer, 280, 4);
			break;

		case 1:
			AI_Movement_Track_Append(kActorPhotographer, 279, 8);
			break;

		case 2:
			AI_Movement_Track_Append(kActorPhotographer, 280, 3);
			break;
		}

		AI_Movement_Track_Repeat(kActorPhotographer);

		return false;

	case 102:
		AI_Movement_Track_Flush(kActorPhotographer);
		AI_Movement_Track_Append(kActorPhotographer, 279, 5);
		AI_Movement_Track_Repeat(kActorPhotographer);
		return 1;

	case 199:
		Actor_Put_In_Set(kActorPhotographer, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorPhotographer, 35, 0);
		Actor_Put_In_Set(kActorMarcus, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorMarcus, 41, 0);

		if (Game_Flag_Query(102)) {
			Item_Remove_From_World(103);
			Item_Remove_From_World(104);
			Item_Remove_From_World(105);
		}

		if (!Actor_Clue_Query(kActorMcCoy, 65)) {
			Actor_Clue_Acquire(kActorSteele, kClueDogCollar1, 1, -1);
			if (Game_Flag_Query(102)) {
				Item_Remove_From_World(84);
			}
			Global_Variable_Increment(14, 1);
		}

		return true;

	default:
		return true;
	}
}

bool AIScriptPhotographer::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 745;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(745)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 744;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(744)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = 747;
		if (!_animationFrame && _flag) {
			*animation = 745;
			_animationState = 0;
			_var2 = 0;
			_flag = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(747)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 749;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(749)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 747;
		}
		break;

	case 4:
		*animation = 749;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(749)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 747;
		}
		break;

	case 5:
		*animation = 750;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(750)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 747;
		}
		break;

	case 6:
		*animation = 746;
		if (_animationFrame == 11) {
			Ambient_Sounds_Play_Sound(463, 80, -20, -20, 20);
		}
		if (_var1) {
			--_var1;
		} else {
			_animationFrame++;
			if (_animationFrame == 10) {
				_var1 = 5;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(746)) {
				*animation = 745;
				_animationState = 0;
				_animationFrame = 0;
				Actor_Change_Animation_Mode(kActorPhotographer, 0);
			}
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptPhotographer::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState > 5) {
			_animationState = 0;
			_var2 = 0;
			_animationFrame = 0;
		} else {
			_flag = 1;
		}
		break;

	case 1:
		_animationState = 1;
		_var2 = 0;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 2;
		_var2 = 0;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 12:
		_animationState = 3;
		_var2 = 0;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 13:
		_animationState = 4;
		_var2 = 0;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 14:
		_animationState = 5;
		_var2 = 0;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 43:
		_animationState = 6;
		_animationFrame = 0;
		break;

	default:
		break;
	}
	return true;
}

void AIScriptPhotographer::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptPhotographer::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptPhotographer::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 276 || waypointId == 278 || waypointId == 280)
		ChangeAnimationMode(43);

	return true;
}

void AIScriptPhotographer::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
