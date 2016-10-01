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

void ScriptCT04::InitializeScene() {
	if (Game_Flag_Query(72)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(-150.0f, -621.3f, 357.0f, 533);
	} else {
		Scene_Loop_Set_Default(1);
		Setup_Scene_Information(-82.86f, -621.3f, 769.03f, 1020);
	}
	Scene_Exit_Add_2D_Exit(0, 590, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 194, 84, 320, 274, 0);
	Ambient_Sounds_Add_Looping_Sound(54, 50, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(56, 15, -100, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 34, 100, 1);
	Ambient_Sounds_Add_Sound(68, 10, 40, 33, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 10, 40, 33, 50, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Speech_Sound(60, 0, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 20, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 40, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Speech_Sound(60, 50, 10, 260, 17, 24, -100, 100, -101, -101, 1, 1);
	Ambient_Sounds_Add_Sound(376, 10, 60, 33, 50, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 10, 60, 33, 50, -100, 100, -101, -101, 0, 0);
}

void ScriptCT04::SceneLoaded() {
	Obstacle_Object("DUMPSTER", true);
	Obstacle_Object("RIGHTWALL01", true);
	Obstacle_Object("BACK-BLDNG", true);
	Clickable_Object("DUMPSTER");
	Footstep_Sounds_Set(0, 1);
	if (Game_Flag_Query(72)) {
		Game_Flag_Reset(72);
	}
	if (!Actor_Query_Goal_Number(12)) {
		Actor_Change_Animation_Mode(12, 38);
	}
}

bool ScriptCT04::MouseClick(int x, int y) {
	return false;
}

bool ScriptCT04::ClickedOn3DObject(const char *objectName, bool a2) {
	if (objectName) {
		if (!Game_Flag_Query(137) && !Game_Flag_Query(169) && !Actor_Query_Goal_Number(12)) {
			Game_Flag_Set(137);
			Actor_Set_Goal_Number(12, 2);
		}
		if (Game_Flag_Query(169) && !Game_Flag_Query(170) && !Game_Flag_Query(171) && !Game_Flag_Query(172) && Global_Variable_Query(1) == 1) {
			if (!Loop_Actor_Walk_To_XYZ(0, -147.41f, -621.3f, 724.57f, 0, 1, false, 0)) {
				Player_Loses_Control();
				Actor_Face_Heading(0, 792, false);
				Actor_Put_In_Set(12, 99);
				Actor_Set_At_XYZ(12, 0, 0, 0, 0);
				Actor_Change_Animation_Mode(0, 40);
				Actor_Voice_Over(320, 99);
				Actor_Voice_Over(330, 99);
				Actor_Voice_Over(340, 99);
				Game_Flag_Set(170);
				Game_Flag_Set(173);
			}
			return false;
		}
		if (Game_Flag_Query(170)) {
			if (Game_Flag_Query(172)) {
				Actor_Voice_Over(270, 99);
				Actor_Voice_Over(280, 99);
			} else if (Game_Flag_Query(171)) {
				Actor_Voice_Over(250, 99);
				Actor_Voice_Over(260, 99);
			} else {
				Actor_Voice_Over(230, 99);
				Actor_Voice_Over(240, 99);
				Game_Flag_Reset(173);
			}
			return true;
		}
		if (Game_Flag_Query(174)) {
			if (!Loop_Actor_Walk_To_Waypoint(0, 75, 0, 1, false)) {
				Actor_Face_Heading(0, 707, false);
				Actor_Change_Animation_Mode(0, 38);
				Ambient_Sounds_Play_Sound(553, 45, 30, 30, 0);
				Actor_Voice_Over(1810, 99);
				Actor_Voice_Over(1820, 99);
				return true;
			}
			return false;
		}
		if (!Loop_Actor_Walk_To_Waypoint(0, 75, 0, 1, false)) {
			Actor_Face_Heading(0, 707, false);
			Actor_Change_Animation_Mode(0, 38);
			Actor_Clue_Acquire(0, 37, 1, -1);
			Item_Pickup_Spin_Effect(952, 392, 225);
			Game_Flag_Set(174);
			return true;
		}
	}
	return false;
}

void ScriptCT04::sub_401D4C() {
	Dialogue_Menu_Clear_List();
	if (Global_Variable_Query(2) > 10 || Query_Difficulty_Level() == 0) {
		DM_Add_To_List_Never_Repeat_Once_Selected(410, 8, 4, -1);
	}
	DM_Add_To_List_Never_Repeat_Once_Selected(420, 2, 6, 8);
	Dialogue_Menu_Appear(320, 240);
	int answer = Dialogue_Menu_Query_Input();
	Dialogue_Menu_Disappear();
	if (answer == 410) {
		Actor_Says(12, 10, 14);
		Actor_Says(12, 20, 14);
		Actor_Modify_Friendliness_To_Other(12, 0, 5);
		if (Query_Difficulty_Level() != 0) {
			Global_Variable_Decrement(2, 10);
		}
	} else if (answer == 420) {
		Actor_Says(0, 430, 3);
		Actor_Says(12, 30, 14);
		Actor_Modify_Friendliness_To_Other(12, 0, -5);
	}
}

bool ScriptCT04::ClickedOnActor(int actorId) {
	if (actorId == 12) {
		if (Game_Flag_Query(169)) {
			if (!Loop_Actor_Walk_To_Actor(0, 12, 36, 1, false)) {
				Actor_Voice_Over(290, 99);
				Actor_Voice_Over(300, 99);
				Actor_Voice_Over(310, 99);
			}
		} else {
			Actor_Set_Targetable(12, false);
			if (!Loop_Actor_Walk_To_Actor(0, 12, 36, 1, false)) {
				Actor_Face_Actor(0, 12, true);
				if (!Game_Flag_Query(137)) {
					if (Game_Flag_Query(40)) {
						Actor_Says(0, 435, 3);
						Actor_Set_Goal_Number(12, 2);
					} else {
						Music_Stop(3);
						Actor_Says(0, 425, 3);
						Actor_Says(12, 0, 13);
						sub_401D4C();
						Actor_Set_Goal_Number(12, 2);
					}
					Game_Flag_Set(137);
				} else {
					Actor_Face_Actor(0, 12, true);
					Actor_Says(0, 435, 3);
				}
			}
		}
		return true;
	}
	return false;
}

bool ScriptCT04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptCT04::ClickedOnExit(int exitId) {
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -82.86f, -621.3f, 769.03f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			if (!Actor_Query_Goal_Number(12)) {
				Actor_Set_Goal_Number(12, 2);
			}
			Game_Flag_Set(74);
			Set_Enter(28, 17);
		}
		return true;
	}
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, -187.0f, -621.3f, 437.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(73);
			Set_Enter(5, 15);
		}
		return true;
	}
	return false;
}

bool ScriptCT04::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptCT04::SceneFrameAdvanced(int frame) {
	if (Game_Flag_Query(325)) {
		Game_Flag_Reset(325);
		Sound_Play(180, 100, 80, 80, 50);
	}
}

void ScriptCT04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptCT04::PlayerWalkedIn() {
}

void ScriptCT04::PlayerWalkedOut() {
}

void ScriptCT04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
