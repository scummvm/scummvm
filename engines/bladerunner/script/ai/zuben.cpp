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

AIScriptZuben::AIScriptZuben(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var_45D258 = 0;
	_var_45D25C = 0;
	_animationFrameTarget2 = 0;
	_animationFrameDelta = 0;
	_animationFrameTarget1 = 0;
}

void AIScriptZuben::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_var_45D258 = 0;
	_var_45D25C = 30;
	_animationFrameTarget2 = 0;
	_animationFrameDelta = 1;
	_animationFrameTarget1 = 2;
	Actor_Put_In_Set(kActorZuben, kSetCT01_CT12);
	Actor_Set_At_XYZ(kActorZuben, -165.01f, -6.5f, 413.12f, 103);
	Actor_Set_Goal_Number(kActorZuben, 0);
}

bool AIScriptZuben::Update() {
	if (Actor_Query_Goal_Number(kActorZuben) == 599 && Actor_Query_Which_Set_In(kActorZuben) != kSetFreeSlotI) {
		if (Actor_Query_Which_Set_In(kActorZuben) != Player_Query_Current_Set() ) {
			Actor_Put_In_Set(kActorZuben, kSetFreeSlotI);
			Actor_Set_At_Waypoint(kActorZuben, 41, 0);
		}
	}
	if (Actor_Query_Goal_Number(kActorZuben) == 0 && Player_Query_Current_Scene() == kSceneCT01 && !Game_Flag_Query(129) ) {
		AI_Countdown_Timer_Reset(kActorZuben, 2);
		AI_Countdown_Timer_Start(kActorZuben, 2, 30);
		Game_Flag_Set(129);
		return true;
	}
	if (Actor_Query_Goal_Number(kActorZuben) == 0 && Actor_Query_Friendliness_To_Other(kActorZuben, kActorMcCoy) < 48 && Actor_Query_Is_In_Current_Set(kActorZuben)) {
		Actor_Face_Actor(kActorZuben, kActorHowieLee, true);
		Actor_Says(kActorZuben, 0, 14);
		if (Random_Query(1, 3) == 1) {
			Actor_Clue_Acquire(kActorZuben, kClueMcCoysDescription, true, -1);
		}
		Game_Flag_Set(29);
		Actor_Set_Goal_Number(kActorZuben, 1);
		return true;
	}
	if (Global_Variable_Query(kVariableChapter) >= 4 && !Game_Flag_Query(kFlagZubenRetired) && Actor_Query_Goal_Number(kActorZuben) < 200) {
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Set_Goal_Number(kActorZuben, 200);
		return true;
	}
	if (Global_Variable_Query(kVariableChapter) == 5 && !Game_Flag_Query(kFlagZubenRetired) && Actor_Query_Goal_Number(kActorZuben) < 400) {
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Set_Goal_Number(kActorZuben, 400);
		return true;
	}
	if (Actor_Query_Goal_Number(kActorZuben) == 12 && Player_Query_Current_Scene() == kSceneCT07) {
		AI_Countdown_Timer_Reset(kActorZuben, 0);
		Game_Flag_Set(kFlagWarehouseOpen);
		Actor_Set_Targetable(kActorZuben, true);
		if (Actor_Query_Goal_Number(kActorGordo) == 0) {
			Actor_Set_Goal_Number(kActorGordo, 1);
		}
		return true;
	}
	if (Actor_Query_Goal_Number(kActorZuben) == 14 && !Game_Flag_Query(210)) {
		Game_Flag_Set(210);
		return true;
	}
	return false;
}

void AIScriptZuben::TimerExpired(int timer) {
	if (timer == 2) {
		if (Actor_Query_Goal_Number(kActorZuben) == 0 && Player_Query_Current_Scene() == kSceneCT01 && Random_Query(1, 3) < 3) {
			Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, -1);
		}
		Game_Flag_Reset(129);
		// return true;
	} else if (timer == 1) {
		if (Actor_Query_Goal_Number(kActorZuben) == 2) {
			Music_Stop(10);
			Actor_Set_Goal_Number(kActorZuben, 13);
			AI_Countdown_Timer_Reset(kActorZuben, 1);
			// return true;
		}
	} else if (timer == 0) {
		if (Player_Query_Current_Set() != kSetCT01_CT12) {
			Music_Stop(2);
		}
		Actor_Set_Goal_Number(kActorZuben, 20);
		AI_Countdown_Timer_Reset(kActorZuben, 0);
		// return true;
	}
	// return false;
}

void AIScriptZuben::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorZuben) == 2 && Game_Flag_Query(719)) {
		Set_Enter(kSetCT03_CT04, kSceneCT03);
	}

	if (Actor_Query_Goal_Number(kActorZuben) == 3) {
		Actor_Set_Goal_Number(kActorZuben, 5);
		//return true;
	} else if (Actor_Query_Goal_Number(kActorZuben) == 13 && Game_Flag_Query(719)) {
		AI_Countdown_Timer_Reset(kActorZuben, 0);
		Game_Flag_Reset(719);
		Game_Flag_Set(720);
		Game_Flag_Set(721);
		Music_Stop(2);
		Actor_Set_Goal_Number(kActorZuben, 20);
		Set_Enter(kSetCT02, kSceneCT02);
		//return true;
	} else if (Actor_Query_Goal_Number(kActorZuben) == 9) {
		Actor_Set_Goal_Number(kActorZuben, 10);
		//return true;
	} else {
		if (Actor_Query_Goal_Number(kActorZuben) == 5) {
			Music_Stop(2);
			Sound_Play(574, 40, 100, 100, 50);
			Delay(2000);
			Game_Flag_Set(144);
			Game_Flag_Set(kFlagZubenSpared);
			Game_Flag_Set(31);
			Actor_Set_Goal_Number(kActorZuben, 7);
			Actor_Set_Goal_Number(kActorGaff, 1);
			Set_Enter(kSetCT06, kSceneCT06);
		}
		if (Actor_Query_Goal_Number(kActorZuben) == 21) {
			Non_Player_Actor_Combat_Mode_On(kActorZuben, kActorCombatStateIdle, false, kActorMcCoy, 6, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 15, 300, false);
		}
		int goal = Actor_Query_Goal_Number(kActorZuben);
		if (goal == 200) {
			Actor_Set_Goal_Number(kActorZuben, 205);
		} else if (goal == 205) {
			Actor_Set_Goal_Number(kActorZuben, 206);
		} else if (goal == 100) {
			Actor_Set_Goal_Number(kActorZuben, 105);
		} else if (goal == 105) {
			Actor_Set_Goal_Number(kActorZuben, 106);
		}
		// return false;
	}
}

void AIScriptZuben::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptZuben::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorZuben) == 599) {
		if (Player_Query_Current_Scene() == kSceneCT06) {
			// return true;
			return;
		}
		Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
		Actor_Says(kActorMcCoy, 8529, 13);
	}
	if (Global_Variable_Query(kVariableChapter) > 1 && Global_Variable_Query(kVariableChapter) < 5) {
		if (Actor_Query_Friendliness_To_Other(kActorZuben, kActorMcCoy) <= 20) {
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 8910, 11);
		} else if (Game_Flag_Query(699)) {
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 8910, 11);
		} else {
			AI_Movement_Track_Pause(kActorZuben);
			Actor_Face_Actor(kActorZuben, kActorMcCoy, true);
			Actor_Says(kActorZuben, 140, 14);
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 7280, 11);
			Actor_Says(kActorZuben, 150, 15);
			Actor_Says(kActorMcCoy, 7285, 12);
			dialogue();
			Game_Flag_Set(699);
			AI_Movement_Track_Unpause(kActorZuben);
		}
		// return true;
	}
	// return false;
}

void AIScriptZuben::EnteredScene(int sceneId) {
	//return false;
}

void AIScriptZuben::OtherAgentEnteredThisScene(int otherActorId) {
	//return false;
}

void AIScriptZuben::OtherAgentExitedThisScene(int otherActorId) {
	//return false;
}

void AIScriptZuben::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == kActorMcCoy && !combatMode && Game_Flag_Query(516) && !Game_Flag_Query(712)) {
		Non_Player_Actor_Combat_Mode_Off(kActorZuben);
		Game_Flag_Reset(516);
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Says(kActorMcCoy, 455, 18);
		Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, 5);
		Actor_Set_Goal_Number(kActorZuben, 4);
		// return true;
	}
	// return false;
}

void AIScriptZuben::ShotAtAndMissed() {
	//return false;
}

bool AIScriptZuben::ShotAtAndHit() {
	Game_Flag_Set(712);
	if (Actor_Query_Goal_Number(kActorZuben) == 5 && !Actor_Clue_Query(kActorZuben, kClueMcCoyShotZubenInTheBack)) {
		Actor_Clue_Acquire(kActorZuben, kClueMcCoyShotZubenInTheBack, true, -1);
		Actor_Clue_Lose(kActorZuben, kClueMcCoyLetZubenEscape);
		Actor_Start_Speech_Sample(kActorMcCoy, 490);
	}
	if (Player_Query_Current_Scene() == kSceneCT07) {
		Music_Stop(2);
	}
	return false;
}

void AIScriptZuben::Retired(int byActorId) {
	if (!Actor_Query_In_Set(kActorZuben, kSetKP07)) {
		// return false;
		return;
	}
	Global_Variable_Decrement(51, 1);
	Actor_Set_Goal_Number(kActorZuben, 599);
	if (Global_Variable_Query(51)) {
		// return false;
		return;
	}
	Player_Loses_Control();
	Delay(2000);
	Player_Set_Combat_Mode(false);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, 0);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Game_Flag_Set(579);
	Game_Flag_Reset(653);
	Set_Enter(kSetKP05_KP06, kSceneKP06);
	// return true;
}

int AIScriptZuben::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptZuben::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append(kActorZuben, 86, 0);
		AI_Movement_Track_Append(kActorZuben, 106, 0);
		AI_Movement_Track_Append_With_Facing(kActorZuben, 45, 0, 481);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 2:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 84, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 85, 0);
		AI_Movement_Track_Append(kActorZuben, 39, 0);
		AI_Countdown_Timer_Reset(kActorZuben, 1);
		AI_Countdown_Timer_Start(kActorZuben, 1, 10);
		AI_Countdown_Timer_Reset(kActorZuben, 0);
		AI_Countdown_Timer_Start(kActorZuben, 0, 70);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 4:
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Face_Actor(kActorZuben, kActorMcCoy, true);
		Music_Stop(3);
		Actor_Says(kActorZuben, 100, 19);
		Actor_Says(kActorMcCoy, 470, 12);
		Actor_Says(kActorZuben, 110, 18);
		Actor_Says(kActorMcCoy, 475, 12);
		Actor_Says(kActorZuben, 120, 15);
		Actor_Says(kActorMcCoy, 480, 16);
		Actor_Says(kActorZuben, 130, 17);
		Actor_Says(kActorMcCoy, 485, 14);
		if (Random_Query(1, 3) < 3) {
			Actor_Clue_Acquire(kActorZuben, kClueMcCoysDescription, 1, -1);
		}
		if (Random_Query(1, 5) < 5) {
			Actor_Clue_Acquire(kActorZuben, kClueMcCoyIsABladeRunner, 1, -1);
		}
		Actor_Clue_Acquire(kActorZuben, kClueMcCoyLetZubenEscape, 1, -1);
		Actor_Set_Goal_Number(kActorZuben, 5);
		return false;
	case 5:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 94, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 33, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 6:
		Game_Flag_Set(kFlagZubenRetired);
		Game_Flag_Set(31);
		if (Actor_Query_In_Set(kActorZuben, kSetCT07)) {
			Game_Flag_Set(144);
			Actor_Set_Goal_Number(kActorGaff, 1);
			Set_Enter(kSetCT06, kSceneCT06);
		} else if (Actor_Query_In_Set(kActorZuben, kSetMA01)) {
			Player_Set_Combat_Mode(false);
			Actor_Set_Goal_Number(kActorGaff, 3);
		}
		Actor_Set_Goal_Number(kActorZuben, 599);
		return false;
	case 8:
		_animationFrame = 0;
		_animationState = 26;
		return false;
	case 9:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 46, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 10:
		Actor_Face_Heading(kActorZuben, kActorMcCoy, false);
		_animationFrame = 0;
		_animationState = 27;
		return false;
	case 11:
		AI_Countdown_Timer_Reset(kActorZuben, 0);
		Actor_Put_In_Set(kActorZuben, kSetCT06);
		Actor_Set_At_XYZ(kActorZuben, 37.14f, -58.23f, 4.0f, 256);
		_animationFrame = 0;
		_animationState = 28;
		return false;
	case 12:
		Player_Loses_Control();
		if (!Player_Query_Combat_Mode()) {
			Player_Set_Combat_Mode(true);
		}
		Player_Gains_Control();
		Game_Flag_Set(142);
		Set_Enter(kSetCT07, kSceneCT07);
		return false;
	case 13:
		Actor_Set_Targetable(kActorZuben, true);
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 51, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 48, 0);
		AI_Movement_Track_Append(kActorZuben, 33, 1);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 14:
		if (Actor_Query_Goal_Number(kActorMcCoy) != 1) {
			Player_Loses_Control();
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			Game_Flag_Set(210);
			Game_Flag_Set(719);
		}
		Actor_Set_Goal_Number(kActorZuben, 9);
		return false;
	case 21:
		Game_Flag_Set(kFlagWarehouseOpen);
		Actor_Set_Targetable(kActorZuben, true);
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 125, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 22:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 125, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 33, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 99:
		AI_Movement_Track_Flush(kActorZuben);
		Scene_Exits_Enable();
		Music_Stop(2);
		return false;
	case 100:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 33, 3);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 105:
		{
			int rnd1 = Random_Query(1, 3);

			if (rnd1 == 1) {
				int rnd2 = Random_Query(1, 3);
				if (rnd2 == 1) {
					AI_Movement_Track_Flush(kActorZuben);
					AI_Movement_Track_Append(kActorZuben, 472, 0);
					AI_Movement_Track_Append_With_Facing(kActorZuben, 470, 2, 506);
					AI_Movement_Track_Append(kActorZuben, 471, 5);
					AI_Movement_Track_Append(kActorZuben, 470, 0);
					AI_Movement_Track_Append(kActorZuben, 469, 0);
					AI_Movement_Track_Append(kActorZuben, 468, 2);
				} else if (rnd2 == 2) {
					AI_Movement_Track_Flush(kActorZuben);
					AI_Movement_Track_Append(kActorZuben, 471, 0);
					AI_Movement_Track_Append(kActorZuben, 472, 5);
					AI_Movement_Track_Append_With_Facing(kActorZuben, 470, 3, 506);
					AI_Movement_Track_Append(kActorZuben, 471, 5);
					AI_Movement_Track_Append(kActorZuben, 472, 0);
				} else if (rnd2 == 3){
					AI_Movement_Track_Flush(kActorZuben);
					AI_Movement_Track_Append(kActorZuben, 468, 0);
					AI_Movement_Track_Append(kActorZuben, 469, 3);
					AI_Movement_Track_Append(kActorZuben, 470, 0);
					AI_Movement_Track_Append(kActorZuben, 471, 0);
				}
				AI_Movement_Track_Append(kActorZuben, 33, 15);
				AI_Movement_Track_Repeat(kActorZuben);
			}

			if (rnd1 == 1 || rnd1 == 2) {
				int rnd2 = Random_Query(1, 3);
				if (rnd2 == 1) {
					AI_Movement_Track_Append(kActorZuben, 473, 0);
					AI_Movement_Track_Append(kActorZuben, 474, 5);
					AI_Movement_Track_Append(kActorZuben, 473, 5);
					AI_Movement_Track_Append(kActorZuben, 475, 0);
				} else if (rnd2 == 2) {
					AI_Movement_Track_Append(kActorZuben, 473, 0);
					AI_Movement_Track_Append(kActorZuben, 476, 0);
				} else if (rnd2 == 3) {
					AI_Movement_Track_Append(kActorZuben, 475, 0);
					AI_Movement_Track_Append(kActorZuben, 473, 5);
					AI_Movement_Track_Append(kActorZuben, 474, 5);
				}
				AI_Movement_Track_Append(kActorZuben, 33, 15);
				AI_Movement_Track_Repeat(kActorZuben);
			}

			if (rnd1 == 1 || rnd1 == 2 || rnd1 == 3) {
				int rnd2 = Random_Query(1, 3);
				if (rnd2 == 1) {
					AI_Movement_Track_Append(kActorZuben, 477, 3);
					AI_Movement_Track_Append(kActorZuben, 478, 0);
				} else if (rnd2 == 2) {
					AI_Movement_Track_Append(kActorZuben, 479, 0);
					AI_Movement_Track_Append(kActorZuben, 488, 0);
					AI_Movement_Track_Append(kActorZuben, 489, 0);
					AI_Movement_Track_Append(kActorZuben, 480, 0);
				} else if (rnd2 == 3) {
					AI_Movement_Track_Append(kActorZuben, 481, 0);
					AI_Movement_Track_Append(kActorZuben, 482, 5);
					AI_Movement_Track_Append(kActorZuben, 483, 5);
				}
				AI_Movement_Track_Append(kActorZuben, 33, 15);
				AI_Movement_Track_Repeat(kActorZuben);
			}
			return false;
		}
	case 106:
		Actor_Set_Goal_Number(kActorZuben, 105);
		return true;
	case 200:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 33, 3);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;
	case 205:
		switch (Random_Query(1, 8)) {
		case 1:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 490, 0);
			AI_Movement_Track_Append(kActorZuben, 491, 8);
			AI_Movement_Track_Append(kActorZuben, 492, 2);
			AI_Movement_Track_Append(kActorZuben, 493, 5);
			AI_Movement_Track_Append(kActorZuben, 494, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, Random_Query(15, 45));
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 2:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 495, 0);
			AI_Movement_Track_Append(kActorZuben, 496, 0);
			AI_Movement_Track_Append(kActorZuben, 33, Random_Query(15, 45));
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 3:
			AI_Movement_Track_Append(kActorZuben, 498, 0);
			AI_Movement_Track_Append(kActorZuben, 497, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 4:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 499, 2);
			AI_Movement_Track_Append(kActorZuben, 500, 3);
			AI_Movement_Track_Append(kActorZuben, 499, 2);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 5:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 503, 0);
			AI_Movement_Track_Append(kActorZuben, 504, 0);
			AI_Movement_Track_Append(kActorZuben, 505, 5);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 6:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 508, 0);
			AI_Movement_Track_Append(kActorZuben, 509, 2);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 7:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 514, 0);
			AI_Movement_Track_Append(kActorZuben, 515, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		case 8:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 510, 0);
			AI_Movement_Track_Append(kActorZuben, 511, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 40);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;
		}
		return false;
	case 206:
		Actor_Set_Goal_Number(kActorZuben, 205);
		return true;
	case 400:
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Put_In_Set(kActorZuben, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorZuben, 33, 0);
		return false;
	}
	return false;
}

bool AIScriptZuben::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (Actor_Query_Goal_Number(kActorZuben) != 0) {
			*animation = 408;
			if (_var_45D258 < _var_45D25C) {
				_animationFrame += _animationFrameDelta;
				if (_animationFrame > _animationFrameTarget1) {
					_animationFrame = _animationFrameTarget1;
					_animationFrameDelta = -1;
				} else if (_animationFrame < _animationFrameTarget2) {
					_animationFrame = _animationFrameTarget2;
					_animationFrameDelta = 1;
				}
				_var_45D258++;
			} else {
				_animationFrame += _animationFrameDelta;
				_var_45D25C = 0;
				if (_animationFrame == 13 && Random_Query(0, 1)) {
					_animationFrameDelta = -1;
					_var_45D258 = 0;
					_animationFrameTarget2 = 8;
					_animationFrameTarget1 = 13;
					_var_45D25C = Random_Query(0, 30);
				}
				if (_animationFrame == 23) {
					if (Random_Query(0, 1)) {
						_animationFrameDelta = -1;
						_var_45D258 = 0;
						_animationFrameTarget2 = 20;
						_animationFrameTarget1 = 23;
						_var_45D25C = Random_Query(0, 30);
					}
				}
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(408)) {
					_animationFrame = 0;
					if (Random_Query(0, 1)) {
						_var_45D258 = 0;
						_animationFrameTarget2 = 0;
						_animationFrameTarget1 = 2;
						_var_45D25C = Random_Query(0, 45);
					}
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(408) - 1;
				}
			}
		} else {
			*animation = 418;
			_animationFrame++;
			if (_animationFrame >= 24) {
				_animationFrame = 5;
			}
			*frame = _animationFrame;
		}
		break;
	case 1:
		*animation = 399;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(399)) {
			_animationFrame = 0;
		}
		break;
	case 2:
		*animation = 391;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(391)) {
			_animationFrame = 0;
		}
		break;
	case 3:
		*animation = 400;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(400)) {
			_animationFrame = 0;
		}
		break;
	case 4:
		*animation = 392;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(392)) {
			_animationFrame = 0;
		}
		break;
	case 5:
		*animation = 396;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(396)) {
			_animationFrame = 0;
			if (Actor_Query_Goal_Number(kActorZuben) == 14) {
				_animationState = 8;
			} else {
				_animationState = 7;
			}
		}
		break;
	case 6:
		*animation = 397;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(397)) {
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	case 7:
		*animation = 388;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(388)) {
			_animationFrame = 0;
		}
		break;
	case 8:
		*animation = 398;
		_animationFrame++;
		if (_animationFrame == 8) {
			int sentenceId;
			if (Random_Query(1, 2) == 1) {
				sentenceId = 9010;
			} else {
				sentenceId = 9015;
			}
			Sound_Play_Speech_Line(kActorZuben, sentenceId, 75, 0, 99);
		}
		if (_animationFrame == 11) {
			Actor_Combat_AI_Hit_Attempt(kActorZuben);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(398)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 14) {
				Actor_Set_Goal_Number(kActorZuben, 9);
				_animationFrame = 0;
				_animationState = 0;
			} else {
				_animationFrame = 0;
				_animationState = 7;
				*animation = 388;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeCombatIdle);
			}
		}
		break;
	case 9:
		*animation = 403;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(403)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 13;
				*animation = 405;
				Actor_Change_Animation_Mode(kActorZuben, 48);
			} else {
				_animationFrame = 0;
				_animationState = 0;
				*animation = 406;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeIdle);
			}
		}
		break;
	case 10:
		*animation = 404;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(404)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 13;
				*animation = 405;
				Actor_Change_Animation_Mode(kActorZuben, 48);
			} else {
				_animationFrame = 0;
				_animationState = 0;
				*animation = 406;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeIdle);
			}
		}
		break;
	case 11:
		*animation = 389;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(389)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 14;
				*animation = 393;
				Actor_Change_Animation_Mode(kActorZuben, 48);
			} else {
				_animationFrame = 0;
				_animationState = 7;
				*animation = 388;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeCombatIdle);
			}
		}
		break;
	case 12:
		*animation = 390;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(390)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 14;
				*animation = 393;
				Actor_Change_Animation_Mode(kActorZuben, 48);
			} else {
				_animationFrame = 0;
				_animationState = 7;
				*animation = 388;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeCombatIdle);
			}
		}
		break;
	case 13:
		*animation = 405;
		_animationFrame++;
		if (_animationFrame == 7) {
			Sound_Play(207, 30, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Set_Goal_Number(kActorZuben, 6);
			_animationState = 15;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Set_Targetable(kActorZuben, false);
		}
		break;
	case 14:
		*animation = 393;
		_animationFrame++;
		if (_animationFrame == 7) {
			Sound_Play(207, 30, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Set_Goal_Number(kActorZuben, 6);
			_animationState = 16;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Set_Targetable(kActorZuben, 0);
		}
		break;
	case 15:
		*animation = 405;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(405) - 1;
		Scene_Exits_Enable();
		break;
	case 16:
		*animation = 393;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(393) - 1;
		break;
	case 17:
		*animation = 409;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(409)) {
			_animationFrame = 0;
		}
		break;
	case 18:
		*animation = 410;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(410)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 19:
		*animation = 411;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(411)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 20:
		*animation = 412;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(412)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 21:
		*animation = 413;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(413)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 22:
		*animation = 414;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(414)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 23:
		*animation = 415;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(415)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 24:
		*animation = 416;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(416)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 25:
		*animation = 417;
		 _animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(417)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 409;
		}
		break;
	case 26:
		Actor_Set_Frame_Rate_FPS(kActorZuben, -1);
		*animation = 419;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(419)) {
			Actor_Set_Frame_Rate_FPS(kActorZuben, -2);
			_animationState = 0;
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorZuben, 14);
		}
		break;
	case 27:
		Actor_Set_Frame_Rate_FPS(kActorZuben, -1);
		*animation = 420;
		_animationFrame++;
		if (_animationFrame == 5) {
			Overlay_Play("ct02over", 1, false, true, 0);
		}
		if (_animationFrame == 6) {
			Sound_Play(201, 40, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(420)) {
			Actor_Set_Frame_Rate_FPS(kActorZuben, -2);
			_animationState = 0;
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorZuben, 2);
		}
		break;
	case 28:
		*animation = 421;
		_animationFrame++;
		if (_animationFrame == 1) {
			Sound_Play(206, 80, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(421)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = 396;
			Actor_Set_Goal_Number(kActorZuben, 12);
		}
		break;
	default:
		*animation = 399;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptZuben::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = 0;
		_animationFrame = 0;
		break;
	case kAnimationModeWalk:
		_animationState = 1;
		_animationFrame = 0;
		break;
	case kAnimationModeRun:
		_animationState = 3;
		_animationFrame = 0;
		break;
	case kAnimationModeTalk:
		_animationState = 17;
		_animationFrame = 0;
		break;
	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 2:
		case 4:
			_animationState = 7;
			_animationFrame = 0;
			break;
		case 5:
		case 7:
			break;
		default:
			_animationState = 5;
			_animationFrame = 0;
			break;
		}
		break;
	case kAnimationModeCombatAttack:
		_animationState = 8;
		_animationFrame = 0;
		break;
	case kAnimationModeCombatWalk:
		_animationState = 2;
		_animationFrame = 0;
		break;
	case kAnimationModeCombatRun:
		_animationState = 4;
		_animationFrame = 0;
		break;
	case 12:
		_animationState = 18;
		_animationFrame = 0;
		break;
	case 13:
		_animationState = 19;
		_animationFrame = 0;
		break;
	case 14:
		_animationState = 20;
		_animationFrame = 0;
		break;
	case 15:
		_animationState = 21;
		_animationFrame = 0;
		break;
	case 16:
		_animationState = 22;
		_animationFrame = 0;
		break;
	case 17:
		_animationState = 23;
		_animationFrame = 0;
		break;
	case 18:
		_animationState = 24;
		_animationFrame = 0;
		break;
	case 19:
		_animationState = 25;
		_animationFrame = 0;
		break;
	case 21:
		switch (_animationState) {
		case 2:
		case 4:
		case 5:
		case 7:
		case 8:
			if (Random_Query(0, 1)) {
				_animationState = 11;
			} else {
				_animationState = 12;
			}
			_animationFrame = 0;
			break;
		case 3:
		case 6:
			if (Random_Query(0, 1)) {
				_animationState = 9;
			} else {
				_animationState = 10;
			}
			_animationFrame = 0;
			break;
		}
		break;
	case 22:
		if (Random_Query(0, 1)) {
			_animationState = 11;
		} else {
			_animationState = 12;
		}
		_animationFrame = 0;
		break;
	case 24:
		_animationState = 26;
		_animationFrame = 0;
		break;
	case 25:
		_animationState = 27;
		_animationFrame = 0;
		break;
	case 26:
		_animationState = 28;
		_animationFrame = 0;
		break;
	case 48:
		Actor_Set_Targetable(kActorZuben, false);
		switch (_animationState) {
			case 2:
			case 4:
			case 5:
			case 7:
			case 8:
			case 11:
			case 12:
				_animationState = 14;
				_animationFrame = 0;
				break;
			case 3:
			case 6:
			case 9:
			case 10:
				_animationState = 13;
				_animationFrame = 0;
				break;
		}
		break;
	case 49:
		Actor_Set_Targetable(kActorZuben, false);
		_animationState = 14;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptZuben::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptZuben::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptZuben::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptZuben::FledCombat() {
	// return false;
}

void AIScriptZuben::dialogue() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(1490, 5, 5, -1);
	if (Actor_Query_Goal_Number(kActorLucy) != 599) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1500, 5, 5, 5);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(1510, -1, 5, 5);
	Dialogue_Menu_Add_DONE_To_List(1520);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 1490:
		Actor_Says(kActorMcCoy, 7290, 12);
		Actor_Says(kActorZuben, 180, 15);
		Actor_Says(kActorMcCoy, 7310, 16);
		Actor_Says(kActorZuben, 190, 15);
		break;
	case 1500:
		Actor_Says(kActorMcCoy, 7295, 11);
		Actor_Says(kActorZuben, 200, 12);
		Actor_Says(kActorMcCoy, 7315, 16);
		Actor_Says(kActorZuben, 210, 12);
		Delay(1000);
		Actor_Says(kActorZuben, 220, 14);
		Actor_Says(kActorMcCoy, 7320, 17);
		if (Game_Flag_Query(kFlagLucyIsReplicant)) {
			Actor_Says(kActorZuben, 230, 14);
			Actor_Says(kActorMcCoy, 7325, 17);
			Actor_Says(kActorZuben, 240, 15);
			Actor_Says(kActorMcCoy, 7330, 16);
			Actor_Clue_Acquire(kActorMcCoy, kClueZubenTalksAboutLucy1, false, kActorZuben);
		} else {
			Actor_Says(kActorZuben, 250, 14);
			Actor_Says(kActorMcCoy, 7335, 14);
			Actor_Says(kActorZuben, 260, 15);
			Actor_Says(kActorMcCoy, 7340, 16);
			Actor_Says(kActorZuben, 340, 15);
			Actor_Says(kActorMcCoy, 7345, 12);
			Actor_Clue_Acquire(kActorMcCoy, kClueZubenTalksAboutLucy2, false, kActorZuben);
		}
		break;
	case 1510:
		Actor_Says(kActorMcCoy, 7300, 13);
		Actor_Says(kActorZuben, 280, 12);
		Actor_Says(kActorMcCoy, 7355, 14);
		Actor_Says(kActorZuben, 290, 15);
		Actor_Says(kActorMcCoy, 7360, 14);
		Actor_Says(kActorZuben, 300, 14);
		Actor_Says(kActorZuben, 310, 13);
		Delay(2000);
		Actor_Says(kActorMcCoy, 7360, 11);
		Actor_Says(kActorZuben, 320, 12);
		Actor_Says(kActorZuben, 330, 12);
		Actor_Clue_Acquire(kActorMcCoy, kClueZubensMotive, false, kActorZuben);
		break;
	case 1520:
		Actor_Says(kActorZuben, 160, 13);
		Actor_Says(kActorMcCoy, 7305, 15);
		Actor_Says(kActorZuben, 170, 14);
		break;
	}
}

} // End of namespace BladeRunner
