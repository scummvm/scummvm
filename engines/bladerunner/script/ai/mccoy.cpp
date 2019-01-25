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

AIScriptMcCoy::AIScriptMcCoy(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	dword_45A0D8 = 0;
	dword_45A0DC = 0;
	_animationFrameDelta = 0;
	dword_45A0E4 = 0;
	off_45A0EC = 0.0f;
	dword_45A0E8 = 0;
	dword_45A0F0 = 0;
	dword_45A0F4 = 0;
	dword_45A0F8 = 0;
	dword_45A0FC = 0;
	off_45A100 = 0.0f;
	flt_462710 = 0.0f;
	flt_462714 = 0.0f;
}

void AIScriptMcCoy::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	dword_45A0D8 = 0;
	dword_45A0DC = 30;
	_animationFrameDelta = 1;
	dword_45A0E4 = 0;
	off_45A0EC = 0;
	dword_45A0E8 = 3;
	dword_45A0F0 = 3;
	dword_45A0F4 = 20;
	dword_45A0F8 = -1;
	dword_45A0FC = 0;
	off_45A100 = 0;
	Actor_Set_Goal_Number(kActorMcCoy, 0);
}

bool AIScriptMcCoy::Update() {
	if (dword_45A0F8 != -1) {
		Sound_Play(dword_45A0F8, 100, 0, 0, 50);
		dword_45A0F8 = -1;
	}

	switch (Actor_Query_Goal_Number(kActorMcCoy)) {
	case kGoalMcCoyBB11PrepareToRunAway:
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyBB11RunAway);
		return true;

	case kGoalMcCoyBB11RunAway:
		if ( Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 316) < 36
		 && !Game_Flag_Query(kFlagBB11SadikPunchedMcCoy)
		) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeHit);
			Game_Flag_Set(kFlagBB11SadikPunchedMcCoy);
		}

		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 316) < 4) {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyBB11GetCaught);
		}
		return true;

	case 201:
		Actor_Set_Goal_Number(kActorMcCoy, 0);
		if (Player_Query_Current_Set() == kSetNR03) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -166.0f, -70.19f, -501.0f, 0, false, false, 0);
			Actor_Face_Heading(kActorMcCoy, 300, false);
		} else {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -461.0f, 0.0f, -373.0f, 0, false, false, 0);
		}
		Player_Gains_Control();
		return true;

	case 212:
		if (Global_Variable_Query(47) >= 13) {
			Global_Variable_Set(47, 500);
		} else if (Global_Variable_Query(47) > 0) {
			Global_Variable_Decrement(47, 1);
		}
		break;

	case 221:
		Actor_Set_Goal_Number(kActorMcCoy, 0);
		Actor_Set_Goal_Number(kActorEarlyQ, 212);
		return true;

	case 309:
		sub_4053E0();
		break;

	case 391:
		Actor_Retired_Here(kActorMcCoy, 12, 48, 1, -1);
		Actor_Set_Goal_Number(kActorMcCoy, 599);
		break;

	case 231:
		sub_4053E0();
		break;
	}
	return false;
}

void AIScriptMcCoy::TimerExpired(int timer) {
}

void AIScriptMcCoy::CompletedMovementTrack() {
}

void AIScriptMcCoy::ReceivedClue(int clueId, int fromActorId) {
	switch (clueId) {
	case kClueChopstickWrapper:
	case kClueSushiMenu:
		Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationChinatown, true);
		break;

	case kClueDragonflyEarring:
	case kClueBombingSuspect:
		Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationAnimoidRow, true);
		break;

	case kClueKingstonKitchenBox1:
	case kClueKingstonKitchenBox2:
		if (Query_Difficulty_Level() == 0) {
			Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationAnimoidRow, true);
		}
		break;

	case kClueDragonflyCollection:
		Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationNightclubRow, true);
		break;

	case kClueHysteriaToken:
	case kClueCarRegistration1:
	case kClueCarRegistration2:
	case kClueCarRegistration3:
	case kClueLichenDogWrapper:
		Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationHysteriaHall, true);
		Spinner_Set_Selectable_Destination_Flag(kSpinnerDestinationNightclubRow, true);
		break;

	case kClueWeaponsCache:
	case kClueWeaponsOrderForm:
	case kClueShippingForm:
	case kCluePoliceIssueWeapons:
		Global_Variable_Increment(49, 1);
		break;

	case kClueFolder:
		Global_Variable_Increment(49, 5);
		break;

	case kClueHomelessManKid:
	case kClueOriginalRequisitionForm:
		Global_Variable_Increment(49, 3);
		break;

	case kClueScaryChair:
	case kClueIzosStashRaided:
		Global_Variable_Increment(49, 2);
		break;

	case kClueDNATyrell:
	case kClueDNASebastian:
	case kClueDNAChew:
	case kClueDNAMoraji:
	case kClueDNALutherLance:
	case kClueDNAMarcus:
		Global_Variable_Increment(kVariableDNAEvidences, 1);
		break;
	}

	if ( Global_Variable_Query(49) > 6
	 &&  Global_Variable_Query(kVariableChapter) > 3
	 && !Actor_Clue_Query(kActorMcCoy, kClueGuzzaFramedMcCoy)
	) {
		Delay(500);
		Actor_Voice_Over(3320, kActorVoiceOver);
		switch (clueId) {
		case kClueWeaponsCache:
		case kClueWeaponsOrderForm:
		case kClueGuzzasCash:
		case kCluePoliceIssueWeapons:
		case kClueIzosStashRaided:
		case kClueOriginalRequisitionForm:
			Actor_Voice_Over(3340, kActorVoiceOver);
			Actor_Voice_Over(3350, kActorVoiceOver);
			Actor_Voice_Over(3360, kActorVoiceOver);
			Actor_Voice_Over(3370, kActorVoiceOver);
			Actor_Voice_Over(3380, kActorVoiceOver);
			break;

		case kClueHomelessManKid:
			Actor_Voice_Over(3330, kActorVoiceOver);
			break;

		case kClueScaryChair:
			Actor_Voice_Over(3390, kActorVoiceOver);
			Actor_Voice_Over(3400, kActorVoiceOver);
			Actor_Voice_Over(3420, kActorVoiceOver);
			break;
		}

		Actor_Clue_Acquire(kActorMcCoy, kClueGuzzaFramedMcCoy, true, -1);

		if (clueId == kClueFolder) {
			Actor_Voice_Over(2780, kActorVoiceOver);
			Actor_Voice_Over(2800, kActorVoiceOver);
			Actor_Voice_Over(2810, kActorVoiceOver);
		} else if (Actor_Clue_Query(kActorMcCoy, kClueFolder)) {
			Actor_Voice_Over(3430, kActorVoiceOver);
			Actor_Voice_Over(3440, kActorVoiceOver);
			Actor_Voice_Over(3450, kActorVoiceOver);
			Actor_Voice_Over(3460, kActorVoiceOver);
			Actor_Voice_Over(3470, kActorVoiceOver);
			Actor_Voice_Over(3480, kActorVoiceOver);
			Actor_Voice_Over(3490, kActorVoiceOver);
			Actor_Voice_Over(3500, kActorVoiceOver);
		} else {
			Actor_Voice_Over(3510, kActorVoiceOver);
			Actor_Voice_Over(3520, kActorVoiceOver);
			Actor_Voice_Over(3530, kActorVoiceOver);
			Actor_Voice_Over(3540, kActorVoiceOver);
		}
	}
}

void AIScriptMcCoy::ClickedByPlayer() {
}

void AIScriptMcCoy::EnteredScene(int sceneId) {
}

void AIScriptMcCoy::OtherAgentEnteredThisScene(int otherActorId) {
}

void AIScriptMcCoy::OtherAgentExitedThisScene(int otherActorId) {
}

void AIScriptMcCoy::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
}

void AIScriptMcCoy::ShotAtAndMissed() {
}

bool AIScriptMcCoy::ShotAtAndHit() {
	return false;
}

void AIScriptMcCoy::Retired(int byActorId) {
	if (byActorId == kActorSteele && Actor_Query_In_Set(kActorSteele, kSetHF06)) {
		if (Actor_Query_In_Set(kActorDektora, kSetHF06) && Actor_Query_Goal_Number(kActorDektora) != 599) {
			Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, kActorDektora, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
		} else if (Actor_Query_In_Set(kActorLucy, kSetHF06) && Actor_Query_Goal_Number(kActorLucy) != 599) {
			Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, kActorLucy, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
		}
	}

	if (Actor_Query_In_Set(kActorMcCoy, kSetHF05) && Actor_Query_In_Set(kActorOfficerLeary, kSetHF05) && Actor_Query_In_Set(kActorDektora, kSetHF05) && Actor_Query_Goal_Number(kActorDektora) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateUncover, true, kActorDektora, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}

	if (Actor_Query_In_Set(kActorMcCoy, kSetHF05) && Actor_Query_In_Set(kActorOfficerGrayford, kSetHF05) && Actor_Query_In_Set(kActorDektora, kSetHF05) && Actor_Query_Goal_Number(kActorDektora) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateUncover, true, kActorDektora, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}

	if (Actor_Query_In_Set(kActorMcCoy, kSetHF05) && Actor_Query_In_Set(kActorOfficerLeary, kSetHF05) && Actor_Query_In_Set(kActorLucy, kSetHF05) && Actor_Query_Goal_Number(kActorLucy) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, kActorCombatStateUncover, true, kActorLucy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}

	if (Actor_Query_In_Set(kActorMcCoy, kSetHF05) && Actor_Query_In_Set(kActorOfficerGrayford, kSetHF05) && Actor_Query_In_Set(kActorLucy, kSetHF05) && Actor_Query_Goal_Number(kActorLucy) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateUncover, true, kActorLucy, 4, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}
}

int AIScriptMcCoy::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMcCoy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	unsigned int affectionTowards;

	switch (newGoalNumber) {
	case kGoalMcCoyDefault:
		return true;

	case kGoalMcCoyDodge:
		dodge();
		return true;

	case 2:
		sub_405920();
		return true;

	case kGoalMcCoyBB11GetUp:
		Actor_Set_At_Waypoint(kActorMcCoy, 315, 263);
		_animationState = 53;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		Actor_Set_Invisible(kActorMcCoy, false);
		return true;

	case kGoalMcCoyBB11RunAway:
		Async_Actor_Walk_To_Waypoint(kActorMcCoy, 316, 0, true);
		return true;

	case kGoalMcCoyBB11GetCaught:
		Actor_Face_Actor(kActorMcCoy, kActorSadik, true);
		return true;

	case 231:
		Player_Set_Combat_Mode(false);
		Preload(18);
		Set_Enter(kSetNR10, kSceneNR10);
		Player_Loses_Control();
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Put_In_Set(kActorMcCoy, kSetNR10);
		Actor_Set_At_XYZ(kActorMcCoy, 14.0f, 110.84f, -300.0f, 926);
		Actor_Change_Animation_Mode(kActorMcCoy, 48);
		_animationState = 27;
		_animationFrame = 0;
		flt_462714 = 2.84f;
		flt_462710 = 110.84f;
		off_45A100 = -6.0f;
		return true;

	case 230:
		dword_45A0FC = Actor_Query_Goal_Number(kActorSteele) == 215;
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAttack);
		return true;

	case 220:
		Actor_Change_Animation_Mode(kActorMcCoy, 75);
		return true;

	case 212:
		Global_Variable_Set(47, 0);
		Player_Set_Combat_Mode_Access(false);
		Player_Gains_Control();
		Scene_Exits_Disable();
		_animationState = 68;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		return true;

	case 211:
		Actor_Face_Heading(kActorMcCoy, 512, false);
		Actor_Face_Heading(kActorMcCoy, 768, true);
		if (Random_Query(0, 1)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 18);
		} else {
			Actor_Change_Animation_Mode(kActorMcCoy, 16);
		}
		Delay(150);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		Actor_Set_Goal_Number(kActorMcCoy, 0);
		Player_Gains_Control();
		return true;

	case 210:
		Actor_Put_In_Set(kActorMcCoy, kSetNR01);
		Actor_Set_At_XYZ(kActorMcCoy, -204.0, 24.0, -817.0, 256);
		Actor_Set_Invisible(kActorMcCoy, false);
		if (Game_Flag_Query(627)) {
			Actor_Set_Goal_Number(kActorMcCoy, 212);
		} else {
			_animationState = 53;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
			Actor_Set_Invisible(kActorMcCoy, false);
		}
		return true;

	case 200:
		Player_Loses_Control();
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		return true;

	case 301:
		_animationState = 62;
		_animationFrame = 0;
		return true;

	case 302:
		_animationState = 64;
		_animationFrame = 0;
		return true;

	case 303:
		_animationState = 65;
		_animationFrame = 0;
		return true;

	case 350:
		Sound_Play(123, 50, 0, 0, 50);
		Delay(1000);
		Sound_Play(403, 30, 0, 0, 50);
		Delay(1000);
		Sound_Play(123, 50, 0, 0, 50);
		Actor_Says(kActorGuzza, 1380, 3);
		Actor_Says(kActorMcCoy, 6610, 13);
		Actor_Says(kActorGuzza, 1390, 3);
		Actor_Says(kActorMcCoy, 6615, 18);
		Actor_Says(kActorGuzza, 1420, 3);
		Actor_Says(kActorMcCoy, 6625, 11);
		Actor_Says(kActorGuzza, 1430, 3);
		Actor_Says(kActorMcCoy, 6630, 12);
		Actor_Says(kActorMcCoy, 6635, 17);
		Actor_Says(kActorMcCoy, 6640, 13);
		Actor_Says(kActorMcCoy, 6645, 19);
		Actor_Says(kActorMcCoy, 6650, 18);
		Actor_Says(kActorMcCoy, 6655, 11);
		Actor_Says(kActorGuzza, 1440, 3);
		Actor_Says(kActorMcCoy, 6660, 17);
		Actor_Says(kActorMcCoy, 6665, 13);
		Delay(1000);
		Actor_Says(kActorGuzza, 1450, 3);
		Actor_Says(kActorMcCoy, 6670, 14);
		Actor_Says(kActorMcCoy, 6675, 11);
		Actor_Says(kActorGuzza, 1460, 3);
		Actor_Says(kActorMcCoy, 6680, 12);
		Actor_Says(kActorGuzza, 1470, 3);
		Actor_Says(kActorMcCoy, 6685, 13);
		Delay(500);
		Actor_Says(kActorMcCoy, 6695, 16);
		Actor_Says(kActorMcCoy, 6700, 17);
		Actor_Says(kActorGuzza, 1480, 3);
		Actor_Says(kActorMcCoy, 6705, 11);
		Sound_Play(123, 50, 0, 0, 50);
		return true;

	case 390:
		Actor_Force_Stop_Walking(kActorMcCoy);
		Player_Loses_Control();
		flt_462710 = 48.07f;
		off_45A100 = -4.0f;
		flt_462714 = -20.0f;
		if (_animationState != 27 && _animationState != 50) {
			_animationState = 50;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		}
		return true;

	case 400:
		Actor_Set_Health(kActorMcCoy, 50, 50);
		Game_Flag_Set(373);
		affectionTowards = Global_Variable_Query(kVariableAffectionTowards);
		if (affectionTowards == kAffectionTowardsSteele) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 3);
		} else if (affectionTowards == kAffectionTowardsDektora) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 3);
		} else if (affectionTowards == kAffectionTowardsLucy) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 5);
		}
		if (Game_Flag_Query(666)) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 3);
		}
		if (Actor_Query_Friendliness_To_Other(kActorSteele, kActorMcCoy) < Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy)) {
			Game_Flag_Set(653);
		}
		affectionTowards = Global_Variable_Query(kVariableAffectionTowards);
		if (affectionTowards == kAffectionTowardsSteele) {
			if (Game_Flag_Query(653)) {
				Global_Variable_Set(kVariableAffectionTowards, kAffectionTowardsNone);
			}
		} else if (affectionTowards == kAffectionTowardsDektora
		        || affectionTowards == kAffectionTowardsLucy
		) {
			if (!Game_Flag_Query(653)) {
				Global_Variable_Set(kVariableAffectionTowards, kAffectionTowardsNone);
			}
		}
		if (!Game_Flag_Query(653)) {
			Game_Flag_Set(kFlagMaggieIsHurt);
		}
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Global_Variable_Set(kVariableChapter, 5);
		Outtake_Play(kOuttakeMovieD, false, -1);
		if (Game_Flag_Query(666)) {
			Chapter_Enter(5, kSetMA07, kSceneMA07);
		} else {
			Game_Flag_Set(kFlagMA06ToMA02);
			Chapter_Enter(5, kSetMA02_MA04, kSceneMA02);
		}
		return true;

	case kGoalMcCoyArrested:
		Music_Stop(3);
		Player_Set_Combat_Mode(false);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		_animationState = 0;
		_animationFrame = 0;
		Game_Flag_Set(kFlagMcCoyArrested);
		Set_Enter(kSetPS09, kScenePS09);
		return true;
	}
	return false;
}

bool AIScriptMcCoy::UpdateAnimation(int *animation, int *frame) {
	int v7, v18, v19, v46;
	switch (_animationState) {
	case 71:
		*animation = 52;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(52)) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
			Player_Gains_Control();
		}
		break;
	case 70:
		*animation = 51;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(51)) {
			_animationFrame = 0;
		}
		break;
	case 69:
		*animation = 50;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(50)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 53);
			*animation = 51;
		}
		break;
	case 68:
		*animation = 18;
		v7 = Slice_Animation_Query_Number_Of_Frames(*animation) - 1 - Global_Variable_Query(47);
		if (_animationFrame < v7) {
			_animationFrame++;
		} else if (_animationFrame > v7) {
			_animationFrame--;
		}
		if (_animationFrame <= 0) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
			*animation = 19;
			_animationState = 0;
			_animationFrame = 0;
			Game_Flag_Reset(627);
			Scene_Exits_Enable();
			Player_Set_Combat_Mode_Access(true);
			Actor_Set_Goal_Number(kActorMcCoy, 0);
		}
		break;
	case 67:
		*animation = 53;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(53)) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
			*animation = 19;
			_animationState = 0;
			_animationFrame = 0;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 220) {
				Actor_Change_Animation_Mode(kActorMcCoy, 48);
			}
		}
		break;
	case 66:
		*animation = 40;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(40)) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	case 65:
		*animation = 45;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(45)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 0;
			Actor_Set_Goal_Number(kActorMcCoy, 0);
		}
		break;
	case 64:
		*animation = 44;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(44)) {
			_animationFrame = 0;
			_animationState = 63;
			*animation = 43;
		}
		break;
	case 63:
		*animation = 43;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(43)) {
			_animationFrame = 0;
		}
		break;
	case 62:
		*animation = 42;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(42)) {
			_animationFrame = 0;
			_animationState = 63;
			*animation = 43;
		}
		break;
	case 61:
		*animation = 41;
		_animationFrame--;
		if (_animationFrame <= 0) {
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 200) {
				Actor_Set_Goal_Number(kActorMcCoy, 201);
			}
		}
		break;
	case 60:
		*animation = 41;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(41) - 1) {
			_animationFrame++;
		}
		break;
	case 59:
		*animation = 48;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(48)) {
			*animation = 19;
			_animationFrame = 0;
			dword_45A0DC = 0;
			_animationState = 0;
			Player_Gains_Control();
			Item_Add_To_World(kItemChair, 982, kSetCT08_CT51_UG12, -110.0, 0.0, -192.0, 0, 48, 32, false, true, false, false);
		}
		break;
	case 58:
		*animation = 47;
		_animationFrame++;
		if (_animationFrame == 6) {
			v18 = Random_Query(0, 2);
			v19 = 0;
			if (v18 == 0) {
				v19 = 595;
			} else if (v18 == 1) {
				v19 = 594;
			} else if (v18 == 2) {
				v19 = 593;
			}
			Ambient_Sounds_Play_Sound(v19, 39, 0, 0, 99);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation) - 1) { //why -1?
			_animationFrame = 0;
		}
		break;
	case 57:
		*animation = 46;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(46)) {
			_animationFrame = 0;
		}
		if (!Game_Flag_Query(kFlagMcCoyTiedDown)) {
			_animationFrame = 0;
			_animationState = 59;
			*animation = 48;
		}
		break;
	case 56:
		*animation = 49;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(49)) {
			_animationFrame = 0;
			if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetUG15) {
				_animationState = 27;
			} else {
				*animation = 19;
				_animationState = 0;
				Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
			}
		}
		break;
	case 55:
		*animation = 32;
		_animationFrame++;
		if (_animationFrame == 7) {
			Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeFeeding);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = 19;
			_animationState = 0;
		}
		break;
	case 53:
		*animation = 18;
		_animationFrame--;
		if (_animationFrame <= 0) {
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
			if (Actor_Query_Goal_Number(kActorMcCoy) == kGoalMcCoyBB11GetUp) {
				Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyBB11PrepareToRunAway);
			}
			if (Actor_Query_Goal_Number(kActorMcCoy) == 210) {
				Actor_Set_Goal_Number(kActorMcCoy, 211);
			}
		}
		break;
	case 52:
		*animation = 31;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(31)) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(31) - 1;
			_animationState = 50;
		}
		break;
	case 51:
		*animation = 28;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(28)) {
			Player_Set_Combat_Mode(true);
			sub_405800();
			Actor_Set_Goal_Number(kActorMcCoy, 0);
			_animationFrame = 0;
			Player_Gains_Control();
		}
		break;
	case 50:
		*animation = 18;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		break;
	case 49:
		*animation = 34;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(34)) {
			Actor_Set_Goal_Number(kActorMcCoy, 0);
			*animation = 19;
			_animationFrame = 0;
			sub_405660();
		}
		break;
	case 48:
		*animation = 33;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(33)) {
			_animationFrame = 0;
			_animationState = 49;
			*animation = 34;
		}
		break;
	case 47:
		*animation = 29;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(29)) {
			_animationFrame = 0;
			_animationState = 48;
			*animation = 33;
		}
		break;
	case 46:
		Actor_Set_Invisible(kActorMcCoy, false);
		*animation = 36;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(36)) {
			*animation = 19;
			_animationFrame = 0;
			Player_Gains_Control();
			sub_405660();
			Actor_Face_Heading(kActorMcCoy, (Actor_Query_Facing_1024(kActorMcCoy) + 512) & 1023, false);
		}
		break;
	case 45:
		*animation = 35;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(35)) {
			Actor_Set_Invisible(kActorMcCoy, true);
			*animation = 19;
			_animationFrame = 0;
			sub_405660();
		}
		break;
	case 44:
		*animation = 30;
		if (_animationFrame++ == 127) {
			Game_Flag_Set(kFlagCT04BodyDumped);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(30)) {
			*animation = 19;
			sub_405660();
			Actor_Set_At_XYZ(kActorMcCoy, -203.41f, -621.3f, 724.57f, 538);
			Player_Gains_Control();
		}
		break;
	case 43:
		*animation = 38;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(38) - 1) { //why -1?
			_animationFrame = 0;
		}
		switch (_animationFrame) {
		case 9:
			Sound_Left_Footstep_Walk(kActorMcCoy);
			break;
		case 4:
			Sound_Right_Footstep_Walk(kActorMcCoy);
			break;
		case 1:
			Sound_Right_Footstep_Walk(kActorMcCoy);
			break;
		}
		break;
	case 42:
		*animation = 37;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(37) - 1) { //why -1?
			_animationFrame = 0;
		}
		switch (_animationFrame) {
		case 9:
			Sound_Left_Footstep_Walk(kActorMcCoy);
			break;
		case 4:
			Sound_Right_Footstep_Walk(kActorMcCoy);
			break;
		case 1:
			Sound_Right_Footstep_Walk(kActorMcCoy);
			break;
		}
		break;
	case 41:
		*animation = 7;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(7)) {
			_animationFrame = 0;
			++off_45A0EC;
		}
		if (_animationFrame == 9) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (_animationFrame == 4) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(kFlagHF05toHF07)) {
			sub_4059D0(-0.2f);
		}
		break;
	case 40:
		*animation = 6;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(6)) {
			_animationFrame = 0;
			++off_45A0EC;
		}
		if (_animationFrame == 8) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (_animationFrame == 3) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(kFlagHF07toHF05)) {
			sub_405940(0.5f);
		}
		break;
	case 39:
		*animation = 16;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(16)) {
			_animationFrame = 0;
			++off_45A0EC;
		}
		if (_animationFrame == 8) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (_animationFrame == 3) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(kFlagHF05toHF07)) {
			sub_4059D0(-0.2f);
		}
		break;
	case 38:
		*animation = 15;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(15)) {
			_animationFrame = 0;
			++off_45A0EC;
		}
		if (_animationFrame == 9) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (_animationFrame == 4) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(kFlagHF07toHF05)) {
			sub_405940(0.5f);
		}
		break;
	case 37:
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(4)) {
			_animationFrame = 0;
		}
		*animation = 4;
		if (_animationFrame == 6) {
			Sound_Left_Footstep_Run(kActorMcCoy);
		}
		if (_animationFrame == 0) {
			Sound_Right_Footstep_Run(kActorMcCoy);
		}
		break;
	case 36:
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(3)) {
			_animationFrame = 0;
		}
		*animation = 3;
		if (_animationFrame == 15) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		}
		if (_animationFrame == 6) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		break;
	case 32:
		_animationFrame = 1;
		_animationState = 30;
		*animation = 13;
		break;
	case 31:
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(14)) {
			_animationFrame = 0;
		}
		*animation = 14;
		if (_animationFrame == 5) {
			Sound_Left_Footstep_Run(kActorMcCoy);
		} else if (_animationFrame == 12) {
			Sound_Right_Footstep_Run(kActorMcCoy);
		}
		break;
	case 30:
		*animation = 13;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(13)) {
			_animationFrame = 0;
		}
		if (_animationFrame == 2) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		} else if (_animationFrame == 10) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		}
		break;
	case 29:
		v46 = _animationFrame + _animationFrameDelta;
		*animation = 18;
		_animationFrame = v46;
		if (v46 < 14) {
			_animationFrameDelta = 1;
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			*animation = 18;
			_animationState = 27;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		}
		break;
	case 28:
		*animation = 5;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(5)) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			_animationState = 50;
			sub_4054F0();
		}
		break;
	case 27:
		*animation = 18;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(18)) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			_animationState = 50;
			sub_4054F0();
			if (Actor_Query_Goal_Number(kActorMcCoy) == 220) {
				Actor_Set_Goal_Number(kActorMcCoy, 221);
			}
		}
		break;
	case 26:
		*animation = 17;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(17)) {
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		}
		break;
	case 25:
		*animation = 17;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(17)) {
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeIdle);
		}
		break;
	case 24:
		*animation = 1;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(1)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatIdle);
		}
		break;
	case 23:
		*animation = 1;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(1)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatIdle);
		}
		break;
	case 22:
		_animationFrame = 0;
		_animationState = 17;
		*animation = 12;
		break;
	case 21:
		*animation = 12;
		_animationFrame++;
		if (_animationFrame == 1 && Actor_Query_Goal_Number(kActorMcCoy) == 230 && dword_45A0FC == 1) {
			dword_45A0F8 = 27;
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(12)) {
			_animationState = 17;
			_animationFrame = 0;
			*animation = 12;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 230) {
				_animationFrame = 0;
				_animationState = 21;
				dword_45A0FC = 1;
				*animation = 12;
			}
		}
		break;
	case 19:
		*animation = 11;
		_animationFrame++;
		if (_animationFrame >= 12) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 0;
		}
		break;
	case 18:
		*animation = 10;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(10)) {
			_animationFrame = 0;
			_animationState = 17;
			*animation = 12;
		}
		break;
	case 17:
		*animation = 12;
		_animationFrame = 0;
		// weird, but thats in game code
		if (Slice_Animation_Query_Number_Of_Frames(12) <= 0) {
			_animationFrame = 0;
			_animationState = 17;
		}
		break;
	case 16:
		*animation = 9;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(9)) {
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	case 15:
		*animation = 8;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(8)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 0;
		}
		break;
	case 14:
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(0)) {
			_animationState = 14;
			_animationFrame = 0;
		}
		*animation = 0;
		break;
	case 13:
		*animation = 19;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(19) / 2) {
			_animationFrame -= 3;
			if (_animationFrame <= 0) {
				_animationFrame = 0;
				*animation = _animationNext;
				_animationState = _animationStateNext;
			}
		} else {
			_animationFrame += 3;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(19)) {
				_animationFrame = 0;
				*animation = _animationNext;
				_animationState = _animationStateNext;
			}
		}
		break;
	case 12:
		*animation = 27;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(27)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 11:
		*animation = 26;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(26)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 10:
		*animation = 25;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(25)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 9:
		*animation = 24;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(24)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 8:
		*animation = 23;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(23)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 7:
		*animation = 22;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(22)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 6:
		*animation = 27;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(27)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 5:
		*animation = 21;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(21)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 20;
		}
		break;
	case 3:
	case 4:
		if (_animationFrame == 0 && !Game_Flag_Query(kFlagMcCoyAnimation1)) {
			_animationFrame = 1;
			_animationState = dword_45A0F0;
			*animation = dword_45A0F4;
			dword_45A0F0 = 4;
			dword_45A0F4 = 20;
		} else if (_animationFrame <= 4 && Game_Flag_Query(kFlagMcCoyAnimation1)) {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			*animation = 19;
			_animationFrame = 0;
			_animationState = 0;
		} else {
			*animation = 20;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(20)) {
				_animationFrame = 0;
			}
		}
		break;
	case 0:
		*animation = 19;
		if (dword_45A0D8 < dword_45A0DC) {
			//*frame = dword_45A0E8;
			_animationFrame += _animationFrameDelta;
			if (_animationFrame > dword_45A0E8) {
				_animationFrame = dword_45A0E8;
				_animationFrameDelta = -1;
			} else if (_animationFrame < dword_45A0E4) {
				_animationFrame = dword_45A0E4;
				_animationFrameDelta = 1;
			}
			dword_45A0D8++;
		} else {
			_animationFrame += _animationFrameDelta;
			dword_45A0DC = 0;
			if (_animationFrame == 18 && Random_Query(0, 2)) {
				_animationFrameDelta = -1;
				dword_45A0D8 = 0;
				dword_45A0E4 = 14;
				dword_45A0E8 = 18;
				dword_45A0DC = Random_Query(0, 30);
			}
			if (_animationFrame == 26) {
				if (Random_Query(0, 2)) {
					_animationFrameDelta = -1;
					dword_45A0D8 = 0;
					dword_45A0E4 = 23;
					dword_45A0E8 = 26;
					dword_45A0DC = Random_Query(0, 30);
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(19)) {
				_animationFrame = 0;
				if (Random_Query(0, 2)) {
					dword_45A0D8 = 0;
					dword_45A0E4 = 0;
					dword_45A0E8 = 3;
					dword_45A0DC = Random_Query(0, 45);
				}
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(19) - 1;
			}
		}
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptMcCoy::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (Game_Flag_Query(kFlagMcCoyTiedDown)) {
			if (_animationFrame <= 6) {
				int random = Random_Query(0, 2);
				int soundId = 0;
				if (random == 0) {
					soundId = 595;
				} else if (random == 1) {
					soundId = 594;
				} else if (random == 2) {
					soundId = 593;
				}
				Ambient_Sounds_Play_Sound(soundId, 39, 0, 0, 99);
			}
			_animationState = 57;
			_animationFrame = 0;
			return true;
		}
		switch (_animationState) {
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			Game_Flag_Set(kFlagMcCoyAnimation1);
			dword_45A0D8 = 0;
			dword_45A0DC = 30;
			dword_45A0E4 = 0;
			_animationFrameDelta = 1;
			dword_45A0E8 = 3;
			return true;
		case 14:
		case 17:
		case 20:
		case 21:
		case 36:
			_animationState = 16;
			_animationFrame = 0;
			return true;
		case 15:
			_animationState = 16;
			_animationFrame = 16 - ((16 * _animationFrame) / 12);
			return true;
		case 16:
		case 25:
		case 26:
		case 55:
			return true;
		case 60:
			_animationState = 61;
			return true;
		default:
			_animationState = 0;
			_animationFrame = 0;
			dword_45A0DC = 0;
			return true;
		}
		break;

	case kAnimationModeWalk:
		if (_animationState != 27
		 && _animationState != 50
		) {
			if (Game_Flag_Query(kFlagMcCoyTiedDown)) {
				_animationState = 58;
				_animationFrame = 0;
			} else {
				_animationState = 32;
				_animationFrame = 0;
			}
		}
		break;

	case kAnimationModeRun:
		if (_animationState != 27
		 && _animationState != 50
		) {
			if (Game_Flag_Query(kFlagMcCoyTiedDown)) {
				_animationState = 58;
				_animationFrame = 4;
			} else {
				_animationState = 31;
				_animationFrame = 0;
			}
		}
		break;

	case kAnimationModeTalk:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 3;
			_animationNext = 20;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 4;
			dword_45A0F4 = 20;
		}
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 14:
		case 15:
		case 19:
		case 21:
			break;
		case 16:
			_animationState = 15;
			_animationFrame = 12 - ((12 * _animationFrame) / 16);
			break;
		case 17:
		case 20:
			_animationState = 19;
			_animationFrame = 0;
			break;
		case 18:
			_animationState = 19;
			_animationFrame = 13 * ((8 - _animationFrame) / 8);
			break;
		case 22:
			_animationState = 19;
			_animationFrame = 41;
			break;
		case 36:
		case 37:
		case 40:
		case 41:
		case 51:
			_animationState = 14;
			_animationFrame = 0;
			break;
		default:
			_animationState = 15;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatAim:
		switch (_animationState) {
		case 17:
		case 18:
		case 21:
			break;
		case 19:
			_animationState = 18;
			_animationFrame = 8 * (13 - _animationFrame) / 13;
			break;
		case 22:
			_animationState = 22;
			_animationFrame = 0;
			break;
		default:
			_animationState = 18;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatAttack:
		_animationState = 21;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalk:
		if (_animationState != 27 && _animationState != 50) {
			_animationState = 36;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatRun:
		if (_animationState != 27 && _animationState != 50) {
			_animationState = 37;
			_animationFrame = 0;
		}
		break;

	case 9:
	case 12:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 5;
			_animationNext = 21;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 5;
			dword_45A0F4 = 21;
		}
		break;

	case 10:
	case 13:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 6;
			_animationNext = 27;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 6;
			dword_45A0F4 = 27;
		}
		break;

	case 11:
	case 14:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 7;
			_animationNext = 22;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 7;
			dword_45A0F4 = 22;
		}
		break;

	case 15:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 8;
			_animationNext = 23;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 8;
			dword_45A0F4 = 23;
		}
		break;

	case 16:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 9;
			_animationNext = 24;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 9;
			dword_45A0F4 = 24;
		}
		break;

	case 17:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 10;
			_animationNext = 25;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 10;
			dword_45A0F4 = 25;
		}
		break;

	case 18:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 11;
			_animationNext = 26;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 11;
			dword_45A0F4 = 26;
		}
		break;

	case 19:
		if (_animationState < 3 || _animationState > 12) {
			_animationState = 13;
			_animationStateNext = 12;
			_animationNext = 27;
		} else {
			Game_Flag_Reset(kFlagMcCoyAnimation1);
			dword_45A0F0 = 12;
			dword_45A0F4 = 27;
		}
		break;

	case 20:
		dodge();
		break;

	case kAnimationModeHit:
		switch (_animationState) {
		case 14:
		case 15:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			if (Random_Query(0, 1) == 1) {
				_animationState = 24;
			} else {
				_animationState = 23;
			}
			_animationFrame = 0;
			break;
		default:
			if (Random_Query(0, 1) == 1) {
				_animationState = 26;
			} else {
				_animationState = 25;
			}
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatHit:
		if (Random_Query(0, 1) == 1) {
			_animationState = 23;
		} else {
			_animationState = 24;
		}
		_animationFrame = 0;
		break;

	case 23:
		_animationState = 66;
		_animationFrame = 0;
		break;

	case 29:
		Player_Loses_Control();
		_animationFrame = 0;
		_animationState = 71;
		break;

	case 38:
		_animationState = 47;
		_animationFrame = 0;
		break;

	case 39:
		_animationFrame = 0;
		_animationState = 52;
		if (Player_Query_Current_Set() == kSetCT02) {
			_animationFrame = 23;
		}
		break;

	case 40:
		_animationState = 44;
		_animationFrame = 0;
		break;

	case 41:
		_animationState = 45;
		_animationFrame = 0;
		break;

	case 42:
		_animationState = 46;
		_animationFrame = 0;
		Player_Loses_Control();
		break;

	case kAnimationModeWalkUp:
		_animationState = 38;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkDown:
		_animationState = 39;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkUp:
		_animationState = 40;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkDown:
		_animationState = 41;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		switch (_animationState) {
		case 14:
		case 15:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
			_animationState = 28;
			_animationFrame = 0;
			break;
		case 27:
		case 28:
		case 50:
			break;
		default:
			_animationState = 27;
			_animationFrame = 0;
			break;
		}
		break;
	case kAnimationModeCombatDie:
		_animationState = 28;
		_animationFrame = 0;
		break;
	case 51:
		_animationState = 27;
		_animationFrame = 0;
		break;
	case kAnimationModeFeeding:
		_animationState = 55;
		_animationFrame = 0;
		break;
	case kAnimationModeSit:
		if (_animationState != 60 && (Player_Query_Current_Set() == kSetNR03 || Player_Query_Current_Set() == kSetNR05_NR08)) {
			_animationState = 60;
			_animationFrame = 0;
		} else {
			_animationFrame = 0;
			_animationState = 70;
		}
		break;
	case kAnimationModeClimbUp:
	case kAnimationModeCombatClimbUp:
		_animationState = 42;
		_animationFrame = 0;
		break;
	case kAnimationModeClimbDown:
	case kAnimationModeCombatClimbDown:
		_animationState = 43;
		_animationFrame = 0;
		break;
	case 68:
		_animationState = 29;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		_animationFrameDelta = -1;
		break;
	case 75:
		_animationState = 67;
		_animationFrame = 0;
		break;
	case 85:
		_animationFrame = 0;
		_animationState = 69;
		break;
	}
	return true;
}

void AIScriptMcCoy::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMcCoy::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMcCoy::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMcCoy::FledCombat() {}

void AIScriptMcCoy::sub_4053E0() {
	float x, y, z;
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	flt_462710 = flt_462710 + off_45A100;
	if (flt_462714 < flt_462710) {
		off_45A100 = off_45A100 - 0.2f;
	} else {
		flt_462710 = flt_462714;
		Actor_Set_Goal_Number(kActorMcCoy, 0);
		Actor_Retired_Here(kActorMcCoy, 12, 48, 1, -1);
	}
	return Actor_Set_At_XYZ(kActorMcCoy, x, flt_462710, z, Actor_Query_Facing_1024(kActorMcCoy));
}

void AIScriptMcCoy::sub_4054F0() {
	if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetUG15 && Actor_Query_Goal_Number(kActorMcCoy) != 390 && !Game_Flag_Query(682)) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if ((z < 220.0f) && (-210.0f < x) && (-70.0f > x)) {
			Game_Flag_Set(682);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(kSceneLoopModeOnce, 2, true);
			Actor_Set_Goal_Number(kActorMcCoy, 390);
			Actor_Query_XYZ(kActorFreeSlotA, &x, &y, &z);
			if (-200.0 < x && -62.0f > x) {
				Actor_Set_Goal_Number(kActorFreeSlotA, 309);
			}
		} else {
			Actor_Set_Goal_Number(kActorMcCoy, 391);
		}
	}
}

void AIScriptMcCoy::sub_405660() {
	if (Game_Flag_Query(kFlagMcCoyTiedDown)) {
		if (_animationFrame <= 6) {
			int v1 = Random_Query(0, 2);
			int v2 = 0;
			if (v1 == 0) {
				v2 = 595;
			} else if (v1 == 1) {
				v2 = 594;
			} else if (v1 == 2) {
				v2 = 593;
			}
			Ambient_Sounds_Play_Sound(v2, 39, 0, 0, 99);
		}
		_animationState = 57;
		_animationFrame = 0;
		return;
	}
	switch (_animationState) {
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		Game_Flag_Set(kFlagMcCoyAnimation1);
		dword_45A0D8 = 0;
		dword_45A0DC = 30;
		dword_45A0E4 = 0;
		_animationFrameDelta = 1;
		dword_45A0E8 = 3;
		break;
	case 14:
		_animationState = 16;
		_animationFrame = 0;
		break;
	case 15:
		_animationState = 16;
		_animationFrame = 16 - 16 * _animationFrame / 12;
		break;
	case 17:
	case 20:
	case 21:
	case 36:
		_animationState = 16;
		_animationFrame = 0;
		break;
	case 16:
	case 25:
	case 26:
		break;
	case 60:
		_animationState = 61;
		break;
	default:
		_animationState = 0;
		_animationFrame = 0;
		dword_45A0DC = 0;
	}
}

void AIScriptMcCoy::sub_405800() {
	switch (_animationState) {
	case 36:
	case 37:
	case 40:
	case 41:
	case 51:
		_animationState = 14;
		_animationFrame = 0;
		break;
	case 22:
		_animationState = 19;
		_animationFrame = 41;
		break;
	case 18:
		_animationState = 19;
		_animationFrame = 13 * ((8 - _animationFrame) / 8);
		break;
	case 17:
	case 20:
		_animationState = 19;
		_animationFrame = 0;
		break;
	case 16:
		_animationFrame = 12 - 12 * _animationFrame / 16;
		_animationState = 15;
		break;
	case 14:
	case 15:
	case 19:
	case 21:
		break;
	default:
		_animationState = 15;
		_animationFrame = 0;
		break;
	}
}

void AIScriptMcCoy::dodge() {
	int setId = Actor_Query_Which_Set_In(kActorMcCoy);
	if (setId == kSetCT02) {
		_animationState = 51;
		_animationFrame = 0;
		Player_Loses_Control();
		Game_Flag_Set(kFlagCT02ZubenFled);
	} else if (setId == kSetRC03 || setId == kSetUG15) {
		_animationState = 56;
		_animationFrame = 0;
	}
}

void AIScriptMcCoy::sub_405920() {
	_animationFrame = 0;
	_animationState = 47;
}

void AIScriptMcCoy::sub_405940(float a1) {
	float x, y, z;
	int currentAngle = Actor_Query_Facing_1024(kActorMcCoy);
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	int angle = currentAngle - 12;
	y = y + a1;
	if (angle < 0) {
		angle = currentAngle + 1012;
	}
	if (angle > 1023) {
		angle -= 1024;
	}
	Actor_Set_At_XYZ(kActorMcCoy, x, y, z, angle);
}

void AIScriptMcCoy::sub_4059D0(float a1) {
	float x, y, z;
	int currentAngle = Actor_Query_Facing_1024(kActorMcCoy);
	Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
	int angle = currentAngle + 15;
	y = y + a1;
	if (angle < 0) {
		angle = currentAngle + 1039;
	}
	if (angle > 1023) {
		angle -= 1024;
	}
	return Actor_Set_At_XYZ(kActorMcCoy, x, y, z, angle);
}

} // End of namespace BladeRunner
