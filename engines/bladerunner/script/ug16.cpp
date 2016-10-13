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

void ScriptUG16::InitializeScene() {
	if (Game_Flag_Query(552)) {
		Setup_Scene_Information(-270.76f, -34.88f, -504.02f, 404);
		Game_Flag_Reset(552);
	} else if (Game_Flag_Query(152)) {
		Setup_Scene_Information(-322.0f, -34.0f, -404.0f, 345);
		Game_Flag_Reset(152);
	} else {
		Setup_Scene_Information(-318.0f, -34.0f, -216.0f, 340);
		Game_Flag_Reset(354);
	}
	Scene_Exit_Add_2D_Exit(0, 242, 169, 282, 262, 3);
	Scene_Exit_Add_2D_Exit(1, 375, 166, 407, 251, 3);
	Scene_Exit_Add_2D_Exit(2, 461, 148, 523, 248, 0);
	Ambient_Sounds_Add_Looping_Sound(516, 33, 81, 0);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	if (Game_Flag_Query(568)) {
		Scene_Loop_Set_Default(5);
	} else {
		Scene_Loop_Set_Default(0);
	}
}

void ScriptUG16::SceneLoaded() {
	Obstacle_Object("BED", true);
	Obstacle_Object("QUADPATCH07", true);
	Obstacle_Object("QUADPATCH05", true);
	Obstacle_Object("SCREEN 01", true);
	Obstacle_Object("BOX49", true);
	Obstacle_Object("CYLINDER07", true);
	Unobstacle_Object("SEAT 1", true);
	Unobstacle_Object("SEAT 2", true);
	Unclickable_Object("BED");
	Unclickable_Object("QUADPATCH07");
	Clickable_Object("QUADPATCH05");
	Clickable_Object("SCREEN 01");
	Unclickable_Object("BOX49");
	Unclickable_Object("CYLINDER07");
	Unobstacle_Object("BOX67", true);
	Footstep_Sounds_Set(0, 3);
	Footstep_Sounds_Set(1, 2);
	Footstep_Sounds_Set(2, 3);
	Footstep_Sounds_Set(6, 3);
}

bool ScriptUG16::MouseClick(int x, int y) {
	return false;
}

bool ScriptUG16::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("QUADPATCH05", objectName) && !Loop_Actor_Walk_To_XYZ(0, 194.0f, -35.0f, 160.8f, 0, 1, false, 0)) {
		Actor_Face_Heading(0, 870, false);
		if (!Game_Flag_Query(597) && Game_Flag_Query(595)) {
			Game_Flag_Set(597);
			Delay(1000);
			Actor_Voice_Over(3480, 99);
			Actor_Change_Animation_Mode(0, 38);
			Sound_Play(339, 100, 0, 0, 50);
			Delay(1000);
			Item_Pickup_Spin_Effect(948, 460, 287);
			Actor_Voice_Over(2740, 99);
			Actor_Voice_Over(2750, 99);
			Actor_Voice_Over(2760, 99);
			Actor_Voice_Over(2770, 99);
			Actor_Clue_Acquire(0, 125, 1, -1);
		} else {
			Actor_Says(0, 8523, 12);
			Actor_Says(0, 8635, 12);
		}
		return true;
	}
	if (Object_Query_Click("SCREEN 01", objectName) && !Loop_Actor_Walk_To_XYZ(0, 194.0f, -35.0f, 160.8f, 0, 1, false, 0)) {
		Actor_Face_Heading(0, 870, false);
		if ((!Game_Flag_Query(595) && Actor_Query_Is_In_Current_Set(10)) || Actor_Clue_Query(0, 151) || Game_Flag_Query(568)) {
			Actor_Says(0, 8525, 12);
			Actor_Says(0, 8526, 12);
			return false;
		}
		Delay(2000);
		Actor_Face_Heading(0, 1016, false);
		Delay(2000);
		Actor_Says(0, 5725, 14);
		Delay(1000);
		Item_Pickup_Spin_Effect(941, 418, 305);
		Actor_Clue_Acquire(0, 151, 1, -1);
		return true;
	}
	return false;
}

bool ScriptUG16::ClickedOnActor(int actorId) {
	if (Actor_Query_Goal_Number(10) < 490) {
		sub_401D78();
		return true;
	}
	return false;
}

bool ScriptUG16::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptUG16::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -322.0f, -34.0f, -216.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(355);
			Set_Enter(87, 99);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -322.0f, -34.0f, -404.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(153);
			Set_Enter(87, 99);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, -316.78f, -34.88f, -533.27f, 0, 1, false, 0)) {
			Actor_Face_Heading(0, 0, false);
			Loop_Actor_Travel_Stairs(0, 13, 1, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(551);
			Set_Enter(36, 30);
		}
		return true;
	}
	return false;
}

bool ScriptUG16::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptUG16::SceneFrameAdvanced(int frame) {
	if (frame == 132) {
		Ambient_Sounds_Remove_Looping_Sound(516, true);
	}
}

void ScriptUG16::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptUG16::PlayerWalkedIn() {
	Game_Flag_Set(715);
	if (!Game_Flag_Query(595)) {
		Actor_Set_Goal_Number(10, 403);
	}
	if (!Game_Flag_Query(556) && Actor_Query_Is_In_Current_Set(10)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, 120.29f, -35.67f, 214.8f, 310, 0, false, 0);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(10, 0, 6);
		Actor_Says(10, 30, 13);
		Actor_Change_Animation_Mode(10, 17);
		Actor_Says(13, 0, 17);
		Actor_Says(0, 5710, 14);
		Actor_Says(10, 40, 13);
		Actor_Says(10, 50, 15);
		Actor_Says(13, 20, 12);
		Actor_Says(10, 60, 23);
		Actor_Says(0, 5715, 14);
		Actor_Says(13, 30, 16);
		Actor_Says(10, 70, 6);
		Player_Gains_Control();
		Game_Flag_Set(556);
	}
}

void ScriptUG16::PlayerWalkedOut() {
	if (!Game_Flag_Query(595)) {
		Actor_Set_Goal_Number(10, 401);
		//return true;
	}
	//return false;
}

void ScriptUG16::DialogueQueueFlushed(int a1) {
}

void ScriptUG16::sub_401D78() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(1400, 5, 6, 2);
	DM_Add_To_List_Never_Repeat_Once_Selected(1410, 5, 4, 8);
	if (Game_Flag_Query(600) || Game_Flag_Query(601)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1420, 6, 4, 5);
		DM_Add_To_List_Never_Repeat_Once_Selected(1430, 6, 4, 5);
		DM_Add_To_List_Never_Repeat_Once_Selected(1440, 6, 4, 5);
	}
	if (Global_Variable_Query(49) > 1 && !Actor_Clue_Query(0, 125)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1450, 6, 4, 5);
	}
	if (Actor_Clue_Query(0, 76)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1460, 6, 4, 5);
	}
	if (Actor_Clue_Query(0, 147) && !Actor_Clue_Query(0, 125) && Game_Flag_Query(698)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1470, 6, 4, 5);
	}
	Dialogue_Menu_Add_DONE_To_List(1480);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 1400:
		Actor_Says(0, 5730, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(10, 100, 18);
		Actor_Says(0, 5775, 13);
		Actor_Says(13, 70, 17);
		Actor_Says(10, 110, 16);
		Actor_Says(13, 80, 6);
		Actor_Says(0, 5780, 13);
		Actor_Says(10, 120, 16);
		Actor_Says(13, 120, 13);
		Actor_Says(0, 5785, 13);
		Actor_Says(10, 130, 6);
		Actor_Says(0, 5825, 13);
		Actor_Modify_Friendliness_To_Other(10, 0, -5);
		if (Game_Flag_Query(560)) {
			Actor_Says(10, 140, 13);
			Actor_Says(10, 150, 14);
			Actor_Says(10, 160, 13);
			Actor_Says(13, 140, 16);
			Actor_Says(0, 5790, 13);
			Actor_Says(10, 170, 14);
			Game_Flag_Set(600);
			Actor_Modify_Friendliness_To_Other(10, 0, 5);
		} else {
			Actor_Says(10, 180, 14);
			Actor_Says(0, 5795, 13);
			Actor_Says(13, 150, 17);
			Actor_Says(0, 5800, 13);
			Actor_Says(10, 190, 15);
			Game_Flag_Set(601);
			Actor_Modify_Friendliness_To_Other(10, 0, -10);
		}
		break;
	case 1410:
		Actor_Says(0, 5735, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(13, 160, 17);
		Actor_Says(10, 200, 14);
		break;
	case 1420:
		Actor_Says(0, 5740, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(13, 180, 15);
		Actor_Says(10, 220, 13);
		Actor_Says(13, 190, 17);
		Actor_Says(0, 5805, 13);
		Actor_Says(10, 230, 14);
		Actor_Says(10, 240, 13);
		Actor_Says(13, 200, 17);
		Actor_Says(10, 260, 13);
		Actor_Says(10, 270, 15);
		Actor_Says(13, 210, 14);
		Actor_Says(0, 5810, 13);
		Actor_Says(13, 220, 14);
		Actor_Says(13, 230, 17);
		Actor_Clue_Acquire(0, 136, 1, 10);
		break;
	case 1430:
		Actor_Says(0, 5745, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(13, 240, 15);
		Actor_Says(0, 5815, 13);
		Actor_Says(13, 250, 16);
		Actor_Says(10, 290, 15);
		Actor_Says(13, 260, 15);
		break;
	case 1440:
		Actor_Says(0, 5750, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(13, 280, 6);
		Actor_Says(10, 300, 14);
		Actor_Says(10, 310, 15);
		Actor_Modify_Friendliness_To_Other(10, 0, -5);
		break;
	case 1450:
		Actor_Says(0, 5755, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(13, 290, 17);
		Actor_Says(10, 320, 16);
		Actor_Says(0, 5820, 13);
		Actor_Says(13, 300, 17);
		Actor_Says(10, 330, 14);
		Actor_Says(0, 5825, 13);
		Actor_Says(10, 340, 13);
		Actor_Says(13, 310, 13);
		Actor_Says(10, 350, 13);
		Actor_Says(10, 360, 15);
		Actor_Says(0, 5830, 13);
		Actor_Says(13, 320, 16);
		Actor_Says(13, 330, 15);
		Game_Flag_Set(698);
		break;
	case 1460:
		Actor_Says(0, 5760, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(10, 370, 15);
		Actor_Says(13, 340, 14);
		Actor_Says(0, 5835, 13);
		Actor_Says(10, 380, 15);
		Actor_Says(13, 370, 6);
		Actor_Says(0, 5840, 13);
		Actor_Says(13, 380, 13);
		break;
	case 1470:
		Actor_Says(0, 5765, 13);
		Actor_Face_Actor(0, 10, true);
		Actor_Says(13, 400, 15);
		Actor_Says(0, 5845, 13);
		Actor_Says(10, 390, 23);
		Actor_Says(13, 410, 14);
		Actor_Says(13, 420, 17);
		Actor_Says(0, 5835, 13);
		Delay(1000);
		Item_Pickup_Spin_Effect(948, 239, 454);
		Actor_Voice_Over(2740, 99);
		Actor_Voice_Over(2750, 99);
		Actor_Voice_Over(2760, 99);
		Actor_Voice_Over(2770, 99);
		Actor_Says(0, 5850, 13);
		Actor_Says(10, 400, 15);
		Actor_Says(13, 430, 6);
		Actor_Says(0, 5855, 13);
		Actor_Says(10, 410, 14);
		Game_Flag_Set(597);
		Actor_Clue_Acquire(0, 125, 1, 10);
		break;
	case 1480:
		Actor_Says(0, 4595, 14);
		break;
	}
}

} // End of namespace BladeRunner
