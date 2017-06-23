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

void SceneScriptBB04::InitializeScene() {
	if (Game_Flag_Query(283)) {
		Setup_Scene_Information(-107.0f, -26.6f, 397.0f, 29);
		Game_Flag_Reset(283);
	} else {
		Setup_Scene_Information(-15.0f, -25.17f, 45.0f, 691);
	}
	Scene_Exit_Add_2D_Exit(0, 218, 102, 360, 254, 1);
	Scene_Exit_Add_2D_Exit(1, 0, 334, 639, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(54, 20, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(105, 44, -100, 1);
	Ambient_Sounds_Add_Sound(443, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
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
	Ambient_Sounds_Add_Sound(72, 5, 80, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 80, 20, 20, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 80, 20, 20, -100, 100, -101, -101, 0, 0);
}

void SceneScriptBB04::SceneLoaded() {
	Obstacle_Object("DH TRASH", true);
}

bool SceneScriptBB04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -92.0f, -26.6f, 45.0f, 0, 1, false, 0)) {
			Player_Loses_Control();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -15.0f, -25.17f, 45.0f, 0, 0, false, 0);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(333);
			Game_Flag_Set(493);
			Set_Enter(1, 3);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -107.0f, -26.6f, 397.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(284);
			Set_Enter(21, 4);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB04::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB04::PlayerWalkedIn() {
	if (Game_Flag_Query(332)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -92.0f, -26.6f, 45.0f, 0, 0, false, 0);
		Player_Gains_Control();
		Game_Flag_Reset(332);
	}
}

void SceneScriptBB04::PlayerWalkedOut() {
}

void SceneScriptBB04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
