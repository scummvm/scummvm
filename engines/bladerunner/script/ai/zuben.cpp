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
	_animationLoopCounter = 0;
	_animationLoopLength = 0;
	_animationLoopFrameMin = 0;
	_animationLoopDirection = 0;
	_animationLoopFrameMax = 0;
}

void AIScriptZuben::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationLoopCounter = 0;
	_animationLoopLength = 30;
	_animationLoopFrameMin = 0;
	_animationLoopDirection = 1;
	_animationLoopFrameMax = 2;
	Actor_Put_In_Set(kActorZuben, kSetCT01_CT12);
	Actor_Set_At_XYZ(kActorZuben, -165.01f, -6.5f, 413.12f, 103);
	Actor_Set_Goal_Number(kActorZuben, kGoalZubenDefault);
}

bool AIScriptZuben::Update() {
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenGone
	 && Actor_Query_Which_Set_In(kActorZuben) != kSetFreeSlotI
	 && Actor_Query_Which_Set_In(kActorZuben) != Player_Query_Current_Set()
	) {
		Actor_Put_In_Set(kActorZuben, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorZuben, 41, 0);
	}

	if ( Actor_Query_Goal_Number(kActorZuben) == kGoalZubenDefault
	 &&  Player_Query_Current_Scene() == kSceneCT01
	 && !Game_Flag_Query(kFlagCT01ZubenMcCoyCheck)
	) {
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask2);
		AI_Countdown_Timer_Start(kActorZuben, kActorTimerAIScriptCustomTask2, 30);
		Game_Flag_Set(kFlagCT01ZubenMcCoyCheck);
		return true;
	}

	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenDefault
	 && Actor_Query_Friendliness_To_Other(kActorZuben, kActorMcCoy) < 48
	 && Actor_Query_Is_In_Current_Set(kActorZuben)
	) {
		Actor_Face_Actor(kActorZuben, kActorHowieLee, true);
		Actor_Says(kActorZuben, 0, 14);
		if (Random_Query(1, 3) == 1) {
			Actor_Clue_Acquire(kActorZuben, kClueMcCoysDescription, true, -1);
		}
		Game_Flag_Set(kFlagCT01ZubenLeft);
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT01WalkToCT02);
		return true;
	}

	if ( Global_Variable_Query(kVariableChapter) >= 4
	 && !Game_Flag_Query(kFlagZubenRetired)
	 &&  Actor_Query_Goal_Number(kActorZuben) < 200
	) {
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Set_Goal_Number(kActorZuben, 200);
		return true;
	}

	if ( Global_Variable_Query(kVariableChapter) == 5
	 && !Game_Flag_Query(kFlagZubenRetired)
	 &&  Actor_Query_Goal_Number(kActorZuben) < 400
	) {
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Set_Goal_Number(kActorZuben, 400);
		return true;
	}

	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT06AttackMcCoy
	 && Player_Query_Current_Scene() == kSceneCT07
	) {
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask0);
		Game_Flag_Set(kFlagCT05WarehouseOpen);
		Actor_Set_Targetable(kActorZuben, true);
		if (Actor_Query_Goal_Number(kActorGordo) == kGoalGordoDefault) {
			Actor_Set_Goal_Number(kActorGordo, kGoalGordoCT01Leave);
		}
		return true;
	}

	if ( Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02PotDodgeCheck
	 && !Game_Flag_Query(kFlagCT02ZubenFled)
	) {
		Game_Flag_Set(kFlagCT02ZubenFled);
		return true;
	}
	return false;
}

void AIScriptZuben::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) { // Check on McCoy every 30s
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenDefault
		 && Player_Query_Current_Scene() == kSceneCT01
		 && Random_Query(1, 3) < 3
		) {
			Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, -1);
		}
		Game_Flag_Reset(kFlagCT01ZubenMcCoyCheck);
		// return true;
	} else if (timer == kActorTimerAIScriptCustomTask1) {
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02RunToFreeSlotG) { // Zuben fleeing, after 10s
			Music_Stop(10);
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT06HideAtFreeSlotA);
			AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask1);
			// return true;
		}
	} else if (timer == kActorTimerAIScriptCustomTask0) { // Zuben fleeing, after 70s
		if (Player_Query_Current_Set() != kSetCT01_CT12) {
			Music_Stop(2);
		}
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenFled); // Let Zuben flee completly, he will catch McCoy on MA01
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask0);
		// return true;
	}
	// return false;
}

void AIScriptZuben::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02RunToFreeSlotG
	 && Game_Flag_Query(kFlagCT02McCoyFell)
	) {
		Set_Enter(kSetCT03_CT04, kSceneCT03);
	}

	if (Actor_Query_Goal_Number(kActorZuben) == 3) { // not set anywhere
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT07RunToFreeSlotA);
		//return true;
	} else if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT06HideAtFreeSlotA
	        && Game_Flag_Query(kFlagCT02McCoyFell)
	) {
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask0);
		Game_Flag_Reset(kFlagCT02McCoyFell);
		Game_Flag_Set(kFlagCT02McCoyCombatReady);
		Game_Flag_Set(kFlagCT02McCoyShouldCommentOnDumpedSoup);
		Music_Stop(2);
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenFled);
		Set_Enter(kSetCT02, kSceneCT02);
		//return true;
	} else if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02RunToDoor) {
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT02OpenDoor);
		//return true;
	} else {
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT07RunToFreeSlotA) {
			Music_Stop(2);
			Sound_Play(kSfxDOORLOCK, 40, 100, 100, 50);
			Delay(2000);
			Game_Flag_Set(kFlagCT07toCT06);
			Game_Flag_Set(kFlagZubenSpared);
			Game_Flag_Set(kFlagCT01ZubenGone);
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenSpared);
			Actor_Set_Goal_Number(kActorGaff, kGoalGaffCT12WaitForMcCoy);
			Set_Enter(kSetCT06, kSceneCT06);
		}
		if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenMA01AttackMcCoy) {
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
	if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenGone) {
		if (Player_Query_Current_Scene() == kSceneCT06) {
			// return true;
			return;
		}
		Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
		Actor_Says(kActorMcCoy, 8529, 13);
	}

	if (Global_Variable_Query(kVariableChapter) > 1
	 && Global_Variable_Query(kVariableChapter) < 5
	) {
		if (Actor_Query_Friendliness_To_Other(kActorZuben, kActorMcCoy) <= 20) {
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 8910, 11);
		} else if (!Game_Flag_Query(kFlagTalkToZuben)) {
			AI_Movement_Track_Pause(kActorZuben);
			Actor_Face_Actor(kActorZuben, kActorMcCoy, true);
			Actor_Says(kActorZuben, 140, 14);
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 7280, 11);
			Actor_Says(kActorZuben, 150, 15);
			Actor_Says(kActorMcCoy, 7285, 12);
			dialogue();
			Game_Flag_Set(kFlagTalkToZuben);
			AI_Movement_Track_Unpause(kActorZuben);
		} else {
			Actor_Face_Actor(kActorMcCoy, kActorZuben, true);
			Actor_Says(kActorMcCoy, 8910, 11);
		}
		// return true;
	}
	// return false;
}

void AIScriptZuben::EnteredSet(int setId) {
	//return false;
}

void AIScriptZuben::OtherAgentEnteredThisSet(int otherActorId) {
	//return false;
}

void AIScriptZuben::OtherAgentExitedThisSet(int otherActorId) {
	//return false;
}

void AIScriptZuben::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if ( otherActorId == kActorMcCoy
	 && !combatMode
	 &&  Game_Flag_Query(kFlagCT07ZubenAttack)
	 && !Game_Flag_Query(kFlagMcCoyShotAtZuben)
	) {
		Non_Player_Actor_Combat_Mode_Off(kActorZuben);
		Game_Flag_Reset(kFlagCT07ZubenAttack);
		AI_Movement_Track_Flush(kActorZuben);
		Actor_Says(kActorMcCoy, 455, 18);
		Actor_Modify_Friendliness_To_Other(kActorZuben, kActorMcCoy, 5);
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT07Spared);
		// return true;
	}
	// return false;
}

void AIScriptZuben::ShotAtAndMissed() {
	//return false;
}

bool AIScriptZuben::ShotAtAndHit() {
	Game_Flag_Set(kFlagMcCoyShotAtZuben);
	if ( Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT07RunToFreeSlotA
	 && !Actor_Clue_Query(kActorZuben, kClueMcCoyShotZubenInTheBack)
	) {
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
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenDiesInChapter1);
#endif // BLADERUNNER_ORIGINAL_BUGS
		// return false;
		return;
	}
	Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1);
	Actor_Set_Goal_Number(kActorZuben, kGoalZubenGone);
	if (Global_Variable_Query(kVariableReplicantsSurvivorsAtMoonbus) == 0) {
		Player_Loses_Control();
		Delay(2000);
		Player_Set_Combat_Mode(false);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, false);
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Game_Flag_Set(kFlagKP07toKP06);
		Game_Flag_Reset(kFlagMcCoyIsHelpingReplicants);
		Set_Enter(kSetKP05_KP06, kSceneKP06);
		// return true;
		return;
	}
	// return false;
}

int AIScriptZuben::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptZuben::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalZubenCT01WalkToCT02:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append(kActorZuben, 86, 0);
		AI_Movement_Track_Append(kActorZuben, 106, 0);
		AI_Movement_Track_Append_With_Facing(kActorZuben, 45, 0, 481);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case kGoalZubenCT02RunToFreeSlotG:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 84, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 85, 0);
		AI_Movement_Track_Append(kActorZuben, 39, 0);
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask1);
		AI_Countdown_Timer_Start(kActorZuben, kActorTimerAIScriptCustomTask1, 10);
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorZuben, kActorTimerAIScriptCustomTask0, 70);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case kGoalZubenCT07Spared:
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
			Actor_Clue_Acquire(kActorZuben, kClueMcCoysDescription, true, -1);
		}
		if (Random_Query(1, 5) < 5) {
			Actor_Clue_Acquire(kActorZuben, kClueMcCoyIsABladeRunner, true, -1);
		}
		Actor_Clue_Acquire(kActorZuben, kClueMcCoyLetZubenEscape, true, -1);
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT07RunToFreeSlotA);
		return false;

	case kGoalZubenCT07RunToFreeSlotA:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 94, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 33, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case kGoalZubenDie:
		Game_Flag_Set(kFlagZubenRetired);
		Game_Flag_Set(kFlagCT01ZubenGone);
		if (Actor_Query_In_Set(kActorZuben, kSetCT07)) {
			Game_Flag_Set(kFlagCT07toCT06);
			Actor_Set_Goal_Number(kActorGaff, kGoalGaffCT12WaitForMcCoy);
			Set_Enter(kSetCT06, kSceneCT06);
		} else if (Actor_Query_In_Set(kActorZuben, kSetMA01)) {
			Player_Set_Combat_Mode(false);
			Actor_Set_Goal_Number(kActorGaff, kGoalGaffMA01ApproachMcCoy);
		}
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenGone);
		return false;

	case kGoalZubenCT02PushPot:
		_animationFrame = 0;
		_animationState = 26;
		return false;

	case kGoalZubenCT02RunToDoor:
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 46, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case kGoalZubenCT02OpenDoor:
		Actor_Face_Heading(kActorZuben, kActorMcCoy, false);
		_animationFrame = 0;
		_animationState = 27;
		return false;

	case kGoalZubenCT06JumpDown:
		AI_Countdown_Timer_Reset(kActorZuben, kActorTimerAIScriptCustomTask0);
		Actor_Put_In_Set(kActorZuben, kSetCT06);
		Actor_Set_At_XYZ(kActorZuben, 37.14f, -58.23f, 4.0f, 256);
		_animationFrame = 0;
		_animationState = 28;
		return false;

	case kGoalZubenCT06AttackMcCoy:
		Player_Loses_Control();
		if (!Player_Query_Combat_Mode()) {
			Player_Set_Combat_Mode(true);
		}
		Player_Gains_Control();
		Game_Flag_Set(kFlagNotUsed142);
		Set_Enter(kSetCT07, kSceneCT07);
		return false;

	case kGoalZubenCT06HideAtFreeSlotA:
		Actor_Set_Targetable(kActorZuben, true);
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 51, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 48, 0);
		AI_Movement_Track_Append(kActorZuben, 33, 1);  // Hide Zuben
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case kGoalZubenCT02PotDodgeCheck:
		if (Actor_Query_Goal_Number(kActorMcCoy) != kGoalMcCoyDodge) {
			Player_Loses_Control();
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
			Game_Flag_Set(kFlagCT02ZubenFled);
			Game_Flag_Set(kFlagCT02McCoyFell);
		}
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT02RunToDoor);
		return false;

	case kGoalZubenMA01AttackMcCoy:
		Game_Flag_Set(kFlagCT05WarehouseOpen);
		Actor_Set_Targetable(kActorZuben, true);
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 125, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case 22: // is never set
		AI_Movement_Track_Flush(kActorZuben);
		AI_Movement_Track_Append_Run(kActorZuben, 125, 0);
		AI_Movement_Track_Append_Run(kActorZuben, 33, 0);
		AI_Movement_Track_Repeat(kActorZuben);
		return false;

	case kGoalZubenDiesInChapter1:
#if BLADERUNNER_ORIGINAL_BUGS
		AI_Movement_Track_Flush(kActorZuben);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Scene_Exits_Enable();
		Music_Stop(2);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Set_Goal_Number(kActorZuben, kGoalZubenDie);
#endif // BLADERUNNER_ORIGINAL_BUGS
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
				} else if (rnd2 == 3) {
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
			AI_Movement_Track_Append(kActorZuben, 490, 0); // kSetNR01
			AI_Movement_Track_Append(kActorZuben, 491, 8);
			AI_Movement_Track_Append(kActorZuben, 492, 2);
			AI_Movement_Track_Append(kActorZuben, 493, 5);
			AI_Movement_Track_Append(kActorZuben, 494, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10); // kSetFreeSlotG
			AI_Movement_Track_Append(kActorZuben, 33, Random_Query(15, 45)); // kSetFreeSlotA
			AI_Movement_Track_Append(kActorZuben, 39, 10); // kSetFreeSlotG
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 2:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 495, 0); // kSetNR02
			AI_Movement_Track_Append(kActorZuben, 496, 0);
			AI_Movement_Track_Append(kActorZuben, 33, Random_Query(15, 45));
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 3:
			AI_Movement_Track_Append(kActorZuben, 498, 0); // kSetNR03
			AI_Movement_Track_Append(kActorZuben, 497, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 4:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 499, 2); // kSetNR05_NR08
			AI_Movement_Track_Append(kActorZuben, 500, 3);
			AI_Movement_Track_Append(kActorZuben, 499, 2);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 5:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 503, 0); // kSetNR05_NR08
			AI_Movement_Track_Append(kActorZuben, 504, 0);
			AI_Movement_Track_Append(kActorZuben, 505, 5);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 6:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 508, 0); // kSetHF01
			AI_Movement_Track_Append(kActorZuben, 509, 2);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 7:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 514, 0); // kSetHF03
			AI_Movement_Track_Append(kActorZuben, 515, 0);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Append(kActorZuben, 33, 30);
			AI_Movement_Track_Append(kActorZuben, 39, 10);
			AI_Movement_Track_Repeat(kActorZuben);
			break;

		case 8:
			AI_Movement_Track_Flush(kActorZuben);
			AI_Movement_Track_Append(kActorZuben, 510, 0); // kSetHF01
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
		if (Actor_Query_Goal_Number(kActorZuben) != kGoalZubenDefault) {
			*animation = kModelAnimationZubenLooksAtSomeone;
			if (_animationLoopCounter < _animationLoopLength) {
				_animationFrame += _animationLoopDirection;
				if (_animationFrame > _animationLoopFrameMax) {
					_animationFrame = _animationLoopFrameMax;
					_animationLoopDirection = -1;
				} else if (_animationFrame < _animationLoopFrameMin) {
					_animationFrame = _animationLoopFrameMin;
					_animationLoopDirection = 1;
				}
				++_animationLoopCounter;
			} else {
				_animationFrame += _animationLoopDirection;
				_animationLoopLength = 0;

				if (_animationFrame == 13 && Random_Query(0, 1)) {
					_animationLoopDirection = -1;
					_animationLoopCounter = 0;
					_animationLoopFrameMin = 8;
					_animationLoopFrameMax = 13;
					_animationLoopLength = Random_Query(0, 30);
				}

				if (_animationFrame == 23) {
					if (Random_Query(0, 1)) {
						_animationLoopDirection = -1;
						_animationLoopCounter = 0;
						_animationLoopFrameMin = 20;
						_animationLoopFrameMax = 23;
						_animationLoopLength = Random_Query(0, 30);
					}
				}

				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
					_animationFrame = 0;
					if (Random_Query(0, 1)) {
						_animationLoopCounter = 0;
						_animationLoopFrameMin = 0;
						_animationLoopFrameMax = 2;
						_animationLoopLength = Random_Query(0, 45);
					}
				}

				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
				}
			}
		} else {
			*animation = kModelAnimationZubenPlayWithHands;
			++_animationFrame;
			if (_animationFrame >= 24) {
				_animationFrame = 5;
			}
			*frame = _animationFrame;
		}
		break;

	case 1:
		*animation = kModelAnimationZubenWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationZubenCombatWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationZubenRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = kModelAnimationZubenCombatRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 5:
		*animation = kModelAnimationZubenCombatUnholsterCleaver;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02PotDodgeCheck) {
				_animationState = 8;
			} else {
				_animationState = 7;
			}
		}
		break;

	case 6:
		*animation = kModelAnimationZubenCombatHolsterCleaver;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 7:
		*animation = kModelAnimationZubenCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 8:
		*animation = kModelAnimationZubenCleaverAttack;
		++_animationFrame;
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

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			if (Actor_Query_Goal_Number(kActorZuben) == kGoalZubenCT02PotDodgeCheck) {
				Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT02RunToDoor);
				_animationFrame = 0;
				_animationState = 0;
			} else {
				_animationFrame = 0;
				_animationState = 7;
				*animation = kModelAnimationZubenCombatIdle;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeCombatIdle);
			}
		}
		break;

	case 9:
		*animation = kModelAnimationZubenClimbShotFront;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 13;
				*animation = kModelAnimationZubenShotDead;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeDie);
			} else {
				_animationFrame = 0;
				_animationState = 0;
				*animation = kModelAnimationZubenIdle;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeIdle);
			}
		}
		break;

	case 10:
		*animation = kModelAnimationZubenClimbShotBack;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 13;
				*animation = kModelAnimationZubenShotDead;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeDie);
			} else {
				_animationFrame = 0;
				_animationState = 0;
				*animation = kModelAnimationZubenIdle;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeIdle);
			}
		}
		break;

	case 11:
		*animation = kModelAnimationZubenCombatHitFront;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 14;
				*animation = kModelAnimationZubenCombatShotDead;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeDie);
			} else {
				_animationFrame = 0;
				_animationState = 7;
				*animation = kModelAnimationZubenCombatIdle;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeCombatIdle);
			}
		}
		break;

	case 12:
		*animation = kModelAnimationZubenCombatHitBack;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			if (Actor_Query_Goal_Number(kActorZuben) == 99) {
				_animationFrame = 0;
				_animationState = 14;
				*animation = kModelAnimationZubenCombatShotDead;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeDie);
			} else {
				_animationFrame = 0;
				_animationState = 7;
				*animation = kModelAnimationZubenCombatIdle;
				Actor_Change_Animation_Mode(kActorZuben, kAnimationModeCombatIdle);
			}
		}
		break;

	case 13:
		*animation = kModelAnimationZubenShotDead;
		++_animationFrame;
		if (_animationFrame == 7) {
			Sound_Play(kSfxZUBDEAD1, 30, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenDie);
#endif // BLADERUNNER_ORIGINAL_BUGS
			_animationState = 15;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Set_Targetable(kActorZuben, false);
		}
		break;

	case 14:
		*animation = kModelAnimationZubenCombatShotDead;
		++_animationFrame;
		if (_animationFrame == 7) {
			Sound_Play(kSfxZUBDEAD1, 30, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenDie);
#endif // BLADERUNNER_ORIGINAL_BUGS
			_animationState = 16;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Set_Targetable(kActorZuben, false);
		}
		break;

	case 15:
		*animation = kModelAnimationZubenShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
#if BLADERUNNER_ORIGINAL_BUGS
		// This enables exits when it should not at the moonbus massacre
		// but it was also used to enable the exits when Zuben is retired at McCoy's roof top.
		Scene_Exits_Enable();
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case 16:
		*animation = kModelAnimationZubenCombatShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
		break;

	case 17:
		*animation = kModelAnimationZubenCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 18:
		*animation = kModelAnimationZubenProtestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 19:
		*animation = kModelAnimationZubenQuestionTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 20:
		*animation = kModelAnimationZubenMoreQuestionTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 21:
		*animation = kModelAnimationZubenPointingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 22:
		*animation = kModelAnimationZubenYetMoreQuestiongTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 23:
		*animation = kModelAnimationZubenScratchEarTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 24:
		*animation = kModelAnimationZubenDontKnowTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 25:
		*animation = kModelAnimationZubenThreatenTalk;
		 ++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = kModelAnimationZubenCalmTalk;
		}
		break;

	case 26:
		Actor_Set_Frame_Rate_FPS(kActorZuben, -1);
		*animation = kModelAnimationZubenToppleSoupCauldron;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			// Time for McCoy to react and avoid tipping pot
			Actor_Set_Frame_Rate_FPS(kActorZuben, -2);
			_animationState = 0;
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT02PotDodgeCheck);
		}
		break;

	case 27:  // Opening the door
		Actor_Set_Frame_Rate_FPS(kActorZuben, -1);
		*animation = kModelAnimationZubenBashOnDoor;
		++_animationFrame;
		if (_animationFrame == 5) {
			Overlay_Play("ct02over", 1, false, true, 0);
		}
		if (_animationFrame == 6) {
			Sound_Play(kSfxMTLDOOR2, 40, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Set_Frame_Rate_FPS(kActorZuben, -2);
			_animationState = 0;
			_animationFrame = 0;
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT02RunToFreeSlotG);
		}
		break;

	case 28:
		*animation = kModelAnimationZubenJumpDownFromCeiling;
		++_animationFrame;
		if (_animationFrame == 1) {
			Sound_Play(kSfxZUBLAND1, 80, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationZubenCombatUnholsterCleaver;
			Actor_Set_Goal_Number(kActorZuben, kGoalZubenCT06AttackMcCoy);
		}
		break;

	default:
		*animation = kModelAnimationZubenWalking;
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
			// fall through
		case 4:
			_animationState = 7;
			_animationFrame = 0;
			break;

		case 5:
			// fall through
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

	case kAnimationModeHit:
		switch (_animationState) {
		case 2:
			// fall through
		case 4:
			// fall through
		case 5:
			// fall through
		case 7:
			// fall through
		case 8:
			if (Random_Query(0, 1)) {
				_animationState = 11;
			} else {
				_animationState = 12;
			}
			_animationFrame = 0;
			break;

		default:
			if (Random_Query(0, 1)) {
				_animationState = 9;
			} else {
				_animationState = 10;
			}
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatHit:
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

	case kAnimationModeDie:
		Actor_Set_Targetable(kActorZuben, false);
		if (_vm->_cutContent) {
			// original code used no voice here
			Sound_Play_Speech_Line(kActorZuben, 9020, 75, 0, 99); // add Zuben's death rattle here
		}
		switch (_animationState) {
			case 2:
				// fall through
			case 4:
				// fall through
			case 5:
				// fall through
			case 7:
				// fall through
			case 8:
				// fall through
			case 11:
				// fall through
			case 12:
				_animationState = 14;
				_animationFrame = 0;
				break;

			default:
				_animationState = 13;
				_animationFrame = 0;
				break;
		}
		break;

	case kAnimationModeCombatDie:
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
	DM_Add_To_List_Never_Repeat_Once_Selected(1490, 5, 5, -1); // CLOVIS
	if (Actor_Query_Goal_Number(kActorLucy) != kGoalLucyGone) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1500, 5, 5, 5); // LUCY
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(1510, -1, 5, 5); // RUNCITER
	Dialogue_Menu_Add_DONE_To_List(1520); // DONE
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
