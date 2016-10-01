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

void ScriptRC04::InitializeScene() {
	Setup_Scene_Information(45.0f, 0.15f, 68.0f, 1018);
	Game_Flag_Reset(120);
	Scene_Exit_Add_2D_Exit(0, 225, 47, 359, 248, 0);
	if (!Game_Flag_Query(289)) {
		Actor_Put_In_Set(14, 71);
		Actor_Set_At_XYZ(14, -60.0f, -11.0f, 62.0f, 12);
	}
	if (Game_Flag_Query(289)) {
		Actor_Change_Animation_Mode(14, 88);
	}
	Ambient_Sounds_Add_Looping_Sound(381, 100, 1, 1);
	Ambient_Sounds_Add_Sound(82, 5, 30, 10, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(83, 5, 30, 10, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(84, 5, 30, 10, 20, -100, 100, -101, -101, 0, 0);
}

void ScriptRC04::SceneLoaded() {
	Obstacle_Object("Door New 01", true);
	Obstacle_Object("GRNDNEON05", true);
	Obstacle_Object("GRNDNEON06", true);
	Obstacle_Object("GRNDNEON07", true);
	Unobstacle_Object("DisplayTrim", true);
	Unobstacle_Object("Display01", true);
	Actor_Set_Goal_Number(67, 200);
}

bool ScriptRC04::MouseClick(int x, int y) {
	return false;
}

bool ScriptRC04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

void ScriptRC04::sub_401DF4() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 11) && !Actor_Clue_Query(0, 62)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(580, -1, 4, 9);
	}
	if (Actor_Clue_Query(0, 5) && !Actor_Clue_Query(0, 11) && !Actor_Clue_Query(0, 62)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(590, 6, 5, 3);
	}
	if (Actor_Clue_Query(0, 57)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(600, -1, 3, 7);
		DM_Add_To_List_Never_Repeat_Once_Selected(1310, -1, 2, 8);
	}
	if (Actor_Clue_Query(0, 62) && !Actor_Clue_Query(0, 63)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(610, 4, 5, 6);
	}
	if (!Game_Flag_Query(305)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(1280, 1, 2, 3);
	}
	if (Actor_Clue_Query(0, 110) && !Actor_Clue_Query(14, 110)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(620, 1, -1, -1);
	}
	Dialogue_Menu_Add_DONE_To_List(630);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();

	switch (answer) {
	case 580:
		Actor_Says(0, 4955, 30);
		Actor_Says(14, 210, 37);
		Actor_Says(14, 220, 37);
		Actor_Says(14, 230, 37);
		Actor_Says(14, 240, 37);
		Actor_Says(0, 4990, 16);
		Actor_Says(0, 4995, 11);
		Actor_Says(14, 270, 31);
		Actor_Says(0, 5005, 16);
		Actor_Says(14, 280, 32);
		Actor_Says(14, 290, 30);
		Actor_Says(14, 300, 33);
		Actor_Says(14, 310, 31);
		Actor_Says(0, 5010, 11);
		Actor_Says(14, 320, 30);
		Actor_Says(14, 330, 33);
		Actor_Says(14, 340, 37);
		Actor_Says(0, 5015, 11);
		if (Game_Flag_Query(44)) {
			Actor_Says(14, 350, 32);
			Actor_Says(14, 360, 33);
			Actor_Says(14, 370, 30);
			Actor_Says(0, 5020, 16);
			Actor_Says(14, 380, 37);
			Actor_Says(14, 390, 11);
			Actor_Says(14, 400, 37);
			Actor_Clue_Acquire(0, 58, 1, 0);
		} else {
			Actor_Says(14, 410, 11);
			Actor_Says(14, 420, 37);
			Actor_Says(0, 5025, 16);
			Actor_Says(14, 430, 30);
			Actor_Says(14, 440, 31);
			Actor_Says(14, 450, 32);
			Actor_Says(0, 5030, 16);
			Actor_Says(14, 460, 37);
			Actor_Clue_Acquire(0, 59, 1, 0);
		}
		return;
	case 590:
		Actor_Says(0, 4960, 13);
		Actor_Says(14, 250, 30);
		Actor_Says(14, 260, 33);
		Actor_Says(0, 4995, 15);
		Actor_Says(14, 270, 32);
		Actor_Says(0, 5005, 11);
		Actor_Says(14, 280, 33);
		Actor_Says(14, 290, 30);
		Actor_Says(14, 300, 32);
		Actor_Says(14, 310, 37);
		Actor_Says(0, 5010, 13);
		Actor_Says(14, 320, 37);
		Actor_Says(14, 330, 33);
		Actor_Says(14, 340, 11);
		Actor_Says(0, 5015, 16);
		Actor_Modify_Friendliness_To_Other(14, 0, 3);
		if (Game_Flag_Query(44)) {
			Actor_Says(14, 350, 32);
			Actor_Says(14, 360, 30);
			Actor_Says(14, 370, 33);
			Actor_Says(0, 5020, 15);
			Actor_Says(14, 380, 33);
			Actor_Says(14, 390, 37);
			Actor_Says(14, 400, 32);
			Actor_Clue_Acquire(0, 58, 1, 0);
		} else {
			Actor_Says(14, 410, 32);
			Actor_Says(14, 420, 30);
			Actor_Says(0, 5025, 13);
			Actor_Says(14, 430, 33);
			Actor_Says(14, 440, 32);
			Actor_Says(14, 450, 37);
			Actor_Says(0, 5030, 16);
			Actor_Says(14, 460, 30);
			Actor_Clue_Acquire(0, 59, 1, 0);
		}
		break;
	case 600:
		Actor_Says(0, 4965, 11);
		Actor_Says(14, 470, 11);
		Actor_Says(0, 5035, 15);
		Actor_Says(14, 480, 30);
		Actor_Says(14, 490, 31);
		Actor_Says(14, 500, 32);
		Actor_Says(14, 510, 33);
		Actor_Says(14, 520, 34);
		Actor_Says(14, 530, 35);
		Actor_Says(14, 540, 36);
		Actor_Says(0, 5040, 16);
		Actor_Says(14, 550, 11);
		Actor_Modify_Friendliness_To_Other(14, 0, -6);
		break;
	case 610:
		Actor_Says(0, 4970, 16);
		if (Actor_Query_Friendliness_To_Other(14, 0) < 50) {
			Actor_Says(14, 700, 11);
			Actor_Says(0, 5070, 11);
			Actor_Says(14, 710, 11);
			Actor_Says(0, 5075, 15);
			Actor_Says(14, 720, 30);
			Actor_Says(0, 5080, 11);
			Actor_Says(14, 730, 37);
			Actor_Clue_Acquire(0, 58, 1, 0);
		} else {
			Actor_Says(14, 560, 37);
			Actor_Says(0, 5070, 13);
			Actor_Says(14, 570, 36);
			Actor_Says(14, 580, 37);
			Actor_Says(14, 590, 31);
			Actor_Says(14, 600, 32);
			Actor_Says(14, 610, 30);
			Actor_Says(0, 5050, 16);
			Actor_Says(14, 620, 35);
			Actor_Says(14, 630, 35);
			Actor_Says(0, 5055, 11);
			Actor_Says(14, 640, 36);
			Actor_Says(14, 650, 35);
			Actor_Says(14, 660, 30);
			Actor_Says(0, 5060, 13);
			Actor_Clue_Acquire(0, 63, 1, 0);
		}
		break;
	case 1280:
		Actor_Says(0, 9040, 16);
		if (!Game_Flag_Query(305)) {
			Actor_Says(14, 2080, 30);
			Actor_Says(14, 2090, 37);
			Actor_Says(0, 9045, 14);
			Actor_Says(14, 2100, 32);
			Actor_Says(14, 2110, 37);
			Game_Flag_Set(305);
		}
		Actor_Says(14, 2120, 31);
		if (Global_Variable_Query(2) > 40 || Query_Difficulty_Level() == 0) {
			Actor_Says(0, 4940, 13);
			if (Query_Difficulty_Level() != 0) {
				Global_Variable_Decrement(2, 40);
			}
			Item_Pickup_Spin_Effect(995, 405, 192);
			Give_McCoy_Ammo(1, 24);
		} else {
			Actor_Says(0, 125, 13);
			Actor_Modify_Friendliness_To_Other(14, 0, -2);
		}
		break;
	case 1310:
		Actor_Says(0, 4980, 11);
		if (Actor_Query_Friendliness_To_Other(14, 0) > 49) {
			Actor_Says(14, 740, 37);
			Actor_Says(0, 5085, 16);
			Actor_Says(14, 750, 37);
			Actor_Says(14, 760, 37);
			Voight_Kampff_Activate(14, 50);
			Actor_Modify_Friendliness_To_Other(14, 0, 3);
			Actor_Says(14, 810, 37);
			Actor_Says(0, 5025, 13);
			Actor_Says(14, 820, 32);
			Actor_Says(0, 5100, 11);
			Actor_Says(14, 830, 31);
			Actor_Says(14, 840, 35);
		} else {
			Actor_Says(14, 770, 36);
			Actor_Says(14, 780, 36);
			Actor_Says(0, 5090, 16);
			Actor_Says(14, 790, 36);
			Actor_Says(14, 800, 35);
			Voight_Kampff_Activate(14, 50);
			Actor_Says(14, 810, 30);
			Actor_Says(0, 5025, 13);
			Actor_Says(14, 820, 31);
			Actor_Says(0, 5100, 15);
			Actor_Says(14, 830, 34);
			Actor_Says(14, 840, 34);
		}
		break;
	case 620:
		Actor_Says(0, 4985, 11);
		Actor_Says(14, 850, 35);
		Actor_Says(0, 5105, 13);
		Actor_Says(0, 5110, 11);
		Actor_Says(14, 860, 30);
		Actor_Says(0, 5115, 16);
		Actor_Says(14, 870, 31);
		Actor_Says(0, 5120, 15);
		Actor_Says(14, 880, 34);
		Actor_Clue_Acquire(14, 110, 1, 0);
		Actor_Modify_Friendliness_To_Other(14, 0, 8);
		if (Query_Difficulty_Level() != 0) {
			Global_Variable_Increment(2, 60);
		}
		break;
	case 630:
		Actor_Says(0, 1315, 12);
		break;
	}
}

bool ScriptRC04::ClickedOnActor(int actorId) {
	if (Player_Query_Combat_Mode()) {
		return false;
	}
	if (actorId == 14 && Global_Variable_Query(1) == 2 && !Game_Flag_Query(289)) {
		Loop_Actor_Walk_To_Waypoint(0, 104, 0, 0, false);
		Actor_Face_Actor(0, 14, true);
		if (Game_Flag_Query(287) && !Game_Flag_Query(292) && Actor_Query_Friendliness_To_Other(14, 0) > 45) {
			Actor_Says(14, 30, 30);
			Actor_Says(0, 4875, 13);
			Actor_Says(14, 80, 31);
			Actor_Says(0, 4900, 15);
			Actor_Says(14, 90, 33);
			Actor_Says(14, 100, 34);
			Actor_Says(0, 4905, 15);
			Game_Flag_Set(292);
		} else if (Game_Flag_Query(287) && !Game_Flag_Query(290) && Actor_Query_Friendliness_To_Other(14, 0) < 45) {
			Actor_Says(14, 40, 30);
			Actor_Says(0, 4880, 13);
			Actor_Says(14, 50, 35);
			Actor_Says(0, 4875, 16);
			Actor_Says(14, 60, 36);
			Actor_Says(0, 4890, 13);
			Actor_Says(14, 70, 33);
			Actor_Says(0, 4895, 16);
			Actor_Modify_Friendliness_To_Other(14, 0, -5);
			Game_Flag_Set(290);
		} else if (Actor_Query_Friendliness_To_Other(14, 0) > 51 && !Game_Flag_Query(717)) {
			Actor_Says(14, 1870, 30);
			Actor_Says(14, 1880, 30);
			Actor_Says(0, 8960, 13);
			Actor_Says(14, 1890, 36);
			Actor_Says(14, 1900, 35);
			Actor_Says(0, 8965, 16);
			Actor_Says(14, 1920, 36);
			Actor_Says(14, 1930, 33);
			Actor_Says(14, 1940, 36);
			Actor_Says(14, 1950, 30);
			Actor_Says(0, 8970, 13);
			Actor_Says(14, 1960, 33);
			Actor_Says(14, 1970, 30);
			Actor_Says(14, 1980, 36);
			Delay(1000);
			Actor_Says(14, 2010, 35);
			if (Global_Variable_Query(2) > 50 || Query_Difficulty_Level() == 0) {
				Actor_Says(0, 8975, 16);
				if (Query_Difficulty_Level() != 0) {
					Global_Variable_Decrement(2, 50);
				}
				Delay(3000);
				Item_Pickup_Spin_Effect(941, 405, 192);
				Actor_Says(14, 2030, 30);
				Game_Flag_Set(487);
			} else {
				Actor_Says(0, 8980, 16);
				Actor_Says(14, 2040, 30);
				Actor_Says(0, 8985, 15);
				Actor_Says(14, 2050, 33);
			}
			Game_Flag_Set(717);
		} else if (Actor_Clue_Query(0, 11) || Actor_Clue_Query(0, 5) || Actor_Clue_Query(0, 62) || Actor_Clue_Query(0, 110) || Actor_Clue_Query(0, 57) || !Game_Flag_Query(305)) {
			sub_401DF4();
		} else {
			Actor_Says(14, 1820, 30);
		}
		return true;
	}
	if (actorId == 14 && Game_Flag_Query(289)) {
		Actor_Face_Actor(0, 14, true);
		if (Actor_Clue_Query(0, 164)) {
			Actor_Says(0, 8590, -1);
		} else {
			Actor_Voice_Over(2100, 99);
			Actor_Voice_Over(2110, 99);
		}
		return true;
	}
	return false;
}

bool ScriptRC04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptRC04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 45.0f, 0.15f, 68.0f, 0, 1, false, 0)) {
			Game_Flag_Set(121);
			Set_Enter(70, 80);
		}
		return true;
	}
	return false;
}

bool ScriptRC04::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptRC04::SceneFrameAdvanced(int frame) {
}

void ScriptRC04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptRC04::PlayerWalkedIn() {
	Loop_Actor_Walk_To_Waypoint(0, 103, 0, 0, false);
	if (Global_Variable_Query(1) != 2 || Game_Flag_Query(287) || Player_Query_Combat_Mode()) {
		if (Global_Variable_Query(1) == 4 && !Game_Flag_Query(289) && !Game_Flag_Query(306)) {
			Actor_Says(38, 40, 3);
			Actor_Says(14, 890, 37);
			Actor_Set_Goal_Number(14, 2);
		}
		Game_Flag_Set(287);
	} else {
		Actor_Says(14, 0, 31);
		Loop_Actor_Walk_To_Waypoint(0, 104, 0, 0, false);
		Actor_Face_Actor(0, 14, true);
		Actor_Says(0, 4865, 13);
		Actor_Says(14, 10, 32);
		Actor_Says(0, 4870, 16);
		Actor_Says(14, 20, 31);
		Game_Flag_Set(287);
	}
}

void ScriptRC04::PlayerWalkedOut() {
	Game_Flag_Reset(303);
}

void ScriptRC04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
