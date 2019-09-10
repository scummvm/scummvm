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

AIScriptDektora::AIScriptDektora(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
	_x = _y = _z = 0.0f;
}

void AIScriptDektora::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;
	_x = _y = _z = 0.0f;

	Actor_Set_Goal_Number(kActorDektora, kGoalDektoraDefault);
}

bool AIScriptDektora::Update() {
	int chapter = Global_Variable_Query(kVariableChapter);

	if (chapter == 2) {
		if (!Game_Flag_Query(kFlagDektoraChapter2Started)) {
			Game_Flag_Set(kFlagDektoraChapter2Started);
			Actor_Put_In_Set(kActorDektora, kSetFreeSlotG);
			Actor_Set_At_Waypoint(kActorDektora, 39, 0);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
		} else {
			if ( Game_Flag_Query(kFlagAR02DektoraWillBuyScorpions)
			 && !Game_Flag_Query(kFlagAR02DektoraBoughtScorpions)
			 &&  Player_Query_Current_Scene() != kSceneAR01
			 &&  Player_Query_Current_Scene() != kSceneAR02
			) {
#if BLADERUNNER_ORIGINAL_BUGS
				if (Game_Flag_Query(kFlagAR02Entered)) {
					Item_Remove_From_World(kItemScorpions);
				}
				Game_Flag_Set(kFlagAR02DektoraBoughtScorpions);
#else
				if (Game_Flag_Query(kFlagScorpionsInAR02)) {
					Item_Remove_From_World(kItemScorpions);
					Game_Flag_Reset(kFlagScorpionsInAR02);
				}
				Game_Flag_Set(kFlagAR02DektoraBoughtScorpions);
#endif
			}
		}
		return true;
	}

	if (chapter == 3) {
		if (Actor_Query_Goal_Number(kActorDektora) < kGoalDektoraStartChapter3) {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartChapter3);
		} else {
			switch (Actor_Query_Goal_Number(kActorDektora)) {
			case kGoalDektoraNR11PrepareBurning:
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11Burning);
				break;

			case kGoalDektoraNR11BurningGoToMcCoy:
#if BLADERUNNER_ORIGINAL_BUGS
				if (Actor_Query_Inch_Distance_From_Actor(kActorDektora, kActorMcCoy) <= 48) {
					Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11BurningFallToNR10);
					break;
				}
#else
				if (Actor_Query_Inch_Distance_From_Actor(kActorDektora, kActorMcCoy) <= 54) {
					Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11BurningFallToNR10);
					break;
				}
#endif // BLADERUNNER_ORIGINAL_BUGS

				if (comp_distance(kActorMcCoy, _x, _y, _z) > 12.0f) {
					Actor_Query_XYZ(kActorMcCoy, &_x, &_y, &_z);
					Async_Actor_Walk_To_XYZ(kActorDektora, _x, _y, _z, 36, false);
				}
				break;

			case kGoalDektoraNR11PrepareFallThroughWindow:
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11FallThroughWindow);
				break;

			// this is unused and goal 275 never set
			case 275:
				Actor_Set_Goal_Number(kActorDektora, 276);
				break;
			}
		}
		return true;
	}

	if (chapter == 4) {
		if (Actor_Query_Goal_Number(kActorDektora) < 300) {
			Actor_Set_Goal_Number(kActorDektora, 300);
		}
		return true;
	}

	return true;
}

void AIScriptDektora::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08Dance) {
			if (Player_Query_Current_Scene() == kSceneNR08) {
				AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask0);
				AI_Countdown_Timer_Start(kActorDektora, kActorTimerAIScriptCustomTask0, 10);
			} else {
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR08Leave);
				AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask0);
			}
			return; //true;
		}

		if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11Burning) {
			AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask0);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11BurningGoToMcCoy);
			return; //true;
		}
	}

	return; //false;
}

void AIScriptDektora::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorDektora)) {
	case kGoalDektoraStartWalkingAround:
		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraWalkAroundAsReplicant);
		} else {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraWalkAroundAsHuman);
		}
		break;

	case kGoalDektoraWalkAroundAsReplicant:
		if (Random_Query(1, 7) == 1
		 && Actor_Query_Goal_Number(kActorEarlyQ) != 1
		 && Actor_Query_Goal_Number(kActorEarlyQ) != 101
		) {
			Game_Flag_Set(kFlagAR02DektoraWillBuyScorpions);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
		} else {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
		}
		break;

	case kGoalDektoraWalkAroundAsHuman:
		if (Random_Query(1, 5) == 1
		 && Actor_Query_Goal_Number(kActorEarlyQ) != 1
		 && Actor_Query_Goal_Number(kActorEarlyQ) != 101
		) {
			Game_Flag_Set(kFlagAR02DektoraWillBuyScorpions);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
		} else {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraStartWalkingAround);
		}
		break;

	case kGoalDektoraNR11WalkAway:
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11RanAway);
		break;

	case kGoalDektoraNR11BurningGoToWindow:
		AI_Movement_Track_Flush(kActorDektora);
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Face_Heading(kActorDektora, 0, 0);
#else
		// better positioning to lessen the bad overlap with the wall
		// There is still some glitch there but it's not as bad as originally
		Actor_Set_At_XYZ(kActorDektora, -100.0f, 10.33f, -312.0f, 220);
#endif // BLADERUNNER_ORIGINAL_BUGS
		Sound_Play(kSfxDEKGLAS1, 71, 0, 0, 50);

		_animationState = 35;
		_animationFrame = 0;
		break;

	default:
		return; //false;

	}

	return; //true;
}

void AIScriptDektora::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptDektora::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraGone) {
		Actor_Face_Actor(0, kActorDektora, true);
		Actor_Says(kActorMcCoy, 8630, 12);  // What a waste

		return; //true;
	}

	if (Actor_Query_Goal_Number(kActorDektora) < kGoalDektoraStartChapter3) {
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		Actor_Says(kActorMcCoy, 8590, 13);  // Not the talkative type
	}

	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08Dance) {
		Game_Flag_Set(kFlagNR08TouchedDektora);
		AI_Movement_Track_Flush(kActorHanoi);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Player_Loses_Control();
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiThrowOutMcCoy);

		return; //true;
	}

	return; //false;
}

void AIScriptDektora::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptDektora::OtherAgentEnteredThisScene(int otherActorId) {
	if (otherActorId == kActorMcCoy
	 && Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR08GoToNR10
	) {
		Scene_Exits_Disable();
		Actor_Change_Animation_Mode(kActorDektora, 23);
	}

	// return false;
}

void AIScriptDektora::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptDektora::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptDektora::ShotAtAndMissed() {
	checkCombat();
	// return false;
}

bool AIScriptDektora::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11Burning
	 || Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11BurningGoToMcCoy
	 || Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11BurningGoToWindow
	) {
		Actor_Set_Health(kActorDektora, 100, 100);
		if (_vm->_cutContent) {
		// add hit sounds with small probability
		switch (Random_Query(1, 10)) {
			case 1:
				Sound_Play_Speech_Line(kActorDektora, 9000, 65, 0, 99);
				break;
			case 2:
				Sound_Play_Speech_Line(kActorDektora, 9005, 65, 0, 99);
				break;
			default:
				break;
			}
		}
		if (Actor_Query_Goal_Number(kActorDektora) != kGoalDektoraNR11BurningGoToWindow) {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11BurningGoToWindow);
		}
		return true;
	}

	if (Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR11WalkAway) {
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Set_Health(kActorDektora, 0, 100);

		if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeDie);
			Actor_Start_Speech_Sample(kActorDektora, 980);
			Delay(2000);
			Actor_Set_Goal_Number(kActorSteele, kGoalSteeleNR11Enter);
		} else {
			// Does this case ever happen?
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeDie);
			if (_vm->_cutContent) {
				Sound_Play_Speech_Line(kActorDektora, 9020, 60, 0, 99); // add Dektora's death rattle here
			}
			Delay(2000);
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
		}

		return true;
	}

	checkCombat();

	return false;
}

void AIScriptDektora::Retired(int byActorId) {
	if (byActorId == kActorMcCoy) {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, -5);
	}

	if (byActorId == kActorSteele
	 && Actor_Query_In_Set(kActorSteele, kSetHF06)
	 && Actor_Query_In_Set(kActorMcCoy, kSetHF06)
	) {
		Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, kActorMcCoy, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}

	if (Actor_Query_In_Set(kActorDektora, kSetKP07)) {
		Global_Variable_Decrement(kVariableReplicantsSurvivorsAtMoonbus, 1);
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraGone);

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

	if (Actor_Query_Goal_Number(kActorDektora) != kGoalDektoraNR11FallThroughWindow) {
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraGone);
	}
	return; //false;

	// return false;
}

int AIScriptDektora::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptDektora::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalDektoraStartWalkingAround:
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append(kActorDektora, 39, 10);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case kGoalDektoraWalkAroundAsReplicant:
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append_With_Facing(kActorDektora, 287, 15, 278);

		if (Game_Flag_Query(kFlagZubenRetired)) {
			AI_Movement_Track_Append(kActorDektora, 33, 240);
		} else if (Game_Flag_Query(kFlagZubenSpared)) {
			AI_Movement_Track_Append(kActorDektora, 33, 120);
		} else {
			AI_Movement_Track_Append(kActorDektora, 33, 90);
		}

		AI_Movement_Track_Append_With_Facing(kActorDektora, 288, 35, 528);

		if (Random_Query(1, 2) == 1
		 && Game_Flag_Query(kFlagAR02DektoraWillBuyScorpions)
		) {
			AI_Movement_Track_Append(kActorDektora, 289, 0);
			AI_Movement_Track_Append_With_Facing(kActorDektora, 290, 2, 979);
			AI_Movement_Track_Append(kActorDektora, 289, 0);
			AI_Movement_Track_Append(kActorDektora, 39, 120);
		} else {
			AI_Movement_Track_Append(kActorDektora, 39, 180);
		}

		AI_Movement_Track_Append(kActorDektora, 282, 0);
		AI_Movement_Track_Append(kActorDektora, 283, 0);
		AI_Movement_Track_Append(kActorDektora, 284, 0);
		AI_Movement_Track_Append(kActorDektora, 285, 0);
		AI_Movement_Track_Append_With_Facing(kActorDektora, 286, 30, 329);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case kGoalDektoraWalkAroundAsHuman:
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append_With_Facing(kActorDektora, 287, 15, 278);
		AI_Movement_Track_Append(kActorDektora, 40, 90);

		if (Game_Flag_Query(kFlagZubenRetired)
		 && Game_Flag_Query(kFlagLucyIsReplicant)
		) {
			AI_Movement_Track_Append(kActorDektora, 33, 180);
		} else {
			AI_Movement_Track_Append(kActorDektora, 33, 160);
		}

		AI_Movement_Track_Append_With_Facing(kActorDektora, 288, 20, 528);

		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append(kActorDektora, 289, 0);
			AI_Movement_Track_Append_With_Facing(kActorDektora, 290, 3, 979);
			AI_Movement_Track_Append(kActorDektora, 289, 0);
			AI_Movement_Track_Append(kActorDektora, 39, 120);
		} else {
			AI_Movement_Track_Append(kActorDektora, 39, 180);
		}

		AI_Movement_Track_Append(kActorDektora, 282, 0);
		AI_Movement_Track_Append(kActorDektora, 283, 0);
		AI_Movement_Track_Append(kActorDektora, 284, 0);
		AI_Movement_Track_Append(kActorDektora, 285, 0);
		AI_Movement_Track_Append_With_Facing(kActorDektora, 286, 35, 329);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case kGoalDektoraStopWalkingAround:
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append(kActorDektora, 39, 240);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case kGoalDektoraStartChapter3:
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR07Sit);
		break;

	case kGoalDektoraNR07Sit:
		Actor_Put_In_Set(kActorDektora, kSetNR07);
		Actor_Set_At_XYZ(kActorDektora, -136.0f, -75.0f, 14.0f, 300);
		Actor_Change_Animation_Mode(kActorDektora, kAnimationModeSit);
		break;

	case kGoalDektoraNR08Dance:
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Put_In_Set(kActorDektora, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorDektora, -923.93f, 127.85f, 413.46f, 30);
		AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorDektora, kActorTimerAIScriptCustomTask0, 45);
		break;

	case kGoalDektoraNR08Leave:
		if (Player_Query_Current_Scene() == kSceneNR08) {
			Game_Flag_Set(kFlagNR08McCoyWatchingShow);
		} else {
			Game_Flag_Set(kFlagNR08Faded);
			Actor_Put_In_Set(kActorDektora, kSetFreeSlotA);
			Actor_Set_At_Waypoint(kActorDektora, 33, 0);
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR07Sit);
		}

		if (Player_Query_Current_Scene() == kSceneNR08) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR08Leave);
		} else {
			Game_Flag_Reset(kFlagNR08McCoyWatchingShow);
		}
		break;

	case kGoalDektoraNR08ReadyToRun:
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Put_In_Set(kActorDektora, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorDektora, -1558.41f, 0.32f, 319.48f, 264);
#else
		// better positioning to remove bad overlap with walls
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Put_In_Set(kActorDektora, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorDektora, -1633.27f, 0.32f, 353.00f, 831);
#endif // BLADERUNNER_ORIGINAL_BUGS
		break;

	case kGoalDektoraNR08GoToNR10:
		Actor_Put_In_Set(kActorDektora, kSetNR10);
		Actor_Set_At_XYZ(kActorDektora, 19.22f, 2.84f, -122.43f, 768);
		break;

	case kGoalDektoraNR10AttackMcCoy:
		Actor_Set_At_XYZ(kActorDektora, -99.0f, 2.88f, -202.0f, 911);
		Actor_Set_Invisible(kActorDektora, false);
		Actor_Change_Animation_Mode(kActorDektora, 70);
		Actor_Retired_Here(kActorMcCoy, 12, 12, true, -1);
		break;

	case kGoalDektoraNR11Hiding:
		AI_Movement_Track_Flush(kActorDektora);
		AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask1);
		AI_Countdown_Timer_Reset(kActorDektora, kActorTimerAIScriptCustomTask2);
		Actor_Put_In_Set(kActorDektora, kSetNR11);
		Actor_Set_At_XYZ(kActorDektora, -184.0f, 0.33f, -268.0f, 256);
		break;

	case kGoalDektoraNR11WalkAway:
		Actor_Set_Targetable(kActorDektora, true);
		Actor_Force_Stop_Walking(kActorDektora);
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append(kActorDektora, 462, 0);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case kGoalDektoraNR11PrepareBurning:
		// fall through
	case kGoalDektoraNR11BurningGoToMcCoy:
		// fall through
	case kGoalDektoraGone:
		break; // return true

	case kGoalDektoraNR11Burning:
		Game_Flag_Set(kFlagNR11DektoraBurning);
		Actor_Set_Targetable(kActorDektora, true);
		Loop_Actor_Walk_To_XYZ(kActorDektora, -135.0f, 0.33f, -267.0f, 0, false, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		if (Actor_Query_Goal_Number(kActorSteele) == kGoalSteeleNR11StopShooting) {
			Actor_Face_Actor(kActorSteele, kActorDektora, true);
			Actor_Change_Animation_Mode(kActorSteele, kAnimationModeCombatAttack);
			Delay(250);
			Sound_Play(kSfxFEMHURT1, 100, 0, 0, 50);
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11BurningGoToWindow);
		} else {
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11BurningGoToMcCoy);
		}
		break;

	case kGoalDektoraNR11BurningGoToWindow:
		Actor_Force_Stop_Walking(kActorDektora);
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append(kActorDektora, 456, 0);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case kGoalDektoraNR11FallThroughWindow:
		Player_Loses_Control();
		Scene_Exits_Enable();
		Game_Flag_Set(kFlagNR01DektoraFall);
		Actor_Put_In_Set(kActorDektora, kSetNR01);
		Actor_Set_At_XYZ(kActorDektora, -177.0f, 23.88f, -373.0f, 300);
		_animationState = 36;
		_animationFrame = 0;
		Set_Enter(kSetNR01, kSceneNR01);
		break;

	case kGoalDektoraNR11BurningFallToNR10:
		Actor_Force_Stop_Walking(kActorDektora);
		Actor_Put_In_Set(kActorDektora, kSetNR10);
		Actor_Set_At_XYZ(kActorDektora, 14.0f, 2.84f, -300.0f, 926);
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNR10Fall);
		_animationState = 36;
		_animationFrame = 0;
		break;

	case kGoalDektoraNR11RanAway:
		Game_Flag_Set(kFlagDektoraRanAway);
		break;

	case 299:
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorDektora, 41, 0);
		Actor_Set_Goal_Number(kActorDektora, kGoalDektoraGone);
		break;

	case 300:
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorDektora, 33, 0);
		break;

	default:
		return false;

	}

	return true;
}

bool AIScriptDektora::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationDektoraStandingIdle;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 19;
		_animationFrame = 0;
		break;

	case 2:
		*animation = kModelAnimationDektoraStandingNodShort;
		if (_animationFrame == 0
		 && _flag
		) {
			*animation = kModelAnimationDektoraStandingIdle;
			_animationState = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingNodShort)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = kModelAnimationDektoraStandingTalkAgreeing;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkAgreeing)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationDektoraStandingNodShort;
		}
		break;

	case 4:
		*animation = kModelAnimationDektoraStandingTalkGestureB;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkGestureB)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationDektoraStandingNodShort;
		}
		break;

	case 5:
		*animation = kModelAnimationDektoraStandingTalkGestureC;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkGestureC)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationDektoraStandingNodShort;
		}
		break;

	case 6:
		//  case 6 is identical to case 5
		*animation = kModelAnimationDektoraStandingTalkGestureC;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkGestureC)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationDektoraStandingNodShort;
		}
		break;

	case 7:
		*animation = kModelAnimationDektoraStandingTalkGestureD;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkGestureD)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationDektoraStandingNodShort;
		}
		break;

	case 8:
		//  case 8 is identical to case 7
		*animation = kModelAnimationDektoraStandingTalkGestureD;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkGestureD)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = kModelAnimationDektoraStandingNodShort;
		}
		break;

	case 9:
		*animation = kModelAnimationDektoraCombatIdle;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatIdle)) {
			_animationFrame = 0;
		}
		break;

	case 10:
		*animation = kModelAnimationDektoraCombatBegin;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatBegin)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
		}
		break;

	case 11:
		*animation = kModelAnimationDektoraCombatEnd;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatEnd)) {
			*animation = kModelAnimationDektoraStandingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 12:
		*animation = kModelAnimationDektoraCombatWalkingA;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatWalkingA)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
		}
		break;

	case 13:
		*animation = kModelAnimationDektoraCombatWalkingB;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatWalkingB)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
		}
		break;

	case 14:
		*animation = kModelAnimationDektoraCombatGotHitRight;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatGotHitRight)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
		}
		break;

	case 15:
		*animation = kModelAnimationDektoraCombatGotHitLeft;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatGotHitLeft)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
		}
		break;

	case 16:
		*animation = kModelAnimationDektoraCombatLegAttack;
		_animationFrame++;
		if (_animationFrame == 2) {
			int speech;

			if (Random_Query(1, 2) == 1) {
				speech = 9010;
			} else {
				speech = 9015;
			}
			Sound_Play_Speech_Line(kActorDektora, speech, 75, 0, 99);
		}

		if (_animationFrame == 5) {
			Actor_Combat_AI_Hit_Attempt(kActorDektora);
		}

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatLegAttack)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
		}
		break;

	case 17:
		*animation = kModelAnimationDektoraCombatPunchAttack;
		_animationFrame++;
		if (_animationFrame == 6
		 && Actor_Query_Goal_Number(kActorDektora) == kGoalDektoraNR10AttackMcCoy
		) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		}

		if (_animationFrame == 3) {
			int speech;

			if (Random_Query(1, 2) == 1) {
				speech = 9010;
			} else {
				speech = 9015;
			}
			Sound_Play_Speech_Line(kActorDektora, speech, 75, 0, 99);
		}

		if (_animationFrame == 6) {
			Actor_Combat_AI_Hit_Attempt(kActorDektora);
		}

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraCombatPunchAttack)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = kModelAnimationDektoraCombatIdle;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeCombatIdle);
		}
		break;

	case 18:
		*animation = kModelAnimationDektoraFrontShoveMove;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraFrontShoveMove)) {
			*animation = kModelAnimationDektoraStandingIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
		}
		break;

	case 19:
		*animation = kModelAnimationDektoraBackDodgeMove;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraBackDodgeMove)) {
			*animation = kModelAnimationDektoraStandingIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
		}
		break;

	case 20:
		*animation = kModelAnimationDektoraFallsDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraFallsDead) - 1) {
			_animationFrame++;
		}
		break;

	case 21:
		*animation = kModelAnimationDektoraWalking;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraWalking)) {
			_animationFrame = 0;
		}
		break;

	case 22:
		*animation = kModelAnimationDektoraRunning;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraRunning)) {
			_animationFrame = 0;
		}
		break;

	case 23:
		*animation = kModelAnimationDektoraClimbStairsUp;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraClimbStairsUp)) {
			_animationFrame = 0;
		}
		break;

	case 24:
		*animation = kModelAnimationDektoraClimbStairsDown;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraClimbStairsDown)) {
			_animationFrame = 0;
		}
		break;

	case 25:
		*animation = kModelAnimationDektoraSittingIdle;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingIdle)) {
			_animationFrame = 0;
		}
		break;

	case 26:
		*animation = kModelAnimationDektoraSittingShootingGun;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingShootingGun)) {
			_animationFrame = 0;
			_animationState = 31;
			*animation = kModelAnimationDektoraSittingHoldingGun;
		}
		break;

	case 27:
		*animation = kModelAnimationDektoraSittingSubtleTalking;
		if (!_animationFrame && _flag) {
			*animation = kModelAnimationDektoraSittingIdle;
			_animationState = 25;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingSubtleTalking)) {
				_animationFrame = 0;
			}
		}
		break;

	case 28:
		*animation = kModelAnimationDektoraSittingIntenseTalking;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingIntenseTalking)) {
			_animationFrame = 0;
			_animationState = 27;
			*animation = kModelAnimationDektoraSittingSubtleTalking;
		}
		break;

	case 29:
		*animation = kModelAnimationDektoraSittingPullingGunOut;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingPullingGunOut)) {
			_animationFrame = 0;
			_animationState = 31;
			*animation = kModelAnimationDektoraSittingHoldingGun;
		}
		break;

	case 30:
		*animation = kModelAnimationDektoraSittingPullingGunOut;
		_animationFrame--;
		if (_animationFrame == 0) {
			_animationFrame = 0;
			_animationState = 25;
			*animation = kModelAnimationDektoraSittingIdle;
		}
		break;

	case 31:
		*animation = kModelAnimationDektoraSittingHoldingGun;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingHoldingGun)) {
			_animationFrame = 0;
		}
		break;

	case 32:
		*animation = kModelAnimationDektoraInFlamesA;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraInFlamesA)) {
			_animationFrame = 0;
		}
		break;

	case 33:
		*animation = kModelAnimationDektoraInFlamesB;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraInFlamesB)) {
			_animationFrame = 0;
		}
		break;

	case 34:
		*animation = kModelAnimationDektoraInFlamesGotHit;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraInFlamesGotHit)) {
			_animationFrame = 0;
			_animationState = 32;
			*animation = kModelAnimationDektoraInFlamesA;
		}
		break;

	case 35:
		*animation = kModelAnimationDektoraInFlamesStartFalling;
		_animationFrame++;
		if (_animationFrame == 2) {
			Game_Flag_Set(kFlagNR11BreakWindow);
		}

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 36;
			*animation = kModelAnimationDektoraInFlamesEndFalling;
			Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR11PrepareFallThroughWindow);
		}
		break;

	case 36:
		*animation = kModelAnimationDektoraInFlamesEndFalling;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraInFlamesEndFalling) - 1) {
			_animationFrame++;
		}

		if (_animationFrame == 11) {
			Ambient_Sounds_Play_Sound(kSfxZUBLAND1, 80, -20, -20, 20);
		}
		break;

	case 37:
		*animation = kModelAnimationDektoraDancingA;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(kSfxWHISTLE1, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(kSfxWHISTLE2, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(kSfxWHISTLE3, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;
			}
		}

		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraDancingA) - 1) {
			_animationFrame = 0;
			_animationState = 38;
			*animation = kModelAnimationDektoraDancingB;
		}
		break;

	case 38:
		*animation = kModelAnimationDektoraDancingB;
		_animationFrame++;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(kSfxWHISTLE1, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(kSfxWHISTLE2, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(kSfxWHISTLE3, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;
			}
		}

		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			_animationFrame = 0;
			_animationState = 39;
			*animation = kModelAnimationDektoraDancingC;
		}
		break;

	case 39:
		*animation = kModelAnimationDektoraDancingC;
		_animationFrame++;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(kSfxWHISTLE1, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(kSfxWHISTLE2, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(kSfxWHISTLE3, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;
			}
		}

		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			_animationFrame = 0;
			_animationState = 40;
			*animation = kModelAnimationDektoraDancingFinale;
		}
		break;

	case 40:
		*animation = kModelAnimationDektoraDancingFinale;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(kSfxWHISTLE1, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(kSfxWHISTLE2, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(kSfxWHISTLE3, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;
			}
		}

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraDancingFinale) - 1) {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraDancingFinale) - 1) {
				Actor_Set_Goal_Number(kActorDektora, kGoalDektoraNR08Leave);
			}
		}
		break;

	case 41:
		*animation = kModelAnimationDektoraStandingTalkGestureA;
		_animationFrame++;
		if (_animationFrame == 8
		 && Actor_Query_In_Set(kActorDektora, kSetNR10)
		) {
			Scene_Loop_Start_Special(kSceneLoopModeOnce, 4, true);
		}

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraStandingTalkGestureA)) {
			*animation = kModelAnimationDektoraStandingIdle;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorDektora, kAnimationModeIdle);
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptDektora::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (Game_Flag_Query(kFlagNR11DektoraBurning)) {
			_animationState = 32;
			_animationFrame = 0;
			break;
		}
		switch (_animationState) {
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 27:
		case 28:
			_flag = true;
			break;
		case 9:
		case 10:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			_animationState = 11;
			_animationFrame = 0;
			break;
		case 11:
		case 18:
		case 19:
		case 20:
		case 26:
		case 29:
		case 30:
			break;
		case 25:
			_animationState = 25;
			_animationFrame = 0;
			break;
		case 31:
			_animationState = 31;
			_animationFrame = 0;
			break;
		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeWalk:
		if (Game_Flag_Query(kFlagNR11DektoraBurning)) {
			_animationState = 33;
			_animationFrame = 0;
		} else {
			_animationState = 21;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeRun:
		_animationFrame = 0;
		_animationState = 22;
		break;

	case kAnimationModeTalk:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 2;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 9:
		case 10:
		case 16:
		case 17:
			break;
		case 25:
		case 27:
		case 28:
			_animationState = 29;
			_animationFrame = 0;
			break;
		case 31:
			_animationState = 30;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationDektoraSittingPullingGunOut) - 1;
			break;
		default:
			_animationState = 10;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatAttack:
		if (_animationState == 31) {
			_animationState = 26;
		} else if (Random_Query(0, 1) == 1) {
			_animationState = 16;
		} else {
			_animationState = 17;
		}
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalk:
		if (Game_Flag_Query(kFlagNR11DektoraBurning)) {
			_animationState = 33;
			_animationFrame = 0;
		} else {
			_animationState = 21;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatRun:
		_animationFrame = 0;
		_animationState = 22;
		break;

	case 12:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 3;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 13:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 4;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 14:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 5;
			_animationFrame = 0;
			_flag = 0;
		}
		break;

	case 15:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 6;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 16:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 7;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 17:
		if (_animationState < 2
		 || _animationState > 8
		) {
			_animationState = 8;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case kAnimationModeHit:
		if (Game_Flag_Query(kFlagNR11DektoraBurning)) {
			_animationState = 34;
			_animationFrame = 0;
			break;
		}
		switch (_animationState) {
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 16:
		case 17:
			if (Random_Query(0, 1) == 1) {
				_animationState = 14;
			} else {
				_animationState = 15;
			}
			break;
		case 14:
		case 15:
			if (Random_Query(0, 1) == 1) {
				_animationState = 18;
			} else {
				_animationState = 19;
			}
			break;
		}
		_animationFrame = 0;
		break;

	case 23:
		_animationState = 41;
		_animationFrame = 0;
		break;

	case 30:
		if (_animationState != 27
		 && _animationState != 28
		) {
			_animationState = 27;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case 31:
		if (_animationState != 27
		 && _animationState != 28
		) {
			_animationState = 28;
			_animationFrame = 0;
			_flag = false;
		}
		break;

	case kAnimationModeWalkUp:
		_animationFrame = 0;
		_animationState = 23;
		break;

	case kAnimationModeWalkDown:
		_animationFrame = 0;
		_animationState = 24;
		break;

	case kAnimationModeDie:
		if (_vm->_cutContent && Global_Variable_Query(kVariableChapter) == 5) {
			// only play the rattle sound in the Act 5 death (moon bud), but not in chapter 3 death
			// The rattle also plays in ShotAtAndHit() in Act 3 (if Dektora is shot at the back in NR11,
			// and she is Human) but that case is currently never triggered.
			Sound_Play_Speech_Line(kActorDektora, 9020, 60, 0, 99); // add Dektora's death rattle here
		}
		_animationState = 20;
		_animationFrame = 0;
		break;

	case 53:
		switch (_animationState) {
		case 26:
		case 29:
		case 30:
		case 31:
			break;
		case 27:
		case 28:
			_flag = true;
			break;
		default:
			_animationState = 25;
			_animationFrame = 0;
			break;
		}
		break;

	case 70:
		_animationState = 17;
		_animationFrame = 0;
		break;

	case 71:
		_animationState = 16;
		_animationFrame = 0;
		break;

	case 79:
		_animationState = 37;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptDektora::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptDektora::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptDektora::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptDektora::FledCombat() {
	// return false;
}

double AIScriptDektora::comp_distance(int actorId, float x1, float y1, float z1) {
	float x, y, z;

	Actor_Query_XYZ(actorId, &x, &y, &z);

	return sqrt((z1 - z) * (z1 - z) + (x1 - x) * (x1 - x) + (y1 - y) * (y1 - y));
}

void AIScriptDektora::checkCombat() {
	if (Actor_Query_In_Set(kActorDektora, kSetHF01)
	 && Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorDektora) != 450
	) {
		if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsDektora) {
			Global_Variable_Set(kVariableAffectionTowards, kAffectionTowardsNone);
		}

		Actor_Set_Goal_Number(kActorDektora, 450);
		Non_Player_Actor_Combat_Mode_On(kActorDektora, kActorCombatStateIdle, false, kActorMcCoy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 20, 300, false);
	}
}

} // End of namespace BladeRunner
