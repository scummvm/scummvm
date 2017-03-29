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

#include "bladerunner/script/scene.h"

namespace BladeRunner {

void SceneScriptNR07::InitializeScene() {
	Setup_Scene_Information(-110.0f, -73.5f, -193.0f, 554);
	Scene_Exit_Add_2D_Exit(0, 429, 137, 506, 251, 0);
	Ambient_Sounds_Add_Looping_Sound(111, 25, 0, 1);
}

void SceneScriptNR07::SceneLoaded() {
	Obstacle_Object("VANITY", true);
	Clickable_Object("VASE");
}

bool SceneScriptNR07::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR07::ClickedOn3DObject(const char *objectName, bool a2) {
	Actor_Set_Goal_Number(kActorHanoi, 201);
	if (Object_Query_Click("VASE", objectName)) {
		sub_401C60();
	}
	Actor_Set_Goal_Number(kActorHanoi, 200);
	return false;
}

bool SceneScriptNR07::ClickedOnActor(int actorId) {
	if (actorId == kActorDektora) {
		if (Actor_Query_Goal_Number(kActorHolloway) <= 239) {
			Actor_Set_Goal_Number(kActorHanoi, 201);
			Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
			Dialogue_Menu_Clear_List();
			if (Game_Flag_Query(638)) {
				DM_Add_To_List_Never_Repeat_Once_Selected(1100, -1, 3, 8);
				DM_Add_To_List_Never_Repeat_Once_Selected(1110, 8, -1, -1);
				if (Actor_Clue_Query(kActorMcCoy, kClueSuspectDektora)) {
					DM_Add_To_List_Never_Repeat_Once_Selected(1120, 3, 6, 7);
				}
				if (Actor_Clue_Query(kActorMcCoy, kClueCarRegistration1)) {
					DM_Add_To_List_Never_Repeat_Once_Selected(1130, 3, 5, 7);
				}
				if (Game_Flag_Query(510)) {
					DM_Add_To_List_Never_Repeat_Once_Selected(1140, 1, 4, 7);
				}
			} else {
				DM_Add_To_List_Never_Repeat_Once_Selected(1080, 3, 5, 7);
				DM_Add_To_List_Never_Repeat_Once_Selected(1090, 7, 5, 4);
			}
			Dialogue_Menu_Add_DONE_To_List(1150);
			Dialogue_Menu_Appear(320, 240);
			int answer = Dialogue_Menu_Query_Input();
			Dialogue_Menu_Disappear();
			switch (answer) {
			case 1140:
				sub_4028FC();
				break;
			case 1130:
				sub_402738();
				break;
			case 1120:
				sub_402614();
				break;
			case 1110:
				sub_402510();
				break;
			case 1100:
				sub_402284();
				break;
			case 1090:
				Actor_Says(kActorMcCoy, 3650, 13);
				Actor_Says(kActorDektora, 630, 30);
				Actor_Says(kActorMcCoy, 3655, 16);
				Actor_Says(kActorDektora, 640, 31);
				break;
			case 1080:
				sub_401EF4();
				break;
			default:
				break;
			}
			Actor_Set_Goal_Number(kActorHanoi, 200);
			return false;;
		}
		return true;
	}
	return false;
}

bool SceneScriptNR07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptNR07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -102.0f, -73.5f, -233.0f, 0, 1, false, 0)) {
			Actor_Set_Goal_Number(kActorHanoi, 201);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(442);
			Set_Enter(56, 59);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR07::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR07::SceneFrameAdvanced(int frame) {
}

void SceneScriptNR07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -110.0f, -73.5f, -169.0f, 0, 0, false, 0);
	if (Actor_Query_In_Set(kActorDektora, 57)) {
		if (Game_Flag_Query(508)) {
			Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -2);
			Actor_Says(kActorDektora, 530, 31);
		} else {
			Game_Flag_Set(508);
			if (!Actor_Clue_Query(kActorDektora, kClueMcCoyIsABladeRunner)) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 5);
			} else if (Actor_Clue_Query(kActorMcCoy, kClueMcCoyWarnedIzo) || Actor_Clue_Query(kActorMcCoy, kClueMcCoyHelpedIzoIzoIsAReplicant)) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 10);
			}
			Actor_Says(kActorDektora, 500, 30);
			Actor_Says(kActorMcCoy, 3585, 14);
			Actor_Says(kActorDektora, 510, 30);
			Actor_Start_Speech_Sample(kActorMcCoy, 3590);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -112.0f, -73.0f, -89.0f, 525, 0, false, 0);
			Actor_Says(kActorDektora, 520, 53);
		}
		Actor_Set_Goal_Number(kActorHanoi, 200);
	}
	//return false;
}

void SceneScriptNR07::PlayerWalkedOut() {

}

void SceneScriptNR07::DialogueQueueFlushed(int a1) {
}

void SceneScriptNR07::sub_4018D4() {
	Actor_Set_Goal_Number(kActorHanoi, 201);
	Player_Loses_Control();
	Actor_Set_At_XYZ(kActorDektora, -136.0f, -73.0f, -18.0f, 300);
	Actor_Change_Animation_Mode(kActorDektora, 71);
	Actor_Change_Animation_Mode(kActorMcCoy, 21);
	Loop_Actor_Walk_To_XYZ(kActorDektora, -102.0f, -73.5f, -233.0f, 0, 0, true, 0);
	if (Game_Flag_Query(47)) {
		Actor_Set_Goal_Number(kActorDektora, 245);
	} else {
		Actor_Set_Goal_Number(kActorDektora, 295);
		Game_Flag_Set(591);
		Actor_Put_In_Set(kActorDektora, 91);
		Actor_Set_At_Waypoint(kActorDektora, 33, 0);
	}
	Player_Gains_Control();
}

void SceneScriptNR07::sub_401A10() {
	Scene_Exits_Disable();
	Actor_Set_Goal_Number(kActorHanoi, 201);
	Actor_Says_With_Pause(kActorDektora, 930, 1.0f, 30);
	Actor_Says_With_Pause(kActorDektora, 910, 1.0f, 30);
	Actor_Face_Object(kActorDektora, "VANITY", true);
	Actor_Says(kActorDektora, 940, 31);
	Actor_Says(kActorMcCoy, 3770, 19);
	Async_Actor_Walk_To_XYZ(kActorMcCoy, -193.0f, -73.5f, -13.0f, 0, false);
	Actor_Says(kActorDektora, 950, 31);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Change_Animation_Mode(kActorDektora, 4);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Says(kActorMcCoy, 3760, 19);
	Actor_Says(kActorDektora, 960, 53);
	Actor_Says(kActorDektora, 920, 53);
	Actor_Says(kActorMcCoy, 3780, 0);
	Actor_Says(kActorDektora, 970, 53);
	Actor_Voice_Over(1710, kActorVoiceOver);
	Actor_Voice_Over(1720, kActorVoiceOver);
	Actor_Voice_Over(1730, kActorVoiceOver);
	Actor_Set_Goal_Number(kActorHolloway, 240);
}

void SceneScriptNR07::sub_401C60() {
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Object(kActorMcCoy, "VASE", true);
	if (Actor_Query_Is_In_Current_Set(kActorDektora)) {
		if (!Actor_Clue_Query(kActorMcCoy, kClueDektoraInterview3)) {
			Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview3, 1, -1);
			int v0 = Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy);
			if (v0 > 50) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 2);
			} else if (v0 <= 50) {
				Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -2);
			}
			Actor_Says(kActorMcCoy, 3600, 19);
			Actor_Says(kActorDektora, 550, 30);
			Actor_Says(kActorMcCoy, 3605, 19);
			Actor_Says(kActorDektora, 560, 31);
			Actor_Says(kActorMcCoy, 3610, 19);
		}
	} else if (Actor_Clue_Query(kActorMcCoy, kClueDektorasCard)) {
		Actor_Says(kActorMcCoy, 8585, 14);
	} else {
		Actor_Clue_Acquire(kActorMcCoy, kClueDektorasCard, 1, -1);
		Loop_Actor_Walk_To_Scene_Object(kActorMcCoy, "VASE", 100, true, false);
		Actor_Change_Animation_Mode(kActorMcCoy, 23);
		Item_Pickup_Spin_Effect(935, 526, 268);
		Actor_Voice_Over(1690, kActorVoiceOver);
		Actor_Voice_Over(1700, kActorVoiceOver);
	}
}

void SceneScriptNR07::sub_401EF4() {
	Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview2, 1, -1);
	Actor_Says(kActorMcCoy, 3625, 19);
	Actor_Says(kActorDektora, 570, 30);
	Actor_Says_With_Pause(kActorDektora, 580, 1.0f, 31);
	Actor_Says(kActorMcCoy, 3630, 13);
	Actor_Says_With_Pause(kActorDektora, 590, 1.0f, 30);
	Actor_Says(kActorDektora, 600, 30);
	Actor_Start_Speech_Sample(kActorMcCoy, 3640);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Game_Flag_Set(638);
	Actor_Clue_Acquire(kActorMcCoy, kClueDragonflyBelt, 1, kActorDektora);
	int v0 = Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy);
	if (!Game_Flag_Query(47) && v0 < 40) {
		sub_4018D4();
		return;
	}
	if (v0 < 36) {
		sub_401A10();
		return;
	}
	sub_4020F0();
}

void SceneScriptNR07::sub_4020F0() {
	if (Actor_Clue_Query(kActorDektora, kClueMcCoysDescription) && Actor_Clue_Query(kActorDektora, kClueMcCoyIsABladeRunner)) {
		Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -1);
	}
	Actor_Says(kActorDektora, 610, 31);
	Actor_Says(kActorMcCoy, 3645, 12);
	Actor_Says(kActorDektora, 620, 30);
	int v0 = Actor_Query_Friendliness_To_Other(kActorDektora, kActorMcCoy);
	if (!Game_Flag_Query(47) && v0 < 40) {
		sub_4018D4();
		return;
	}
	if (v0 < 36) {
		sub_401A10();
		return;
	}
	Actor_Face_Object(kActorDektora, "VANITY", true);
}

void SceneScriptNR07::sub_402284() {
	Actor_Clue_Acquire(kActorMcCoy, kClueDektoraInterview1, 1, -1);
	Actor_Start_Speech_Sample(kActorMcCoy, 3660);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Says(kActorDektora, 650, 30);
	Actor_Says(kActorDektora, 660, 31);
	Actor_Says(kActorMcCoy, 3665, 18);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Says(kActorDektora, 670, 31);
	Actor_Says(kActorDektora, 680, 30);
	Actor_Says(kActorDektora, 690, 31);
	Actor_Says(kActorMcCoy, 3670, 17);
	Actor_Says(kActorDektora, 700, 30);
	Actor_Says(kActorMcCoy, 3675, 19);
	Actor_Says(kActorDektora, 710, 30);
	Actor_Says(kActorMcCoy, 3680, 19);
	Actor_Says(kActorDektora, 720, 30);
	Actor_Says(kActorDektora, 730, 30);
	Actor_Says(kActorMcCoy, 3685, 13);
	Voight_Kampff_Activate(3, 40);
	if (Game_Flag_Query(47)) {
		sub_401A10();
	} else {
		sub_4018D4();
	}
}

void SceneScriptNR07::sub_402510() {
	Actor_Says(kActorMcCoy, 3690, 14);
	Actor_Start_Speech_Sample(kActorDektora, 750);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
	Actor_Face_Actor(kActorDektora, kActorMcCoy, true);
	Actor_Says(kActorMcCoy, 3695, 15);
	Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, 5);
	if (Game_Flag_Query(47)) {
		sub_401A10();
	} else {
		sub_4018D4();
	}
}

void SceneScriptNR07::sub_402614() {
	Actor_Says(kActorMcCoy, 3705, 19);
	Actor_Says(kActorDektora, 760, 53);
	if (Game_Flag_Query(47)) {
		Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -5);
		Actor_Says(kActorMcCoy, 3710, 18);
		sub_401A10();
	} else {
		Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -3);
		Actor_Start_Speech_Sample(kActorMcCoy, 3710);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
		Actor_Face_Actor(kActorMcCoy, kActorDektora, true);
		sub_4018D4();
	}
}

void SceneScriptNR07::sub_402738() {
	Actor_Modify_Friendliness_To_Other(kActorDektora, kActorMcCoy, -3);
	Actor_Says(kActorMcCoy, 3615, 16);
	Actor_Says(kActorDektora, 770, 30);
	Actor_Says(kActorMcCoy, 3720, 15);
	Actor_Says_With_Pause(kActorDektora, 780, 2.0f, 30);
	Actor_Says(kActorDektora, 790, 31);
	Actor_Says(kActorMcCoy, 3725, 18);
	Actor_Says(kActorDektora, 800, 30);
	Actor_Says_With_Pause(kActorMcCoy, 3730, 2.0f, 13);
	Actor_Says_With_Pause(kActorDektora, 810, 1.0f, 53);
	Actor_Says(kActorDektora, 820, 30);
	Actor_Says(kActorMcCoy, 3735, 14);
	Actor_Says(kActorDektora, 830, 31);
	Actor_Says(kActorMcCoy, 3740, 19);
}

void SceneScriptNR07::sub_4028FC() {
	Actor_Says(kActorMcCoy, 3620, 19);
	Actor_Says(kActorDektora, 840, 30);
	Actor_Says(kActorMcCoy, 3745, 9);
	Actor_Says_With_Pause(kActorDektora, 850, 1.0f, 30);
	Actor_Says(kActorDektora, 860, 30);
	Actor_Says(kActorDektora, 870, 53);
	Actor_Says(kActorMcCoy, 3750, 11);
	Actor_Says(kActorDektora, 880, 30);
	Actor_Says(kActorMcCoy, 3755, 16);
	Actor_Says(kActorDektora, 890, 31);
}

} // End of namespace BladeRunner
