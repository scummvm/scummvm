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

void SceneScriptNR02::InitializeScene() {
	sub_402134();
	Setup_Scene_Information(-283.0f, -24.0f, 326.0f, 200);
	Game_Flag_Reset(532);
	Scene_Exit_Add_2D_Exit(0, 0, 105, 75, 291, 3);
	Ambient_Sounds_Add_Looping_Sound(280, 50, 38, 0);
	Ambient_Sounds_Add_Sound(252, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(254, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(255, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(256, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(257, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(258, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(259, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(260, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(261, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(262, 3, 60, 25, 25, -100, 100, -101, -101, 0, 0);
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

void SceneScriptNR02::SceneLoaded() {
	Obstacle_Object("VID PHONE 01", true);
	Unobstacle_Object("VICTORIAN CHAIR", true);
	Unobstacle_Object("WALL CANDLES", true);
	Unobstacle_Object("STAIRS", true);
	Unobstacle_Object("BOX30", true);
	Unobstacle_Object("VID CAM COVER", true);
	Unobstacle_Object("VID CAM COVER01", true);
	Unobstacle_Object("VID OUTER GLASS", true);
	Unobstacle_Object("VID OUTER GLASS01", true);
	Unobstacle_Object("VID MAIN MONITOR", true);
	Unobstacle_Object("VID MAIN MONITOR01", true);
	Clickable_Object("VID PHONE 01");
	Clickable_Object("VID PHONE 02");
}

bool SceneScriptNR02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptNR02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("VID PHONE 01", objectName) || Object_Query_Click("VID PHONE 02", objectName)) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -191.9f, -24.0f, 62.15f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 13, false);
			if (Actor_Clue_Query(kActorMcCoy, kClueFolder) && Actor_Clue_Query(kActorMcCoy, kClueGuzzaFramedMcCoy) && !Game_Flag_Query(670)) {
				Actor_Set_Goal_Number(kActorMcCoy, 350);
				Game_Flag_Set(670);
			} else {
				Sound_Play(123, 50, 0, 0, 50);
				Delay(1000);
				Sound_Play(403, 30, 0, 0, 50);
				Delay(1500);
				Sound_Play(403, 30, 0, 0, 50);
				Delay(1500);
				Sound_Play(403, 30, 0, 0, 50);
				Delay(1500);
				Sound_Play(123, 50, 0, 0, 50);
				Delay(1000);
				Actor_Says(kActorMcCoy, 170, 14);
			}
		}
	}
	return false;
}

bool SceneScriptNR02::ClickedOnActor(int actorId) {
	if (actorId == 2 && Actor_Query_Goal_Number(kActorGordo) == 201 && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 67.37f, -24.0f, 389.32f, 0, 1, false, 0)) {
		Actor_Set_Goal_Number(kActorGordo, 215);
	}
	return false;
}

bool SceneScriptNR02::ClickedOnItem(int itemId, bool a2) {
	if ((itemId == 89 || itemId == 90) && !Loop_Actor_Walk_To_XYZ(kActorMcCoy, 109.38f, -24.0f, 420.5f, 0, 1, false, 0)) {
		Actor_Face_Heading(kActorMcCoy, 423, false);
		if (itemId == 89) {
			Item_Remove_From_World(89);
			Item_Pickup_Spin_Effect(953, 214, 380);
			Actor_Clue_Acquire(kActorMcCoy, kClueGordosLighter1, 1, -1);
		}
		if (itemId == 90) {
			Item_Remove_From_World(90);
			Item_Pickup_Spin_Effect(954, 214, 380);
			Actor_Clue_Acquire(kActorMcCoy, kClueGordosLighter2, 1, -1);
		}
	}
	return false;
}

bool SceneScriptNR02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -283.0f, -24.0f, 326.0f, 0, 1, false, 0)) {
			if (Actor_Query_Goal_Number(kActorGordo) < 230 || Actor_Query_Goal_Number(kActorGordo) > 250) {
				Music_Stop(2);
			}
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(533);
			Set_Enter(54, 54);
		}
		return true;
	}
	return false;
}

bool SceneScriptNR02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptNR02::SceneFrameAdvanced(int frame) {
	if (!Music_Is_Playing() && (Actor_Query_Goal_Number(kActorGordo) < 210 || Actor_Query_Goal_Number(kActorGordo) > 222)) {
		sub_402134();
	}
	//return false;
}

void SceneScriptNR02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptNR02::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorGordo) == 211) {
		Actor_Set_Goal_Number(kActorGordo, 220);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 204) {
		Actor_Set_Goal_Number(kActorGordo, 205);
	}
	if (Actor_Query_Goal_Number(kActorGordo) == 206) {
		Actor_Set_Goal_Number(kActorGordo, 205);
	}
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -203.0f, -24.0f, 334.0f, 0, 0, false, 0);
	//return false;
}

void SceneScriptNR02::PlayerWalkedOut() {
	Music_Stop(2);
	if (Actor_Query_Goal_Number(kActorGordo) < 210 && Actor_Query_Goal_Number(kActorGordo) >= 205) {
		Actor_Set_Goal_Number(kActorGordo, 204);
	}
}

void SceneScriptNR02::DialogueQueueFlushed(int a1) {
	if (Player_Query_Current_Scene() == 55 && Actor_Query_Goal_Number(kActorGordo) == 206) {
		Sound_Play(575, 50, 0, 0, 50);
		Sound_Play(321, 50, 0, 0, 50);
	}
	if (Player_Query_Current_Scene() == 55 && Actor_Query_Goal_Number(kActorGordo) == 207) {
		Sound_Play(576, 50, 0, 0, 50);
		Sound_Play(323, 50, 0, 0, 50);
	}
	if (Player_Query_Current_Scene() == 55 && Actor_Query_Goal_Number(kActorGordo) == 208) {
		Sound_Play(579, 50, 0, 0, 50);
		Sound_Play(324, 50, 0, 0, 50);
	}
	if (Player_Query_Current_Scene() == 55 && Actor_Query_Goal_Number(kActorGordo) > 205 && Actor_Query_Goal_Number(kActorGordo) < 210) {
		Actor_Set_Goal_Number(kActorGordo, 205);
		//return true;
		return;
	} else if (Actor_Query_Goal_Number(kActorGordo) > 205 && Actor_Query_Goal_Number(kActorGordo) < 210) {
		Actor_Set_Goal_Number(kActorGordo, 204);
		//return true;
		return;
	}
	//return false;
}

void SceneScriptNR02::sub_402134() {
	int v0 = Global_Variable_Query(50);
	if (v0 == 0) {
		Music_Play(8, 41, 0, 2, -1, 0, 0);
	} else if (v0 == 1) {
		Music_Play(9, 41, 0, 2, -1, 0, 0);
	} else if (v0 == 2) {
		Music_Play(10, 41, 0, 2, -1, 0, 0);
	}
	v0++;
	if (v0 > 2) {
		v0 = 0;
	}
	Global_Variable_Set(50, v0);
}

} // End of namespace BladeRunner
