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

void SceneScriptUG08::InitializeScene() {
	if (Game_Flag_Query(430)) {
		Setup_Scene_Information(-124.0f, 93.18f, 71.0f, 745);
	} else {
		Setup_Scene_Information(-432.0f, 0.0f, -152.0f, 370);
	}
	Scene_Exit_Add_2D_Exit(0, 125, 220, 157, 303, 3);
	Scene_Exit_Add_2D_Exit(1, 353, 145, 552, 309, 1);
	Ambient_Sounds_Add_Looping_Sound(331, 28, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(401, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(398, 2, 120, 11, 12, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(291, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(292, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(293, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(294, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(295, 2, 20, 20, 25, -100, 100, -100, 100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5, 50, 17, 37, -100, 100, -101, -101, 0, 0);
	if (!Game_Flag_Query(610)) {
		Game_Flag_Set(431);
		Game_Flag_Set(610);
	}
	if (Game_Flag_Query(430)) {
		Scene_Loop_Start_Special(0, 0, 0);
		Scene_Loop_Set_Default(1);
	} else if (Game_Flag_Query(431)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(4);
	}
}

void SceneScriptUG08::SceneLoaded() {
	Obstacle_Object("ELEV LEGS", true);
	Unobstacle_Object("ELEV LEGS", true);
	Unobstacle_Object("BOX RIGHT WALL ", true);
}

bool SceneScriptUG08::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG08::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG08::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG08::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG08::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -432.0f, 0.0f, -152.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(426);
			Set_Enter(kSetUG07, kSceneUG07);
		}
	} else if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -312.0f, -2.0f, 152.0f, 0, 1, false, 0)) {
			Actor_Face_Heading(kActorMcCoy, 240, false);
			Footstep_Sound_Override_On(2);
			Loop_Actor_Travel_Stairs(kActorMcCoy, 11, 1, kAnimationModeIdle);
			Footstep_Sound_Override_Off();
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, -118.02f, 93.02f, 52.76f, 0, 0, false, 0);
			Player_Loses_Control();
			Actor_Set_Invisible(kActorMcCoy, true);
			Game_Flag_Set(429);
			Game_Flag_Reset(431);
			Set_Enter(kSetUG13, kSceneUG13);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, 0);
		}
	}
	return false;
}

bool SceneScriptUG08::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG08::SceneFrameAdvanced(int frame) {
	if (frame == 91) {
		Ambient_Sounds_Play_Sound(372, 90, 0, 0, 100);
	}
}

void SceneScriptUG08::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG08::PlayerWalkedIn() {
	if (Game_Flag_Query(430)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -167.0f, 93.18f, 71.0f, 0, 0, false, 0);
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, -180.0f, 93.18f, 134.0f, 0, 0, false, 0);
		Actor_Face_Heading(kActorMcCoy, 745, false);
		Footstep_Sound_Override_On(2);
		Loop_Actor_Travel_Stairs(kActorMcCoy, 11, 0, kAnimationModeIdle);
		Footstep_Sound_Override_Off();
		Player_Gains_Control();
	}
	Game_Flag_Reset(425);
	Game_Flag_Reset(430);
}

void SceneScriptUG08::PlayerWalkedOut() {
}

void SceneScriptUG08::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
