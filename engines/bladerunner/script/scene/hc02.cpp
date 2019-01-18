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

#include "bladerunner/script/scene_script.h"

namespace BladeRunner {

void SceneScriptHC02::InitializeScene() {
	Music_Play(4, 45, -60, 1, -1, 1, 3);
	if (Game_Flag_Query(kFlagHC04toHC02)) {
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

void SceneScriptHC02::SceneLoaded() {
	Obstacle_Object("BARSTOOL01", true);
}

bool SceneScriptHC02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptHC02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptHC02::ClickedOnActor(int actorId) {
	if (actorId == 32) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -150.51f, 0.14f, 62.74f, 0, 1, false, 0)) {
			Actor_Face_Actor(kActorMcCoy, kActorHawkersBarkeep, true);
			if (!Game_Flag_Query(404)) {
				Actor_Says(kActorMcCoy, 1225, 13);
				Actor_Says_With_Pause(kActorHawkersBarkeep, 0, 0.0f, 13);
				Actor_Says(kActorHawkersBarkeep, 10, 16);
				Actor_Set_Goal_Number(kActorHawkersBarkeep, 1);
				Actor_Change_Animation_Mode(kActorMcCoy, 23);
				Delay(1500);
				Actor_Change_Animation_Mode(kActorMcCoy, 75);
				Delay(1500);
				Global_Variable_Increment(42, 1);
				Game_Flag_Set(404);
			} else if (Actor_Clue_Query(kActorMcCoy, kClueChinaBarSecurityCamera) && !Actor_Clue_Query(kActorMcCoy, kClueChinaBarSecurityDisc)) {
				Actor_Says(kActorMcCoy, 4545, 11);
				Actor_Says(kActorHawkersBarkeep, 120, 12);
				Actor_Says(kActorHawkersBarkeep, 180, 13);
				Actor_Clue_Acquire(kActorMcCoy, kClueChinaBarSecurityDisc, 1, kActorHawkersBarkeep);
				Item_Pickup_Spin_Effect(975, 229, 215);
			} else if (Actor_Clue_Query(kActorMcCoy, kClueHomelessManInterview1) && !Actor_Clue_Query(kActorMcCoy, kClueFlaskOfAbsinthe) && (Global_Variable_Query(2) > 20 || Query_Difficulty_Level() == 0)) {
				Actor_Clue_Acquire(kActorMcCoy, kClueFlaskOfAbsinthe, 1, kActorHawkersBarkeep);
				Actor_Says(kActorMcCoy, 1230, 13);
				Actor_Says(kActorHawkersBarkeep, 20, 12);
				Actor_Says(kActorMcCoy, 1235, 13);
				Actor_Says(kActorHawkersBarkeep, 30, 15);
				Actor_Says(kActorMcCoy, 1240, 13);
				Actor_Says(kActorHawkersBarkeep, 40, 14);
				Item_Pickup_Spin_Effect(945, 229, 215);
				Actor_Set_Goal_Number(kActorHawkersBarkeep, 2);
				Actor_Change_Animation_Mode(kActorMcCoy, 23);
				Delay(1500);
				Actor_Says_With_Pause(kActorHawkersBarkeep, 50, 1.6f, 17);
				if (Query_Difficulty_Level() != 0) {
					Global_Variable_Decrement(2, 20);
				}
				Actor_Says(kActorMcCoy, 1245, 13);
			} else {
				if (Actor_Clue_Query(kActorMcCoy, kClueMaggieBracelet)  && !Game_Flag_Query(405)) {
					Actor_Says(kActorHawkersBarkeep, 80, 16);
					Actor_Says(kActorMcCoy, 1265, 13);
					Actor_Says(kActorHawkersBarkeep, 90, 13);
					Game_Flag_Set(405);
				}
				if (Global_Variable_Query(2) > 5 || Query_Difficulty_Level() == 0) {
					if (Query_Difficulty_Level() != 0) {
						Global_Variable_Decrement(2, 5);
					}
					Global_Variable_Increment(42, 1);
					Actor_Says(kActorMcCoy, 1250, 13);
					Actor_Says_With_Pause(kActorHawkersBarkeep, 60, 0.8f, 14);
					Actor_Says(kActorMcCoy, 1255, 13);
					Actor_Set_Goal_Number(kActorHawkersBarkeep, 1);
					Actor_Change_Animation_Mode(kActorMcCoy, 23);
					Delay(1500);
					Actor_Change_Animation_Mode(kActorMcCoy, 75);
					Delay(1500);
				} else {
					Actor_Says_With_Pause(kActorMcCoy, 1260, 0.3f, 13);
					Actor_Says(kActorHawkersBarkeep, 70, 14);
				}
			}
		}
		return true;
	}
	return false;
}

bool SceneScriptHC02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptHC02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 7.0f, 0.14f, 79.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(385);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC01);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.0f, 0.14f, -339.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagHC02toHC04);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -88.0f, 0.14f, -463.0f, 0, false);
			Set_Enter(kSetHC01_HC02_HC03_HC04, kSceneHC04);
		}
		return true;
	}
	return false;
}

bool SceneScriptHC02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptHC02::SceneFrameAdvanced(int frame) {
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

void SceneScriptHC02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptHC02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagHC04toHC02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.0f, 0.14f, -339.0f, 0, 0, false, 0);
		Game_Flag_Reset(kFlagHC04toHC02);
	}
}

void SceneScriptHC02::PlayerWalkedOut() {
}

void SceneScriptHC02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
