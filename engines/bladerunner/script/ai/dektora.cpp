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
				if (Game_Flag_Query(kFlagAR02Entered)) {
					Item_Remove_From_World(kItemScrorpions);
				}
				Game_Flag_Set(kFlagAR02DektoraBoughtScorpions);
			}
		}
		return true;
	}

	if (chapter == 3) {
		if (Actor_Query_Goal_Number(kActorDektora) < 199) {
			Actor_Set_Goal_Number(kActorDektora, 199);
		} else {
			switch (Actor_Query_Goal_Number(kActorDektora)) {
			case 269:
				Actor_Set_Goal_Number(kActorDektora, 270);
				break;

			case 271:
				if (Actor_Query_Inch_Distance_From_Actor(kActorDektora, kActorMcCoy) <= 48) {
					Actor_Set_Goal_Number(kActorDektora, 279);
					break;
				}

				if (comp_distance(kActorMcCoy, _x, _y, _z) > 12.0f) {
					Actor_Query_XYZ(kActorMcCoy, &_x, &_y, &_z);
					Async_Actor_Walk_To_XYZ(kActorDektora, _x, _y, _z, 36, 0);
				}
				break;

			case 273:
				Actor_Set_Goal_Number(kActorDektora, 274);
				break;

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
	if (timer == 0) {
		if (Actor_Query_Goal_Number(kActorDektora) == 210) {
			if (Player_Query_Current_Scene() == 61) {
				AI_Countdown_Timer_Reset(kActorDektora, 0);
				AI_Countdown_Timer_Start(kActorDektora, 0, 10);
			} else {
				Actor_Set_Goal_Number(kActorDektora, 211);
				AI_Countdown_Timer_Reset(kActorDektora, 0);
			}
			return; //true;
		}

		if (Actor_Query_Goal_Number(kActorDektora) == 270) {
			AI_Countdown_Timer_Reset(kActorDektora, 0);
			Actor_Set_Goal_Number(kActorDektora, 271);
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

	case 260:
		Actor_Set_Goal_Number(kActorDektora, 290);
		break;

	case 272:
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Face_Heading(kActorDektora, 0, 0);
		Sound_Play(451, 71, 0, 0, 50);

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
	if (Actor_Query_Goal_Number(kActorDektora) == 599) {
		Actor_Face_Actor(0, kActorDektora, true);
		Actor_Says(kActorMcCoy, 8630, 12);

		return; //true;
	}

	if (Actor_Query_Goal_Number(kActorDektora) < 199) {
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		Actor_Says(kActorMcCoy, 8590, 13);
	}

	if (Actor_Query_Goal_Number(kActorDektora) == 210) {
		Game_Flag_Set(kFlagNR08TouchedDektora);
		AI_Movement_Track_Flush(kActorHanoi);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Player_Loses_Control();
		Actor_Set_Goal_Number(kActorHanoi, 220);

		return; //true;
	}

	return; //false;
}

void AIScriptDektora::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptDektora::OtherAgentEnteredThisScene(int otherActorId) {
	if (otherActorId == kActorMcCoy
	 && Actor_Query_Goal_Number(kActorDektora) == 246
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
	if (Actor_Query_Goal_Number(kActorDektora) == 270
	 || Actor_Query_Goal_Number(kActorDektora) == 271
	 || Actor_Query_Goal_Number(kActorDektora) == 272
	) {
		Actor_Set_Health(kActorDektora, 100, 100);

		if (Actor_Query_Goal_Number(kActorDektora) != 272) {
			Actor_Set_Goal_Number(kActorDektora, 272);
		}
		return true;
	}

	if (Actor_Query_Goal_Number(kActorDektora) == 260) {
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Set_Health(kActorDektora, 0, 100);

		if (Game_Flag_Query(47)) {
			Actor_Change_Animation_Mode(kActorDektora, 48);
			Actor_Start_Speech_Sample(kActorDektora, 980);
			Delay(2000);
			Actor_Set_Goal_Number(kActorSteele, 212);
		} else {
			Actor_Change_Animation_Mode(kActorDektora, 48);
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
		Actor_Modify_Friendliness_To_Other(kActorClovis, 0, -5);
	}

	if (byActorId == kActorSteele
	 && Actor_Query_In_Set(kActorSteele, kSetHF06)
	 && Actor_Query_In_Set(kActorMcCoy, kSetHF06)
	) {
		Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, kActorMcCoy, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}

	if (Actor_Query_In_Set(kActorDektora, kSetKP07)) {
		Global_Variable_Decrement(kVariableReplicants, 1);
		Actor_Set_Goal_Number(kActorDektora, 599);

		if (Global_Variable_Query(kVariableReplicants) == 0) {
			Player_Loses_Control();
			Delay(2000);
			Player_Set_Combat_Mode(false);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, true, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(579);
			Game_Flag_Reset(653);
			Set_Enter(kSetKP05_KP06, kSceneKP06);

			return; //true;
		}
	}

	if (Actor_Query_Goal_Number(kActorDektora) != 274) {
		Actor_Set_Goal_Number(kActorDektora, 599);
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

	case 199:
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Set_Goal_Number(kActorDektora, 200);
		break;

	case 200:
		Actor_Put_In_Set(kActorDektora, kSetNR07);
		Actor_Set_At_XYZ(kActorDektora, -136.0f, -75.0f, 14.0f, 300);
		Actor_Change_Animation_Mode(kActorDektora, 53);
		break;

	case 210:
		AI_Movement_Track_Flush(kActorDektora);
		Actor_Put_In_Set(kActorDektora, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorDektora, -923.93f, 127.85f, 413.46f, 30);
		AI_Countdown_Timer_Reset(kActorDektora, 0);
		AI_Countdown_Timer_Start(kActorDektora, 0, 45);
		break;

	case 211:
		if (Player_Query_Current_Scene() == 61) {
			Game_Flag_Set(651);
		} else {
			Game_Flag_Set(636);
			Actor_Put_In_Set(kActorDektora, kSetFreeSlotA);
			Actor_Set_At_Waypoint(kActorDektora, 33, 0);
			Actor_Change_Animation_Mode(kActorDektora, 0);
			Actor_Set_Goal_Number(kActorDektora, 200);
		}
		if (Player_Query_Current_Scene() == 61) {
			Actor_Set_Goal_Number(kActorHanoi, 235);
		} else {
			Game_Flag_Reset(651);
		}
		break;

	case 245:
		Actor_Put_In_Set(kActorDektora, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorDektora, -1558.41f, 0.32f, 319.48f, 264);
		break;

	case 246:
		Actor_Put_In_Set(kActorDektora, kSetNR10);
		Actor_Set_At_XYZ(kActorDektora, 19.22f, 2.84f, -122.43f, 768);
		break;

	case 247:
		Actor_Set_At_XYZ(kActorDektora, -99.0f, 2.88f, -202.0f, 911);
		Actor_Set_Invisible(kActorDektora, 0);
		Actor_Change_Animation_Mode(kActorDektora, 70);
		Actor_Retired_Here(kActorMcCoy, 12, 12, 1, -1);
		break;

	case 250:
		AI_Movement_Track_Flush(kActorDektora);
		AI_Countdown_Timer_Reset(kActorDektora, 0);
		AI_Countdown_Timer_Reset(kActorDektora, 1);
		AI_Countdown_Timer_Reset(kActorDektora, 2);
		Actor_Put_In_Set(kActorDektora, kSetNR11);
		Actor_Set_At_XYZ(kActorDektora, -184.0f, 0.33f, -268.0f, 256);
		break;

	case 260:
		Actor_Set_Targetable(kActorDektora, 1);
		Actor_Force_Stop_Walking(kActorDektora);
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append(kActorDektora, 462, 0);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case 269:
	case 271:
	case 599:
		break; // return true

	case 270:
		Game_Flag_Set(633);
		Actor_Set_Targetable(kActorDektora, 1);
		Loop_Actor_Walk_To_XYZ(kActorDektora, -135.0f, 0.33f, -267.0f, 0, 0, 0, 0);
		Actor_Face_Actor(kActorMcCoy, kActorDektora, 1);
		if (Actor_Query_Goal_Number(kActorSteele) == 216) {
			Actor_Face_Actor(kActorSteele, kActorDektora, 1);
			Actor_Change_Animation_Mode(kActorSteele, 6);
			Delay(250);
			Sound_Play(3, 100, 0, 0, 50);
			Actor_Set_Goal_Number(kActorDektora, 272);
		} else {
			Actor_Set_Goal_Number(kActorDektora, 271);
		}
		break;

	case 272:
		Actor_Force_Stop_Walking(kActorDektora);
		AI_Movement_Track_Flush(kActorDektora);
		AI_Movement_Track_Append(kActorDektora, 456, 0);
		AI_Movement_Track_Repeat(kActorDektora);
		break;

	case 274:
		Player_Loses_Control();
		Scene_Exits_Enable();
		Game_Flag_Set(632);
		Actor_Put_In_Set(kActorDektora, kSetNR01);
		Actor_Set_At_XYZ(kActorDektora, -177.0f, 23.88f, -373.0f, 300);
		_animationState = 36;
		_animationFrame = 0;
		Set_Enter(kSetNR01, kSceneNR01);
		break;

	case 279:
		Actor_Force_Stop_Walking(kActorDektora);
		Actor_Put_In_Set(kActorDektora, kSetNR10);
		Actor_Set_At_XYZ(kActorDektora, 14.0f, 2.84f, -300.0f, 926);
		Actor_Set_Goal_Number(kActorMcCoy, 231);
		_animationState = 36;
		_animationFrame = 0;
		break;

	case 290:
		Game_Flag_Set(kFlagDektoraRanAway);
		break;

	case 299:
		Actor_Put_In_Set(kActorDektora, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorDektora, 41, 0);
		Actor_Set_Goal_Number(kActorDektora, 599);
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
		*animation = 156;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(156) - 1) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 19;
		_animationFrame = 0;
		break;

	case 2:
		*animation = 158;
		if (!_animationFrame && _flag) {
			*animation = 156;
			_animationState = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(158)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 159;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(159)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 158;
		}
		break;

	case 4:
		*animation = 160;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(160)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 158;
		}
		break;

	case 5:
		*animation = 161;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(161)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 158;
		}
		break;

	case 6:
		*animation = 161;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(161)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 158;
		}
		break;

	case 7:
		*animation = 162;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(162)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 158;
		}
		break;

	case 8:
		*animation = 162;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(162)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 158;
		}
		break;

	case 9:
		*animation = 134;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(134)) {
			_animationFrame = 0;
		}
		break;

	case 10:
		*animation = 139;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(139)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
		}
		break;

	case 11:
		*animation = 140;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(140)) {
			*animation = 156;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 12:
		*animation = 135;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(135)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
			Actor_Change_Animation_Mode(kActorDektora, 4);
		}
		break;

	case 13:
		*animation = 136;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(136)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
			Actor_Change_Animation_Mode(kActorDektora, 4);
		}
		break;

	case 14:
		*animation = 137;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(137)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
			Actor_Change_Animation_Mode(kActorDektora, 4);
		}
		break;

	case 15:
		*animation = 138;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(138)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
			Actor_Change_Animation_Mode(kActorDektora, 4);
		}
		break;

	case 16:
		*animation = 141;
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
			Actor_Combat_AI_Hit_Attempt(3);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(141)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
			Actor_Change_Animation_Mode(kActorDektora, 4);
		}
		break;

	case 17:
		*animation = 142;
		_animationFrame++;
		if (_animationFrame == 6 && Actor_Query_Goal_Number(kActorDektora) == 247) {
			Actor_Change_Animation_Mode(0, 48);
		}
		if (_animationFrame == 3) {
			int speech;

			if (Random_Query(1, 2) == 1) {
				speech = 9010;
			} else {
				speech = 9015;
			}
			Sound_Play_Speech_Line(3, speech, 75, 0, 99);
		}
		if (_animationFrame == 6) {
			Actor_Combat_AI_Hit_Attempt(3);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(142)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 134;
			Actor_Change_Animation_Mode(kActorDektora, 4);
		}
		break;

	case 18:
		*animation = 147;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(147)) {
			*animation = 156;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorDektora, 0);
		}
		break;

	case 19:
		*animation = 148;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(148)) {
			*animation = 156;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorDektora, 0);
		}
		break;

	case 20:
		*animation = 149;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(149) - 1) {
			_animationFrame++;
		}
		break;

	case 21:
		*animation = 143;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(143)) {
			_animationFrame = 0;
		}
		break;

	case 22:
		*animation = 144;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(144)) {
			_animationFrame = 0;
		}
		break;

	case 23:
		*animation = 145;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(145)) {
			_animationFrame = 0;
		}
		break;

	case 24:
		*animation = 146;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(146)) {
			_animationFrame = 0;
		}
		break;

	case 25:
		*animation = 150;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(150)) {
			_animationFrame = 0;
		}
		break;

	case 26:
		*animation = 151;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(151)) {
			_animationFrame = 0;
			_animationState = 31;
			*animation = 155;
		}
		break;

	case 27:
		*animation = 152;
		if (!_animationFrame && _flag) {
			*animation = 150;
			_animationState = 25;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(152)) {
				_animationFrame = 0;
			}
		}
		break;

	case 28:
		*animation = 153;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(153)) {
			_animationFrame = 0;
			_animationState = 27;
			*animation = 152;
		}
		break;

	case 29:
		*animation = 154;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(154)) {
			_animationFrame = 0;
			_animationState = 31;
			*animation = 155;
		}
		break;

	case 30:
		*animation = 154;
		_animationFrame--;
		if (_animationFrame == 0) {
			_animationFrame = 0;
			_animationState = 25;
			*animation = 150;
		}
		break;

	case 31:
		*animation = 155;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(155)) {
			_animationFrame = 0;
		}
		break;

	case 32:
		*animation = 163;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(163)) {
			_animationFrame = 0;
		}
		break;

	case 33:
		*animation = 166;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(166)) {
			_animationFrame = 0;
		}
		break;

	case 34:
		*animation = 164;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(164)) {
			_animationFrame = 0;
			_animationState = 32;
			*animation = 163;
		}
		break;

	case 35:
		*animation = 165;
		_animationFrame++;
		if (_animationFrame == 2) {
			Game_Flag_Set(659);
		}
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			_animationFrame = 0;
			_animationState = 36;
			*animation = 167;
			Actor_Set_Goal_Number(kActorDektora, 273);
		}
		break;

	case 36:
		*animation = 167;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(167) - 1) {
			_animationFrame++;
		}
		if (_animationFrame == 11) {
			Ambient_Sounds_Play_Sound(206, 80, -20, -20, 20);
		}
		break;

	case 37:
		*animation = 168;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(567, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(568, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(569, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			default:
				break;
			}
		}
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(168) - 1) {
			_animationFrame = 0;
			_animationState = 38;
			*animation = 169;
		}
		break;

	case 38:
		*animation = 169;
		_animationFrame++;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(567, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(568, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(569, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			default:
				break;
			}
		}
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			_animationFrame = 0;
			_animationState = 39;
			*animation = 170;
		}
		break;

	case 39:
		*animation = 170;
		_animationFrame++;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(567, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(568, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(569, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			default:
				break;
			}
		}
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			_animationFrame = 0;
			_animationState = 40;
			*animation = 171;
		}
		break;

	case 40:
		*animation = 171;
		if (_animationFrame == 1) {
			switch (Random_Query(0, 2)) {
			case 0:
				Sound_Play(567, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 1:
				Sound_Play(568, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			case 2:
				Sound_Play(569, (100 / Random_Query(5, 9)), 0, 0, 50);
				break;

			default:
				break;
			}
		}
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(171) - 1) {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(171) - 1) {
				Actor_Set_Goal_Number(kActorDektora, 211);
			}
		}
		break;

	case 41:
		*animation = 157;
		_animationFrame++;
		if (_animationFrame == 8 && Actor_Query_In_Set(kActorDektora, kSetNR10)) {
			Scene_Loop_Start_Special(2, 4, 1);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(157)) {
			*animation = 156;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorDektora, 0);
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
		if (Game_Flag_Query(633)) {
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
		if (!Game_Flag_Query(633)) {
			_animationFrame = 0;
			_animationState = 21;
		} else {
			_animationState = 33;
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
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(154) - 1;
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
		if (!Game_Flag_Query(633)) {
			_animationFrame = 0;
			_animationState = 21;
		} else {
			_animationState = 33;
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

	case 21:
		if (Game_Flag_Query(633)) {
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
