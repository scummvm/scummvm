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

void SceneScriptBB02::InitializeScene() {
	if (Game_Flag_Query(kFlagBB03toBB02)) {
		Setup_Scene_Information(179.0f, -415.06f, 274.0f, 904);
	} else if (Game_Flag_Query(333)) {
		Setup_Scene_Information(-12.0f, -415.06f, -27.0f, 264);
		Scene_Loop_Start_Special(0, 0, 0);
	} else {
		Setup_Scene_Information(98.0f, -415.06f, -593.0f, 530);
		Game_Flag_Reset(kFlagBB01toBB02);
	}
	Scene_Exit_Add_2D_Exit(0, 313, 137, 353, 173, 0);
	Scene_Exit_Add_2D_Exit(1, 207, 291, 275, 443, 3);
	Scene_Exit_Add_2D_Exit(2, 303, 422, 639, 479, 2);
	Ambient_Sounds_Add_Looping_Sound(54, 20, 0, 1);
	Ambient_Sounds_Add_Looping_Sound(103, 40, 0, 1);
	Ambient_Sounds_Add_Sound(443, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(444, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(445, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(446, 2, 180, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(82, 5, 60, 20, 40, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(83, 5, 60, 20, 45, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(84, 5, 60, 20, 40, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(90, 5, 50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(91, 5, 50, 17, 17, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(375, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(376, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(377, 5, 180, 25, 25, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(72, 5, 80, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(73, 5, 80, 14, 16, -100, 100, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(74, 5, 80, 14, 16, -100, 100, -101, -101, 0, 0);
	if (!Game_Flag_Query(494)) {
		Game_Flag_Set(493);
		Game_Flag_Set(494);
	}
	if (Game_Flag_Query(493)) {
		Scene_Loop_Set_Default(1);
	} else {
		Scene_Loop_Set_Default(4);
	}
}

void SceneScriptBB02::SceneLoaded() {
	Obstacle_Object("ELEVATOR01", true);
	Obstacle_Object("U2 DOOR", true);
}

bool SceneScriptBB02::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptBB02::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptBB02::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptBB02::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptBB02::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 98.0f, -415.06f, -593.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagBB02toBB01);
			Set_Enter(kSetBB01, kSceneBB01);
		}
		return true;
	}
	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -415.06f, -27.0f, 0, 1, false, 0)) {
			Player_Loses_Control();
			if (!Game_Flag_Query(493)) {
				Scene_Loop_Start_Special(kSceneLoopModeOnce, 0, true);
			}
			Game_Flag_Set(332);
			Game_Flag_Reset(493);
			Set_Enter(kSetBB02_BB04_BB06_BB51, kSceneBB04);
			Scene_Loop_Start_Special(kSceneLoopModeChangeSet, 3, false);
		}
		return true;
	}
	if (exitId == 2) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, 86.0f, -415.06f, 174.0f, 0, 1, false, 0)) {
			Loop_Actor_Walk_To_XYZ(kActorMcCoy, 179.0f, -415.06f, 274.0f, 0, 0, false, 0);
			Game_Flag_Set(kFlagBB02toBB03);
			Game_Flag_Reset(493);
			Set_Enter(kSetBB03, kSceneBB03);
		}
		return true;
	}
	return false;
}

bool SceneScriptBB02::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptBB02::SceneFrameAdvanced(int frame) {
	if (frame == 1) {
		Ambient_Sounds_Play_Sound(434, 40, -50, -50, 0);
	}
	if (frame == 124) {
		Ambient_Sounds_Play_Sound(434, 40, -50, -50, 0);
	}
}

void SceneScriptBB02::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptBB02::PlayerWalkedIn() {
	if (Game_Flag_Query(kFlagBB03toBB02)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 86.0f, -415.06f, 174.0f, 0, 0, false, 0);
		Game_Flag_Reset(kFlagBB03toBB02);
	} else if (Game_Flag_Query(333)) {
		Loop_Actor_Walk_To_XYZ(kActorMcCoy, 35.0f, -415.06f, -27.0f, 0, 0, false, 0);
		Player_Gains_Control();
		Game_Flag_Reset(333);
	}
}

void SceneScriptBB02::PlayerWalkedOut() {
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
}

void SceneScriptBB02::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
