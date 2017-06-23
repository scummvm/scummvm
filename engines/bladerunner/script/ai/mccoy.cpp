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

#include "bladerunner/script/ai.h"

namespace BladeRunner {

void AIScriptMcCoy::Initialize() {
	dword_45A0D0_animation_state = 0;
	dword_45A0D4_animation_frame = 0;
	dword_45A0D8 = 0;
	dword_45A0DC = 30;
	dword_45A0E0 = 1;
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
	case 101:
		Actor_Set_Goal_Number(kActorMcCoy, 102);
		return true;
	case 102:
		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 316) < 36 && !Game_Flag_Query(375)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 21);
			Game_Flag_Set(375);
		}
		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 316) < 4) {
			Actor_Set_Goal_Number(kActorMcCoy, 103);
		}
		return true;
	case 201:
		Actor_Set_Goal_Number(kActorMcCoy, 0);
		if (Player_Query_Current_Set() == 55) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -166.0f, -70.19f, -501.0f, 0, 0, false, 0);
			Actor_Face_Heading(kActorMcCoy, 300, false);
		} else {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -461.0f, 0.0f, -373.0f, 0, 0, false, 0);
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
	case 8:
	case 9:
		Spinner_Set_Selectable_Destination_Flag(3, 1);
		break;
	case 44:
	case 47:
		Spinner_Set_Selectable_Destination_Flag(4, 1);
		break;
	case 53:
	case 263:
		if (Query_Difficulty_Level() == 0) {
			Spinner_Set_Selectable_Destination_Flag(4, 1);
		}
		break;
	case 90:
		Spinner_Set_Selectable_Destination_Flag(8, 1);
		break;
	case 84:
	case 113:
	case 114:
	case 115:
	case 118:
		Spinner_Set_Selectable_Destination_Flag(9, 1);
		Spinner_Set_Selectable_Destination_Flag(8, 1);
		break;
	case 66:
	case 80:
	case 82:
	case 83:
		Global_Variable_Increment(49, 1);
		break;
	case 125:
		Global_Variable_Increment(49, 5);
		break;
	case 124:
	case 128:
		Global_Variable_Increment(49, 3);
		break;
	case 120:
	case 121:
		Global_Variable_Increment(49, 2);
		break;
	case 147:
	case 148:
	case 149:
	case 150:
	case 151:
	case 152:
		Global_Variable_Increment(48, 1);
		break;
	}
	if (Global_Variable_Query(49) > 6 && Global_Variable_Query(1) > 3 && !Actor_Clue_Query(kActorMcCoy, kClueGuzzaFramedMcCoy)) {
		Delay(500);
		Actor_Voice_Over(3320, kActorVoiceOver);
		switch (clueId) {
		case 66:
		case 80:
		case 82:
		case 83:
		case 121:
		case 128:
			Actor_Voice_Over(3340, kActorVoiceOver);
			Actor_Voice_Over(3350, kActorVoiceOver);
			Actor_Voice_Over(3360, kActorVoiceOver);
			Actor_Voice_Over(3370, kActorVoiceOver);
			Actor_Voice_Over(3380, kActorVoiceOver);
			break;
		case 124:
			Actor_Voice_Over(3330, kActorVoiceOver);
			break;
		case 120:
			Actor_Voice_Over(3390, kActorVoiceOver);
			Actor_Voice_Over(3400, kActorVoiceOver);
			Actor_Voice_Over(3420, kActorVoiceOver);
			break;
		}
		Actor_Clue_Acquire(kActorMcCoy, kClueGuzzaFramedMcCoy, 1, -1);
		if (clueId == 125) {
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

void AIScriptMcCoy::ShotAtAndHit() {
}

void AIScriptMcCoy::Retired(int byActorId) {
	if (byActorId == kActorSteele && Actor_Query_In_Set(kActorSteele, 42)) {
		if (Actor_Query_In_Set(kActorDektora, 42) && Actor_Query_Goal_Number(kActorDektora) != 599) {
			Non_Player_Actor_Combat_Mode_On(kActorSteele, 3, 1, kActorDektora, 15, 4, 7, 8, 0, 0, 100, 25, 300, 0);
		} else if (Actor_Query_In_Set(kActorLucy, 42) && Actor_Query_Goal_Number(kActorLucy) != 599) {
			Non_Player_Actor_Combat_Mode_On(kActorSteele, 3, 1, kActorLucy, 15, 4, 7, 8, 0, 0, 100, 25, 300, 0);
		}
	}
	if (Actor_Query_In_Set(kActorMcCoy, 41) && Actor_Query_In_Set(kActorOfficerLeary, 41) && Actor_Query_In_Set(kActorDektora, 41) && Actor_Query_Goal_Number(kActorDektora) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 3, 1, kActorDektora, 4, 4, 7, 8, 0, 0, 100, 25, 300, 0);
	}
	if (Actor_Query_In_Set(kActorMcCoy, 41) && Actor_Query_In_Set(kActorOfficerGrayford, 41) && Actor_Query_In_Set(kActorDektora, 41) && Actor_Query_Goal_Number(kActorDektora) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, 3, 1, kActorDektora, 4, 4, 7, 8, 0, 0, 100, 25, 300, 0);
	}
	if (Actor_Query_In_Set(kActorMcCoy, 41) && Actor_Query_In_Set(kActorOfficerLeary, 41) && Actor_Query_In_Set(kActorLucy, 41) && Actor_Query_Goal_Number(kActorLucy) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerLeary, 3, 1, kActorLucy, 4, 4, 7, 8, 0, 0, 100, 25, 300, 0);
	}
	if (Actor_Query_In_Set(kActorMcCoy, 41) && Actor_Query_In_Set(kActorOfficerGrayford, 41) && Actor_Query_In_Set(kActorLucy, 41) && Actor_Query_Goal_Number(kActorLucy) != 599) {
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, 3, 1, kActorLucy, 4, 4, 7, 8, 0, 0, 100, 25, 300, 0);
	}
}

int AIScriptMcCoy::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMcCoy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	unsigned int v5;
	unsigned int v7;

	switch (newGoalNumber) {
	case 0:
		return true;
	case 1:
		sub_4058B0();
		return true;
	case 2:
		sub_405920();
		return true;
	case 100:
		Actor_Set_At_Waypoint(kActorMcCoy, 315, 263);
		dword_45A0D0_animation_state = 53;
		dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		Actor_Set_Invisible(kActorMcCoy, false);
		return true;
	case 102:
		Async_Actor_Walk_To_Waypoint(kActorMcCoy, 316, 0, 1);
		return true;
	case 103:
		Actor_Face_Actor(kActorMcCoy, kActorSadik, true);
		return true;
	case 231:
		Player_Set_Combat_Mode(false);
		Preload(18);
		Set_Enter(59, 63);
		Player_Loses_Control();
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Put_In_Set(kActorMcCoy, 59);
		Actor_Set_At_XYZ(kActorMcCoy, 14.0f, 110.84f, -300.0f, 926);
		Actor_Change_Animation_Mode(kActorMcCoy, 48);
		dword_45A0D0_animation_state = 27;
		dword_45A0D4_animation_frame = 0;
		flt_462714 = 2.84f;
		flt_462710 = 110.84f;
		off_45A100 = -6.0f;
		return true;
	case 230:
		dword_45A0FC = Actor_Query_Goal_Number(kActorSteele) == 215;
		Actor_Change_Animation_Mode(kActorMcCoy, 6);
		return true;
	case 220:
		Actor_Change_Animation_Mode(kActorMcCoy, 75);
		return true;
	case 212:
		Global_Variable_Set(47, 0);
		Player_Set_Combat_Mode_Access(false);
		Player_Gains_Control();
		Scene_Exits_Disable();
		dword_45A0D0_animation_state = 68;
		dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
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
		Actor_Change_Animation_Mode(kActorMcCoy, 0);
		Actor_Set_Goal_Number(kActorMcCoy, 0);
		Player_Gains_Control();
		return true;
	case 210:
		Actor_Put_In_Set(kActorMcCoy, 54);
		Actor_Set_At_XYZ(kActorMcCoy, -204.0, 24.0, -817.0, 256);
		Actor_Set_Invisible(kActorMcCoy, false);
		if (Game_Flag_Query(627)) {
			Actor_Set_Goal_Number(kActorMcCoy, 212);
		} else {
			dword_45A0D0_animation_state = 53;
			dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
			Actor_Set_Invisible(kActorMcCoy, false);
		}
		return true;
	case 200:
		Player_Loses_Control();
		Actor_Change_Animation_Mode(kActorMcCoy, 0);
		return true;
	case 301:
		dword_45A0D0_animation_state = 62;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 302:
		dword_45A0D0_animation_state = 64;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 303:
		dword_45A0D0_animation_state = 65;
		dword_45A0D4_animation_frame = 0;
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
		if (dword_45A0D0_animation_state != 27 && dword_45A0D0_animation_state != 50) {
			dword_45A0D0_animation_state = 50;
			dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		}
		return true;
	case 400:
		Actor_Set_Health(kActorMcCoy, 50, 50);
		Game_Flag_Set(373);
		v5 = Global_Variable_Query(45);
		if (v5 == 1) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 3);
		} else if (v5 == 2) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 3);
		} else if (v5 == 3) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, -5);
			Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 5);
		}
		if (Game_Flag_Query(666)) {
			Actor_Modify_Friendliness_To_Other(kActorSteele, kActorMcCoy, 3);
		}
		if (Actor_Query_Friendliness_To_Other(kActorSteele, kActorMcCoy) < Actor_Query_Friendliness_To_Other(kActorClovis, kActorMcCoy)) {
			Game_Flag_Set(653);
		}
		v7 = Global_Variable_Query(45);
		if (v7 == 1) {
			if (Game_Flag_Query(653)) {
				Global_Variable_Set(45, 0);
			}
		} else if (v7 == 2 || v7 == 3) {
			if (!Game_Flag_Query(653)) {
				Global_Variable_Set(45, 0);
			}
		}
		if (!Game_Flag_Query(653)) {
			Game_Flag_Set(461);
		}
		Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
		Ambient_Sounds_Remove_All_Looping_Sounds(1);
		Global_Variable_Set(1, 5);
		Outtake_Play(10, 0, -1);
		if (Game_Flag_Query(666)) {
			Chapter_Enter(5, 53, 53);
		} else {
			Game_Flag_Set(34);
			Chapter_Enter(5, 10, 49);
		}
		return true;
	case 500:
		Music_Stop(3);
		Player_Set_Combat_Mode(false);
		Actor_Change_Animation_Mode(kActorMcCoy, 0);
		dword_45A0D0_animation_state = 0;
		dword_45A0D4_animation_frame = 0;
		Game_Flag_Set(465);
		Set_Enter(67, 72);
		return true;
	}
	return false;
}

bool AIScriptMcCoy::UpdateAnimation(int *animation, int *frame) {
	int v7, v18, v19, v46;
	switch (dword_45A0D0_animation_state) {
	case 71:
		*animation = 52;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(52)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
			Player_Gains_Control();
		}
		break;
	case 70:
		*animation = 51;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(51)) {
			dword_45A0D4_animation_frame = 0;
		}
		break;
	case 69:
		*animation = 50;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(50)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 53);
			*animation = 51;
		}
		break;
	case 68:
		*animation = 18;
		v7 = Slice_Animation_Query_Number_Of_Frames(*animation) - 1 - Global_Variable_Query(47);
		if (dword_45A0D4_animation_frame < v7) {
			dword_45A0D4_animation_frame++;
		} else if (dword_45A0D4_animation_frame > v7) {
			dword_45A0D4_animation_frame--;
		}
		if (dword_45A0D4_animation_frame <= 0) {
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
			*animation = 19;
			dword_45A0D0_animation_state = 0;
			dword_45A0D4_animation_frame = 0;
			Game_Flag_Reset(627);
			Scene_Exits_Enable();
			Player_Set_Combat_Mode_Access(true);
			Actor_Set_Goal_Number(kActorMcCoy, 0);
		}
		break;
	case 67:
		*animation = 53;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(53)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
			*animation = 19;
			dword_45A0D0_animation_state = 0;
			dword_45A0D4_animation_frame = 0;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 220) {
				Actor_Change_Animation_Mode(kActorMcCoy, 48);
			}
		}
		break;
	case 66:
		*animation = 40;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(40)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
		}
		break;
	case 65:
		*animation = 45;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(45)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 14;
			*animation = 0;
			Actor_Set_Goal_Number(kActorMcCoy, 0);
		}
		break;
	case 64:
		*animation = 44;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(44)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 63;
			*animation = 43;
		}
		break;
	case 63:
		*animation = 43;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(43)) {
			dword_45A0D4_animation_frame = 0;
		}
		break;
	case 62:
		*animation = 42;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(42)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 63;
			*animation = 43;
		}
		break;
	case 61:
		*animation = 41;
		dword_45A0D4_animation_frame--;
		if (dword_45A0D4_animation_frame <= 0) {
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 200) {
				Actor_Set_Goal_Number(kActorMcCoy, 201);
			}
		}
		break;
	case 60:
		*animation = 41;
		if (dword_45A0D4_animation_frame < Slice_Animation_Query_Number_Of_Frames(41) - 1) {
			dword_45A0D4_animation_frame++;
		}
		return true;
	case 59:
		*animation = 48;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(48)) {
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0DC = 0;
			dword_45A0D0_animation_state = 0;
			Player_Gains_Control();
			Item_Add_To_World(109, 982, 6, -110.0, 0.0, -192.0, 0, 48, 32, false, true, false, false);
		}
		break;
	case 58:
		*animation = 47;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame == 6) {
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
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(*animation) - 1) { //why -1? 
			dword_45A0D4_animation_frame = 0;
		}
		break;
	case 57:
		*animation = 46;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(46)) {
			dword_45A0D4_animation_frame = 0;
		}
		if (!Game_Flag_Query(550)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 59;
			*animation = 48;
		}
		break;
	case 56:
		*animation = 49;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(49)) {
			dword_45A0D4_animation_frame = 0;
			if (Actor_Query_Which_Set_In(kActorMcCoy) == 87) {
				dword_45A0D0_animation_state = 27;
			} else {
				*animation = 19;
				dword_45A0D0_animation_state = 0;
				Actor_Change_Animation_Mode(kActorMcCoy, 0);
			}
		}
		break;
	case 55:
		*animation = 32;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame == 7) {
			Actor_Change_Animation_Mode(kActorMaggie, 52);
		}
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = 19;
			dword_45A0D0_animation_state = 0;
		}
		break;
	case 53:
		*animation = 18;
		dword_45A0D4_animation_frame--;
		if (dword_45A0D4_animation_frame <= 0) {
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 100) {
				Actor_Set_Goal_Number(kActorMcCoy, 101);
			}
			if (Actor_Query_Goal_Number(kActorMcCoy) == 210) {
				Actor_Set_Goal_Number(kActorMcCoy, 211);
			}
		}
		break;
	case 52:
		*animation = 31;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(31)) {
			dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(31) - 1;
			dword_45A0D0_animation_state = 50;
		}
		break;
	case 51:
		*animation = 28;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(28)) {
			Player_Set_Combat_Mode(true);
			sub_405800();
			Actor_Set_Goal_Number(kActorMcCoy, 0);
			dword_45A0D4_animation_frame = 0;
			Player_Gains_Control();
		}
		break;
	case 50:
		*animation = 18;
		dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		break;
	case 49:
		*animation = 34;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(34)) {
			Actor_Set_Goal_Number(kActorMcCoy, 0);
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			sub_405660();
		}
		break;
	case 48:
		*animation = 33;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(33)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 49;
			*animation = 34;
		}
		break;
	case 47:
		*animation = 29;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(29)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 48;
			*animation = 33;
		}
		break;
	case 46:
		Actor_Set_Invisible(kActorMcCoy, false);
		*animation = 36;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(36)) {
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			Player_Gains_Control();
			sub_405660();
			Actor_Face_Heading(kActorMcCoy, (Actor_Query_Facing_1024(kActorMcCoy) + 512) & 1023, false);
		}
		break;
	case 45:
		*animation = 35;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(35)) {
			Actor_Set_Invisible(kActorMcCoy, true);
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			sub_405660();
		}
		break;
	case 44:
		*animation = 30;
		if (dword_45A0D4_animation_frame++ == 127) {
			Game_Flag_Set(325);
		}
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(30)) {
			*animation = 19;
			sub_405660();
			Actor_Set_At_XYZ(kActorMcCoy, -203.41f, -621.3f, 724.57f, 538);
			Player_Gains_Control();
		}
		break;
	case 43:
		*animation = 38;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(38) - 1) { //why -1? 
			dword_45A0D4_animation_frame = 0;
		}
		switch (dword_45A0D4_animation_frame) {
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
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(37) - 1) { //why -1? 
			dword_45A0D4_animation_frame = 0;
		}
		switch (dword_45A0D4_animation_frame) {
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
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(7)) {
			dword_45A0D4_animation_frame = 0;
			++off_45A0EC;
		}
		if (dword_45A0D4_animation_frame == 9) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (dword_45A0D4_animation_frame == 4) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(359)) {
			sub_4059D0(-0.2f);
		}
		break;
	case 40:
		*animation = 6;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(6)) {
			dword_45A0D4_animation_frame = 0;
			++off_45A0EC;
		}
		if (dword_45A0D4_animation_frame == 8) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (dword_45A0D4_animation_frame == 3) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(358)) {
			sub_405940(0.5f);
		}
		break;
	case 39:
		*animation = 16;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(16)) {
			dword_45A0D4_animation_frame = 0;
			++off_45A0EC;
		}
		if (dword_45A0D4_animation_frame == 8) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (dword_45A0D4_animation_frame == 3) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(359)) {
			sub_4059D0(-0.2f);
		}
		break;
	case 38:
		*animation = 15;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(15)) {
			dword_45A0D4_animation_frame = 0;
			++off_45A0EC;
		}
		if (dword_45A0D4_animation_frame == 9) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		} else if (dword_45A0D4_animation_frame == 4) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		if (Game_Flag_Query(358)) {
			sub_405940(0.5f);
		}
		break;
	case 37:
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(4)) {
			dword_45A0D4_animation_frame = 0;
		}
		*animation = 4;
		if (dword_45A0D4_animation_frame == 6) {
			Sound_Left_Footstep_Run(kActorMcCoy);
		}
		if (dword_45A0D4_animation_frame == 0) {
			Sound_Right_Footstep_Run(kActorMcCoy);
		}
		break;
	case 36:
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(3)) {
			dword_45A0D4_animation_frame = 0;
		}
		*animation = 3;
		if (dword_45A0D4_animation_frame == 15) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		}
		if (dword_45A0D4_animation_frame == 6) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		}
		break;
	case 32:
		dword_45A0D4_animation_frame = 1;
		dword_45A0D0_animation_state = 30;
		*animation = 13;
		break;
	case 31:
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(14)) {
			dword_45A0D4_animation_frame = 0;
		}
		*animation = 14;
		if (dword_45A0D4_animation_frame == 5) {
			Sound_Left_Footstep_Run(kActorMcCoy);
		} else if (dword_45A0D4_animation_frame == 12) {
			Sound_Right_Footstep_Run(kActorMcCoy);
		}
		break;
	case 30:
		*animation = 13;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(13)) {
			dword_45A0D4_animation_frame = 0;
		}
		if (dword_45A0D4_animation_frame == 2) {
			Sound_Right_Footstep_Walk(kActorMcCoy);
		} else if (dword_45A0D4_animation_frame == 10) {
			Sound_Left_Footstep_Walk(kActorMcCoy);
		}
		break;
	case 29:
		v46 = dword_45A0D4_animation_frame + dword_45A0E0;
		*animation = 18;
		dword_45A0D4_animation_frame = v46;
		if (v46 < 14) {
			dword_45A0E0 = 1;
		}
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
			*animation = 18;
			dword_45A0D0_animation_state = 27;
			dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		}
		break;
	case 28:
		*animation = 5;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(5)) {
			dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			dword_45A0D0_animation_state = 50;
			sub_4054F0();
		}
		break;
	case 27:
		*animation = 18;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(18)) {
			dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			dword_45A0D0_animation_state = 50;
			sub_4054F0();
			if (Actor_Query_Goal_Number(kActorMcCoy) == 220) {
				Actor_Set_Goal_Number(kActorMcCoy, 221);
			}
		}
		break;
	case 26:
		*animation = 17;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(17)) {
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
		}
		break;
	case 25:
		*animation = 17;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(17)) {
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, 0);
		}
		break;
	case 24:
		*animation = 1;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(1)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 14;
			*animation = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, 4);
		}
		break;
	case 23:
		*animation = 1;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(1)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 14;
			*animation = 0;
			Actor_Change_Animation_Mode(kActorMcCoy, 4);
		}
		break;
	case 22:
		dword_45A0D4_animation_frame = 0;
		dword_45A0D0_animation_state = 17;
		*animation = 12;
		break;
	case 21:
		*animation = 12;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame == 1 && Actor_Query_Goal_Number(kActorMcCoy) == 230 && dword_45A0FC == 1) {
			dword_45A0F8 = 27;
		}
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(12)) {
			dword_45A0D0_animation_state = 17;
			dword_45A0D4_animation_frame = 0;
			*animation = 12;
			if (Actor_Query_Goal_Number(kActorMcCoy) == 230) {
				dword_45A0D4_animation_frame = 0;
				dword_45A0D0_animation_state = 21;
				dword_45A0FC = 1;
				*animation = 12;
			}
		}
		break;
	case 19:
		*animation = 11;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= 12) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 14;
			*animation = 0;
		}
		break;
	case 18:
		*animation = 10;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(10)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 17;
			*animation = 12;
		}
		break;
	case 17:
		*animation = 12;
		dword_45A0D4_animation_frame = 0;
		// weird, but thats in game code
		if (Slice_Animation_Query_Number_Of_Frames(12) <= 0) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 17;
		}
		break;
	case 16:
		*animation = 9;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(9)) {
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
		}
		break;
	case 15:
		*animation = 8;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(8)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 14;
			*animation = 0;
		}
		break;
	case 14:
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(0)) {
			dword_45A0D0_animation_state = 14;
			dword_45A0D4_animation_frame = 0;
		}
		*animation = 0;
		break;
	case 13:
		*animation = 19;
		if (dword_45A0D4_animation_frame < Slice_Animation_Query_Number_Of_Frames(19) / 2) {
			dword_45A0D4_animation_frame -= 3;
			if (dword_45A0D4_animation_frame <= 0) {
				dword_45A0D4_animation_frame = 0;
				*animation = dword_46271C;
				dword_45A0D0_animation_state = dword_462718;
			}
		} else {
			dword_45A0D4_animation_frame += 3;
			if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(19)) {
				dword_45A0D4_animation_frame = 0;
				*animation = dword_46271C;
				dword_45A0D0_animation_state = dword_462718;
			}
		}
		break;
	case 12:
		*animation = 27;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(27)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 11:
		*animation = 26;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(26)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 10:
		*animation = 25;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(25)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 9:
		*animation = 24;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(24)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 8:
		*animation = 23;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(23)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 7:
		*animation = 22;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(22)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 6:
		*animation = 27;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(27)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 5:
		*animation = 21;
		dword_45A0D4_animation_frame++;
		if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(21)) {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 3;
			*animation = 20;
		}
		break;
	case 3:
	case 4:
		if (dword_45A0D4_animation_frame == 0 && !Game_Flag_Query(236)) {
			dword_45A0D4_animation_frame = 1;
			dword_45A0D0_animation_state = dword_45A0F0;
			*animation = dword_45A0F4;
			dword_45A0F0 = 4;
			dword_45A0F4 = 20;
		} else if (dword_45A0D4_animation_frame <= 4 && Game_Flag_Query(236)) {
			Game_Flag_Reset(236);
			*animation = 19;
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 0;
		} else {
			*animation = 20;
			dword_45A0D4_animation_frame++;
			if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(20)) {
				dword_45A0D4_animation_frame = 0;
			}
		}
		break;
	case 0:
		*animation = 19;
		if (dword_45A0D8 < dword_45A0DC) {
			//*frame = dword_45A0E8;
			dword_45A0D4_animation_frame += dword_45A0E0;
			if (dword_45A0D4_animation_frame > dword_45A0E8) {
				dword_45A0D4_animation_frame = dword_45A0E8;
				dword_45A0E0 = -1;
			} else if (dword_45A0D4_animation_frame < dword_45A0E4) {
				dword_45A0D4_animation_frame = dword_45A0E4;
				dword_45A0E0 = 1;
			}
			dword_45A0D8++;
		} else {
			dword_45A0D4_animation_frame += dword_45A0E0;
			dword_45A0DC = 0;
			if (dword_45A0D4_animation_frame == 18 && Random_Query(0, 2)) {
				dword_45A0E0 = -1;
				dword_45A0D8 = 0;
				dword_45A0E4 = 14;
				dword_45A0E8 = 18;
				dword_45A0DC = Random_Query(0, 30);
			}
			if (dword_45A0D4_animation_frame == 26) {
				if (Random_Query(0, 2)) {
					dword_45A0E0 = -1;
					dword_45A0D8 = 0;
					dword_45A0E4 = 23;
					dword_45A0E8 = 26;
					dword_45A0DC = Random_Query(0, 30);
				}
			}
			if (dword_45A0D4_animation_frame >= Slice_Animation_Query_Number_Of_Frames(19)) {
				dword_45A0D4_animation_frame = 0;
				if (Random_Query(0, 2)) {
					dword_45A0D8 = 0;
					dword_45A0E4 = 0;
					dword_45A0E8 = 3;
					dword_45A0DC = Random_Query(0, 45);
				}
			}
			if (dword_45A0D4_animation_frame < 0) {
				dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(19) - 1;
			}
		}
		break;
	}
	*frame = dword_45A0D4_animation_frame;
	return true;
}

bool AIScriptMcCoy::ChangeAnimationMode(int mode) {
	int v2;
	switch (mode) {
	case 85:
		dword_45A0D4_animation_frame = 0;
		dword_45A0D0_animation_state = 69;
		return true;
	case 75:
		dword_45A0D0_animation_state = 67;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 68:
		dword_45A0D0_animation_state = 29;
		dword_45A0D4_animation_frame = Slice_Animation_Query_Number_Of_Frames(18) - 1;
		dword_45A0E0 = -1;
		return true;
	case 65:
	case 67:
		dword_45A0D0_animation_state = 43;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 64:
	case 66:
		dword_45A0D0_animation_state = 42;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 53:
		if (dword_45A0D0_animation_state != 60 && (Player_Query_Current_Set() == 55 || Player_Query_Current_Set() == 13)) {
			dword_45A0D0_animation_state = 60;
			dword_45A0D4_animation_frame = 0;
		} else {
			dword_45A0D4_animation_frame = 0;
			dword_45A0D0_animation_state = 70;
		}
		return true;
	case 52:
		dword_45A0D0_animation_state = 55;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 51:
		dword_45A0D0_animation_state = 27;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 49:
		dword_45A0D0_animation_state = 28;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 48:
		switch (dword_45A0D0_animation_state) {
		case 14:
			dword_45A0D0_animation_state = 28;
			dword_45A0D4_animation_frame = 0;
			break;
		case 13:
			dword_45A0D0_animation_state = 22;
			dword_45A0D4_animation_frame = 0;
			return true;
		case 10:
			dword_45A0D0_animation_state = 18;
			dword_45A0D4_animation_frame = 8 * (13 - dword_45A0D4_animation_frame) / 13;
			return true;
		case 8:
		case 9:
		case 12:
			return true;
		case 11:
			dword_45A0D0_animation_state = 18;
			dword_45A0D4_animation_frame = 0;
			break;
		case 7:
			dword_45A0D0_animation_state = 14;
			dword_45A0D4_animation_frame = 0;
			break;
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			dword_45A0D0_animation_state = 15;
			dword_45A0D4_animation_frame = 0;
			break;
		default:
			if (dword_45A0D0_animation_state != 50) {
				dword_45A0D0_animation_state = 27;
				dword_45A0D4_animation_frame = 0;
			}
			break;
		}
		return true;
	case 47:
		dword_45A0D0_animation_state = 41;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 46:
		dword_45A0D0_animation_state = 40;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 45:
		dword_45A0D0_animation_state = 39;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 44:
		dword_45A0D0_animation_state = 38;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 42:
		dword_45A0D0_animation_state = 46;
		dword_45A0D4_animation_frame = 0;
		Player_Loses_Control();
		return true;
	case 41:
		dword_45A0D0_animation_state = 45;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 40:
		dword_45A0D0_animation_state = 44;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 39:
		dword_45A0D4_animation_frame = 0;
		dword_45A0D0_animation_state = 52;
		if (Player_Query_Current_Set() == 27) {
			dword_45A0D4_animation_frame = 23;
		}
		return true;
	case 38:
		dword_45A0D0_animation_state = 47;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 29:
		Player_Loses_Control();
		dword_45A0D4_animation_frame = 0;
		dword_45A0D0_animation_state = 71;
		return true;
	case 23:
		dword_45A0D0_animation_state = 66;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 22:
		if (Random_Query(0, 1)) {
			dword_45A0D0_animation_state = 23;
		} else {
			dword_45A0D0_animation_state = 24;
		}
		dword_45A0D4_animation_frame = 0;
		return true;
	case 21:
		switch (dword_45A0D0_animation_state) {
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			dword_45A0D0_animation_state = 28;
			dword_45A0D4_animation_frame = 0;
			break;
		case 1:
		case 8:
			dword_45A0D0_animation_state = 27;
			dword_45A0D4_animation_frame = 0;
			break;
		default:

			if (Random_Query(0, 1)) {
				dword_45A0D0_animation_state = 26;
			} else {
				dword_45A0D0_animation_state = 25;
			}
			dword_45A0D4_animation_frame = 0;
			break;
		}
		return true;
	case 20:
		v2 = Actor_Query_Which_Set_In(kActorMcCoy);
		if (v2 == 27) {
			dword_45A0D0_animation_state = 51;
			dword_45A0D4_animation_frame = 0;
			Player_Loses_Control();
			Game_Flag_Set(210);
		} else if (v2 == 70 || v2 == 87) {
			dword_45A0D0_animation_state = 56;
			dword_45A0D4_animation_frame = 0;
		}
		return true;
	case 19:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 12;
			dword_46271C = 27;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 12;
			dword_45A0F4 = 27;
		}
		return true;
	case 18:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 11;
			dword_46271C = 26;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 11;
			dword_45A0F4 = 26;
		}
		return true;
	case 17:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 10;
			dword_46271C = 25;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 10;
			dword_45A0F4 = 25;
		}
		return true;
	case 16:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 9;
			dword_46271C = 24;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 9;
			dword_45A0F4 = 24;
		}
		return true;
	case 15:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 8;
			dword_46271C = 23;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 8;
			dword_45A0F4 = 23;
		}
		return true;
	case 11:
	case 14:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 7;
			dword_46271C = 22;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 7;
			dword_45A0F4 = 22;
		}
		return true;
	case 10:
	case 13:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 6;
			dword_46271C = 27;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 6;
			dword_45A0F4 = 27;
		}
		return true;
	case 9:
	case 12:
		if (dword_45A0D0_animation_state < 3 || dword_45A0D0_animation_state > 12) {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 5;
			dword_46271C = 21;
		} else {
			Game_Flag_Reset(236);
			dword_45A0F0 = 5;
			dword_45A0F4 = 21;
		}
		return true;
	case 8:
		if (dword_45A0D0_animation_state != 27 && dword_45A0D0_animation_state != 50) {
			dword_45A0D0_animation_state = 37;
			dword_45A0D4_animation_frame = 0;
		}
		return true;
	case 7:
		if (dword_45A0D0_animation_state != 27 && dword_45A0D0_animation_state != 50) {
			dword_45A0D0_animation_state = 36;
			dword_45A0D4_animation_frame = 0;
		}
		return true;
	case 6:
		dword_45A0D0_animation_state = 21;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 5:
		switch (dword_45A0D0_animation_state) {
		case 1:
		case 2:
		case 5:
			dword_45A0D0_animation_state = 14;
			dword_45A0D4_animation_frame = 0;
			break;
		case 0:
		case 3:
		case 4:
			dword_45A0D0_animation_state = 15;
			dword_45A0D4_animation_frame = 0;
			break;
		default:
			dword_45A0D0_animation_state = 18;
			dword_45A0D4_animation_frame = 0;
			break;
		}
		return true;
	case 4:
		switch (dword_45A0D0_animation_state) {
		case 22:
			dword_45A0D0_animation_state = 19;
			dword_45A0D4_animation_frame = 41;
			break;
		case 18:
			dword_45A0D0_animation_state = 19;
			dword_45A0D4_animation_frame = 13 * ((8 - dword_45A0D4_animation_frame) / 8);
			break;
		case 17:
		case 20:
			dword_45A0D0_animation_state = 19;
			dword_45A0D4_animation_frame = 0;
			break;
		case 16:
			dword_45A0D0_animation_state = 15;
			dword_45A0D4_animation_frame = 12 - 12 * dword_45A0D4_animation_frame / 16;
			break;
		case 14:
		case 15:
		case 19:
		case 21:
		case 2:
		case 3:
			return true;
		default:
			dword_45A0D0_animation_state = 15;
			dword_45A0D4_animation_frame = 0;
			break;
		case 13:
			dword_45A0D0_animation_state = 16;
			dword_45A0D4_animation_frame = 0;
			break;
		case 0:
		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			dword_45A0D0_animation_state = 0;
			dword_45A0D4_animation_frame = 0;
			dword_45A0DC = 0;
			break;
		}
		return true;
	case 3:
		if (dword_45A0D0_animation_state >= 3 && dword_45A0D0_animation_state <= 12) {
			Game_Flag_Reset(236);
			dword_45A0F0 = 4;
			dword_45A0F4 = 20;
		} else {
			dword_45A0D0_animation_state = 13;
			dword_462718 = 3;
			dword_46271C = 20;
		}
		return true;
	case 2:
		if (dword_45A0D0_animation_state == 27 || dword_45A0D0_animation_state == 50) {
			return true;
		}
		if (!Game_Flag_Query(550)) {
			dword_45A0D0_animation_state = 31;
			dword_45A0D4_animation_frame = 0;
			return true;
		}
		dword_45A0D0_animation_state = 58;
		dword_45A0D4_animation_frame = 4;
		return true;
	case 1:
		if (dword_45A0D0_animation_state == 27 || dword_45A0D0_animation_state == 50) {
			return true;
		}
		if (!Game_Flag_Query(550)) {
			dword_45A0D0_animation_state = 32;
			dword_45A0D4_animation_frame = 0;
			return true;
		}
		dword_45A0D0_animation_state = 58;
		dword_45A0D4_animation_frame = 0;
		return true;
	case 0:
		if (Game_Flag_Query(550)) {
			if (dword_45A0D4_animation_frame > 6) {
				dword_45A0D0_animation_state = 57;
				dword_45A0D4_animation_frame = 0;
				return true;
			}
			int v3 = Random_Query(0, 2);
			int v4 = 0;
			if (v3 == 0) {
				v4 = 595;
			} else if (v3 == 1) {
				v4 = 594;
			} else if (v3 == 2) {
				v4 = 593;
			}
			Ambient_Sounds_Play_Sound(v4, 39, 0, 0, 99);
			dword_45A0D0_animation_state = 57;
			dword_45A0D4_animation_frame = 0;
			return true;
		}
		if (dword_45A0D0_animation_state == 60) {
			dword_45A0D0_animation_state = 61;
			return true;
		}
		dword_45A0D0_animation_state = 0;
		dword_45A0D4_animation_frame = 0;
		dword_45A0DC = 0;
		return true;
	}
	return true;
}

void AIScriptMcCoy::QueryAnimationState(int *animationState, int *a2, int *a3, int *a4) {
	*animationState = dword_45A0D0_animation_state;
	*a2 = dword_45A0D4_animation_frame;
	*a3 = dword_462718;
	*a4 = dword_46271C;
}

void AIScriptMcCoy::SetAnimationState(int animationState, int a2, int a3, int a4) {
	dword_45A0D0_animation_state = animationState;
	dword_45A0D4_animation_frame = a2;
	dword_462718 = a3;
	dword_46271C = a4;
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
	if (Actor_Query_Which_Set_In(kActorMcCoy) == 87 && Actor_Query_Goal_Number(kActorMcCoy) != 390 && !Game_Flag_Query(682)) {
		float x, y, z;
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if ((z < 220.0f) && (-210.0f < x) && (-70.0f > x)) {
			Game_Flag_Set(682);
			Scene_Loop_Set_Default(3);
			Scene_Loop_Start_Special(2, 2, 1);
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
	if (Game_Flag_Query(550)) {
		if (dword_45A0D4_animation_frame <= 6) {
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
		dword_45A0D0_animation_state = 57;
		dword_45A0D4_animation_frame = 0;
		return;
	}
	switch (dword_45A0D0_animation_state) {
	case 17:
	case 20:
	case 21:
	case 36:
		dword_45A0D0_animation_state = 16;
		dword_45A0D4_animation_frame = 0;
		break;
	case 16:
	case 25:
	case 26:
		break;
	case 15:
		dword_45A0D0_animation_state = 16;
		dword_45A0D4_animation_frame = 16 - 16 * dword_45A0D4_animation_frame / 12;
		break;
	case 14:
		dword_45A0D0_animation_state = 16;
		dword_45A0D4_animation_frame = 0;
		break;
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
		Game_Flag_Set(236);
		dword_45A0D8 = 0;
		dword_45A0DC = 30;
		dword_45A0E4 = 0;
		dword_45A0E0 = 1;
		dword_45A0E8 = 3;
		break;
	case 60:
		dword_45A0D0_animation_state = 61;
		break;
	default:
		dword_45A0D0_animation_state = 0;
		dword_45A0D4_animation_frame = 0;
		dword_45A0DC = 0;
	}
}

void AIScriptMcCoy::sub_405800() {
	switch (dword_45A0D0_animation_state) {
	case 36:
	case 37:
	case 40:
	case 41:
	case 51:
		dword_45A0D0_animation_state = 14;
		dword_45A0D4_animation_frame = 0;
		break;
	case 22:
		dword_45A0D0_animation_state = 19;
		dword_45A0D4_animation_frame = 41;
		break;
	case 18:
		dword_45A0D0_animation_state = 19;
		dword_45A0D4_animation_frame = 13 * ((8 - dword_45A0D4_animation_frame) / 8);
		break;
	case 17:
	case 20:
		dword_45A0D0_animation_state = 19;
		dword_45A0D4_animation_frame = 0;
		break;
	case 16:
		dword_45A0D4_animation_frame = 12 - 12 * dword_45A0D4_animation_frame / 16;
		dword_45A0D0_animation_state = 15;
		break;
	case 14:
	case 15:
	case 19:
	case 21:
		break;
	default:
		dword_45A0D0_animation_state = 15;
		dword_45A0D4_animation_frame = 0;
		break;
	}
}

void AIScriptMcCoy::sub_4058B0() {
	int v0 = Actor_Query_Which_Set_In(kActorMcCoy);
	if (v0 == 27) {
		dword_45A0D0_animation_state = 51;
		dword_45A0D4_animation_frame = 0;
		Player_Loses_Control();
		Game_Flag_Set(210);
	} else if (v0 == 70 || v0 == 87) {
		dword_45A0D0_animation_state = 56;
		dword_45A0D4_animation_frame = 0;
	}
}

void AIScriptMcCoy::sub_405920() {
	dword_45A0D4_animation_frame = 0;
	dword_45A0D0_animation_state = 47;
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
