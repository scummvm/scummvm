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

AIScriptHolloway::AIScriptHolloway(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = 0;
}

void AIScriptHolloway::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = 0;

	Actor_Set_Goal_Number(kActorHolloway, 0);
}

bool AIScriptHolloway::Update() {
	if (Actor_Query_Goal_Number(kActorHolloway) == 256)
		Actor_Set_Goal_Number(kActorHolloway, 257);

	return false;
}

void AIScriptHolloway::TimerExpired(int timer) {
	if (!timer) {
		AI_Countdown_Timer_Reset(kActorHolloway, 0);
		if (Global_Variable_Query(40) == 1) {
			Player_Gains_Control();
		}
		Actor_Set_Goal_Number(kActorHolloway, 251);
	}
}

void AIScriptHolloway::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorHolloway) < 245 && Actor_Query_Goal_Number(kActorHolloway) > 239) {
		Loop_Actor_Walk_To_Actor(kActorHolloway, 0, 24, 0, 0);
		Actor_Set_Goal_Number(kActorHolloway, 250);
	}
}

void AIScriptHolloway::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHolloway::ClickedByPlayer() {
	//return false;
}

void AIScriptHolloway::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptHolloway::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHolloway::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHolloway::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == kActorMcCoy && Actor_Query_Goal_Number(kActorHolloway) == 250) {
		AI_Countdown_Timer_Reset(kActorHolloway, 0);
		Actor_Set_Goal_Number(kActorHolloway, 255);
	}
}

void AIScriptHolloway::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHolloway::ShotAtAndHit() {
	return false;
}

void AIScriptHolloway::Retired(int byActorId) {
	// return false;
}

int AIScriptHolloway::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHolloway::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 240:
		Actor_Put_In_Set(kActorHolloway, kSetNR07);
		Actor_Set_At_XYZ(kActorHolloway, -102.0f, -73.5f, -233.0f, 0);
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorHolloway);
		AI_Movement_Track_Append(kActorHolloway, 336, 1);
		AI_Movement_Track_Repeat(kActorHolloway);
		break;

	case 241:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorHolloway);
		AI_Movement_Track_Append(kActorHolloway, 375, 0);
		AI_Movement_Track_Append(kActorHolloway, 376, 0);
		AI_Movement_Track_Repeat(kActorHolloway);
		break;

	case 242:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorHolloway);
		AI_Movement_Track_Append(kActorHolloway, 372, 0);
		AI_Movement_Track_Repeat(kActorHolloway);
		break;

	case 250:
		Scene_Exits_Disable();
		Actor_Says(kActorHolloway, 20, 3);
		Actor_Face_Actor(kActorHolloway, 0, 1);
		if (Player_Query_Combat_Mode() == 1) {
			Actor_Set_Goal_Number(kActorHolloway, 255);
		} else {
			Actor_Says(kActorHolloway, 30, 3);
			Actor_Face_Actor(kActorMcCoy, kActorHolloway, 1);
			AI_Countdown_Timer_Reset(kActorHolloway, 0);
			AI_Countdown_Timer_Start(kActorHolloway, 0, 1);
		}
		break;

	case 251:
		Actor_Face_Actor(kActorMcCoy, kActorHolloway, 1);
		Actor_Says(kActorMcCoy, 6130, 15);
		Actor_Says(kActorHolloway, 40, 3);
		Actor_Says(kActorMcCoy, 6135, 13);
		Actor_Says(kActorHolloway, 50, 3);
		Actor_Says(kActorMcCoy, 6140, 16);
		Actor_Says(kActorHolloway, 60, 3);
		Actor_Says(kActorMcCoy, 6145, 12);
		Actor_Says(kActorHolloway, 70, 3);
		Actor_Set_Goal_Number(kActorHolloway, 255);
		break;

	case 255:
		Player_Loses_Control();
		Actor_Change_Animation_Mode(kActorHolloway, 6);
		break;

	case 257:
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Player_Gains_Control();
		Outtake_Play(kOuttakeInterrogation, 0, 1);
		if (Global_Variable_Query(40) == 1) {
			Actor_Set_Goal_Number(kActorDektora, 245);
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
		}
		Player_Gains_Control();
		Game_Flag_Set(616);
		Scene_Exits_Enable();
		Actor_Set_Goal_Number(kActorSteele, 230);
		Actor_Put_In_Set(kActorHolloway, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorHolloway, 41, 0);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		if (Global_Variable_Query(40) != 1) {
			Player_Gains_Control();
		}
		Game_Flag_Set(kFlagUG03toUG04);
		Set_Enter(kSetUG04, kSceneUG04);
		break;

	default:
		break;
	}

	return false;
}

bool AIScriptHolloway::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 717;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(717) - 1) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 719;
		_animationFrame++;
		if (_animationFrame == 9) {
			Ambient_Sounds_Play_Sound(222, 90, 99, 0, 0);
		}
		if (_animationFrame == 10) {
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
		}
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(719) - 1) {
			Actor_Change_Animation_Mode(kActorHolloway, 0);
			_animationFrame = 0;
			_animationState = 0;
			*animation = 717;
			Actor_Set_Goal_Number(kActorHolloway, 256);
		}
		break;

	case 2:
		if (!_animationFrame && _flag) {
			*animation = 717;
			_animationState = 0;
		} else {
			*animation = 720;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(720) - 1) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 721;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(721) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 720;
		}
		break;

	case 4:
		*animation = 721;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(721) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 720;
		}
		break;

	case 5:
		*animation = 721;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(721) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 720;
		}
		break;

	case 6:
		*animation = 721;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(721) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 720;
		}
		break;

	case 7:
		*animation = 716;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(716) - 1) {
			_animationFrame = 0;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptHolloway::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState > 6) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_flag = 1;
		}
		break;

	case 1:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 2;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 6:
		_animationState = 1;
		_animationFrame = 0;
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

	default:
		break;
	}

	return true;
}

void AIScriptHolloway::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHolloway::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHolloway::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHolloway::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
