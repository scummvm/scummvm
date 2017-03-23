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

void SceneScriptDR03::InitializeScene() {
	if (Game_Flag_Query(226)) {
		Game_Flag_Reset(226);
		Setup_Scene_Information(330.31f, 4.27f, -910.91f, 297);
	}
	Setup_Scene_Information(330.31f, 4.27f, -910.91f, 297);
	Scene_Exit_Add_2D_Exit(0, 377, 122, 445, 266, 0);
	if (Global_Variable_Query(1) == 3) {
		Actor_Put_In_Set(52, 34);
		Actor_Set_At_XYZ(52, 431.21f, 4.27f, -776.26f, 0);
	} else {
		Actor_Put_In_Set(52, 34);
		Actor_Set_At_XYZ(52, 360.77f, 4.4f, -806.67f, 126);
	}
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Ambient_Sounds_Add_Looping_Sound(110, 7, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(109, 50, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(95, 20, 70, 1);
}

void SceneScriptDR03::SceneLoaded() {
	Obstacle_Object("W2-CENTCASE02", 1);
	Obstacle_Object("W2-CARTTOP", 1);
	Obstacle_Object("W2-TANKAFLUID01", 1);
}

bool SceneScriptDR03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptDR03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptDR03::ClickedOnActor(int actorId) {
	if (actorId == 52) {
		Actor_Face_Actor(0, 52, 1);
		Actor_Face_Actor(52, 0, 1);
		if (!Game_Flag_Query(267)) {
			Actor_Says(0, 755, 18);
			Actor_Says(52, 10, 14);
			Actor_Says(0, 760, 18);
			Actor_Says(52, 20, 14);
			Actor_Says(0, 765, 18);
			Actor_Says(52, 30, 14);
			Game_Flag_Set(267);
			return true;
		}
		if (Actor_Clue_Query(0, 67)) {
			if (Game_Flag_Query(266) && Game_Flag_Query(267)) {
				Actor_Says(0, 815, 18);
				Actor_Says(52, 60, 14);
				Actor_Says(52, 70, 14);
				Actor_Says(52, 80, 14);
				Actor_Says(0, 820, 18);
				Actor_Says(52, 90, 14);
				Actor_Says(0, 825, 18);
				Actor_Says(52, 100, 14);
				Game_Flag_Reset(266);
				Game_Flag_Set(505);
				return true;
			}
			if ((Actor_Clue_Query(0, 147) || Actor_Clue_Query(0, 71) || Actor_Clue_Query(0, 76) || Actor_Clue_Query(0, 67))
				&& Game_Flag_Query(505)) {
				sub_401B18();
			} else {
				Actor_Says(0, 810, 18);
				Actor_Says(52, 40, 14);
				Actor_Says(52, 50, 14);
			}
			return true;
		}
		Actor_Says(0, 770, 12);
		Actor_Says(52, 110, 12);
		Actor_Says(52, 120, 13);
		Actor_Says(0, 835, 13);
		Actor_Says(52, 130, 14);
		Actor_Says(0, 840, 16);
		Actor_Says(52, 140, 15);
		if (!Game_Flag_Query(505)) {
			Actor_Says(52, 150, 13);
			Actor_Says(0, 845, 17);
			Actor_Says(52, 170, 18);
			Actor_Says(52, 180, 16);
			Actor_Says(0, 850, 15);
			Actor_Says(52, 190, 14);
			Actor_Says(52, 200, 13);
			Actor_Says(0, 855, 18);
			Actor_Says(52, 210, 12);
		}
		Actor_Clue_Acquire(0, 67, 1, 52);
		return true;
	}
	return false;
}

bool SceneScriptDR03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptDR03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 330.31f, 4.27f, -910.91f, 24, 1, 0, 0)) {
			Game_Flag_Set(227);
			Set_Enter(7, 26);
		}
		return true;
	}
	return false;
}

bool SceneScriptDR03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptDR03::SceneFrameAdvanced(int frame) {
	if (frame == 1 || frame == 4 || frame == 8 || frame == 10 || frame == 19 || frame == 21 || frame == 22 || frame == 23 || frame == 30 || frame == 31 || frame == 32 || frame == 33 || frame == 46 || frame == 49) {
		if (Random_Query(0, 1)) {
			Sound_Play(97, Random_Query(20, 33), 80, 80, 50);
		} else {
			Sound_Play(59, Random_Query(5, 6), 80, 80, 50);
		}
	}
}

void SceneScriptDR03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptDR03::PlayerWalkedIn() {
	if (!Game_Flag_Query(226)) {
		if (Random_Query(1, 2) == 1) {
			Actor_Says(52, 660, 14);
			Actor_Says(52, 680, 14);
		} else if (Random_Query(1, 2) == 2) {
			Actor_Says(52, 670, 14);
			Actor_Says(52, 620, 14);
		} else {
			Actor_Says(52, 690, 14);
			Actor_Says(52, 710, 14);
		}
	}
}

void SceneScriptDR03::PlayerWalkedOut() {
}

void SceneScriptDR03::DialogueQueueFlushed(int a1) {
}

void SceneScriptDR03::sub_401B18() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 67) || Actor_Clue_Query(0, 71) || Actor_Clue_Query(0, 68)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(650, 5, 5, 5);
	}
	if (Actor_Clue_Query(0, 67)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(660, 5, 5, 5);
	}
	if (Actor_Clue_Query(0, 279)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(670, 6, 5, 2);
	}
	if (Game_Flag_Query(505)) {
		if (Actor_Clue_Query(0, 71)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(680, 8, 8, 8);
		}
		if (Actor_Clue_Query(0, 76)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(1270, 2, 5, 7);
		}
	}
	Dialogue_Menu_Add_DONE_To_List(690);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 640:
		Actor_Says(0, 770, 12);
		Actor_Says(52, 110, 12);
		Actor_Says(52, 120, 13);
		Actor_Says(0, 835, 13);
		Actor_Says(52, 130, 14);
		Actor_Says(0, 840, 16);
		Actor_Says(52, 140, 15);
		if (!Game_Flag_Query(505)) {
			Actor_Says(52, 150, 13);
			Actor_Says(0, 845, 17);
			Actor_Says(52, 170, 18);
			Actor_Says(52, 180, 16);
			Actor_Says(0, 850, 15);
			Actor_Says(52, 190, 14);
			Actor_Says(52, 200, 13);
			Actor_Says(0, 855, 18);
			Actor_Says(52, 210, 12);
		}
		Actor_Clue_Acquire(0, 67, 1, 52);
		break;
	case 650:
		Actor_Says(0, 775, 11);
		Actor_Says(52, 220, 14);
		Actor_Says(0, 860, 11);
		Actor_Says(52, 230, 14);
		Actor_Says(0, 865, 11);
		Actor_Says(52, 240, 14);
		Actor_Says(52, 250, 14);
		break;
	case 660:
		Actor_Says(0, 780, 13);
		if (Game_Flag_Query(505)) {
			Actor_Says(52, 260, 14);
			Actor_Says(52, 270, 13);
			Actor_Says(52, 280, 12);
		} else {
			Actor_Says(52, 260, 14);
			Actor_Says(52, 270, 13);
			Actor_Says(52, 280, 12);
			Actor_Says(0, 870, 18);
			Actor_Says(52, 290, 15);
			if (!Game_Flag_Query(266)) {
				Actor_Says(52, 300, 12);
			}
		}
		Actor_Clue_Acquire(0, 67, 1, 52);
		break;
	case 670:
		Actor_Says(0, 765, 12);
		Actor_Says(0, 790, 13);
		Actor_Says(52, 310, 12);
		Actor_Says(52, 320, 3);
		break;
	case 680:
		Actor_Says(0, 795, 3);
		if (Game_Flag_Query(505) == 1) {
			Actor_Says(52, 330, 12);
			Actor_Says(52, 340, 15);
			Actor_Says(0, 875, 16);
			Actor_Says(52, 350, 12);
			Actor_Says(52, 360, 15);
			Game_Flag_Set(326);
		} else {
			Actor_Says(52, 320, 13);
			Actor_Says(52, 150, 14);
			Game_Flag_Set(326);
		}
		break;
	case 1270:
		Actor_Says(0, 800, 16);
		Actor_Says(52, 370, 3);
		Actor_Says(0, 880, 15);
		Actor_Says(52, 380, 13);
		Actor_Says(52, 390, 12);
		Actor_Says(0, 885, 14);
		Actor_Says(52, 400, 13);
		Actor_Says(52, 410, 15);
		Actor_Says(0, 890, 18);
		Actor_Says(52, 420, 13);
		Actor_Says(52, 430, 12);
		break;
	case 690:
		Actor_Says(0, 805, 3);
		break;
	}
}

} // End of namespace BladeRunner
