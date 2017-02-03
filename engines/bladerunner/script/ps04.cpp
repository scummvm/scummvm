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

void ScriptPS04::InitializeScene() {
	AI_Movement_Track_Pause(4);
	if (Game_Flag_Query(42)) {
		Game_Flag_Reset(42);
	}
	Setup_Scene_Information(-668.0f, -354.0f, 974.0f, 475);
	if (Global_Variable_Query(1) == 1) {
		Actor_Put_In_Set(4, 64);
		Actor_Set_At_XYZ(4, -728.0f, -354.0f, 1090.0f, 150);
		Actor_Change_Animation_Mode(4, 53);
	}
	Scene_Exit_Add_2D_Exit(0, 347, 113, 469, 302, 0);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(0);
	Ambient_Sounds_Add_Looping_Sound(45, 16, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(46, 50, 1, 1);
	Ambient_Sounds_Add_Sound(47, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(48, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(49, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(50, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(51, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(52, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(53, 9, 40, 20, 20, 0, 0, -101, -101, 0, 0);
	Scene_Loop_Start_Special(0, 0, 0);
	Scene_Loop_Set_Default(1);
}

void ScriptPS04::SceneLoaded() {
	Obstacle_Object("CHAIR07", true);
	Unobstacle_Object("GOOD B.WALL", true);
	Unobstacle_Object("B.DOOR", true);
	Unobstacle_Object("B.CHAIR01", true);
	Unclickable_Object("CHAIR07");
	if (Global_Variable_Query(1) == 2 && !Actor_Clue_Query(0, 80) && !Game_Flag_Query(727)) {
		Item_Add_To_World(111, 958, 64, -643.5f, -318.82f, 1148.87f, 525, 16, 12, false, true, false, true);
		Game_Flag_Set(727);
	}
	if (Actor_Query_Is_In_Current_Set(4)) {
		Actor_Change_Animation_Mode(4, 53);
	}
}

bool ScriptPS04::MouseClick(int x, int y) {
	return false;
}

bool ScriptPS04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptPS04::ClickedOnActor(int actorId) {
	if (actorId == 4) {
		if (!Loop_Actor_Walk_To_Actor(0, 4, 36, 1, false)) {
			Actor_Face_Actor(0, 4, true);
			Actor_Face_Actor(4, 0, true);
			sub_4017E4();
			return true;
		}
	}
	return false;
}

bool ScriptPS04::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 111 && Actor_Query_Is_In_Current_Set(4)) {
		Actor_Says(4, 560, 30);
	} else if (!Actor_Clue_Query(0, 80)) {
		Item_Remove_From_World(111);
		Item_Pickup_Spin_Effect(958, 464, 362);
		Actor_Says(0, 4485, 3);
		Actor_Clue_Acquire(0, 80, 1, 0);
	}
	return false;
}

bool ScriptPS04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -668.0f, -350.85f, 962.0f, 0, 1, false, 0)) {
			Game_Flag_Set(39);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Set_Enter(63, 67);
		}
		return true;
	}
	return false;
}

bool ScriptPS04::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptPS04::SceneFrameAdvanced(int frame) {
}

void ScriptPS04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptPS04::PlayerWalkedIn() {
	if (Actor_Query_Which_Set_In(4) == 64) {
		Actor_Face_Actor(0, 4, true);
	}
	//return false;
}

void ScriptPS04::PlayerWalkedOut() {
	AI_Movement_Track_Unpause(4);
}

void ScriptPS04::DialogueQueueFlushed(int a1) {
}

void ScriptPS04::sub_4017E4() {
	Dialogue_Menu_Clear_List();
	if (Global_Variable_Query(1) > 1) {
		if (Actor_Clue_Query(0, 51)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(110, 5, 7, 4);
		}
		DM_Add_To_List_Never_Repeat_Once_Selected(120, 1, -1, -1);
		if (Actor_Clue_Query(0, 110)) {
			DM_Add_To_List_Never_Repeat_Once_Selected(150, 7, 6, 5);
		}
	}
	if (Game_Flag_Query(169) == 1) {
		DM_Add_To_List_Never_Repeat_Once_Selected(140, 3, -1, -1);
	}
	DM_Add_To_List(130, 1, 1, 1);
	Dialogue_Menu_Add_DONE_To_List(160);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	switch (answer) {
	case 130:
		if (Game_Flag_Query(40) && !Game_Flag_Query(159)) {
			Actor_Says(0, 3920, 13);
			Actor_Says(4, 140, 30);
			Actor_Face_Current_Camera(4, true);
			Actor_Says(4, 150, 31);
			Actor_Says(4, 160, 32);
			Actor_Says(0, 3925, 18);
			Actor_Face_Actor(4, 0, true);
			Actor_Says(4, 170, 33);
			Loop_Actor_Walk_To_XYZ(0, -716.0f, -354.85f, 1042.0f, 0, 0, false, 0);
			Actor_Face_Actor(0, 4, true);
			Actor_Says(0, 3930, 13);
			Actor_Face_Actor(4, 0, true);
			Actor_Says(4, 180, 34);
			Actor_Says(0, 3935, 13);
			Actor_Says(4, 190, 30);
			Actor_Says(0, 3940, 16);
			Actor_Says(4, 200, 31);
			Actor_Says(4, 210, 33);
			Actor_Says(4, 220, 34);
			Actor_Says(0, 3945, 17);
			Actor_Says(4, 230, 32);
			Actor_Says(4, 240, 31);
			Actor_Says(0, 3950, 13);
			Actor_Says(4, 250, 34);
			Actor_Says(4, 260, 33);
			Actor_Says(4, 270, 32);
			Game_Flag_Set(159);
			if (Query_Difficulty_Level() != 0) {
				Global_Variable_Increment(2, 200);
			}
			Game_Flag_Set(723);
		} else if (Game_Flag_Query(41) && !Game_Flag_Query(160)) {
			Actor_Says(0, 3955, 13);
			Actor_Says(4, 280, 30);
			Actor_Says(0, 3960, 18);
			Actor_Says(4, 290, 32);
			Actor_Says(4, 300, 31);
			Actor_Says(0, 3965, 13);
			Actor_Says(4, 310, 33);
			Actor_Says(4, 320, 34);
			Game_Flag_Set(160);
		} else if ((Actor_Clue_Query(0, 8) || Actor_Clue_Query(0, 9)) && Actor_Clue_Query(0, 22) && Actor_Query_Friendliness_To_Other(4, 0) < 50 && !Game_Flag_Query(161)) {
			Actor_Says(0, 3970, 18);
			Actor_Says(4, 330, 30);
			Actor_Says(4, 340, 32);
			Actor_Says(0, 3975, 13);
			Actor_Says(4, 350, 31);
			Actor_Says(4, 360, 34);
			Actor_Says(0, 3980, 13);
			Actor_Says(4, 370, 33);
			Actor_Says(4, 380, 32);
			Actor_Says(4, 390, 31);
			Actor_Says(0, 3985, 18);
			Actor_Says(4, 400, 34);
			Actor_Says(4, 410, 31);
			Game_Flag_Set(161);
		} else if ((Actor_Clue_Query(0, 8) || Actor_Clue_Query(0, 9))
			&& Actor_Clue_Query(0, 22)
			&& !Game_Flag_Query(162)) {
			Actor_Says(0, 3920, 13);
			Actor_Says(4, 570, 32);
			Actor_Says(0, 4070, 13);
			Game_Flag_Set(162);
		} else if (Actor_Query_Friendliness_To_Other(4, 0) >= 50) {
			Actor_Says(0, 4020, 13);
			Actor_Says(4, 580, 34);
			Actor_Says(0, 4075, 16);
			Actor_Says(4, 590, 33);
		} else {
			Actor_Says(0, 4020, 18);
			Actor_Says(4, 130, 30);
			Actor_Face_Current_Camera(4, true);
			Actor_Says(0, 3915, 13);
		}
		break;
	case 110:
		Actor_Says(0, 3990, 19);
		Actor_Says(0, 3995, 17);
		Actor_Says(4, 440, 31);
		Actor_Says(0, 4035, 13);
		Actor_Says(4, 450, 34);
		Actor_Says(4, 460, 33);
		Actor_Says(0, 4040, 17);
		Game_Flag_Set(625);
		break;
	case 120:
		Actor_Says(0, 4000, 18);
		Actor_Clue_Acquire(0, 82, 1, 4);
		Actor_Says(4, 520, 33);
		Actor_Says(0, 4055, 13);
		Actor_Says(4, 530, 31);
		Actor_Says(0, 4060, 13);
		Actor_Says(4, 540, 31);
		Actor_Says(4, 550, 32);
		Actor_Says(0, 4065, 18);
		Actor_Says(4, 560, 34);
		if (Query_Difficulty_Level() != 0) {
			Global_Variable_Increment(2, 100);
		}
		break;
	case 140:
		Actor_Says(0, 4010, 12);
		Actor_Says(4, 600, 31);
		Actor_Says(0, 4080, 18);
		Actor_Says(4, 610, 33);
		Actor_Face_Heading(4, 400, false);
		Actor_Says(4, 620, 32);
		Actor_Face_Actor(4, 0, true);
		Actor_Says(4, 700, 34);
		Actor_Says(0, 4100, 13);
		Actor_Says(4, 710, 31);
		Actor_Says(4, 720, 34);
		Actor_Says(0, 4105, 18);
		Loop_Actor_Walk_To_XYZ(0, -668.0f, -350.85f, 962.0f, 0, 0, false, 0);
		Actor_Says(4, 730, 32);
		Actor_Face_Actor(0, 4, true);
		Loop_Actor_Walk_To_XYZ(0, -716.0f, -354.85f, 1042.0f, 0, 0, false, 0);
		Actor_Face_Actor(4, 0, true);
		Actor_Says(4, 740, 31);
		Actor_Says(4, 750, 32);
		Actor_Says(4, 760, 33);
		Actor_Face_Actor(0, 4, true);
		Actor_Says(0, 4110, 13);
		Actor_Says(4, 770, 32);
		Actor_Says(4, 780, 31);
		break;
	case 150:
		Actor_Says(0, 4015, 16);
		Actor_Says(4, 630, 34);
		Actor_Says(0, 4085, 19);
		Actor_Says(0, 4090, 18);
		Actor_Says(4, 640, 31);
		Actor_Says(4, 650, 32);
		Actor_Says(4, 670, 34);
		Actor_Says(0, 4095, 17);
		Actor_Says(4, 680, 32);
		Actor_Says(4, 690, 31);
		break;
	default:
		//TODO: what is this for?
		//answer != 160;
		break;
	}
}

} // End of namespace BladeRunner
