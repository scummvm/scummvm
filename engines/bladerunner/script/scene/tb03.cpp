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

void SceneScriptTB03::InitializeScene() {
	if (Game_Flag_Query(kFlagUG17toTB03)) {
		Setup_Scene_Information(-260.0f, 0.15f, 2014.0f, 276);
	} else {
		Setup_Scene_Information(-152.0f,  0.0f, 1890.0f, 500);
	}

	Scene_Exit_Add_2D_Exit(0, 25, 227, 81, 300, 0);
	Scene_Exit_Add_2D_Exit(1, 298, 0, 639, 305, 0);

	Ambient_Sounds_Add_Looping_Sound(211, 16, 0, 1);
	Ambient_Sounds_Add_Sound(212, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(213, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(214, 2, 20, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(215, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	Ambient_Sounds_Add_Sound(216, 2, 15, 16, 20, 0, 0, -101, -101, 0, 0);
	if (Global_Variable_Query(kVariableChapter) <= 3) {
		Ambient_Sounds_Add_Looping_Sound(45, 25, 0, 1);
		Ambient_Sounds_Add_Sound(181, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(182, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(183, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(184, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(185, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(186, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(188, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(189, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(190, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(191, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(192, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(193, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(194, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
		Ambient_Sounds_Add_Sound(195, 5, 70, 12, 12, -100, 100, -101, -101, 0, 0);
	}

	Actor_Put_In_Set(kActorTyrellGuard, kSetTB02_TB03);
	Actor_Set_At_XYZ(kActorTyrellGuard, -38.53f, 2.93f, 1475.97f, 673);
	if (Global_Variable_Query(kVariableChapter) == 4) {
		int goal = Actor_Query_Goal_Number(kActorTyrellGuard);
		if (goal == 304) {
			Actor_Change_Animation_Mode(kActorTyrellGuard, 0);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 399);
		} else if (goal != 302) {
			Actor_Set_Goal_Number(kActorTyrellGuard, 300);
		}
	}

	if (Game_Flag_Query(kFlagUG17toTB03)) {
		if (!Game_Flag_Query(kFlagTB03Entered)) {
			Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
			Scene_Loop_Set_Default(1);
			Game_Flag_Set(kFlagTB03Entered);
		} else {
			Scene_Loop_Set_Default(1);
		}
		Game_Flag_Reset(kFlagUG17toTB03);
	} else {
		Scene_Loop_Start_Special(kSceneLoopModeLoseControl, 0, false);
		Scene_Loop_Set_Default(1);
	}
}

void SceneScriptTB03::SceneLoaded() {
	Obstacle_Object("SPHERE02", true);
}

bool SceneScriptTB03::MouseClick(int x, int y) {
	return false;
}

bool SceneScriptTB03::ClickedOn3DObject(const char *objectName, bool a2) {
	return false;
}

bool SceneScriptTB03::ClickedOnActor(int actorId) {
	return false;
}

bool SceneScriptTB03::ClickedOnItem(int itemId, bool a2) {
	return false;
}

bool SceneScriptTB03::ClickedOnExit(int exitId) {
	if (exitId == 0) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -260.0f, 0.15f, 2014.0f, 0, 1, false, 0)) {
			Actor_Set_Goal_Number(kActorTyrellGuard, 304);
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagTB03toUG17);
			Set_Enter(kSetUG17, kSceneUG17);
		}
		return true;
	}

	if (exitId == 1) {
		if (!Loop_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1774.0f, 0, 1, false, 0)) {
			Ambient_Sounds_Remove_All_Non_Looping_Sounds(true);
			Ambient_Sounds_Remove_All_Looping_Sounds(1);
			Game_Flag_Set(kFlagTB03toTB02);
			Set_Enter(kSetTB02_TB03, kSceneTB02);
			Async_Actor_Walk_To_XYZ(kActorMcCoy, -152.0f, 0.0f, 1702.0f, 0, false);
		}
		return true;
	}
	return false;
}

bool SceneScriptTB03::ClickedOn2DRegion(int region) {
	return false;
}

void SceneScriptTB03::SceneFrameAdvanced(int frame) {
}

void SceneScriptTB03::ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
}

void SceneScriptTB03::PlayerWalkedIn() {
	if (Actor_Query_Goal_Number(kActorTyrellGuard) == 304) {
		Player_Set_Combat_Mode(false);
		Actor_Says(kActorOfficerGrayford, 260, -1);
		Actor_Says(kActorMcCoy, 170, 14);
		Delay(1000);
		Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
	}
}

void SceneScriptTB03::PlayerWalkedOut() {
	Music_Stop(2);
}

void SceneScriptTB03::DialogueQueueFlushed(int a1) {
}

} // End of namespace BladeRunner
