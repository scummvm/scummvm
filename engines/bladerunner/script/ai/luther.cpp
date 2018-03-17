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

AIScriptLuther::AIScriptLuther(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
}

void AIScriptLuther::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;

	Actor_Put_In_Set(kActorLuther, kSetUG16);
	Actor_Set_At_XYZ(kActorLuther, 176.91f, -40.67f, 225.92f, 486);
	Actor_Set_Goal_Number(kActorLuther, 400);
	Actor_Set_Targetable(kActorLuther, 1);
}

bool AIScriptLuther::Update() {
	if (!Actor_Query_Is_In_Current_Set(kActorLuther)
			|| Player_Query_Combat_Mode() != 1
			|| Global_Variable_Query(29)
			|| Game_Flag_Query(596)
			|| Global_Variable_Query(kVariableChapter) != 4) {
		if (Actor_Query_Goal_Number(kActorLuther) == 400 && Actor_Query_Goal_Number(kActorLuther) != 499) {
			Actor_Set_Goal_Number(kActorLuther, 401);
		} else if (Actor_Query_Goal_Number(kActorLuther) == 494) {
			Actor_Set_Goal_Number(kActorLuther, 495);
			ChangeAnimationMode(48);
		} else if (Actor_Query_Goal_Number(kActorLuther) != 495 || Game_Flag_Query(587)) {
			if (Actor_Query_Goal_Number(kActorLuther) != 497
					|| Global_Variable_Query(29) >= 2
					|| Game_Flag_Query(568)) {
				if (Actor_Query_Goal_Number(kActorLuther) != 497
						|| Global_Variable_Query(29) <= 1
						|| Game_Flag_Query(568)) {
					if (Actor_Query_Goal_Number(kActorLuther) == 498) {
						Game_Flag_Set(595);
						Actor_Set_Goal_Number(kActorLuther, 499);
						Actor_Set_Targetable(kActorLuther, 0);
					} else {
						return false;
					}
				} else {
					Actor_Set_Targetable(kActorLuther, 0);
					Actor_Set_Goal_Number(kActorLuther, 498);
					Actor_Set_Targetable(kActorLuther, 0);
				}
			} else {
				Game_Flag_Set(568);
				ChangeAnimationMode(50);
				ChangeAnimationMode(48);
				Actor_Set_Goal_Number(kActorLuther, 498);
				Actor_Set_Targetable(kActorLuther, 0);
				Scene_Loop_Set_Default(5);
				Scene_Loop_Start_Special(2, 4, 1);
				Ambient_Sounds_Play_Sound(559, 50, 0, 0, 99);
				Ambient_Sounds_Remove_Looping_Sound(516, 1);
			}
		} else {
			AI_Countdown_Timer_Reset(kActorLuther, 2);
			AI_Countdown_Timer_Start(kActorLuther, 2, 5);
			Actor_Set_Goal_Number(kActorLuther, 496);
			Game_Flag_Set(587);
		}
	} else {
		Actor_Says(kActorMcCoy, 5720, 12);
		Actor_Says(kActorLuther, 80, 13);
		Actor_Says(kActorLance, 40, 12);
		Game_Flag_Set(596);
	}

	return false;
}

void AIScriptLuther::TimerExpired(int timer) {
	if (timer != 2)
		return; //false;

	AI_Countdown_Timer_Reset(kActorLuther, 2);
	Actor_Set_Goal_Number(kActorLuther, 497);

	return; //true;
}

void AIScriptLuther::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorLuther) != 401)
		return; //false;

	Actor_Set_Goal_Number(kActorLuther, 402);

	return; //true;
}

void AIScriptLuther::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptLuther::ClickedByPlayer() {
	//return false;
}

void AIScriptLuther::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptLuther::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptLuther::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptLuther::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptLuther::ShotAtAndMissed() {
	// return false;
}

bool AIScriptLuther::ShotAtAndHit() {
	if (Actor_Query_Which_Set_In(kActorLuther) == 19) {
		Actor_Set_Health(kActorLuther, 50, 50);
	}
	Global_Variable_Increment(29, 1);
	Music_Stop(2);
	if (Global_Variable_Query(29) <= 0) {
		return false;
	}
	if (!Game_Flag_Query(560)) {
		Game_Flag_Set(557);
	}
	Actor_Set_Goal_Number(kActorLuther, 494);

	return true;
}

void AIScriptLuther::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorLuther, 599);
}

int AIScriptLuther::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptLuther::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 401:
		AI_Movement_Track_Flush(kActorLuther);
		AI_Movement_Track_Append(kActorLuther, 39, 20);
		AI_Movement_Track_Append_With_Facing(kActorLuther, 368, 120, 486);
		AI_Movement_Track_Append(kActorLuther, 40, 10);
		AI_Movement_Track_Repeat(kActorLuther);
		break;

	case 402:
		Actor_Set_Goal_Number(kActorLuther, 401);
		break;

	case 403:
		AI_Movement_Track_Flush(kActorLuther);
		break;

	case 499:
		Actor_Set_Goal_Number(kActorLuther, 599);
		break;
	}

	return false;
}

bool AIScriptLuther::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 346;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(346) - 1) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 348;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(348) - 1) {
			*animation = 346;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorLuther, 0);
		}
		break;

	case 2:
		if (!_animationFrame && _flag) {
			*animation = 346;
			_animationState = 0;
		} else {
			*animation = 349;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(349) - 1) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 350;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(350) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 349;
		}
		break;

	case 4:
		*animation = 351;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(351) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 349;
		}
		break;

	case 5:
		*animation = 352;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(352) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 349;
		}
		break;

	case 6:
		*animation = 353;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(353) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 349;
		}
		break;

	case 7:
		*animation = 354;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(354) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 349;
		}
		break;

	case 8:
		*animation = 355;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(355) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 349;
		}
		break;

	case 9:
		*animation = 356;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(356) - 1) {
			*animation = 346;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorLuther, 0);
		}
		break;

	case 10:
		*animation = 357;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(357) - 1) {
			Actor_Change_Animation_Mode(kActorLuther, 50);
			*animation = 358;
			_animationFrame = 0;
		}
		break;

	case 11:
		*animation = 358;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(358) - 1) {
			_animationFrame++;
		}
		break;

	case 12:
		*animation = 359;
		if (_animationFrame == 12) {
			Ambient_Sounds_Play_Sound(557, 59, 0, 0, 20);
		}
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			_animationFrame++;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptLuther::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if ((unsigned int)(_animationState - 2) > 6) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_flag = 1;
		}
		break;

	case 3:
		_animationState = 2;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 6:
		_animationState = 9;
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

	case 23:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 48:
		_animationState = 12;
		_animationFrame = 0;
		break;

	case 50:
		_animationState = 11;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptLuther::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptLuther::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptLuther::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptLuther::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
