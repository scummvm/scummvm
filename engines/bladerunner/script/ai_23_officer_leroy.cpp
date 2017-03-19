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

#include "bladerunner/script/ai_23_officer_leroy.h"

namespace BladeRunner {

AIScript_Officer_Leroy::AIScript_Officer_Leroy(BladeRunnerEngine *vm)
	: AIScriptBase(vm), var_45D5B0_animation_state(0), var_45D5B4_animation_frame(0), var_45D5B8(0), var_45D5BC(0), var_462880(0), var_462884(0) {}

void AIScript_Officer_Leroy::Initialize() {
	var_45D5B0_animation_state = 0;
	var_45D5B4_animation_frame = 0;
	var_462880 = 0;
	var_45D5B8 = 0;
	var_45D5BC = 0;

	Actor_Put_In_Set(23, 69);
	Actor_Set_At_XYZ(23, -261.80f, 6.00f, 79.58f, 512);
	Actor_Set_Goal_Number(23, 0);
	Actor_Set_Frame_Rate_FPS(23, 8);
}

bool AIScript_Officer_Leroy::Update() {
	if (Global_Variable_Query(1) == 4 && Actor_Query_Goal_Number(23) < 300) {
		AI_Movement_Track_Flush(23);
		Actor_Set_Goal_Number(23, 300);
	} else if (Global_Variable_Query(1) == 5 && Actor_Query_Goal_Number(23) < 400) {
		AI_Movement_Track_Flush(23);
		Actor_Set_Goal_Number(23, 400);
	} else if (!Game_Flag_Query(182) && Game_Flag_Query(147) == 1 && Game_Flag_Query(163) == 1 && Player_Query_Current_Scene() != 78 && Global_Variable_Query(1) < 3) {
		Game_Flag_Set(186);
		Actor_Set_Goal_Number(23, 3);
	} else if (Actor_Query_Goal_Number(23) != 1 && Actor_Query_Goal_Number(23) != 2 && Game_Flag_Query(199) == 1) {
		Game_Flag_Reset(199);
	} else if (Global_Variable_Query(15) > 4 && !Actor_Clue_Query(23, 239)) {
		Actor_Clue_Acquire(23, 239, 1, -1);
	} else if (Game_Flag_Query(629) == 1) {
		Game_Flag_Reset(629);
	} else if (Game_Flag_Query(623) == 1 && !Game_Flag_Query(664)) {
		Game_Flag_Set(664);
		Actor_Set_Goal_Number(23, 305);
	} else if (Actor_Query_Goal_Number(23) == 310 && Actor_Query_Which_Set_In(23) != Player_Query_Current_Set()) {
		Non_Player_Actor_Combat_Mode_Off(23);
		Actor_Set_Goal_Number(23, 305);
	} else if (Actor_Query_Goal_Number(23) == 599 && Actor_Query_Which_Set_In(23) != Player_Query_Current_Set()) {
		Actor_Set_Health(23, 40, 40);
		Actor_Set_Goal_Number(23, 305);
	} else if (Actor_Query_Goal_Number(23) == 305) {
		switch (Actor_Query_Which_Set_In(23)) {
		case 7:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 0, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 20:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 1, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 33:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 5, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 53:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 7, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 54:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 3, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 70:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 18, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 74:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 11, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 77:
		case 78:
		case 79:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 10, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 81:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 13, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 83:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 14, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 84:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 16, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 86:
			if (Actor_Query_Which_Set_In(23) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(23, 310);
				Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 17, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		}
	}
	return false;
}

void AIScript_Officer_Leroy::TimerExpired(int timer) {
	if (timer == 1) {
		AI_Countdown_Timer_Reset(23, 1);
		if (Actor_Query_In_Set(0, 41)) {
			Actor_Set_Goal_Number(23, 430);
			Actor_Set_Goal_Number(24, 430);
		} else {
			Game_Flag_Set(684);
		}
	} else if (timer == 2) {
		AI_Countdown_Timer_Reset(23, 2);
		sub_431420();
	}
}

void AIScript_Officer_Leroy::CompletedMovementTrack() {
	int v0;
	unsigned int v1;

	v0 = Actor_Query_Goal_Number(23);
	if (v0 == 1) {
		Actor_Set_Goal_Number(23, 2);
		return;
	}
	//todo: tidyup
	v1 = v0 - 305;
	if (v1 > 3) {
		return;
	}
	if (!v1) {
		Actor_Set_Goal_Number(23, 306);
		return;
	}
	if (v1 != 2) {
		if (v1 == 3) {
			Actor_Change_Animation_Mode(23, 4);
			Actor_Face_Actor(23, 0, true);
			Actor_Set_Goal_Number(23, 309);
		}
		return;
	}
	Non_Player_Actor_Combat_Mode_On(23, 0, 1, 0, 12, 4, 7, 8, 0, -1, -1, 15, 300, 0);
}

void AIScript_Officer_Leroy::ReceivedClue(int clueId, int fromActorId) {
	if (clueId == 222) {
		Actor_Modify_Friendliness_To_Other(23, 0, 5);
	}
	if (clueId == 215) {
		Actor_Modify_Friendliness_To_Other(23, 0, -4);
	}
}

void AIScript_Officer_Leroy::ClickedByPlayer() {}

void AIScript_Officer_Leroy::EnteredScene(int sceneId) {}

void AIScript_Officer_Leroy::OtherAgentEnteredThisScene() {}

void AIScript_Officer_Leroy::OtherAgentExitedThisScene() {}

void AIScript_Officer_Leroy::OtherAgentEnteredCombatMode() {}

void AIScript_Officer_Leroy::ShotAtAndMissed() {}

void AIScript_Officer_Leroy::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(23) == 307) {
		Actor_Set_Health(23, 50, 50);
	}
}

void AIScript_Officer_Leroy::Retired(int byActorId) {
	Actor_Set_Goal_Number(23, 599);
	Game_Flag_Set(607);
}

int AIScript_Officer_Leroy::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	if (otherActorId) {
		return 0;
	}
	switch (clueId) {
	case 242:
		return -6;
	case 240:
		return -2;
	case 239:
		return -5;
	case 228:
		return 2;
	case 227:
		return 4;
	case 226:
		return 4;
	case 225:
		return 3;
	case 224:
		return 3;
	case 223:
		return 2;
	case 222:
		return 3;
	case 215:
		return -5;
	}
	return 0;
}

bool AIScript_Officer_Leroy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(23);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append(23, 57, 7);
			AI_Movement_Track_Append(23, 58, 7);
		} else {
			AI_Movement_Track_Append(23, 58, 7);
			AI_Movement_Track_Append(23, 57, 7);
		}
		AI_Movement_Track_Repeat(23);
		return true;
	case 2:
		if (Random_Query(1, 3) == 1) {
			if (Random_Query(1, 2) == 1 && !Actor_Clue_Query(23, 16)) {
				Actor_Clue_Acquire(23, 16, 0, -1);
			} else if (!Actor_Clue_Query(23, 17)) {
				Actor_Clue_Acquire(23, 17, 0, -1);
			}
		}
		if (Game_Flag_Query(182)) {
			if (Actor_Clue_Query(23, 16) && Actor_Clue_Query(23, 17)) {
				Actor_Set_Goal_Number(23, 0);
			} else {
				Actor_Set_Goal_Number(23, 1);
			}
		} else {
			Actor_Set_Goal_Number(23, 0);
		}
		return true;
	case 3:
		AI_Movement_Track_Flush(23);
		AI_Movement_Track_Append(23, 39, Random_Query(120, 240));
		AI_Movement_Track_Append(23, 35, 0);
		AI_Movement_Track_Repeat(23);
		return true;
	case 99:
		AI_Movement_Track_Flush(23);
		return false;
	case 102:
		AI_Movement_Track_Flush(23);
		AI_Movement_Track_Append(23, 107, 0);
		AI_Movement_Track_Append(23, 108, 0);
		AI_Movement_Track_Repeat(23);
		return true;
	case 300:
		Actor_Set_Goal_Number(23, 305);
		return true;
	case 305:
		AI_Movement_Track_Flush(23);
		switch (Random_Query(1, 10)) {
		case 1:
			AI_Movement_Track_Append(23, 398, 15);
			AI_Movement_Track_Append(23, 399, 0);
			AI_Movement_Track_Append(23, 400, 0);
			AI_Movement_Track_Append(23, 401, 0);
			AI_Movement_Track_Append_With_Facing(23, 402, 3, 276);
			AI_Movement_Track_Append(23, 403, 0);
			AI_Movement_Track_Append(23, 404, 15);
			AI_Movement_Track_Repeat(23);
			break;
		case 2:
			AI_Movement_Track_Append(23, 385, 10);
			AI_Movement_Track_Append(23, 242, 2);
			AI_Movement_Track_Append(23, 386, 2);
			AI_Movement_Track_Append(23, 387, 15);
			AI_Movement_Track_Repeat(23);
			break;
		case 3:
			AI_Movement_Track_Append(23, 390, 10);
			AI_Movement_Track_Append(23, 391, 0);
			AI_Movement_Track_Append(23, 392, 5);
			AI_Movement_Track_Append(23, 345, 0);
			AI_Movement_Track_Append(23, 393, 15);
			AI_Movement_Track_Repeat(23);
			break;
		case 4:
			AI_Movement_Track_Append(23, 381, 15);
			AI_Movement_Track_Append(23, 382, 0);
			AI_Movement_Track_Append(23, 383, 15);
			AI_Movement_Track_Append(23, 382, 3);
			AI_Movement_Track_Append(23, 384, 0);
			AI_Movement_Track_Append(23, 35, 30);
			AI_Movement_Track_Repeat(23);
			break;
		case 5:
			AI_Movement_Track_Append(23, 388, 10);
			AI_Movement_Track_Append(23, 389, 10);
			AI_Movement_Track_Append(23, 35, 30);
			AI_Movement_Track_Repeat(23);
			break;
		case 6:
			AI_Movement_Track_Append(23, 385, 10);
			AI_Movement_Track_Append(23, 242, 2);
			AI_Movement_Track_Append(23, 386, 2);
			AI_Movement_Track_Append(23, 387, 15);
			AI_Movement_Track_Repeat(23);
			break;
		case 7:
			AI_Movement_Track_Append(23, 394, 15);
			AI_Movement_Track_Append(23, 395, 0);
			AI_Movement_Track_Append(23, 396, 0);
			AI_Movement_Track_Append(23, 397, 15);
			AI_Movement_Track_Append(23, 396, 0);
			AI_Movement_Track_Append(23, 395, 0);
			AI_Movement_Track_Append(23, 430, 15);
			AI_Movement_Track_Append(23, 35, 30);
			AI_Movement_Track_Repeat(23);
			break;
		case 8:
			switch (Random_Query(1, 7)) {
			case 1:
				AI_Movement_Track_Append(23, 302, 0);
				AI_Movement_Track_Append(23, 407, 0);
				AI_Movement_Track_Append(23, 408, 0);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				break;
			case 2:
				AI_Movement_Track_Append(23, 536, 0);
				AI_Movement_Track_Append(23, 537, 0);
				AI_Movement_Track_Append(23, 538, 1);
				AI_Movement_Track_Append(23, 537, 0);
				AI_Movement_Track_Append(23, 536, 0);
				AI_Movement_Track_Repeat(23);
				break;
			case 3:
				AI_Movement_Track_Append(23, 296, 10);
				AI_Movement_Track_Append(23, 409, 2);
				AI_Movement_Track_Append(23, 296, 10);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				break;
			case 4:
				AI_Movement_Track_Append(23, 411, 10);
				AI_Movement_Track_Append(23, 412, 5);
				AI_Movement_Track_Append(23, 411, 0);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				break;
			case 5:
				AI_Movement_Track_Append(23, 413, 10);
				AI_Movement_Track_Append(23, 414, 0);
				AI_Movement_Track_Append_With_Facing(23, 431, 0, 1017);
				AI_Movement_Track_Append(23, 432, 10);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				break;
			case 6:
				AI_Movement_Track_Append(23, 415, 0);
				AI_Movement_Track_Append_With_Facing(23, 416, 0, 620);
				AI_Movement_Track_Append(23, 417, 0);
				AI_Movement_Track_Append(23, 418, 0);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				break;
			case 7:
				AI_Movement_Track_Append(23, 405, 10);
				AI_Movement_Track_Append(23, 406, 0);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				return false;
			default:
				return false;
			}
			//no break
		case 9:
			if (Random_Query(1, 2) - 1 == 1) {
				AI_Movement_Track_Append(23, 433, 10);
				AI_Movement_Track_Append(23, 434, 0);
				AI_Movement_Track_Append(23, 435, 0);
				AI_Movement_Track_Append(23, 35, 30);
				AI_Movement_Track_Repeat(23);
				return false;
			}
			AI_Movement_Track_Append(23, 420, 10);
			AI_Movement_Track_Append(23, 422, 2);
			AI_Movement_Track_Append(23, 421, 1);
			AI_Movement_Track_Append_With_Facing(23, 422, 4, 182);
			AI_Movement_Track_Append(23, 420, 10);
			AI_Movement_Track_Append(23, 35, 30);
			AI_Movement_Track_Repeat(23);
			//no break
		case 10:
			AI_Movement_Track_Append(23, 310, 0);
			AI_Movement_Track_Append(23, 307, 0);
			AI_Movement_Track_Append(23, 309, 0);
			AI_Movement_Track_Append(23, 310, 0);
			AI_Movement_Track_Append(23, 35, 30);
			AI_Movement_Track_Repeat(23);
			return false;
		default:
			return false;
		}
		//no break
	case 306:
		Actor_Set_Goal_Number(23, 305);
		return true;
	case 307:
		AI_Movement_Track_Flush(23);
		AI_Movement_Track_Append_With_Facing(23, 416, 0, 556);
		Actor_Change_Animation_Mode(23, 4);
		AI_Movement_Track_Repeat(23);
		return true;
	case 308:
		AI_Movement_Track_Flush(23);
		AI_Movement_Track_Append_Run_With_Facing(23, 440, 2, 355);
		AI_Movement_Track_Append_Run_With_Facing(23, 441, 0, 825);
		AI_Movement_Track_Repeat(23);
		return true;
	case 400:
		AI_Movement_Track_Flush(23);
		AI_Movement_Track_Append(23, 35, 0);
		AI_Movement_Track_Repeat(23);
		Actor_Set_Goal_Number(23, 410);
		return true;
	case 420:
		AI_Countdown_Timer_Reset(23, 1);
		AI_Countdown_Timer_Start(23, 1, 120);
		Actor_Set_Goal_Number(23, 410);
		return true;
	case 425:
		AI_Countdown_Timer_Reset(23, 1);
		AI_Countdown_Timer_Start(23, 1, 60);
		Actor_Set_Goal_Number(23, 410);
		return true;
	case 430:
		Actor_Set_Goal_Number(23, 410);
		Actor_Set_Goal_Number(24, 410);
		return false;
	default:
		return false;
	}
}

bool AIScript_Officer_Leroy::UpdateAnimation(int *animation, int *frame) {
	int v57;
	int a1;

	switch (var_45D5B0_animation_state) {
	case 32:
		*animation = 603;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(603)) {
			*animation = 589;
			var_45D5B0_animation_state = 0;
			var_45D5B8 = 0;
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 31:
		*animation = 604;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(604)) {
			*animation = 601;
			var_45D5B0_animation_state = 1;
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 30:
		*animation = 587;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(587)) {
			*animation = 589;
			var_45D5B0_animation_state = 0;
			var_45D5B4_animation_frame = 0;
			Actor_Change_Animation_Mode(23, 0);
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 29:
		*animation = 586;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(586)) {
			*animation = 589;
			var_45D5B0_animation_state = 0;
			var_45D5B4_animation_frame = 0;
			Actor_Change_Animation_Mode(23, 0);
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 28:
		*animation = 576;
		if (var_45D5B4_animation_frame < Slice_Animation_Query_Number_Of_Frames(576) - 1) {
			++var_45D5B4_animation_frame;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 27:
		*animation = 588;
		if (var_45D5B4_animation_frame < Slice_Animation_Query_Number_Of_Frames(588) - 1) {
			++var_45D5B4_animation_frame;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 26:
		*animation = 573;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(573)) {
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			*animation = 571;
			Actor_Change_Animation_Mode(23, 4);
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 25:
		*animation = 572;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(572)) {
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			*animation = 571;
			Actor_Change_Animation_Mode(23, 4);
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 24:
		*animation = 581;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame == 4) {
			if (Random_Query(1, 2) == 1) {
				Sound_Play_Speech_Line(23, 9010, 75, 0, 99);
			} else {
				Sound_Play_Speech_Line(23, 9015, 75, 0, 99);
			}
		}
		if (var_45D5B4_animation_frame == 5) {
			Actor_Combat_AI_Hit_Attempt(23);
		}
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(581)) {
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			*animation = 571;
			Actor_Change_Animation_Mode(23, 4);
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 23:
		*animation = 580;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(580)) {
			*animation = 589;
			var_45D5B0_animation_state = 0;
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 22:
		*animation = 579;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(579)) {
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			*animation = 571;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 21:
		*animation = 571;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(571)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 20:
		*animation = 571;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame > Slice_Animation_Query_Number_Of_Frames(571) - 1) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 19:
		*animation = 600;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(600)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 18:
		*animation = 599;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(599)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 17:
		*animation = 598;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(598)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 16:
		*animation = 597;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(597)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 15:
		*animation = 596;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(596)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 14:
		*animation = 595;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(595)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 13:
		*animation = 594;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(594)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 12:
		*animation = 593;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(593)) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			*animation = 592;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 11:
		if (var_45D5BC && var_45D5B4_animation_frame <= 2) {
			var_45D5BC = 0;
			var_45D5B0_animation_state = 0;
			var_45D5B4_animation_frame = 0;
			*animation = 589;
			var_45D5B8 = Random_Query(0, 1);
			*frame = var_45D5B4_animation_frame;
			return true;
		}

		*animation = 592;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(592)) {
			var_45D5B4_animation_frame = 0;
			if (var_45D5BC) {
				var_45D5BC = 0;
				var_45D5B0_animation_state = 0;
				var_45D5B4_animation_frame = 0;
				*animation = 589;
				var_45D5B8 = Random_Query(0, 1);
			} else {
				var_45D5B0_animation_state = Random_Query(0, 8) + 11;
			}
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 10:
		*animation = 578;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(578)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 9:
		*animation = 577;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(577)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 8:
		*animation = 575;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(575)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 7:
		*animation = 574;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(574)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 6:
		*animation = 585;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(585)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 5:
		*animation = 584;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(584)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 4:
		*animation = 583;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(583)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 3:
		*animation = 582;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(582)) {
			var_45D5B4_animation_frame = 0;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 2:
		if (Game_Flag_Query(199)) {
			*animation = 603;
			var_45D5B4_animation_frame++;
			if (var_45D5B4_animation_frame > Slice_Animation_Query_Number_Of_Frames(603) - 1) {
				Game_Flag_Reset(199);
				var_45D5B4_animation_frame = 0;
				var_45D5B0_animation_state = var_462880;
				*animation = var_462884;
			}
		} else {
			if (var_45D5B8 == 0) {
				*animation = 589;
			}
			if (var_45D5B8 == 1) {
				*animation = 590;
			}
			if (var_45D5B8 == 2) {
				*animation = 591;
			}
			if (var_45D5B4_animation_frame > Slice_Animation_Query_Number_Of_Frames(*animation) / 2) {
				a1 = *animation;
				var_45D5B4_animation_frame += 3;
				v57 = var_45D5B4_animation_frame;
				if (v57 > Slice_Animation_Query_Number_Of_Frames(a1) - 1) {
					var_45D5B4_animation_frame = 0;
					var_45D5B0_animation_state = var_462880;
					*animation = var_462884;
				}
			} else {
				var_45D5B4_animation_frame -= 3;
				if (var_45D5B4_animation_frame < 0) {
					var_45D5B4_animation_frame = 0;
					var_45D5B0_animation_state = var_462880;
					*animation = var_462884;
				}
			}
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 1:
		*animation = 601;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(601)) {
			var_45D5B4_animation_frame = 0;
		}
		if (!Game_Flag_Query(199)) {
			var_45D5B0_animation_state = 32;
			var_45D5B4_animation_frame = 0;
			*animation = 603;
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 0:
		if (Game_Flag_Query(199) && !Game_Flag_Query(392)) {
			var_45D5B0_animation_state = 31;
			var_45D5B4_animation_frame = 0;
			*animation = 604;
		} else if (var_45D5B8 == 1) {
			*animation = 590;
			var_45D5B4_animation_frame++;
			if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(590)) {
				var_45D5B8 = Random_Query(0, 2);
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B8 == 2) {
			*animation = 591;
			var_45D5B4_animation_frame++;
			if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(591)) {
				var_45D5B8 = Random_Query(0, 2);
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B8 == 0) {
			*animation = 589;
			var_45D5B4_animation_frame++;
			if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(589)) {
				var_45D5B8 = Random_Query(0, 2);
				var_45D5B4_animation_frame = 0;
			}
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	default:
		*animation = 399;
		*frame = var_45D5B4_animation_frame;
		return true;
	}
}

bool AIScript_Officer_Leroy::ChangeAnimationMode(int mode) {
	int v1;

	switch (mode) {
	case 28:
		Game_Flag_Set(199);
		break;
	case 27:
		Game_Flag_Reset(199);
		break;
	case 22:
		if (Random_Query(0, 1)) {
			var_45D5B0_animation_state = 25;
		} else {
			var_45D5B0_animation_state = 26;
		}
		var_45D5B4_animation_frame = 0;
		break;
	case 21:
		if (var_45D5B0_animation_state == 21 || var_45D5B0_animation_state == 24) {
			if (Random_Query(0, 1)) {
				var_45D5B0_animation_state = 25;
			} else {
				var_45D5B0_animation_state = 26;
			}
			var_45D5B4_animation_frame = 0;
		} else {
			if (Random_Query(0, 1)) {
				var_45D5B0_animation_state = 29;
			} else {
				var_45D5B0_animation_state = 30;
			}
			var_45D5B4_animation_frame = 0;
		}
		break;
	case 19:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 19;
			var_462884 = 600;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 19;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 18:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 18;
			var_462884 = 599;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 18;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 17:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 17;
			var_462884 = 598;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 17;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 16:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 16;
			var_462884 = 597;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 16;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 15:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 15;
			var_462884 = 596;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 15;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 14:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 14;
			var_462884 = 595;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 14;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 13:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 13;
			var_462884 = 594;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 13;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 12:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 12;
			var_462884 = 593;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 12;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 8:
		var_45D5B0_animation_state = 8;
		var_45D5B4_animation_frame = 0;
		break;
	case 7:
		var_45D5B0_animation_state = 7;
		var_45D5B4_animation_frame = 0;
		break;
	case 6:
		var_45D5B0_animation_state = 24;
		var_45D5B4_animation_frame = 0;
		break;
	case 4:
		switch (var_45D5B0_animation_state) {
		case 21:
		case 22:
		case 24:
			return true;
		case 7:
		case 8:
		case 9:
		case 10:
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			break;
		case 0:
			var_45D5B0_animation_state = 22;
			var_45D5B4_animation_frame = 0;
			break;
		default:
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			break;
		}
		break;
	case 3:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 11;
			var_462884 = 592;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		} else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case 2:
		var_45D5B0_animation_state = 4;
		var_45D5B4_animation_frame = 0;
		break;
	case 1:
		var_45D5B0_animation_state = 3;
		var_45D5B4_animation_frame = 0;
		break;
	case 0:
		switch (var_45D5B0_animation_state) {
		case 21:
		case 24:
			var_45D5B0_animation_state = 23;
			var_45D5B4_animation_frame = 0;
			break;
		case 20:
			v1 = var_45D5B4_animation_frame;
			Actor_Change_Animation_Mode(23, 4);
			var_45D5B4_animation_frame = v1;
			var_45D5B0_animation_state = 21;
			break;
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			var_45D5BC = 1;
			break;
		case 0:
		case 23:
			return true;
		default:
			var_45D5B0_animation_state = 0;
			var_45D5B4_animation_frame = 0;
			var_45D5B8 = Random_Query(0, 1);
			break;
		}
		break;
	case 5:
	case 9:
	case 10:
	case 11:
	case 20:
	case 23:
	case 24:
	case 25:
	case 26:
		return true;
	default:
		switch (mode) {
		case 58:
			var_45D5B0_animation_state = 20;
			var_45D5B4_animation_frame = 0;
			break;
		case 48:
			if (var_45D5B0_animation_state == 21 || var_45D5B0_animation_state == 22 || var_45D5B0_animation_state == 24) {
				var_45D5B0_animation_state = 28;
				var_45D5B4_animation_frame = 0;
			} else {
				var_45D5B0_animation_state = 27;
				var_45D5B4_animation_frame = 0;
			}
			break;
		case 47:
			var_45D5B0_animation_state = 10;
			var_45D5B4_animation_frame = 0;
			break;
		case 46:
			var_45D5B0_animation_state = 9;
			var_45D5B4_animation_frame = 0;
			break;
		case 45:
			var_45D5B0_animation_state = 6;
			var_45D5B4_animation_frame = 0;
			break;
		case 44:
			var_45D5B0_animation_state = 5;
			var_45D5B4_animation_frame = 0;
			break;
		default:
			return true;
		}
		break;
	}
	return true;
}

void AIScript_Officer_Leroy::QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4) {
	*animationState = var_45D5B0_animation_state;
	*animationFrame = var_45D5B4_animation_frame;
	*a3 = var_462880;
	*a4 = var_462884;
}

void AIScript_Officer_Leroy::SetAnimationState(int animationState, int animationFrame, int a3, int a4) {
	var_45D5B0_animation_state = animationState;
	var_45D5B4_animation_frame = animationFrame;
	var_462880 = a3;
	var_462884 = a4;
}

bool AIScript_Officer_Leroy::ReachedMovementTrackWaypoint(int a1) {
	if (a1 == 57 || a1 == 58) {
		sub_431408();
		AI_Countdown_Timer_Reset(23, 2);
		AI_Countdown_Timer_Start(23, 2, 6);
	}
	return true;
}

void AIScript_Officer_Leroy::FledCombat() {
	Actor_Set_Goal_Number(23, 300);
}

bool AIScript_Officer_Leroy::sub_431408() {
	Game_Flag_Set(199);
	return true;
}

bool AIScript_Officer_Leroy::sub_431420() {
	Game_Flag_Reset(199);
	return true;
}
} // End of namespace BladeRunner
