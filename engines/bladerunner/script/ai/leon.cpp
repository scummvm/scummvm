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

AIScriptLeon::AIScriptLeon(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	var_45EDA0_z = 0.0f;
	var_45EDA4_y = 0.0f;
	var_45EDA8_x = 0.0f;
	var_45EDAC = 0;
}

void AIScriptLeon::Initialize() {
	var_45EDA0_z = 0.0f;
	var_45EDA4_y = 0.0f;
	var_45EDA8_x = 0.0f;
	var_45EDAC = 0;
	_animationStateNext = 0;
	_animationNext = 0;
	_animationFrame = 0;
	_animationState = 0;
}

bool AIScriptLeon::Update() {
	int goalNumber = Actor_Query_Goal_Number(kActorLeon);
	if (goalNumber == 3) {
		Actor_Set_Goal_Number(kActorLeon, 4);
		return true;
	}
	if (goalNumber == 5) {
		if (Player_Query_Combat_Mode() == 1) {
			Actor_Set_Goal_Number(kActorLeon, 7);
			return true;
		}
		if (Actor_Query_Inch_Distance_From_Actor(kActorLeon, 0) <= 36 && !Player_Query_Combat_Mode()) {
			Actor_Set_Goal_Number(kActorLeon, 6);
			return true;
		}
		if (sub_446700(kActorLeon, var_45EDA8_x, var_45EDA4_y, var_45EDA0_z) > 12.0f) {
			Actor_Query_XYZ(kActorMcCoy, &var_45EDA8_x, &var_45EDA4_y, &var_45EDA0_z);
			Async_Actor_Walk_To_XYZ(kActorLeon, var_45EDA8_x, var_45EDA4_y, var_45EDA0_z, 24, false);
		}
		return true;
	}
	return false;
}

void AIScriptLeon::TimerExpired(int timer) {
	if (timer == 0 && Actor_Query_Goal_Number(kActorLeon) == 7) {
		AI_Countdown_Timer_Reset(kActorLeon, 0);
		Actor_Set_Goal_Number(kActorLeon, 8);
	}
}

void AIScriptLeon::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorLeon) == 7) {
		AI_Countdown_Timer_Reset(kActorLeon, 0);
		AI_Countdown_Timer_Start(kActorLeon, 0, 8);
		//return true;
	}
	//return false;
}

void AIScriptLeon::ReceivedClue(int clueId, int fromActorId) {}

void AIScriptLeon::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorLeon) == 1) {
		Game_Flag_Set(539);
		Player_Loses_Control();
		Actor_Face_Actor(kActorMcCoy, kActorLeon, true);
		ADQ_Flush();
		Actor_Says(kActorMcCoy, 495, 3);
		Actor_Set_Goal_Number(kActorLeon, 2);
		//return true;
	}
	//return false;
}

void AIScriptLeon::EnteredScene(int sceneId) {}

void AIScriptLeon::OtherAgentEnteredThisScene(int otherActorId) {
	if (otherActorId == kActorMcCoy && Actor_Query_Goal_Number(kActorLeon) == 7) {
		AI_Countdown_Timer_Reset(kActorLeon, 0);
		AI_Movement_Track_Flush(kActorLeon);
		AI_Movement_Track_Append(kActorLeon, 353, 0);
		AI_Movement_Track_Repeat(kActorLeon);
		//return true;
	}
	///return false;
}

void AIScriptLeon::OtherAgentExitedThisScene(int otherActorId) {
	if (otherActorId == kActorMcCoy && Actor_Query_Which_Set_In(kActorLeon) == 33) {
		AI_Movement_Track_Flush(kActorLeon);
		ADQ_Flush();
		Actor_Set_Goal_Number(kActorLeon, 8);
		Actor_Set_Goal_Number(kActorDeskClerk, 1);
		//return true;
	}
	///return false;
}

void AIScriptLeon::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == kActorMcCoy && combatMode == 1 && Actor_Query_Goal_Number(kActorLeon) == 1) {
		Game_Flag_Set(539);
		Player_Loses_Control();
		Actor_Face_Actor(kActorMcCoy, kActorLeon, true);
		Actor_Says(kActorMcCoy, 500, 5);
		Actor_Set_Goal_Number(kActorLeon, 2);
		//return true;
	}
	//return false;
}

void AIScriptLeon::ShotAtAndMissed() {}

bool AIScriptLeon::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorLeon) != 7) {
		Actor_Set_Goal_Number(kActorLeon, 7);
	}
	return false;
}

void AIScriptLeon::Retired(int byActorId) {}

int AIScriptLeon::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptLeon::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 8:
		Actor_Put_In_Set(kActorLeon, kSetFreeSlotA);
		AI_Movement_Track_Flush(kActorLeon);
		return true;
	case 7:
		Actor_Force_Stop_Walking(kActorLeon);
		AI_Movement_Track_Flush(kActorLeon);
		AI_Movement_Track_Append(kActorLeon, 351, 0);
		AI_Movement_Track_Append(kActorLeon, 352, 0);
		AI_Movement_Track_Repeat(kActorLeon);
		return true;
	case 6:
		Player_Loses_Control();
		Actor_Says(kActorLeon, 40, 3);
		Actor_Says(kActorMcCoy, 510, 3);
		Actor_Says(kActorLeon, 50, 3);
		Actor_Change_Animation_Mode(kActorLeon, 6);
		return false;
	case 5:
		return true;
	case 4:
		Actor_Face_Actor(kActorLeon, kActorMcCoy, true);
		Player_Set_Combat_Mode(false);
		Actor_Says(kActorLeon, 30, 12);
		Actor_Face_Actor(kActorMcCoy, kActorLeon, true);
		if (Player_Query_Combat_Mode() == 1) {
			Player_Set_Combat_Mode(false);
		}
		Actor_Says(kActorMcCoy, 505, 23);
		Actor_Says(kActorLeon, 60, 13);
		Player_Gains_Control();
		Loop_Actor_Walk_To_XYZ(kActorLeon, 233.0f, 349.0f, 849.0f, 0, 0, false, 0);
		Actor_Face_Actor(kActorLeon, 0, true);
		Actor_Face_Actor(kActorMcCoy, kActorLeon, true);
		Actor_Says(kActorMcCoy, 515, 18);
		Actor_Says_With_Pause(kActorLeon, 70, 0.3f, 12);
		Actor_Says(kActorMcCoy, 520, 15);
		Actor_Says(kActorLeon, 80, 12);
		Loop_Actor_Walk_To_XYZ(kActorLeon, 198.0f, 349.0f, 865.0f, 0, 0, false, 0);
		Actor_Face_Actor(kActorLeon, 0, true);
		Actor_Face_Actor(kActorMcCoy, kActorLeon, true);
		if (Actor_Clue_Query(kActorMcCoy, kClueWantedPoster)) {
			Actor_Says_With_Pause(kActorMcCoy, 525, 0.2f, 14);
			Actor_Says(kActorLeon, 90, 13);
			Actor_Says(kActorMcCoy, 530, 16);
			Actor_Set_Goal_Number(kActorLeon, 5);
		} else {
			Actor_Says_With_Pause(kActorMcCoy, 535, 0.8f, 17);
			Actor_Says(kActorLeon, 100, 13);
			Actor_Says_With_Pause(kActorMcCoy, 540, 0.0f, 19);
			Actor_Says(kActorLeon, 110, 3);
			Actor_Says(kActorMcCoy, 550, 17);
			Actor_Says(kActorMcCoy, 555, 18);
			Actor_Says(kActorLeon, 120, 13);
			Actor_Says(kActorMcCoy, 560, 3);
			Actor_Says_With_Pause(kActorLeon, 130, 0.8f, 3);
			Actor_Face_Current_Camera(kActorMcCoy, true);
			Actor_Says(kActorMcCoy, 565, 18);
			Actor_Face_Actor(kActorMcCoy, kActorLeon, true);
			Actor_Says(kActorLeon, 140, 12);
			Actor_Says_With_Pause(kActorMcCoy, 570, 0.0f, 17);
			Actor_Says_With_Pause(kActorMcCoy, 575, 1.2f, 13);
			Actor_Says(kActorLeon, 150, 3);
			Actor_Says(kActorMcCoy, 580, 3);
			Actor_Says(kActorLeon, 160, 13);
			Actor_Says(kActorLeon, 170, 12);
			Actor_Set_Goal_Number(kActorLeon, 7);
		}
		return true;
	case 2:
		Actor_Change_Animation_Mode(kActorLeon, 26);
		Actor_Change_Animation_Mode(kActorDeskClerk, 26);
		return true;
	case 1:
		Actor_Change_Animation_Mode(kActorDeskClerk, 72);
		Actor_Change_Animation_Mode(kActorLeon, 72);
		Actor_Put_In_Set(kActorLeon, kSetCT09);
		Actor_Set_At_XYZ(kActorLeon, 264.0f, 348.52f, 827.0f, 0);
		Actor_Face_Actor(kActorDeskClerk, kActorLeon, true);
		ADQ_Add(kActorLeon, 0, 16);
		ADQ_Add(kActorDeskClerk, 0, 58);
		ADQ_Add(kActorLeon, 10, 15);
		ADQ_Add(kActorDeskClerk, 10, 58);
		ADQ_Add(kActorLeon, 20, 16);
		return true;
	case 0:
		Actor_Put_In_Set(kActorLeon, kSetFreeSlotA);
		Actor_Change_Animation_Mode(kActorLeon, 0);
		return true;
	}
	return false;
}

bool AIScriptLeon::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 10:
		*animation = 856;
		if (_animationFrame++ == 7) {
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Actor_Retired_Here(kActorMcCoy, 12, 12, 1, -1);
		}
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			*animation = 847;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorLeon, 0);
		}
		break;
	case 9:
		*animation = 849;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(849) - 1) {
			Actor_Change_Animation_Mode(kActorLeon, 0);
			*animation = 847;
			_animationFrame = 0;
			_animationState = 0;
			if (Actor_Query_Goal_Number(kActorLeon) == 2) {
				Actor_Set_Goal_Number(kActorLeon, 3);
			}
		}
		break;
	case 8:
		*animation = 854;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(854) - 1) {
			_animationFrame = 0;
		}
		break;
	case 7:
		*animation = 855;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(855) - 1) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 854;
		}
		break;
	case 6:
		if (_animationFrame == 0 && var_45EDAC != 0) {
			Actor_Change_Animation_Mode(kActorLeon, 72);
			*animation = 848;
		} else {
			*animation = 854;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(854) - 1) {
				_animationFrame = 0;
			}
		}
		break;
	case 5:
		*animation = 853;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(853) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 850;
		}
		break;
	case 4:
		*animation = 852;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(852) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 850;
		}
		break;
	case 3:
		*animation = 851;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(851) - 1) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 850;
		}
		break;
	case 2:
		if (!_animationFrame && var_45EDAC) {
			*animation = 847;
			_animationState = 0;
		} else {
			*animation = 850;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(850) - 1) {
				_animationFrame = 0;
			}
		}
		break;
	case 1:
		*animation = 846;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(846) - 1) {
			_animationFrame = 0;
		}
		break;
	case 0:
		*animation = 847;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(847) - 1) {
			_animationFrame = 0;
		}
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptLeon::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			var_45EDAC = 1;
			break;
		case 8:
			Actor_Change_Animation_Mode(kActorLeon, 72);
			break;
		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;
	case kAnimationModeWalk:
		_animationState = 1;
		_animationFrame = 0;
		break;
	case kAnimationModeTalk:
		_animationState = 2;
		_animationFrame = 0;
		var_45EDAC = 0;
		break;
	case kAnimationModeCombatAttack:
		_animationState = 10;
		_animationFrame = 0;
		break;
	case 12:
		_animationState = 3;
		_animationFrame = 0;
		var_45EDAC = 0;
		break;
	case 13:
		_animationState = 4;
		_animationFrame = 0;
		var_45EDAC = 0;
		break;
	case 14:
		_animationState = 5;
		_animationFrame = 0;
		var_45EDAC = 0;
		break;
	case 15:
		_animationState = 6;
		_animationFrame = 0;
		var_45EDAC = 0;
		break;
	case 16:
		_animationState = 7;
		_animationFrame = 0;
		var_45EDAC = 0;
		break;
	case 26:
		_animationState = 9;
		_animationFrame = 0;
		break;
	case 72:
		if (_animationState != 8) {
			_animationState = 8;
			_animationFrame = 0;
		}
		break;
	}
	return true;
}

void AIScriptLeon::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptLeon::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptLeon::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 353) {
		Actor_Set_Goal_Number(kActorLeon, 8);
	}
	return true;
}

void AIScriptLeon::FledCombat() {}

float AIScriptLeon::sub_446700(int actorId, float x, float y, float z) {
	float actorX, actorY, actorZ;
	Actor_Query_XYZ(actorId, &actorX, &actorY, &actorZ);
	return sqrt(static_cast<float>((z - actorZ) * (z - actorZ) + (y - actorY) * (y - actorY) + (x - actorX) * (x - actorX)));
}

} // End of namespace BladeRunner
