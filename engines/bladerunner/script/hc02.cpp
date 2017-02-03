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

void ScriptHC02::InitializeScene() {
	Music_Play(4, 45, -60, 1, -1, 1, 3);
	if (Game_Flag_Query(109)) {
		Setup_Scene_Information(-88.0f, 0.14f, -463.0f, 540);
	} else {
		Setup_Scene_Information(-57.0f, 0.14f, 83.0f, 746);
	}
	Scene_Exit_Add_2D_Exit(0, 589, 255, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(1, 505, 0, 639, 170, 0);
	Ambient_Sounds_Add_Looping_Sound(103, 50, 50, 0);
	Ambient_Sounds_Add_Looping_Sound(280, 50, 50, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 33, 33, -60, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 33, 33, -60, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 33, 33, -60, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 33, 33, -60, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 33, 33, -60, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 33, 33, -60, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 33, 33, -100, 20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 33, 33, -100, 20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 33, 33, -100, 20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 33, 33, -100, 20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(242, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(243, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(244, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(245, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(246, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(247, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(248, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(249, 3, 30, 14, 14, 30, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(238, 3, 50, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(240, 3, 50, 25, 25, -100, 100, -101, -101, 0, 0);
	if (Game_Flag_Query(384)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
		Game_Flag_Reset(384);
	} else {
		Scene_Loop_Set_Default(1);
	}
}

void ScriptHC02::SceneLoaded() {
	Obstacle_Object("BARSTOOL01", true);
}

bool ScriptHC02::MouseClick(int x, int y) {
	return false;
}

bool ScriptHC02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool ScriptHC02::ClickedOnActor(int actorId) {
	if (actorId == 32) {
		if (!Loop_Actor_Walk_To_XYZ(0, -150.51f, 0.14f, 62.74f, 0, 1, false, 0)) {
			Actor_Face_Actor(0, 32, true);
			if (!Game_Flag_Query(404)) {
				Actor_Says(0, 1225, 13);
				Actor_Says_With_Pause(32, 0, 0.0f, 13);
				Actor_Says(32, 10, 16);
				Actor_Set_Goal_Number(32, 1);
				Actor_Change_Animation_Mode(0, 23);
				Delay(1500);
				Actor_Change_Animation_Mode(0, 75);
				Delay(1500);
				Global_Variable_Increment(42, 1);
				Game_Flag_Set(404);
			} else if (Actor_Clue_Query(0, 254) && !Actor_Clue_Query(0, 257)) {
				Actor_Says(0, 4545, 11);
				Actor_Says(32, 120, 12);
				Actor_Says(32, 180, 13);
				Actor_Clue_Acquire(0, 257, 1, 32);
				Item_Pickup_Spin_Effect(975, 229, 215);
			} else if (Actor_Clue_Query(0, 122) && !Actor_Clue_Query(0, 131) && (Global_Variable_Query(2) > 20 || Query_Difficulty_Level() == 0)) {
				Actor_Clue_Acquire(0, 131, 1, 32);
				Actor_Says(0, 1230, 13);
				Actor_Says(32, 20, 12);
				Actor_Says(0, 1235, 13);
				Actor_Says(32, 30, 15);
				Actor_Says(0, 1240, 13);
				Actor_Says(32, 40, 14);
				Item_Pickup_Spin_Effect(945, 229, 215);
				Actor_Set_Goal_Number(32, 2);
				Actor_Change_Animation_Mode(0, 23);
				Delay(1500);
				Actor_Says_With_Pause(32, 50, 1.6f, 17);
				if (Query_Difficulty_Level() != 0) {
					Global_Variable_Decrement(2, 20);
				}
				Actor_Says(0, 1245, 13);
			} else {
				if (Actor_Clue_Query(0, 75)  && !Game_Flag_Query(405)) {
					Actor_Says(32, 80, 16);
					Actor_Says(0, 1265, 13);
					Actor_Says(32, 90, 13);
					Game_Flag_Set(405);
				}
				if (Global_Variable_Query(2) > 5 || Query_Difficulty_Level() == 0) {
					if (Query_Difficulty_Level() != 0) {
						Global_Variable_Decrement(2, 5);
					}
					Global_Variable_Increment(42, 1);
					Actor_Says(0, 1250, 13);
					Actor_Says_With_Pause(32, 60, 0.8f, 14);
					Actor_Says(0, 1255, 13);
					Actor_Set_Goal_Number(32, 1);
					Actor_Change_Animation_Mode(0, 23);
					Delay(1500);
					Actor_Change_Animation_Mode(0, 75);
					Delay(1500);
				} else {
					Actor_Says_With_Pause(0, 1260, 0.3f, 13);
					Actor_Says(32, 70, 14);
				}
			}
		}
		return true;
	}
	return false;
}

bool ScriptHC02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptHC02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 7.0f, 0.14f, 79.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(385);
			Set_Enter(8, 31);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -76.0f, 0.14f, -339.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(110);
			Async_Actor_Walk_To_XYZ(0, -88.0f, 0.14f, -463.0f, 0, false);
			Set_Enter(8, 106);
		}
		return true;
	}
	return false;
}

bool ScriptHC02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptHC02::SceneFrameAdvanced(int frame) {
	if (frame == 70) {
		Sound_Play(73, 11, 50, -90, 50);
	}
	if (frame == 58) {
		Sound_Play(73, 11, 50, -90, 50);
	}
	if (frame == 69 || frame == 77 || frame == 86 || frame == 95 || frame == 104 || frame == 113 || frame == 119) {
		Sound_Play(60, Random_Query(6, 7), -20, 20, 50);
	}
}

void ScriptHC02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptHC02::PlayerWalkedIn() {
	if (Game_Flag_Query(109)) {
		Loop_Actor_Walk_To_XYZ(0, -76.0f, 0.14f, -339.0f, 0, 0, false, 0);
		Game_Flag_Reset(109);
	}
}

void ScriptHC02::PlayerWalkedOut() {
}

void ScriptHC02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
