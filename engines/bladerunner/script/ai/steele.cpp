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

AIScriptSteele::AIScriptSteele(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
	_var1 = 0;
	_var2 = 0;
}

void AIScriptSteele::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;
	_var1 = 0;
	_var2 = 0;

	Actor_Put_In_Set(kActorSteele, kSetFreeSlotG);
	Actor_Set_At_Waypoint(kActorSteele, 39, 0);
	Actor_Set_Goal_Number(kActorSteele, 0);
	Actor_Clue_Acquire(kActorSteele, kClueCrimeSceneNotes, true, -1);
}

bool AIScriptSteele::Update() {
	if ( Global_Variable_Query(kVariableMcCoyEvidenceMissed) > 5
	 && !Actor_Clue_Query(kActorSteele, kClueMcCoyIsStupid)
	) {
		Actor_Clue_Acquire(kActorSteele, kClueMcCoyIsStupid, true, -1);
		return true;
	}

	switch (Global_Variable_Query(kVariableChapter)) {
	case 1:
		if (Game_Flag_Query(kFlagMcCoyAtRCxx)
		 && Game_Flag_Query(183)
		) {
			Actor_Set_Goal_Number(kActorSteele, 3);
			return true;
		}

		if (Game_Flag_Query(kFlagMcCoyAtCTxx)
		 && Game_Flag_Query(184)
		) {
			Actor_Set_Goal_Number(kActorSteele, 6);
			return true;
		}

		if (Game_Flag_Query(kFlagMcCoyAtPSxx)
		 && Game_Flag_Query(185)
		) {
			Actor_Set_Goal_Number(kActorSteele, 10);
			return true;
		}

		if ( Actor_Query_Goal_Number(kActorSteele) == 0
		 && !Game_Flag_Query(kFlagMcCoyAtRCxx)
		 &&  Player_Query_Current_Scene() != kSceneRC01
		) {
			Actor_Set_Goal_Number(kActorSteele, 1);
			Game_Flag_Set(183);
			return true;
		}

		if (Actor_Query_Goal_Number(kActorSteele) == 11) {
			Actor_Set_Goal_Number(kActorSteele, 0);
			return true;
		}

		break;

	case 2:
		if ( Game_Flag_Query(kFlagTB05Entered)
		 && !Game_Flag_Query(kFlagTB02SteeleEnter)
		) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleGoToTB02);
			Game_Flag_Set(kFlagTB02SteeleEnter);
			return true;
		}

		if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleGoToTB02
		 && Game_Flag_Query(kFlagTB02SteeleTalk)) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleLeaveTB02);
			return true;
		 }

		if (!Game_Flag_Query(kFlagSteeleWalkingAround)
		 && Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleGoToPoliceStation
		) {
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleWalkAround);
			Game_Flag_Set(kFlagSteeleWalkingAround);
			return true;
		}

		if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleWalkAroundRestart) {
			Game_Flag_Reset(kFlagSteeleWalkingAround);
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleGoToPoliceStation);
			return true;
		}

		break;

	case 3:
		if (Global_Variable_Query(kVariableChapter) != 3) { // why is this here? double check?
			break;
		}

		if (Actor_Query_Goal_Number(kActorSteele) < 205) {
			Actor_Set_Goal_Number(kActorSteele, 205);
		}

		if (Actor_Query_Goal_Number(kActorSteele) == 251
		 && Player_Query_Current_Scene() == kSceneNR01
		 && Actor_Query_In_Between_Two_Actors(kActorMcCoy, kActorSteele, kActorGordo)
		) {
			Actor_Set_Goal_Number(kActorSteele, 255);
		}

		if (Actor_Query_Goal_Number(kActorSteele) == 213) {
			Actor_Set_Goal_Number(kActorSteele, 214);
			return true;
		}

		if (Actor_Query_Goal_Number(kActorSteele) == 261) {
			Actor_Set_Goal_Number(kActorSteele, 262);
			return true;
		}

		if (Actor_Query_Goal_Number(kActorSteele) != 271) {
			Actor_Set_Goal_Number(kActorSteele, 270);
		}

		break;

	case 4:
		if (Actor_Query_Goal_Number(kActorSteele) < 300) {
			Actor_Set_Goal_Number(kActorSteele, 300);
		}

		break;

	case 5:
		if (Game_Flag_Query(653)
		 && Actor_Query_Goal_Number(kActorSteele) < 400
		) {
			Actor_Set_Goal_Number(kActorSteele, 400);
			return true;
		}

		break;

	default:
		break;
	}

	if ( Actor_Query_Goal_Number(kActorSteele) == 599
		&& !Actor_Query_In_Set(kActorSteele, kSetFreeSlotI)
	) {
		if (Actor_Query_Which_Set_In(kActorMcCoy) != Actor_Query_Which_Set_In(kActorSteele)) {
			AI_Movement_Track_Flush(kActorSteele);
			AI_Movement_Track_Append(kActorSteele, 41, 100);
			AI_Movement_Track_Repeat(kActorSteele);
		}
	}
	return false;

}

void AIScriptSteele::TimerExpired(int timer) {
	if (timer == 0
	 && Actor_Query_Goal_Number(kActorSteele) == 210
	 && Player_Query_Current_Scene() == kSceneNR11
	) {
		if (Player_Query_Current_Scene() == kSceneNR11) {
			Actor_Set_Goal_Number(kActorSteele, 212);
		} else {
			Actor_Set_Goal_Number(kActorSteele, 211);
		}
		return; // true;
	}

	if (timer == 1) {
		int goal = Actor_Query_Goal_Number(kActorSteele);

		AI_Countdown_Timer_Reset(kActorSteele, 1);

		if (goal == 415) {
			Actor_Set_Goal_Number(kActorSteele, 416);
		} else if (goal == 416) {
			Actor_Set_Goal_Number(kActorSteele, 419);
		}
	}
	return; // false;
}

void AIScriptSteele::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorSteele)) {
	case 1:
		Actor_Set_Goal_Number(kActorSteele, 2);
		break;

	case 2:
		Actor_Set_Goal_Number(kActorSteele, 3);
		break;

	case 3:
		if (Random_Query(1, 3) == 1) {
			Actor_Clues_Transfer_New_To_Mainframe(kActorSteele);
			Actor_Clues_Transfer_New_From_Mainframe(kActorSteele);
		}

		if (Query_Score(kActorMcCoy) > Query_Score(kActorSteele)
		 && Query_Score(kActorMcCoy) < 75
		) {
			Set_Score(kActorSteele, Random_Query(2, 5) + Query_Score(kActorMcCoy));
		}

		Actor_Set_Goal_Number(kActorSteele, 4);
		break;

	case 4:
		Actor_Set_Goal_Number(kActorSteele, 5);
		break;

	case 5:
		Actor_Set_Goal_Number(kActorSteele, 6);
		break;

	case 6:
		if (Random_Query(1, 3) == 1) {
			Actor_Clues_Transfer_New_To_Mainframe(kActorSteele);
			Actor_Clues_Transfer_New_From_Mainframe(kActorSteele);
		}

		if (Query_Score(kActorMcCoy) > Query_Score(kActorSteele) && Query_Score(kActorMcCoy) < 75)
			Set_Score(kActorSteele, Random_Query(2, 5) + Query_Score(kActorMcCoy));

		Actor_Set_Goal_Number(kActorSteele, 7);
		break;

	case 7:
		Actor_Set_Goal_Number(kActorSteele, 8);
		break;

	case 8:
		Actor_Set_Goal_Number(kActorSteele, 9);
		break;

	case 9:
		Actor_Set_Goal_Number(kActorSteele, 10);
		break;

	case 10:
		Actor_Set_Goal_Number(kActorSteele, 11);
		break;

	case 100:
		if (Actor_Query_Goal_Number(kActorSteele) == 100) { //why is this here? double check?
			Player_Set_Combat_Mode(kActorMcCoy);

			if (Actor_Query_Goal_Number(kActorIzo) != 199) {
				if (Actor_Query_In_Between_Two_Actors(kActorMcCoy, kActorSteele, kActorIzo)) {
					Actor_Set_Goal_Number(kActorSteele, 125);
				} else if (Game_Flag_Query(kFlagIzoIsReplicant)) {
					Actor_Set_Goal_Number(kActorSteele, 120);
				} else {
					Actor_Set_Goal_Number(kActorSteele, 121);
				}
			}
		}
		break;

	case kGoalSteeleLeaveTB02:
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleGoToPoliceStation);
		break;

	case kGoalSteeleWalkAround:
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleWalkAroundRestart);
		break;

	case 241:
		Actor_Set_Goal_Number(kActorSteele, 242);
		break;

	case 423:
	case 424:
	case 425:
	case 426:
	case 427:
	case 428:
	case 429:
	case 430:
	case 431:
		Actor_Set_Goal_Number(kActorSteele, 423);
		break;

	case 432:
		Player_Set_Combat_Mode(kActorMcCoy);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 2265, 11);
		Actor_Says(kActorSteele, 640, 58);
		Actor_Says(kActorMcCoy, 2270, 12);
		Actor_Says(kActorSteele, 650, 59);
		Actor_Says(kActorMcCoy, 2275, 16);
		Actor_Says(kActorMcCoy, 2280, 15);
		Actor_Says(kActorSteele, 660, 60);
		Actor_Says(kActorSteele, 670, 59);
		Actor_Set_Goal_Number(kActorSteele, 433);
		Player_Set_Combat_Mode(kActorSteele);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -731.0f, 8.26f, -657.0f, 0, false, false, 0);
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Game_Flag_Set(578);
		Set_Enter(kSetKP07, kSceneKP07);
		break;

	default:
		break;
	}
	return; //true;
}

void AIScriptSteele::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptSteele::ClickedByPlayer() {
	int goal = Actor_Query_Goal_Number(kActorSteele);

	if (goal == 599) {
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 8630, 14);
		return; //true;
	}

	if (goal > 399 || Global_Variable_Query(kVariableChapter) > 2) {
		return; //true;
	}

	if (goal - 241 <= 9) {
		if (goal == 241) {
			Actor_Set_Goal_Number(kActorSteele, 242);
			return; //true;
		}

		if (goal == 250) {
			return; //true;
		}
	}

	AI_Movement_Track_Pause(1);
	Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
	Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);

	switch (Random_Query(1, 3)) {
	case 1:
		Actor_Says(kActorMcCoy, 3970, 15);
		Actor_Says(kActorSteele, 2700, 15);
		break;

	case 2:
		Actor_Says(kActorMcCoy, 3970, 15);
		Actor_Says(kActorSteele, 2060, 15);
		break;

	case 3:
		Actor_Says(kActorMcCoy, 3970, 15);
		Actor_Says(kActorSteele, 1750, 15);
		break;
	}
	AI_Movement_Track_Unpause(1);

	return; //false;
}

void AIScriptSteele::EnteredScene(int sceneId) {
	if (Actor_Query_Goal_Number(kActorSteele) == 2) {
		if (!Game_Flag_Query(kFlagChopstickWrapperTaken)
		 &&  Random_Query(1, 3) == 1
		) {
			Actor_Clue_Acquire(kActorSteele, kClueChopstickWrapper, true, -1);
			Game_Flag_Set(kFlagChopstickWrapperTaken);

			if (Game_Flag_Query(kFlagRC51Discovered)) {
				Item_Remove_From_World(kItemChopstickWrapper);
			}

			Global_Variable_Increment(kVariableMcCoyEvidenceMissed, 1);
			return;  //true;
		}

		if (!Game_Flag_Query(kFlagCandyTaken)
		 &&  Random_Query(1, 3) == 1
		) {
			Actor_Clue_Acquire(kActorSteele, kClueCandy, true, -1);
			Game_Flag_Set(kFlagCandyTaken);

			if (Game_Flag_Query(kFlagRC51Discovered)) {
				Item_Remove_From_World(kItemCandy);
			}
			return; //true;
		}

		if (!Game_Flag_Query(kFlagToyDogTaken)
		 &&  Random_Query(1, 20) == 1
		) {
			Actor_Clue_Acquire(kActorSteele, kClueToyDog, true, -1);
			Game_Flag_Set(kFlagToyDogTaken);

			if (Game_Flag_Query(kFlagRC51Discovered)) {
				Item_Remove_From_World(kItemToyDog);
			}
			return; //true;
		}
	}

	if ( Actor_Query_Goal_Number(kActorSteele) == 5
	 &&  Actor_Query_In_Set(kActorSteele, kSetCT03_CT04)
	 &&  Game_Flag_Query(kFlagMcCoyKilledHomeless)
	 && !Game_Flag_Query(kFlagHomelessBodyInDumpster)
	 && !Game_Flag_Query(kFlagHomelessBodyFound)
	) {
		Game_Flag_Set(kFlagHomelessBodyFound);
		return; //true;
	}

	return; //false;
}

void AIScriptSteele::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptSteele::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptSteele::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (otherActorId == kActorMcCoy
	 && Actor_Query_Goal_Number(kActorSteele) == 410
	)
		Actor_Set_Goal_Number(kActorSteele, 418);
}

void AIScriptSteele::ShotAtAndMissed() {
	// return false;
}

bool AIScriptSteele::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorSteele) == 251) {
		Actor_Set_Goal_Number(kActorSteele, 271);
	}

	if (/* !a1 && */ Actor_Query_In_Set(kActorSteele, kSetHF06))
		Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, kActorMcCoy, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);

	return false;
}

void AIScriptSteele::Retired(int byActorId) {
	Game_Flag_Set(646);

	if (Actor_Query_Goal_Number(kActorSteele) == 450) {
		Scene_Exits_Enable();
		Game_Flag_Set(484);
	}

	if (Actor_Query_Goal_Number(kActorSteele) != 599) {
		Actor_Change_Animation_Mode(kActorSteele, 49);
		Actor_Set_Goal_Number(kActorSteele, 599);
	}
}

int AIScriptSteele::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	if (otherActorId != kActorMcCoy) {
		return 0;
	}

	switch (clueId) {
	case kClueMcCoyRecoveredHoldensBadge:
	case kClueMcCoyRetiredLucy:
	case kClueMcCoyRetiredDektora:
	case kClueMcCoyRetiredSadik:
	case kClueMcCoyShotZubenInTheBack:
	case kClueMcCoyRetiredLutherLance:
	case kClueClovisOrdersMcCoysDeath:
	case kClueMcCoyIsKind:
	case kClueMcCoyIsInsane:
		return 5;

	case kClueMcCoyKilledRunciter1:
	case kClueMcCoyShotGuzza:
	case kClueMcCoyKilledRunciter2:
		return 0;

	case kClueMcCoyLetZubenEscape:
		return -4;

	case kClueMcCoyWarnedIzo:
	case kClueMcCoyHelpedIzoIzoIsAReplicant:
	case kClueMcCoyHelpedDektora:
	case kClueMcCoyHelpedLucy:
	case kClueMcCoyHelpedGordo:
		return -5;

	case kClueMcCoyRetiredZuben:
		return 4;

	case kClueMcCoyIsStupid:
		return -3;

	case kClueMcCoyIsAnnoying:
		return -2;
	}

	return 0;
}

double AIScriptSteele::comp_distance(int actorId, float a5, float a6, int a1, float a2, float a3, float a4) {
	float actorZ;
	float actorX;
	float actorY;

	Actor_Query_XYZ(actorId, &actorX, &actorY, &actorZ);
	return sqrt((a4 - actorZ) * (a4 - actorZ) + (a2 - actorX) * (a2 - actorX) + (a3 - actorY) * (a3 - actorY));
}

bool AIScriptSteele::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (currentGoalNumber == 599)
		return false;

	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 12, 5);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 2:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 61, 30);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 3:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 35, 45);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 4:
		AI_Movement_Track_Flush(kActorSteele);
		Game_Flag_Set(184);
		Game_Flag_Reset(183);
		AI_Movement_Track_Append(kActorSteele, 39, 45);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 5:
		AI_Movement_Track_Flush(kActorSteele);
		if (Random_Query(1, 10) == 1) {
			AI_Movement_Track_Append(kActorSteele, 63, 20);
		} else {
			AI_Movement_Track_Append(kActorSteele, 64, 10);
		}
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 6:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 35, 45);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 7:
		AI_Movement_Track_Flush(kActorSteele);
		Game_Flag_Set(185);
		Game_Flag_Reset(184);
		AI_Movement_Track_Append(kActorSteele, 39, 45);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 8:
		AI_Movement_Track_Flush(kActorSteele);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append(kActorSteele, 19, 10);
		} else {
			AI_Movement_Track_Append(kActorSteele, 35, 30);
		}
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 9:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 62, 1);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 10:
		AI_Movement_Track_Flush(kActorSteele);
		Game_Flag_Reset(185);
		AI_Movement_Track_Append(kActorSteele, 39, 30);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 11:
		Actor_Clue_Acquire(kActorSteele, kClueGrigorianInterviewA, true, kActorGrigorian);
		if (Game_Flag_Query(kFlagIzoIsReplicant)) {
			Actor_Clue_Acquire(kActorSteele, kClueGrigorianInterviewB1, true, kActorGrigorian);
		} else {
			Actor_Clue_Acquire(kActorSteele, kClueGrigorianInterviewB2, true, kActorGrigorian);
		}
		Actor_Set_Goal_Number(kActorSteele, 0);
		return true;

	case 100:
		AI_Movement_Track_Flush(kActorSteele);
		if (Game_Flag_Query(kFlagIzoIsReplicant)) {
			AI_Movement_Track_Append_Run(kActorSteele, 202, 0);
			AI_Movement_Track_Append_Run(kActorSteele, 203, 0);
		} else {
			AI_Movement_Track_Append(kActorSteele, 174, 0);
			AI_Movement_Track_Append_Run(kActorSteele, 175, 0);
		}
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case kGoalSteeleGoToTB02:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 192, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case kGoalSteeleLeaveTB02:
		Loop_Actor_Walk_To_XYZ(kActorSteele, -187.82f, 0.08f, 1460.74f, 0, 0, 0, 0);
		Actor_Face_Heading(kActorSteele, 802, false);
		Loop_Actor_Travel_Stairs(kActorSteele, 10, false, kAnimationModeIdle);
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 35, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case kGoalSteeleGoToPoliceStation:
		if (Random_Query(1, 2) == 1) {
			Actor_Clues_Transfer_New_To_Mainframe(kActorSteele);
			Actor_Clues_Transfer_New_From_Mainframe(kActorSteele);
		}
		if (Query_Score(kActorMcCoy) > Query_Score(kActorSteele)
		 && Query_Score(kActorMcCoy) < 75
		) {
			Set_Score(kActorSteele, Random_Query(2, 5) + Query_Score(kActorMcCoy));
		}
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 35, Random_Query(45, 120));
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case kGoalSteeleWalkAround:
		AI_Movement_Track_Flush(kActorSteele);
		if (Random_Query(1, 5) == 1) { // Animoid row & Bradbury building
			AI_Movement_Track_Append(kActorSteele, 222, 0);
			AI_Movement_Track_Append_With_Facing(kActorSteele, 223, 3, 973);
			AI_Movement_Track_Append(kActorSteele, 224, 5);
			AI_Movement_Track_Append(kActorSteele, 216, 5);
			AI_Movement_Track_Append(kActorSteele, 217, 0);
			AI_Movement_Track_Append(kActorSteele, 218, 0);
			AI_Movement_Track_Append(kActorSteele, 210, 0);
			AI_Movement_Track_Append(kActorSteele, 211, 0);
			AI_Movement_Track_Append(kActorSteele, 204, 0);
			AI_Movement_Track_Append(kActorSteele, 205, 0);
			AI_Movement_Track_Append(kActorSteele, 206, 0);
			AI_Movement_Track_Append(kActorSteele, 39, 20);
			AI_Movement_Track_Append(kActorSteele, 35, 60);
			AI_Movement_Track_Append(kActorSteele, 39, 20);
			AI_Movement_Track_Append(kActorSteele, 228, 0);
			AI_Movement_Track_Append(kActorSteele, 229, 10);
			AI_Movement_Track_Repeat(kActorSteele);
		} else if (Random_Query(1, 3) == 1) { // Chinatown & DNA row
			AI_Movement_Track_Append(kActorSteele, 232, 0);
			AI_Movement_Track_Append(kActorSteele, 233, 0);
			AI_Movement_Track_Append(kActorSteele, 234, 3);
			AI_Movement_Track_Append(kActorSteele, 237, 0);
			AI_Movement_Track_Append(kActorSteele, 238, 0);
			AI_Movement_Track_Append(kActorSteele, 239, 0);
			AI_Movement_Track_Append(kActorSteele, 240, 10);
			AI_Movement_Track_Append(kActorSteele, 39, 120);
			AI_Movement_Track_Append(kActorSteele, 243, 0);
			AI_Movement_Track_Append(kActorSteele, 244, 10);
			AI_Movement_Track_Repeat(kActorSteele);
		} else { // Police station
			AI_Movement_Track_Append(kActorSteele, 248, 0);
			AI_Movement_Track_Append(kActorSteele, 249, 0);
			AI_Movement_Track_Append(kActorSteele, 250, 10);
			AI_Movement_Track_Append(kActorSteele, 253, 0);
			AI_Movement_Track_Append_With_Facing(kActorSteele, 254, 3, 1015);
			AI_Movement_Track_Append_With_Facing(kActorSteele, 255, 2, 1015);
			AI_Movement_Track_Append_With_Facing(kActorSteele, 256, 3, 1015);
			AI_Movement_Track_Append(kActorSteele, 257, 0);
			AI_Movement_Track_Append(kActorSteele, 39, 30);
			AI_Movement_Track_Append(kActorSteele, 35, 120);
			AI_Movement_Track_Append(kActorSteele, 39, 30);
			AI_Movement_Track_Repeat(kActorSteele);
		}
		return true;

	case 120:
		Actor_Force_Stop_Walking(kActorMcCoy);
		Sound_Play(27, 100, 0, 0, 50);
		Actor_Set_Goal_Number(kActorIzo, 199);
		Actor_Change_Animation_Mode(kActorMcCoy, 0);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Loop_Actor_Walk_To_Actor(kActorSteele, 0, 48, 0, 1);
		Actor_Says(kActorSteele, 1860, 3);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 4825, 18);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 1870, 3);
		Actor_Says(kActorSteele, 1880, 3);
		Actor_Says(kActorMcCoy, 4830, 13);
		Actor_Says(kActorSteele, 1890, 3);
		Actor_Says(kActorSteele, 1900, 3);
		Actor_Face_Actor(kActorSteele, kActorIzo, 1);
		Actor_Says(kActorSteele, 1910, 3);
		Actor_Says(kActorSteele, 1920, 3);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 1930, 3);
		Actor_Set_Goal_Number(kActorSteele, 130);
		Player_Gains_Control();
		return true;

	case 121:
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Face_Actor(kActorSteele, kActorIzo, 1);
		Actor_Change_Animation_Mode(kActorIzo, 0);
		Actor_Face_Actor(kActorIzo, kActorSteele, 1);
		Actor_Says_With_Pause(kActorSteele, 2010, 0.0, 4);
		Actor_Change_Animation_Mode(kActorSteele, 4);
		Loop_Actor_Walk_To_Actor(kActorSteele, kActorIzo, 60, 0, 0);
		Actor_Change_Animation_Mode(kActorSteele, 4);
		_animationState = 23;
		_animationFrame = 0;
		Actor_Says(kActorIzo, 660, 3);
		Actor_Says_With_Pause(kActorSteele, 2020, 0.0f, 4);
		Actor_Says_With_Pause(kActorSteele, 2030, 0.0f, 4);
		Actor_Change_Animation_Mode(kActorSteele, 4);
		Actor_Says(kActorIzo, 670, 3);
		Actor_Says(kActorSteele, 2040, 4);
		Actor_Says(kActorIzo, 680, 3);
		Actor_Says(kActorSteele, 2050, 3);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 2060, 3);
		Actor_Says(kActorMcCoy, 4845, 3);
		Actor_Says(kActorSteele, 2070, 3);
		Actor_Says(kActorSteele, 2080, 3);
		Actor_Face_Actor(kActorSteele, kActorIzo, 1);
		Actor_Says(kActorSteele, 2090, 3);
		Actor_Says_With_Pause(kActorSteele, 2100, 1.0f, 3);
		Actor_Says(kActorIzo, 690, 3);
		Actor_Says(kActorSteele, 2110, 3);
		Actor_Says(kActorSteele, 2120, 3);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 2140, 3);
		Actor_Says(kActorMcCoy, 4850, 3);
		Actor_Says(kActorSteele, 2150, 3);
		Actor_Set_Goal_Number(kActorIzo, 120);
		Actor_Set_Goal_Number(kActorSteele, 130);
		Actor_Set_Goal_Number(kActorSteele, 0);
		return true;

	case 125:
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Set_Goal_Number(kActorIzo, 114);
		Actor_Says(kActorSteele, 1940, 3);
		Loop_Actor_Walk_To_Actor(kActorSteele, 0, 60, 0, 1);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorSteele, 1950, 3);
		Actor_Says(kActorMcCoy, 4835, 14);
		Actor_Says(kActorSteele, 1980, 3);
		Actor_Says(kActorMcCoy, 4840, 15);
		Actor_Says(kActorSteele, 1990, 3);
		Actor_Says(kActorSteele, 2000, 3);
		Actor_Set_Goal_Number(kActorSteele, 130);
		Scene_Exits_Enable();
		return true;

	case 126:
		return true;

	case 130:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 174, 0);
		AI_Movement_Track_Append(kActorSteele, 35, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 200:
		AI_Movement_Track_Flush(kActorSteele);
		return true;

	case 205:
		if (Query_Score(kActorMcCoy) > Query_Score(kActorSteele) && Query_Score(kActorMcCoy) < 75) {
			Set_Score(kActorSteele, Random_Query(2, 5) + Query_Score(kActorMcCoy));
		}
		if (Game_Flag_Query(kFlagRC04Locked)) {
			Game_Flag_Set(643);
		}
		AI_Movement_Track_Flush(kActorSteele);
		Actor_Put_In_Set(kActorSteele, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorSteele, 35, 0);
		return true;

	case 210:
		AI_Countdown_Timer_Reset(kActorSteele, 0);
		AI_Countdown_Timer_Start(kActorSteele, 0, 15);
		return true;

	case 211:
		AI_Countdown_Timer_Reset(kActorSteele, 0);
		return true;

	case 212:
		if (comp_distance(kActorMcCoy, -4.0, 0.33f, 0.0f, 100.0f, 0.33f, -4.0f) < 48.0f) {
			Loop_Actor_Walk_To_XYZ(0, 32.0f, 0.33f, 17.0f, 0, 0, 0, 0);
		}
		AI_Countdown_Timer_Reset(kActorSteele, 0);
		Player_Loses_Control();
		if (Actor_Query_Goal_Number(kActorDektora) == 250) {
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -15.53f, 0.33f, 73.49f, 0, 0);
		}
		Actor_Put_In_Set(kActorSteele, kSetNR11);
		Actor_Set_At_XYZ(kActorSteele, 100.0f, -51.56f, 0.0, 0);
		_animationState = 10;
		_animationFrame = 0;
		return true;

	case 213:
	case 216:
	case 271:
		return true;

	case 214:
		Player_Gains_Control();
		switch (Actor_Query_Goal_Number(kActorDektora)) {
		case 250:
			Actor_Face_Heading(kActorMcCoy, 954, 0);
			Actor_Change_Animation_Mode(kActorSteele, 4);
			Delay(2000);
			Actor_Says(kActorSteele, 1700, 58);
			Actor_Says(kActorMcCoy, 3800, 3);
			Actor_Says(kActorSteele, 1710, 59);
			Actor_Set_Goal_Number(kActorSteele, 215);
			break;
		case 260:
			Actor_Face_Actor(kActorSteele, kActorDektora, 1);
			Actor_Says(kActorSteele, 1790, 3);
			Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
			Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
			Actor_Says(kActorMcCoy, 3830, 17);
			Actor_Says(kActorSteele, 1800, 17);
			Actor_Says(kActorMcCoy, 3835, 18);
			Actor_Says(kActorSteele, 1810, 16);
			Game_Flag_Set(591);
			Actor_Set_Goal_Number(kActorDektora, 599);
			Actor_Set_Goal_Number(kActorSteele, 275);
			break;
		case 274:
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 6);
			Actor_Says(kActorSteele, 1750, 12);
			Actor_Says(kActorMcCoy, 3815, 19);
			Actor_Says(kActorSteele, 1760, 13);
			Actor_Says(kActorMcCoy, 3820, 18);
			Actor_Says(kActorSteele, 1770, 14);
			Actor_Says(kActorSteele, 1780, 15);
			Actor_Says(kActorMcCoy, 3825, 14);
			Game_Flag_Set(591);
			Actor_Set_Goal_Number(kActorDektora, 599);
			Actor_Set_Goal_Number(kActorSteele, 275);
		}
		return true;

	case 215:
		Game_Flag_Set(635);
		Actor_Change_Animation_Mode(kActorSteele, 6);
		Scene_Exits_Disable();
		return true;

	case 230:
		AI_Movement_Track_Flush(kActorSteele);
		Actor_Put_In_Set(kActorSteele, kSetNR01);
		Actor_Set_At_XYZ(kActorSteele, 94.03f, 23.88f, -794.46f, 685);
		return true;

	case 231:
		AI_Movement_Track_Flush(kActorSteele);
		Actor_Put_In_Set(kActorSteele, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorSteele, -1034.09f, 0.32f, 224.77f, 440);
		Game_Flag_Set(722);
		Set_Enter(kSetNR05_NR08, kSceneNR08);
		return true;

	case 232:
		Game_Flag_Set(532);
		Set_Enter(kSetNR02, kSceneNR02);
		return true;

	case 233:
		Game_Flag_Set(kFlagHF01toHF03);
		Game_Flag_Reset(kFlagMcCoyAtNRxx);
		Game_Flag_Set(kFlagMcCoyAtHFxx);
		Actor_Put_In_Set(kActorSteele, kSetHF03);
		Actor_Set_At_XYZ(kActorSteele, 291.0f, 47.76f, -892.0f, 453);
		Set_Enter(kSetHF03, kSceneHF03);
		return true;
	case 234:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append_Run(kActorSteele, 372, 0);
		AI_Movement_Track_Append(kActorSteele, 35, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 235:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append_Run(kActorSteele, 467, 0);
		AI_Movement_Track_Append(kActorSteele, 35, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 236:
		Actor_Put_In_Set(kActorSteele, kSetNR10);
		Actor_Set_At_XYZ(kActorSteele, -118.13f, 2.84f, -197.90f, 305);
		return true;

	case 240:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Flush(kActorLucy);
		Actor_Put_In_Set(kActorSteele, kSetHF02);
		Actor_Set_At_XYZ(kActorSteele, 254.94f, 47.76f, -262.58f, 210);
		Actor_Put_In_Set(kActorLucy, kSetHF02);
		Actor_Set_At_XYZ(kActorLucy, 596.49f, 47.76f, -260.04f, 731);
		return true;

	case 241:
		Actor_Change_Animation_Mode(kActorSteele, 6);
		Delay(500);
		Actor_Change_Animation_Mode(kActorLucy, 48);
		Actor_Set_Targetable(kActorLucy, 0);
		Actor_Retired_Here(kActorLucy, 36, 18, 1, -1);
		Delay(1000);
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append_Run(kActorSteele, 380, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		Delay(500);
		ADQ_Add(6, 340, 48);
		return true;

	case 242:
		AI_Movement_Track_Flush(kActorSteele);
		Player_Loses_Control();
		Delay(500);
		Actor_Says(kActorSteele, 0, 14);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 1540, 16);
		Actor_Says(kActorSteele, 10, 13);
		Actor_Says(kActorSteele, 20, 18);
		Actor_Says(kActorMcCoy, 1545, 13);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 30, 18);
		Actor_Says(kActorSteele, 40, 18);
		Actor_Says(kActorMcCoy, 1550, 13);
		Actor_Face_Actor(kActorSteele, kActorLucy, 1);
		Actor_Says(kActorSteele, 50, 12);
		Actor_Says(kActorMcCoy, 1555, 13);
		Actor_Says(kActorSteele, 60, 12);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 80, 16);
		Actor_Says(kActorMcCoy, 1560, 13);
		Delay(500);
		Game_Flag_Set(617);
		Player_Gains_Control();
		if (Game_Flag_Query(255)) {
			Actor_Put_In_Set(kActorSteele, kSetNR01);
			Actor_Set_At_XYZ(kActorSteele, 12.17f, 23.88f, -533.37f, 674);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetNR01, kSceneNR01);
		} else {
			Actor_Put_In_Set(kActorSteele, kSetHF01);
			Actor_Set_At_XYZ(kActorSteele, 324.32f, 8.0f, -465.87f, 674);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetHF01, kSceneHF04);
		}
		return true;

	case 243:
		Player_Loses_Control();
		Delay(500);
		Actor_Put_In_Set(kActorSteele, kSetHF02);
		Actor_Set_At_XYZ(kActorSteele, 254.94f, 47.76f, -262.58f, 210);
		return true;

	case 244:
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 1560, 14);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 100, 13);
		Actor_Says(kActorMcCoy, 1575, 13);
		Actor_Says(kActorSteele, 120, 15);
		Actor_Set_Goal_Number(kActorSteele, 246);
		return true;

	case 245:
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 150, 13);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 1580, 13);
		Actor_Says(kActorSteele, 160, 14);
		Actor_Says(kActorMcCoy, 1585, 16);
		Actor_Says(kActorSteele, 170, 16);
		Actor_Says(kActorSteele, 180, 17);
		Actor_Says(kActorMcCoy, 1590, 17);
		Actor_Says(kActorSteele, 200, 15);
		Actor_Set_Goal_Number(kActorSteele, 246);
		return true;

	case 246:
		Actor_Says(kActorSteele, 130, 13);
		Actor_Says(kActorSteele, 140, 13);
		Delay(500);
		Game_Flag_Set(617);
		Player_Gains_Control();
		if (Game_Flag_Query(255)) {
			Actor_Put_In_Set(kActorSteele, kSetNR01);
			Actor_Set_At_XYZ(kActorSteele, 12.17f, 23.88f, -533.37f, 674);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetNR01, kSceneNR01);
		} else {
			Actor_Put_In_Set(kActorSteele, kSetHF01);
			Actor_Set_At_XYZ(kActorSteele, 324.32f, 8.0f, -465.87f, 674);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetHF01, kSceneHF01);
		}
		return true;

	case 250:
		AI_Movement_Track_Flush(kActorSteele);
		Actor_Put_In_Set(kActorSteele, kSetNR01);
		Actor_Set_At_XYZ(kActorSteele, 94.03f, 23.88f, -794.46f, 685);
		if (Game_Flag_Query(255)) {
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
		}
		Game_Flag_Set(603);
		Actor_Change_Animation_Mode(kActorSteele, 0);
		_var1 = 3;
		return true;

	case 251:
		Actor_Set_Targetable(kActorSteele, 1);
		return true;

	case 252:
		Player_Loses_Control();
		Actor_Change_Animation_Mode(kActorSteele, 6);
		Sound_Play(27, 100, 0, 0, 50);
		Game_Flag_Reset(603);
		Actor_Set_Targetable(kActorSteele, 0);
		Actor_Set_Goal_Number(kActorGordo, 299);
		Delay(1500);
		Player_Set_Combat_Mode(0);
		Actor_Says(kActorSteele, 1070, 14);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 1260, 16);
		Actor_Says(kActorSteele, 1270, 13);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 3065, 18);
		Actor_Says(kActorSteele, 1280, 13);
		Actor_Says(kActorSteele, 1290, 13);
		Game_Flag_Set(592);
		Player_Gains_Control();
		if (Game_Flag_Query(256)) {
			Actor_Put_In_Set(kActorSteele, kSetHF01);
			Actor_Set_At_XYZ(kActorSteele, 324.32f, 8.0f, -465.87f, 674);
			Game_Flag_Set(617);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetHF01, kSceneHF01);
		} else {
			Actor_Set_Goal_Number(kActorSteele, 280);
		}
		return true;

	case 255:
		ADQ_Flush();
		Actor_Change_Animation_Mode(kActorSteele, 6);
		Sound_Play(27, 100, 0, 0, 50);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Change_Animation_Mode(kActorMcCoy, 48);
		Actor_Retired_Here(kActorMcCoy, 12, 12, 1, -1);
		Actor_Set_Goal_Number(kActorGordo, 251);
		Actor_Says(kActorSteele, 1250, 3);
		return true;

	case 258:
		Game_Flag_Reset(603);
		ADQ_Flush();
		Game_Flag_Set(592);
		Actor_Set_Targetable(kActorSteele, 0);
		Delay(1000);
		Player_Set_Combat_Mode(0);
		Player_Loses_Control();
		_animationState = 40;
		_animationFrame = 0;
		Delay(2000);
		Actor_Change_Animation_Mode(kActorSteele, 0);
		Actor_Change_Animation_Mode(kActorMcCoy, 0);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 1080, 17);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 3025, 14);
		Actor_Says(kActorSteele, 1090, 16);
		Actor_Face_Actor(kActorSteele, kActorGordo, 1);
		Actor_Says(kActorSteele, 1100, 13);
		Actor_Says(kActorSteele, 1110, 17);
		Actor_Says(kActorMcCoy, 3030, 19);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 1130, 15);
		Actor_Says(kActorSteele, 1140, 12);
		Player_Gains_Control();
		Actor_Set_Goal_Number(kActorSteele, 280);
		return true;

	case 260:
		ADQ_Flush();
		Actor_Change_Animation_Mode(kActorMcCoy, 6);
		Sound_Play(27, 100, 0, 0, 50);
		_animationState = 38;
		_animationFrame = 0;
		Actor_Set_Goal_Number(kActorGordo, 251);
		Game_Flag_Reset(603);
		Game_Flag_Set(592);
		Actor_Set_Targetable(kActorSteele, 0);
		Player_Loses_Control();
		return true;

	case 262:
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 1160, 15);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 3035, 14);
		Actor_Says(kActorSteele, 1170, 16);
		Actor_Says(kActorMcCoy, 3040, 15);
		Actor_Says(kActorSteele, 1180, 16);
		Actor_Says(kActorMcCoy, 3045, 15);
		Actor_Says(kActorMcCoy, 3055, 15);
		Actor_Says(kActorSteele, 1190, 16);
		Actor_Says(kActorMcCoy, 3060, 15);
		Actor_Says(kActorSteele, 1200, 16);
		Actor_Says(kActorSteele, 1210, 14);
		Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 2);
		Player_Set_Combat_Mode(0);
		Music_Stop(2);
		Player_Gains_Control();
		Actor_Set_Goal_Number(kActorSteele, 280);
		return true;

	case 270:
		ADQ_Flush();
		Player_Loses_Control();
		Game_Flag_Reset(603);
		Actor_Set_Targetable(kActorSteele, 0);
		Actor_Set_Goal_Number(kActorGordo, 251);
		Actor_Start_Speech_Sample(kActorSteele, 1150);
		Actor_Change_Animation_Mode(kActorSteele, 49);
		Delay(5000);
		Actor_Set_Goal_Number(kActorMcCoy, 500);
		return true;

	case 275:
		Delay(500);
		Game_Flag_Set(617);
		Player_Gains_Control();
		if (Game_Flag_Query(255)) {
			Actor_Put_In_Set(kActorSteele, kSetNR01);
			Actor_Set_At_XYZ(kActorSteele, 12.17f, 23.88f, -533.37f, 674);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetNR01, kSceneNR01);
		} else {
			Actor_Put_In_Set(kActorSteele, kSetHF01);
			Actor_Set_At_XYZ(kActorSteele, 324.32f, 8.0f, -465.87f, 674);
			Game_Flag_Reset(255);
			Game_Flag_Reset(256);
			Set_Enter(kSetHF01, kSceneHF01);
		}
		return true;

	case 280:
		Actor_Says(kActorMcCoy, 2970, 15);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 840, 16);
		Actor_Says(kActorMcCoy, 2975, 15);
		Actor_Says(kActorSteele, 850, 16);
		Actor_Says(kActorDispatcher, 0, 3);
		Actor_Says(kActorDispatcher, 10, 3);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorMcCoy, 2980, 12);
		Actor_Says(kActorSteele, 860, 14);
		Actor_Says(kActorMcCoy, 2985, 13);
		Actor_Says(kActorSteele, 870, 15);
		Actor_Says(kActorMcCoy, 2990, 16);
		Actor_Says(kActorSteele, 880, 17);
		Actor_Says(kActorSteele, 890, 18);
		Actor_Says(kActorMcCoy, 2995, 12);
		Actor_Says(kActorSteele, 900, 13);
		Actor_Says(kActorMcCoy, 3000, 13);
		Actor_Says(kActorSteele, 920, 12);
		Actor_Says(kActorDispatcher, 30, 3);
		Actor_Says(kActorMcCoy, 3005, 14);
		Actor_Says(kActorSteele, 930, 16);
		Actor_Says(kActorSteele, 950, 13);
		Actor_Says(kActorMcCoy, 3010, 15);
		Actor_Says(kActorSteele, 960, 15);
		Actor_Says(kActorSteele, 970, 13);
		Actor_Says(kActorMcCoy, 3015, 12);
		Actor_Says(kActorSteele, 980, 17);
		if (Player_Query_Current_Scene() == 54) {
			Actor_Set_Goal_Number(kActorSteele, 290);
		} else {
			Actor_Set_Goal_Number(kActorSteele, 291);
		}
		return true;

	case 285:
		Actor_Set_Goal_Number(kActorSteele, 290);
		return true;

	case 290:
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -281.0f, 31.93f, -1061.0f, 0, 0, 1, 0);
		Actor_Face_Heading(kActorMcCoy, 45, 0);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 3, 0, kAnimationModeIdle);
		Global_Variable_Set(1, 4);
		Scene_Exits_Enable();
		Actor_Set_Health(kActorMcCoy, 50, 50);
		Game_Flag_Set(523);
		Game_Flag_Set(343);
		Game_Flag_Set(680);
		Chapter_Enter(4, 79, 91);
		return true;

	case 291:
		Game_Flag_Set(722);
		Global_Variable_Set(1, 4);
		Scene_Exits_Enable();
		Game_Flag_Set(523);
		Game_Flag_Set(680);
		Chapter_Enter(4, 79, 91);
		return true;

	case 300:
		if (Query_Score(kActorMcCoy) > Query_Score(kActorSteele) && Query_Score(kActorMcCoy) < 75) {
			Set_Score(kActorSteele, Random_Query(2, 5) + Query_Score(kActorMcCoy));
		}
		if (Query_Score(kActorMcCoy) > Query_Score(kActorSteele) && Query_Score(kActorMcCoy) > 74) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 3);
		}
		Actor_Set_Goal_Number(kActorSteele, 305);
		return true;

	case 305:
		Actor_Put_In_Set(kActorSteele, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorSteele, 35, 0);
		return true;

	case 400:
	case 401:
	case 423:
		return true;

	case 410:
		Actor_Set_Targetable(kActorSteele, 1);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Start_Speech_Sample(kActorSteele, 2510);
		Actor_Change_Animation_Mode(kActorSteele, 4);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Delay(3000);
		Async_Actor_Walk_To_XYZ(kActorSteele, -109.0f, -36.55f, 26.0f, 0, 0);
		return true;

	case 411:
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		Actor_Says(kActorSteele, 430, 14);
		Actor_Change_Animation_Mode(kActorSteele, 4);
		Delay(4000);
		Async_Actor_Walk_To_XYZ(kActorSteele, -109.0f, -36.55f, 26.0f, 0, 0);
		return true;

	case 412:
		Actor_Force_Stop_Walking(kActorSteele);
		return true;

	case 413:
		Async_Actor_Walk_To_XYZ(kActorSteele, 1.0, -36.55f, 111.0f, 0, 0);
		return true;

	case 415:
		Actor_Force_Stop_Walking(kActorSteele);
		Actor_Change_Animation_Mode(kActorSteele, 51);
		AI_Countdown_Timer_Start(kActorSteele, 1, 2);
		return true;

	case 416:
		Actor_Set_At_XYZ(kActorSteele, -48.83f, -36.55f, 69.98f, 280);
		Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorSteele, 36, 0, 1);
		Actor_Face_Actor(kActorMcCoy, kActorSteele, 1);
		if (Game_Flag_Query(653) == 1) {
			Actor_Says(kActorSteele, 400, 3);
			Actor_Says(kActorMcCoy, 2165, 14);
			Actor_Says(kActorSteele, 410, 3);
			Actor_Says(kActorMcCoy, 2170, 14);
			Actor_Says(kActorSteele, 420, 3);
		} else {
			Actor_Says(kActorSteele, 440, 3);
			Actor_Says(kActorMcCoy, 2175, 14);
			Actor_Says(kActorSteele, 450, 3);
			Actor_Says(kActorSteele, 460, 3);
			Actor_Says(kActorSteele, 470, 3);
		}
		AI_Countdown_Timer_Start(kActorSteele, 1, 3);
		return true;

	case 418:
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Change_Animation_Mode(kActorSteele, 6);
		Actor_Change_Animation_Mode(kActorMcCoy, 48);
		Actor_Retired_Here(kActorMcCoy, 72, 18, 1, -1);
		return true;

	case 419:
		Actor_Set_Targetable(kActorSteele, 0);
		Game_Flag_Set(646);
		Actor_Set_Goal_Number(kActorSteele, 599);
		Actor_Retired_Here(kActorSteele, 60, 12, 1, -1);
		return true;

	case 420:
		Actor_Put_In_Set(kActorSteele, kSetKP01);
		Actor_Set_At_XYZ(kActorSteele, 20.0f, -12.2f, -97.0f, 907);
		Actor_Change_Animation_Mode(kActorSteele, 43);
		return true;

	case 421:
		Game_Flag_Reset(482);
		Delay(2000);
		Actor_Face_Actor(kActorSteele, kActorMcCoy, 1);
		Actor_Says(kActorSteele, 360, -1);
		Loop_Actor_Walk_To_Actor(kActorMcCoy, kActorSteele, 48, 0, 0);
		Actor_Says(kActorMcCoy, 2150, 14);
		Actor_Says(kActorSteele, 370, 15);
		Actor_Says(kActorMcCoy, 2155, 12);
		Actor_Says(kActorSteele, 380, 16);
		Actor_Says(kActorMcCoy, 2160, 18);
		Actor_Says(kActorSteele, 390, 13);
		Actor_Set_Goal_Number(kActorSteele, 422);
		return true;

	case 422:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append(kActorSteele, 530, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 430:
		Actor_Put_In_Set(kActorSteele, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorSteele, -1110.0f, 0.0f, 952.0f, 155);
		Actor_Change_Animation_Mode(kActorSteele, 4);
		return true;

	case 431:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append_Run(kActorSteele, 541, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 432:
		Actor_Set_At_Waypoint(kActorSteele, 542, 0);
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append_Run(kActorSteele, 543, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 433:
		AI_Movement_Track_Flush(kActorSteele);
		AI_Movement_Track_Append_Run(kActorSteele, 542, 0);
		AI_Movement_Track_Repeat(kActorSteele);
		return true;

	case 450:
		Actor_Put_In_Set(kActorSteele, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorSteele, -947.39f, 0.0f, 728.89f, 0);
		return true;

	case 499:
		AI_Movement_Track_Flush(kActorSteele);
		Actor_Put_In_Set(kActorSteele, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorSteele, -782.15f, 8.26f, -263.64f, 52);
		return true;

	default:
		return false;
	}
}

bool AIScriptSteele::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_var1 <= 3) {
			if (_var1) {
				if (_var1 == 1) {
					Game_Flag_Set(482);
					_animationState = 41;
					_var1 = 1;
					_var2 = 0;
				} else if (_var1 == 3) {
					*animation = 88;
					_animationFrame++;
					if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(88) - 1)
						_animationFrame = 0;
				}
			} else {
				*animation = 74;
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(74)) {
					_animationFrame = 0;
					if (Game_Flag_Query(482) == 1) {
						_animationState = 41;
						_var1 = 1;
						_var2 = 0;
					}
				}
			}
		}
		break;
	case 1:
		_animationFrame = 0;
		*animation = _animationNext;
		_animationState = _animationStateNext;
		break;
	case 2:
		*animation = 67;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(67))
			_animationFrame = 0;
		break;
	case 3:
		*animation = 68;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(68))
			_animationFrame = 0;
		break;
	case 4:
		*animation = 59;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(59)) {
			_animationFrame = 0;
		}
		break;
	case 5:
		*animation = 60;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(60)) {
			_animationFrame = 0;
		}
		break;
	case 6:
		*animation = 69;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(69)) {
			_animationFrame = 0;
		}
		break;
	case 7:
		*animation = 70;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(70)) {
			_animationFrame = 0;
		}
		break;
	case 8:
		*animation = 62;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(62)) {
			_animationFrame = 0;
		}
		break;
	case 9:
		*animation = 63;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(63)) {
			_animationFrame = 0;
		}
		break;
	case 10:
		*animation = 83;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(83)) {
			Actor_Change_Animation_Mode(kActorSteele, 0);
			*animation = 74;
			_animationFrame = 0;
			_animationState = 0;
			if (Actor_Query_Goal_Number(kActorSteele) == 212) {
				Actor_Set_Goal_Number(kActorSteele, 213);
			}
		}
		break;
	case 11:
		*animation = 84;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(84)) {
			_animationFrame = 0;
		}
		break;
	case 12:
		*animation = 85;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(85)) {
			_animationFrame = 0;
		}
		break;
	case 13:
		*animation = 86;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(86)) {
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	case 14:
		*animation = 77;
		if (!_animationFrame && _flag) {
			*animation = 74;
			_animationFrame = 0;
			_animationState = 0;
			_flag = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(77) - 1) {
				_animationFrame = 0;
			}
		}
		break;
	case 15:
		*animation = 78;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(78) - 1) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 77;
		}
		break;
	case 16:
		*animation = 79;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(79) - 1) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 77;
		}
		break;
	case 17:
		*animation = 80;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(80) - 1) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 77;
		}
		break;
	case 18:
		*animation = 81;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(81) - 1) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 77;
		}
		break;
	case 19:
		*animation = 81;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(81) - 1) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 77;
		}
		break;
	case 20:
		*animation = 82;
		if (!_animationFrame && _flag) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 66;
			Actor_Change_Animation_Mode(kActorSteele, 4);
			_flag = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(82) - 1) {
				_animationFrame = 0;
			}
		}
		break;
	case 21:
		*animation = 82;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(82) - 1) {
			_animationFrame = 0;
			_animationState = 20;
			*animation = 82;
		}
		break;
	case 22:
		*animation = 82;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(82) - 1) {
			_animationFrame = 0;
			_animationState = 20;
			*animation = 82;
		}
		break;
	case 23:
		*animation = 54;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(54) - 1) {
			_animationFrame = 0;
		}
		break;
	case 24:
		*animation = 65;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(65) - 1) {
			*animation = 74;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	case 25:
		*animation = 64;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(64) - 1) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 54;
		}
		break;
	case 26:
		*animation = 66;
		_animationFrame++;
		if (_animationFrame == 3) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(1, snd, 75, 0, 99);
		}

		if (_animationFrame == 4)
			Actor_Combat_AI_Hit_Attempt(1);

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(66) - 1) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 54;
			Actor_Change_Animation_Mode(kActorSteele, 4);
			if (Actor_Query_Goal_Number(kActorSteele) == 215) {
				Actor_Change_Animation_Mode(kActorSteele, 6);
				_animationState = 26;
				_animationFrame = 0;
				*animation = 66;
			}
		}
		break;
	case 27:
		*animation = 55;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(55) - 1) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 54;
			Actor_Change_Animation_Mode(kActorSteele, 4);
		}
		break;
	case 28:
		*animation = 56;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(56) - 1) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 54;
			Actor_Change_Animation_Mode(kActorSteele, 4);
		}
		break;
	case 29:
		*animation = 57;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(57) - 1) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 54;
			Actor_Change_Animation_Mode(kActorSteele, 4);
		}
		break;
	case 30:
		*animation = 58;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(58) - 1) {
			_animationFrame = 0;
			_animationState = 23;
			*animation = 54;
			Actor_Change_Animation_Mode(kActorSteele, 4);
		}
		break;
	case 31:
		*animation = 71;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(71) - 1) {
			*animation = 74;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSteele, 0);
		}
		break;
	case 32:
		*animation = 72;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(72) - 1) {
			*animation = 74;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorSteele, 0);
		}
		break;
	case 33:
		*animation = 61;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(61) - 1)
			_animationFrame++;
		break;
	case 34:
		*animation = 73;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(73) - 1)
			_animationFrame++;
		break;
	case 35:
		*animation = 61;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(61) - 1)
			_animationFrame++;
		break;
	case 36:
		*animation = 88;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(88) - 1) {
			_animationFrame = 0;
		}
		break;
	case 37:
		if (!_animationFrame && _flag) {
			_animationFrame = 0;
			_animationState = 0;
			_var1 = 3;
			_flag = 0;
			*animation = 89;
		} else {
			*animation = 89;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(89) - 1) {
				_animationFrame = 0;
			}
		}
		break;
	case 38:
		*animation = 90;
		_animationFrame++;
		if (_animationFrame > 5) {
			Actor_Change_Animation_Mode(kActorSteele, 0);
			_animationState = 0;
			_animationFrame = 0;
			*animation = 74;
			Actor_Set_Goal_Number(kActorSteele, 261);
		}
		break;
	case 39:
		*animation = 92;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(92) - 1) {
			_animationFrame = 0;
			_animationState = 36;
			*animation = 88;
		}
		break;
	case 40:
		*animation = 91;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(91) - 1) {
			*animation = 74;
			_animationFrame = 0;
			_animationState = 0;
			_var1 = 0;
		}
		break;
	case 41:
		if (_var1 > 3) {
			break;
		}
		if (_var1) {
			if (_var1 == 1) {
				*animation = 75;
				if (_var2) {
					_var2--;
				} else {
					if (++_animationFrame >= 6) {
						if (_animationFrame <= 9) {
							_var2 = Random_Query(1, 3);
							break;
						}
					}
					if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(75)) {
						_animationFrame = 0;
						if (Game_Flag_Query(482) == 1) {
							_var1 = 0;
						} else {
							_var1 = 3;
						}
					}
				}
			} else if (_var1 == 3) {
				*animation = 76;
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(76) - 1) {
					_animationState = 0;
					_animationFrame = 0;
					_var1 = 0;
					Game_Flag_Reset(482);
				}
			}
		} else {
			*animation = 74;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(74)) {
				_animationFrame = 0;
				if (Game_Flag_Query(482) == 1) {
					if (Random_Query(1, 3) == 1) {
						_var1 = 1;
					}
				} else {
					_var1 = 3;
				}
			}
		}
		break;
	default:
		*animation = 406;
		_animationFrame = 0;
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptSteele::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (Game_Flag_Query(603)) {
			_var1 = 3;
		} else {
			_var1 = 0;
		}
		switch (_animationState) {
		case 0:
		case 24:
		case 34:
		case 35:
		case 41:
			return true;
		case 4:
			_animationState = 23;
			_animationFrame = 0;
			break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
			_flag = 1;
			break;
		case 20:
		case 21:
		case 22:
		case 37:
			_flag = 1;
			_animationState = 23;
			_animationFrame = 0;
			break;
		case 23:
		case 26:
			_animationState = 24;
			_animationFrame = 0;
			break;
		default:
			_animationFrame = 0;
			if (Game_Flag_Query(482) == 1) {
				_animationState = 41;
				_var1 = 1;
			} else {
				_animationState = 0;
			}
			break;
		}
		break;
	case 1:
		if ((_animationState != 4 && _animationState != 23 && _animationState != 25 && _animationState != 26)
				|| _animationState > 26) {
			if (Actor_Query_Goal_Number(kActorSteele) != 100 && Actor_Query_Goal_Number(kActorSteele) != 121) {
				_animationState = 2;
				_animationFrame = 0;
			} else {
				_animationState = 4;
				_animationFrame = 0;
			}
		} else {
			_animationState = 4;
			_animationFrame = 0;
		}
		break;
	case 2:
		if (Actor_Query_Goal_Number(kActorSteele) != 100
				&& Actor_Query_Goal_Number(kActorSteele) != 121
				&& _animationState != 23) {
			_animationState = 3;
			_animationFrame = 0;
		} else {
			_animationState = 5;
			_animationFrame = 0;
		}
		break;
	case 3:
		if (_animationState != 35) {
			if (Game_Flag_Query(603)) {
				_animationState = 37;
				_animationFrame = 0;
				_flag = 0;
			} else {
				if (_animationState) {
					_animationState = 14;
					_animationFrame = 0;
				} else {
					_animationState = 1;
					_animationFrame = 0;
					_animationStateNext = 14;
					_animationNext = 77;
				}
				_flag = 0;
			}
		}
		break;
	case 4:
		if (_animationState) {
			if (_animationState > 25 || (_animationState != 23 && _animationState != 25)) {
				_animationState = 23;
				_animationFrame = 0;
			}
		} else {
			_animationState = 25;
			_animationFrame = 0;
		}
		break;
	case 6:
		if (Game_Flag_Query(603)) {
			_animationState = 39;
		} else {
			_animationState = 26;
		}
		_animationFrame = 0;
		break;
	case 7:
		_animationState = 4;
		_animationFrame = 0;
		break;
	case 8:
		_animationState = 5;
		_animationFrame = 0;
		break;
	case 12:
		if (_animationState) {
			_animationState = 15;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationFrame = 0;
			_animationStateNext = 15;
			_animationNext = 78;
		}
		_flag = 0;
		break;
	case 13:
		if (_animationState) {
			_animationState = 16;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationFrame = 0;
			_animationStateNext = 16;
			_animationNext = 79;
		}
		_flag = 0;
		break;
	case 14:
		if (_animationState) {
			_animationState = 17;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationFrame = 0;
			_animationStateNext = 17;
			_animationNext = 80;
		}
		_flag = 0;
		break;
	case 15:
		if (_animationState) {
			_animationState = 18;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationFrame = 0;
			_animationStateNext = 18;
			_animationNext = 81;
		}
		_flag = 0;
		break;
	case 16:
		if (_animationState) {
			_animationState = 19;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationFrame = 0;
			_animationStateNext = 19;
			_animationNext = 81;
		}
		_flag = 0;
		break;
	case 21:
		if (_animationState > 28) {
			if (Random_Query(0, 1)) {
				_animationState = 31;
			} else {
				_animationState = 32;
			}
		} else if (Random_Query(0, 1)) {
			_animationState = 29;
		} else {
			_animationState = 30;
		}
		_animationFrame = 0;
		break;
	case 22:
		if (Random_Query(0, 1)) {
			_animationState = 29;
		} else {
			_animationState = 30;
		}
		_animationFrame = 0;
		break;
	case 43:
		Game_Flag_Set(482);
		_animationState = 41;
		_animationFrame = 0;
		_var2 = 0;
		_var1 = 1;
		break;
	case 44:
		_animationState = 6;
		_animationFrame = 0;
		break;
	case 45:
		_animationState = 7;
		_animationFrame = 0;
		break;
	case 46:
		_animationState = 8;
		_animationFrame = 0;
		break;
	case 47:
		_animationState = 9;
		_animationFrame = 0;
		break;
	case 48:
		if (_animationState != 33 && _animationState != 34) {
			if (_animationState > 26 || (_animationState != 23 && _animationState != 25 && _animationState != 26)) {
				_animationState = 34;
				_animationFrame = 0;
			} else {
				_animationState = 33;
				_animationFrame = 0;
			}
		}
		break;
	case 49:
		if (_animationState != 33) {
			_animationState = 33;
			_animationFrame = 0;
		}
		break;
	case 51:
		_animationState = 35;
		_animationFrame = 0;
		break;
	case 58:
		_animationState = 20;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 59:
		_animationState = 21;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 60:
		_animationState = 22;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 86:
		_animationState = 10;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptSteele::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptSteele::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptSteele::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 174
	 && Actor_Query_Goal_Number(kActorSteele) == 130
	)
		Actor_Set_Goal_Number(kActorSteele, kGoalSteeleGoToPoliceStation);

	return true;
}

void AIScriptSteele::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
