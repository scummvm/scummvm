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

#include "bladerunner/script/script.h"

namespace BladeRunner {

void ScriptNR07::InitializeScene() {
	Setup_Scene_Information(-110.0f, -73.5f, -193.0f, 554);
	Scene_Exit_Add_2D_Exit(0, 429, 137, 506, 251, 0);
	Ambient_Sounds_Add_Looping_Sound(111, 25, 0, 1);
}

void ScriptNR07::SceneLoaded() {
	Obstacle_Object("VANITY", true);
	Clickable_Object("VASE");
}

bool ScriptNR07::MouseClick(int x, int y) {
	return false;
}

bool ScriptNR07::ClickedOn3DObject(const char *objectName, bool a2) {
	Actor_Set_Goal_Number(25, 201);
	if (Object_Query_Click("VASE", objectName)) {
		sub_401C60();
	}
	Actor_Set_Goal_Number(25, 200);
	return false;
}

bool ScriptNR07::ClickedOnActor(int actorId) {
	if (actorId == 3) {
		if (Actor_Query_Goal_Number(33) <= 239) {
			Actor_Set_Goal_Number(25, 201);
			Actor_Face_Actor(0, 3, true);
			Dialogue_Menu_Clear_List();
			if (Game_Flag_Query(638)) {
				DM_Add_To_List_Never_Repeat_Once_Selected(1100, -1, 3, 8);
				DM_Add_To_List_Never_Repeat_Once_Selected(1110, 8, -1, -1);
				if (Actor_Clue_Query(0, 95)) {
					DM_Add_To_List_Never_Repeat_Once_Selected(1120, 3, 6, 7);
				}
				if (Actor_Clue_Query(0, 113)) {
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
				Actor_Says(0, 3650, 13);
				Actor_Says(3, 630, 30);
				Actor_Says(0, 3655, 16);
				Actor_Says(3, 640, 31);
				break;
			case 1080:
				sub_401EF4();
				break;
			default:
				break;
			}
			Actor_Set_Goal_Number(25, 200);
			return false;;
		}
		return true;
	}
	return false;
}

bool ScriptNR07::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptNR07::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -102.0f, -73.5f, -233.0f, 0, 1, false, 0)) {
			Actor_Set_Goal_Number(25, 201);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(442);
			Set_Enter(56, 59);
		}
		return true;
	}
	return false;
}

bool ScriptNR07::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptNR07::SceneFrameAdvanced(int frame) {
}

void ScriptNR07::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptNR07::PlayerWalkedIn() {
	Loop_Actor_Walk_To_XYZ(0, -110.0f, -73.5f, -169.0f, 0, 0, false, 0);
	if (Actor_Query_In_Set(3, 57)) {
		if (Game_Flag_Query(508)) {
			Actor_Modify_Friendliness_To_Other(3, 0, -2);
			Actor_Says(3, 530, 31);
		} else {
			Game_Flag_Set(508);
			if (!Actor_Clue_Query(3, 214)) {
				Actor_Modify_Friendliness_To_Other(3, 0, 5);
			} else if (Actor_Clue_Query(0, 216) || Actor_Clue_Query(0, 217)) {
				Actor_Modify_Friendliness_To_Other(3, 0, 10);
			}
			Actor_Says(3, 500, 30);
			Actor_Says(0, 3585, 14);
			Actor_Says(3, 510, 30);
			Actor_Start_Speech_Sample(0, 3590);
			Loop_Actor_Walk_To_XYZ(0, -112.0f, -73.0f, -89.0f, 525, 0, false, 0);
			Actor_Says(3, 520, 53);
		}
		Actor_Set_Goal_Number(25, 200);
	}
	//return false;
}

void ScriptNR07::PlayerWalkedOut() {

}

void ScriptNR07::DialogueQueueFlushed(int a1) {
}

void ScriptNR07::sub_4018D4() {
	Actor_Set_Goal_Number(25, 201);
	Player_Loses_Control();
	Actor_Set_At_XYZ(3, -136.0f, -73.0f, -18.0f, 300);
	Actor_Change_Animation_Mode(3, 71);
	Actor_Change_Animation_Mode(0, 21);
	Loop_Actor_Walk_To_XYZ(3, -102.0f, -73.5f, -233.0f, 0, 0, true, 0);
	if (Game_Flag_Query(47)) {
		Actor_Set_Goal_Number(3, 245);
	} else {
		Actor_Set_Goal_Number(3, 295);
		Game_Flag_Set(591);
		Actor_Put_In_Set(3, 91);
		Actor_Set_At_Waypoint(3, 33, 0);
	}
	Player_Gains_Control();
}

void ScriptNR07::sub_401A10() {
	Scene_Exits_Disable();
	Actor_Set_Goal_Number(25, 201);
	Actor_Says_With_Pause(3, 930, 1.0f, 30);
	Actor_Says_With_Pause(3, 910, 1.0f, 30);
	Actor_Face_Object(3, "VANITY", true);
	Actor_Says(3, 940, 31);
	Actor_Says(0, 3770, 19);
	Async_Actor_Walk_To_XYZ(0, -193.0f, -73.5f, -13.0f, 0, false);
	Actor_Says(3, 950, 31);
	Actor_Face_Actor(3, 0, true);
	Actor_Change_Animation_Mode(3, 4);
	Actor_Face_Actor(0, 3, true);
	Actor_Says(0, 3760, 19);
	Actor_Says(3, 960, 53);
	Actor_Says(3, 920, 53);
	Actor_Says(0, 3780, 0);
	Actor_Says(3, 970, 53);
	Actor_Voice_Over(1710, 99);
	Actor_Voice_Over(1720, 99);
	Actor_Voice_Over(1730, 99);
	Actor_Set_Goal_Number(33, 240);
}

void ScriptNR07::sub_401C60() {
	Loop_Actor_Walk_To_XYZ(0, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Object(0, "VASE", true);
	if (Actor_Query_Is_In_Current_Set(3)) {
		if (!Actor_Clue_Query(0, 97)) {
			Actor_Clue_Acquire(0, 97, 1, -1);
			int v0 = Actor_Query_Friendliness_To_Other(3, 0);
			if (v0 > 50) {
				Actor_Modify_Friendliness_To_Other(3, 0, 2);
			} else if (v0 <= 50) {
				Actor_Modify_Friendliness_To_Other(3, 0, -2);
			}
			Actor_Says(0, 3600, 19);
			Actor_Says(3, 550, 30);
			Actor_Says(0, 3605, 19);
			Actor_Says(3, 560, 31);
			Actor_Says(0, 3610, 19);
		}
	} else if (Actor_Clue_Query(0, 98)) {
		Actor_Says(0, 8585, 14);
	} else {
		Actor_Clue_Acquire(0, 98, 1, -1);
		Loop_Actor_Walk_To_Scene_Object(0, "VASE", 100, 1, false);
		Actor_Change_Animation_Mode(0, 23);
		Item_Pickup_Spin_Effect(935, 526, 268);
		Actor_Voice_Over(1690, 99);
		Actor_Voice_Over(1700, 99);
	}
}

void ScriptNR07::sub_401EF4() {
	Actor_Clue_Acquire(0, 96, 1, -1);
	Actor_Says(0, 3625, 19);
	Actor_Says(3, 570, 30);
	Actor_Says_With_Pause(3, 580, 1.0f, 31);
	Actor_Says(0, 3630, 13);
	Actor_Says_With_Pause(3, 590, 1.0f, 30);
	Actor_Says(3, 600, 30);
	Actor_Start_Speech_Sample(0, 3640);
	Loop_Actor_Walk_To_XYZ(0, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Actor(0, 3, true);
	Actor_Face_Actor(3, 0, true);
	Game_Flag_Set(638);
	Actor_Clue_Acquire(0, 91, 1, 3);
	int v0 = Actor_Query_Friendliness_To_Other(3, 0);
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

void ScriptNR07::sub_4020F0() {
	if (Actor_Clue_Query(3, 213) && Actor_Clue_Query(3, 214)) {
		Actor_Modify_Friendliness_To_Other(3, 0, -1);
	}
	Actor_Says(3, 610, 31);
	Actor_Says(0, 3645, 12);
	Actor_Says(3, 620, 30);
	int v0 = Actor_Query_Friendliness_To_Other(3, 0);
	if (!Game_Flag_Query(47) && v0 < 40) {
		sub_4018D4();
		return;
	}
	if (v0 < 36) {
		sub_401A10();
		return;
	}
	Actor_Face_Object(3, "VANITY", true);
}

void ScriptNR07::sub_402284() {
	Actor_Clue_Acquire(0, 94, 1, -1);
	Actor_Start_Speech_Sample(0, 3660);
	Loop_Actor_Walk_To_XYZ(0, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Actor(0, 3, true);
	Actor_Says(3, 650, 30);
	Actor_Says(3, 660, 31);
	Actor_Says(0, 3665, 18);
	Actor_Face_Actor(3, 0, true);
	Actor_Says(3, 670, 31);
	Actor_Says(3, 680, 30);
	Actor_Says(3, 690, 31);
	Actor_Says(0, 3670, 17);
	Actor_Says(3, 700, 30);
	Actor_Says(0, 3675, 19);
	Actor_Says(3, 710, 30);
	Actor_Says(0, 3680, 19);
	Actor_Says(3, 720, 30);
	Actor_Says(3, 730, 30);
	Actor_Says(0, 3685, 13);
	Voight_Kampff_Activate(3, 40);
	if (Game_Flag_Query(47)) {
		sub_401A10();
	} else {
		sub_4018D4();
	}
}

void ScriptNR07::sub_402510() {
	Actor_Says(0, 3690, 14);
	Actor_Start_Speech_Sample(3, 750);
	Loop_Actor_Walk_To_XYZ(0, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
	Actor_Face_Actor(0, 3, true);
	Actor_Face_Actor(3, 0, true);
	Actor_Says(0, 3695, 15);
	Actor_Modify_Friendliness_To_Other(3, 0, 5);
	if (Game_Flag_Query(47)) {
		sub_401A10();
	} else {
		sub_4018D4();
	}
}

void ScriptNR07::sub_402614() {
	Actor_Says(0, 3705, 19);
	Actor_Says(3, 760, 53);
	if (Game_Flag_Query(47)) {
		Actor_Modify_Friendliness_To_Other(3, 0, -5);
		Actor_Says(0, 3710, 18);
		sub_401A10();
	} else {
		Actor_Modify_Friendliness_To_Other(3, 0, -3);
		Actor_Start_Speech_Sample(0, 3710);
		Loop_Actor_Walk_To_XYZ(0, -109.0f, -73.0f, -89.0f, 0, 0, false, 0);
		Actor_Face_Actor(0, 3, true);
		sub_4018D4();
	}
}

void ScriptNR07::sub_402738() {
	Actor_Modify_Friendliness_To_Other(3, 0, -3);
	Actor_Says(0, 3615, 16);
	Actor_Says(3, 770, 30);
	Actor_Says(0, 3720, 15);
	Actor_Says_With_Pause(3, 780, 2.0f, 30);
	Actor_Says(3, 790, 31);
	Actor_Says(0, 3725, 18);
	Actor_Says(3, 800, 30);
	Actor_Says_With_Pause(0, 3730, 2.0f, 13);
	Actor_Says_With_Pause(3, 810, 1.0f, 53);
	Actor_Says(3, 820, 30);
	Actor_Says(0, 3735, 14);
	Actor_Says(3, 830, 31);
	Actor_Says(0, 3740, 19);
}

void ScriptNR07::sub_4028FC() {
	Actor_Says(0, 3620, 19);
	Actor_Says(3, 840, 30);
	Actor_Says(0, 3745, 9);
	Actor_Says_With_Pause(3, 850, 1.0f, 30);
	Actor_Says(3, 860, 30);
	Actor_Says(3, 870, 53);
	Actor_Says(0, 3750, 11);
	Actor_Says(3, 880, 30);
	Actor_Says(0, 3755, 16);
	Actor_Says(3, 890, 31);
}

} // End of namespace BladeRunner
