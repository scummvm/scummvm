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

void AIScriptOfficerLeary::Initialize() {
	var_45D5B0_animation_state = 0;
	var_45D5B4_animation_frame = 0;
	var_462880 = 0;
	var_45D5B8 = 0;
	var_45D5BC = 0;
	var_462884 = 0;

	Actor_Put_In_Set(kActorOfficerLeary, 69);
	Actor_Set_At_XYZ(kActorOfficerLeary, -261.80f, 6.00f, 79.58f, 512);
	Actor_Set_Goal_Number(kActorOfficerLeary, 0);
	Actor_Set_Frame_Rate_FPS(kActorOfficerLeary, 8);
}

bool AIScriptOfficerLeary::Update() {
	if (Global_Variable_Query(1) == 4 && Actor_Query_Goal_Number(kActorOfficerLeary) < 300) {
		AI_Movement_Track_Flush(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, 300);
	} else if (Global_Variable_Query(1) == 5 && Actor_Query_Goal_Number(kActorOfficerLeary) < 400) {
		AI_Movement_Track_Flush(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, 400);
	} else if (!Game_Flag_Query(182) && Game_Flag_Query(147) == 1 && Game_Flag_Query(163) == 1 && Player_Query_Current_Scene() != 78 && Global_Variable_Query(1) < 3) {
		Game_Flag_Set(186);
		Actor_Set_Goal_Number(kActorOfficerLeary, 3);
	} else if (Actor_Query_Goal_Number(kActorOfficerLeary) != 1 && Actor_Query_Goal_Number(kActorOfficerLeary) != 2 && Game_Flag_Query(199) == 1) {
		Game_Flag_Reset(199);
	} else if (Global_Variable_Query(15) > 4 && !Actor_Clue_Query(kActorOfficerLeary, kClueMcCoyIsStupid)) {
		Actor_Clue_Acquire(kActorOfficerLeary, kClueMcCoyIsStupid, 1, -1);
	} else if (Game_Flag_Query(629) == 1) {
		Game_Flag_Reset(629);
	} else if (Game_Flag_Query(623) == 1 && !Game_Flag_Query(664)) {
		Game_Flag_Set(664);
		Actor_Set_Goal_Number(kActorOfficerLeary, 305);
	} else if (Actor_Query_Goal_Number(kActorOfficerLeary) == 310 && Actor_Query_Which_Set_In(kActorOfficerLeary) != Player_Query_Current_Set()) {
		Non_Player_Actor_Combat_Mode_Off(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, 305);
	} else if (Actor_Query_Goal_Number(kActorOfficerLeary) == 599 && Actor_Query_Which_Set_In(kActorOfficerLeary) != Player_Query_Current_Set()) {
		Actor_Set_Health(kActorOfficerLeary, 40, 40);
		Actor_Set_Goal_Number(kActorOfficerLeary, 305);
	} else if (Actor_Query_Goal_Number(kActorOfficerLeary) == 305) {
		switch (Actor_Query_Which_Set_In(kActorOfficerLeary)) {
		case 7:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 0, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 20:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 1, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 33:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 5, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 53:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 7, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 54:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 3, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 70:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 18, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 74:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 11, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 77:
		case 78:
		case 79:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 10, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 81:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 13, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 83:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 14, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 84:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 16, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		case 86:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 17, 4, 7, 8, -1, -1, -1, 10, 300, 0);
			}
			break;
		}
	}
	return false;
}

void AIScriptOfficerLeary::TimerExpired(int timer) {
	if (timer == 1) {
		AI_Countdown_Timer_Reset(kActorOfficerLeary, 1);
		if (Actor_Query_In_Set(kActorMcCoy, 41)) {
			Actor_Set_Goal_Number(kActorOfficerLeary, 430);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 430);
		} else {
			Game_Flag_Set(684);
		}
	} else if (timer == 2) {
		AI_Countdown_Timer_Reset(kActorOfficerLeary, 2);
		sub_431420();
	}
}

void AIScriptOfficerLeary::CompletedMovementTrack() {
	int v0;
	unsigned int v1;

	v0 = Actor_Query_Goal_Number(kActorOfficerLeary);
	if (v0 == 1) {
		Actor_Set_Goal_Number(kActorOfficerLeary, 2);
		return;
	}
	//todo: tidyup
	v1 = v0 - 305;
	if (v1 > 3) {
		return;
	}
	if (!v1) {
		Actor_Set_Goal_Number(kActorOfficerLeary, 306);
		return;
	}
	if (v1 != 2) {
		if (v1 == 3) {
			Actor_Change_Animation_Mode(kActorOfficerLeary, 4);
			Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
			Actor_Set_Goal_Number(kActorOfficerLeary, 309);
		}
		return;
	}
	Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 0, 1, kActorMcCoy, 12, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 15, 300, 0);
}

void AIScriptOfficerLeary::ReceivedClue(int clueId, int fromActorId) {
	if (clueId == 222) {
		Actor_Modify_Friendliness_To_Other(kActorOfficerLeary, kActorMcCoy, 5);
	}
	if (clueId == 215) {
		Actor_Modify_Friendliness_To_Other(kActorOfficerLeary, kActorMcCoy, -4);
	}
}

void AIScriptOfficerLeary::ClickedByPlayer() {}

void AIScriptOfficerLeary::EnteredScene(int sceneId) {}

void AIScriptOfficerLeary::OtherAgentEnteredThisScene(int otherActorId) {}

void AIScriptOfficerLeary::OtherAgentExitedThisScene(int otherActorId) {}

void AIScriptOfficerLeary::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {}

void AIScriptOfficerLeary::ShotAtAndMissed() {}

void AIScriptOfficerLeary::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorOfficerLeary) == 307) {
		Actor_Set_Health(kActorOfficerLeary, 50, 50);
	}
}

void AIScriptOfficerLeary::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorOfficerLeary, 599);
	Game_Flag_Set(607);
}

int AIScriptOfficerLeary::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	if (otherActorId != kActorMcCoy) {
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

bool AIScriptOfficerLeary::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append(kActorOfficerLeary, 57, 7);
			AI_Movement_Track_Append(kActorOfficerLeary, 58, 7);
		} else {
			AI_Movement_Track_Append(kActorOfficerLeary, 58, 7);
			AI_Movement_Track_Append(kActorOfficerLeary, 57, 7);
		}
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;
	case 2:
		if (Random_Query(1, 3) == 1) {
			if (Random_Query(1, 2) == 1 && !Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewA)) {
				Actor_Clue_Acquire(kActorOfficerLeary, kClueCrowdInterviewA, 0, -1);
			} else if (!Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewB)) {
				Actor_Clue_Acquire(kActorOfficerLeary, kClueCrowdInterviewB, 0, -1);
			}
		}
		if (Game_Flag_Query(182)) {
			if (Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewA) && Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewB)) {
				Actor_Set_Goal_Number(kActorOfficerLeary, 0);
			} else {
				Actor_Set_Goal_Number(kActorOfficerLeary, 1);
			}
		} else {
			Actor_Set_Goal_Number(kActorOfficerLeary, 0);
		}
		return true;
	case 3:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append(kActorOfficerLeary, 39, Random_Query(120, 240));
		AI_Movement_Track_Append(kActorOfficerLeary, 35, 0);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;
	case 99:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		return false;
	case 102:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append(kActorOfficerLeary, 107, 0);
		AI_Movement_Track_Append(kActorOfficerLeary, 108, 0);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;
	case 300:
		Actor_Set_Goal_Number(kActorOfficerLeary, 305);
		return true;
	case 305:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		switch (Random_Query(1, 10)) {
		case 1:
			AI_Movement_Track_Append(kActorOfficerLeary, 398, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 399, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 400, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 401, 0);
			AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 402, 3, 276);
			AI_Movement_Track_Append(kActorOfficerLeary, 403, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 404, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 2:
			AI_Movement_Track_Append(kActorOfficerLeary, 385, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 242, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 386, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 3:
			AI_Movement_Track_Append(kActorOfficerLeary, 390, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 391, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 392, 5);
			AI_Movement_Track_Append(kActorOfficerLeary, 345, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 393, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 4:
			AI_Movement_Track_Append(kActorOfficerLeary, 381, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 382, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 383, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 382, 3);
			AI_Movement_Track_Append(kActorOfficerLeary, 384, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 5:
			AI_Movement_Track_Append(kActorOfficerLeary, 388, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 389, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 6:
			AI_Movement_Track_Append(kActorOfficerLeary, 385, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 242, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 386, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 7:
			AI_Movement_Track_Append(kActorOfficerLeary, 394, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 395, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 396, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 397, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 396, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 395, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 430, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
		case 8:
			switch (Random_Query(1, 7)) {
			case 1:
				AI_Movement_Track_Append(kActorOfficerLeary, 302, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 407, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 408, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
			case 2:
				AI_Movement_Track_Append(kActorOfficerLeary, 536, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 537, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 538, 1);
				AI_Movement_Track_Append(kActorOfficerLeary, 537, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 536, 0);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
			case 3:
				AI_Movement_Track_Append(kActorOfficerLeary, 296, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 409, 2);
				AI_Movement_Track_Append(kActorOfficerLeary, 296, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
			case 4:
				AI_Movement_Track_Append(kActorOfficerLeary, 411, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 412, 5);
				AI_Movement_Track_Append(kActorOfficerLeary, 411, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
			case 5:
				AI_Movement_Track_Append(kActorOfficerLeary, 413, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 414, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 431, 0, 1017);
				AI_Movement_Track_Append(kActorOfficerLeary, 432, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
			case 6:
				AI_Movement_Track_Append(kActorOfficerLeary, 415, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 416, 0, 620);
				AI_Movement_Track_Append(kActorOfficerLeary, 417, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 418, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
			case 7:
				AI_Movement_Track_Append(kActorOfficerLeary, 405, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 406, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				return false;
			default:
				return false;
			}
			//no break
		case 9:
			if (Random_Query(1, 2) - 1 == 1) {
				AI_Movement_Track_Append(kActorOfficerLeary, 433, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 434, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 435, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				return false;
			}
			AI_Movement_Track_Append(kActorOfficerLeary, 420, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 422, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 421, 1);
			AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 422, 4, 182);
			AI_Movement_Track_Append(kActorOfficerLeary, 420, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			//no break
		case 10:
			AI_Movement_Track_Append(kActorOfficerLeary, 310, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 307, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 309, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 310, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			return false;
		default:
			return false;
		}
		//no break
	case 306:
		Actor_Set_Goal_Number(kActorOfficerLeary, 305);
		return true;
	case 307:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 416, 0, 556);
		Actor_Change_Animation_Mode(kActorOfficerLeary, 4);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;
	case 308:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append_Run_With_Facing(kActorOfficerLeary, 440, 2, 355);
		AI_Movement_Track_Append_Run_With_Facing(kActorOfficerLeary, 441, 0, 825);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;
	case 400:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append(kActorOfficerLeary, 35, 0);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, 410);
		return true;
	case 420:
		AI_Countdown_Timer_Reset(kActorOfficerLeary, 1);
		AI_Countdown_Timer_Start(kActorOfficerLeary, 1, 120);
		Actor_Set_Goal_Number(kActorOfficerLeary, 410);
		return true;
	case 425:
		AI_Countdown_Timer_Reset(kActorOfficerLeary, 1);
		AI_Countdown_Timer_Start(kActorOfficerLeary, 1, 60);
		Actor_Set_Goal_Number(kActorOfficerLeary, 410);
		return true;
	case 430:
		Actor_Set_Goal_Number(kActorOfficerLeary, 410);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 410);
		return false;
	default:
		return false;
	}
}

bool AIScriptOfficerLeary::UpdateAnimation(int *animation, int *frame) {
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
			Actor_Change_Animation_Mode(kActorOfficerLeary, 0);
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
			Actor_Change_Animation_Mode(kActorOfficerLeary, 0);
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
			Actor_Change_Animation_Mode(kActorOfficerLeary, 4);
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
			Actor_Change_Animation_Mode(kActorOfficerLeary, 4);
		}
		*frame = var_45D5B4_animation_frame;
		return true;
	case 24:
		*animation = 581;
		var_45D5B4_animation_frame++;
		if (var_45D5B4_animation_frame == 4) {
			if (Random_Query(1, 2) == 1) {
				Sound_Play_Speech_Line(kActorOfficerLeary, 9010, 75, 0, 99);
			} else {
				Sound_Play_Speech_Line(kActorOfficerLeary, 9015, 75, 0, 99);
			}
		}
		if (var_45D5B4_animation_frame == 5) {
			Actor_Combat_AI_Hit_Attempt(kActorOfficerLeary);
		}
		if (var_45D5B4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(581)) {
			var_45D5B0_animation_state = 21;
			var_45D5B4_animation_frame = 0;
			*animation = 571;
			Actor_Change_Animation_Mode(kActorOfficerLeary, 4);
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

bool AIScriptOfficerLeary::ChangeAnimationMode(int mode) {
	int v1;

	switch (mode) {
	case kAnimationModeIdle:
		switch (var_45D5B0_animation_state) {
		case 21:
		case 24:
			var_45D5B0_animation_state = 23;
			var_45D5B4_animation_frame = 0;
			break;
		case 20:
			v1 = var_45D5B4_animation_frame;
			Actor_Change_Animation_Mode(kActorOfficerLeary, 4);
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
			break;
		default:
			var_45D5B0_animation_state = 0;
			var_45D5B4_animation_frame = 0;
			var_45D5B8 = Random_Query(0, 1);
			break;
		}
		break;
	case kAnimationModeWalk:
		var_45D5B0_animation_state = 3;
		var_45D5B4_animation_frame = 0;
		break;
	case kAnimationModeRun:
		var_45D5B0_animation_state = 4;
		var_45D5B4_animation_frame = 0;
		break;
	case 3:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 11;
			var_462884 = 592;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 11;
			var_45D5B4_animation_frame = 0;
			var_45D5BC = 0;
		}
		break;
	case kAnimationModeCombatIdle:
		switch (var_45D5B0_animation_state) {
		case 21:
		case 22:
		case 24:
			break;
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
	case 6:
		var_45D5B0_animation_state = 24;
		var_45D5B4_animation_frame = 0;
		break;
	case kAnimationModeCombatWalk:
		var_45D5B0_animation_state = 7;
		var_45D5B4_animation_frame = 0;
		break;
	case kAnimationModeCombatRun:
		var_45D5B0_animation_state = 8;
		var_45D5B4_animation_frame = 0;
		break;
	case 12:
		if (var_45D5B0_animation_state == 0 || var_45D5B0_animation_state == 1) {
			var_45D5B0_animation_state = 2;
			var_462880 = 12;
			var_462884 = 593;
			if (Game_Flag_Query(199) == 1) {
				var_45D5B4_animation_frame = 0;
			}
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 12;
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
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 13;
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
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 14;
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
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 15;
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
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 16;
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
		}
		else if (var_45D5B0_animation_state < 11 || var_45D5B0_animation_state > 19) {
			var_45D5B0_animation_state = 17;
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
	case 22:
		if (Random_Query(0, 1)) {
			var_45D5B0_animation_state = 25;
		} else {
			var_45D5B0_animation_state = 26;
		}
		var_45D5B4_animation_frame = 0;
		break;
	case 27:
		Game_Flag_Reset(199);
		break;
	case 28:
		Game_Flag_Set(199);
		break;
	case 44:
		var_45D5B0_animation_state = 5;
		var_45D5B4_animation_frame = 0;
		break;
	case 45:
		var_45D5B0_animation_state = 6;
		var_45D5B4_animation_frame = 0;
		break;
	case 46:
		var_45D5B0_animation_state = 9;
		var_45D5B4_animation_frame = 0;
		break;
	case 47:
		var_45D5B0_animation_state = 10;
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
	case 58:
		var_45D5B0_animation_state = 20;
		var_45D5B4_animation_frame = 0;
		break;
	}
	return true;
}

void AIScriptOfficerLeary::QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4) {
	*animationState = var_45D5B0_animation_state;
	*animationFrame = var_45D5B4_animation_frame;
	*a3 = var_462880;
	*a4 = var_462884;
}

void AIScriptOfficerLeary::SetAnimationState(int animationState, int animationFrame, int a3, int a4) {
	var_45D5B0_animation_state = animationState;
	var_45D5B4_animation_frame = animationFrame;
	var_462880 = a3;
	var_462884 = a4;
}

bool AIScriptOfficerLeary::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 57 || waypointId == 58) {
		sub_431408();
		AI_Countdown_Timer_Reset(kActorOfficerLeary, 2);
		AI_Countdown_Timer_Start(kActorOfficerLeary, 2, 6);
	}
	return true;
}

void AIScriptOfficerLeary::FledCombat() {
	Actor_Set_Goal_Number(kActorOfficerLeary, 300);
}

bool AIScriptOfficerLeary::sub_431408() {
	Game_Flag_Set(199);
	return true;
}

bool AIScriptOfficerLeary::sub_431420() {
	Game_Flag_Reset(199);
	return true;
}
} // End of namespace BladeRunner
