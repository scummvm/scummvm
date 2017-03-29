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

void SceneScriptBB08::InitializeScene() {
	if (Game_Flag_Query(219)) {
		Setup_Scene_Information(204.0f, 0.0f, 92.0f, 875);
	} else {
		Setup_Scene_Information(247.0f, 0.0f, 27.0f, 790);
	}
	Scene_Exit_Add_2D_Exit(0, 307, 0, 361, 238, 0);
	Scene_Exit_Add_2D_Exit(1, 117, 38, 214, 245, 0);
	Ambient_Sounds_Add_Looping_Sound(105, 44, 0, 1);
	Ambient_Sounds_Add_Sound(291, 1, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 1, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 1, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 1, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 1, 20, 20, 25, -100, 100, -100, 100, 0, 0);
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
	if (!Game_Flag_Query(496)) {
		Overlay_Play("BB08OVER", 0, 0, 0, 0);
		Game_Flag_Set(496);
	}
}

void SceneScriptBB08::SceneLoaded() {
	Obstacle_Object("BATHTUB", true);
	Unobstacle_Object("DOORWAY", true);
	Unclickable_Object("BATHTUB");
}

bool SceneScriptBB08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 204.0f, 0.1f, 94.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 256, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Ladder(kActorMcCoy, 8, 1, 0);
			Footstep_Sound_Override_Off();
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(218);
			Set_Enter(24, 10);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 247.0f, 0.1f, 27.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(506);
			Set_Enter(102, 120);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB08::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB08::SceneFrameAdvanced(int frame) {
}

void SceneScriptBB08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB08::PlayerWalkedIn() {
	if (Game_Flag_Query(219)) {
		Actor_Set_At_XYZ(kActorMcCoy, 204.0f, 96.1f, 94.0f, 256);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Ladder(kActorMcCoy, 8, 0, 0);
		Footstep_Sound_Override_Off();
		Actor_Face_Heading(kActorMcCoy, 768, false);
		Game_Flag_Reset(219);
	} else {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 188.0f, 0.1f, 28.0f, 0, 0, false, 0);
	}
}

void SceneScriptBB08::PlayerWalkedOut() {
}

void SceneScriptBB08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
