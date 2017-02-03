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

void ScriptHC04::InitializeScene() {
	if (Game_Flag_Query(108)) {
		Setup_Scene_Information(-112.0f, 0.14f, -655.0f, 460);
		Game_Flag_Reset(108);
	} else {
		Setup_Scene_Information(-88.0f, 0.14f, -463.0f, 1013);
	}
	Music_Play(4, 14, -90, 1, -1, 1, 2);
	Actor_Put_In_Set(59, 8);
	Actor_Set_At_XYZ(59, -210.0f, 0.0f, -445.0f, 250);
	Scene_Exit_Add_2D_Exit(0, 539, 51, 639, 309, 0);
	Scene_Exit_Add_2D_Exit(1, 0, 456, 639, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(103, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(329, 16, 16, 0);
	Ambient_Sounds_Add_Looping_Sound(330, 40, 40, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 16, 16, -100, -100, -101, -101, 0, 0);
	if (Game_Flag_Query(110)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(110);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void ScriptHC04::SceneLoaded() {
	Obstacle_Object("CAN FIRE", true);
	Unobstacle_Object("ASIANMALE01", true);
	Clickable_Object("CAN FIRE");
}

bool ScriptHC04::MouseClick(int x, int y) {
	return false;
}

bool ScriptHC04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptHC04::ClickedOnActor(int actorId) {
	if (actorId == 59) {
		if (!Loop_Actor_Walk_To_XYZ(0, -155.0f, 0.0f, -475.0f, 12, 1, false, 0)) {
			Actor_Face_Actor(0, 59, true);
			Actor_Face_Actor(59, 0, true);
			if (Game_Flag_Query(106)) {
				sub_401B90();
				return true;
			} else {
				Actor_Says(59, 0, 3);
				Actor_Says(0, 1280, 3);
				Actor_Says(59, 20, 3);
				Game_Flag_Set(106);
				return true;
			}
		}
	}
	return false;
}

bool ScriptHC04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptHC04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -108.0f, 0.14f, -639.0f, 0, 1, false, 0)) {
			Music_Stop(2);
			Game_Flag_Set(107);
			Game_Flag_Reset(479);
			Game_Flag_Set(182);
			Set_Enter(70, 80);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -72.0f, 0.14f, -399.0f, 0, 1, false, 0)) {
			Game_Flag_Set(109);
			Set_Enter(8, 32);
		}
		return true;
	}
	return false;
}

bool ScriptHC04::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptHC04::SceneFrameAdvanced(int frame) {
}

void ScriptHC04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptHC04::PlayerWalkedIn() {
}

void ScriptHC04::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptHC04::DialogueQueueFlushed(int a1) {
}

void ScriptHC04::sub_401B90() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 263) || Actor_Clue_Query(0, 53)) {
		if (Actor_Clue_Query(0, 47)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(340, 5, 6, 5);
		} else if (Actor_Clue_Query(0, 259)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(350, 5, 6, 5);
		}
		DM_Add_To_List_Never_Repeat_Once_Selected(360, 6, 4, 3);
	}
	if (Actor_Clue_Query(0, 87) && !Actor_Clue_Query(0, 101)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(370, 3, 4, 7);
	}
	if (Actor_Clue_Query(0, 101)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(380, -1, 5, 8);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(390, 7, 5, -1);
	Dialogue_Menu_Add_DONE_To_List(400);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 340:
		Actor_Says(0, 1285, 3);
		Actor_Says(59, 50, 3);
		Actor_Says(0, 1330, 3);
		Actor_Says(59, 60, 3);
		break;
	case 350:
		Actor_Says(0, 1290, 3);
		Actor_Says(59, 70, 3);
		Actor_Says(0, 1335, 3);
		Actor_Says(59, 80, 3);
		Actor_Says(0, 1340, 3);
		Actor_Says(59, 90, 3);
		Actor_Says(0, 1345, 3);
		break;
	case 360:
		Actor_Says(0, 1295, 3);
		Actor_Says(59, 100, 3);
		Actor_Says(0, 1350, 3);
		Actor_Says(59, 110, 3);
		Actor_Says(0, 1355, 3);
		Actor_Says(59, 130, 3);
		Actor_Says(0, 1360, 3);
		break;
	case 370:
		Actor_Says(0, 1300, 3);
		Actor_Says(59, 140, 3);
		Actor_Says(0, 1365, 3);
		Actor_Says(59, 150, 3);
		break;
	case 380:
		Actor_Says(0, 1305, 3);
		Actor_Modify_Friendliness_To_Other(59, 0, -2);
		Actor_Says(59, 160, 3);
		Actor_Says(0, 1370, 3);
		Actor_Says(59, 170, 3);
		Actor_Says(0, 1375, 3);
		Actor_Says(59, 180, 3);
		Actor_Says(0, 1380, 3);
		Actor_Says(59, 190, 3);
		Actor_Says(59, 210, 3);
		Actor_Says(59, 240, 3);
		Actor_Says(0, 1385, 3);
		Actor_Says(59, 260, 3);
		Actor_Says(0, 1390, 3);
		Actor_Says(59, 300, 3);
		Actor_Says(59, 310, 3);
		Actor_Says(59, 320, 3);
		Actor_Says(0, 1395, 3);
		Actor_Says(59, 330, 3);
		Actor_Clue_Acquire(0, 102, 0, 59);
		break;
	case 390:
		Actor_Says(0, 1310, 3);
		Actor_Modify_Friendliness_To_Other(59, 0, 2);
		Actor_Says(59, 340, 3);
		break;
	case 400:
		Actor_Says(0, 1315, 3);
		break;
	default:
		Actor_Says(0, 1320, 3);
		Actor_Says(59, 30, 3);
		Actor_Says(0, 1325, 3);
		Actor_Says(0, 1345, 3);
		break;
	}
}

} // End of namespace BladeRunner
