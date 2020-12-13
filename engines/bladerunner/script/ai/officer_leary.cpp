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
//#include "common/debug.h"

namespace BladeRunner {

AIScriptOfficerLeary::AIScriptOfficerLeary(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1, 2
	_varChooseIdleAnimation = 0;
	_idleModeRequestedWhileInTalkingState = false;
}

void AIScriptOfficerLeary::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationStateNext = 0;
	_varChooseIdleAnimation = 0;
	_idleModeRequestedWhileInTalkingState = false;
	_animationNext = 0;

	Actor_Put_In_Set(kActorOfficerLeary, kSetRC01);
	Actor_Set_At_XYZ(kActorOfficerLeary, -261.80f, 6.00f, 79.58f, 512);
	Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
	Actor_Set_Frame_Rate_FPS(kActorOfficerLeary, 8);
}

bool AIScriptOfficerLeary::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4
	 && Actor_Query_Goal_Number(kActorOfficerLeary) < kGoalOfficerLearyStartOfAct4
	) {
		AI_Movement_Track_Flush(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyStartOfAct4);
		return false;
	}

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorOfficerLeary) < kGoalOfficerLearyStartOfAct5
	) {
		AI_Movement_Track_Flush(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyStartOfAct5);
		return false;
	}

	if (!Game_Flag_Query(kFlagMcCoyInRunciters)
	 &&  Game_Flag_Query(kFlagRC51ChopstickWrapperTaken)
	 &&  Game_Flag_Query(kFlagRC01ChromeDebrisTaken)
	 &&  Player_Query_Current_Scene() != kSceneRC01
	 &&  !Game_Flag_Query(kFlagRC01PoliceDone)          // otherwise this clause keeps repeating
	 &&  Global_Variable_Query(kVariableChapter) < 3
	) {
		Game_Flag_Set(kFlagRC01PoliceDone);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyPoliceDoneFromRC01);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerLeary) != kGoalOfficerLearyRC01WalkToCrowd
	 && Actor_Query_Goal_Number(kActorOfficerLeary) != kGoalOfficerLearyRC01CrowdInterrogation
	 && Game_Flag_Query(kFlagOfficerLearyTakingNotes)
	) {
		Game_Flag_Reset(kFlagOfficerLearyTakingNotes);
		return false;
	}

	if ( Global_Variable_Query(15) > 4
	 && !Actor_Clue_Query(kActorOfficerLeary, kClueMcCoyIsStupid)
	) {
		Actor_Clue_Acquire(kActorOfficerLeary, kClueMcCoyIsStupid, true, -1);
		return false;
	}

	if (Game_Flag_Query(kFlagUnpauseGenWalkers)) {
		Game_Flag_Reset(kFlagUnpauseGenWalkers);
		return false;
	}

	if ( Game_Flag_Query(kFlagUG07Empty)
	 && !Game_Flag_Query(kFlagUG07PoliceLeave)
	) {
		Game_Flag_Set(kFlagUG07PoliceLeave);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyHuntingAroundAct4);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyAttackMcCoyAct4
	 && Actor_Query_Which_Set_In(kActorOfficerLeary) != Player_Query_Current_Set()
	) {
		Non_Player_Actor_Combat_Mode_Off(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyHuntingAroundAct4);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyDead
	 && Actor_Query_Which_Set_In(kActorOfficerLeary) != Player_Query_Current_Set()
	) {
		// dead officer gets revived and re-used
		Actor_Set_Health(kActorOfficerLeary, 40, 40);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyHuntingAroundAct4);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyHuntingAroundAct4) {
		switch (Actor_Query_Which_Set_In(kActorOfficerLeary)) {
		case kSetDR01_DR02_DR04:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 0, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetBB01:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 1, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetCT11:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 5, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetMA07:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 7, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetNR01:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 3, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetRC03:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 18, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG01:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 11, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG04:
			// fall through
		case kSetUG05:
			// fall through
		case kSetUG06:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 10, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

// TODO Missing UG07 case -  would be a type 10 combat, 12 flee?

		case kSetUG08:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 13, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

// TODO Missing UG09 case -  would be a type ?? //

		case kSetUG10:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 14, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG12:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 16, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG14:
			if (Actor_Query_Which_Set_In(kActorOfficerLeary) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 17, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;
		}
	}
	return false;
}

void AIScriptOfficerLeary::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask1) {
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask1);
		if (Actor_Query_In_Set(kActorMcCoy, kSetHF05)) {
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyPoliceAboutToAttackHF05);
			Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPoliceAboutToAttackHF05);
		} else {
			Game_Flag_Set(kFlagHF05PoliceAttacked);
		}
	} else if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask2);
		Game_Flag_Reset(kFlagOfficerLearyTakingNotes);
	}
#if BLADERUNNER_ORIGINAL_BUGS
#else
	else if (timer == kActorTimerAIScriptCustomTask0) {
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask0);
		if (Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyRC01ResumeWalkToCrowd) {
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01WalkToCrowd);
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

}

void AIScriptOfficerLeary::CompletedMovementTrack() {
	int goal = Actor_Query_Goal_Number(kActorOfficerLeary);
	if (goal == kGoalOfficerLearyRC01WalkToCrowd) {
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01CrowdInterrogation);
		return;
	}
	if (goal > 308) {
		return;
	}
	if (goal == kGoalOfficerLearyHuntingAroundAct4) {
//		debug("Leary completed Movement");
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyPrepareToHuntAroundAct4);
		return;
	}
	if (goal == kGoalOfficerLearyBlockingUG07) {
		// UG07 before McCoy visits his apartment in Act 4
		Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateIdle, true, kActorMcCoy, 12, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 15, 300, false);
	}
	if (goal == 308) {
		// goal 308 (and 309) are never triggered in the original code
		Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeCombatIdle);
		Actor_Face_Actor(kActorOfficerLeary, kActorMcCoy, true);
		Actor_Set_Goal_Number(kActorOfficerLeary, 309);
	}
}

void AIScriptOfficerLeary::ReceivedClue(int clueId, int fromActorId) {
	if (clueId == kClueMcCoyRetiredZuben) {
		Actor_Modify_Friendliness_To_Other(kActorOfficerLeary, kActorMcCoy, 5);
	}
	if (clueId == kClueMcCoyLetZubenEscape) {
		Actor_Modify_Friendliness_To_Other(kActorOfficerLeary, kActorMcCoy, -4);
	}
}

void AIScriptOfficerLeary::ClickedByPlayer() {}

void AIScriptOfficerLeary::EnteredSet(int setId) {}

void AIScriptOfficerLeary::OtherAgentEnteredThisSet(int otherActorId) {}

void AIScriptOfficerLeary::OtherAgentExitedThisSet(int otherActorId) {}

void AIScriptOfficerLeary::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {}

void AIScriptOfficerLeary::ShotAtAndMissed() {}

bool AIScriptOfficerLeary::ShotAtAndHit() {
	// Leary is invincible while blocking the UG07 (before McCoy visits his apartment in Act 4)
	if (Actor_Query_Goal_Number(kActorOfficerLeary) == kGoalOfficerLearyBlockingUG07) {
		Actor_Set_Health(kActorOfficerLeary, 50, 50);
	}
	return false;
}

void AIScriptOfficerLeary::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDead);
	Game_Flag_Set(kFlagMcCoyRetiredHuman);
}

int AIScriptOfficerLeary::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	if (otherActorId != kActorMcCoy) {
		return 0;
	}
	switch (clueId) {
	case kClueMcCoyIsInsane:
		return -6;

	case kClueMcCoyIsAnnoying:
		return -2;

	case kClueMcCoyIsStupid:
		return -5;

	case kClueMcCoyRetiredLutherLance:
		return 2;

	case kClueMcCoyShotZubenInTheBack:
		return 4;

	case kClueMcCoyRetiredSadik:
		return 4;

	case kClueMcCoyRetiredGordo:
		return 3;

	case kClueMcCoyRetiredDektora:
		return 3;

	case kClueMcCoyRetiredLucy:
		return 2;

	case kClueMcCoyRetiredZuben:
		return 3;

	case kClueMcCoyLetZubenEscape:
		return -5;
	}
	return 0;
}

bool AIScriptOfficerLeary::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalOfficerLearyRC01WalkToCrowd:
		// kSetRC01
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

	case kGoalOfficerLearyRC01CrowdInterrogation:
		if (Random_Query(1, 3) == 1) {
			if ( Random_Query(1, 2) == 1
			 && !Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewA)
			) {
				Actor_Clue_Acquire(kActorOfficerLeary, kClueCrowdInterviewA, false, -1);
			} else if (!Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewB)) {
				Actor_Clue_Acquire(kActorOfficerLeary, kClueCrowdInterviewB, false, -1);
			}
		}
		if (Game_Flag_Query(kFlagMcCoyInRunciters)) {
			if (Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewA)
			 && Actor_Clue_Query(kActorOfficerLeary, kClueCrowdInterviewB)
			) {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
			} else {
				Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyRC01WalkToCrowd);
			}
		} else {
			Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDefault);
		}
		return true;

	case kGoalOfficerLearyPoliceDoneFromRC01:
		// kSetFreeSlotG -> kSetFreeSlotC
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append(kActorOfficerLeary, 39, Random_Query(120, 240)); // kSetFreeSlotG
		AI_Movement_Track_Append(kActorOfficerLeary, 35, 0); // kSetFreeSlotC
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;

#if BLADERUNNER_ORIGINAL_BUGS
#else
	case kGoalOfficerLearyRC01ResumeWalkToCrowd:
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask0);    // usable for custom stuff are timers 0-2
		AI_Countdown_Timer_Start(kActorOfficerLeary, kActorTimerAIScriptCustomTask0, 4); // wait a few seconds before starting taking notes again
		return true;
#endif // BLADERUNNER_ORIGINAL_BUGS

	case kGoalOfficerLearyEndOfAct1:
		AI_Movement_Track_Flush(kActorOfficerLeary);
		return false;

	case kGoalOfficerLearyVisitsBulletBob:
		// kSetRC04
		// Leary visits Bullet Bob shop - un-triggered
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append(kActorOfficerLeary, 107, 0);
		AI_Movement_Track_Append(kActorOfficerLeary, 108, 0);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;

	case kGoalOfficerLearyStartOfAct4:
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyHuntingAroundAct4);
		return true;

	case kGoalOfficerLearyHuntingAroundAct4:
//		debug("Flushing Leary movement track");
		AI_Movement_Track_Flush(kActorOfficerLeary);
		switch (Random_Query(1, 10)) {
		case 1:
			// kSetNR01
//			debug("leary 1 kSetNR01");
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
			// kSetCT11
//			debug("leary 2 kSetCT11");
			AI_Movement_Track_Append(kActorOfficerLeary, 385, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 242, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 386, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;

		case 3:
			// kSetDR01_DR02_DR04
//			debug("leary 3 kSetDR01_DR02_DR04");
			AI_Movement_Track_Append(kActorOfficerLeary, 390, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 391, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 392, 5);
			AI_Movement_Track_Append(kActorOfficerLeary, 345, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 393, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;

		case 4:
			// kSetRC03 -> kSetFreeSlotC
//			debug("leary 4 kSetRC03 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerLeary, 381, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 382, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 383, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 382, 3);
			AI_Movement_Track_Append(kActorOfficerLeary, 384, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;

		case 5:
			// kSetBB01 -> kSetFreeSlotC
//			debug("leary 5 kSetBB01 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerLeary, 388, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 389, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;

		case 6:
			// kSetCT11 - identical to case 2
//			debug("leary 6 kSetCT11 - identical to case 2");
			AI_Movement_Track_Append(kActorOfficerLeary, 385, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 242, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 386, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;

#if BLADERUNNER_ORIGINAL_BUGS
		// Gaff is waiting at MA07 and he will trigger a non-interactive dialogue with McCoy.
		// When the police officer is there as well he will kill McCoy because player cannot control him.

		case 7:
			// kSetMA07 -> kSetFreeSlotC
			AI_Movement_Track_Append(kActorOfficerLeary, 394, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 395, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 396, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 397, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 396, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 395, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 430, 15);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
#else
		case 7:
			// kSetFreeSlotC
//			debug("leary 7 MA07 changed to kSetFreeSlotC");
			// just put him away for a few seconds
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			break;
#endif // BLADERUNNER_ORIGINAL_BUGS

		case 8:
			switch (Random_Query(1, 7)) {
			case 1:
				// kSetUG10 -> kSetFreeSlotC
//				debug("leary 8-1  kSetUG10 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 302, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 407, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 408, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;

			case 2:
				// kSetUG14
//				debug("leary 8-2 kSetUG14");
				AI_Movement_Track_Append(kActorOfficerLeary, 536, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 537, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 538, 1);
				AI_Movement_Track_Append(kActorOfficerLeary, 537, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 536, 0);
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;

			case 3:
				// kSetUG04 -> kSetFreeSlotC
//				debug("leary 8-3 kSetUG04 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 296, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 409, 2);
				AI_Movement_Track_Append(kActorOfficerLeary, 296, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;

			case 4:
				// kSetUG05 -> kSetFreeSlotC
//				debug("leary 8-4 kSetUG05 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 411, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 412, 5);
				AI_Movement_Track_Append(kActorOfficerLeary, 411, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;

			case 5:
#if BLADERUNNER_ORIGINAL_BUGS
				// kSetUG06 -> kSetFreeSlotC
//				debug("leary 8-5 kSetUG06 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 413, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 414, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 431, 0, 1017);
				AI_Movement_Track_Append(kActorOfficerLeary, 432, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
#else
				// Don't allow police officers to shoot McCoy while he is
				// disabled reciting his monologue at start of Act 4
				if (Game_Flag_Query(kFlagUG06Chapter4Started)) {
					// kSetUG06 -> kSetFreeSlotC
					// debug("leary 8-5 kSetUG06 -> kSetFreeSlotC");
					AI_Movement_Track_Append(kActorOfficerLeary, 413, 10);
					AI_Movement_Track_Append(kActorOfficerLeary, 414, 0);
					AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 431, 0, 1017);
					AI_Movement_Track_Append(kActorOfficerLeary, 432, 10);
					AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
					AI_Movement_Track_Repeat(kActorOfficerLeary);
					break;
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
				// fall through
			case 6:
				// kSetUG07 -> kSetFreeSlotC
#if BLADERUNNER_ORIGINAL_BUGS
				// debug("leary 8-6 kSetUG07 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 415, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 416, 0, 620);
				AI_Movement_Track_Append(kActorOfficerLeary, 417, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 418, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				break;
#else
				// Don't put police in UG07 after the UG18 Guzza scene
				// since Clovis may be there too and that does not work well
				if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
					// debug("leary 8-6 kSetUG07 -> kSetFreeSlotC");
					AI_Movement_Track_Append(kActorOfficerLeary, 415, 0);
					AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 416, 0, 620);
					AI_Movement_Track_Append(kActorOfficerLeary, 417, 0);
					AI_Movement_Track_Append(kActorOfficerLeary, 418, 0);
					AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
					AI_Movement_Track_Repeat(kActorOfficerLeary);
					break;
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
				// fall through

			case 7:
				// kSetUG01 -> kSetFreeSlotC
//				debug("leary 8-7 kSetUG01 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 405, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 406, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				return false;

			default:
				return false;
			}
#if !BLADERUNNER_ORIGINAL_BUGS
			break;
#endif // BLADERUNNER_ORIGINAL_BUGS
			// a bug in original game - no break here
			// fall through
		case 9:
			if (Random_Query(1, 2) == 2) {
				// kSetUG09 -> kSetFreeSlotC
//				debug("leary 9-2 kSetUG09 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerLeary, 433, 10);
				AI_Movement_Track_Append(kActorOfficerLeary, 434, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 435, 0);
				AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerLeary);
				return false;
			}
			// kSetUG08 -> kSetFreeSlotC
//			debug("leary 9-1 kSetUG08 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerLeary, 420, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 422, 2);
			AI_Movement_Track_Append(kActorOfficerLeary, 421, 1);
			AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 422, 4, 182);
			AI_Movement_Track_Append(kActorOfficerLeary, 420, 10);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerLeary);
#if !BLADERUNNER_ORIGINAL_BUGS
			return false;
#endif // BLADERUNNER_ORIGINAL_BUGS
			// a bug in original game - no break or return here
			// fall through
		case 10:
			// kSetUG12 -> kSetFreeSlotC
//			debug("leary 10 kSetUG12 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerLeary, 310, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 307, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 309, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 310, 0);
			AI_Movement_Track_Append(kActorOfficerLeary, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerLeary);
			return false;
		default:
			return false;
		}
#if !BLADERUNNER_ORIGINAL_BUGS
			return false;
#endif // BLADERUNNER_ORIGINAL_BUGS
		// a bug in original game - no break or return here
		// fall through
	case kGoalOfficerLearyPrepareToHuntAroundAct4:
		// aux goal in order to immediately switch back to kGoalOfficerLearyHuntingAroundAct4 goal
		// and run GoalChanged() for kGoalOfficerLearyHuntingAroundAct4 again
//		debug("Setting Leary goal to kGoalOfficerLearyHuntingAroundAct4");
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyHuntingAroundAct4);
		return true;

	case kGoalOfficerLearyBlockingUG07:
		// kSetUG07
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append_With_Facing(kActorOfficerLeary, 416, 0, 556);
		Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeCombatIdle);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;

	case 308:
		// kSetCT01_CT12
		// never triggered - TODO a bug? Could be related to cut McCoy's arrest from Grayford
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append_Run_With_Facing(kActorOfficerLeary, 440, 2, 355);
		AI_Movement_Track_Append_Run_With_Facing(kActorOfficerLeary, 441, 0, 825);
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		return true;

	case kGoalOfficerLearyStartOfAct5:
		// kSetFreeSlotC
		AI_Movement_Track_Flush(kActorOfficerLeary);
		AI_Movement_Track_Append(kActorOfficerLeary, 35, 0); // kSetFreeSlotC
		AI_Movement_Track_Repeat(kActorOfficerLeary);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDummyGoalAct5);
		return true;

	case kGoalOfficerLearyPoliceWait120SecondsToAttackHF05:
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask1);
		AI_Countdown_Timer_Start(kActorOfficerLeary, kActorTimerAIScriptCustomTask1, 120);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDummyGoalAct5);
		return true;

	case kGoalOfficerLearyPoliceWait60SecondsToAttackHF05:
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask1);
		AI_Countdown_Timer_Start(kActorOfficerLeary, kActorTimerAIScriptCustomTask1, 60);
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDummyGoalAct5);
		return true;

	case kGoalOfficerLearyPoliceAboutToAttackHF05:
		Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyDummyGoalAct5);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordDummyGoalAct5);
		return false;

	default:
		return false;
	}
}

bool AIScriptOfficerLeary::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 32:
		*animation = kModelAnimationOfficerLearyPutNotepadAway;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyPutNotepadAway)) {
			*animation = kModelAnimationOfficerLearyOscillateIdle;
			_animationState = 0;
			_varChooseIdleAnimation = 0;
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 31:
		*animation = kModelAnimationOfficerLearyTakeOutNotepad;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyTakeOutNotepad)) {
			*animation = kModelAnimationOfficerLearyTakingNotes;
			_animationState = 1;
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 30:
		*animation = kModelAnimationOfficerLearyGotHitBack;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyGotHitBack)) {
			*animation = kModelAnimationOfficerLearyOscillateIdle;
			_animationState = 0;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeIdle);
		}
		*frame = _animationFrame;
		return true;

	case 29:
		*animation = kModelAnimationOfficerLearyGotHitFront;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyGotHitFront)) {
			*animation = kModelAnimationOfficerLearyOscillateIdle;
			_animationState = 0;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeIdle);
		}
		*frame = _animationFrame;
		return true;

	case 28:
		*animation = kModelAnimationOfficerLearyCombatShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatShotDead) - 1) {
			++_animationFrame;
		}
		*frame = _animationFrame;
		return true;

	case 27:
		*animation = kModelAnimationOfficerLearyShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyShotDead) - 1) {
			++_animationFrame;
		}
		*frame = _animationFrame;
		return true;

	case 26:
		*animation = kModelAnimationOfficerLearyCombatGotHitLeft;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatGotHitLeft)) {
			_animationState = 21;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCombatIdle;
			Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeCombatIdle);
		}
		*frame = _animationFrame;
		return true;

	case 25:
		*animation = kModelAnimationOfficerLearyCombatGotHitRight;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatGotHitRight)) {
			_animationState = 21;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCombatIdle;
			Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeCombatIdle);
		}
		*frame = _animationFrame;
		return true;

	case 24:
		*animation = kModelAnimationOfficerLearyCombatFireGun;
		++_animationFrame;
		if (_animationFrame == 4) {
			if (Random_Query(1, 2) == 1) {
				Sound_Play_Speech_Line(kActorOfficerLeary, 9010, 75, 0, 99);
			} else {
				Sound_Play_Speech_Line(kActorOfficerLeary, 9015, 75, 0, 99);
			}
		}
		if (_animationFrame == 5) {
			Actor_Combat_AI_Hit_Attempt(kActorOfficerLeary);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatFireGun)) {
			_animationState = 21;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCombatIdle;
			Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeCombatIdle);
		}
		*frame = _animationFrame;
		return true;

	case 23:
		*animation = kModelAnimationOfficerLearyCombatHolsterGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatHolsterGun)) {
			*animation = kModelAnimationOfficerLearyOscillateIdle;
			_animationState = 0;
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 22:
		*animation = kModelAnimationOfficerLearyCombatUnholsterGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatUnholsterGun)) {
			_animationState = 21;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCombatIdle;
		}
		*frame = _animationFrame;
		return true;

	case 21:
		*animation = kModelAnimationOfficerLearyCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatIdle)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 20:
		*animation = kModelAnimationOfficerLearyCombatIdle;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatIdle) - 1) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 19:
		*animation = kModelAnimationOfficerLearyScratchHeadApologyTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyScratchHeadApologyTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 18:
		*animation = kModelAnimationOfficerLearyHandsOnWaistTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyHandsOnWaistTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 17:
		*animation = kModelAnimationOfficerLearyUrgeOrQuestionTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyUrgeOrQuestionTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 16:
		*animation = kModelAnimationOfficerLearyMoreUpsetTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyMoreUpsetTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 15:
		*animation = kModelAnimationOfficerLearyUpsetOrCommandTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyUpsetOrCommandTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 14:
		*animation = kModelAnimationOfficerLearyWarnOrThreatenTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyWarnOrThreatenTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 13:
		*animation = kModelAnimationOfficerLearyLongerExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyLongerExplainTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 12:
		*animation = kModelAnimationOfficerLearyIndicateTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyIndicateTalk)) {
			_animationState = 11;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		}
		*frame = _animationFrame;
		return true;

	case 11:
		if (_animationFrame <= 2) {
			_idleModeRequestedWhileInTalkingState = false;
			_animationState = 0;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyOscillateIdle;
			_varChooseIdleAnimation = Random_Query(0, 1);
			*frame = _animationFrame;
			return true;
		}

		*animation = kModelAnimationOfficerLearyCalmExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCalmExplainTalk)) {
			_animationFrame = 0;
			if (_idleModeRequestedWhileInTalkingState) {
				_idleModeRequestedWhileInTalkingState = false;
				_animationState = 0;
				_animationFrame = 0;
				*animation = kModelAnimationOfficerLearyOscillateIdle;
				_varChooseIdleAnimation = Random_Query(0, 1);
			} else {
				_animationState = Random_Query(0, 8) + 11;
			}
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 10:
		*animation = kModelAnimationOfficerLearyCombatClimbStairsDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatClimbStairsDown)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 9:
		*animation = kModelAnimationOfficerLearyCombatClimbStairsUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatClimbStairsUp)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 8:
		*animation = kModelAnimationOfficerLearyCombatRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatRunning)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 7:
		*animation = kModelAnimationOfficerLearyCombatWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyCombatWalking)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 6:
		*animation = kModelAnimationOfficerLearyClimbStairsDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyClimbStairsDown)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 5:
		*animation = kModelAnimationOfficerLearyClimbStairsUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyClimbStairsUp)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 4:
		*animation = kModelAnimationOfficerLearyRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyRunning)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 3:
		*animation = kModelAnimationOfficerLearyWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyWalking)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		return true;

	case 2:
		if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
			*animation = kModelAnimationOfficerLearyPutNotepadAway;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyPutNotepadAway)) {
				Game_Flag_Reset(kFlagOfficerLearyTakingNotes);
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else {
			if (_varChooseIdleAnimation == 0) {
				*animation = kModelAnimationOfficerLearyOscillateIdle;
			}
			if (_varChooseIdleAnimation == 1) {
				*animation = kModelAnimationOfficerLearyLookAroundIdle;
			}
			if (_varChooseIdleAnimation == 2) {
				*animation = kModelAnimationOfficerLearyRockBackForthIdle;
			}
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) / 2) {
				_animationFrame += 3;
				if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
					_animationFrame = 0;
					_animationState = _animationStateNext;
					*animation = _animationNext;
				}
			} else {
				_animationFrame -= 3;
				if (_animationFrame < 0) {
					_animationFrame = 0;
					_animationState = _animationStateNext;
					*animation = _animationNext;
				}
			}
		}
		*frame = _animationFrame;
		return true;

	case 1:
		*animation = kModelAnimationOfficerLearyTakingNotes;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyTakingNotes)) {
			_animationFrame = 0;
		}
		if (!Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
			_animationState = 32;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyPutNotepadAway;
		}
		*frame = _animationFrame;
		return true;

	case 0:
		if ( Game_Flag_Query(kFlagOfficerLearyTakingNotes)
		 && !Game_Flag_Query(kFlagRC01McCoyAndOfficerLearyTalking)) {
			_animationState = 31;
			_animationFrame = 0;
			*animation = kModelAnimationOfficerLearyTakeOutNotepad;
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationOfficerLearyLookAroundIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyLookAroundIdle)) {
				_varChooseIdleAnimation = Random_Query(0, 2);
				_animationFrame = 0;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationOfficerLearyRockBackForthIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyRockBackForthIdle)) {
				_varChooseIdleAnimation = Random_Query(0, 2);
				_animationFrame = 0;
			}
		} else if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationOfficerLearyOscillateIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerLearyOscillateIdle)) {
				_varChooseIdleAnimation = Random_Query(0, 2);
				_animationFrame = 0;
			}
		}
		*frame = _animationFrame;
		return true;
	default:
		// Dummy placeholder, kModelAnimationZubenWalking (399) is a Zuben animation
		*animation = kModelAnimationZubenWalking;
		*frame = _animationFrame;
		return true;
	}
}

bool AIScriptOfficerLeary::ChangeAnimationMode(int mode) {
	int v1;

	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 21:
			// fall through
		case 24:
			_animationState = 23;
			_animationFrame = 0;
			break;

		case 20:
			v1 = _animationFrame;
			Actor_Change_Animation_Mode(kActorOfficerLeary, kAnimationModeCombatIdle);
			_animationFrame = v1;
			_animationState = 21;
			break;

		case 11:
			// fall through
		case 12:
			// fall through
		case 13:
			// fall through
		case 14:
			// fall through
		case 15:
			// fall through
		case 16:
			// fall through
		case 17:
			// fall through
		case 18:
			// fall through
		case 19:
			_idleModeRequestedWhileInTalkingState = true;
			break;

		case 0:
			// fall through
		case 23:
			break;

		default:
			_animationState = 0;
			_animationFrame = 0;
			_varChooseIdleAnimation = Random_Query(0, 1);
			break;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case kAnimationModeRun:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 11;
			_animationNext = kModelAnimationOfficerLearyCalmExplainTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		}
		else if (_animationState > 19) {
			_animationState = 11;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 21:
			// fall through
		case 22:
			// fall through
		case 24:
			break;

		case 7:
			// fall through
		case 8:
			// fall through
		case 9:
			// fall through
		case 10:
			_animationState = 21;
			_animationFrame = 0;
			break;

		case 0:
			_animationState = 22;
			_animationFrame = 0;
			break;

		default:
			_animationState = 21;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatAttack:
		_animationState = 24;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalk:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatRun:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case 12:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 12;
			_animationNext = kModelAnimationOfficerLearyIndicateTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 12;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 13:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 13;
			_animationNext = kModelAnimationOfficerLearyLongerExplainTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 13;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 14:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 14;
			_animationNext = kModelAnimationOfficerLearyWarnOrThreatenTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 14;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 15:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 15;
			_animationNext = kModelAnimationOfficerLearyUpsetOrCommandTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 15;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 16:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 16;
			_animationNext = kModelAnimationOfficerLearyMoreUpsetTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 16;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 17:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 17;
			_animationNext = kModelAnimationOfficerLearyUrgeOrQuestionTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 17;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 18:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 18;
			_animationNext = kModelAnimationOfficerLearyHandsOnWaistTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 18;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 19:
		if (_animationState == 1) {
			_animationState = 2;
			_animationStateNext = 19;
			_animationNext = kModelAnimationOfficerLearyScratchHeadApologyTalk;
			if (Game_Flag_Query(kFlagOfficerLearyTakingNotes)) {
				_animationFrame = 0;
			}
		} else if (_animationState > 19) {
			_animationState = 19;
			_animationFrame = 0;
			_idleModeRequestedWhileInTalkingState = false;
		}
		break;

	case 21:
		if (_animationState == 24) {
			if (Random_Query(0, 1)) {
				_animationState = 25;
			} else {
				_animationState = 26;
			}
			_animationFrame = 0;
		} else {
			if (Random_Query(0, 1)) {
				_animationState = 29;
			} else {
				_animationState = 30;
			}
			_animationFrame = 0;
		}
		break;

	case 22:
		if (Random_Query(0, 1)) {
			_animationState = 25;
		} else {
			_animationState = 26;
		}
		_animationFrame = 0;
		break;

	case 27:
		Game_Flag_Reset(kFlagOfficerLearyTakingNotes);
		break;

	case 28:
		Game_Flag_Set(kFlagOfficerLearyTakingNotes);
		break;

	case kAnimationModeWalkUp:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkDown:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkUp:
		_animationState = 9;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkDown:
		_animationState = 10;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		if (_animationState == 24) {
			_animationState = 28;
			_animationFrame = 0;
		} else {
			_animationState = 27;
			_animationFrame = 0;
		}
		break;

	case 58:
		_animationState = 20;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptOfficerLeary::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptOfficerLeary::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptOfficerLeary::ReachedMovementTrackWaypoint(int waypointId) {
//	debug("Leary reached waypoint: %d", waypointId);
	if (waypointId == 57 || waypointId == 58) {
		// Interrogating crowd in kSetRC01
		Game_Flag_Set(kFlagOfficerLearyTakingNotes);
		AI_Countdown_Timer_Reset(kActorOfficerLeary, kActorTimerAIScriptCustomTask2);
		AI_Countdown_Timer_Start(kActorOfficerLeary, kActorTimerAIScriptCustomTask2, 6);
	}
	return true;
}

void AIScriptOfficerLeary::FledCombat() {
	Actor_Set_Goal_Number(kActorOfficerLeary, kGoalOfficerLearyStartOfAct4);
}

} // End of namespace BladeRunner
