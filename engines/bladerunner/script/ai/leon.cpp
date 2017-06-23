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

#include "bladerunner/script/ai.h"

namespace BladeRunner {

void AIScriptLeon::Initialize() {
	var_45EDA0_z = 0.0f;
	var_45EDA4_y = 0.0f;
	var_45EDA8_x = 0.0f;
	var_45EDAC = 0;
	var_462AF0 = 0;
	var_462AF4 = 0;
	var_45EDB4_animation_frame = 0;
	var_45EDB0_animation_state = 0;
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
	if (otherActorId == 0 && Actor_Query_Goal_Number(kActorLeon) == 7) {
		AI_Countdown_Timer_Reset(kActorLeon, 0);
		AI_Movement_Track_Flush(kActorLeon);
		AI_Movement_Track_Append(kActorLeon, 353, 0);
		AI_Movement_Track_Repeat(kActorLeon);
		//return true;
	}
	///return false;
}

void AIScriptLeon::OtherAgentExitedThisScene(int otherActorId) {
	if (otherActorId == 0 && Actor_Query_Which_Set_In(kActorLeon) == 33) {
		AI_Movement_Track_Flush(kActorLeon);
		ADQ_Flush();
		Actor_Set_Goal_Number(kActorLeon, 8);
		Actor_Set_Goal_Number(kActorDeskClerk, 1);
		//return true;
	}
	///return false;
}

void AIScriptLeon::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == 0 && combatMode == 1 && Actor_Query_Goal_Number(kActorLeon) == 1) {
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

void AIScriptLeon::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorLeon) != 7) {
		Actor_Set_Goal_Number(kActorLeon, 7);
	}
}

void AIScriptLeon::Retired(int byActorId) {}

int AIScriptLeon::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptLeon::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 8:
		Actor_Put_In_Set(kActorLeon, 91);
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
			Actor_Says_With_Pause(kActorMcCoy, 570, 0.0, 17);
			Actor_Says_With_Pause(kActorMcCoy, 575, 1.2, 13);
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
		Actor_Put_In_Set(kActorLeon, 31);
		Actor_Set_At_XYZ(kActorLeon, 264.0f, 348.52f, 827.0f, 0);
		Actor_Face_Actor(kActorDeskClerk, kActorLeon, true);
		ADQ_Add(kActorLeon, 0, 16);
		ADQ_Add(kActorDeskClerk, 0, 58);
		ADQ_Add(kActorLeon, 10, 15);
		ADQ_Add(kActorDeskClerk, 10, 58);
		ADQ_Add(kActorLeon, 20, 16);
		return true;
	case 0:
		Actor_Put_In_Set(kActorLeon, 91);
		Actor_Change_Animation_Mode(kActorLeon, 0);
		return true;
	}
	return false;
}

bool AIScriptLeon::UpdateAnimation(int *animation, int *frame) {

	switch (var_45EDB0_animation_state) {
	case 10:
		*animation = 856;
		if (var_45EDB4_animation_frame++ == 7) {
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Actor_Retired_Here(kActorMcCoy, 12, 12, 1, -1);
		}
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			*animation = 847;
			var_45EDB4_animation_frame = 0;
			Actor_Change_Animation_Mode(kActorLeon, 0);
		}
		break;
	case 9:
		*animation = 849;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(849) - 1) {
			Actor_Change_Animation_Mode(kActorLeon, 0);
			*animation = 847;
			var_45EDB4_animation_frame = 0;
			var_45EDB0_animation_state = 0;
			if (Actor_Query_Goal_Number(kActorLeon) == 2) {
				Actor_Set_Goal_Number(kActorLeon, 3);
			}
		}
		break;
	case 8:
		*animation = 854;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(854) - 1) {
			var_45EDB4_animation_frame = 0;
		}
		break;
	case 7:
		*animation = 855;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(855) - 1) {
			var_45EDB4_animation_frame = 0;
			var_45EDB0_animation_state = 6;
			*animation = 854;
		}
		break;
	case 6:
		if (var_45EDB4_animation_frame == 0 && var_45EDAC != 0) {
			Actor_Change_Animation_Mode(kActorLeon, 72);
			*animation = 848;
		} else {
			*animation = 854;
			var_45EDB4_animation_frame++;
			if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(854) - 1) {
				var_45EDB4_animation_frame = 0;
			}
		}
		break;
	case 5:
		*animation = 853;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(853) - 1) {
			var_45EDB4_animation_frame = 0;
			var_45EDB0_animation_state = 2;
			*animation = 850;
		}
		break;
	case 4:
		*animation = 852;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(852) - 1) {
			var_45EDB4_animation_frame = 0;
			var_45EDB0_animation_state = 2;
			*animation = 850;
		}
		break;
	case 3:
		*animation = 851;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(851) - 1) {
			var_45EDB4_animation_frame = 0;
			var_45EDB0_animation_state = 2;
			*animation = 850;
		}
		break;
	case 2:
		if (!var_45EDB4_animation_frame && var_45EDAC) {
			*animation = 847;
			var_45EDB0_animation_state = 0;
		} else {
			*animation = 850;
			var_45EDB4_animation_frame++;
			if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(850) - 1) {
				var_45EDB4_animation_frame = 0;
			}
		}
		break;
	case 1:
		*animation = 846;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(846) - 1) {
			var_45EDB4_animation_frame = 0;
		}
		break;
	case 0:
		*animation = 847;
		var_45EDB4_animation_frame++;
		if (var_45EDB4_animation_frame > Slice_Animation_Query_Number_Of_Frames(847) - 1) {
			var_45EDB4_animation_frame = 0;
		}
		break;
	}
	*frame = var_45EDB4_animation_frame;
	return true;
}

bool AIScriptLeon::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		switch (var_45EDB0_animation_state) {
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
			var_45EDB0_animation_state = 0;
			var_45EDB4_animation_frame = 0;
			break;
		}
		break;
	case 1:
		var_45EDB0_animation_state = 1;
		var_45EDB4_animation_frame = 0;
		break;
	case 3:
		var_45EDB0_animation_state = 2;
		var_45EDB4_animation_frame = 0;
		var_45EDAC = 0;
		break;
	case 6:
		var_45EDB0_animation_state = 10;
		var_45EDB4_animation_frame = 0;
		break;
	case 12:
		var_45EDB0_animation_state = 3;
		var_45EDB4_animation_frame = 0;
		var_45EDAC = 0;
		break;
	case 13:
		var_45EDB0_animation_state = 4;
		var_45EDB4_animation_frame = 0;
		var_45EDAC = 0;
		break;
	case 14:
		var_45EDB0_animation_state = 5;
		var_45EDB4_animation_frame = 0;
		var_45EDAC = 0;
		break;
	case 15:
		var_45EDB0_animation_state = 6;
		var_45EDB4_animation_frame = 0;
		var_45EDAC = 0;
		break;
	case 16:
		var_45EDB0_animation_state = 7;
		var_45EDB4_animation_frame = 0;
		var_45EDAC = 0;
		break;
	case 26:
		var_45EDB0_animation_state = 9;
		var_45EDB4_animation_frame = 0;
		break;
	case 72:
		if (var_45EDB0_animation_state != 8) {
			var_45EDB0_animation_state = 8;
			var_45EDB4_animation_frame = 0;
		}
		break;
	}
	return true;
}

void AIScriptLeon::QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4) {
	*animationState = var_45EDB0_animation_state;
	*animationFrame = var_45EDB4_animation_frame;
	*a3 = var_462AF0;
	*a4 = var_462AF4;
}

void AIScriptLeon::SetAnimationState(int animationState, int animationFrame, int a3, int a4) {
	var_45EDB0_animation_state = animationState;
	var_45EDB4_animation_frame = animationFrame;
	var_462AF0 = a3;
	var_462AF4 = a4;
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
	return sqrtf((z - actorZ) * (z - actorZ) + (y - actorX) * (y - actorX) + (y - actorY) * (y - actorY));
}

} // End of namespace BladeRunner
