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

AIScriptGordo::AIScriptGordo(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	var_45B078 = 0;
	_counter = 0;
	_counterTarget = 0;
	_frameMin = 0;
	_frameDelta = 0;
	_frameMax = 0;
	_state = 0;
}

void AIScriptGordo::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationStateNext = 0;
	var_45B078 = 0;
	_counter = 0;
	_counterTarget = 0;
	_frameMin = 0;
	_frameDelta = 1;
	_frameMax = 0;
	_state = 0;
	Actor_Set_Goal_Number(kActorGordo, 0);
}

bool AIScriptGordo::Update() {
	if (Global_Variable_Query(kVariableChapter) == 1) {
		if (Actor_Query_Goal_Number(kActorGordo) == 0 && Actor_Query_Friendliness_To_Other(kActorGordo, kActorMcCoy) < 48 && Actor_Query_Is_In_Current_Set(kActorGordo) == kSetBB02_BB04_BB06_BB51) {
			Actor_Set_Goal_Number(kActorGordo, 90);
			return true;
		}
		if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetCT03_CT04 && Actor_Query_Which_Set_In(kActorGordo) == kSetCT01_CT12) {
			Actor_Set_Goal_Number(kActorGordo, 91);
			return true;
		}
	}

	if (Global_Variable_Query(kVariableChapter) == 2 && Actor_Query_Goal_Number(kActorGordo) < 100) {
		Actor_Set_Goal_Number(kActorGordo, 100);
	}

	if (Global_Variable_Query(kVariableChapter) == 3) {
		if (Actor_Query_Goal_Number(kActorGordo) < 200) {
			Actor_Set_Goal_Number(kActorGordo, 200);
		}
		if (Actor_Query_Goal_Number(kActorGordo) == 254) {
			Actor_Set_Goal_Number(kActorGordo, 255);
		}
	}

	if (Global_Variable_Query(kVariableChapter) == 4) {
		if (Actor_Query_Goal_Number(kActorGordo) < 300 && Actor_Query_Goal_Number(kActorGordo) != 260) {
			Actor_Set_Goal_Number(kActorGordo, 300);
		} else if (Actor_Query_Goal_Number(kActorGordo) == 599) {
			Actor_Put_In_Set(kActorGordo, kSetFreeSlotI);
			Actor_Set_At_Waypoint(kActorGordo, 41, 0);
		}
	}

	if (Global_Variable_Query(kVariableChapter) == 5 && Actor_Query_Goal_Number(kActorGordo) < 400) {
		Actor_Set_Goal_Number(kActorGordo, 400);
	}

	return false;
}

void AIScriptGordo::TimerExpired(int timer) {
	if (timer == 0) {
		AI_Countdown_Timer_Reset(kActorGordo, 0);
		if (Player_Query_Combat_Mode()) {
			Actor_Set_Goal_Number(kActorGordo, 243);
		} else {
			Actor_Set_Goal_Number(kActorGordo, 242);
		}
	}
	//return false;
}

void AIScriptGordo::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorGordo) == 1 || Actor_Query_Goal_Number(kActorGordo) == 91) {
		Actor_Set_Goal_Number(kActorGordo, 2);
		return;// true;
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 3) {
		if (Player_Query_Current_Set() == kSetCT05) {
			Actor_Force_Stop_Walking(kActorMcCoy);
			Player_Loses_Control();
			Player_Set_Combat_Mode(true);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim); // TODO: check this, it was set directly by calling actor script
			Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
			Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
			Actor_Says(kActorGordo, 50, 13);
			Actor_Says(kActorMcCoy, 465, kAnimationModeCombatAim);
			Actor_Says(kActorGordo, 60, 14);
			Player_Set_Combat_Mode(false);
			Player_Gains_Control();
		}
		Actor_Clue_Acquire(kActorGordo, kClueMcCoyRetiredZuben, 1, -1);
		Actor_Set_Goal_Number(kActorGordo, 4);
		return;// true;
	}

	if (Actor_Query_Goal_Number(kActorGordo) == 4) {
		Actor_Set_Goal_Number(kActorGordo, 5);
		return;// true;
	}

	if (Actor_Query_Goal_Number(kActorGordo) == 93) {
		Actor_Set_Immunity_To_Obstacles(kActorGordo, false);
		Player_Gains_Control();
		Actor_Set_Goal_Number(kActorGordo, 92);
		return;// true;
	}

	if (Actor_Query_Goal_Number(kActorGordo) > 100 && Actor_Query_Goal_Number(kActorGordo) < 200) {
		Actor_Set_Goal_Number(kActorGordo, 100);
		return;// true;
	}

	if (Actor_Query_Goal_Number(kActorGordo) == 202) {
		if (Player_Query_Current_Set() == kSetNR02) {
			Ambient_Sounds_Play_Sound(581, 58, 0, 0, 0);
			Actor_Face_Heading(kActorGordo, 0, false);
			Loop_Actor_Travel_Stairs(kActorGordo, 4, true, kAnimationModeIdle);
			Actor_Face_Heading(kActorGordo, 506, false);
			Actor_Says(kActorGordo, 280, 14);
		} else {
			Actor_Set_At_Waypoint(kActorGordo, 357, 506);
		}
		Actor_Set_Goal_Number(kActorGordo, 205);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 220) {
		Game_Flag_Set(594);
		Actor_Set_Goal_Number(kActorGordo, 221);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 222) {
		if (Game_Flag_Query(kFlagGordoIsReplicant)) {
			if (Global_Variable_Query(40) == 2) {
				Actor_Set_Goal_Number(kActorGordo, 250);
			} else {
				Actor_Set_Goal_Number(kActorGordo, 230);
			}
		} else {
			Actor_Set_Goal_Number(kActorGordo, 240);
		}
		Scene_Exits_Enable();
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 225) {
		Actor_Set_Goal_Number(kActorGordo, 200);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 243) {
		Music_Stop(2);
		Actor_Set_Goal_Number(kActorGordo, 280);
	}
	// return false;
}

void AIScriptGordo::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptGordo::ClickedByPlayer() {
	int goal = Actor_Query_Goal_Number(kActorGordo);
	if (goal == 101 || goal == 102) {
		sub_40FD00();
	} else if (goal == 260) {
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		Actor_Says(kActorMcCoy, 8600, 11);
		Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
		Actor_Says(kActorGordo, 1390, 16);
	} else if (goal == 599) {
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		Actor_Says(kActorMcCoy, 8665, 14);
	}
	//return false;
}

void AIScriptGordo::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptGordo::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptGordo::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptGordo::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptGordo::ShotAtAndMissed() {
	// return false;
}

bool AIScriptGordo::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorGordo) > 219 && Actor_Query_Goal_Number(kActorGordo) < 223) {
		Actor_Set_Health(kActorGordo, 50, 50);
		return true;
	}
	if (Actor_Query_Goal_Number(kActorGordo) > 239 && Actor_Query_Goal_Number(kActorGordo) < 245) {
		Actor_Set_Goal_Number(kActorGordo, 299);
		Delay(1500);
		Actor_Voice_Over(2410, kActorVoiceOver);
		Actor_Voice_Over(2420, kActorVoiceOver);
		Game_Flag_Set(532);
		Actor_Set_Goal_Number(kActorMcCoy, 500);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 250) {
		Actor_Set_Goal_Number(kActorGordo, 254);
		return true;
	}
	return false;
}

void AIScriptGordo::Retired(int byActorId) {
	if (Actor_Query_Goal_Number(kActorGordo) == 231) {
		Player_Loses_Control();
		Game_Flag_Set(592);
		Actor_Set_Goal_Number(kActorGordo, 599);
		Delay(2000);
		Player_Set_Combat_Mode(false);
		Actor_Voice_Over(1410, kActorVoiceOver);
		Actor_Voice_Over(1430, kActorVoiceOver);
		Actor_Voice_Over(1440, kActorVoiceOver);
		if (Query_Difficulty_Level() > 0) {
			Global_Variable_Increment(kVariableChinyen, 200);
		}
		Player_Gains_Control();
		Scene_Exits_Enable();
	}
	if (Actor_Query_In_Set(kActorGordo, kSetKP07)) {
		Global_Variable_Decrement(kVariableReplicants, 1);
		Actor_Set_Goal_Number(kActorGordo, 599);
		if (Global_Variable_Query(kVariableReplicants) == 0) {
			Player_Loses_Control();
			Delay(2000);
			Player_Set_Combat_Mode(false);
			Player_Gains_Control();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(579);
			Game_Flag_Reset(653);
			Set_Enter(kSetKP05_KP06, kSceneKP06);
			return;// true;
		}
	}
	Actor_Set_Goal_Number(kActorGordo, 599);
	//return false;
}

int AIScriptGordo::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptGordo::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 0:
		Actor_Put_In_Set(kActorGordo, kSetCT01_CT12);
		Actor_Set_At_XYZ(kActorGordo, -308.44f, -12.5f, 442.78f, 256);
		Actor_Change_Animation_Mode(kActorGordo, 53);
		break;
	case 3:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 119, 0);
		AI_Movement_Track_Append(kActorGordo, 118, 0);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 4:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 119, 0);
		AI_Movement_Track_Append(kActorGordo, 33, 1);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 90:
		Game_Flag_Set(kFlagCT01McCoyTalkedToGordo);
		Actor_Set_Goal_Number(kActorGordo, 99);
		Actor_Change_Animation_Mode(kActorGordo, 29);
		break;
	case 91:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 43, 0);
		AI_Movement_Track_Append(kActorGordo, 33, 1);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 92:
		Actor_Face_Actor(kActorGordo, kActorHowieLee, true);
		Actor_Says(kActorGordo, 0, 13);
		Actor_Says(kActorGordo, 10, 16);
		Actor_Set_Goal_Number(kActorGordo, 91);
		Player_Gains_Control();
		break;
	case 93:
		Player_Loses_Control();
		Actor_Set_Immunity_To_Obstacles(kActorGordo, true);
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 100, 0);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 100: {
			AI_Movement_Track_Flush(kActorGordo);
			_animationState = 0;
			_animationFrame = 0;
			_animationStateNext = 0;
			sub_41117C();
			int rnd = Random_Query(1, 4);
			if (Actor_Clue_Query(kActorMcCoy, kClueGordoInterview1) || Actor_Clue_Query(kActorMcCoy, kClueGordoInterview2)) {
				rnd = Random_Query(4, 5);
			}
			switch (rnd) {
			case 1:
			case 2:
				Actor_Set_Goal_Number(kActorGordo, 101);
				break;
			case 3:
				Actor_Set_Goal_Number(kActorGordo, 102);
				break;
			case 4:
				Actor_Set_Goal_Number(kActorGordo, 103);
				break;
			case 5:
				Actor_Set_Goal_Number(kActorGordo, 104);
				break;
			}
		}
		break;
	case 101:
		AI_Movement_Track_Flush(kActorGordo);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append(kActorGordo, 343, 5);
			AI_Movement_Track_Append(kActorGordo, 344, 0);
			AI_Movement_Track_Append(kActorGordo, 345, 0);
			AI_Movement_Track_Append(kActorGordo, 346, 0);
			AI_Movement_Track_Append(kActorGordo, 347, 0);
			AI_Movement_Track_Append(kActorGordo, 348, 5);
		} else {
			AI_Movement_Track_Append(kActorGordo, 348, 5);
			AI_Movement_Track_Append(kActorGordo, 347, 0);
			AI_Movement_Track_Append(kActorGordo, 346, 0);
			AI_Movement_Track_Append(kActorGordo, 345, 0);
			AI_Movement_Track_Append(kActorGordo, 344, 0);
			AI_Movement_Track_Append(kActorGordo, 343, 5);
		}
		if (Game_Flag_Query(kFlagGordoIsReplicant)) {
			AI_Movement_Track_Append(kActorGordo, 33, Random_Query(5, 20));
		} else {
			AI_Movement_Track_Append(kActorGordo, 40, Random_Query(5, 20));
		}
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 102:
		if (Random_Query(1, 3) == 1) {
			AI_Movement_Track_Append(kActorGordo, 54, 1);
			AI_Movement_Track_Append(kActorGordo, 56, 0);
			AI_Movement_Track_Append(kActorGordo, 43, 1);
		} else {
			AI_Movement_Track_Append(kActorGordo, 43, 1);
			AI_Movement_Track_Append(kActorGordo, 56, 0);
			AI_Movement_Track_Append(kActorGordo, 54, 1);
		}
		AI_Movement_Track_Append(kActorGordo, 40, 15);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 103:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 40, Random_Query(15, 45));
		if (Game_Flag_Query(kFlagGordoIsReplicant)) {
			AI_Movement_Track_Append(kActorGordo, 33, Random_Query(10, 30));
		} else {
			AI_Movement_Track_Append(kActorGordo, 33, Random_Query(5, 15));
		}
		AI_Movement_Track_Append(kActorGordo, 39, Random_Query(5, 30));
		AI_Movement_Track_Append(kActorGordo, 42, Random_Query(10, 20));
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 104:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 33, Random_Query(15, 45));
		if (!Game_Flag_Query(kFlagGordoIsReplicant)) {
			AI_Movement_Track_Append(kActorGordo, 40, Random_Query(15, 45));
		}
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 200:
		if (Global_Variable_Query(40) == 2) {
			if (Game_Flag_Query(593) && Game_Flag_Query(591) && Player_Query_Current_Scene() != kSceneNR02) {
				Actor_Set_Goal_Number(kActorGordo, 201);
			} else {
				Actor_Set_Goal_Number(kActorGordo, 225);
			}
		} else {
			Actor_Set_Goal_Number(kActorGordo, 201);
		}
		break;
	case 201:
		AI_Movement_Track_Flush(kActorGordo);
		Actor_Put_In_Set(kActorGordo, kSetNR02);
		Actor_Set_At_XYZ(kActorGordo, 148.12f, -24.0f, 456.04f, 506);
		break;
	case 202:
		Player_Gains_Control();
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 356, 0);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 205:
		switch(Global_Variable_Query(41)) {
		case 0:
			Global_Variable_Increment(41, 1);
			Actor_Set_Goal_Number(kActorGordo, 206);
			break;
		case 1:
			Global_Variable_Increment(41, 1);
			Actor_Set_Goal_Number(kActorGordo, 207);
			break;
		case 2:
			Global_Variable_Increment(41, 1);
			Actor_Set_Goal_Number(kActorGordo, 208);
			break;
		case 3:
			Global_Variable_Increment(41, 1);
			Actor_Set_Goal_Number(kActorGordo, 210);
			break;
		}
		break;
	case 206:
		ADQ_Add_Pause(5000);
		ADQ_Add(kActorGordo, 630, 15);
		ADQ_Add(kActorGordo, 640, 16);
		ADQ_Add_Pause(1000);
		ADQ_Add(kActorGordo, 650, 14);
		break;
	case 207:
		ADQ_Add_Pause(1000);
		ADQ_Add(kActorGordo, 660, 16);
		ADQ_Add(kActorGordo, 670, 14);
		ADQ_Add(kActorGordo, 680, 13);
		ADQ_Add(kActorGordo, 690, 17);
		break;
	case 208:
		ADQ_Add_Pause(1000);
		ADQ_Add(kActorGordo, 700, 15);
		ADQ_Add(kActorGordo, 710, 13);
		break;
	case 210:
		Actor_Force_Stop_Walking(0);
		Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
		Actor_Says(kActorGordo, 720, 16);
		Actor_Says(kActorGordo, 730, 18);
		Music_Stop(1);
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		Actor_Says(kActorGordo, 740, 17);
		Sound_Play(575, 50, 0, 0, 50);
		Sound_Play(319, 50, 0, 0, 50);
		Actor_Says(kActorGordo, 750, 16);
		Actor_Says(kActorGordo, 760, 15);
		Actor_Says(kActorGordo, 770, 14);
		Actor_Says(kActorGordo, 780, 13);
		Actor_Says(kActorMcCoy, 3885, 16);
		if (Game_Flag_Query(kFlagGordoIsReplicant)) {
			Actor_Says(kActorGordo, 850, 12);
			Actor_Says(kActorGordo, 860, 15);
			Actor_Says(0, 3910, 16);
			if (Global_Variable_Query(40) == 2) {
				Actor_Says(kActorGordo, 870, 16);
				Sound_Play(576, 50, 0, 0, 50);
				Sound_Play(319, 50, 0, 0, 50);
				Actor_Set_Goal_Number(33, 241);
				Actor_Says(kActorGordo, 880, 17);
				Actor_Set_Goal_Number(kActorGordo, 211);
				Player_Loses_Control();
			} else {
				Actor_Set_Goal_Number(kActorGordo, 220);
			}
		} else {
			Actor_Says(kActorGordo, 790, 12);
			Actor_Says(kActorMcCoy, 3890, 15);
			Sound_Play(577, 50, 0, 0, 50);
			Sound_Play(321, 50, 0, 0, 50);
			Actor_Says(kActorGordo, 800, 15);
			Actor_Says(kActorGordo, 810, 17);
			Actor_Says(kActorMcCoy, 3895, 16);
			Actor_Says(kActorGordo, 820, 14);
			Actor_Says(kActorMcCoy, 3900, 14);
			Actor_Says(kActorGordo, 830, 15);
			Actor_Says(kActorMcCoy, 3905, 13);
			Actor_Says(kActorGordo, 840, 13);
			Sound_Play(578, 50, 0, 0, 50);
			Sound_Play(321, 50, 0, 0, 50);
			Actor_Set_Goal_Number(kActorGordo, 220);
		}
		break;
	case 215:
		sub_4103B8();
		break;
	case 220:
		Actor_Set_Targetable(kActorGordo, true);
		Scene_Exits_Disable();
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append_Run(kActorGordo, 366, 0);
		AI_Movement_Track_Repeat(kActorGordo);
		Music_Play(1, 50, 0, 2, -1, 0, 0);
		break;
	case 221:
		Actor_Set_At_XYZ(kActorGordo, -90.91f, -24.0f, -14.71f, 708);
		Actor_Change_Animation_Mode(kActorGordo, 26);
		break;
	case 222:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append_Run(kActorGordo, 367, 0);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 225:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append(kActorGordo, 39, 10);
		AI_Movement_Track_Append(kActorGordo, 33, Random_Query(5, 30));
		AI_Movement_Track_Append(kActorGordo, 39, 10);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 230:
		Actor_Put_In_Set(kActorGordo, kSetNR01);
		Actor_Set_At_XYZ(kActorGordo, -194.24f, 23.88f, -851.98f, 343);
		break;
	case 240:
		Actor_Put_In_Set(kActorGordo, kSetNR01);
		Actor_Set_At_XYZ(kActorGordo, -194.24f, 23.88f, -851.98f, 343);
		break;
	case 241:
		ADQ_Add(kActorGordo, 170, 18);
		AI_Countdown_Timer_Start(kActorGordo, 0, 10);
		break;
	case 242:
		Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
		Actor_Says(kActorGordo, 180, 13);
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		Actor_Says(kActorMcCoy, 3075, 13);
		Actor_Says(kActorGordo, 190, 15);
		Loop_Actor_Walk_To_Actor(kActorGordo, 0, 60, false, true);
		Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		Actor_Says(kActorMcCoy, 3080, 15);
		Actor_Says(kActorGordo, 200, 15);
		Actor_Says(kActorMcCoy, 3085, 15);
		sub_410590();
		break;
	case 243:
		AI_Movement_Track_Flush(kActorGordo);
		AI_Movement_Track_Append_Run(kActorGordo, 369, 0);
		AI_Movement_Track_Append_Run(kActorGordo, 370, 0);
		AI_Movement_Track_Append(kActorGordo, 33, 0);
		AI_Movement_Track_Repeat(kActorGordo);
		break;
	case 250:
		Actor_Put_In_Set(kActorGordo, kSetNR01);
		Actor_Set_At_XYZ(kActorGordo, -194.24f, 23.88f, -851.98f, 343);
		Actor_Change_Animation_Mode(kActorGordo, 80);
		Actor_Set_Goal_Number(kActorSteele, 250);
		break;
	case 251:
		Actor_Change_Animation_Mode(kActorGordo, 83);
		break;
	case 255:
		ADQ_Flush();
		Actor_Change_Animation_Mode(kActorGordo, 83);
		Player_Loses_Control();
		Game_Flag_Reset(603);
		Actor_Set_Targetable(kActorSteele, false);
		Delay(3000);
		Actor_Set_Goal_Number(kActorMcCoy, 500);
		break;
	case 260:
		Actor_Set_Targetable(kActorGordo, false);
		break;
	case 280:
		Scene_Exits_Enable();
		Game_Flag_Set(592);
		Actor_Put_In_Set(kActorGordo, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorGordo, 33, 0);
		break;
	case 299:
		Music_Stop(2);
		AI_Countdown_Timer_Reset(kActorGordo, 0);
		ADQ_Flush();
		AI_Movement_Track_Flush(kActorGordo);
		if (Game_Flag_Query(kFlagGordoIsReplicant)) {
			Actor_Change_Animation_Mode(kActorGordo, 84);
		} else {
			Actor_Change_Animation_Mode(kActorGordo, 48);
		}
		Actor_Set_Goal_Number(kActorGordo, 599);
		Actor_Retired_Here(kActorGordo, 36, 18, 1, -1);
		break;
	case 400:
		AI_Movement_Track_Flush(kActorGordo);
		Actor_Put_In_Set(kActorGordo, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorGordo, 33, 0);
		break;
	}
	return false;
}

bool AIScriptGordo::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		switch (_state) {
		case 0:
			*animation = 116;
			if (_counter < _counterTarget) {
				_animationFrame += _frameDelta;
				if (_animationFrame > _frameMax) {
					_animationFrame = _frameMax;
					_frameDelta = -1;
				} else if (_animationFrame < _frameMin) {
					_animationFrame = _frameMin;
					_frameDelta = 1;
				}
				_counter++;
			} else {
				_animationFrame += _frameDelta;
				_counterTarget = 0;
				if (_animationFrame == 7 && Random_Query(0, 3)) {
					_frameDelta = -1;
					_counter = 0;
					_frameMin = 7;
					_frameMax = 10;
					_counterTarget = Random_Query(5, 20);
				}
				if (_animationFrame == 17) {
					if (Random_Query(0, 3)) {
						_frameDelta = -1;
						_counter = 0;
						_frameMin = 17;
						_frameMax = 20;
						_counterTarget = Random_Query(5, 20);
					}
				}
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
				}
				if (_animationFrame == 0) {
					if (Random_Query(0, 1)) {
						_state = Random_Query(1, 2);
					}
				}
			}
			break;
		case 1:
			*animation = 117;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(117)) {
				*animation = 116;
				_animationFrame = 0;
				_state = 0;
			}
			break;
		case 2:
			*animation = 118;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(118)) {
				*animation = 116;
				_animationFrame = 0;
				_state = 0;
			}
			break;
		}
		break;
	case 1:
		switch (_state) {
		case 0:
			*animation = 116;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(116) / 2) {
				_animationFrame += 2;
			} else {
				_animationFrame -= 2;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation) || _animationFrame <= 0) {
				*animation = _animationNext;
				_animationFrame = 0;
				_animationState = _animationStateNext;
			}
			break;
		case 1:
			*animation = 117;
			_animationFrame += 2;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(117)) {
				*animation = _animationNext;
				_animationFrame = 0;
				_animationState = _animationStateNext;
			}
			break;
		case 2:
			*animation = 118;
			_animationFrame += 2;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(118)) {
				*animation = _animationNext;
				_animationFrame = 0;
				_animationState = _animationStateNext;
			}
			break;
		}
		break;
	case 2:
		*animation = 114;
		if (Random_Query(0, 1)) {
			_animationFrame++;
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;
	case 3:
		*animation = 115;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(115)) {
			*animation = 116;
			_animationState = 0;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeIdle);
			Actor_Set_Goal_Number(kActorGordo, 93);
		}
		break;
	case 4:
		*animation = 120;
		if (_animationFrame == 0 && var_45B078) {
			*animation = 116;
			_animationState = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;
	case 5:
		*animation = 121;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(121)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 6:
		*animation = 122;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(122)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 7:
		*animation = 123;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(123)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 8:
		*animation = 124;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(124)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 9:
		*animation = 125;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(125)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 10:
		*animation = 126;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(126)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 11:
		*animation = 127;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(127)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 12:
		*animation = 127;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(127)) {
			*animation = 120;
			_animationFrame = 0;
			_animationState = 4;
		}
		break;
	case 13:
		*animation = 114;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(114)) {
			_animationFrame = 0;
		}
		break;
	case 14:
		*animation = 103;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(103)) {
			*animation = 93;
			_animationFrame = 0;
			_animationState = 16;
		}
		break;
	case 15:
		*animation = 104;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(104)) {
			*animation = 116;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	case 16:
		*animation = 93;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(93)) {
			_animationFrame = 0;
		}
		break;
	case 17:
		break;
	case 18:
		*animation = 105;
		_animationFrame++;
		if (_animationFrame == 1) {
			Sound_Play_Speech_Line(kActorGordo, Random_Query(0, 1) ? 9010 : 9015, 75, 0, 99);
		}
		if (_animationFrame == 2) {
			Actor_Combat_AI_Hit_Attempt(kActorGordo);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = 93;
			_animationFrame = 0;
			_animationState = 16;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeCombatIdle);
		}
		break;
	case 19:
		*animation = 111;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(111)) {
			*animation = 116;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeIdle);
		}
		break;
	case 20:
		*animation = 112;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(112)) {
			*animation = 116;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeIdle);
		}
		break;
	case 21:
		*animation = 96;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(96)) {
			*animation = 93;
			_animationFrame = 0;
			_animationState = 16;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeCombatIdle);
		}
		break;
	case 22:
		*animation = 97;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(97)) {
			*animation = 93;
			_animationFrame = 0;
			_animationState = 16;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeCombatIdle);
		}
		break;
	case 23:
		*animation = 113;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(113) - 1) {
			_animationFrame++;
		}
		break;
	case 24:
		*animation = 100;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(100) - 1) {
			++_animationFrame;
		}
		break;
	case 25:
		*animation = 107;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(107)) {
			_animationFrame = 0;
		}
		break;
	case 26:
		*animation = 108;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(108)) {
			_animationFrame = 0;
		}
		break;
	case 27:
		*animation = 98;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(98)) {
			_animationFrame = 0;
		}
		break;
	case 28:
		*animation = 99;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(99)) {
			_animationFrame = 0;
		}
		break;
	case 29:
		*animation = 109;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(109)) {
			_animationFrame = 0;
		}
		break;
	case 30:
		*animation = 110;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(110)) {
			_animationFrame = 0;
		}
		break;
	case 31:
		*animation = 101;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(101)) {
			_animationFrame = 0;
		}
		break;
	case 32:
		*animation = 102;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(102)) {
			_animationFrame = 0;
		}
		break;
	case 33:
		*animation = 106;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(106)) {
			*animation = 93;
			_animationState = 16;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeCombatIdle);
			if (Game_Flag_Query(594)) {
				Actor_Set_Goal_Number(kActorGordo, 222);
				Game_Flag_Set(592);
			}
		}
		break;
	case 34:
		*animation = 119;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(119)) {
			if (Game_Flag_Query(561)) {
				Game_Flag_Reset(561);
				if (Game_Flag_Query(kFlagGordoIsReplicant)) {
					Item_Add_To_World(89, 953, kSetNR02, 148.94f, 22.19f, 476.1f, 0, 6, 6, false, true, false, false);
				} else {
					Item_Add_To_World(90, 954, kSetNR02, 148.94f, 22.19f, 476.1f, 0, 6, 6, false, true, false, false);
				}
				Actor_Set_Goal_Number(kActorGordo, 202);
			}
			*animation = 116;
			_animationState = 0;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeIdle);
		}
		break;
	case 35:
		*animation = 128;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(128)) {
			_animationFrame = 0;
		}
		break;
	case 36:
		if (_animationFrame == 0 && var_45B078) {
			Actor_Change_Animation_Mode(kActorGordo, 80);
			*animation = 128;
			_animationState = 35;
		} else {
			*animation = 129;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(129)) {
				_animationFrame = 0;
			}
		}
		break;
	case 37:
		if (_animationFrame == 0 && var_45B078) {
			Actor_Change_Animation_Mode(kActorGordo, 80);
			*animation = 128;
			_animationState = 35;
		} else {
			*animation = 130;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(130)) {
				_animationFrame = 0;
			}
		}
		break;
	case 38:
		*animation = 131;
		_animationFrame++;
		if (_animationFrame == 7) {
			if (Actor_Query_Goal_Number(kActorGordo) == 255) {
				Actor_Set_Goal_Number(kActorTaffyPatron, 255);
			} else {
				Actor_Set_Goal_Number(kActorTaffyPatron, 250);
			}
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorGordo, kAnimationModeCombatIdle);
			*animation = 93;
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorGordo, 243);
		}
		break;
	case 39:
		*animation = 132;
		_animationFrame++;
		if (_animationFrame == 6) {
			Actor_Set_Goal_Number(kActorTaffyPatron, 250);
			Actor_Change_Animation_Mode(kActorGordo, 49);
			*animation = 100;
			_animationFrame = 0;
			_animationState = 24;
		}
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptGordo::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 0:
			break;
		case 1:
		case 3:
		case 38:
		case 39:
			_animationState = 0;
			_animationFrame = 0;
			_state = 0;
			_counterTarget = 0;
			break;
		case 2:
		case 13:
			Actor_Change_Animation_Mode(kActorGordo, 53);
			break;
		default:
			var_45B078 = 1;
			break;
		}
		break;
	case kAnimationModeWalk:
		_animationState = 25;
		_animationFrame = 0;
		break;
	case kAnimationModeRun:
		_animationState = 26;
		_animationFrame = 0;
		break;
	case kAnimationModeTalk:
		if (_animationState) {
			_animationState = 4;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 4;
			_animationNext = 120;
		}
		var_45B078 = 0;
		break;
	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 14:
		case 16:
		case 17:
		case 18:
		case 21:
		case 22:
			break;
		case 15:
		case 27:
		case 28:
		case 31:
		case 32:
			_animationState = 16;
			_animationFrame = 0;
			break;
		default:
			_animationState = 14;
			_animationFrame = 0;
			break;
		}
		break;
	case kAnimationModeCombatAttack:
		_animationState = 18;
		_animationFrame = 0;
		break;
	case kAnimationModeCombatWalk:
		_animationState = 27;
		_animationFrame = 0;
		break;
	case kAnimationModeCombatRun:
		_animationState = 28;
		_animationFrame = 0;
		break;
	case 12:
		if (_animationState) {
			_animationState = 5;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 5;
			_animationNext = 121;
		}
		var_45B078 = 0;
		break;
	case 13:
		if (_animationState) {
			_animationState = 6;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 6;
			_animationNext = 122;
		}
		var_45B078 = 0;
		break;
	case 14:
		if (_animationState) {
			_animationState = 7;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 7;
			_animationNext = 123;
		}
		var_45B078 = 0;
		break;
	case 15:
		if (_animationState) {
			_animationState = 8;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 8;
			_animationNext = 124;
		}
		var_45B078 = 0;
		break;
	case 16:
		if (_animationState) {
			_animationState = 9;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 9;
			_animationNext = 125;
		}
		var_45B078 = 0;
		break;
	case 17:
		if (_animationState) {
			_animationState = 10;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 10;
			_animationNext = 126;
		}
		var_45B078 = 0;
		break;
	case 18:
		if (_animationState) {
			_animationState = 11;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 11;
			_animationNext = 127;
		}
		var_45B078 = 0;
		break;
	case 19:
		if (_animationState) {
			_animationState = 12;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 12;
			_animationNext = 127;
		}
		var_45B078 = 0;
		break;
	case 21:
		switch (_animationState) {
		case 14:
		case 16:
		case 17:
		case 18:
			if (Random_Query(0, 1)) {
				_animationState = 21;
			} else {
				_animationState = 22;
			}
			_animationFrame = 0;
			break;
		case 19:
		case 20:
		case 21:
		case 22:
			return true;
		default:
			if (Random_Query(0, 1)) {
				_animationState = 19;
			} else {
				_animationState = 20;
			}
			_animationFrame = 0;
			break;
		}
		break;
	case 22:
		if (Random_Query(0, 1)) {
			_animationState = 21;
		} else {
			_animationState = 22;
		}
		_animationFrame = 0;
		break;
	case 23:
		_animationState = 34;
		_animationFrame = 0;
		break;
	case 26:
		_animationState = 33;
		_animationFrame = 0;
		break;
	case 29:
		_animationState = 3;
		_animationFrame = 0;
		break;
	case 30:
		_animationState = 13;
		_animationFrame = 0;
		break;
	case 44:
		_animationState = 29;
		_animationFrame = 0;
		break;
	case 45:
		_animationState = 30;
		_animationFrame = 0;
		break;
	case 46:
		_animationState = 31;
		_animationFrame = 0;
		break;
	case 47:
		_animationState = 32;
		_animationFrame = 0;
		break;
	case 48:
		switch (_animationState) {
		case 14:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			_animationState = 24;
			_animationFrame = 0;
			break;
		case 15:
			_animationState = 23;
			_animationFrame = 0;
			break;
		}
		break;
	case 49:
		_animationState = 24;
		_animationFrame = 0;
		break;
	case 53:
		_animationState = 2;
		_animationFrame = 0;
		break;
	case 80:
		_animationState = 35;
		_animationFrame = 0;
		break;
	case 81:
		_animationState = 36;
		_animationFrame = 0;
		var_45B078 = 0;
		break;
	case 82:
		_animationState = 37;
		_animationFrame = 0;
		var_45B078 = 0;
		break;
	case 83:
		_animationState = 38;
		_animationFrame = 0;
		break;
	case 84:
		_animationState = 39;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptGordo::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptGordo::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptGordo::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptGordo::FledCombat() {
	Scene_Exits_Enable();
	if (Global_Variable_Query(kVariableChapter) == 3) {
		Actor_Set_Goal_Number(kActorGordo, 280);
	}
	// return false;
}

void AIScriptGordo::sub_40FD00() {
	Player_Loses_Control();
	Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
	if (Game_Flag_Query(543)) {
		if (!Game_Flag_Query(272) || Actor_Clue_Query(kActorMcCoy, kClueGordoInterview1) || Actor_Clue_Query(kActorMcCoy, kClueGordoInterview2)) {
			if (Game_Flag_Query(544)) {
				switch(Random_Query(1, 4)) {
					case 1:
						Actor_Says(kActorMcCoy, 6460, 13);
						break;
					case 2:
						Actor_Says(kActorMcCoy, 6550, 14);
						break;
					case 3:
						Actor_Says(kActorMcCoy, 365, 12);
						break;
					case 4:
						Actor_Says(kActorMcCoy, 525, 12);
						break;
				}
			} else {
				Actor_Says(kActorMcCoy, 6490, 14);
				AI_Movement_Track_Pause(kActorGordo);
				Loop_Actor_Walk_To_Actor(kActorGordo, kActorMcCoy, 60, false, false);
				Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
				Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
				Actor_Says(kActorGordo, 990, 13);
				Actor_Says(kActorGordo, 1000, 15);
				Game_Flag_Set(544);
				AI_Movement_Track_Unpause(kActorGordo);
			}
		} else {
			Actor_Says(kActorMcCoy, 6485, 12);
			AI_Movement_Track_Pause(kActorGordo);
			Loop_Actor_Walk_To_Actor(kActorGordo, kActorMcCoy, 48, false, false);
			Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
			if (Game_Flag_Query(kFlagGordoIsReplicant)) {
				Actor_Says(kActorGordo, 1010, 12);
				Actor_Says(kActorMcCoy, 6495, 14);
				Actor_Says(kActorGordo, 1020, 13);
				Actor_Says(kActorMcCoy, 6500, 14);
				Actor_Says(kActorGordo, 1030, 15);
				Actor_Clue_Acquire(kActorMcCoy, kClueGordoInterview1, 0, kActorGordo);
			} else {
				Actor_Says(kActorGordo, 1040, 12);
				Actor_Says(kActorGordo, 1050, 13);
				Actor_Says(kActorMcCoy, 6505, 14);
				Actor_Says(kActorGordo, 1060, 13);
				Actor_Says(kActorGordo, 1070, 14);
				Actor_Says(kActorMcCoy, 6510, 16);
				Actor_Says(kActorGordo, 1080, 15);
				Actor_Clue_Acquire(kActorMcCoy, kClueGordoInterview2, 0, kActorGordo);
			}
			AI_Movement_Track_Unpause(kActorGordo);
		}
	} else {
		Actor_Says(kActorMcCoy, 6460, 12);
		AI_Movement_Track_Pause(kActorGordo);
		Loop_Actor_Walk_To_Actor(kActorGordo, kActorMcCoy, 36, false, false);
		Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
		Actor_Says(kActorGordo, 890, 14);
		Actor_Says(kActorMcCoy, 6465, 15);
		Actor_Says(kActorGordo, 900, 13);
		Actor_Says(kActorGordo, 910, 13);
		Actor_Says(kActorMcCoy, 6470, 12);
		Actor_Says(kActorGordo, 920, 14);
		Actor_Says(kActorGordo, 930, 12);
		Actor_Says(kActorMcCoy, 6475, 13);
		Actor_Says(kActorGordo, 940, 12);
		Actor_Says(kActorGordo, 950, 13);
		Actor_Says(kActorMcCoy, 6480, 14);
		Actor_Says(kActorGordo, 960, 15);
		Actor_Says(kActorGordo, 970, 12);
		AI_Movement_Track_Unpause(kActorGordo);
		Game_Flag_Set(543);
	}
	return Player_Gains_Control();
}

void AIScriptGordo::sub_4103B8() {
	Player_Loses_Control();
	Actor_Face_Heading(kActorMcCoy, 308, false);
	Actor_Says(kActorMcCoy, 3210, kAnimationModeTalk);
	Actor_Face_Actor(kActorGordo, kActorMcCoy, true);
	Actor_Face_Actor(kActorMcCoy, kActorGordo, true);
	Actor_Says(kActorGordo, 290, 13);
	Actor_Says(kActorGordo, 300, 14);
	Actor_Says(kActorGordo, 310, 12);
	Actor_Says(kActorGordo, 320, 15);
	Actor_Says(kActorMcCoy, 3215, kAnimationModeTalk);
	Actor_Says(kActorMcCoy, 3220, 12);
	Actor_Says(kActorGordo, 330, 17);
	Actor_Says(kActorGordo, 350, 13);
	Actor_Says(kActorMcCoy, 3225, 18);
	Player_Gains_Control();
	sub_41090C();
	Player_Loses_Control();
	Game_Flag_Set(561);
	Actor_Face_Heading(kActorGordo, 506, false);
	Actor_Change_Animation_Mode(kActorGordo, 23);
}

void AIScriptGordo::sub_410590() {
	Music_Stop(5);
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(820, -1, 5, 7);
	DM_Add_To_List_Never_Repeat_Once_Selected(830, 7, 5, -1);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answer == 820) {
		Actor_Says(kActorMcCoy, 3090, 16);
		Actor_Says(kActorMcCoy, 3095, 15);
		Actor_Says(kActorGordo, 210, 17);
		Actor_Says(kActorGordo, 220, 14);
		Actor_Says(kActorGordo, 230, 15);
		Actor_Set_Goal_Number(kActorGordo, 260);
		Actor_Put_In_Set(kActorGordo, kSetPS09);
		Actor_Set_At_XYZ(kActorGordo, -479.15f, 0.0f, -197.84f, 547);
		Game_Flag_Reset(255);
		Game_Flag_Reset(256);
		Game_Flag_Set(251);
		Game_Flag_Set(592);
		Scene_Exits_Enable();
		Game_Flag_Reset(kFlagMcCoyAtNRxx);
		Game_Flag_Set(kFlagMcCoyAtPSxx);
		Set_Enter(kSetPS09, kScenePS09);
	} else if (answer == 830) {
		Actor_Says(kActorMcCoy, 3100, 16);
		Actor_Says(kActorGordo, 240, 14);
		if (Actor_Clue_Query(0, 102)) {
			Actor_Says(kActorMcCoy, 3105, 15);
			Actor_Says(kActorMcCoy, 3110, 17);
			Actor_Says(kActorGordo, 250, 13);
			Actor_Says(kActorGordo, 260, 18);
			Actor_Says(kActorMcCoy, 3115, 14);
			Actor_Says(kActorGordo, 270, 15);
			Actor_Clue_Acquire(kActorGordo, kClueMcCoyHelpedGordo, 1, -1);
		} else {
			Delay(1000);
			Actor_Says(kActorGordo, 570, 13);
		}
		Game_Flag_Set(592);
		Scene_Exits_Enable();
		Actor_Set_Goal_Number(kActorGordo, 243);
	}
}

void AIScriptGordo::sub_41090C() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(760, 2, 5, 6);
	DM_Add_To_List_Never_Repeat_Once_Selected(770, 6, 3, 1);
	if (Actor_Clue_Query(kActorMcCoy, kClueDektorasDressingRoom) || Actor_Clue_Query(kActorMcCoy, kClueWomanInAnimoidRow)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(780, 5, 5, 5);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueLucy)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(790, 5, 5, 5);
	}
	if (Actor_Clue_Query(kActorMcCoy, kClueStolenCheese)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(800, -1, 4, 8);
	}
	Dialogue_Menu_Add_DONE_To_List(810);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 760:
		Actor_Says(kActorMcCoy, 3230, kAnimationModeTalk);
		Actor_Says(kActorGordo, 360, 16);
		Actor_Says(kActorMcCoy, 3260, 15);
		Actor_Says(kActorGordo, 370, 13);
		Actor_Says(kActorGordo, 380, 12);
		Actor_Says(kActorGordo, 390, 14);
		if (Player_Query_Agenda() == kPlayerAgendaSurly) {
			Actor_Says(kActorMcCoy, 3265, 13);
			Actor_Says(kActorGordo, 400, 12);
			Actor_Modify_Friendliness_To_Other(kActorGordo, kActorMcCoy, -3);
		}
		break;
	case 770:
		Actor_Says(kActorMcCoy, 3235, kAnimationModeTalk);
		Actor_Says(kActorGordo, 410, 12);
		Actor_Says(kActorMcCoy, 3270, 15);
		Actor_Says(kActorGordo, 420, 14);
		Actor_Says(kActorMcCoy, 3275, 15);
		Actor_Says(kActorGordo, 430, 12);
		Actor_Says(kActorGordo, 440, 13);
		if (Game_Flag_Query(kFlagGordoIsReplicant)) {
			Actor_Says(kActorGordo, 450, 17);
			Actor_Says(kActorMcCoy, 3280, 15);
			Actor_Clue_Acquire(kActorMcCoy, kClueGordoInterview3, 0, kActorGordo);
		}
		Actor_Says(kActorGordo, 460, 15);
		break;
	case 780:
		Actor_Says(kActorMcCoy, 3240, kAnimationModeTalk);
		Actor_Says(kActorGordo, 470, 17);
		Actor_Says(kActorMcCoy, 3285, 16);
		Actor_Says(kActorGordo, 480, 13);
		Actor_Says(kActorGordo, 490, 12);
		Actor_Says(kActorGordo, 500, 13);
		Actor_Says(kActorMcCoy, 3290, 16);
		Actor_Says(kActorGordo, 510, 15);
		Actor_Says(kActorMcCoy, 3295, 14);
		Actor_Says(kActorGordo, 520, 12);
		Actor_Modify_Friendliness_To_Other(kActorGordo, kActorMcCoy, -4);
		break;
	case 790:
		Actor_Says(kActorMcCoy, 3245, kAnimationModeTalk);
		Actor_Says(kActorGordo, 470, 17);
		Actor_Says(kActorMcCoy, 3285, 16);
		Actor_Says(kActorGordo, 480, 13);
		Actor_Says(kActorGordo, 490, 12);
		Actor_Says(kActorGordo, 500, 13);
		Actor_Says(kActorMcCoy, 3290, 16);
		Actor_Says(kActorGordo, 510, 15);
		Actor_Says(kActorMcCoy, 3295, 14);
		Actor_Says(kActorGordo, 520, 12);
		Actor_Modify_Friendliness_To_Other(kActorGordo, kActorMcCoy, -1);
		break;
	case 800:
		Actor_Says(kActorMcCoy, 3250, kAnimationModeTalk);
		Actor_Says(kActorGordo, 530, 18);
		Actor_Says(kActorMcCoy, 3300, 15);
		Actor_Says(kActorGordo, 540, 13);
		Actor_Says(kActorMcCoy, 3305, kAnimationModeTalk);
		Actor_Says(kActorGordo, 550, 16);
		Actor_Says(kActorMcCoy, 3310, kAnimationModeTalk);
		Actor_Says(kActorGordo, 560, 13);
		Actor_Says(kActorMcCoy, 3315, 15);
		Actor_Says(kActorGordo, 570, 14);
		Actor_Clue_Acquire(kActorMcCoy, kClueGordoConfession, 0, kActorGordo);
		break;
	case 810:
		Actor_Says(kActorMcCoy, 3255, kAnimationModeTalk);
		break;
	}
}

void AIScriptGordo::sub_41117C() {
	switch (_animationState) {
	case 0:
		break;
	case 1:
	case 3:
	case 38:
	case 39:
		_animationState = 0;
		_animationFrame = 0;
		_state = 0;
		_counterTarget = 0;
		break;
	case 2:
	case 13:
		Actor_Change_Animation_Mode(kActorGordo, 53);
		break;
	default:
		var_45B078 = 1;
		break;
	}
}

} // End of namespace BladeRunner
