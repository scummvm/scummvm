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
#include "bladerunner/vector.h"

namespace BladeRunner {

void AIScriptMaggie::Initialize() {
	var_45F3F0_animation_state = 0;
	var_45F3F4_animation_frame = 0;
	var_462B30 = 0;
	var_45F3F8 = 0;
	var_45F3FC = 0;
	var_45F400 = 0;
	var_45F404 = 0;
	var_45F408 = 0;
	Actor_Set_Goal_Number(kActorMaggie, 0);
}

bool AIScriptMaggie::Update() {
	int goal = Actor_Query_Goal_Number(kActorMaggie);
	if (Actor_Query_Which_Set_In(kActorMaggie) == 10 && Global_Variable_Query(1) == 4) {
		Actor_Put_In_Set(kActorMaggie, 97);
		Actor_Set_At_Waypoint(kActorMaggie, 39, 0);
	}
	if (goal == 414) {
		Actor_Set_Goal_Number(kActorMaggie, 415);
	} else if (goal == 413 && Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorMaggie) < 60) {
		Actor_Set_Goal_Number(kActorMaggie, 415);
	}
	if (Global_Variable_Query(1) == 5) {
		if (Actor_Query_Goal_Number(kActorMaggie) < 400) {
			Actor_Set_Goal_Number(66, 400);
		}
		return true;
	}
	return false;
}

void AIScriptMaggie::TimerExpired(int timer) {
	if (timer != 0) {
		return;
	}
	int goal = Actor_Query_Goal_Number(kActorMaggie);
	if (goal == 8) {
		AI_Countdown_Timer_Reset(kActorMaggie, 0);
		if (Random_Query(0, 4)) {
			AI_Movement_Track_Flush(kActorMaggie);
			AI_Movement_Track_Append(kActorMaggie, sub_44B260(), 0);
			AI_Movement_Track_Repeat(kActorMaggie);
		} else {
			Actor_Change_Animation_Mode(kActorMaggie, 54);
		}
		return; //true
	}
	if (goal == 10) {
		AI_Countdown_Timer_Reset(kActorMaggie, 0);
		Actor_Change_Animation_Mode(kActorMaggie, 55);
		return; //true
	}
	return; //false
}

void AIScriptMaggie::CompletedMovementTrack() {
	int goal = Actor_Query_Goal_Number(kActorMaggie);
	if (goal == 0 || goal > 9) {
		if (goal == 413) {
			Actor_Set_Goal_Number(kActorMaggie, 414);
			return; //true
		}
	} else {
		if (goal == 1) {
			Actor_Set_Goal_Number(kActorMaggie, 7);
			return; //true
		}
		if (goal == 8) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			AI_Countdown_Timer_Reset(kActorMaggie, 0);
			AI_Countdown_Timer_Start(kActorMaggie, 0, Random_Query(1, 5));
			return; //true
		}
		if (goal == 9) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 54);
			return; //true
		}
	}
	Actor_Set_Goal_Number(kActorMaggie, 8);
	return; //true
}

void AIScriptMaggie::ReceivedClue(int clueId, int fromActorId) {
}

void AIScriptMaggie::ClickedByPlayer() {
	if (!Game_Flag_Query(653) && Global_Variable_Query(1) == 5) {
		if (Actor_Query_Goal_Number(kActorMaggie) == 413) {
			Actor_Set_Targetable(kActorMaggie, true);
			AI_Movement_Track_Flush(kActorMaggie);
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Says(kActorMcCoy, 2400, 52);
		}
		return; // true
	}
	if (var_45F3F0_animation_state == 1 || var_45F3F0_animation_state == 12 || var_45F3F0_animation_state == 16) {
		return; // false
	}

	float mccoy_x, mccoy_y, mccoy_z;

	Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
	Actor_Query_XYZ(kActorMcCoy, &mccoy_x, &mccoy_y, &mccoy_z);
	float distance = sub_44B200(kActorMaggie, mccoy_x, mccoy_y, mccoy_z);
	if (distance > 60.0f) {
		Actor_Says(0, 2430, 18);
		Actor_Set_Goal_Number(kActorMaggie, 7);
		return; // true
	}

	int v6 = Random_Query(0, 4);
	if (v6 == 1) {
		Actor_Says(kActorMcCoy, 2440, 18);
	}
	if (v6 == 0) {
		Actor_Says(kActorMcCoy, 2435, 13);
	}

	int goal = Actor_Query_Goal_Number(kActorMaggie);
	if (goal == 8) {
		if (Random_Query(0, 1)) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 57);
		} else {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 56);
		}
		AI_Countdown_Timer_Reset(kActorMaggie, 0);
		AI_Countdown_Timer_Start(kActorMaggie, 0, Random_Query(3, 9));
		return; // true
	}
	if (goal == 10) {
		Actor_Change_Animation_Mode(kActorMaggie, 0);
		return; // true
	}
	if (goal == 11) {
		Actor_Change_Animation_Mode(kActorMaggie, 54);
		return; // true
	}
	Actor_Set_Goal_Number(kActorMaggie, 8);
	return; // true
}

void AIScriptMaggie::EnteredScene(int sceneId) {
}

void AIScriptMaggie::OtherAgentEnteredThisScene(int otherActorId) {
}

void AIScriptMaggie::OtherAgentExitedThisScene(int otherActorId) {
	if (otherActorId == kActorMcCoy && Actor_Query_Which_Set_In(kActorMaggie) == 10 && Global_Variable_Query(1) < 4) {
		AI_Movement_Track_Flush(kActorMaggie);
		Actor_Set_Goal_Number(kActorMaggie, 0);
	}
}

void AIScriptMaggie::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
}

void AIScriptMaggie::ShotAtAndMissed() {
}

void AIScriptMaggie::ShotAtAndHit() {
	AI_Movement_Track_Flush(kActorMaggie);
	Actor_Set_Goal_Number(kActorMaggie, 414);
	Actor_Set_Targetable(kActorMaggie, false);
}

void AIScriptMaggie::Retired(int byActorId) {
}

int AIScriptMaggie::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMaggie::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (currentGoalNumber == 599) {
		return true;
	}
	switch (newGoalNumber) {
	case 11:
		Actor_Change_Animation_Mode(kActorMaggie, 55);
		var_45F3F0_animation_state = 9;
		var_45F3F4_animation_frame = 0;
		return true;
	case 10:
		Actor_Change_Animation_Mode(kActorMaggie, 54);
		var_45F3F0_animation_state = 6;
		var_45F3F4_animation_frame = 0;
		AI_Countdown_Timer_Reset(kActorMaggie, 0);
		AI_Countdown_Timer_Start(kActorMaggie, 0, Random_Query(2, 9));
		return true;
	case 9:
		if (Random_Query(0, 2) <= 0) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, false);
			Actor_Change_Animation_Mode(kActorMaggie, 54);
		} else {
			AI_Movement_Track_Flush(kActorMaggie);
			if (Actor_Query_Which_Set_In(kActorMaggie) == 10) {
				AI_Movement_Track_Append(kActorMaggie, sub_44B260(), 486);
			}
			AI_Movement_Track_Repeat(kActorMaggie);
		}
		return true;
	case 8:
		AI_Countdown_Timer_Reset(kActorMaggie, 0);
		AI_Countdown_Timer_Start(kActorMaggie, 0, Random_Query(3, 9));
		return true;
	case 7:
		AI_Countdown_Timer_Reset(kActorMaggie, 0);
		AI_Movement_Track_Flush(kActorMaggie);
		Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 30, 0, false);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 56);
		Actor_Set_Goal_Number(kActorMaggie, 8);
		return true;
	case 3:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorMaggie);
		Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 48, 0, false);
		Actor_Face_Actor(0, kActorMaggie, true);
		Actor_Face_Actor(kActorMaggie, 0, false);
		Actor_Says(kActorMcCoy, 2400, 52);
		Actor_Set_Goal_Number(kActorMaggie, 8);
		Player_Gains_Control();
		return true;
	case 1:
		Actor_Put_In_Set(kActorMaggie, 10);
		Actor_Set_At_Waypoint(kActorMaggie, sub_44B260(), 512);
		AI_Movement_Track_Flush(kActorMaggie);
		AI_Movement_Track_Append(kActorMaggie, 264, 0);
		AI_Movement_Track_Repeat(kActorMaggie);
		return true;
	case 0:
		Actor_Put_In_Set(kActorMaggie, 10);
		Actor_Set_At_Waypoint(kActorMaggie, 265, 780);
		return true;
	case 415:
		AI_Movement_Track_Flush(kActorMaggie);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Sound_Play(494, 50, 0, 0, 100);
		Actor_Set_Goal_Number(kActorMaggie, 599);
		Actor_Change_Animation_Mode(kActorMaggie, 51);
		if (Actor_Query_Inch_Distance_From_Actor(0, kActorMaggie) < 144) {
			Player_Loses_Control();
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
		} else {
			Delay(3000);
			Scene_Exits_Disable();
			Actor_Says(kActorMcCoy, 2235, 12);
			if (Actor_Query_Is_In_Current_Set(kActorSteele)) {
				Actor_Says(kActorSteele, 1530, 58);
				Actor_Set_Goal_Number(kActorSteele, 431);
			}
			Delay(2000);
			Actor_Says(kActorMcCoy, 2390, 13);
			if (Actor_Query_Goal_Number(kActorSadik) == 411) {
				Actor_Set_Goal_Number(kActorSadik, 412);
			} else {
				Actor_Set_Goal_Number(kActorClovis, 512);
			}
		}
		break;
	case 413:
		if (Actor_Query_Is_In_Current_Set(kActorSteele)) {
			Actor_Says(kActorSteele, 3270, 59);
		}
		AI_Movement_Track_Flush(kActorMaggie);
		AI_Movement_Track_Append(kActorMaggie, 540, 0);
		AI_Movement_Track_Repeat(kActorMaggie);
		break;
	case 412:
		Scene_Exits_Disable();
		Loop_Actor_Walk_To_XYZ(kActorMaggie, -734.0, 0.0, -432.0, 0, 0, false, 0);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 56);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Actor_Says(kActorMcCoy, 2225, 3);
		Actor_Set_Goal_Number(kActorMaggie, 413);
		break;
	case 411:
		AI_Movement_Track_Flush(kActorMaggie);
		Game_Flag_Set(461);
		Actor_Put_In_Set(kActorMaggie, 9);
		Actor_Set_At_XYZ(kActorMaggie, -672.0, 0.0, -428.0, 653);
		Actor_Change_Animation_Mode(kActorMaggie, 0);
		break;
	case 400:
		Actor_Set_Goal_Number(kActorMaggie, 410);
		break;
	}
	return false;
}

bool AIScriptMaggie::UpdateAnimation(int *animation, int *frame) {
	int goal;
	switch (var_45F3F0_animation_state) {
	case 16:
		*animation = 871;
		var_45F3F4_animation_frame = 0;
		break;
	case 15:
		*animation = 874;
		var_45F3F4_animation_frame = Slice_Animation_Query_Number_Of_Frames(874) - 1;
		break;
	case 14:
		*animation = 874;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(874) - 1) {
			var_45F3F0_animation_state = 15;
			var_45F3F4_animation_frame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Put_In_Set(kActorMaggie, 99);
			Actor_Set_At_Waypoint(kActorMaggie, 41, 0);
		}
		break;
	case 13:
		*animation = 873;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(873)) {
			var_45F3F0_animation_state = 11;
			var_45F3F4_animation_frame = 0;
			*animation = 875;
			Actor_Set_Goal_Number(kActorMaggie, 414);
		}
		break;
	case 12:
		*animation = 872;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(872)) {
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 11:
		*animation = 875;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(875)) {
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 10:
		*animation = 876;
		var_45F3F4_animation_frame--;
		if (var_45F3F4_animation_frame > 0) {
			break;
		}
		var_45F3F0_animation_state = 6;
		var_45F3F4_animation_frame = 0;
		*animation = 867;
		goal = Actor_Query_Goal_Number(66);
		if (goal == 3) {
			var_45F3F0_animation_state = 7;
			var_45F3F4_animation_frame = 0;
			*animation = 868;
		} else if (goal == 7) {
			Actor_Set_Goal_Number(kActorMaggie, 10);
			Actor_Set_Goal_Number(kActorMaggie, 7);
		} else {
			Actor_Set_Goal_Number(kActorMaggie, 10);
		}
		break;
	case 9:
		*animation = 876;
		var_45F3F4_animation_frame = Slice_Animation_Query_Number_Of_Frames(876) - 1;
		break;
	case 8:
		*animation = 876;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(876) - 1) {
			var_45F3F0_animation_state = 9;
			Actor_Set_Goal_Number(kActorMaggie, 11);
		}
		break;
	case 7:
		*animation = 868;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(868)) {
			*animation = 864;
			var_45F3F0_animation_state = 0;
			var_45F3F4_animation_frame = 0;
			if (Actor_Query_Goal_Number(kActorMaggie) == 10) {
				Actor_Set_Goal_Number(kActorMaggie, 8);
			} else if (Actor_Query_Goal_Number(kActorMaggie) == 7) {
				Actor_Set_Goal_Number(kActorMaggie, 12);
				Actor_Set_Goal_Number(kActorMaggie, 7);
			}
		}
		break;
	case 6:
		*animation = 867;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(867)) {
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 5:
		*animation = 866;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(866)) {
			var_45F3F0_animation_state = 6;
			var_45F3F4_animation_frame = 0;
			*animation = 867;
			if (Actor_Query_Goal_Number(kActorMaggie) == 9) {
				Actor_Set_Goal_Number(kActorMaggie, 10);
			}
		}
		break;
	case 4:
		*animation = 865;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(865)) {
			var_45F3F4_animation_frame = 0;
			var_45F3F8--;
			if (var_45F3F8 <= 0) {
				Actor_Change_Animation_Mode(kActorMaggie, 0);
				*animation = 864;
			}
		}
		break;
	case 3:
		*animation = 870;
		if (var_45F3F4_animation_frame == 1) {
			Sound_Play(Random_Query(263, 264), 50, 0, 0, 50);
		}
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			var_45F3FC--;
			if (var_45F3FC <= 0 ) {
				Actor_Change_Animation_Mode(kActorMaggie, 0);
				*animation = 864;
				var_45F3F0_animation_state = 0;
			}
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 2:
		*animation = 869;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(869)) {
			Actor_Change_Animation_Mode(kActorMaggie, 0);
			*animation = 864;
			var_45F3F0_animation_state = 0;
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 1:
		*animation = 863;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(863)) {
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 0:
		*animation = 864;
		var_45F3F4_animation_frame++;
		if (var_45F3F4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(864)) {
			var_45F3F4_animation_frame = 0;
		}
		break;
	}
	*frame = var_45F3F4_animation_frame;
	return true;
}

bool AIScriptMaggie::ChangeAnimationMode(int mode) {
	if (mode == 1) {
		if (Game_Flag_Query(461)) {
			var_45F3F0_animation_state = 12;
			var_45F3F4_animation_frame = 0;
		} else {
			var_45F3F0_animation_state = 1;
			var_45F3F4_animation_frame = 0;
		}
		return true;
	}
	if (mode == 0) {
		if (Game_Flag_Query(461)) {
			var_45F3F0_animation_state = 11;
			var_45F3F4_animation_frame = 0;
		} else {
			switch (var_45F3F0_animation_state) {
			case 8:
			case 9:
				var_45F3F0_animation_state = 10;
				break;
			case 6:
				var_45F3F0_animation_state = 7;
				var_45F3F4_animation_frame = 0;
				break;
			case 5:
				var_45F3F0_animation_state = 7;
				var_45F3F4_animation_frame = 0;
				break;
			case 2:
			case 7:
			case 10:
				break;
			default:
				var_45F3F0_animation_state = 0;
				var_45F3F4_animation_frame = 0;
				break;
			}
		}
		return true;
	}

	switch (mode) {
	case 88:
		var_45F3F0_animation_state = 16;
		var_45F3F4_animation_frame = 0;
		break;
	case 57:
		if (var_45F3F0_animation_state != 4) {
			var_45F3F4_animation_frame = 0;
			var_45F3F0_animation_state = 4;
		}
		var_45F3F8 = Random_Query(2, 6);
		Sound_Play(276, 50, 0, 0, 50);
		break;
	case 56:
		if (var_45F3F0_animation_state != 3) {
			var_45F3F4_animation_frame = 0;
			var_45F3F0_animation_state = 3;
		}
		var_45F3FC = Random_Query(2, 6);
		break;
	case 55:
		if (var_45F3F0_animation_state == 6) {
			var_45F3F0_animation_state = 8;
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 54:
		if ((unsigned int)var_45F3F0_animation_state <= 9) {
			if (var_45F3F0_animation_state) {
				if (var_45F3F0_animation_state == 9) {
					var_45F3F0_animation_state = 10;
					var_45F3F4_animation_frame = 0;
				}
			} else {
				var_45F3F0_animation_state = 5;
				var_45F3F4_animation_frame = 0;
			}
		}
		break;
	case 52:
		if (Game_Flag_Query(461)) {
			var_45F3F0_animation_state = 13;
			var_45F3F4_animation_frame = 0;
		} else {
			var_45F3F0_animation_state = 2;
			var_45F3F4_animation_frame = 0;
		}
		break;
	case 51:
		var_45F3F0_animation_state = 14;
		var_45F3F4_animation_frame = 0;
		Sound_Play(272, 50, 0, 0, 50);
		break;
	}
	return true;
}

void AIScriptMaggie::QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4) {
	*animationState = var_45F3F0_animation_state;
	*animationFrame = var_45F3F4_animation_frame;
	*a3 = var_462B30;
	*a4 = var_462B34;
}

void AIScriptMaggie::SetAnimationState(int animationState, int animationFrame, int a3, int a4) {
	var_45F3F0_animation_state = animationState;
	var_45F3F4_animation_frame = animationFrame;
	var_462B30 = a3;
	var_462B34 = a4;
}

bool AIScriptMaggie::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMaggie::FledCombat() {
}

int AIScriptMaggie::sub_44B260() {
	int random = Random_Query(0, 3);
	if (random == 0) {
		return 264;
	}
	if (random == 1) {
		return 265;
	}
	if (random == 2) {
		return 266;
	}
	return 267;
}

float AIScriptMaggie::sub_44B200(int actorId, float x, float y, float z) {
	float actorX, actorY, actorZ;
	Actor_Query_XYZ(actorId, &actorX, &actorY, &actorZ);
	return sqrtf((z - actorZ) * (z - actorZ) + (y - actorY) * (y - actorY) + (x - actorX) * (x - actorX));
}

} // End of namespace BladeRunner
