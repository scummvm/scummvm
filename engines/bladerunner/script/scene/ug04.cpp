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

void SceneScriptUG04::InitializeScene() {
	if (Game_Flag_Query(kFlagUG05toUG04)) {
		Setup_Scene_Information(   0.0f, -1.74f, -2400.0f, 496);
		Game_Flag_Reset(kFlagUG05toUG04);
	} else if (Game_Flag_Query(kFlagUG06toUG04)) {
		Setup_Scene_Information( 164.0f, 11.87f, -1013.0f,  83);
	} else {
		Setup_Scene_Information(-172.0f, 16.29f,  -735.0f, 380);
		Game_Flag_Reset(kFlagUG03toUG04);
	}

	Scene_Exit_Add_2D_Exit(0, 123, 308, 159, 413, 3);
	if (Global_Variable_Query(kVariableChapter) > 3) {
		Scene_Exit_Add_2D_Exit(1, 256, 333, 290, 373, 0);
	}
	Scene_Exit_Add_2D_Exit(2, 344, 298, 451, 390, 1);

	Ambient_Sounds_Add_Looping_Sound(331, 25, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(332, 40, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(333, 40, 0, 1);
	Ambient_Sounds_Add_Sound(234, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(224, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(225, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(227, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(229, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(368, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(369, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(370, 2, 120, 11, 12, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(235, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(392, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(394, 2, 190, 12, 16, -100, 100, -100,  100, 0, 0);
	Ambient_Sounds_Add_Sound(303, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(304, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(305, 5,  50, 17, 37, -100, 100, -101, -101, 0, 0);

	if ((Global_Variable_Query(kVariableChapter) == 3)
		|| (Global_Variable_Query(kVariableChapter) > 3 && Random_Query(1, 5) == 1)
	){ // enhancement: don't always play the passing train after chapter 3
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
	}
	Scene_Loop_Set_Default(1);
}

void SceneScriptUG04::SceneLoaded() {
	Obstacle_Object("NAV", true);
	Unobstacle_Object("RUBBLE", true);
	Unobstacle_Object("FLOOR DEBRIS WADS", true);
	Unobstacle_Object("FLOOR DEBRIS WADS01", true);
	Unobstacle_Object("FLOOR DEBRIS WADS02", true);
}

bool SceneScriptUG04::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptUG04::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptUG04::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptUG04::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptUG04::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -172.0f, 16.29f, -735.0f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagUG04toUG03);
			Set_Enter(kSetUG03, kSceneUG03);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 0.0f, -1.74f, -2400.0f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagUG04toUG05);
			Set_Enter(kSetUG05, kSceneUG05);
		}
		return true;
	}

	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 164.0f, 11.87f, -1013.0f, 0, true, false, 0)) {
			Game_Flag_Set(kFlagUG04toUG06);
			Set_Enter(kSetUG06, kSceneUG06);
		}
		return true;
	}
	return false;
}

bool SceneScriptUG04::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptUG04::SceneFrameAdvanced(int frame) {
	if (frame == 1) { // TODO - Maybe only play this sound when the train shows up
		Ambient_Sounds_Play_Sound(kSfxSUBWAY1, 90, -100, 100, 100);
	}
}

void SceneScriptUG04::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptUG04::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagUG06toUG04)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 60.0f, -1.74f, -976.0f, 6, false, false, 0);
		Game_Flag_Reset(kFlagUG06toUG04);
	}
}

void SceneScriptUG04::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptUG04::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
