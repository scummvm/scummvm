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

void SceneScriptKP04::InitializeScene() {
	if (Game_Flag_Query(kFlagKP05toKP04)) {
		Setup_Scene_Information(-544.0f, 94.89f,  288.0f, 700);
	} else {
		Setup_Scene_Information(-905.0f, 94.89f, 1357.0f, 970);
	}
	Scene_Exit_Add_2D_Exit(0,   0, 455, 639, 479, 2);
	Scene_Exit_Add_2D_Exit(1, 475, 247, 514, 416, 1);

	Ambient_Sounds_Add_Looping_Sound(464, 34, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(383, 27, 1, 1);
	Ambient_Sounds_Add_Looping_Sound(384, 90, 1, 1);
	Ambient_Sounds_Add_Sound(440, 2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(441, 2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(442, 2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 50, 100, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(443, 2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 100, 25,  33, -100, 100, -101, -101, 0, 0);
}

void SceneScriptKP04::SceneLoaded() {
	Obstacle_Object("BUILDING04", true);
	Unobstacle_Object("BOX06", true);
	Unclickable_Object("BUILDING04");
}

bool SceneScriptKP04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptKP04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptKP04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptKP04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptKP04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -905.0f, 94.89f, 1357.0f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP04toKP01);
			Set_Enter(kSetKP01, kSceneKP01);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -544.0f, 94.89f, 288.0f, 0, true, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagKP04toKP05);
			Set_Enter(kSetKP05_KP06, kSceneKP05);
		}
		return true;
	}
	return false;
}

bool SceneScriptKP04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptKP04::SceneFrameAdvanced(int frame) {
}

void SceneScriptKP04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptKP04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagKP05toKP04)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -584.0f, 94.89f, 288.0f, 0, false, false, 0);
		Game_Flag_Reset(kFlagKP05toKP04);
	} else {
		Game_Flag_Reset(kFlagKP01toKP04);
	}
}

void SceneScriptKP04::PlayerWalkedOut() {
}

void SceneScriptKP04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
