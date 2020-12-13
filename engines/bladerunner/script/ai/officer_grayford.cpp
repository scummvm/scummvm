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

AIScriptOfficerGrayford::AIScriptOfficerGrayford(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1, 2
	_varChooseIdleAnimation = 0;
	_resumeIdleAfterFramesetCompletesFlag = false;
	_varNumOfTimesToHoldCurrentFrame = 0;
}

void AIScriptOfficerGrayford::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_varChooseIdleAnimation = 0;
	_resumeIdleAfterFramesetCompletesFlag = false;
	_varNumOfTimesToHoldCurrentFrame = 0;

	Actor_Put_In_Set(kActorOfficerGrayford, kSetFreeSlotG);
	Actor_Set_At_Waypoint(kActorOfficerGrayford, 39, 0); // kSetFreeSlotG
	Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordDefault);
}

bool AIScriptOfficerGrayford::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4
	 && Actor_Query_Goal_Number(kActorOfficerGrayford) < kGoalOfficerGrayfordStartOfAct4) {
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStartOfAct4);
		return false;
	}

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorOfficerGrayford) < kGoalOfficerGrayfordStartOfAct5
	) {
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStartOfAct5);
		return false;
	}

	if (!Game_Flag_Query(kFlagMcCoyInDNARow)
	 &&  Actor_Query_Goal_Number(kActorOfficerGrayford) > kGoalOfficerGrayfordArrivedAtDR04
	 &&  Actor_Query_Goal_Number(kActorOfficerGrayford) < kGoalOfficerGrayfordLeavesWithMorajiCorpseDR04
	) {
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordLeavesWithMorajiCorpseDR04);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordDefault) {
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03a);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordPrepareToRestartWalkAround) {
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordDefault);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordArrivedAtDR04) {
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordTalkToMcCoyAndReportAtDR04);
		return false;
	}

	if (Game_Flag_Query(kFlagUnpauseGenWalkers)) {
		AI_Movement_Track_Unpause(kActorGenwalkerA);
		AI_Movement_Track_Unpause(kActorGenwalkerB);
		AI_Movement_Track_Unpause(kActorGenwalkerC);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordAttackMcCoyAct4
	 && Actor_Query_Which_Set_In(kActorOfficerGrayford) != Player_Query_Current_Set()
	) {
		Non_Player_Actor_Combat_Mode_Off(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordDead
	 && Actor_Query_Which_Set_In(kActorOfficerGrayford) != Player_Query_Current_Set()
	) {
		// dead officer gets revived and re-used
		Actor_Set_Health(kActorOfficerGrayford, 50, 50);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4);
		return false;
	}

	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordHuntingAroundAct4) {
		switch (Actor_Query_Which_Set_In(kActorOfficerGrayford)) {
		case kSetRC03:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 18, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG01:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 11, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG04:
			// fall through
		case kSetUG05:
			// fall through
		case kSetUG06:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 10, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

// TODO missing UG07 case - would be a type 10 combat, 12 flee?

		case kSetUG08:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 13, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

// TODO missing UG09 case - would be a type ?? //

		case kSetUG10:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 14, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG12:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 16, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG14:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 17, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetMA07:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 7, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetNR01:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 3, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetDR01_DR02_DR04:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 0, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetBB01:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 1, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetCT11:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordAttackMcCoyAct4);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 5, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		}
		return false;
	}
	return false;
}

void AIScriptOfficerGrayford::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);
		if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordPatrolsAtDR04a) {
			Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPatrolsAtDR04b);
		} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordPatrolsAtDR04b) {
			Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPatrolsAtDR04a);
		}
	}
}

void AIScriptOfficerGrayford::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorOfficerGrayford)) {
	case kGoalOfficerGrayfordWalksInPS03a:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03b);
		break;

	case kGoalOfficerGrayfordWalksInPS03b:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03c);
		break;

	case kGoalOfficerGrayfordWalksInPS03c:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03d);
		break;

	case kGoalOfficerGrayfordWalksInPS03d:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInFreeSlotC);
		break;

	case kGoalOfficerGrayfordWalksInFreeSlotC:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS09a);
		break;

	case kGoalOfficerGrayfordWalksInPS09a:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS09b);
		break;

	case kGoalOfficerGrayfordWalksInPS09b:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS09c);
		break;

	case kGoalOfficerGrayfordWalksInPS09c:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03e);
		break;

	case kGoalOfficerGrayfordWalksInPS03e:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPrepareToRestartWalkAround);
		break;

	case kGoalOfficerGrayfordPatrolsAtDR04a:
		// fall through
	case kGoalOfficerGrayfordPatrolsAtDR04b:
		if (Random_Query(0, 2)) {
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 43);
		} else {
			AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);
			AI_Countdown_Timer_Start(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2, Random_Query(6, 12));
		}
		Actor_Face_Waypoint(kActorOfficerGrayford, 97, true); // kSetDR01_DR02_DR04
		// return false;
		break;

	case kGoalOfficerGrayfordHuntingAroundAct4:
//		debug("Grayford completed Movement");
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPrepareToHuntAroundAct4);
		break;

	case kGoalOfficerGrayfordBlockingUG07:
		// UG07 before McCoy visits his apartment in Act 4
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 12, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 15, 300, false);
		break;

	case kGoalOfficerGrayfordArrestsMcCoyAct4CT12:
		Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		break;

	}

	// return true;
}

void AIScriptOfficerGrayford::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptOfficerGrayford::ClickedByPlayer() {
	switch (Actor_Query_Goal_Number(kActorOfficerGrayford)) {
	case kGoalOfficerGrayfordWalksInPS03a:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalk1);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
		} else {
			Actor_Says(kActorMcCoy, 4515, 13);
			Actor_Says(kActorOfficerGrayford, 230, 13);
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03a);
		break;

	case kGoalOfficerGrayfordWalksInPS03b:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalk1);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
		} else {
			Actor_Says(kActorMcCoy, 4515, 13);
			Actor_Says(kActorOfficerGrayford, 330, 13);
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03b);
		break;

	case kGoalOfficerGrayfordWalksInPS03c:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalk1);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14); // Hey, pal.
		} else {
			Actor_Says(kActorMcCoy, 5075, 14); // Hey, pal. TODO  bug in the original? Matches the above statement
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03c);
		break;

	case kGoalOfficerGrayfordWalksInPS03d:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalk1);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
			Actor_Says(kActorOfficerGrayford, 160, 13);
		} else {
			Actor_Says(kActorMcCoy, 4515, 13);
			Actor_Says(kActorOfficerGrayford, 330, 13);
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS03d);
		break;

	case kGoalOfficerGrayfordWalksInPS09b:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalk1);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 4515, 14);
		Actor_Says(kActorOfficerGrayford, 330, 13);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS09b);
		break;

// TODO goals 6 and 9?

	case kGoalOfficerGrayfordWalksInPS09c:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalk1);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 5075, 13);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordWalksInPS09c);
		break;

	case kGoalOfficerGrayfordPatrolsAtDR04a:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalkDR04);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 1005, kAnimationModeTalk); // Don't miss anything.
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);
		if (_animationState == 35 || _animationState == 34) {
			_animationState = 37;
			_animationFrame = 0;
		}
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorOfficerGrayford, 190, 19);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPatrolsAtDR04a);
		break;

	case kGoalOfficerGrayfordPatrolsAtDR04b:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordStopAndTalkDR04);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 1005, kAnimationModeTalk); // Don't miss anything.
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);
		if (_animationState == 35 || _animationState == 34) {
			_animationState = 37;
			_animationFrame = 0;
		}
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorOfficerGrayford, 190, 19);
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPatrolsAtDR04b);
		break;

	default:
		return; //false;
//		break; // redundant
	}

	return; //true;
}

void AIScriptOfficerGrayford::EnteredSet(int setId) {
	// return false;
}

void AIScriptOfficerGrayford::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptOfficerGrayford::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptOfficerGrayford::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptOfficerGrayford::ShotAtAndMissed() {
	// return false;
}

bool AIScriptOfficerGrayford::ShotAtAndHit() {
	// Grayford is invincible while blocking the UG07 (before McCoy visits his apartment in Act 4)
	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordBlockingUG07) {
		Actor_Set_Health(kActorOfficerGrayford, 50, 50);
	}
	return false;
}

void AIScriptOfficerGrayford::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordDead);
	Game_Flag_Set(kFlagMcCoyRetiredHuman);
}

int AIScriptOfficerGrayford::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptOfficerGrayford::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalOfficerGrayfordWalksInPS03a:
		// kSetPS03
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 82, Random_Query(5, 20));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS03b:
		// kSetPS03
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 76, Random_Query(10, 20));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS03c:
		// kSetPS03
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 77, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS03d:
		// kSetPS03
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 78, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInFreeSlotC:
		// kSetFreeSlotC
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS09a:
		// kSetPS09
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 79, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS09b:
		// kSetPS09
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 80, 1);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS09c:
		// kSetPS09
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 81, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordWalksInPS03e:
		// kSetPS03
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 82, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordArrivesToDR04:
		// kSetDR01_DR02_DR04
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Put_In_Set(kActorOfficerGrayford, kSetDR01_DR02_DR04);
		Actor_Set_At_Waypoint(kActorOfficerGrayford, 110, 0);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Loop_Actor_Walk_To_Waypoint(kActorOfficerGrayford, 111, 0, false, true);

		_animationState = 23;
		_animationFrame = kActorMcCoy;

		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);

		if (Game_Flag_Query(kFlagDR04McCoyShotMoraji)) {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
		} else {
			Actor_Says(kActorMcCoy, 960, 15);
			Actor_Says(kActorMcCoy, 965, 18);
			_animationState = 24;
		}
		return true;

	case kGoalOfficerGrayfordArrivedAtDR04:
		return true;

	case kGoalOfficerGrayfordTalkToMcCoyAndReportAtDR04:
		// kSetDR01_DR02_DR04
		Actor_Says(kActorOfficerGrayford, 120, 19);
		Actor_Says_With_Pause(kActorMcCoy, 970, 0.2f, 13); // Got a dead man here. Victim of an explosion.
		Actor_Says(kActorMcCoy, 975, 12); // TODO - a bug? McCoy may not know Moraji's name here(?)

		if (Actor_Clue_Query(kActorMcCoy, kClueMorajiInterview) == 1) {
			Actor_Says(kActorMcCoy, 980, 16);
			Actor_Says_With_Pause(kActorOfficerGrayford, 130, 0.1f, 13);
			Actor_Says(kActorMcCoy, 985, 14);
			Actor_Says_With_Pause(kActorMcCoy, 990, 0.0f, 17);
			Actor_Says_With_Pause(kActorOfficerGrayford, 140, 1.0f, 16);
			Actor_Says_With_Pause(kActorOfficerGrayford, 150, 0.0f, 17);
			Actor_Says(kActorOfficerGrayford, 160, 15);
			Actor_Says_With_Pause(kActorMcCoy, 995, 0.3f, 14);
		}

		Player_Gains_Control();

		if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDead) {
			Actor_Face_Actor(kActorOfficerGrayford, kActorMoraji, true);
		} else {
			Actor_Face_Waypoint(kActorOfficerGrayford, 97, true);
		}

		Actor_Change_Animation_Mode(kActorOfficerGrayford, 43);

		if (Player_Query_Current_Scene() == kSceneDR04) {
			Actor_Says(kActorOfficerGrayford, 170, kAnimationModeTalk); // This is 32, Sector 3. Reporting a homicide. Possible act of terrorism.
		}
		return true;

	case kGoalOfficerGrayfordPatrolsAtDR04a:
		// kSetDR01_DR02_DR04
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 112, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordPatrolsAtDR04b:
		// kSetDR01_DR02_DR04
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 113, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordStopPatrolToTalkToMcCoyAtDR04:
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 1000, 14); // You got a sheet or something...?
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, kActorTimerAIScriptCustomTask2);

		if (_animationState == 35
		 || _animationState == 34
		) {
			_animationState = 37;
			_animationFrame = 0;
		}

		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorOfficerGrayford, 180, 18);
		Actor_Set_Goal_Number(kActorOfficerGrayford, currentGoalNumber);
		return true; // possible bugfix: was break;

	case kGoalOfficerGrayfordLeavesWithMorajiCorpseDR04:
		// kSetFreeSlotC
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 35, 0); // kSetFreeSlotC
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		AI_Movement_Track_Flush(kActorMoraji);
		AI_Movement_Track_Append(kActorMoraji, 41, 0); // kSetFreeSlotI
		AI_Movement_Track_Repeat(kActorMoraji);
		return true;

	case kGoalOfficerGrayfordStartOfAct4:
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4);
		return true;

	case kGoalOfficerGrayfordHuntingAroundAct4:
//		debug("Flushing Grayford movement track");
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		switch (Random_Query(1, 10)) {
		case 1:
			// kSetNR01
//			debug("gray 1 kSetNR01");
			AI_Movement_Track_Append(kActorOfficerGrayford, 398, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 399, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 400, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 401, 0);
			AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 402, 3, 276);
			AI_Movement_Track_Append(kActorOfficerGrayford, 403, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 404, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;

		case 2:
			// kSetCT11
//			debug("gray 2 kSetCT11");
			AI_Movement_Track_Append(kActorOfficerGrayford, 385, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 242, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 386, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;

		case 3:
			// kSetDR01_DR02_DR04
//			debug("gray 3 kSetDR01_DR02_DR04");
			AI_Movement_Track_Append(kActorOfficerGrayford, 390, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 391, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 392, 5);
			AI_Movement_Track_Append(kActorOfficerGrayford, 345, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 393, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;

		case 4:
			// kSetRC03 -> kSetFreeSlotC
//			debug("gray 4 kSetRC03 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerGrayford, 381, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 382, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 383, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 382, 3);
			AI_Movement_Track_Append(kActorOfficerGrayford, 384, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;

		case 5:
			// kSetBB01 -> kSetFreeSlotC
//			debug("gray 5 kSetBB01 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerGrayford, 388, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 389, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;

		case 6:
			// kSetCT11 - identical to case 2
//			debug("gray 6 kSetCT11");
			AI_Movement_Track_Append(kActorOfficerGrayford, 385, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 242, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 386, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;

#if BLADERUNNER_ORIGINAL_BUGS
		// Gaff is waiting at MA07 and he will trigger a non-interactive dialogue with McCoy.
		// When the police officer is there as well he will kill McCoy because player cannot control him.

		case 7:
			// kSetMA07 -> kSetFreeSlotC
			AI_Movement_Track_Append(kActorOfficerGrayford, 394, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 395, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 396, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 397, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 396, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 395, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 430, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			//Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4); // redundant
			return true;
#else
		case 7:
//			debug("gray 7 MA07 changed to kSetFreeSlotC");
			// just put him away for a few seconds
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			return true;
#endif // BLADERUNNER_ORIGINAL_BUGS

		case 8:
			switch (Random_Query(1, 7)) {
			case 1:
				// kSetUG10 -> kSetFreeSlotC
//				debug("gray 8-1 kSetUG10 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 302, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 407, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 408, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break;

			case 2:
				// kSetUG14
//				debug("gray 8-2 kSetUG14");
				AI_Movement_Track_Append(kActorOfficerGrayford, 536, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 537, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 538, 5);
				AI_Movement_Track_Append(kActorOfficerGrayford, 537, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 536, 0);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break;

			case 3:
				// kSetUG04 -> kSetFreeSlotC
//				debug("gray 8-3  kSetUG04 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 296, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 409, 2);
				AI_Movement_Track_Append(kActorOfficerGrayford, 296, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break;

			case 4:
				// kSetUG05 -> kSetFreeSlotC
//				debug("gray 8-4 kSetUG05 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 411, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 412, 5);
				AI_Movement_Track_Append(kActorOfficerGrayford, 411, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break;

			case 5:
#if BLADERUNNER_ORIGINAL_BUGS
				// kSetUG06 -> kSetFreeSlotC
				// debug("gray 8-5 kSetUG06 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 413, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 414, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 431, 0, 1017);
				AI_Movement_Track_Append(kActorOfficerGrayford, 432, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break;
#else
				// Don't allow police officers to shoot McCoy while he is
				// disabled reciting his monologue at start of Act 4
				if (Game_Flag_Query(kFlagUG06Chapter4Started)) {
					// kSetUG06 -> kSetFreeSlotC
					// debug("gray 8-5 kSetUG06 -> kSetFreeSlotC");
					AI_Movement_Track_Append(kActorOfficerGrayford, 413, 10);
					AI_Movement_Track_Append(kActorOfficerGrayford, 414, 0);
					AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 431, 0, 1017);
					AI_Movement_Track_Append(kActorOfficerGrayford, 432, 10);
					AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
					AI_Movement_Track_Repeat(kActorOfficerGrayford);
					break;
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
				// fall through

			case 6:
				// kSetUG07 -> kSetFreeSlotC
#if BLADERUNNER_ORIGINAL_BUGS
				// debug("gray 8-6 kSetUG07 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 415, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 416, 0, 620);
				AI_Movement_Track_Append(kActorOfficerGrayford, 417, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 418, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break;
#else
				// Don't put police in UG07 after the UG18 Guzza scene
				// since Clovis may be there too and that does not work well
				if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
					// kSetUG07 -> kSetFreeSlotC
					// debug("gray 8-6 kSetUG07 -> kSetFreeSlotC");
					AI_Movement_Track_Append(kActorOfficerGrayford, 415, 0);
					AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 416, 0, 620);
					AI_Movement_Track_Append(kActorOfficerGrayford, 417, 0);
					AI_Movement_Track_Append(kActorOfficerGrayford, 418, 0);
					AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
					AI_Movement_Track_Repeat(kActorOfficerGrayford);
					break;
				}
#endif // BLADERUNNER_ORIGINAL_BUGS
				// fall through

			case 7:
				// kSetUG01 -> kSetFreeSlotC
//				debug("gray 8-7 kSetUG01 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 405, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 406, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				return false;

			}
#if !BLADERUNNER_ORIGINAL_BUGS
			// does it matter if false or true? case 9 and 10 return false
			return false;
#endif // BLADERUNNER_ORIGINAL_BUGS
			// bug in the game - there should be nothing track related after AI_Movement_Track_Repeat
			// fall through
		case 9:
			if (Random_Query(0, 1)) {
				// kSetUG09 -> kSetFreeSlotC
//				debug("gray 9-1 kSetUG09 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 433, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 434, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 435, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
			} else {
				// kSetUG08 -> kSetFreeSlotC
//				debug("gray 9-0 kSetUG08 -> kSetFreeSlotC");
				AI_Movement_Track_Append(kActorOfficerGrayford, 420, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 422, 2);
				AI_Movement_Track_Append(kActorOfficerGrayford, 421, 1);
				AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 422, 4, 182);
				AI_Movement_Track_Append(kActorOfficerGrayford, 420, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
#if BLADERUNNER_ORIGINAL_BUGS
				// bug in the game? Same code bellow looks like a case 10 and are from set 84 whereas upper one are from set 81
				AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 307, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 309, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
#endif
			}
			return false;

		case 10:
			// kSetUG12 -> kSetFreeSlotC
//			debug("gray 10 kSetUG12 -> kSetFreeSlotC");
			AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 307, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 309, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30); // kSetFreeSlotC
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			return false;

		}
		return false;

	case kGoalOfficerGrayfordPrepareToHuntAroundAct4:
		// aux goal in order to immediately switch back to kGoalOfficerGrayfordHuntingAroundAct4 goal
		// and run GoalChanged() for kGoalOfficerGrayfordHuntingAroundAct4 again
//		debug("Setting Grayford goal to kGoalOfficerGrayfordHuntingAroundAct4");
		Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordHuntingAroundAct4);
		return true;

	case kGoalOfficerGrayfordBlockingUG07:
		// kSetUG07
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 419, 0, 512);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordArrestsMcCoyAct4CT12:
		// kSetCT01_CT12
		// never triggered in original game
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append_Run(kActorOfficerGrayford, 440, 0);
		AI_Movement_Track_Append_Run(kActorOfficerGrayford, 441, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case kGoalOfficerGrayfordArrestMcCoyInTB03Act4:
		// kSetTB02_TB03
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Put_In_Set(kActorOfficerGrayford, kSetTB02_TB03);
		Actor_Set_At_XYZ(kActorOfficerGrayford, -173.89f, 0.0f, 2084.22f, 859);
		Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		return true;

	case kGoalOfficerGrayfordDead:
		_animationState = 32;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordShotDead) - 1;
		return true;

	}
	return false;
}

bool AIScriptOfficerGrayford::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationOfficerGrayfordOscillateIdle;
		}
		if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationOfficerGrayfordLookAroundDownwardsIdle;
		}
		if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationOfficerGrayfordLookAroundSidewaysIdle;
		}
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_varChooseIdleAnimation = 0;
			if (!Random_Query(0, 1)) {
				_varChooseIdleAnimation = Random_Query(1, 2);
			}
		}
		break;

	case 1:
		*animation = kModelAnimationOfficerGrayfordWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordWalking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationOfficerGrayfordRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordRunning)) {
			_animationFrame = 0;
		}
		break;

	case 5:
		*animation = kModelAnimationOfficerGrayfordCombatRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatRunning)) {
			_animationFrame = 0;
		}
		break;

	case 6:
		*animation = kModelAnimationOfficerGrayfordCombatWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatWalking)) {
			_animationFrame = 0;
		}
		break;

	case 9:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationOfficerGrayfordOscillateIdle;
			_animationState = 0;
			_varChooseIdleAnimation = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		} else {
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCalmExplainTalk)) {
				_animationFrame = 0;
				_animationState = Random_Query(9, 11);
			}
		}
		break;

	case 10:
		*animation = kModelAnimationOfficerGrayfordCalmRightHandMoveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCalmRightHandMoveTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 11:
		*animation = kModelAnimationOfficerGrayfordScratchHeadTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordScratchHeadTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 12:
		*animation = kModelAnimationOfficerGrayfordQuickHandMoveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordQuickHandMoveTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 13:
		*animation = kModelAnimationOfficerGrayfordSevereTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordSevereTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 14:
		*animation = kModelAnimationOfficerGrayfordMockTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordMockTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 15:
		*animation = kModelAnimationOfficerGrayfordPointingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordPointingTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 16:
		*animation = kModelAnimationOfficerGrayfordQuestionTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordQuestionTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 17:
		*animation = kModelAnimationOfficerGrayfordDismissTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordDismissTalk)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationOfficerGrayfordCalmExplainTalk;
		}
		break;

	case 18:
		// fall through
	case 19:
		*animation = kModelAnimationOfficerGrayfordCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatIdle)) {
			_animationFrame = 0;
		}
		break;

	case 20:
		*animation = kModelAnimationOfficerGrayfordCombatUnholsterGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatUnholsterGun)) {
			_animationFrame = 0;
			_animationState = 19;
		}
		break;

	case 21:
		*animation = kModelAnimationOfficerGrayfordCombatHolsterGun;
		++_animationFrame;
		if (_animationFrame == 11) {
			Ambient_Sounds_Play_Sound(kSfxHOLSTER1, 25, 0, 0, 25);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationOfficerGrayfordOscillateIdle;
			_animationFrame = 0;
			_animationState = 0;
			_varChooseIdleAnimation = 0;
			if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordArrivesToDR04) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordArrivedAtDR04);
			}
		}
		break;

	case 22:
		*animation = kModelAnimationOfficerGrayfordCombatFireGun;
		++_animationFrame;
		if (_animationFrame == 3) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorOfficerGrayford, snd, 75, 0, 99);
		}
		if (_animationFrame == 5) {
			Actor_Combat_AI_Hit_Attempt(kActorOfficerGrayford);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatFireGun)) {
			_animationState = 19;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		}
		break;

	case 23:
		*animation = kModelAnimationOfficerGrayfordCombatFireGun;
		if (_animationFrame < 2) {
			++_animationFrame;
		}
		break;

	case 24:
		*animation = kModelAnimationOfficerGrayfordCombatFireGun;
		--_animationFrame;
		if (_animationFrame < 0) {
			_animationFrame = 0;
			_animationState = 21;
			*animation = kModelAnimationOfficerGrayfordCombatHolsterGun;
		}
		break;

	case 27:
		*animation = kModelAnimationOfficerGrayfordCombatGotHitRight;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatGotHitRight) - 1) {
			_animationFrame = 0;
			_animationState = 19;
			*animation = kModelAnimationOfficerGrayfordOscillateIdle;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		}
		break;

	case 28:
		*animation = kModelAnimationOfficerGrayfordCombatGotHitLeft;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatGotHitLeft) - 1) {
			_animationFrame = 0;
			_animationState = 19;
			*animation = kModelAnimationOfficerGrayfordOscillateIdle;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		}
		break;

	case 29:
		*animation = kModelAnimationOfficerGrayfordGotHitFront;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordGotHitFront) - 1) {
			*animation = kModelAnimationOfficerGrayfordCombatIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 0);
		}
		break;

	case 30:
		*animation = kModelAnimationOfficerGrayfordGotHitBack;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordGotHitBack) - 1) {
			*animation = kModelAnimationOfficerGrayfordCombatIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 0);
		}
		break;

	case 31:
		*animation = kModelAnimationOfficerGrayfordCombatShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordCombatShotDead) - 1) {
			++_animationFrame;
		}
		break;

	case 32:
		*animation = kModelAnimationOfficerGrayfordShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordShotDead) - 1) {
			++_animationFrame;
		}
		break;

	case 34:
		*animation = kModelAnimationOfficerGrayfordTalkToAndLowerRadio;
		if (_varNumOfTimesToHoldCurrentFrame > 0) {
			--_varNumOfTimesToHoldCurrentFrame;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordTalkToAndLowerRadio)) {
				if (Random_Query(0, 1)) {
					*animation = kModelAnimationOfficerGrayfordPutAwayRadioInPocket;
					_animationState = 37;
				} else {
					*animation = kModelAnimationOfficerGrayfordPlaceRadioToEar;
					_animationState = 35;
				}
				_animationFrame = 0;
			} else {
				if (_animationFrame == 12) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(5, 18);
				}
			}
		}
		break;

	case 35:
		*animation = kModelAnimationOfficerGrayfordPlaceRadioToEar;
		if (_varNumOfTimesToHoldCurrentFrame > 0) {
			--_varNumOfTimesToHoldCurrentFrame;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordPlaceRadioToEar)) {
				if (Random_Query(0, 1)) {
					*animation = kModelAnimationOfficerGrayfordPutAwayRadioInPocket;
					_animationState = 37;
				} else {
					*animation = kModelAnimationOfficerGrayfordTalkToAndLowerRadio;
					_animationState = 34;
				}
				_animationFrame = 0;
			} else {
				if (_animationFrame == 10) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(5, 18);
				}
			}
		}
		break;

	case 36:
		*animation = kModelAnimationOfficerGrayfordTakeOutRadioFromPocket;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordTakeOutRadioFromPocket)) {
			_animationFrame = 0;
			_animationState = 34;
			*animation = kModelAnimationOfficerGrayfordTalkToAndLowerRadio;
		}
		break;

	case 37:
		*animation = kModelAnimationOfficerGrayfordPutAwayRadioInPocket;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationOfficerGrayfordPutAwayRadioInPocket)) {
			*animation = kModelAnimationOfficerGrayfordOscillateIdle;
			_animationState = 0;
			_animationFrame = 0;

			switch (Actor_Query_Goal_Number(kActorOfficerGrayford)) {
			case kGoalOfficerGrayfordTalkToMcCoyAndReportAtDR04:
				// fall through
			case kGoalOfficerGrayfordPatrolsAtDR04a:
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPatrolsAtDR04b);
				break;

			case kGoalOfficerGrayfordPatrolsAtDR04b:
				Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordPatrolsAtDR04a);
				break;
			}
		}
		break;

	default:
		// Dummy placeholder, kModelAnimationZubenWalking (399) is a Zuben animation
		*animation = kModelAnimationZubenWalking;
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptOfficerGrayford::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 9:
			// fall through
		case 10:
			// fall through
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
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		case 18:
			{
				int tmp = _animationFrame;
				Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
				_animationFrame = tmp;
				_animationState = 19;
				break;
			}

		case 19:
			_animationState = 21;
			_animationFrame = 0;
			break;

		case 20:
			// fall through
		case 21:
			// fall through
		case 34:
			// fall through
		case 35:
			// fall through
		case 36:
			// fall through
		case 37:
			return true;

		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeWalk:
		if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordArrivesToDR04) {
			_animationState = 6;
			_animationFrame = 0;
		} else if (_animationState != 1) {
			_animationState = 1;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeRun:
		if (Actor_Query_Goal_Number(kActorOfficerGrayford) == kGoalOfficerGrayfordArrivesToDR04) {
			_animationState = 5;
			_animationFrame = 0;
		} else if (_animationState != 2) {
			_animationState = 2;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeTalk:
		if (_animationState != 36 && _animationState != 34) {
			_animationState = 9;
			_animationFrame = 0;
			_resumeIdleAfterFramesetCompletesFlag = false;
		}
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 5:
			// fall through
		case 6:
			// fall through
		case 7:
			// fall through
		case 8:
			_animationState = 19;
			_animationFrame = 0;
			break;

		case 19:
			// fall through
		case 20:
			// fall through
		case 22:
			// fall through
		case 24:
			return true;

		case 23:
			_animationState = 24;
			_animationFrame = 0;
			break;

		default:
			_animationState = 20;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatAim:
		_animationState = 23;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatAttack:
		_animationFrame = 0;
		_animationState = 22;
		break;

	case kAnimationModeCombatWalk:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatRun:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 10;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		_animationState = 11;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		_animationState = 12;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 15:
		_animationState = 13;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 16:
		_animationState = 14;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 17:
		_animationState = 15;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 18:
		_animationState = 16;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 19:
		_animationState = 17;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case kAnimationModeHit:
		switch (_animationState) {
		case 19:
			// fall through
		case 20:
			// fall through
		case 22:
			// fall through
		case 23:
			// fall through
		case 24:
			// fall through
		case 25:
			// fall through
		case 26:
			if (Random_Query(0, 1)) {
				_animationState = 27;
			} else {
				_animationState = 28;
			}
			_animationFrame = 0;
			break;

		case 21:
			if (Random_Query(0, 1)) {
				_animationState = 29;
			} else {
				_animationState = 30;
			}
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatHit:
		if (Random_Query(0, 1)) {
			_animationState = 27;
		} else {
			_animationState = 28;
		}
		_animationFrame = 0;
		break;

	case 43:
		_animationState = 36;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkUp:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkDown:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkUp:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkDown:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		switch (_animationState) {
		case 0:
			// fall through
		case 1:
			// fall through
		case 2:
			// fall through
		case 3:
			// fall through
		case 4:
			// fall through
		case 7:
			_animationState = 20;
			_animationFrame = 0;
			break;

		case 5:
			// fall through
		case 6:
			return true;

		default:
			_animationState = 32;
			_animationFrame = 0;
			break;
		}
		break;

	case 58:
		_animationState = 18;
		_animationFrame = 0;
		break;

	}

	return true;
}

void AIScriptOfficerGrayford::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptOfficerGrayford::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptOfficerGrayford::ReachedMovementTrackWaypoint(int waypointId) {
//	debug("Grayford reached waypoint: %d", waypointId);
	return true;
}

void AIScriptOfficerGrayford::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
