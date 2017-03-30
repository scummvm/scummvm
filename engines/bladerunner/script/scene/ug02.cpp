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

void SceneScriptUG02::InitializeScene() {
	if (Game_Flag_Query(319)) {
		Setup_Scene_Information(-313.0f, 155.73f, -128.0f, 556);
	} else {
		Setup_Scene_Information(-95.0f, 74.78f, -503.0f, 556);
	}
	Scene_Exit_Add_2D_Exit(0, 529, 130, 607, 277, 0);
	Scene_Exit_Add_2D_Exit(1, 305, 36, 335, 192, 0);
	Ambient_Sounds_Add_Looping_Sound(332, 43, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 43, 0, 1);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, 100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(123, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(130, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(131, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(132, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(133, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(134, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(135, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(136, 2, 50, 17, 37, -50, -20, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(47, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(48, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(49, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(50, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(51, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(52, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(53, 2, 50, 27, 27, 10, 30, -101, -101, 0, 0);
}

void SceneScriptUG02::SceneLoaded() {
	Unobstacle_Object("BOX BACKROOM 2", true);
	Unobstacle_Object("BACK_ROOM HALFWALL_", true);
	Unobstacle_Object("GUN_4", true);
	Obstacle_Object("GUN_1", true);
	Unobstacle_Object("WALL_LEFT", true);
	Unobstacle_Object("BOX BY STAIRS 1", true);
	Unobstacle_Object("TANK", true);
	Unobstacle_Object("DESK_DRUM", true);
	Clickable_Object("GUN_1");
	Clickable_Object("GUN_2");
	Clickable_Object("CRATE_3");
	Footstep_Sounds_Set(0, 0);
	Footstep_Sounds_Set(8, 2);
	if (!Game_Flag_Query(656) && Game_Flag_Query(44)) {
		Item_Add_To_World(88, 963, 75, -300.37f, 120.16f, -81.31f, 0, 8, 8, false, true, false, true);
	}
}

bool SceneScriptUG02::MouseClick(int x, int y) {
	if (Game_Flag_Query(499)) {
		return false;
	}
	if (Region_Check(0, 0, 245, 285) || Region_Check(0, 0, 350, 257)) {
		return true;
	}
	if (Region_Check(81, 224, 639, 479) && !Game_Flag_Query(498)) {
		Game_Flag_Set(499);
		sub_402354();
		Game_Flag_Reset(499);
		return true;
	}
	return false;
}

bool SceneScriptUG02::ClickedOn3DObject(const char *objectName, bool a2) {
	if (Object_Query_Click("GUN_1", objectName) || Object_Query_Click("GUN_2", objectName) || Object_Query_Click("CRATE_3", objectName)) {
		Actor_Face_Object(kActorMcCoy, "GUN_1", true);
		if (!Game_Flag_Query(449) && Global_Variable_Query(1) < 4) {
			Actor_Voice_Over(2430, kActorVoiceOver);
			Actor_Voice_Over(2440, kActorVoiceOver);
			Actor_Voice_Over(2450, kActorVoiceOver);
			Actor_Voice_Over(2460, kActorVoiceOver);
			Game_Flag_Set(449);
			Actor_Clue_Acquire(kActorMcCoy, kClueWeaponsCache, 1, -1);
			return true;
		}
		if (Global_Variable_Query(1) <= 3) {
			Actor_Says(kActorMcCoy, 8580, 14);
			return false;
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueWeaponsCache) && !Actor_Clue_Query(kActorMcCoy, kClueIzosStashRaided)) {
			Actor_Voice_Over(2470, kActorVoiceOver);
			Actor_Voice_Over(2480, kActorVoiceOver);
			Actor_Voice_Over(2490, kActorVoiceOver);
			Actor_Voice_Over(2500, kActorVoiceOver);
			Actor_Clue_Acquire(kActorMcCoy, kClueIzosStashRaided, 1, -1);
		} else if (!Actor_Clue_Query(kActorMcCoy, kClueWeaponsCache)) {
			Actor_Voice_Over(2510, kActorVoiceOver);
			Actor_Voice_Over(2520, kActorVoiceOver);
			Actor_Voice_Over(2530, kActorVoiceOver);
		} else if (Game_Flag_Query(708)) {
			Actor_Says(kActorMcCoy, 8580, 14);
		} else {
			Item_Pickup_Spin_Effect(996, 360, 440);
			Actor_Says(kActorMcCoy, 8525, 14);
			Give_McCoy_Ammo(2, 18);
			Game_Flag_Set(708);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG02::ClickedOnItem(int itemId, bool a2) {
	if (itemId == 88) {
		Actor_Face_Item(kActorMcCoy, 88, true);
		Actor_Clue_Acquire(kActorMcCoy, kClueRadiationGoggles, 1, -1);
		Game_Flag_Set(656);
		Item_Remove_From_World(88);
		Item_Pickup_Spin_Effect(963, 426, 316);
		return true;
	}
	return false;
}

bool SceneScriptUG02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (Game_Flag_Query(498) || !sub_402354()) {
			int v2 = Player_Query_Combat_Mode();
			if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -202.0f, 120.16f, -74.0f, 0, 1, v2, 0)) {
				Actor_Face_Heading(kActorMcCoy, 270, false);
				Footstep_Sound_Override_On(2);
				Loop_Actor_Travel_Stairs(kActorMcCoy, 4, 0, 0);
				Footstep_Sound_Override_Off();
				int v3 = Player_Query_Combat_Mode();
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -96.57f, 74.870003f, -271.28f, 0, 0, v3, 0);
				int v4 = Player_Query_Combat_Mode();
				Loop_Actor_Walk_To_XYZ(kActorMcCoy, -95.0f, 74.870003f, -503.0f, 0, 0, v4, 0);
				Game_Flag_Set(315);
				Set_Enter(74, 86);
			}
		}
		return true;
	}
	if (exitId == 1) {
		if (Game_Flag_Query(498)) {
			if (sub_402354()) {
				return true;
			}
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -368.75f, 155.75f, -63.0f, 0, 0, false, 0);
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -340.75f, 155.75f, -119.0f, 0, 0, false, 0);
		}
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -304.75f, 155.75f, -171.0f, 0, 0, false, 0);
		Actor_Face_Heading(kActorMcCoy, 14, false);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 9, 1, 0);
		Game_Flag_Set(318);
		Game_Flag_Reset(259);
		Game_Flag_Set(479);
		if (!Game_Flag_Query(403)) {
			Game_Flag_Set(388);
			Game_Flag_Set(403);
			Item_Remove_From_World(121);
		}
		Set_Enter(8, 33);
		return true;
	}
	return false;
}

bool SceneScriptUG02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG02::SceneFrameAdvanced(int frame) {
	//return true;
}

void SceneScriptUG02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG02::PlayerWalkedIn() {
	if (Game_Flag_Query(314)) {
		Actor_Set_At_XYZ(kActorMcCoy, -106.01f, 84.13f, -228.62f, 575);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -148.0f, 84.13f, -67.0f, 0, 0, false, 0);
		Actor_Face_Heading(kActorMcCoy, 761, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, 1, 0);
		Footstep_Sound_Override_Off();
		Game_Flag_Reset(314);
		Game_Flag_Set(498);
	} else if (Game_Flag_Query(319)) {
		Actor_Set_At_XYZ(kActorMcCoy, -304.75f, 265.0f, -171.0f, 0);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 9, 0, 0);
		Game_Flag_Reset(319);
		Game_Flag_Reset(498);
	} else {
		Actor_Set_At_XYZ(kActorMcCoy, -269.24f, 120.16f, -9.94f, 477);
		Game_Flag_Set(498);
	}
	Game_Flag_Reset(499);
	//return false;
}

void SceneScriptUG02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG02::DialogueQueueFlushed(int a1) {
}

bool SceneScriptUG02::sub_402354() {
	if (!Game_Flag_Query(498)) {
		int v0 = Player_Query_Combat_Mode();
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -340.75f, 155.75f, -119.0f, 0, 0, v0, 0);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -368.75f, 155.75f, -63.0f, 0, 0, v0, 0);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -365.0f, 155.65f, -19.0f, 0, 0, v0, 0);
		Actor_Face_Heading(kActorMcCoy, 318, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, 0, 0);
		Footstep_Sound_Override_Off();
		Game_Flag_Set(498);
		return false;
	}
	if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -312.75f, 120.16f, 1.01f, 0, 1, false, 0)) {
		Actor_Face_Heading(kActorMcCoy, 830, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 4, 1, 0);
		Footstep_Sound_Override_Off();
		Game_Flag_Reset(498);
		return false;
	}
	return true;
}

} // End of namespace BladeRunner
