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

void ScriptTB05::InitializeScene() {
	if (Game_Flag_Query(98)) {
		Setup_Scene_Information(23.0f, 151.53f, -205.0f, 450);
		Game_Flag_Set(102);
		Game_Flag_Reset(98);
	} else {
		Setup_Scene_Information(14.0f, 151.53f, -77.0f, 6);
	}
	Scene_Exit_Add_2D_Exit(0, 62, 193, 206, 419, 0);
	Scene_Exit_Add_2D_Exit(1, 0, 455, 639, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(236, 100, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(237, 100, 0, 1);
	Ambient_Sounds_Add_Sound(217, 5, 30, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(218, 5, 30, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(219, 5, 30, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(220, 5, 30, 25, 33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(146, 2, 30, 20, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(147, 2, 30, 20, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(148, 2, 30, 20, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(149, 2, 30, 20, 25, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(151, 2, 30, 20, 25, 0, 0, -101, -101, 0, 0);
	Scene_Loop_Set_Default(0);
}

void ScriptTB05::SceneLoaded() {
	Clickable_Object("MONITOR05");
	Unclickable_Object("SMUDGE_GLASS01");
	if (!Actor_Clue_Query(0, 44)) {
		Item_Add_To_World(76, 940, 72, 76.160004f, 147.36f, -235.14999f, 0, 6, 6, false, true, false, true);
	}
	if (!Actor_Clue_Query(0, 54) && !Actor_Clue_Query(0, 55) && (Game_Flag_Query(45) || Game_Flag_Query(46))) {
		Item_Add_To_World(119, 972, 72, 129.00999f, 147.12f, -162.98f, 0, 8, 8, false, true, false, true);
	}
}

bool ScriptTB05::MouseClick(int x, int y) {
	return false;
}

bool ScriptTB05::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("MONITOR05", objectName) && !Loop_Actor_Walk_To_XYZ(0, 122.54f, 147.12f, -197.17f, 0, 1, false, 0)) {
		Actor_Face_Heading(0, 38, false);
		if (!Actor_Clue_Query(0, 152) && !Game_Flag_Query(99)) {
			Actor_Clue_Acquire(0, 51, 1, -1);
			Actor_Voice_Over(2170, 99);
			Actor_Voice_Over(2180, 99);
			Actor_Voice_Over(2190, 99);
			Actor_Voice_Over(2200, 99);
			Game_Flag_Set(99);
			return true;
		}
		if (Game_Flag_Query(99) && !Game_Flag_Query(100) && !Actor_Clue_Query(0, 152)) {
			if (Actor_Clue_Query(0, 65) || Actor_Clue_Query(0, 262)) {
				Actor_Clue_Acquire(0, 152, 1, -1);
				Actor_Voice_Over(2230, 99);
				Item_Pickup_Spin_Effect(941, 352, 333);
				Actor_Voice_Over(2240, 99);
				Actor_Voice_Over(2250, 99);
				Actor_Voice_Over(2260, 99);
				Game_Flag_Set(100);
				Game_Flag_Set(101);
			} else {
				Actor_Voice_Over(2270, 99);
				Game_Flag_Set(100);
			}
			return true;
		}
		if (Game_Flag_Query(100) && !Game_Flag_Query(101)) {
			if (Actor_Clue_Query(0, 65) || Actor_Clue_Query(0, 262)) {
				Actor_Clue_Acquire(0, 152, 1, -1);
				Actor_Voice_Over(2230, 99);
				Item_Pickup_Spin_Effect(941, 352, 333);
				Actor_Voice_Over(2240, 99);
				Actor_Voice_Over(2250, 99);
				Actor_Voice_Over(2260, 99);
				Game_Flag_Set(101);
			} else {
				Actor_Voice_Over(2280, 99);
				Actor_Voice_Over(2290, 99);
				Game_Flag_Set(101);
			}
			return true;
		}
		if (Game_Flag_Query(101)) {
			Actor_Voice_Over(3700, 99);
			return true;
		}
		return false;
	}
	return false;
}

bool ScriptTB05::ClickedOnActor(int actorId) {
	return false;
}

bool ScriptTB05::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 76 && !Loop_Actor_Walk_To_XYZ(0, 54.0f, 147.12f, -209.0f, 0, 1, false, 0)) {
		Actor_Face_Heading(0, 126, false);
		Item_Remove_From_World(76);
		Item_Pickup_Spin_Effect(940, 295, 408);
		Actor_Voice_Over(2140, 99);
		Actor_Voice_Over(2150, 99);
		Actor_Voice_Over(2160, 99);
		Actor_Clue_Acquire(0, 44, 1, -1);
		return true;
	}
	if (itemId == 119 && !Loop_Actor_Walk_To_XYZ(0, 107.89f, 147.12f, -156.26f, 0, 1, false, 0)) {
		Actor_Face_Heading(0, 126, false);
		Item_Remove_From_World(119);
		Item_Pickup_Spin_Effect(972, 449, 431);
		Actor_Voice_Over(4280, 99);
		if (Game_Flag_Query(45)) {
			Actor_Voice_Over(4290, 99);
			Actor_Clue_Acquire(0, 54, 1, -1);
		} else {
			Actor_Voice_Over(4300, 99);
			Actor_Clue_Acquire(0, 55, 1, -1);
		}
	}
	return false;
}

bool ScriptTB05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(0, 23.0f, 151.53f, -205.0f, 12, 1, false, 0)) {
			Game_Flag_Set(97);
			Set_Enter(73, 85);
			Scene_Loop_Start_Special(1, 2, 1);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(0, 14.0f, 147.12f, 123.0f, 0, 1, false, 0)) {
			Game_Flag_Set(95);
			Set_Enter(17, 82);
		}
		return true;
	}
	return false;
}

bool ScriptTB05::ClickedOn2DRegion(int region) {
	return false;
}

void ScriptTB05::SceneFrameAdvanced(int frame) {
	if (frame == 61) {
		Sound_Play(150, Random_Query(52, 52), 0, 0, 50);
	}
	if (frame == 63) {
		Sound_Play(283, Random_Query(55, 55), 0, 0, 50);
	}
	//return true;
}

void ScriptTB05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void ScriptTB05::PlayerWalkedIn() {
}

void ScriptTB05::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void ScriptTB05::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
