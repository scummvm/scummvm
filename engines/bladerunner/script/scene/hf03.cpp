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

void SceneScriptHF03::InitializeScene() {
	if (Game_Flag_Query(527)) {
		Setup_Scene_Information(479.0f, 47.76f, -496.0f, 600);
	} else {
		Setup_Scene_Information(185.62f, 47.76f, -867.42f, 300);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);
	Scene_Exit_Add_2D_Exit(1, 589, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(2, 323, 110, 380, 166, 0);
	Ambient_Sounds_Add_Looping_Sound(340, 50, 0, 1);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
}

void SceneScriptHF03::SceneLoaded() {
	Obstacle_Object("MAIN", true);
	Unclickable_Object("MAIN");
}

bool SceneScriptHF03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHF03::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click(objectName, "MAIN")) {
		Actor_Says(0, Random_Query(0, 3) + 8525, 18);
	}
	return false;
}

void SceneScriptHF03::sub_401C80() {
	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(840, -1, 3, 8);
	DM_Add_To_List_Never_Repeat_Once_Selected(850, 6, 5, 2);
	DM_Add_To_List_Never_Repeat_Once_Selected(860, 8, -1, -1);
	DM_Add_To_List_Never_Repeat_Once_Selected(870, 2, 8, 6);
	Dialogue_Menu_Add_DONE_To_List(880);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 840:
		Actor_Says(0, 1630, 15);
		if (Global_Variable_Query(40) == 3) {
			Actor_Set_Goal_Number(6, 214);
		} else if (Game_Flag_Query(46)) {
			Actor_Set_Goal_Number(6, 212);
		} else {
			Actor_Set_Goal_Number(6, 210);
			Game_Flag_Set(593);
		}
		break;
	case 850:
		Actor_Says(0, 1635, 15);
		Actor_Says(6, 200, 13);
		Actor_Modify_Friendliness_To_Other(6, 0, 3);
		break;
	case 860:
		Actor_Says(0, 1640, 12);
		if (Global_Variable_Query(40) == 3) {
			Actor_Set_Goal_Number(6, 214);
		} else {
			Actor_Says(6, 210, 13);
			Actor_Says(0, 1655, 15);
			Actor_Modify_Friendliness_To_Other(6, 0, Random_Query(9, 10));
			if (Actor_Query_Friendliness_To_Other(6, 0) > 59 && !Global_Variable_Query(45)) {
				Global_Variable_Set(45, 3);
				Actor_Says(6, 940, 14);
				Actor_Says(0, 6780, 11);
				Actor_Says(6, 950, 12);
				Actor_Says(6, 960, 13);
				Actor_Says(0, 6785, 15);
				Actor_Says(6, 970, 16);
				Actor_Says(6, 980, 17);
				if (Game_Flag_Query(47)) {
					Actor_Says(6, 990, 17);
				}
				Actor_Says(0, 6790, 15);
				Actor_Says(6, 1000, 13);
				Actor_Says(6, 1010, 17);
				Actor_Says(6, 1020, 18);
				Actor_Says(0, 6795, 14);
				Actor_Says(6, 1030, 17);
				Actor_Says(0, 6800, 14);
			}
			Actor_Says(6, 220, 13);
			Actor_Says(0, 1660, 15);
			Actor_Says(6, 230, 14);
			Actor_Clue_Acquire(6, 219, 1, 0);
			if (Game_Flag_Query(46)) {
				Actor_Set_Goal_Number(6, 212);
			} else {
				Actor_Set_Goal_Number(6, 210);
			}
		}
		break;
	case 870:
		Actor_Says(0, 1645, 18);
		Actor_Says(6, 240, 14);
		Actor_Says(6, 250, 12);
		Actor_Says(6, 260, 13);
		Actor_Says(6, 270, 19);
		Actor_Says(0, 1665, 18);
		Actor_Says(6, 280, 13);
		Actor_Says(0, 1670, 12);
		Actor_Says(6, 290, 14);
		Actor_Says(6, 300, 16);
		Actor_Says(0, 1675, 12);
		Actor_Says(6, 310, 13);
		Actor_Clue_Acquire(0, 273, 0, 6);
		break;
	case 880:
		Actor_Says(0, 1650, 14);
		break;
	}
}

bool SceneScriptHF03::ClickedOnActor(int actorId) {
	if (actorId == 6 && Actor_Query_Goal_Number(6) == 205) {
		if (Game_Flag_Query(46) ? !Loop_Actor_Walk_To_Waypoint(0, 377, 0, 1, false) : !Loop_Actor_Walk_To_Waypoint(0, 378, 0, 1, false)) {
			Actor_Face_Actor(0, 6, true);
			if (!Game_Flag_Query(613)) {
				Game_Flag_Set(613);
				if (Game_Flag_Query(46)) {
					Actor_Says(0, 1605, 15);
					Actor_Says(6, 100, 12);
					Actor_Says(0, 1610, 14);
				} else {
					Actor_Says(0, 1615, 16);
					Actor_Says(6, 110, 13);
				}
				Actor_Says(6, 120, 13);
				Actor_Says(0, 1620, 14);
				Actor_Says(6, 130, 17);
				Actor_Says(0, 1625, 15);
				if (Game_Flag_Query(46)) {
					Actor_Says(6, 140, 12);
					Actor_Says(6, 150, 13);
					Actor_Says(6, 160, 15);
				} else {
					Actor_Says(6, 170, 12);
					Actor_Says(6, 180, 13);
					Actor_Says(6, 190, 15);
				}
			}
			sub_401C80();
		}
	}
	return false;
}

bool SceneScriptHF03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHF03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 179.0f, 46.76f, -824.0f, 0, 1, false, 0)) {
			Game_Flag_Set(311);
			Set_Enter(37, 34);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 479.0f, 47.76f, -524.0f, 0, 1, false, 0)) {
			Game_Flag_Set(528);
			Set_Enter(38, 35);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(0, 942.0f, 47.76f, -847.0f, 0, 1, false, 0)) {
			Game_Flag_Set(566);
			Set_Enter(40, 37);
		}
		return true;
	}
	return false;
}

bool SceneScriptHF03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHF03::SceneFrameAdvanced(int frame) {
}

void SceneScriptHF03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHF03::PlayerWalkedIn() {
	if (Game_Flag_Query(527)) {
		Loop_Actor_Walk_To_XYZ(0, 479.0f, 47.76f, -524.0f, 0, 0, false, 0);
		Game_Flag_Reset(527);
	} else {
		Loop_Actor_Walk_To_XYZ(0, 179.0f, 47.76f, -824.0f, 0, 0, false, 0);
		Game_Flag_Reset(310);
	}
	if (Actor_Query_Goal_Number(6) == 250) {
		Actor_Set_Goal_Number(6, 212);
		Actor_Says(1, 210, 13);
		Actor_Face_Actor(0, 1, true);
		Actor_Says(0, 1680, 15);
		Actor_Says(1, 220, 14);
		Actor_Says(0, 1685, 13);
		Actor_Says(1, 230, 16);
		Actor_Says(0, 1690, 12);
		Actor_Says(1, 240, 13);
		Actor_Set_Goal_Number(1, 234);
	}
}

void SceneScriptHF03::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptHF03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
