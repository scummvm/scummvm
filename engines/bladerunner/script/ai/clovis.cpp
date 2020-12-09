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

AIScriptClovis::AIScriptClovis(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 0;
	_var3 = 1;
	_var4 = 0;
	_var5 = 0;
	_flag = 0;
}

void AIScriptClovis::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 0;
	_var3 = 1;
	_var4 = 0;
	_var5 = 0;
	_flag = 0;

	Actor_Set_Goal_Number(kActorClovis, kGoalClovisDefault);
}

bool AIScriptClovis::Update() {
	if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisBB11PrepareTalkToMcCoy) {
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisBB11TalkToMcCoy);
		return true;
	}

	if (Global_Variable_Query(kVariableChapter) == 3
	 && Actor_Query_Goal_Number(kActorClovis) < kGoalClovisStartChapter3
	) {
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisStartChapter3);
		return true;
	}

	if ( Global_Variable_Query(kVariableChapter) == 4
	 && !Game_Flag_Query(kFlagClovisChapter4Started)
	) {
		Game_Flag_Set(kFlagClovisChapter4Started);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisStartChapter4);
		return true;
	}

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorClovis) < kGoalClovisStartChapter5
	) {
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisStartChapter5);
	}

	if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisKP06Wait
	 && Game_Flag_Query(kFlagNotUsed657)
	) {
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP06TalkToMcCoy);
	}

	if ( Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	 && !Game_Flag_Query(kFlagKP07ReplicantsAttackMcCoy)
	 &&  Game_Flag_Query(kFlagKP07McCoyPulledGun)
	) {
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07ReplicantsAttackMcCoy);
		Game_Flag_Set(kFlagKP07ReplicantsAttackMcCoy);
		return true;
	}
	return false;

}

void AIScriptClovis::TimerExpired(int timer) {
	//return false;
}

void AIScriptClovis::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorClovis)) {
	case kGoalClovisBB11WalkToMcCoy:
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisBB11StopSadik);
		break;

	case kGoalClovisBB11StopSadik:
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisBB11TalkWithSadik);
		break;

	case kGoalClovisUG07ChaseMcCoy:
		AI_Movement_Track_Flush(kActorClovis);

		if (Player_Query_Current_Scene() == kSceneUG07) {
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisUG07KillMcCoy);
		} else {
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisStartChapter4);
		}
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptClovis::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptClovis::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisGone) {
		Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
		Actor_Says(kActorMcCoy, 8630, 16);
	}
}

void AIScriptClovis::EnteredSet(int setId) {
	// return false;
}

void AIScriptClovis::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptClovis::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptClovis::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	 && Actor_Query_In_Set(kActorMcCoy, kSetKP07)
	) {
		Game_Flag_Set(kFlagKP07McCoyPulledGun);
		Game_Flag_Set(kFlagMcCoyAttackedReplicants);
		// return true;
	}
	// return false;
}

void AIScriptClovis::ShotAtAndMissed() {
	// return false;
}

bool AIScriptClovis::ShotAtAndHit() {
	if (!Game_Flag_Query(kFlagKP07McCoyPulledGun)) {
		if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisKP07SayFinalWords) {
			ADQ_Flush();
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisGone);
			shotAnim();
			Actor_Set_Targetable(kActorClovis, false);
			ADQ_Add(kActorMcCoy, 2340, -1);
			Music_Stop(3);
		} else if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisKP07Wait
		        || Actor_Query_Goal_Number(kActorClovis) == kGoalClovisKP07LayDown
		) {
			ADQ_Flush();
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisGone);
			shotAnim();
			Actor_Set_Targetable(kActorClovis, false);
			Music_Stop(3);
		}
}
	return false;
}

void AIScriptClovis::Retired(int byActorId) {
	if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
		if (Actor_Query_In_Set(kActorClovis, kSetKP07)) {
			Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisGone);

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
				return; //true;
			}
		}
	}
	return; //false;
}

int AIScriptClovis::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	if (otherActorId != kActorMcCoy) {
		return 0;
	}

	switch (clueId) {
	case kClueMcCoyKilledRunciter1:
	case kClueMcCoyKilledRunciter2:
		return 6;
	case kClueMcCoyIsABladeRunner:
	case kClueMcCoyIsStupid:
	case kClueMcCoyIsAnnoying:
		return -2;
	case kClueMcCoyLetZubenEscape:
	case kClueMcCoyHelpedIzoIzoIsAReplicant:
	case kClueMcCoyHelpedDektora:
	case kClueMcCoyHelpedLucy:
	case kClueMcCoyHelpedGordo:
	case kClueMcCoyShotGuzza:
	case kClueMcCoyIsKind:
		return 4;
	case kClueMcCoyWarnedIzo:
		return 2;
	case kClueMcCoyRetiredZuben:
		return -3;
	case kClueMcCoyRetiredLucy:
	case kClueMcCoyRetiredDektora:
		return -10;
	case kClueMcCoyRetiredSadik:
	case kClueMcCoyShotZubenInTheBack:
	case kClueMcCoyRetiredLutherLance:
	case kClueMcCoyIsInsane:
		return -5;
	default:
		break;
	}
	return 0;
}

bool AIScriptClovis::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalClovisDefault:
		AI_Movement_Track_Flush(kActorClovis);
		AI_Movement_Track_Append(kActorClovis, 33, 0);
		AI_Movement_Track_Repeat(kActorClovis);
		return true;

	case kGoalClovisBB11WalkToMcCoy:
		AI_Movement_Track_Flush(kActorClovis);
		AI_Movement_Track_Append(kActorClovis, 319, 0);
		AI_Movement_Track_Append(kActorClovis, 320, 0);
		AI_Movement_Track_Repeat(kActorClovis);
		return true;

	case kGoalClovisBB11StopSadik:
		Actor_Start_Speech_Sample(kActorClovis, 0);
		AI_Movement_Track_Flush(kActorClovis);
		AI_Movement_Track_Append(kActorClovis, 321, 0);
		AI_Movement_Track_Repeat(kActorClovis);
		return true;

	case kGoalClovisBB11TalkWithSadik:
		Actor_Set_Goal_Number(kActorSadik, kGoalSadikBB11TalkWithClovis);
		Actor_Says(kActorClovis, 10, 15);
		Actor_Says(kActorSadik, 0, kAnimationModeTalk);
		Actor_Face_Actor(kActorClovis, kActorSadik, true);
		Actor_Says(kActorClovis, 20, 13);
		Actor_Says(kActorClovis, 30, 12);
		Actor_Face_Actor(kActorSadik, kActorClovis, true);
		Actor_Says(kActorSadik, 10, kAnimationModeTalk);
		Actor_Says(kActorClovis, 40, 17);
		Actor_Says(kActorSadik, 20, kAnimationModeTalk);
		Actor_Face_Actor(kActorClovis, kActorMcCoy, true);
		Actor_Face_Actor(kActorSadik, kActorMcCoy, true);
		Actor_Says(kActorClovis, 50, 14);
		Actor_Change_Animation_Mode(kActorClovis, kAnimationModeSit);
		return true;

	case kGoalClovisBB11TalkToMcCoy:
		Actor_Says(kActorClovis, 60, 30);
		Actor_Says(kActorSadik, 30, kAnimationModeTalk);
		Actor_Says(kActorClovis, 70, 30);
		Actor_Says(kActorClovis, 80, 30);
		Actor_Change_Animation_Mode(kActorClovis, 29);
		Actor_Says(kActorSadik, 40, kAnimationModeTalk);
		Actor_Says(kActorSadik, 50, kAnimationModeTalk);
		Actor_Says(kActorClovis, 90, 13);
		Actor_Face_Current_Camera(5, true);
		Actor_Says(kActorClovis, 100, 17);
		Delay(1000);
		if (!Game_Flag_Query(kFlagSadikIsReplicant)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueStaggeredbyPunches, true, kActorSadik);
		}
		Game_Flag_Set(kFlagNotUsed383);
		Game_Flag_Reset(kFlagBB11SadikFight);
		Global_Variable_Set(kVariableChapter, 3);
		Actor_Set_Goal_Number(kActorClovis, 200);
		Actor_Set_Goal_Number(kActorSadik, 200);
		Actor_Clue_Acquire(kActorMcCoy, kClueAct2Ended, true, -1);
		Player_Gains_Control();
		Chapter_Enter(3, kSetCT08_CT51_UG12, kSceneCT08);
		Game_Flag_Set(kFlagMcCoyTiedDown);
		Game_Flag_Set(kFlagChapter3Intro);
		return true;

	case kGoalClovisUG18Wait:
		Actor_Put_In_Set(kActorClovis, kSetUG18);
		Actor_Set_At_XYZ(kActorClovis, -52.26f, 0.0f, 611.02f, 900);
		Actor_Change_Animation_Mode(kActorClovis, kAnimationModeIdle);
		return true;

	case kGoalClovisUG18SadikWillShootGuzza:
		// fall through
	case kGoalClovisUG18SadikIsShootingGuzza:
		// fall through
	case kGoalClovisUG18GuzzaDied:
		// fall through
	case kGoalClovisUG18Leave:
		return true;

	case kGoalClovisStartChapter3:
		Actor_Put_In_Set(kActorClovis, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorClovis, 33, 0);
		return true;

	case kGoalClovisStartChapter4:
		AI_Movement_Track_Flush(kActorClovis);
		Actor_Put_In_Set(kActorClovis, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorClovis, 33, 0);
		return true;

	case kGoalClovisUG07ChaseMcCoy:
		AI_Movement_Track_Flush(kActorClovis);
		AI_Movement_Track_Append(kActorClovis, 341, 0);
		AI_Movement_Track_Append(kActorClovis, 342, 0);
		AI_Movement_Track_Repeat(kActorClovis);
		return true;

	case kGoalClovisUG07KillMcCoy:
		Player_Loses_Control();
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Face_Actor(kActorMcCoy, kActorClovis, true);
		Loop_Actor_Walk_To_Actor(kActorClovis, kActorMcCoy, 48, false, true);
		Actor_Face_Actor(kActorClovis, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorClovis, kAnimationModeCombatAttack);
		if (Player_Query_Combat_Mode()) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatDie);
		} else {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		}
#if BLADERUNNER_ORIGINAL_BUGS
		Delay(3000);
		Actor_Retired_Here(kActorMcCoy, 12, 48, true, kActorClovis);
#else
		Actor_Retired_Here(kActorMcCoy, 12, 48, true, kActorClovis);
		Delay(3000);
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;

	case kGoalClovisStartChapter5:
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisDecide);
		return true;

	case kGoalClovisDecide:
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07Wait);
		} else {
			Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP06Wait);
			Game_Flag_Set(kFlagClovisLyingDown);
		}
		return true;

	case kGoalClovisKP06Wait:
		Actor_Put_In_Set(kActorClovis, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorClovis, -1072.0f, 8.26f, -708.0f, 530);
		return true;

	case kGoalClovisKP06TalkToMcCoy:
		Actor_Says(kActorClovis, 110, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2255, kAnimationModeTalk);
		Actor_Says(kActorClovis, 120, kAnimationModeTalk);
		Actor_Says(kActorClovis, 130, kAnimationModeTalk);
		Actor_Says(kActorClovis, 140, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2260, kAnimationModeTalk);
		Actor_Says(kActorClovis, 150, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07Wait);
		return true;

	case kGoalClovisKP07Wait:
		Actor_Put_In_Set(kActorClovis, kSetKP07);
		Actor_Set_Targetable(kActorClovis, true);
		if (Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)) {
			Global_Variable_Set(kVariableReplicantsSurvivorsAtMoonbus, 0);
			Global_Variable_Increment(kVariableReplicantsSurvivorsAtMoonbus, 1);
			Actor_Set_At_XYZ(kActorClovis, 45.0f, -41.52f, -85.0f, 750);
		} else {
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_At_XYZ(kActorClovis, 84.85f, -50.56f, -68.87f, 800);
#else
			// same as kGoalClovisKP07LayDown
			// Actor_Set_Targetable(kActorClovis, true) is already done above
			Game_Flag_Set(kFlagClovisLyingDown);
			// prevent Clovis rotating while lying on the bed when McCoy enters KP07
			Actor_Set_At_XYZ(kActorClovis, 84.85f, -50.56f, -68.87f, 1022);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Face_Heading(kActorClovis, 1022, false);
		}
		someAnim();
		return true;

	case kGoalClovisKP07TalkToMcCoy:
		Actor_Says(kActorMcCoy, 2345, 16);
		Actor_Says(kActorClovis, 170, -1);
		Actor_Says(kActorClovis, 180, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2350, 17);
		if (!Game_Flag_Query(kFlagMcCoyAttackedReplicants)) {
			Actor_Says(kActorMcCoy, 2355, 11);
		}
		Actor_Says(kActorClovis, 190, -1);
		Actor_Says(kActorClovis, 200, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 2360, 18);
		Actor_Says(kActorClovis, 210, kAnimationModeTalk);
		Actor_Says(kActorClovis, 220, -1);
		Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP07SayFinalWords);
		return true;

	case kGoalClovisKP07SayFinalWords:
		ADQ_Add(kActorClovis, 240, -1);
		ADQ_Add(kActorClovis, 250, -1);
		ADQ_Add(kActorClovis, 260, -1);
		ADQ_Add(kActorClovis, 270, -1);
		ADQ_Add_Pause(1000);
		ADQ_Add(kActorClovis, 280, -1);
		ADQ_Add(kActorClovis, 290, -1);
		ADQ_Add(kActorClovis, 300, -1);
		return true;

	case kGoalClovisKP07FlyAway:
		Actor_Says(kActorMcCoy, 8501, kAnimationModeTalk);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Face_Actor(kActorClovis, kActorMcCoy, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorClovis, 1260, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 8502, kAnimationModeTalk);
		Actor_Says(kActorClovis, 1270, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 8504, kAnimationModeTalk);
		Actor_Says(kActorClovis, 1290, kAnimationModeTalk);
		Actor_Says(kActorMcCoy, 8505, kAnimationModeTalk);
		Actor_Says(kActorClovis, 1300, kAnimationModeTalk);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		Actor_Face_Heading(kActorClovis, 780, true);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Says(kActorClovis, 1310, kAnimationModeTalk);
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Outtake_Play(kOuttakeEnd4A, false, -1);
		if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy
		 && Game_Flag_Query(kFlagLucyIsReplicant)
		) {
			Outtake_Play(kOuttakeEnd4B, false, -1);
		} else if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora
		       && Game_Flag_Query(kFlagDektoraIsReplicant)
		) {
			Outtake_Play(kOuttakeEnd4C, false, -1);
		}
		Outtake_Play(kOuttakeEnd4D, false, -1);
		Game_Over();
		return true;

	case kGoalClovisKP07ReplicantsAttackMcCoy:
		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorLucy, kSetKP07)
		) {
#if BLADERUNNER_ORIGINAL_BUGS
			// Lucy's retirement on the moonbus should be handled in her ai script AIScriptLucy::Retired()
			// like the others - even if she won't attack McCoy, she should be retired immediately (with one shot)
			Actor_Set_Goal_Number(kActorLucy, kGoalLucyGone);
			Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1);
#else
			// This is her code if she's attacked when escaping with McCoy
			// will this work?
			Non_Player_Actor_Combat_Mode_On(kActorLucy, kActorCombatStateIdle, false, kActorMcCoy, 4, kAnimationModeIdle, kAnimationModeWalk, kAnimationModeRun, -1, 0, 0, 10, 300, false);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorLuther, kSetKP07)
		) {
#if BLADERUNNER_ORIGINAL_BUGS
			// Luther's retirement on the moonbus should be handled in his ai script AIScriptLuther:Retired()
			// like the others - even if he won't attack McCoy, he should be retired immediately (with one shot)
			Actor_Set_Goal_Number(kActorLuther, kGoalLutherGone);
			Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1);
#endif // BLADERUNNER_ORIGINAL_BUGS
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorDektora, kSetKP07)
		) {
			Non_Player_Actor_Combat_Mode_On(kActorDektora, kActorCombatStateIdle, false, kActorMcCoy, 19, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorZuben, kSetKP07)
		) {
			Non_Player_Actor_Combat_Mode_On(kActorZuben, kActorCombatStateIdle, false, kActorMcCoy, 19, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorSadik, kSetKP07)
		) {
			Non_Player_Actor_Combat_Mode_On(kActorSadik, kActorCombatStateIdle, true, kActorMcCoy, 19, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorIzo, kSetKP07)
		) {
			Non_Player_Actor_Combat_Mode_On(kActorIzo, kActorCombatStateIdle, false, kActorMcCoy, 19, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorGordo, kSetKP07)
		) {
			Non_Player_Actor_Combat_Mode_On(kActorGordo, kActorCombatStateIdle, true, kActorMcCoy, 19, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
		}

		if (Global_Variable_Query(kVariableChapter) == 5
		 && Actor_Query_In_Set(kActorClovis, kSetKP07)
		) {
			Non_Player_Actor_Combat_Mode_On(kActorClovis, kActorCombatStateIdle, false, kActorMcCoy, 19, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 10, 300, false);
		}
		return true;

	case kGoalClovisKP07LayDown:
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_At_XYZ(kActorClovis, 84.85f, -50.56f, -68.87f, 800);
#else
		// prevent Clovis rotating while lying on the bed when McCoy enters KP07
		Actor_Set_At_XYZ(kActorClovis, 84.85f, -50.56f, -68.87f, 1022);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Actor_Face_Heading(kActorClovis, 1022, false);
		Actor_Set_Targetable(kActorClovis, true);
		Game_Flag_Set(kFlagClovisLyingDown);
		someAnim();
		return true;

	default:
		break;
	}
	return false;
}

bool AIScriptClovis::UpdateAnimation(int *animation, int *frame) {
	bool flag;

	switch (_animationState) {
	case 0:
		if (_var1 == 1) {
			*animation = kModelAnimationGlovisIdle;
			if (_var2) {
				--_var2;
			} else if (++_animationFrame == 7) {
				_var2 = Random_Query(5, 15);
			} else {
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisIdle)) {
					_animationFrame = 0;
					_var1 = 0;
				}
			}
		} else if (_var1 == 0) {
			*animation = kModelAnimationGlovisLookingUpAndAbout;
			if (_var2) {
				_animationFrame += _var3;
				if (_animationFrame > _var5) {
					_animationFrame = _var5;
					_var3 = -1;
				} else {
					if (_animationFrame < _var4) {
						_animationFrame = _var4;
						_var3 = 1;
					}
				}
				--_var2;
			} else {
				_animationFrame += _var3;
				if (_animationFrame == 13 && Random_Query(0, 1)) {
					_var3 = -1;
					_var4 = 9;
					_var5 = 13;
					_var2 = Random_Query(4, 15);
				} else {
					if (_animationFrame == 6) {
						if (Random_Query(0, 1)) {
							_var3 = -1;
							_var4 = 3;
							_var5 = 6;
							_var2 = Random_Query(4, 15);
						}
					}
				}
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLookingUpAndAbout)) {
					_animationFrame = 0;
				} else {
					if (_animationFrame < 0) {
						_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLookingUpAndAbout) - 1;
					}
				}
				if (!_animationFrame) {
					if (Random_Query(0, 1) > 0) {
						_var1 = 1;
					} else {
						if (Random_Query(0, 2) > 0) {
							_var3 = 1;
							_var4 = 0;
							_var5 = 0;
							_var2 = Random_Query(5, 15);
						}
					}
				}
			}
		}
		break;

	case 1:
		if (_var1 == 0) {
			*animation = kModelAnimationGlovisLookingUpAndAbout;
		}
		if (_var1 == 1) {
			*animation = kModelAnimationGlovisIdle;
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame += 3;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
				*animation = _animationNext;
				_animationState = _animationStateNext;
			}
		} else {
			_animationFrame -= 3;
			if (_animationFrame <= 0) {
				_animationFrame = 0;
				*animation = _animationNext;
				_animationState = _animationStateNext;
			}
		}
		break;

	case 2:
		*animation = kModelAnimationGlovisKneelingChecking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisKneelingChecking)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisKneelingChecking) - 1;
			}
		}
		break;

	case 3:
		*animation = kModelAnimationGlovisStandingToKneeling;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisStandingToKneeling)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisStandingToKneeling) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationGlovisKneelingChecking;
			if (Actor_Query_Goal_Number(kActorClovis) == kGoalClovisBB11TalkWithSadik) {
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisBB11PrepareTalkToMcCoy);
			}
		}
		break;

	case 4:
		*animation = kModelAnimationGlovisKneelingToStanding;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisKneelingToStanding)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisKneelingToStanding) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelAnimationGlovisIdle;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeIdle);
		}
		break;

	case 5:
		*animation = kModelAnimationGlovisCalmTalk;
		if (!_animationFrame && _flag) {
			_animationState = 0;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCalmTalk)) {
				_animationFrame = 0;
			}
		}
		break;

	case 6:
		*animation = kModelAnimationGlovisSuggestingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisSuggestingTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationGlovisCalmTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationGlovisSuggestingAndBeardScratchTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisSuggestingAndBeardScratchTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationGlovisCalmTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationGlovisAffirmingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisAffirmingTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationGlovisCalmTalk;
		}
		break;

	case 9:
		*animation = kModelAnimationGlovisHandCircularMoveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisHandCircularMoveTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationGlovisCalmTalk;
		}
		break;

	case 10:
		*animation = kModelAnimationGlovisPointingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisPointingTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationGlovisCalmTalk;
		}
		break;

	case 11:
		*animation = kModelAnimationGlovisNegotiatingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisNegotiatingTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationGlovisCalmTalk;
		}
		break;

	case 12:
		*animation = kModelAnimationGlovisKneelingTalking;
		if (!_animationFrame && _flag) {
			_animationState = 2;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeSit);
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisKneelingTalking)) {
				_animationFrame = 0;
			}
		}
		break;

	case 13:
		*animation = kModelAnimationGlovisCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatIdle)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatIdle) - 1;
			}
		}
		break;

	case 14:
		*animation = kModelAnimationGlovisCombatAssumePosition;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatAssumePosition)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatAssumePosition) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelAnimationGlovisCombatIdle;
			_animationState = 13;
		}
		break;

	case 15:
		*animation = kModelAnimationGlovisCombatResumeNonCombat;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatResumeNonCombat)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatResumeNonCombat) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelAnimationGlovisIdle;
			_animationState = 0;
		}
		break;

	case 16:
		*animation = kModelAnimationGlovisCombatPunchAttack;
		++_animationFrame;
		if (_animationFrame == 2) {
			int snd;

			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorClovis, snd, 75, 0, 99);
		}
		if (_animationFrame == 4) {
			Actor_Combat_AI_Hit_Attempt(kActorClovis);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			_animationState = 13;
			*animation = kModelAnimationGlovisCombatIdle;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeCombatIdle);
		}
		break;

	case 17:
		*animation = kModelAnimationGlovisCombatGotHitRight;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatGotHitRight)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatGotHitRight) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			_animationState = 13;
			*animation = kModelAnimationGlovisCombatIdle;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeCombatIdle);
		}
		break;

	case 18:
		*animation = kModelAnimationGlovisCombatGotHitLeft;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatGotHitLeft)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatGotHitLeft) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			_animationState = 13;
			*animation = kModelAnimationGlovisCombatIdle;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeCombatIdle);
		}
		break;

	case 19:
		*animation = kModelAnimationGlovisGotHitRight;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisGotHitRight)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisGotHitRight) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelAnimationGlovisIdle;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeIdle);
		}
		break;

	case 20:
		*animation = kModelAnimationGlovisGotHitLeft;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisGotHitLeft)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisGotHitLeft) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelAnimationGlovisIdle;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeIdle);
		}
		break;

	case 21:
		*animation = kModelAnimationGlovisWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisWalking)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisWalking) - 1;
			}
		}
		break;

	case 22:
		*animation = kModelAnimationGlovisRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisRunning)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisRunning) - 1;
			}
		}
		break;

	case 23:
		*animation = kModelAnimationGlovisCombatWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatWalking)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatWalking) - 1;
			}
		}
		break;

	case 24:
		*animation = kModelAnimationGlovisCombatAssumePosition;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatAssumePosition)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatAssumePosition) - 1;
			}
		}
		break;

	case 25:
		*animation = kModelAnimationGlovisClimbStairsUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisClimbStairsUp)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisClimbStairsUp) - 1;
			}
		}
		break;

	case 26:
		*animation = kModelAnimationGlovisClimbStairsDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisClimbStairsDown)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisClimbStairsDown) - 1;
			}
		}
		break;

	case 27:
		*animation = kModelAnimationGlovisCombatClimbStairsUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatClimbStairsUp)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatClimbStairsUp) - 1;
			}
		}
		break;

	case 28:
		*animation = kModelAnimationGlovisCombatClimbStairsDown;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatClimbStairsDown)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatClimbStairsDown) - 1;
			}
		}
		break;

	case 29:
		*animation = kModelAnimationGlovisCombatStrafeSlow;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatStrafeSlow)) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatStrafeSlow) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			_animationState = 13;
			*animation = kModelAnimationGlovisCombatIdle;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeCombatIdle);
		}
		break;

	case 30:
		*animation = kModelAnimationGlovisCombatStrafeFast;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatStrafeFast)) {
			flag = 1;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisCombatStrafeFast) - 1;
				flag = 1;
			} else {
				flag = 0;
			}
		}
		if (flag) {
			_animationState = 13;
			*animation = kModelAnimationGlovisCombatIdle;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeCombatIdle);
		}
		break;

	case 31:
		*animation = kModelAnimationGlovisJumpingDodging;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisJumpingDodging)) {
			flag = 1;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisJumpingDodging) - 1;
				flag = 1;
			} else {
				flag = 0;
			}
		}
		if (flag) {
			*animation = kModelAnimationGlovisIdle;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeIdle);
		}
		break;

	case 32:
		*animation = kModelAnimationGlovisLayingWithBookIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookIdle)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookIdle) - 1;
			}
		}
		break;

	case 33:
		*animation = kModelAnimationGlovisLayingWithBookStopsReadingTalk;
		if (_animationFrame == 0
		 && _flag
		) {
			_animationState = 32;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorClovis, 54);
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookStopsReadingTalk)) {
				_animationFrame = 0;
			} else {
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookStopsReadingTalk) - 1;
				}
			}
		}
		break;

	case 34:
		*animation = kModelAnimationGlovisLayingWithBookStillReadingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookStillReadingTalk)) {
			_animationState = 33;
			_animationFrame = 0;
			*animation = kModelAnimationGlovisLayingWithBookStopsReadingTalk;
			Actor_Change_Animation_Mode(kActorClovis, 54);
		}
		break;

	case 35:
		++_animationFrame;
		*animation = kModelAnimationGlovisLayingWithBookReadingOutLoud;
		Actor_Change_Animation_Mode(kActorClovis, 54);
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookReadingOutLoud)) {
			_animationFrame = 0;
			_animationState = 33;
			*animation = kModelAnimationGlovisLayingWithBookStopsReadingTalk;
		}
		break;

	case 36:
		*animation = kModelAnimationGlovisLayingWithBookGotHitOrViolentCough;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookGotHitOrViolentCough)) {
			_animationFrame = 0;
			_animationState = 37;
			*animation = kModelAnimationGlovisLayingWithBookDyingDropBook;
		}
		break;

	case 37:
		*animation = kModelAnimationGlovisLayingWithBookDyingDropBook;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookDyingDropBook) - 1) {
			++_animationFrame;
		}
		break;

	case 38:
		*animation = kModelAnimationGlovisLayingWithBookUnholsterGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookUnholsterGun)) {
			_animationFrame = 0;
			_animationState = 39;
			*animation = kModelAnimationGlovisLayingWithBookPointingGun;
		}
		break;

	case 39:
		*animation = kModelAnimationGlovisLayingWithBookPointingGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookPointingGun)) {
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookPointingGun) - 1;
			}
		}
		break;

	case 40:
		*animation = kModelAnimationGlovisLayingWithBookDyingDropGun;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisLayingWithBookDyingDropGun) - 1) {
			++_animationFrame;
		}
		break;

	case 41:
		*animation = kModelAnimationGlovisShotDead;
		++_animationFrame;
		if (_animationFrame == Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisShotDead) - 1) {
			Actor_Change_Animation_Mode(kActorClovis, 88);
			_animationState = 42;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisShotDead) - 1;
		}
		break;

	case 42:
		*animation = kModelAnimationGlovisShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisShotDead) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptClovis::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (Game_Flag_Query(kFlagClovisLyingDown)) {
			_animationState = 32;
			_animationFrame = 0;
			break;
		}

		switch (_animationState) {
		case 2:
			Actor_Change_Animation_Mode(kActorClovis, kAnimationModeSit);
			break;
		case 4:
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			_flag = 1;
			break;
		case 12:
			_flag = 1;
			break;
		case 13:
			_animationState = 15;
			_animationFrame = 0;
			break;
		default:
			_animationState = 0;
			_animationFrame = 0;
			_var1 = 0;
			break;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 21;
		_animationFrame = 0;
		break;

	case kAnimationModeRun:
		_animationState = 22;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
	case 9:
		if (Game_Flag_Query(kFlagClovisLyingDown)) {
			_animationFrame = 0;
			_animationState = 34;
		} else {
			if (_animationState < 5 || _animationState > 11) {
				_animationState = 1;
				_animationStateNext = 5;
				_animationNext = kModelAnimationGlovisCalmTalk;
				_flag = 0;
			}
		}
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 13:
		case 14:
			break;
		case 23:
		case 24:
		case 27:
		case 28:
			_animationState = 13;
			_animationFrame = 0;
			break;
		default:
			_animationState = 1;
			_animationStateNext = 14;
			_animationNext = kModelAnimationGlovisCombatAssumePosition;
			break;
		}
		break;

	case kAnimationModeCombatAttack:
		_animationState = 16;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalk:
		_animationState = 21;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatRun:
		_animationState = 22;
		_animationFrame = 0;
		break;

	case 10:
	case 12:
		if (_animationState < 5 || _animationState > 11) {
			_animationState = 1;
			_animationStateNext = 6;
			_animationNext = kModelAnimationGlovisSuggestingTalk;
			_flag = 0;
		}
		break;

	case 11:
	case 14:
		if (_animationState < 5 || _animationState > 11) {
			_animationState = 1;
			_animationStateNext = 8;
			_animationNext = kModelAnimationGlovisAffirmingTalk;
			_flag = 0;
		}
		break;

	case 13:
		if (_animationState < 5 || _animationState > 11) {
			_animationState = 1;
			_animationStateNext = 7;
			_animationNext = kModelAnimationGlovisSuggestingAndBeardScratchTalk;
			_flag = 0;
		}
		break;

	case 15:
		if (_animationState < 5 || _animationState > 11) {
			_animationState = 1;
			_animationStateNext = 9;
			_animationNext = kModelAnimationGlovisHandCircularMoveTalk;
			_flag = 0;
		}
		break;

	case 16:
		if (_animationState < 5 || _animationState > 11) {
			_animationState = 1;
			_animationStateNext = 10;
			_animationNext = kModelAnimationGlovisPointingTalk;
			_flag = 0;
		}
		break;

	case 17:
		if (_animationState < 5 || _animationState > 11) {
			_animationState = 1;
			_animationStateNext = 11;
			_animationNext = kModelAnimationGlovisNegotiatingTalk;
			_flag = 0;
		}
		break;

	case 20:
		_animationState = 31;
		_animationFrame = 0;
		break;

	case kAnimationModeHit:
		if ((unsigned int)(_animationState - 13) > 3) {
			if ((unsigned int)(_animationState - 32) > 8) {
				if (Random_Query(0, 1)) {
					_animationState = 20;
				} else {
					_animationState = 19;
				}
				_animationFrame = 0;
			} else {
				_animationState = 40;
				_animationFrame = 0;
			}
		} else {
			if (Random_Query(0, 1)) {
				_animationState = 18;
			} else {
				_animationState = 17;
			}
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatHit:
		if (Random_Query(0, 1)) {
			_animationState = 17;
		} else {
			_animationState = 18;
		}
		_animationFrame = 0;
		break;

	case 29:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case 30:
		_animationState = 12;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 88:
		_animationState = 42;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationGlovisShotDead) - 1;
		break;

	case kAnimationModeDie:
		_animationState = 41;
		_animationFrame = 0;
		break;

	case kAnimationModeSit:
		switch (_animationState) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// Sometimes the animationState will be 0 here (seems to happen randomly if skipping fast through the dialogue)
		// and this would cause Clovis to not switch to his sitting animation
		// and thus the BB11 rooftop scene would get stuck there
		case 0:
			// fall through
#endif // BLADERUNNER_ORIGINAL_BUGS
		case 4:
			// fall through
		case 5:
			// fall through
		case 6:
			// fall through
		case 7:
			// fall through
		case 8:
			// fall through
		case 9:
			// fall through
		case 10:
			// fall through
		case 11:
			_animationState = 3;
			_animationFrame = 0;
			break;
		default:
			break;
		}
		break;

	case 54:
		_animationFrame = 0;
		_animationState = 32;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptClovis::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptClovis::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptClovis::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptClovis::FledCombat() {
	// return false;
}

void AIScriptClovis::shotAnim() {
	if ((unsigned int)(_animationState - 13) > 3) {
		if ((unsigned int)(_animationState - 32) <= 8) {
			_animationState = 40;
			_animationFrame = 0;
		} else {
			if (Random_Query(0, 1)) {
				_animationState = 20;
			} else {
				_animationState = 19;
			}
			_animationFrame = 0;
		}
	} else {
		if (Random_Query(0, 1)) {
			_animationState = 18;
		} else {
			_animationState = 17;
		}
		_animationFrame = 0;
	}
}

void AIScriptClovis::someAnim() {
	if (Game_Flag_Query(kFlagClovisLyingDown)) {
		_animationState = 32;
		_animationFrame = 0;
		return;
	}

	switch (_animationState) {
	case 2:
		Actor_Change_Animation_Mode(kActorClovis, kAnimationModeSit);
		break;
	case 4:
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
		_flag = 1;
		break;
	case 12:
		_flag = 1;
		break;
	case 13:
		_animationState = 15;
		_animationFrame = 0;
		break;
	default:
		_animationState = 0;
		_animationFrame = 0;
		_var1 = 0;
		break;
	}
}

} // End of namespace BladeRunner
