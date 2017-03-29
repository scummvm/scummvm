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

void SceneScriptBB05::InitializeScene() {
	if (Game_Flag_Query(298)) {
		Setup_Scene_Information(95.0f, -60.31f, 331.0f, 0);
	} else if (Game_Flag_Query(302)) {
		Setup_Scene_Information(87.0f, -60.34f, -96.0f, 0);
	} else if (Game_Flag_Query(300)) {
		Setup_Scene_Information(271.0f, -60.31f, 203.0f, 0);
	} else {
		Setup_Scene_Information(-212.0f, -60.31f, 131.0f, 0);
	}
	Scene_Exit_Add_2D_Exit(0, 92, 125, 187, 317, 3);
	Scene_Exit_Add_2D_Exit(1, 0, 0, 30, 479, 3);
	Scene_Exit_Add_2D_Exit(2, 589, 0, 639, 479, 1);
	Scene_Exit_Add_2D_Exit(3, 481, 113, 573, 307, 0);
	Ambient_Sounds_Add_Looping_Sound(54, 12, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 14, 0, 1);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(309, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(310, 5, 50, 17, 27, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(90, 5, 50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(91, 5, 50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(72, 5, 80, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 80, 14, 14, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 80, 14, 14, -100, 100, -101, -101, 0, 0);
	if (!Game_Flag_Query(495)) {
		Overlay_Play("BB05OVER", 0, 0, 0, 0);
		Game_Flag_Set(495);
	}
}

void SceneScriptBB05::SceneLoaded() {
	Obstacle_Object("PINHEAD", true);
	Obstacle_Object("X2WALLS&MOLDNG05", true);
	Obstacle_Object("QUADPATCH04", true);
	Unobstacle_Object("BOX16", true);
	Clickable_Object("PINHEAD");
	Clickable_Object("BOX06");
	Unclickable_Object("BOX06");
	Unclickable_Object("BOX14");
	if (Actor_Query_Goal_Number(kActorSebastian) == 200) {
		Actor_Set_Goal_Number(kActorGeneralDoll, 299);
		Actor_Put_In_Set(kActorGeneralDoll, 97);
		Actor_Set_At_Waypoint(kActorGeneralDoll, 39, 0);
	}
}

bool SceneScriptBB05::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB05::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB05::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB05::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB05::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -212.0f, -60.31f, 131.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(286);
			Set_Enter(21, 4);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 95.0f, -60.31f, 331.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(297);
			Set_Enter(2, 7);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 271.0f, -60.31f, 203.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(299);
			Set_Enter(3, 8);
		}
		return true;
	}
	if (exitId == 3) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 151.0f, -60.34f, -108.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(301);
			Set_Enter(102, 120);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB05::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB05::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB05::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB05::PlayerWalkedIn() {
	if (Game_Flag_Query(298)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 95.0f, -60.31f, 303.0f, 0, 0, false, 0);
		Game_Flag_Reset(298);
	} else if (Game_Flag_Query(300)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 231.0f, -60.31f, 203.0f, 0, 0, false, 0);
		Game_Flag_Reset(300);
	} else if (Game_Flag_Query(302)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 111.0f, -60.31f, -24.0f, 0, 0, false, 0);
		Game_Flag_Reset(302);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -76.0f, -60.31f, 131.0f, 0, 0, false, 0);
		Game_Flag_Reset(285);
	}
	if (Actor_Query_Goal_Number(kActorSebastian) == 200) {
		Actor_Face_Actor(kActorSebastian, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorSebastian, true);
		Actor_Says(kActorSebastian, 120, 13);
		Actor_Says(kActorMcCoy, 7030, 15);
		Actor_Says(kActorSebastian, 130, 17);
		Actor_Says(kActorSebastian, 140, 16);
		Actor_Says(kActorSebastian, 150, 14);
		Actor_Says(kActorSebastian, 160, 15);
		Actor_Says(kActorMcCoy, 7035, 14);
		Actor_Says(kActorSebastian, 170, 12);
		Actor_Says(kActorMcCoy, 7040, 14);
		Actor_Says(kActorSebastian, 180, 16);
		Actor_Says(kActorMcCoy, 7045, 14);
		if (Game_Flag_Query(399)) {
			Actor_Says(kActorSebastian, 190, 15);
			Actor_Says(kActorMcCoy, 7050, 17);
			Actor_Says(kActorSebastian, 200, 16);
			Actor_Says_With_Pause(kActorSebastian, 210, 1.5f, 14);
			Actor_Says(kActorMcCoy, 7055, 15);
		} else {
			Actor_Put_In_Set(kActorGeneralDoll, 22);
			Actor_Set_At_Waypoint(kActorGeneralDoll, 134, 0);
			Loop_Actor_Walk_To_Waypoint(kActorGeneralDoll, 135, 0, 0, false);
			Actor_Says(kActorGeneralDoll, 0, 3);
			Actor_Face_Actor(kActorMcCoy, kActorGeneralDoll, true);
			Actor_Face_Actor(kActorSebastian, kActorGeneralDoll, true);
			Actor_Says(kActorSebastian, 220, 13);
			Loop_Actor_Walk_To_Waypoint(kActorGeneralDoll, 134, 0, 0, false);
			Actor_Face_Actor(kActorSebastian, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorSebastian, true);
			Actor_Says(kActorSebastian, 230, 15);
			Actor_Says(kActorMcCoy, 7060, 17);
			Actor_Says(kActorSebastian, 240, 12);
		}
		Actor_Says(kActorMcCoy, 7065, 16);
		Actor_Says(kActorSebastian, 250, 16);
		Actor_Says(kActorMcCoy, 7070, 18);
		Actor_Set_Goal_Number(kActorSebastian, 205);
		Actor_Set_Goal_Number(kActorGeneralDoll, 201);
		Actor_Set_Goal_Number(kActorBryant, 101);
		Actor_Set_Goal_Number(kActorGeneralDoll, 200);
	}
}

void SceneScriptBB05::PlayerWalkedOut() {
}

void SceneScriptBB05::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
