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

void ScriptMA02::InitializeScene() {
	if (Game_Flag_Query(36)) {
		Setup_Scene_Information(-172.0f, -144.13f, 6.27f, 500);
	} else {
		Setup_Scene_Information(23.19f, -144.12f, 378.27f, 750);
		if (Global_Variable_Query(1) == 4) {
			Actor_Set_Goal_Number(40, 300);
		}
		Game_Flag_Reset(711);
	}
	Scene_Exit_Add_2D_Exit(0, 538, 84, 639, 327, 1);
	Scene_Exit_Add_2D_Exit(1, 56, 98, 150, 260, 0);
	if (Global_Variable_Query(1) >= 4 && Global_Variable_Query(1) == 5 && Game_Flag_Query(653)) {
		Actor_Set_Goal_Number(66, 599);
		Actor_Change_Animation_Mode(66, 88);
		Actor_Put_In_Set(66, 10);
		Actor_Set_At_XYZ(66, -35.51f, -144.12f, 428.0f, 0);
		Actor_Retired_Here(66, 24, 24, 1, -1);
	}
	Ambient_Sounds_Add_Looping_Sound(104, 12, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(71, 25, 0, 1);
	Ambient_Sounds_Add_Sound(72, 5, 30, 5, 5, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 30, 5, 5, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 30, 5, 5, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 10, 60, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 10, 60, 20, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(87, 10, 60, 12, 12, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(68, 60, 180, 14, 14, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(69, 60, 180, 14, 14, 0, 0, -101, -101, 0, 0);
	if (sub_401F7C()) {
		Ambient_Sounds_Add_Sound(403, 3, 3, 27, 27, -100, -100, -100, -100, 99, 0);
	}
	if (Global_Variable_Query(1) == 5 && Game_Flag_Query(653) && !Actor_Clue_Query(0, 264)) {
		Overlay_Play("MA02OVER", 0, 1, 0, 0);
	}
}

void ScriptMA02::SceneLoaded() {
	Obstacle_Object("COUCH1", true);
	Unobstacle_Object("COUCH1", true);
	Clickable_Object("BAR-MAIN");
	Clickable_Object("E-ESPER");
}

bool ScriptMA02::MouseClick(int x, int y) {
	return false;
}

bool ScriptMA02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("E-ESPER", objectName)) {
		Actor_Face_Object(0, "E-ESPER", true);
		Delay(1000);
		ESPER_Flag_To_Activate();
		return true;
	}
	if (Object_Query_Click("BAR-MAIN", objectName) && !Loop_Actor_Walk_To_XYZ(0, -29.0f, -140.4f, 298.0f, 36, 1, false, 0)) {
		Actor_Face_Object(0, "BAR-MAIN", true);
		if (Global_Variable_Query(1) < 4) {
			Actor_Set_Goal_Number(66, 3);
		} else if (Global_Variable_Query(1) == 5 && Game_Flag_Query(653) && !Actor_Clue_Query(0, 264)) {
			Overlay_Remove("MA02OVER");
			Item_Pickup_Spin_Effect(985, 480, 240);
			Actor_Voice_Over(1150, 99);
			Actor_Voice_Over(1160, 99);
			Actor_Voice_Over(1170, 99);
			Actor_Voice_Over(1180, 99);
			Actor_Voice_Over(1190, 99);
			Actor_Voice_Over(1200, 99);
			Actor_Clue_Acquire(0, 264, 1, -1);
		} else {
			Actor_Says(0, 8526, 0);
		}
		return true;
	}
	return false;
}

bool ScriptMA02::ClickedOnActor(int actorId) {
	if (actorId == 66 && Actor_Query_Goal_Number(66) == 599) {
		if (!Loop_Actor_Walk_To_Actor(0, 66, 30, 1, false)) {
			Actor_Face_Actor(0, 66, true);
			Actor_Voice_Over(1140, 99);
		}
	}
	return false;
}

bool ScriptMA02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool ScriptMA02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 23.19f, -144.12f, 378.27f, 0, 1, false, 0)) {
			Music_Stop(10);
			Game_Flag_Set(33);
			Set_Enter(52, 52);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, -168.0f, -144.13f, 10.27f, 0, 1, false, 0)) {
			Game_Flag_Set(35);
			Set_Enter(50, 50);
		}
		return true;
	}
	return false;
}

bool ScriptMA02::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptMA02::SceneFrameAdvanced(int frame) {
}

void ScriptMA02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptMA02::PlayerWalkedIn() {
	if (Game_Flag_Query(34)) {
		sub_402044();
	}
	if (Game_Flag_Query(36)) {
		Loop_Actor_Walk_To_XYZ(0, -148.12f, -144.13f, 34.27f, 0, 1, false, 0);
	}
	if (Global_Variable_Query(1) == 4 && !Game_Flag_Query(655)) {
		Game_Flag_Set(623);
		Game_Flag_Set(655);
		sub_401E4C();
		Loop_Actor_Walk_To_XYZ(0, 23.19f, -144.12f, 378.27f, 0, 0, false, 0);
		Game_Flag_Set(33);
		Set_Enter(52, 52);
		//	return true;
		return;
	}
	if (Global_Variable_Query(1) == 5 && !Game_Flag_Query(654)) {
		if (Game_Flag_Query(653)) {
			Actor_Says(0, 2390, 0);
			Music_Play(2, 25, 0, 3, -1, 0, 0);
		} else {
			Actor_Says(0, 2385, 3);
		}
		Game_Flag_Set(654);
		Autosave_Game(3);
	}
	if (Global_Variable_Query(1) < 4 && !Game_Flag_Query(36) && Actor_Query_Goal_Number(66) != 2) {
		Actor_Set_Goal_Number(66, 1);
		if (!Game_Flag_Query(60)) {
			Game_Flag_Set(60);
			Actor_Face_Actor(0, 66, true);
			Actor_Voice_Over(1210, 99);
			if (!Game_Flag_Query(378)) {
				Actor_Voice_Over(1220, 99);
			}
			Actor_Voice_Over(1230, 99);
			if (!Game_Flag_Query(378)) {
				Actor_Voice_Over(1240, 99);
				Actor_Voice_Over(1250, 99);
			}
		}
	}
	Game_Flag_Reset(36);
	Game_Flag_Reset(34);
	//return false;
	return;
}

void ScriptMA02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptMA02::DialogueQueueFlushed(int a1) {
}

void ScriptMA02::sub_401E4C() {
	Actor_Says(0, 2365, 13);
	Actor_Says(40, 0, 13);
	Actor_Says(0, 2370, 13);
	Actor_Says(40, 10, 13);
	Actor_Says(0, 2375, 13);
	Actor_Says(40, 20, 13);
	Actor_Says(0, 2380, 13);
	Sound_Play(492, 100, 0, 100, 50);
	Actor_Says(40, 40, 13);
	Delay(3000);
}

bool ScriptMA02::sub_401F7C() {
	return Global_Variable_Query(1) == 5
		&& !Actor_Clue_Query(0, 143)
		&& !Actor_Clue_Query(0, 144)
		&& !Actor_Clue_Query(0, 139)
		&& !Actor_Clue_Query(0, 140)
		&& !Actor_Clue_Query(0, 141)
		&& !Actor_Clue_Query(0, 142);
}

void ScriptMA02::sub_402044() {
	// int v0;
	// int v1;
	// int v3[7];

	// v0 = 0;

	int i = 0;
	int arr[7];
	if (Global_Variable_Query(1) < 4 && Game_Flag_Query(45)) {
		// v0 = 1;
		// v3[0] = 0;
		arr[i++] = 0;
	}

	// v1 = v0 + 1;
	// v3[v0] = 1;
	arr[i++] = 1;
	if (Global_Variable_Query(1) >= 3) {
		// v3[v1] = 2;
		// v1 = v0 + 2;
		arr[i++] = 2;
	}
	if (Global_Variable_Query(1) >= 2 && Global_Variable_Query(1) <= 4) {
		// v3[v1++] = 3;
		arr[i++] = 3;
	}
	if (Game_Flag_Query(171) && Game_Flag_Query(170)) {
		// v3[v1++] = 4;
		arr[i++] = 4;
	}
	//if (v1 <= 0) {
	if (i == 0) {
		Global_Variable_Set(52, -1);
	} else {
		// Global_Variable_Set(52, v3[Random_Query(0, v1 - 1)]);
		Global_Variable_Set(52, arr[Random_Query(0, i - 1)]);
	}
}

} // End of namespace BladeRunner
