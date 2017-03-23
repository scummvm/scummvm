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

void SceneScriptPS09::InitializeScene() {
	if (Game_Flag_Query(465)) {
		Setup_Scene_Information(-410.0f, 0.26f, -200.0f, 512);
	} else {
		Setup_Scene_Information(-559.0f, 0.0f, -85.06f, 250);
	}
	Scene_Exit_Add_2D_Exit(0, 0, 0, 30, 479, 3);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(138, 50, 0, 0);
	Ambient_Sounds_Add_Looping_Sound(137, 30, 0, 0);
	Ambient_Sounds_Add_Looping_Sound(124, 30, 0, 0);
	Ambient_Sounds_Add_Sound(125, 15, 60, 7, 10, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(126, 25, 60, 7, 10, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(127, 25, 60, 7, 10, 100, 100, -101, -101, 0, 0);
	if (!Game_Flag_Query(55)) {
		Actor_Put_In_Set(11, 67);
		Actor_Set_At_XYZ(11, -417.88f, 0.0f, -200.74f, 512);
		Game_Flag_Set(55);
	}
	if (Game_Flag_Query(465)) {
		Actor_Put_In_Set(11, 94);
		Actor_Set_At_XYZ(11, 0.0f, 0.0f, 0.0f, 512);
	}
	if (Game_Flag_Query(164) ) {
		Actor_Put_In_Set(7, 67);
		Actor_Set_At_XYZ(7, -476.0f, 0.2f, -225.0f, 518);
	}
	if (Game_Flag_Query(165) ) {
		Actor_Put_In_Set(9, 67);
		Actor_Set_At_XYZ(9, -290.0f, 0.33f, -235.0f, 207);
	}
}

void SceneScriptPS09::SceneLoaded() {
	Obstacle_Object("OFFICE DOOR", true);
	Unobstacle_Object("OFFICE DOOR", true);
	Unclickable_Object("OFFICE DOOR");
}

bool SceneScriptPS09::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptPS09::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptPS09::ClickedOnActor(int actorId) {
	if (actorId == 11 && !Loop_Actor_Walk_To_XYZ(0, -381.11f, 0.0f, -135.55f, 0, 1, false, 0)) {
		Actor_Face_Actor(0, 11, true);
		Actor_Face_Actor(11, 0, true);
		if (!Game_Flag_Query(49)) {
			Actor_Says(11, 0, 12);
			Actor_Says(0, 4235, 18);
			Actor_Says(11, 10, 13);
			Game_Flag_Set(49);
			return true;
		}
		if (Game_Flag_Query(49)  && !Game_Flag_Query(54) && !Actor_Clue_Query(0, 179) && !Actor_Clue_Query(0, 180) && !Actor_Clue_Query(0, 181)) {
			Actor_Says(0, 4245, 14);
			Actor_Says(11, 20, 14);
			Game_Flag_Set(54);
			return true;
		}
		if ((!Game_Flag_Query(53) && Game_Flag_Query(49)  && Actor_Clue_Query(0, 179) ) || Actor_Clue_Query(0, 180)  || Actor_Clue_Query(0, 181)  || Actor_Clue_Query(0, 99) ) {
			Game_Flag_Set(53);
			Actor_Says(0, 4240, 13);
			Actor_Says(11, 550, 15);
			Actor_Says(11, 480, 16);
			sub_402090();
			return true;
		}
		if (Game_Flag_Query(51) ) {
			Actor_Says(0, 4270, 18);
			Actor_Says(11, 30, 14);
			Actor_Says(11, 40, 13);
			return true;
		}
		if (Game_Flag_Query(53)  && Game_Flag_Query(49)  && (Actor_Clue_Query(0, 179)  || Actor_Clue_Query(0, 180)  || Actor_Clue_Query(0, 99) )) {
			sub_402090();
			return true;
		}
		Actor_Says(0, 4270, 18);
		Actor_Says(11, 30, 14);
		Actor_Says(11, 40, 13);
		return true;
	}
	if (actorId == 7 && !Loop_Actor_Walk_To_XYZ(0, -473.0f, 0.2f, -133.0f, 12, 1, false, 0)) {
		Actor_Face_Actor(0, 7, true);
		Actor_Face_Actor(7, 0, true);
		if (!Game_Flag_Query(167)) {
			Actor_Says(0, 4200, 14);
			Actor_Says(7, 570, 3);
			Actor_Says(0, 4205, 18);
			Game_Flag_Set(167);
			return true;
		}
		if (Game_Flag_Query(167)  && !Game_Flag_Query(168)) {
			Actor_Says(0, 4210, 18);
			Actor_Says(7, 580, 3);
			Actor_Says(0, 4215, 14);
			Actor_Says(7, 590, 3);
			Actor_Says(7, 600, 3);
			Actor_Says(0, 4220, 18);
			Actor_Says(7, 610, 3);
			Actor_Says(0, 4225, 19);
			Actor_Says(7, 620, 3);
			Actor_Says(0, 4230, 14);
			Game_Flag_Set(168);
			return true;
		}
		Actor_Says(0, 4200, 13);
	}
	if (actorId == 9 && !Loop_Actor_Walk_To_XYZ(0, -295.0f, 0.34f, -193.0f, 12, 1, false, 0)) {
		Actor_Face_Actor(0, 9, true);
		Actor_Face_Actor(9, 0, true);
		//TODO: cleanup
		if (Game_Flag_Query(166) || (Actor_Says(0, 4415, 18) , Actor_Says(9, 1090, 3) , Actor_Says(0, 4420, 18) , Game_Flag_Set(166) , Game_Flag_Query(166) != 1) || Game_Flag_Query(55) != 1 || Game_Flag_Query(56)) {
			if (!Game_Flag_Query(166) || Game_Flag_Query(55) || Game_Flag_Query(175)) {
				Actor_Says(0, 4425, 18);
				Actor_Says(9, 1160, 3);
				return true;
			} else {
				Actor_Says(0, 4425, 18);
				Actor_Says(9, 1100, 3);
				Actor_Says(0, 4430, 19);
				Actor_Says(9, 1110, 3);
				Game_Flag_Set(175);
				return true;
			}
		} else {
			Actor_Face_Actor(11, 9, true);
			Actor_Says(11, 420, 14);
			Actor_Face_Actor(9, 11, true);
			Actor_Says(9, 1120, 3);
			Actor_Face_Actor(0, 11, true);
			Actor_Says(0, 4435, 14);
			Actor_Says(11, 430, 16);
			Actor_Says(9, 1130, 3);
			Game_Flag_Set(56);
			return true;
		}
	}
	return false;
}

bool SceneScriptPS09::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptPS09::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -559.15f, 0.0f, -85.06f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(62, 66);
			Game_Flag_Reset(211);
		}
		return true;
	}
	return false;
}

bool SceneScriptPS09::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptPS09::SceneFrameAdvanced(int frame) {
	if (frame == 1 || frame == 15 || frame == 20 || frame == 31 || frame == 33 || frame == 35 || frame == 52 || frame == 54) {
		Sound_Play(97, Random_Query(50, 33), 10, 10, 50);
	}
	//return true;
}

void SceneScriptPS09::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptPS09::PlayerWalkedIn() {
	if (Game_Flag_Query(465)) {
		Player_Loses_Control();
		Delay(2000);
		Actor_Retired_Here(0, 6, 6, 1, -1);
		//return true;
		return;
	}
	if (!Game_Flag_Query(211)) {
		Player_Loses_Control();
		Loop_Actor_Walk_To_XYZ(0, -491.15f, 0.0f, -73.06f, 0, 0, false, 0);
		Player_Gains_Control();
		Game_Flag_Set(211);
	}
	if (Game_Flag_Query(133) ) {
		Game_Flag_Reset(133);
		//return true;
		return;
	}
	//return false;
}

void SceneScriptPS09::PlayerWalkedOut() {
}

void SceneScriptPS09::DialogueQueueFlushed(int a1) {
}

void SceneScriptPS09::sub_402090() {
	Dialogue_Menu_Clear_List();
	if (Actor_Clue_Query(0, 179)  || Actor_Clue_Query(0, 180)  || Actor_Clue_Query(0, 181) ) {
		DM_Add_To_List_Never_Repeat_Once_Selected(170, 5, 5, 3);
		DM_Add_To_List_Never_Repeat_Once_Selected(180, -1, 5, 5);
		DM_Add_To_List_Never_Repeat_Once_Selected(200, -1, 3, 6);
	}
	if (Actor_Clue_Query(0, 99) 		&& (Actor_Clue_Query(0, 179)  || Actor_Clue_Query(0, 180)  || Actor_Clue_Query(0, 181) )) {
		DM_Add_To_List_Never_Repeat_Once_Selected(190, 5, 6, -1);
	}
	Dialogue_Menu_Add_To_List(210);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 170:
		Actor_Says(0, 4270, 13);
		Actor_Says(0, 4250, 18);
		Actor_Says(11, 50, 13);
		Actor_Says(0, 4275, 18);
		Actor_Says(0, 4280, 19);
		if (Game_Flag_Query(44) ) {
			Actor_Says(11, 60, 14);
			Actor_Says(0, 4285, 13);
			Actor_Says(11, 70, 12);
			Actor_Says(0, 4290, 13);
			Actor_Says(11, 80, 13);
			Actor_Says(11, 90, 13);
			Actor_Says(0, 4295, 18);
			Actor_Says(11, 110, 14);
			Actor_Says(0, 4300, 17);
			return;
		}
		if (!Game_Flag_Query(44)) {
			Actor_Says(11, 130, 15);
			Actor_Says(11, 140, 13);
			Actor_Says(0, 4305, 13);
			Actor_Says(11, 150, 14);
			Actor_Says(11, 160, 12);
			Actor_Says(0, 4310, 13);
			Actor_Says(11, 170, 15);
			Actor_Says(11, 180, 16);
			Actor_Says(0, 4315, 18);
			Actor_Says(11, 200, 13);
			return;
		}
		break;
	case 180:
		Actor_Says(0, 4270, 18);
		Actor_Says(0, 4255, 3);
		Actor_Says(11, 210, 12);
		Actor_Says(11, 220, 13);
		Actor_Says(11, 230, 14);
		Actor_Says(0, 4320, 14);
		Actor_Says(11, 240, 16);
		Actor_Says(11, 250, 15);
		Actor_Says(0, 4330, 13);
		Actor_Says(11, 260, 13);
		Actor_Says(11, 270, 12);
		Actor_Says(0, 4335, 18);
		Actor_Says(11, 290, 15);
		Actor_Says(0, 4340, 13);
		Actor_Modify_Friendliness_To_Other(11, 0, -5);
		if (Game_Flag_Query(165)) {
			Actor_Says(11, 300, 12);
			Actor_Face_Actor(9, 11, true);
			Actor_Says(9, 1010, 3);
			Actor_Face_Actor(11, 9, true);
			Actor_Says(11, 310, 16);
			Actor_Face_Actor(0, 9, true);
			Actor_Says(0, 4345, 14);
			Actor_Face_Actor(9, 0, true);
			Actor_Says(9, 1020, 3);
			Actor_Says(0, 4350, 18);
			Actor_Says(9, 1030, 3);
			Actor_Says(0, 4355, 19);
			Actor_Says(9, 1040, 3);
			Actor_Says(0, 4360, 16);
			Actor_Says(0, 4365, 14);
			Actor_Says(9, 1050, 3);
			Actor_Says(9, 1060, 3);
			Actor_Says(0, 4370, 14);
			Actor_Says(9, 1070, 3);
			Actor_Says(9, 1080, 3);
		}
		else {
			Actor_Says(11, 320, 13);
			Actor_Says(11, 340, 14);
			Actor_Says(11, 350, 12);
			Actor_Says(0, 4375, 18);
		}
		break;
	case 190:
		Actor_Says(0, 4270, 18);
		Actor_Says(0, 4260, 3);
		Actor_Says(11, 360, 16);
		Actor_Says(0, 4380, 19);
		Actor_Says(0, 4385, 19);
		Actor_Says(11, 370, 13);
		Actor_Says(0, 4390, 19);
		Actor_Says(0, 4395, 18);
		Actor_Says(11, 380, 14);
		Actor_Says(11, 390, 12);
		Actor_Modify_Friendliness_To_Other(11, 0, -5);
		break;
	case 200:
		Actor_Says(0, 4265, 14);
		Actor_Says(11, 400, 13);
		Actor_Says(0, 4400, 13);
		Actor_Says(11, 410, 16);
		Actor_Says(0, 4405, 14);
		Actor_Says(0, 4410, 15);
		Voight_Kampff_Activate(11, 20);
		Actor_Modify_Friendliness_To_Other(11, 0, -10);
		break;
	case 210:
		Actor_Says(0, 8600, 18);
		Actor_Says(11, 20, 15);
		break;
	}
}

} // End of namespace BladeRunner
