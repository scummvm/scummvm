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

void SceneScriptUG19::InitializeScene() {
	if (Game_Flag_Query(357)) {
		Setup_Scene_Information(67.03f, 105.0f, -74.97f, 256);
	} else {
		Setup_Scene_Information(181.0f, 11.52f, -18.0f, 777);
	}
	Scene_Exit_Add_2D_Exit(0, 351, 0, 491, 347, 0);
	Scene_Exit_Add_2D_Exit(1, 548, 124, 639, 369, 1);
	Ambient_Sounds_Add_Looping_Sound(95, 45, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 76, 0, 1);
	Ambient_Sounds_Add_Sound(291, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 25, 33, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(401, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(396, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(397, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(1, 5, 50, 47, 57, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(57, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(58, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(306, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(307, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(308, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(196, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(197, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(198, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(199, 5, 50, 27, 37, -100, 100, -101, -101, 0, 0);

}

void SceneScriptUG19::SceneLoaded() {
	Obstacle_Object("LADDER", true);
	Unclickable_Object("LADDER");
	Footstep_Sounds_Set(1, 0);
	Footstep_Sounds_Set(0, 3);
}

bool SceneScriptUG19::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG19::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG19::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG19::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG19::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 67.03f, 7.29f, -74.97f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 256, false);
			Footstep_Sound_Override_On(3);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 8, 1, 0);
			Footstep_Sound_Override_Off();
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(356);
			Set_Enter(kSetMA07, kSceneMA07);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 181.0f, 11.52f, -18.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(349);
			Set_Enter(kSetUG14, kSceneUG14);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG19::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG19::SceneFrameAdvanced(int frame) {
}

void SceneScriptUG19::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG19::PlayerWalkedIn() {
	if (Game_Flag_Query(348)) {
		Game_Flag_Reset(348);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 129.0f, 11.52f, -18.0f, 0, 0, false, 0);
	} else {
		Game_Flag_Reset(357);
		Footstep_Sound_Override_On(3);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 8, 0, 0);
		Footstep_Sound_Override_Off();
	}
}

void SceneScriptUG19::PlayerWalkedOut() {
}

void SceneScriptUG19::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
